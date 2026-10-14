
#include "timer.hpp"

#include <Arduino.h>

void Timer::start(unsigned long timeout) {
  expireTime = static_cast<uint32_t>(millis()) + static_cast<uint32_t>(timeout);
  active = true;
}
bool Timer::isExpired() {
  if (not active)
    return true;
  if (static_cast<int32_t>(static_cast<uint32_t>(millis()) - expireTime) >= 0) {
    active = false;
    return true;
  }
  return false;
}

void Timer::stop() {
  active = false;
}

unsigned long Timer::remainingTime() {
  if (not active)
    return 0xffffffff;
  const int32_t remaining = expireTime - static_cast<uint32_t>(millis());
  return remaining > 0 ? static_cast<uint32_t>(remaining) : 0;
}

