#ifndef SRC_PIXEL_BOX_HPP_
#define SRC_PIXEL_BOX_HPP_

#include <stdint.h>

// These IDs are persisted in EEPROM. Append new effects without renumbering.
enum class PixelPlayAnimation : uint8_t {
  dotForwardPulse = 0,
  colorCycle = 1,
  NUMBER_OF_ANIMATIONS = 2
};

constexpr uint16_t pixelHueFromOption(uint8_t option) {
  return static_cast<uint16_t>(option - 1) * 1000u;
}

// These checks also run in the real AVR compiler, where int is only 16 bits.
static_assert(pixelHueFromOption(33) == 32000, "Pixel hue overflows at option 33");
static_assert(pixelHueFromOption(65) == 64000, "Pixel hue must reach option 65");

#endif
