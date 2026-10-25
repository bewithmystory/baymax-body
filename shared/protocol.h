#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>

#define DOOM_MAGIC 0xB808
#define WIFI_CHANNEL 1

enum AuxTarget : uint8_t {
  AUX_CAMERA_POWER = 1,
  AUX_HEAD_LED = 2,
  AUX_LASER = 3,
  AUX_SERVO_1 = 4,
  AUX_SERVO_2 = 5,
  AUX_VOLUME = 6
};

typedef struct {
  uint32_t sequence;
  float distanceCm;
  uint8_t valid;
} SensorPacket;

typedef struct {
  uint8_t type;
  uint8_t laserOn;
  uint16_t magic;
} LaserCommand;

typedef struct __attribute__((packed)) {
  uint16_t magic;
  uint8_t type;
  uint8_t mode;
  int8_t x;
  int8_t y;
  uint8_t zoom;
  uint32_t sequence;
} V3ControlPacket;

typedef struct __attribute__((packed)) {
  uint16_t magic;
  uint8_t type;
  uint8_t target;
  uint8_t state;
  uint8_t value;
  uint32_t sequence;
} V3AuxCommand;

typedef struct __attribute__((packed)) {
  uint16_t magic;
  uint8_t type;
  uint8_t target;
  uint8_t state;
  uint8_t value;
  uint32_t sequence;
} AuxAckPacket;

typedef struct __attribute__((packed)) {
  uint16_t magic;
  uint8_t type;
  uint8_t status;
  uint8_t mode;
  int8_t x;
  int8_t y;
  uint32_t sequence;
} BodyAckPacket;

typedef struct __attribute__((packed)) {
  uint16_t magic;
  uint8_t type;
  uint32_t sequence;
  float distanceCm;
  uint8_t sensorValid;
  uint8_t laserEnabled;
  uint8_t headLedEnabled;
  uint8_t cameraPowerEnabled;
  uint8_t buzzerEnabled;
  uint8_t imuReady;
  float headingYawDeg;
  uint8_t servoPanTarget;
  uint8_t servoTiltTarget;
  uint8_t controlMode;
  int8_t joystickX;
  int8_t joystickY;
  uint32_t controlAgeMs;
  uint8_t radioChannel;
  uint32_t rxPacketCount;
  uint32_t rxControlCount;
  uint32_t rxAuxCount;
  uint32_t rxLegacyCount;
  uint32_t rxRejectedCount;
  uint32_t telemetryTxFailureCount;
  uint32_t lastAuxSequence;
  uint8_t lastAuxTarget;
  uint8_t lastAuxState;
  uint8_t lastAuxValue;
  uint32_t echoDurationUs;
} HeadStatusPacket;

static_assert(sizeof(V3ControlPacket) == 11, "V3 control wire size changed");
static_assert(sizeof(V3AuxCommand) == 10, "V3 auxiliary wire size changed");
static_assert(sizeof(AuxAckPacket) == 10, "Aux ACK wire size changed");
static_assert(sizeof(BodyAckPacket) == 11, "Body ACK wire size changed");
static_assert(sizeof(HeadStatusPacket) == 66, "HEAD status wire size changed");

#endif
