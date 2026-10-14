#include "neo_pixels.hpp"

#include "constants.hpp"

#ifdef NeoPixels

#include "settings.hpp"

Neo_pixels::Neo_pixels()
    : pixels(NeoPixelCount, NeoPixelPin, NEO_GRB + NEO_KHZ800) {}

void Neo_pixels::init(const Settings& settings)
{
  pixels.begin();
  pixels.clear();
  pixels.setBrightness(NeoPixelBrightness);

  state = State::startup;
  renderedState = State::none;
  normalState = State::idle;
  isStateBlocking = false;
  nightLightEnabled = false;
  timedNight = false;
  lightMode = LightMode::normal;
  feedback = Feedback::none;
  gameSelection = GameSelection::none;
  sleepEnabled = false;
  sceneStarted = millis();

  staticFrame = false;
  pixels.clear();
  updateFromSettings(settings);
  animPower(false);
}

void Neo_pixels::loop(uint8_t volume, uint8_t minVolume, uint8_t maxVolume)
{
  if (!isShuttingDown() && feedback != Feedback::none) {
    const uint32_t elapsed = static_cast<uint32_t>(millis()) - feedbackStarted;
    const uint16_t duration = feedback == Feedback::color_selected ? 1200 : feedback == Feedback::recognized ? 900 : feedback == Feedback::locked ? 550
                            : feedback == Feedback::game_good || feedback == Feedback::game_bad ? 1800 : 1600;
    if (elapsed < duration) {
      animFeedback(feedback, elapsed);
      return;
    }
    feedback = Feedback::none;
    renderedState = State::none;
    staticFrame = false;
  }
  if (nightLightEnabled && normalState != State::admin && !isShuttingDown()) {
    animNightLight();
    return;
  }
  if (state != State::startup && !isShuttingDown() && state != State::admin && state != State::play_preview
      && state != State::new_card && state != State::next && state != State::previous && state != State::volume) {
    if (sleepEnabled) {
      animSleep();
      return;
    }
    if (lightMode == LightMode::quiet) {
      displayColor(scaleColor(0xffa62dUL, 33));
      return;
    }
    if (gameSelection != GameSelection::none) {
      animGame();
      return;
    }
  }
  if (state != State::idle && state != State::admin && state != State::night_light)
    staticFrame = false;
  switch (state)
  {
  case State::idle:
    animIdle();
    break;
  case State::admin:
    animAdmin();
    break;
  case State::new_card:
    animNewCard();
    break;
  case State::play:
  case State::play_preview:
    animPlay();
    break;
  case State::pause:
    animPause();
    break;
  case State::next:
    animTrack(false);
    break;
  case State::previous:
    animTrack(true);
    break;
  case State::volume:
    animVolume(volume, minVolume, maxVolume);
    break;
  case State::startup:
    animPower(false);
    break;
  case State::shutdown:
    animPower(true);
    break;
  default:
    break;
  }
}

void Neo_pixels::setNormalState(State nextState)
{
  const bool changed = normalState != nextState;
  normalState = nextState;
  if (nextState != State::play)
    gameSelection = GameSelection::none;
  if (isShuttingDown() || state == State::startup)
    return;
  // Admin owns live previews, including while a night-light timer is running.
  if (state == State::play_preview && nextState == State::admin)
    return;
  if (nightLightEnabled) {
    if (state == State::play_preview) {
      playAnimation = savedPlayAnimation;
      renderedState = State::none;
      staticFrame = false;
    }
    state = nextState == State::admin ? State::admin : State::night_light;
    return;
  }
  if (isStateBlocking && nextState != State::idle && nextState != State::admin)
    return;
  // A repeated normal-state update must not truncate short feedback.
  if (!changed && (state == State::new_card || state == State::next
                  || state == State::previous || state == State::volume))
    return;
  if (state == State::play_preview) {
    playAnimation = savedPlayAnimation;
    renderedState = State::none;
  }
  isStateBlocking = false;
  state = nextState;
}

