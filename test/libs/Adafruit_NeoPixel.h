#ifndef TEST_ADAFRUIT_NEOPIXEL_H
#define TEST_ADAFRUIT_NEOPIXEL_H

#include <Arduino.h>
#include <vector>

constexpr uint16_t NEO_GRB = 0;
constexpr uint16_t NEO_KHZ800 = 0;

// In-memory driver: tests can inspect frames without the LED hardware.
class Adafruit_NeoPixel {
public:
  inline static std::vector<uint32_t> frame{};
  inline static unsigned int showCount{};
  Adafruit_NeoPixel(uint16_t count, uint8_t, uint16_t) : colors(count) {}
  void begin() {}
  void setBrightness(uint8_t) {}
  uint16_t numPixels() const { return colors.size(); }
  void show() { frame = colors; ++showCount; }
  void clear() { fill(0); }
  void fill(uint32_t color, uint16_t first = 0) {
    for (uint16_t i = first; i < colors.size(); ++i) colors[i] = color;
  }
  void setPixelColor(uint16_t i, uint32_t color) {
    if (i < colors.size()) colors[i] = color;
  }
  uint32_t getPixelColor(uint16_t i) const {
    return i < colors.size() ? colors[i] : 0;
  }
  static uint32_t ColorHSV(uint16_t hue, uint8_t saturation = 255, uint8_t value = 255) {
    // Mirror the real driver's integer HSV conversion; no electrical timing.
    uint8_t r, g, b;
    uint16_t h = (static_cast<uint32_t>(hue) * 1530 + 32768) / 65536;
    if (h < 510) {
      b = 0;
      if (h < 255) { r = 255; g = h; }
      else { r = 510 - h; g = 255; }
    } else if (h < 1020) {
      r = 0;
      if (h < 765) { g = 255; b = h - 510; }
      else { g = 1020 - h; b = 255; }
    } else if (h < 1530) {
      g = 0;
      if (h < 1275) { r = h - 1020; b = 255; }
      else { r = 255; b = 1530 - h; }
    } else { r = 255; g = b = 0; }
    const uint32_t v = 1 + value;
    const uint16_t s = 1 + saturation;
    const uint8_t s2 = 255 - saturation;
    return (((((r * s) >> 8) + s2) * v & 0xff00) << 8)
         | (((((g * s) >> 8) + s2) * v & 0xff00))
         | (((((b * s) >> 8) + s2) * v) >> 8);
  }
private:
  std::vector<uint32_t> colors;
};
#endif
