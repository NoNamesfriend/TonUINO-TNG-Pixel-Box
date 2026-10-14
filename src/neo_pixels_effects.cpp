#include "neo_pixels.hpp"

#ifdef NeoPixels

namespace {
// One cosine pulse, calculated offline. No floating point or extra LED buffer.
const uint8_t pulseTable[] PROGMEM = {
  0, 2, 10, 21, 37, 57, 79, 103, 127, 152, 176, 198, 218, 234, 245, 253,
  255, 253, 245, 234, 218, 198, 176, 152, 128, 103, 79, 57, 37, 21, 10, 2
};
constexpr uint32_t warm = 0xffa62dUL;
constexpr uint32_t amber = 0xff6e0cUL;
constexpr uint32_t green = 0x2de664UL;
constexpr uint32_t cyan = 0x19beebUL;
constexpr uint32_t violet = 0xa548f0UL;
}

uint8_t Neo_pixels::pulseValue(uint32_t elapsed, uint16_t period)
{
  uint8_t value;
  const uint8_t index = (elapsed % period) * 32 / period;
  PROGMEM_read(&pulseTable[index], value);
  return value;
}

uint32_t Neo_pixels::scaleColor(uint32_t color, uint8_t value)
{
  const uint8_t r = static_cast<uint16_t>((color >> 16) & 255) * value / 255;
  const uint8_t g = static_cast<uint16_t>((color >> 8) & 255) * value / 255;
  const uint8_t b = static_cast<uint16_t>(color & 255) * value / 255;
  return (static_cast<uint32_t>(r) << 16) | (static_cast<uint16_t>(g) << 8) | b;
}

void Neo_pixels::setLightMode(LightMode mode)
{
  if (isShuttingDown()) return;
  setNightLight(false);
  lightMode = mode;
  sceneStarted = millis();
  renderedState = State::none;
  staticFrame = false;
}

void Neo_pixels::setTimedNightLight(uint8_t minutes)
{
  if (isShuttingDown()) return;
  setNightLight(true);
  timedNight = true;
  nightMinutes = minutes >= 1 && minutes <= 120 ? minutes : pixelDefaultNightMinutes;
  nightStarted = millis();
  animStepStart = millis() - cycleTime;
}

void Neo_pixels::call_on_feedback(Feedback next)
{
  if (isShuttingDown() || next == Feedback::none) return;
  if (feedback == next) return;
  if (isStateBlocking && next == Feedback::recognized) return;
  if (isStateBlocking) {
    isStateBlocking = false;
    state = normalState;
  }
  feedback = next;
  feedbackStarted = millis();
  animStepStart = millis() - cycleTime;
  staticFrame = false;
}

void Neo_pixels::setGameSelection(GameSelection selection)
{
  if (gameSelection == selection) return;
  gameSelection = selection;
  sceneStarted = millis();
  renderedState = State::none;
  staticFrame = false;
}

void Neo_pixels::setSleepTimer(uint8_t remaining, bool waitingForTrack)
{
  if (!sleepEnabled || sleepRemaining != remaining || sleepWaitingForTrack != waitingForTrack)
    staticFrame = false;
  sleepEnabled = true;
  sleepRemaining = remaining;
  sleepWaitingForTrack = waitingForTrack;
}

void Neo_pixels::clearSleepTimer()
{
  if (!sleepEnabled) return;
  sleepEnabled = false;
  renderedState = State::none;
  staticFrame = false;
}