void Neo_pixels::setState(State nextState)
{
  if (nextState == State::idle || nextState == State::play
      || nextState == State::pause || nextState == State::admin) {
    setNormalState(nextState);
    return;
  }
  if (nightLightEnabled || isStateBlocking || isShuttingDown())
    return;
  state = nextState;
  if (nextState == State::new_card) {
    feedback = Feedback::none;
    feedbackStarted = millis();
  }
  if (nextState == State::new_card)
    isStateBlocking = true;
}

void Neo_pixels::setNightLight(bool enabled)
{
  if (isShuttingDown() || (nightLightEnabled == enabled && !timedNight))
    return;
  nightLightEnabled = enabled;
  timedNight = false;
  feedback = Feedback::none;
  staticFrame = false;
  isStateBlocking = false;
  renderedState = State::none;
  state = enabled ? State::night_light : normalState;
  if (enabled)
    displayNightLight();
}

void Neo_pixels::shutdown()
{
  if (isShuttingDown())
    return;
  nightLightEnabled = false;
  feedback = Feedback::none;
  isStateBlocking = false;
  staticFrame = false;
  state = State::shutdown;
  animPower(true);
}

void Neo_pixels::animPower(bool shuttingDown)
{
  const uint16_t pairs = (pixels.numPixels() + 1) / 2;
  if (renderedState != state) {
    renderedState = state;
    step = 0;
  }
  else if (step > pairs || static_cast<uint32_t>(millis() - animStepStart) < 100)
    return;

  animStepStart = millis();
  if (step == pairs) {
    ++step; // Keep the final frame for its full 100 ms before power-off.
    if (!shuttingDown) {
      state = normalState;
      renderedState = State::none;
    }
    return;
  }
  const uint16_t first = shuttingDown ? pixels.numPixels() / 2 + step : step;
  const uint16_t second = shuttingDown ? (pixels.numPixels() - 1) / 2 - step
                                       : pixels.numPixels() - 1 - step;
  const uint32_t color = shuttingDown ? 0 : pixels.ColorHSV(hue, 255, baseValue);
  pixels.setPixelColor(first, color);
  pixels.setPixelColor(second, color);
  pixels.show();
  ++step;
}

void Neo_pixels::animIdle()
{
  if (renderedState != state)
  {
    renderedState = state;
    displayAll(hue, baseValue);
  }

}

void Neo_pixels::animAdmin()
{
  if (renderedState != state)
  {
    displayAdmin();
  }
}

void Neo_pixels::animNewCard()
{
  if (renderedState != state) {
    renderedState = state;
    feedbackStarted = millis();
    animStepStart = millis() - cycleTime;
  }
  const uint32_t elapsed = static_cast<uint32_t>(millis()) - feedbackStarted;
  if (elapsed >= 900) {
    isStateBlocking = false;
    state = normalState;
    renderedState = State::none;
    return;
  }
  animFeedback(Feedback::recognized, elapsed);
}

void Neo_pixels::animPlay()
{
  switch (playAnimation)
  {
  case PlayAnimation::dotForwardPulse:
    dotForwardPulse();
    break;
  case PlayAnimation::colorCycle:
    colorCycle();
    break;
  case PlayAnimation::comet:
    animComet(false);
    break;
  case PlayAnimation::colorBlend:
    animColorBlend();
    break;
  case PlayAnimation::lively:
    animComet(true);
    break;
  default:
    break;
  }
}

void Neo_pixels::animPause()
{
  if (renderedState != state)
  {
    isPreparing = true;
    step = 1;
    miniStep = 1;
    updatePause = 100;
    animStepStart = millis();
    renderedState = state;
  }

  if (static_cast<uint32_t>(millis() - animStepStart) < updatePause)
    return;

  animStepStart = millis();

  if (isPreparing) {
    isPreparing = fadeTo(16);
    if (isPreparing)
      return;
  }

  uint8_t dimUpValue;
  if (16 * miniStep > 255)
    dimUpValue = 255;
  else
    dimUpValue = 16 * miniStep;

  uint8_t dimDownValue;
  if (255 - (16 * (miniStep - 18)) < 16)
    dimDownValue = 16;
  else
    dimDownValue = 255 - (16 * (miniStep - 18));

  pixels.fill(pixels.ColorHSV(hue, 255, miniStep <= 18 ? dimUpValue : dimDownValue));

  pixels.show();

  miniStep++;

  if (miniStep > 36)
    miniStep = 1;
}

