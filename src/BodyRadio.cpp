#include "comms/BodyRadio.h"
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <atomic>
#include <string.h>
#include "config.h"
#include "app/BodyState.h"
#include "protocol.h"

BodyState g_body;

namespace {
const uint8_t kController[6] = CONTROLLER_MAC;
bool isZero(const uint8_t* m) { for (int i = 0; i < 6; i++) if (m[i]) return false; return true; }

std::atomic<uint32_t> g_ackSent{0};
std::atomic<uint32_t> g_ackFailed{0};

bool ensurePeer(const uint8_t* mac) {
  if (esp_now_is_peer_exist(mac)) return true;
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, mac, 6);
  peer.channel = WIFI_CHANNEL;
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = false;
  const esp_err_t result = esp_now_add_peer(&peer);
  return result == ESP_OK || result == ESP_ERR_ESPNOW_EXIST;
}

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
void onSent(const esp_now_send_info_t*, esp_now_send_status_t status) {
#else
void onSent(const uint8_t*, esp_now_send_status_t status) {
#endif
  if (status == ESP_NOW_SEND_SUCCESS) {
    g_ackSent.fetch_add(1, std::memory_order_relaxed);
  } else {
    g_ackFailed.fetch_add(1, std::memory_order_relaxed);
  }
}

// Receive ring: the ESP-NOW callback only copies bytes (it runs in the Wi-Fi task); parsing is in update().
struct RxSlot { uint8_t src[6]; uint8_t data[64]; uint8_t len; };
constexpr uint8_t RX_N = 8;
RxSlot g_rx[RX_N];
std::atomic<uint8_t> g_rxHead{0}, g_rxTail{0};

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
void onRecv(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
  if (!info || !data || len <= 0 || len > 64) return;
  const uint8_t* source = info->src_addr;
#else
void onRecv(const uint8_t* source, const uint8_t* data, int len) {
  if (!source || !data || len <= 0 || len > 64) return;
#endif
  const uint8_t h = g_rxHead.load(std::memory_order_relaxed);
  const uint8_t n = (uint8_t)((h + 1) % RX_N);
  if (n == g_rxTail.load(std::memory_order_acquire)) return;          // full: drop
  memcpy(g_rx[h].src, source, 6);
  memcpy(g_rx[h].data, data, (size_t)len);
  g_rx[h].len = (uint8_t)len;
  g_rxHead.store(n, std::memory_order_release);
}
}  // namespace

void BodyRadio::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  const esp_err_t channelResult =
      esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
  if (channelResult != ESP_OK) {
    Serial.printf("[body] failed to set Wi-Fi channel %d (%s, %d)\n",
                  WIFI_CHANNEL, esp_err_to_name(channelResult), channelResult);
    return;
  }
  Serial.printf("[body] MAC: %s, channel %d\n", WiFi.macAddress().c_str(),
                WIFI_CHANNEL);

  const esp_err_t initResult = esp_now_init();
  if (initResult != ESP_OK) {
    Serial.printf("[body] ESP-NOW init failed (%s, %d)\n",
                  esp_err_to_name(initResult), initResult);
    return;
  }
  const esp_err_t recvResult = esp_now_register_recv_cb(onRecv);
  const esp_err_t sendResult = esp_now_register_send_cb(onSent);
  if (recvResult != ESP_OK || sendResult != ESP_OK) {
    Serial.printf("[body] callback registration failed (rx=%s, tx=%s)\n",
                  esp_err_to_name(recvResult), esp_err_to_name(sendResult));
    return;
  }

  locked_ = !isZero(kController);
  if (locked_) {
    memcpy(peer_, kController, sizeof(peer_));
    ready_ = ensurePeer(peer_);
  } else {
    ready_ = true;
  }
  Serial.printf("[body] channel %d, %s, %s\n", WIFI_CHANNEL,
                locked_ ? "locked to controller MAC" : "accepting controller broadcasts",
                ready_ ? "radio OK" : "radio FAILED");
}

