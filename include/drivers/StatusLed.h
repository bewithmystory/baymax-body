#pragma once

#include <Arduino.h>
#include "core/Component.h"
#include "pins.h"

class StatusLed : public Component {
 public:
  void begin() override {
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, LOW);
  }

  void update(uint32_t now) override {
    if ((uint32_t)(now - lastToggle_) >= 500) {
      lastToggle_ = now;
      state_ = !state_;
      digitalWrite(PIN_STATUS_LED, state_ ? HIGH : LOW);
    }
  }

 private:
  uint32_t lastToggle_ = 0;
  bool state_ = false;
};
