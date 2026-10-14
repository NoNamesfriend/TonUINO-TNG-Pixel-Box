#ifndef SRC_NEO_PIXELS_HPP_
#define SRC_NEO_PIXELS_HPP_

#include "constants.hpp"

#ifdef NeoPixels
#include <Adafruit_NeoPixel.h>
#include "pixel_box.hpp"

struct Settings;

class Neo_pixels {
public:
  Neo_pixels();

  enum class State : uint8_t {
    none, idle, admin, new_card, play, pause, next, previous, volume,
    night_light, play_preview, startup, shutdown
  };
  using PlayAnimation = PixelPlayAnimation;
  enum class LightMode : uint8_t { normal, quiet, lively };
  enum class Feedback : uint8_t { none, recognized, empty_card, read_error, locked, battery, game_good, game_bad };
  enum class GameSelection : uint8_t { none, first, second, quiz };

  void init(const Settings& settings);
  void loop(uint8_t volume = 0, uint8_t minVolume = 0, uint8_t maxVolume = 0);
  void shutdown();
  bool isShuttingDown() const { return state == State::shutdown; }
  bool shutdownFinished() const { return isShuttingDown() && step > (pixels.numPixels() + 1) / 2; }

  State getState() const { return state; }
  void setNormalState(State nextState);
  void setState(State nextState);
  void setNightLight(bool enabled);
  void setLightMode(LightMode mode);
  LightMode getLightMode() const { return lightMode; }
  void setTimedNightLight(uint8_t minutes);
  bool timedNightLightActive() const { return timedNight && nightLightEnabled && nightMinutes != 0; }
  bool nightLightKeepsAwake() const { return nightLightEnabled && (!timedNight || nightMinutes != 0); }
  void call_on_feedback(Feedback next);
  void setGameSelection(GameSelection selection);
  void setSleepTimer(uint8_t remaining, bool waitingForTrack);
  void clearSleepTimer();
  void call_on_volume() { setState(State::volume); if (state == State::volume) animStepStart = millis(); }
  void updateFromSettings(const Settings& settings);
  void previewPlayAnimation(PlayAnimation animation);
  void displayAll(uint16_t displayHue, uint8_t value);
  void displayAdmin();
  void displayNightLight();

private:
  Adafruit_NeoPixel pixels;
  State state{State::idle};
  State renderedState{State::none};
  State normalState{State::idle};
  PlayAnimation playAnimation{PlayAnimation::dotForwardPulse};
  PlayAnimation savedPlayAnimation{PlayAnimation::dotForwardPulse};
  uint16_t hue{};
  uint8_t baseValue{95};
  uint8_t nightLightValue{255};
  uint16_t step{};
  uint8_t miniStep{};
  uint32_t updatePause{};
  uint32_t animStepStart{};
  uint32_t staticColor{};
  bool staticFrame{};
  bool isPreparing{};
  bool isStateBlocking{};
  bool nightLightEnabled{};
  LightMode lightMode{LightMode::normal};
  Feedback feedback{Feedback::none};
  GameSelection gameSelection{GameSelection::none};
  uint8_t sleepRemaining{255};
  bool sleepEnabled{};
  bool sleepWaitingForTrack{};
  bool timedNight{};
  uint8_t nightMinutes{};
  uint32_t nightStarted{};
  uint32_t feedbackStarted{};
  uint32_t sceneStarted{};

  void animPower(bool shuttingDown);
  void animIdle();
  void animNewCard();
  void animAdmin();
  void animPlay();
  void animPause();
  void animTrack(bool backwards);
  void animVolume(uint8_t volume, uint8_t minVolume, uint8_t maxVolume);
  void dotForwardPulse();
  void colorCycle();
  void animComet(bool lively);
  void animColorBlend();
  void animFeedback(Feedback event, uint32_t elapsed);
  void animGame();
  void animSleep();
  void animNightLight();
  static uint8_t pulseValue(uint32_t elapsed, uint16_t period);
  static uint32_t scaleColor(uint32_t color, uint8_t value);
  void displayColor(uint32_t color);
  bool fadeTo(uint8_t target);
  static uint8_t valueFromColor(uint32_t color);
};
#endif // NeoPixels
#endif // SRC_NEO_PIXELS_HPP_