void Neo_pixels::animTrack(bool backwards)
{
  if (renderedState != state) {
    step = 1;
    updatePause = cycleTime; // The main loop already advances every 50 ms.
    animStepStart = millis();
    renderedState = state;
    pixels.fill(pixels.ColorHSV(hue, 255, baseValue));
    pixels.show();
  }
  if (static_cast<uint32_t>(millis() - animStepStart) < updatePause)
    return;
  animStepStart = millis();
  pixels.fill(pixels.ColorHSV(hue, 255, baseValue));
  for (uint8_t trail = 0; trail < 4 && trail < step; ++trail) {
    const int16_t index = backwards ? pixels.numPixels() - step + trail : step - 1 - trail;
    pixels.setPixelColor(index, pixels.ColorHSV(hue, 255, 255 - 40 * trail));
  }
  pixels.show();
  if (++step > pixels.numPixels() + 4)
    state = normalState;
}

void Neo_pixels::animVolume(uint8_t currentVolume, uint8_t minVolume, uint8_t maxVolume)
{
  if (renderedState != state)
  {
    step = 0xffff;
    updatePause = 2000;
    animStepStart = millis();
    renderedState = state;
  }

  const uint8_t volume = min(max(currentVolume, minVolume), maxVolume);
  const uint16_t onPixels = maxVolume > minVolume
      ? map(volume, minVolume, maxVolume, 1, pixels.numPixels()) : pixels.numPixels();
  if (step != onPixels) {
    step = onPixels;
    pixels.clear();
    for (uint16_t i = 0; i < onPixels; ++i) {
      const uint16_t volumeHue = map(i, 0, pixels.numPixels(), 22000, -2000);
      pixels.setPixelColor(i, pixels.ColorHSV(volumeHue, 255, 255));
    }
    pixels.show();
  }

  if (static_cast<uint32_t>(millis() - animStepStart) < updatePause)
    return;

  animStepStart = millis();

  state = normalState;
}

void Neo_pixels::dotForwardPulse()
{
  // Keep the default background as the maximum for this effect. At the global
  // brightness of 20, a 191 background gives only 15 versus 20 RGB levels;
  // 95 gives about 7 versus 20, so the moving dots remain clearly visible.
  const uint8_t pulseBase = min(baseValue, static_cast<uint8_t>(95));
  if (renderedState != state)
  {
    isPreparing = true;
    step = 1;
    miniStep = 1;
    updatePause = 100;
    animStepStart = millis();
    renderedState = state;
  }

  if (static_cast<uint32_t>(millis() - animStepStart) < updatePause)
    return;

  animStepStart = millis();

  if (isPreparing) {
    isPreparing = fadeTo(pulseBase);
    if (isPreparing)
      return;
  }

  uint8_t dimUpValue;
  if (pulseBase + (16 * miniStep) > 255)
    dimUpValue = 255;
  else
    dimUpValue = pulseBase + (16 * miniStep);

  uint8_t dimDownValue;
  if (255 - (16 * (miniStep - 15)) < pulseBase)
    dimDownValue = pulseBase;
  else
    dimDownValue = 255 - (16 * (miniStep - 15));

  for (uint16_t i = 0; i < pixels.numPixels(); i++)
  {
    if ((i - step) % 4 == 0)
    {
      if (miniStep <= 15)
        pixels.setPixelColor(i, pixels.ColorHSV(hue, 255, dimUpValue));
      else
        pixels.setPixelColor(i, pixels.ColorHSV(hue, 255, dimDownValue));
    }
    else
      pixels.setPixelColor(i, pixels.ColorHSV(hue, 255, pulseBase));
  }

  pixels.show();

  miniStep++;

  if (miniStep > 30)
  {
    miniStep = 1;
    step++;
  }

  if (step > 4)
    step = 1;
}

