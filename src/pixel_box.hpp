#ifndef SRC_PIXEL_BOX_HPP_
#define SRC_PIXEL_BOX_HPP_

#include <stdint.h>

// These IDs are persisted in EEPROM. Append new effects without renumbering.
enum class PixelPlayAnimation : uint8_t {
  dotForwardPulse = 0,
  colorCycle = 1,
  comet = 2,
  colorBlend = 3,
  lively = 4,
  NUMBER_OF_ANIMATIONS = 5
};

// Fork extensions: folder 0 is mandatory. Menu positions are separate from
// stored RFID IDs and from system prompt numbers.
constexpr uint8_t pixelQuietCardMode = 200; // Legacy alias for unlimited night light.
constexpr uint8_t pixelCycleAnimationCardMode = 201;
constexpr uint8_t pixelTimedNightCardMode = 202;
constexpr uint8_t pixelCycleColorCardMode = 203;
constexpr uint8_t pixelFixedColorCardMode = 204;
constexpr uint8_t pixelDefaultNightMinutes = 30;
// Keep 0 as the legacy default; 255 explicitly opts out of the time limit.
constexpr uint8_t pixelUnlimitedNightMinutes = 255;
constexpr uint16_t pixelNightLightHue = 5000;
constexpr uint16_t pixelModifierSound(uint8_t option) {
  // One night-light entry at 12; stored card IDs and prompt IDs stay unchanged.
  return option == 13 ? 1101u : option >= 14 ? 1104u + option - 14 : 970u + option;
}

constexpr uint16_t pixelHueFromOption(uint8_t option) {
  return static_cast<uint16_t>(option - 1) * 1000u;
}

constexpr uint16_t pixelNextHue(uint16_t hue) {
  // Red, orange, yellow, green, cyan, blue, violet, pink. Arbitrary hues
  // advance to the next palette colour; no extra mutable palette state.
  return hue < 5000 ? 5000 : hue < 11000 ? 11000 : hue < 22000 ? 22000 :
         hue < 32000 ? 32000 : hue < 44000 ? 44000 : hue < 50000 ? 50000 :
         hue < 58000 ? 58000 : 0;
}

// These checks also run in the real AVR compiler, where int is only 16 bits.
static_assert(pixelHueFromOption(33) == 32000, "Pixel hue overflows at option 33");
static_assert(pixelHueFromOption(65) == 64000, "Pixel hue must reach option 65");

#endif
