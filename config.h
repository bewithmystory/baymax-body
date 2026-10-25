#pragma once
// ============================================================================
//  BODY ESP32 (B) - CONFIG   (everything you may want to tweak lives here)
// ============================================================================

// ---- Radio -----------------------------------------------------------------------
// MAC of the hand controller. All zeros accepts any controller; replies go to the packet sender.
// Set it later (hand prints its MAC at boot) to lock the link to your controller only.
#define CONTROLLER_MAC  {0x00,0x00,0x00,0x00,0x00,0x00}

// ---- Steering --------------------------------------------------------------------
//   DRIVE_PIVOT : forward/back = both motors | left or right = ONE motor only (as you described)
//   DRIVE_TANK  : smooth arcade mixing (curves while moving)
#define DRIVE_PIVOT  0
#define DRIVE_TANK   1
#define DRIVE_MODE   DRIVE_PIVOT

// PIVOT turn: which motor runs when you push the stick sideways?
//   0 = conventional: stick RIGHT runs the LEFT motor (bot turns right), stick LEFT runs the RIGHT motor
//   1 = same side:    stick RIGHT runs the RIGHT motor (bot turns left!) - set this only if your
//                     wiring/mounting needs it
#define TURN_SAME_SIDE   0

#define INPUT_THRESHOLD_PCT  3       // ignore stick values below this (the hand already applied its dead-zone)

// ---- Motors / PWM ----------------------------------------------------------------
#define MOTOR_L_INVERT   0           // 1 = swap direction of the LEFT motor  (if it spins backwards)
#define MOTOR_R_INVERT   0           // 1 = swap direction of the RIGHT motor
#define MOTOR_PWM_FREQ   20000       // Hz. 20 kHz = silent. The L298N is a slow bipolar chip: if the module gets hot or the motors are weak, try 5000-10000
#define MOTOR_PWM_BITS   8           // duty resolution (8 -> 0..255)
#define MOTOR_MIN_PWM    90          // below this a BO motor only hums (0-255)  -> raise if it stalls at low speed
#define MOTOR_MAX_PWM    255         // cap total power (0-255). The L298N drops ~2 V: 7.4 V pack -> ~5.4 V motors (fine for 6 V BO motors).
                                     // 12 V pack on 6 V motors -> use about 130
#define SPEED_LIMIT_PCT  100         // overall speed limit (1-100)
#define STOP_BRAKE       0           // 0 = coast to a stop, 1 = brake (L298N: both inputs HIGH + enable HIGH)

// ---- Smoothing (protects the driver, battery and gears) ---------------------------------
#define ACCEL_PCT_PER_S  300         // speed-up rate, % per second (300 = 0..100 % in ~0.33 s). 0 = instant
#define DECEL_PCT_PER_S    0         // slow-down rate. 0 = stop instantly when the stick is released

// ---- Safety ----------------------------------------------------------------------
#define FAILSAFE_MS     500          // no control packet for this long -> motors STOP (instantly)

// ---- Debug -----------------------------------------------------------------------
#define BODY_SERIAL_DEBUG  1         // print L/R speeds when they change