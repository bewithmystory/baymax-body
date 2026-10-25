#pragma once

#include <stdint.h>
#include "core/Component.h"
#include "protocol.h"

class BodyRadio : public Component {
 public:
  void begin() override;
  void update(uint32_t now) override;

 private:
  void drain(uint32_t now);
  void sendControlAck(const uint8_t* destination,
                      const V3ControlPacket& control);
  void handleCommand(uint8_t feature, int16_t value);

  uint8_t peer_[6] = {};
  bool locked_ = false;
  bool ready_ = false;
  uint32_t lastAckPacketLog_ = 0;
  uint32_t lastAckResultsLog_ = 0;
  uint32_t reportedAckSent_ = 0;
  uint32_t reportedAckFailed_ = 0;
};
