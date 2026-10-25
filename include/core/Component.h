#pragma once

#include <stdint.h>

class Component {
 public:
  virtual ~Component() = default;
  virtual void begin() = 0;
  virtual void update(uint32_t now) = 0;
};
