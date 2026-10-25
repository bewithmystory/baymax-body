// ============================================================================
//  DOOM BODY (ESP32 "B") - main.cpp
//  main.cpp only wires components together. To ADD a part: create a Component (.h/.cpp)
//  and add it to `components[]`. To REMOVE one: delete its line.
//
//  Receives the hand's joystick over ESP-NOW and drives two DC motors through an L298N.
// ============================================================================
#include <Arduino.h>
#include "core/Component.h"
#include "pins.h"
#include "config.h"
#include "app/DriveController.h"
#include "comms/BodyRadio.h"
#include "drivers/MotorDriver.h"
#include "drivers/StatusLed.h"

static MotorDriver     motorL(PIN_MOTOR_L_PWM, PIN_MOTOR_L_IN1, PIN_MOTOR_L_IN2, MOTOR_L_INVERT != 0);
static MotorDriver     motorR(PIN_MOTOR_R_PWM, PIN_MOTOR_R_IN1, PIN_MOTOR_R_IN2, MOTOR_R_INVERT != 0);
static StatusLed       led;
static BodyRadio       radio;
static DriveController drive(motorL, motorR);

// Order = begin()/update() order. Motors first so their pins are safe before anything else runs.
static Component* const components[] = { &motorL, &motorR, &led, &radio, &drive };
static const size_t kComponentCount = sizeof(components) / sizeof(components[0]);

void setup() {
  Serial.begin(115200);
  Serial.println("\n[doom-body] starting (L298N, ESP-NOW)");
  for (size_t i = 0; i < kComponentCount; i++) components[i]->begin();
}

void loop() {
  const uint32_t now = millis();
  for (size_t i = 0; i < kComponentCount; i++) components[i]->update(now);
}
