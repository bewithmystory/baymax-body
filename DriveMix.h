#pragma once
#include <stdint.h>
#include "config.h"
// Pure steering + speed maths (no hardware) so it can be unit-tested on a PC.
//   stick x: -100 (left) .. +100 (right)      stick y: -100 (back) .. +100 (forward)
//   output : left / right motor speed in percent, -100 (full reverse) .. +100 (full forward)

struct DriveOut { int left; int right; };

inline int dmAbs(int v) { return v < 0 ? -v : v; }
inline int dmClamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

// PIVOT (as described): straight = both motors; sideways = ONE motor only.
inline DriveOut mixPivot(int x, int y, bool turnSameSide) {
  const int ax = dmAbs(x), ay = dmAbs(y);
  if (ax == 0 && ay == 0) return {0, 0};
  if (ay >= ax) return {y, y};                              // forward / backward: both motors
  const bool right = x > 0;
  const bool runLeftMotor = turnSameSide ? !right : right;  // conventional: turning right runs the LEFT motor
  return runLeftMotor ? DriveOut{ax, 0} : DriveOut{0, ax};  // the other motor stays still
}

// TANK / arcade: smooth curves, spins on the spot when y = 0.
inline DriveOut mixTank(int x, int y) {
  int l = y + x, r = y - x;
  const int m = dmAbs(l) > dmAbs(r) ? dmAbs(l) : dmAbs(r);
  if (m > 100) { l = l * 100 / m; r = r * 100 / m; }
  return {l, r};
}

inline DriveOut mixDrive(int x, int y) {
  x = dmClamp(x, -100, 100); y = dmClamp(y, -100, 100);
  if (dmAbs(x) < INPUT_THRESHOLD_PCT) x = 0;
  if (dmAbs(y) < INPUT_THRESHOLD_PCT) y = 0;
#if DRIVE_MODE == DRIVE_TANK
  return mixTank(x, y);
#else
  return mixPivot(x, y, TURN_SAME_SIDE != 0);
#endif
}

// Speed percent (0..100) -> PWM duty (0..MOTOR_MAX_PWM). 0 stays 0; anything else starts at MOTOR_MIN_PWM
// so the motor actually turns instead of humming.
inline int pctToPwm(int pct) {
  if (pct <= 0) return 0;
  pct = pct * SPEED_LIMIT_PCT / 100;
  if (pct < 1) pct = 1;
  if (pct > 100) pct = 100;
  const int lo = MOTOR_MIN_PWM, hi = MOTOR_MAX_PWM > MOTOR_MIN_PWM ? MOTOR_MAX_PWM : MOTOR_MIN_PWM;
  return lo + (hi - lo) * pct / 100;
}

// Acceleration / deceleration limiter in integer milli-percent. Reversing passes through zero.
class SlewLimiter {
 public:
  SlewLimiter(int accelPctPerS, int decelPctPerS) : accel_(accelPctPerS), decel_(decelPctPerS) {}
  void reset(int pct = 0) { cur_ = (int32_t)pct * 1000; }
  int step(int targetPct, uint32_t dtMs) {
    const int32_t tgt = (int32_t)targetPct * 1000;
    int32_t goal = tgt;
    if (cur_ != 0 && tgt != 0 && ((cur_ > 0) != (tgt > 0))) goal = 0;   // reversing: slow to 0 first
    const bool speedingUp = dmAbs32(goal) > dmAbs32(cur_);
    const int32_t rate = speedingUp ? accel_ : decel_;                   // % per second
    if (rate <= 0) cur_ = goal;
    else {
      const int32_t maxd = rate * (int32_t)dtMs;                         // milli-percent
      if (goal > cur_) cur_ = (cur_ + maxd < goal) ? cur_ + maxd : goal;
      else             cur_ = (cur_ - maxd > goal) ? cur_ - maxd : goal;
    }
    return (int)((cur_ >= 0 ? cur_ + 500 : cur_ - 500) / 1000);
  }
 private:
  static int32_t dmAbs32(int32_t v) { return v < 0 ? -v : v; }
  int32_t accel_, decel_, cur_ = 0;
};