void Neo_pixels::animComet(bool lively)
{
  if (renderedState != state) {
    renderedState = state;
    sceneStarted = millis();
    animStepStart = millis() - cycleTime;
  }
  if (static_cast<uint32_t>(millis() - animStepStart) < cycleTime) return;
  animStepStart = millis();
  staticFrame = false;
  const uint32_t elapsed = static_cast<uint32_t>(millis()) - sceneStarted;
  const uint16_t effectHue = lively ? (elapsed % 12000) * 65536UL / 12000 : hue;
  const uint16_t head = elapsed / 200 % pixels.numPixels();
  pixels.fill(pixels.ColorHSV(effectHue, 255, 6));
  for (uint8_t group = 0; group < (lively ? 2 : 1); ++group) {
    for (uint8_t trail = 0; trail < 4; ++trail) {
      const uint16_t index = (head + group * pixels.numPixels() / 2 + pixels.numPixels() * 4 - trail) % pixels.numPixels();
      const uint8_t value = trail == 0 ? 255 : trail == 1 ? 127 : trail == 2 ? 56 : 18;
      pixels.setPixelColor(index, pixels.ColorHSV(effectHue, 255, value));
    }
  }
  pixels.show();
}

void Neo_pixels::animColorBlend()
{
  if (renderedState != state) {
    renderedState = state;
    sceneStarted = millis();
  }
  const uint32_t elapsed = static_cast<uint32_t>(millis()) - sceneStarted;
  const uint8_t segment = elapsed / 4000 % 3;
  const uint16_t offsets[] = {0, 14418, 36044};
  const uint32_t a = pixels.ColorHSV(hue + offsets[segment], 255, 166);
  const uint32_t b = pixels.ColorHSV(hue + offsets[(segment + 1) % 3], 255, 166);
  const uint8_t weight = pulseValue(elapsed % 4000, 8000);
  uint32_t color = 0;
  for (uint8_t shift = 0; shift <= 16; shift += 8) {
    const uint8_t component = (static_cast<uint16_t>((a >> shift) & 255) * (255 - weight)
                            + static_cast<uint16_t>((b >> shift) & 255) * weight) / 255;
    color |= static_cast<uint32_t>(component) << shift;
  }
  displayColor(color);
}

void Neo_pixels::animFeedback(Feedback event, uint32_t elapsed)
{
  if (static_cast<uint32_t>(millis() - animStepStart) < cycleTime) return;
  animStepStart = millis();
  staticFrame = false;
  const uint8_t limit = lightMode == LightMode::quiet ? 64 : 255;
  pixels.fill(nightLightEnabled ? 0 : lightMode == LightMode::quiet ? scaleColor(warm, 33)
                                                               : pixels.ColorHSV(hue, 255, baseValue));
  if (event == Feedback::recognized) {
    const uint8_t reach = min(static_cast<uint32_t>((pixels.numPixels() + 1) / 2), elapsed * pixels.numPixels() / 1800 + 1);
    for (uint16_t i = 0; i < pixels.numPixels(); ++i)
      if (static_cast<int16_t>(i) >= static_cast<int16_t>(pixels.numPixels() / 2) - reach && i < (pixels.numPixels() + 1) / 2 + reach)
        pixels.setPixelColor(i, scaleColor(green, static_cast<uint16_t>(limit) * 204 / 255));
  }
  else if (event == Feedback::game_good || event == Feedback::game_bad) {
    const bool good = event == Feedback::game_good;
    for (uint16_t i = 0; i < pixels.numPixels(); ++i) {
      const uint8_t value = good ? static_cast<uint16_t>(pulseValue(elapsed, 1800)) * (static_cast<uint16_t>(limit) * 179 / 255) / 255
                                : (elapsed / 300 % 2 == (i < pixels.numPixels() / 2 ? 0 : 1) ? static_cast<uint16_t>(limit) * 153 / 255 : 13);
      pixels.setPixelColor(i, scaleColor(good ? green : amber, value));
    }
  }
  else if (event == Feedback::battery) {
    const uint8_t value = static_cast<uint16_t>(pulseValue(elapsed, 1600)) * (static_cast<uint16_t>(limit) * 153 / 255) / 255;
    pixels.fill(scaleColor(amber, value));
  }
  else {
    const bool on = event == Feedback::empty_card ? elapsed < 650 || (elapsed >= 950 && elapsed < 1450)
                  : event == Feedback::locked ? elapsed < 300
                  : elapsed < 220 || (elapsed >= 400 && elapsed < 620) || (elapsed >= 800 && elapsed < 1020);
    if (on) pixels.fill(scaleColor(event == Feedback::empty_card ? cyan : amber, static_cast<uint16_t>(limit) * 166 / 255));
  }
  pixels.show();
}

