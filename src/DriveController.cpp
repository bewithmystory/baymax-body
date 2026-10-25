#include "app/DriveController.h"
#include <Arduino.h>
#include "config.h"
#include "app/BodyState.h"
#include "drivers/MotorDriver.h"
#include "protocol.h"

void DriveController::begin() { last_ = millis(); left_.drive(0); right_.drive(0); }

void DriveController::update(uint32_t now) {
  const uint32_t dt = (uint32_t)(now - last_);
  last_ = now;

  const bool fresh = g_body.hasControl && (uint32_t)(now - g_body.lastControlMs) < FAILSAFE_MS;
  g_body.failsafe = !fresh;
  const bool active = fresh && g_body.mode == 1;

  int l = 0, r = 0;
  if (active) {
    const DriveOut t = mixDrive(g_body.x, g_body.y);
    l = slewL_.step(t.left, dt);
    r = slewR_.step(t.right, dt);
  } else {                                   // fail-safe or HEAD mode: no ramp, stop now
    slewL_.reset(0);
    slewR_.reset(0);
  }
  g_body.leftPct = (int8_t)l;
  g_body.rightPct = (int8_t)r;
  left_.drive(l);
  right_.drive(r);

#if BODY_SERIAL_DEBUG
  if ((l != shownL_ || r != shownR_) && (uint32_t)(now - lastPrint_) >= 200) {
    lastPrint_ = now; shownL_ = l; shownR_ = r;
    Serial.printf("[drive] L=%4d%%  R=%4d%%  %s\n", l, r,
                  g_body.failsafe ? "FAILSAFE" : (g_body.mode == 1 ? "body" : "head-mode (stopped)"));
  }
#endif
}
