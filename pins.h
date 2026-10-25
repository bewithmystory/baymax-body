#pragma once
// ============================================================================
//  BODY ESP32 (B)  -  PIN MAP                    CHIP: esp32
//  Motor driver: L298N MOTOR DRIVER MODULE (the red board with a heat sink).
//  Classic ESP32 DevKit assumed. Each motor = 1 PWM pin on ENA/ENB + 2 direction pins.
//
//    ESP32 pin        L298N module pin     Motor (screw terminal)
//    PIN_MOTOR_L_PWM  ENA                  LEFT   motor -> OUT1 / OUT2
//    PIN_MOTOR_L_IN1  IN1
//    PIN_MOTOR_L_IN2  IN2
//    PIN_MOTOR_R_PWM  ENB                  RIGHT  motor -> OUT3 / OUT4
//    PIN_MOTOR_R_IN1  IN3
//    PIN_MOTOR_R_IN2  IN4
//    GND              GND  (common ground with the battery is REQUIRED)
//
//  !! REMOVE the two jumpers on ENA and ENB, otherwise they tie the enable pins to +5 V:
//     no speed control, and the ESP32 pin would be connected to the 5 V rail.
//  Full wiring notes: WIRING.md
// ============================================================================
#define PIN_MOTOR_L_PWM  25   // [OUT]  -> ENA   (PWM speed, LEFT)
#define PIN_MOTOR_L_IN1  26   // [OUT]  -> IN1
#define PIN_MOTOR_L_IN2  27   // [OUT]  -> IN2
#define PIN_MOTOR_R_PWM  33   // [OUT]  -> ENB   (PWM speed, RIGHT)
#define PIN_MOTOR_R_IN1  32   // [OUT]  -> IN3
#define PIN_MOTOR_R_IN2  18   // [OUT]  -> IN4
#define PIN_STATUS_LED   23   // [OUT]  status LED (through ~330 ohm to GND)