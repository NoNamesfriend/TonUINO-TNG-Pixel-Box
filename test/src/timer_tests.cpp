#include <gtest/gtest.h>
#include <Arduino.h>
#include "timer.hpp"

TEST(timer, deadline_and_remaining_time_survive_32_bit_rollover) {
  Timer timer;
  current_time = 0xffffff00UL;
  timer.start(1000);
  EXPECT_EQ(timer.remainingTime(), 1000u);
  EXPECT_FALSE(timer.isExpired());
  current_time = 0x20;
  EXPECT_EQ(timer.remainingTime(), 712u);
  EXPECT_FALSE(timer.isExpired());
  current_time = 0x2e8;
  EXPECT_EQ(timer.remainingTime(), 0u);
  EXPECT_TRUE(timer.isExpired());
  EXPECT_FALSE(timer.isActive());
}

TEST(timer, signed_boundary_does_not_expire_early_and_overdue_time_is_zero) {
  Timer timer;
  current_time = 0x7ffffff0UL;
  timer.start(50);
  current_time = 0x80000000UL;
  EXPECT_FALSE(timer.isExpired());
  EXPECT_EQ(timer.remainingTime(), 34u);
  current_time = 0x80000030UL;
  EXPECT_EQ(timer.remainingTime(), 0u);
  EXPECT_TRUE(timer.isExpired());
}

TEST(timer, stopped_timer_retains_the_inactive_contract) {
  Timer timer;
  EXPECT_TRUE(timer.isExpired());
  EXPECT_EQ(timer.remainingTime(), 0xffffffffUL);
  timer.start(30 * 60000UL);
  EXPECT_EQ(timer.remainingTime(), 1800000UL);
  timer.stop();
  EXPECT_FALSE(timer.isActive());
  EXPECT_TRUE(timer.isExpired());
  EXPECT_EQ(timer.remainingTime(), 0xffffffffUL);
}
