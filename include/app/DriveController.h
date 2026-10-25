#pragma once

#include <stdint.h>
#include "core/Component.h"
#include "DriveMix.h"

class MotorDriver;

class DriveController : public Component {
 public:
  DriveController(MotorDriver& left, MotorDriver& right)
      : left_(left), right_(right), slewL_(ACCEL_PCT_PER_S, DECEL_PCT_PER_S),
        slewR_(ACCEL_PCT_PER_S, DECEL_PCT_PER_S) {}

  void begin() override;
  void update(uint32_t now) override;

 private:
  MotorDriver& left_;
  MotorDriver& right_;
  SlewLimiter slewL_;
  SlewLimiter slewR_;
  uint32_t last_ = 0;
  uint32_t lastPrint_ = 0;
  int shownL_ = 0;
  int shownR_ = 0;
};