void Neo_pixels::animGame()
{
  if (static_cast<uint32_t>(millis() - animStepStart) < cycleTime) return;
  animStepStart = millis();
  staticFrame = false;
  const uint8_t value = 51 + static_cast<uint16_t>(pulseValue(static_cast<uint32_t>(millis()) - sceneStarted, 3000)) * 76 / 255;
  pixels.clear();
  for (uint16_t i = 0; i < pixels.numPixels(); ++i) {
    if (i < pixels.numPixels() / 2 || gameSelection != GameSelection::first)
      pixels.setPixelColor(i, scaleColor(i < pixels.numPixels() / 2 ? cyan : violet, value));
  }
  pixels.show();
}

void Neo_pixels::animSleep()
{
  if (!sleepWaitingForTrack && staticFrame && staticColor == sleepRemaining) return;
  if (sleepWaitingForTrack && static_cast<uint32_t>(millis() - animStepStart) < cycleTime) return;
  animStepStart = millis();
  pixels.clear();
  const uint8_t maximum = lightMode == LightMode::quiet ? 33 : 166;
  if (sleepWaitingForTrack) {
    const uint8_t value = 30 + pulseValue(static_cast<uint32_t>(millis()) - sceneStarted, 2800) / 4;
    const uint32_t color = scaleColor(warm, static_cast<uint16_t>(value) * maximum / 166);
    pixels.setPixelColor((pixels.numPixels() - 1) / 2, color);
    pixels.setPixelColor(pixels.numPixels() / 2, color);
    staticFrame = false;
  } else {
    const uint16_t extent = static_cast<uint16_t>(sleepRemaining) * ((pixels.numPixels() + 1) / 2);
    for (uint16_t i = 0; i < pixels.numPixels(); ++i) {
      const int16_t distance = i < pixels.numPixels() / 2 ? (pixels.numPixels() - 1) / 2 - i : i - pixels.numPixels() / 2;
      const int16_t value = static_cast<int16_t>(extent) - distance * 255;
      pixels.setPixelColor(i, scaleColor(warm, static_cast<uint16_t>(min(255, max(0, value))) * maximum / 255));
    }
    staticFrame = true;
    staticColor = sleepRemaining;
  }
  pixels.show();
}

void Neo_pixels::animNightLight()
{
  if (!timedNight) {
    displayNightLight();
    return;
  }
  if (nightMinutes == 0) {
    displayColor(0);
    return;
  }
  const uint32_t duration = static_cast<uint32_t>(nightMinutes) * 60000UL;
  const uint32_t elapsed = static_cast<uint32_t>(millis()) - nightStarted;
  if (elapsed >= duration) {
    nightMinutes = 0;
    staticFrame = false;
    displayColor(0);
    return;
  }
  const uint32_t remaining = duration - elapsed;
  if (remaining >= 60000) {
    displayColor(scaleColor(warm, nightLightValue));
    return;
  }
  if (static_cast<uint32_t>(millis() - animStepStart) < cycleTime) return;
  animStepStart = millis();
  staticFrame = false;
  const uint16_t extent = remaining * ((pixels.numPixels() + 1) / 2) * 255 / 60000;
  for (uint16_t i = 0; i < pixels.numPixels(); ++i) {
    const int16_t distance = i < pixels.numPixels() / 2 ? (pixels.numPixels() - 1) / 2 - i : i - pixels.numPixels() / 2;
    const uint8_t value = min(255, max(0, static_cast<int16_t>(extent) - distance * 255));
    pixels.setPixelColor(i, scaleColor(warm, static_cast<uint16_t>(value) * nightLightValue / 255));
  }
  pixels.show();
}

#endif // NeoPixels