void BodyRadio::handleCommand(uint8_t feature, int16_t value) {
  // The body has no switchable features yet. To add one (e.g. a horn):
  //   1) add an ID in the shared protocol  2) handle it here.
  (void)feature; (void)value;
}

void BodyRadio::drain(uint32_t now) {
  while (g_rxTail.load(std::memory_order_relaxed) != g_rxHead.load(std::memory_order_acquire)) {
    const uint8_t t = g_rxTail.load(std::memory_order_relaxed);
    RxSlot& s = g_rx[t];
    const bool fromHand = !locked_ || memcmp(s.src, kController, 6) == 0;
    if (fromHand && s.len == sizeof(V3ControlPacket)) {
      V3ControlPacket control;
      memcpy(&control, s.data, sizeof(control));
      const bool valid = control.magic == DOOM_MAGIC && control.type == 3 &&
                         control.mode <= 1 && control.x >= -100 &&
                         control.x <= 100 && control.y >= -100 &&
                         control.y <= 100 && control.zoom <= 100;
      if (valid) {
        g_body.mode = control.mode;
        g_body.x = control.x;
        g_body.y = control.y;
        g_body.controlSequence = control.sequence;
        g_body.lastControlMs = now;
        g_body.hasControl = true;
        g_body.controlPackets++;
        if (control.mode == 1) sendControlAck(s.src, control);
      } else {
        g_body.badPackets++;
      }
    } else if (fromHand && s.len == sizeof(V3AuxCommand)) {
      V3AuxCommand command;
      memcpy(&command, s.data, sizeof(command));
      if (command.magic == DOOM_MAGIC && command.type == 4) {
        handleCommand(command.target, command.state);
        g_body.commandPackets++;
      } else {
        g_body.badPackets++;
      }
    }
    g_rxTail.store((uint8_t)((t + 1) % RX_N), std::memory_order_release);
  }
}

void BodyRadio::sendControlAck(const uint8_t* destination,
                               const V3ControlPacket& control) {
  if (!ready_ || !ensurePeer(destination)) {
    g_ackFailed.fetch_add(1, std::memory_order_relaxed);
    Serial.println("[body] control ACK failed: could not register HAND peer");
    return;
  }

  BodyAckPacket ack = {};
  ack.magic = DOOM_MAGIC;
  ack.type = 7;
  ack.status = 0;
  ack.mode = control.mode;
  ack.x = control.x;
  ack.y = control.y;
  ack.sequence = control.sequence;
  const esp_err_t result =
      esp_now_send(destination, reinterpret_cast<const uint8_t*>(&ack), sizeof(ack));
  if (result != ESP_OK) {
    g_ackFailed.fetch_add(1, std::memory_order_relaxed);
    Serial.printf("[body] control ACK send failed seq=%lu (%s, %d)\n",
                  static_cast<unsigned long>(control.sequence),
                  esp_err_to_name(result), result);
  } else if ((uint32_t)(millis() - lastAckPacketLog_) >= 1000) {
    lastAckPacketLog_ = millis();
    Serial.printf("[body] control accepted; ACK queued seq=%lu x=%d y=%d\n",
                  static_cast<unsigned long>(control.sequence),
                  control.x, control.y);
  }
}

void BodyRadio::update(uint32_t now) {
  drain(now);
  if ((uint32_t)(now - lastAckResultsLog_) >= 1000) {
    lastAckResultsLog_ = now;
    const uint32_t sent = g_ackSent.load(std::memory_order_relaxed);
    const uint32_t failed = g_ackFailed.load(std::memory_order_relaxed);
    if (sent != reportedAckSent_ || failed != reportedAckFailed_) {
      Serial.printf("[body] ACK TX results: delivered=%lu failed=%lu lastControl=%lu\n",
                    static_cast<unsigned long>(sent),
                    static_cast<unsigned long>(failed),
                    static_cast<unsigned long>(g_body.controlSequence));
      reportedAckSent_ = sent;
      reportedAckFailed_ = failed;
    }
  }
}
