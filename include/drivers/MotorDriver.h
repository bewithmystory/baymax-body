#pragma once

#include <Arduino.h>
#include "DriveMix.h"
#include "config.h"
#include "core/Component.h"

class MotorDriver : public Component {
 public:
  MotorDriver(uint8_t pwmPin, uint8_t in1Pin, uint8_t in2Pin, bool inverted)
      : pwmPin_(pwmPin), in1Pin_(in1Pin), in2Pin_(in2Pin), inverted_(inverted) {}

  void begin() override {
    pinMode(in1Pin_, OUTPUT);
    pinMode(in2Pin_, OUTPUT);
    pinMode(pwmPin_, OUTPUT);
    digitalWrite(in1Pin_, LOW);
    digitalWrite(in2Pin_, LOW);
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    pwmReady_ = ledcAttach(pwmPin_, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
#else
    static uint8_t nextChannel = 0;
    channel_ = nextChannel++;
    pwmReady_ = ledcSetup(channel_, MOTOR_PWM_FREQ, MOTOR_PWM_BITS) > 0;
    ledcAttachPin(pwmPin_, channel_);
#endif
    drive(0);
  }

  void update(uint32_t) override {}

  void drive(int percent) {
    percent = constrain(percent, -100, 100);
    if (inverted_) percent = -percent;
    if (percent == 0) {
#if STOP_BRAKE
      digitalWrite(in1Pin_, HIGH);
      digitalWrite(in2Pin_, HIGH);
      writePwm((1U << MOTOR_PWM_BITS) - 1U);
#else
      digitalWrite(in1Pin_, LOW);
      digitalWrite(in2Pin_, LOW);
      writePwm(0);
#endif
      return;
    }
    digitalWrite(in1Pin_, percent > 0 ? HIGH : LOW);
    digitalWrite(in2Pin_, percent > 0 ? LOW : HIGH);
    writePwm(pctToPwm(abs(percent)));
  }

 private:
  void writePwm(uint32_t duty) {
    if (!pwmReady_) return;
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWrite(pwmPin_, duty);
#else
    ledcWrite(channel_, duty);
#endif
  }

  uint8_t pwmPin_;
  uint8_t in1Pin_;
  uint8_t in2Pin_;
  bool inverted_;
  bool pwmReady_ = false;
#if ESP_ARDUINO_VERSION_MAJOR < 3
  uint8_t channel_ = 0;
#endif
};