void Neo_pixels::colorCycle()
{
  if (renderedState != state)
  {
    isPreparing = true;
    step = 0;
    miniStep = 1;
    updatePause = 50;
    animStepStart = millis();
    renderedState = state;
  }

  if (static_cast<uint32_t>(millis() - animStepStart) < updatePause)
    return;

  animStepStart = millis();

  uint16_t hueOffset = 65536UL * step / pixels.numPixels() / 6;
  uint32_t hueMultiplier = 65536UL / pixels.numPixels();

  for(uint16_t i = 0; i < pixels.numPixels(); i++)
  {
    pixels.setPixelColor(i, (pixels.ColorHSV(i * hueMultiplier - hueOffset, 255, baseValue)));
  }

  pixels.show();

  step++;

  if (step >= pixels.numPixels() * 6)
    step = 0;
}

void Neo_pixels::updateFromSettings(const Settings& settings)
{
  hue = settings.neoPixelHue;
  baseValue = settings.neoPixelBaseValue;
  savedPlayAnimation = static_cast<PlayAnimation>(settings.neoPixelPlayAnimation);
  playAnimation = savedPlayAnimation;
  nightLightValue = settings.neoPixelNightLightValue;
  renderedState = State::none;
  staticFrame = false;
}

void Neo_pixels::previewPlayAnimation(PlayAnimation animation)
{
  if (animation >= PlayAnimation::NUMBER_OF_ANIMATIONS || (nightLightEnabled && normalState != State::admin) || isStateBlocking || isShuttingDown())
    return;
  if (state == State::play_preview && playAnimation == animation)
    return; // Continue the current animation instead of restarting each menu cycle.
  playAnimation = animation;
  state = State::play_preview;
  renderedState = State::none;
}

void Neo_pixels::displayAll(uint16_t displayHue, uint8_t value)
{
  displayColor(pixels.ColorHSV(displayHue, 255, value));
}

void Neo_pixels::displayColor(uint32_t color)
{
  if (staticFrame && staticColor == color)
    return;
  pixels.fill(color);
  pixels.show();
  staticColor = color;
  staticFrame = true;
}

void Neo_pixels::displayAdmin()
{
  state = State::admin;
  playAnimation = savedPlayAnimation;
  renderedState = state;
  staticFrame = false;
  const uint16_t adminHue = hue + 65536UL / 6;

  for (uint16_t i = 0; i < pixels.numPixels(); i++)
  {

    if (i % 2 == 0)
      pixels.setPixelColor(i, pixels.ColorHSV(adminHue, 255, 255));
    else
      pixels.setPixelColor(i, pixels.ColorHSV(hue, 255, 255));
  }
  pixels.show();
}

void Neo_pixels::displayNightLight()
{
  displayAll(5000, nightLightValue);
}

bool Neo_pixels::fadeTo(uint8_t target)
{
  bool pending = false;
  for (uint16_t i = 0; i < pixels.numPixels(); ++i) {
    const uint8_t current = valueFromColor(pixels.getPixelColor(i));
    const uint8_t next = current < target ? min(static_cast<uint16_t>(current) + 16, static_cast<uint16_t>(target))
                                         : max(static_cast<int16_t>(current) - 16, static_cast<int16_t>(target));
    pixels.setPixelColor(i, pixels.ColorHSV(hue, 255, next));
    // At low global brightness, read-back is quantized. Never wait forever
    // for a value which the physical buffer cannot represent.
    if (next != target && valueFromColor(pixels.getPixelColor(i)) != current)
      pending = true;
    else if (next != target)
      pixels.setPixelColor(i, pixels.ColorHSV(hue, 255, target));
  }
  pixels.show();
  return pending;
}

uint8_t Neo_pixels::valueFromColor(uint32_t color)
{
  const uint8_t r = color >> 16;
  const uint8_t g = color >> 8;
  const uint8_t b = color;
  return max(r, max(g, b));
}

#endif // NeoPixels
