#pragma once

#include <stdint.h>

struct BodyState {
  uint8_t mode = 0;
  int8_t x = 0;
  int8_t y = 0;
  uint32_t controlSequence = 0;
  uint32_t lastControlMs = 0;
  bool hasControl = false;
  bool failsafe = true;
  int8_t leftPct = 0;
  int8_t rightPct = 0;
  uint32_t controlPackets = 0;
  uint32_t commandPackets = 0;
  uint32_t badPackets = 0;
};

extern BodyState g_body;
