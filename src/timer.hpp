#ifndef SRC_TIMER_HPP_
#define SRC_TIMER_HPP_

#include <stdint.h>

class Timer {
public:
  void start(unsigned long timeout);
  bool isExpired();
  void stop();
  bool isActive() { return active; }
  unsigned long remainingTime();
private:
  // Deadlines are compared modulo 32 bits. Timeouts must be below 2^31 ms.
  uint32_t expireTime{0    };
  bool          active    {false};
};

#endif /* SRC_TIMER_HPP_ */
