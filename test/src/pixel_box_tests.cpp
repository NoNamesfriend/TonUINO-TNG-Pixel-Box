#include <gtest/gtest.h>
#include "tonuino_fixture.hpp"

#ifdef NeoPixels
#include <Adafruit_NeoPixel.h>
#include <limits>

TEST(pixel_settings, imports_legacy_values_and_preserves_other_eeprom_regions) {
  memset(EEPROM.eeprom_mem, 0xff, EEPROM.max_len);
  Settings settings{};
  settings.resetSettings();
  settings.spkMaxVolume = 28;
  settings.shortCuts[0] = {3, pmode_t::album, 0, 0};
  settings.writeSettingsToFlash();
  settings.writeFolderSettingToFlash(3, 17);
  settings.writeExtShortCutToFlash(0, {4, pmode_t::hoerbuch, 0, 0});
  // Emulate the old fork's unsigned little-endian fields after the base layout.
  const int legacy = 140; // Frozen original AVR layout, independent of host padding.
  const uint8_t old[] = {0x00, 0xfa, 111, 1, 127}; // hue 64000
  for (unsigned int i = 0; i < sizeof(old); ++i) EEPROM.write(legacy + i, old[i]);
  EEPROM.write(153, 0xff); // No new signature.
  Settings loaded{};
  loaded.loadSettingsFromFlash();
  EXPECT_EQ(loaded.neoPixelHue, 64000);
  EXPECT_EQ(loaded.neoPixelBaseValue, 111);
  EXPECT_EQ(loaded.neoPixelPlayAnimation, 1);
  EXPECT_EQ(loaded.neoPixelNightLightValue, 127);
  EXPECT_EQ(loaded.spkMaxVolume, 28);
  EXPECT_EQ(loaded.shortCuts[0], (folderSettings{3, pmode_t::album, 0, 0}));
  EXPECT_EQ(loaded.readFolderSettingFromFlash(3), 17);
  folderSettings shortcut{};
  loaded.readExtShortCutFromFlash(0, shortcut);
  EXPECT_EQ(shortcut, (folderSettings{4, pmode_t::hoerbuch, 0, 0}));
  EXPECT_EQ(EEPROM.read(153), 'P');
  EXPECT_EQ(EEPROM.read(154), 'B');
  // The signed block wins on subsequent boots, even if legacy data changes.
  EEPROM.write(legacy, 0);
  loaded.loadSettingsFromFlash();
  EXPECT_EQ(loaded.neoPixelHue, 64000);
}

TEST(pixel_settings, erased_extension_uses_defaults_and_corrupt_animation_is_repaired) {
  memset(EEPROM.eeprom_mem, 0xff, EEPROM.max_len);
  Settings settings{};
  settings.resetSettings();
  for (int i = 140; i < 156; ++i) EEPROM.write(i, 0xff);
  settings.loadSettingsFromFlash();
  EXPECT_EQ(settings.neoPixelBaseValue, 95);
  EXPECT_EQ(settings.neoPixelNightLightValue, 255);
  EXPECT_EQ(settings.neoPixelPlayAnimation, 0);
  EEPROM.write(151, 255);
  settings.loadSettingsFromFlash();
  EXPECT_EQ(settings.neoPixelPlayAnimation, 0);
  EXPECT_EQ(EEPROM.read(151), 0);
}

TEST(pixel_settings, round_trip_includes_full_brightness_and_zero_values) {
  Settings settings{};
  settings.resetSettings();
  settings.neoPixelHue = 32000;
  settings.neoPixelBaseValue = 0;
  settings.neoPixelPlayAnimation = 1;
  settings.neoPixelNightLightValue = 255;
  settings.writeSettingsToFlash();
  Settings loaded{};
  loaded.loadSettingsFromFlash();
  EXPECT_EQ(loaded.neoPixelHue, 32000);
  EXPECT_EQ(loaded.neoPixelBaseValue, 0);
  EXPECT_EQ(loaded.neoPixelPlayAnimation, 1);
  EXPECT_EQ(loaded.neoPixelNightLightValue, 255);
}

class pixel_box_fixture : public tonuino_fixture {
public:
  pixel_box_fixture() {
    for (int i = 0; i < 6; ++i) {
      current_time += 100;
      tonuino.getNeoPixels().loop();
    }
  }
};

TEST_F(pixel_box_fixture, new_card_feedback_survives_play_entry_and_resumes_pause) {
  auto& pixels = tonuino.getNeoPixels();
  pixels.setState(Neo_pixels::State::new_card);
  pixels.setState(Neo_pixels::State::play);
  EXPECT_EQ(pixels.getState(), Neo_pixels::State::new_card);
  pixels.loop();
  pixels.setState(Neo_pixels::State::pause);
  for (int i = 0; i < 30; ++i) { current_time += 50; pixels.loop(); }
  EXPECT_EQ(pixels.getState(), Neo_pixels::State::pause);
}

TEST_F(pixel_box_fixture, volume_feedback_returns_to_idle_and_handles_equal_limits) {
  auto& pixels = tonuino.getNeoPixels();
  getSettings().spkMinVolume = getSettings().spkMaxVolume = 15;
  pixels.setState(Neo_pixels::State::idle);
  pixels.setState(Neo_pixels::State::volume);
  pixels.loop();
  current_time += 2001;
  pixels.loop();
  EXPECT_EQ(pixels.getState(), Neo_pixels::State::idle);
}

TEST_F(pixel_box_fixture, night_light_is_stable_and_releases_when_modifier_changes) {
  goto_idle();
  EXPECT_TRUE(tonuino.specialCard({0, pmode_t::night_light, 0, 0}));
  auto& pixels = tonuino.getNeoPixels();
  const auto frame = Adafruit_NeoPixel::frame;
  const auto shows = Adafruit_NeoPixel::showCount;
  pixels.setState(Neo_pixels::State::pause);
  current_time += 1000;
  getModifier().loop();
  pixels.loop();
  EXPECT_EQ(pixels.getState(), Neo_pixels::State::night_light);
  EXPECT_EQ(Adafruit_NeoPixel::frame, frame);
  EXPECT_EQ(Adafruit_NeoPixel::showCount, shows);
  EXPECT_TRUE(tonuino.specialCard({0, pmode_t::repeat_single, 0, 0}));
  EXPECT_EQ(pixels.getState(), Neo_pixels::State::pause);
}

TEST_F(pixel_box_fixture, color_cycle_continues_past_255_steps) {
  getSettings().neoPixelPlayAnimation = 1;
  auto& pixels = tonuino.getNeoPixels();
  pixels.updateFromSettings(getSettings());
  pixels.setState(Neo_pixels::State::play);
  pixels.loop();
  // 12 LEDs need 72 steps. More than 255 total steps must keep rendering.
  const auto start = Adafruit_NeoPixel::showCount;
  for (int i = 0; i < 300; ++i) { current_time += 50; pixels.loop(); }
  EXPECT_EQ(Adafruit_NeoPixel::showCount - start, 300);
  EXPECT_NE(Adafruit_NeoPixel::frame[0], Adafruit_NeoPixel::frame[1]);
}

TEST_F(pixel_box_fixture, pause_animation_timer_survives_32_bit_rollover) {
  auto& pixels = tonuino.getNeoPixels();
  const auto savedTime = current_time;
  current_time = 0xfffffff0UL;
  pixels.setState(Neo_pixels::State::pause);
  pixels.loop();
  const auto start = Adafruit_NeoPixel::showCount;
  current_time = 0x10;
  pixels.loop();
  EXPECT_EQ(Adafruit_NeoPixel::showCount, start);
  current_time = 0x90;
  pixels.loop();
  EXPECT_GT(Adafruit_NeoPixel::showCount, start);
  current_time = savedTime + 200;
}

TEST(pixel_driver, brightness_scales_stored_frame_and_readback_is_lossy) {
  Adafruit_NeoPixel driver(1, 18, NEO_GRB + NEO_KHZ800);
  driver.setBrightness(20);
  driver.setPixelColor(0, driver.ColorHSV(0, 255, 95));
  driver.show();
  EXPECT_EQ(Adafruit_NeoPixel::frame[0], 7u << 16);
  EXPECT_EQ(driver.getPixelColor(0), 85u << 16);
  driver.setBrightness(255);
  driver.show();
  EXPECT_EQ(Adafruit_NeoPixel::frame[0], 89u << 16);
}

TEST(pixel_settings, unchanged_settings_do_not_write_and_single_byte_changes_write_once) {
  Settings settings{};
  settings.resetSettings();
  EEPROM.writeCount = 0;
  settings.writeSettingsToFlash();
  EXPECT_EQ(EEPROM.writeCount, 0u);
  settings.neoPixelBaseValue = 111;
  settings.writeSettingsToFlash();
  EXPECT_EQ(EEPROM.writeCount, 1u);
  settings.spkMaxVolume = 28;
  settings.writeSettingsToFlash();
  EXPECT_EQ(EEPROM.writeCount, 2u);
  settings.writeFolderSettingToFlash(5, 18);
  EEPROM.writeCount = 0;
  settings.writeFolderSettingToFlash(5, 18);
  EXPECT_EQ(EEPROM.writeCount, 0u);
  settings.clearEEPROM();
  EEPROM.writeCount = 0;
  settings.clearEEPROM();
  EXPECT_EQ(EEPROM.writeCount, 0u);
}

TEST(pixel_settings, hue_options_cover_the_full_avr_unsigned_range) {
  static_assert(pixelHueFromOption(33) == 32000, "AVR must not overflow at option 33");
  static_assert(pixelHueFromOption(65) == 64000, "AVR hue range must reach 64000");
  for (uint8_t option = 1; option <= 65; ++option)
    EXPECT_EQ(pixelHueFromOption(option), (static_cast<uint32_t>(option) - 1) * 1000);
}

TEST(pixel_animations, startup_keeps_pair_frames_and_does_not_advance_time) {
  Settings settings{};
  settings.resetSettings();
  Neo_pixels pixels;
  const auto start = current_time;
  pixels.init(settings);
  EXPECT_EQ(current_time, start);
  EXPECT_EQ(pixels.getState(), Neo_pixels::State::startup);
  for (unsigned int i = 0; i < NeoPixelCount; ++i)
    EXPECT_EQ(Adafruit_NeoPixel::frame[i] != 0, i == 0 || i == NeoPixelCount - 1);
  const auto shows = Adafruit_NeoPixel::showCount;
  pixels.setNormalState(Neo_pixels::State::play);
  current_time += 99;
  pixels.loop();
  EXPECT_EQ(Adafruit_NeoPixel::showCount, shows);
  ++current_time;
  pixels.loop();
  EXPECT_EQ(Adafruit_NeoPixel::showCount, shows + 1);
  for (int i = 0; i < 5; ++i) { current_time += 100; pixels.loop(); }
  EXPECT_EQ(pixels.getState(), Neo_pixels::State::play);
  EXPECT_EQ(Adafruit_NeoPixel::showCount, shows + 5);
}

TEST_F(pixel_box_fixture, shutdown_waits_for_final_frame_and_overrides_night_light) {
  auto& pixels = tonuino.getNeoPixels();
  pixels.setNightLight(true);
  const auto start = current_time;
  getMp3().called_sleep = false;
  tonuino.shutdown();
  EXPECT_EQ(current_time, start);
  EXPECT_TRUE(pixels.isShuttingDown());
  EXPECT_FALSE(pixels.shutdownFinished());
  EXPECT_EQ(pin_value[shutdownPin], HIGH);
  for (unsigned int i = 0; i < NeoPixelCount; ++i)
    EXPECT_EQ(Adafruit_NeoPixel::frame[i] == 0, i == NeoPixelCount / 2 - 1 || i == NeoPixelCount / 2);
  const auto frame = Adafruit_NeoPixel::frame;
  tonuino.shutdown(); // Repeated requests must not reset the animation.
  pixels.setState(Neo_pixels::State::new_card);
  pixels.setNightLight(true);
  EXPECT_EQ(Adafruit_NeoPixel::frame, frame);
  for (int i = 0; i < 12; ++i) {
    const auto before = current_time;
    execute_cycle();
    EXPECT_EQ(current_time - before, cycleTime);
    EXPECT_EQ(pin_value[shutdownPin], HIGH);
    EXPECT_FALSE(getMp3().called_sleep);
  }
  for (auto color : Adafruit_NeoPixel::frame) EXPECT_EQ(color, 0u);
  execute_cycle(); // At 600 ms the last frame has completed its full duration.
  EXPECT_TRUE(pixels.shutdownFinished());
  EXPECT_EQ(pin_value[shutdownPin], LOW);
  EXPECT_TRUE(getMp3().called_sleep);
}

TEST_F(pixel_box_fixture, shutdown_timer_survives_32_bit_rollover) {
  auto& pixels = tonuino.getNeoPixels();
  current_time = 0xfffffff0UL;
  pixels.shutdown();
  const auto shows = Adafruit_NeoPixel::showCount;
  current_time = 0x10;
  pixels.loop();
  EXPECT_EQ(Adafruit_NeoPixel::showCount, shows);
  current_time = 0x90;
  pixels.loop();
  EXPECT_EQ(Adafruit_NeoPixel::showCount, shows + 1);
  EXPECT_FALSE(pixels.shutdownFinished());
}

TEST_F(pixel_box_fixture, static_previews_and_volume_only_show_when_the_image_changes) {
  auto& pixels = tonuino.getNeoPixels();
  pixels.displayAll(32000, 255);
  const auto start = Adafruit_NeoPixel::showCount;
  for (int i = 0; i < 20; ++i) pixels.displayAll(32000, 255);
  EXPECT_EQ(Adafruit_NeoPixel::showCount, start);
  pixels.displayAll(33000, 255);
  EXPECT_EQ(Adafruit_NeoPixel::showCount, start + 1);
  pixels.setState(Neo_pixels::State::volume);
  pixels.loop(15, 5, 25);
  const auto volumeStart = Adafruit_NeoPixel::showCount;
  for (int i = 0; i < 20; ++i) { current_time += 50; pixels.loop(15, 5, 25); }
  EXPECT_EQ(Adafruit_NeoPixel::showCount, volumeStart);
  pixels.loop(25, 5, 25);
  EXPECT_EQ(Adafruit_NeoPixel::showCount, volumeStart + 1);
  current_time += 1001;
  pixels.loop(25, 5, 25);
  EXPECT_EQ(pixels.getState(), Neo_pixels::State::idle);
}

TEST_F(pixel_box_fixture, color_cycle_wraps_after_exactly_72_frames) {
  getSettings().neoPixelPlayAnimation = 1;
  auto& pixels = tonuino.getNeoPixels();
  pixels.updateFromSettings(getSettings());
  pixels.setState(Neo_pixels::State::play);
  pixels.loop();
  current_time += 50; pixels.loop();
  const auto first = Adafruit_NeoPixel::frame;
  for (int i = 1; i < NeoPixelCount * 6; ++i) { current_time += 50; pixels.loop(); }
  EXPECT_NE(Adafruit_NeoPixel::frame, first);
  current_time += 50; pixels.loop();
  EXPECT_EQ(Adafruit_NeoPixel::frame, first);
  current_time += 50; pixels.loop();
  EXPECT_NE(Adafruit_NeoPixel::frame, first);
}

TEST_F(pixel_box_fixture, track_feedback_keeps_50_ms_tempo_and_mirrors_the_trail) {
  auto& pixels = tonuino.getNeoPixels();
  pixels.setState(Neo_pixels::State::next);
  pixels.loop();
  const auto start = Adafruit_NeoPixel::showCount;
  current_time += 25; pixels.loop();
  EXPECT_EQ(Adafruit_NeoPixel::showCount, start);
  current_time += 25; pixels.loop();
  EXPECT_EQ(Adafruit_NeoPixel::showCount, start + 1);
  for (int i = 0; i < 3; ++i) { current_time += 50; pixels.loop(); }
  const auto next = Adafruit_NeoPixel::frame;
  const uint8_t expectedValues[] = {135, 175, 215, 255};
  for (unsigned int i = 0; i < NeoPixelCount; ++i) {
    const uint8_t value = i < 4 ? expectedValues[i] : 95;
    EXPECT_EQ(next[i], static_cast<uint32_t>(value * (NeoPixelBrightness + 1) >> 8) << 16);
  }
  pixels.setState(Neo_pixels::State::previous);
  pixels.loop();
  for (int i = 0; i < 4; ++i) { current_time += 50; pixels.loop(); }
  for (unsigned int i = 0; i < NeoPixelCount; ++i)
    EXPECT_EQ(Adafruit_NeoPixel::frame[i], next[NeoPixelCount - 1 - i]);
}

TEST_F(pixel_box_fixture, pulse_preparation_finishes_at_low_and_full_base_brightness) {
  auto& pixels = tonuino.getNeoPixels();
  for (uint8_t base : {0, 1, 15, 95, 255}) {
    getSettings().neoPixelBaseValue = base;
    pixels.updateFromSettings(getSettings());
    pixels.setState(Neo_pixels::State::idle);
    pixels.loop();
    pixels.setState(Neo_pixels::State::play);
    pixels.loop();
    for (int i = 0; i < 20; ++i) { current_time += 100; pixels.loop(); }
    if (base != 255) {
      bool hasDot = false;
      for (unsigned int i = 1; i < NeoPixelCount; ++i)
        hasDot |= Adafruit_NeoPixel::frame[i] != Adafruit_NeoPixel::frame[0];
      EXPECT_TRUE(hasDot) << "base=" << static_cast<int>(base);
    }
    pixels.setState(Neo_pixels::State::pause);
    pixels.loop();
    for (int i = 0; i < 30; ++i) { current_time += 100; pixels.loop(); }
    EXPECT_GT(Adafruit_NeoPixel::frame[0], 0u);
  }
}

#ifdef MEMORY_GAME
TEST_F(pixel_box_fixture, memory_uses_play_animation_and_volume_feedback) {
  card_in({2, pmode_t::memory_game, 0, 0}, 20);
  ASSERT_TRUE(SM_tonuino::is_in_state<StartPlay<Memory>>());
  execute_cycle();
  getMp3().end_track();
  execute_cycle();
  execute_cycle_for_ms(dfPlayer_timeUntilStarts + 50);
  ASSERT_TRUE(SM_tonuino::is_in_state<Memory>());
  execute_cycle_for_ms(1000);
  EXPECT_EQ(tonuino.getNeoPixels().getState(), Neo_pixels::State::play);
  const auto volume = getVolume();
  button_for_command(command::volume_up, state_for_command::play);
  ASSERT_EQ(getVolume(), volume + 1);
  EXPECT_EQ(tonuino.getNeoPixels().getState(), Neo_pixels::State::volume);
  execute_cycle_for_ms(2100);
  EXPECT_EQ(tonuino.getNeoPixels().getState(), Neo_pixels::State::play);
}
#endif

#ifdef QUIZ_GAME
TEST_F(pixel_box_fixture, quiz_uses_play_animation_and_volume_changes_only_for_volume) {
  card_in({2, pmode_t::quiz_game, 2, 0}, 30);
  ASSERT_TRUE(SM_tonuino::is_in_state<StartPlay<Quiz>>());
  execute_cycle();
  getMp3().end_track();
  execute_cycle();
  execute_cycle_for_ms(dfPlayer_timeUntilStarts + 50);
  ASSERT_TRUE(SM_tonuino::is_in_state<Quiz>());
  execute_cycle_for_ms(1000);
  EXPECT_EQ(tonuino.getNeoPixels().getState(), Neo_pixels::State::play);
  const auto volume = getVolume();
  button_for_command(command::volume_up, state_for_command::play);
  EXPECT_EQ(getVolume(), volume + 1);
  EXPECT_EQ(tonuino.getNeoPixels().getState(), Neo_pixels::State::volume);
  execute_cycle_for_ms(2100);
  EXPECT_EQ(tonuino.getNeoPixels().getState(), Neo_pixels::State::play);
  getMp3().end_track();
  execute_cycle();
  button_for_command(command::pause, state_for_command::play); // Ask a question.
  const auto answerVolume = getVolume();
  button_for_command(command::volume_up, state_for_command::play); // Select an answer.
  EXPECT_EQ(getVolume(), answerVolume);
  EXPECT_EQ(tonuino.getNeoPixels().getState(), Neo_pixels::State::play);
}
#endif

TEST_F(pixel_box_fixture, standby_uses_the_same_async_shutdown_path) {
  getSettings().standbyTimer = 1;
  tonuino.setStandbyTimer();
  current_time += 60001;
  getMp3().called_sleep = false;
  execute_cycle();
  EXPECT_TRUE(tonuino.getNeoPixels().isShuttingDown());
  EXPECT_FALSE(tonuino.getNeoPixels().shutdownFinished());
  EXPECT_EQ(pin_value[shutdownPin], HIGH);
  EXPECT_FALSE(getMp3().called_sleep);
  execute_cycle_for_ms(600);
  EXPECT_EQ(pin_value[shutdownPin], LOW);
  EXPECT_TRUE(getMp3().called_sleep);
}

#ifdef BAT_VOLTAGE_MEASUREMENT
TEST_F(pixel_box_fixture, empty_battery_uses_the_same_async_shutdown_path) {
  pin_value[voltageMeasurementPin] = 0;
  execute_cycle();
  current_time += 10001;
  getMp3().called_sleep = false;
  execute_cycle();
  EXPECT_TRUE(tonuino.getNeoPixels().isShuttingDown());
  EXPECT_FALSE(tonuino.getNeoPixels().shutdownFinished());
  EXPECT_EQ(pin_value[shutdownPin], HIGH);
  EXPECT_FALSE(getMp3().called_sleep);
  execute_cycle_for_ms(600);
  EXPECT_EQ(pin_value[shutdownPin], LOW);
  EXPECT_TRUE(getMp3().called_sleep);
}
#endif


TEST(pixel_settings, all_four_effect_ids_round_trip_without_changing_the_eeprom_layout) {
  static_assert(static_cast<uint8_t>(PixelPlayAnimation::dotForwardPulse) == 0);
  static_assert(static_cast<uint8_t>(PixelPlayAnimation::colorCycle) == 1);
  Settings settings{};
  settings.resetSettings();
  settings.writeExtShortCutToFlash(0, {7, pmode_t::album, 0, 0});
  for (uint8_t effect = 0; effect < 4; ++effect) {
    settings.neoPixelPlayAnimation = effect;
    settings.writeSettingsToFlash();
    Settings loaded{};
    loaded.loadSettingsFromFlash();
    EXPECT_EQ(loaded.neoPixelPlayAnimation, effect);
    EXPECT_EQ(EEPROM.read(151), effect);
    EXPECT_EQ(EEPROM.read(155), 1);
    folderSettings shortcut{};
    loaded.readExtShortCutFromFlash(0, shortcut);
    EXPECT_EQ(shortcut, (folderSettings{7, pmode_t::album, 0, 0}));
  }
}

TEST_F(pixel_box_fixture, comet_keeps_one_peak_three_trail_pixels_and_a_2400_ms_period) {
  auto& pixels = tonuino.getNeoPixels();
  getSettings().neoPixelPlayAnimation = 2;
  pixels.updateFromSettings(getSettings());
  pixels.setState(Neo_pixels::State::play);
  pixels.loop();
  const auto first = Adafruit_NeoPixel::frame;
  EXPECT_EQ(first[0], 20u << 16);
  EXPECT_EQ(first[11], 10u << 16);
  EXPECT_EQ(first[10], 4u << 16);
  EXPECT_EQ(first[9], 1u << 16);
  current_time += 200;
  pixels.loop();
  EXPECT_EQ(Adafruit_NeoPixel::frame[1], 20u << 16);
  EXPECT_NE(Adafruit_NeoPixel::frame, first);
  current_time += 2200;
  pixels.loop();
  EXPECT_EQ(Adafruit_NeoPixel::frame, first);
}

TEST_F(pixel_box_fixture, color_blend_changes_color_uniformly_and_keeps_advancing) {
  auto& pixels = tonuino.getNeoPixels();
  getSettings().neoPixelPlayAnimation = 3;
  pixels.updateFromSettings(getSettings());
  pixels.setState(Neo_pixels::State::play);
  pixels.loop();
  const auto first = Adafruit_NeoPixel::frame;
  current_time += 2000;
  pixels.loop();
  EXPECT_NE(Adafruit_NeoPixel::frame, first);
  for (auto color : Adafruit_NeoPixel::frame) EXPECT_EQ(color, Adafruit_NeoPixel::frame[0]);
  const auto middle = Adafruit_NeoPixel::frame;
  current_time += 2000;
  pixels.loop();
  EXPECT_NE(Adafruit_NeoPixel::frame, middle);
  current_time += 8000;
  pixels.loop();
  EXPECT_EQ(Adafruit_NeoPixel::frame, first);
}

TEST_F(pixel_box_fixture, quiet_light_is_warm_static_and_can_coexist_with_sleep_timer) {
  goto_play({2, pmode_t::album, 0, 0});
  ASSERT_TRUE(tonuino.specialCard({0, pmode_t::sleep_timer, 30, 0}));
  ASSERT_EQ(getModifier().getActive(), pmode_t::sleep_timer);
  uint8_t before[EEPROM.max_len];
  memcpy(before, EEPROM.eeprom_mem, sizeof(before));
  ASSERT_TRUE(tonuino.specialCard({0, pmode_t::pixel_quiet, 0, 0}));
  EXPECT_EQ(tonuino.getNeoPixels().getLightMode(), Neo_pixels::LightMode::quiet);
  EXPECT_EQ(getModifier().getActive(), pmode_t::sleep_timer);
  EXPECT_EQ(memcmp(before, EEPROM.eeprom_mem, sizeof(before)), 0);
  ASSERT_TRUE(tonuino.specialCard({0, pmode_t::pixel_quiet, 0, 0}));
  EXPECT_EQ(tonuino.getNeoPixels().getLightMode(), Neo_pixels::LightMode::normal);
  EXPECT_EQ(getModifier().getActive(), pmode_t::sleep_timer);
  auto& pixels = tonuino.getNeoPixels();
  pixels.clearSleepTimer();
  pixels.setLightMode(Neo_pixels::LightMode::quiet);
  pixels.loop();
  const auto frame = Adafruit_NeoPixel::frame;
  const auto shows = Adafruit_NeoPixel::showCount;
  EXPECT_EQ(frame[0], 0x020100u);
  for (int i = 0; i < 20; ++i) { current_time += 50; pixels.loop(); }
  EXPECT_EQ(Adafruit_NeoPixel::frame, frame);
  EXPECT_EQ(Adafruit_NeoPixel::showCount, shows);
}

TEST_F(pixel_box_fixture, lively_profile_has_two_peaks_and_does_not_overwrite_the_saved_animation) {
  auto& pixels = tonuino.getNeoPixels();
  const auto saved = getSettings().neoPixelPlayAnimation;
  pixels.setState(Neo_pixels::State::play);
  pixels.setLightMode(Neo_pixels::LightMode::lively);
  pixels.loop();
  EXPECT_EQ(Adafruit_NeoPixel::frame[0], 20u << 16);
  EXPECT_EQ(Adafruit_NeoPixel::frame[6], 20u << 16);
  const auto first = Adafruit_NeoPixel::frame;
  current_time += 200;
  pixels.loop();
  EXPECT_NE(Adafruit_NeoPixel::frame, first);
  EXPECT_EQ(getSettings().neoPixelPlayAnimation, saved);
  pixels.setLightMode(Neo_pixels::LightMode::normal);
  EXPECT_EQ(getSettings().neoPixelPlayAnimation, saved);
}

TEST_F(pixel_box_fixture, timed_night_defaults_to_30_minutes_and_only_ends_the_light) {
  auto& pixels = tonuino.getNeoPixels();
  pixels.setTimedNightLight(0);
  pixels.loop();
  EXPECT_TRUE(pixels.timedNightLightActive());
  for (auto color : Adafruit_NeoPixel::frame) EXPECT_NE(color, 0u);
  current_time += 29 * 60000UL + 30000;
  pixels.loop();
  for (unsigned int i = 0; i < 12; ++i)
    EXPECT_EQ(Adafruit_NeoPixel::frame[i] != 0, i >= 3 && i <= 8);
  current_time += 30000;
  getMp3().called_sleep = false;
  pixels.loop();
  EXPECT_FALSE(pixels.timedNightLightActive());
  for (auto color : Adafruit_NeoPixel::frame) EXPECT_EQ(color, 0u);
  EXPECT_FALSE(getMp3().called_sleep);
  EXPECT_EQ(pin_value[shutdownPin], HIGH);
  pixels.setTimedNightLight(0);
  pixels.loop();
  EXPECT_TRUE(pixels.timedNightLightActive());
  EXPECT_NE(Adafruit_NeoPixel::frame[5], 0u);
}

TEST_F(pixel_box_fixture, timed_night_survives_rollover_and_old_unlimited_night_remains_unlimited) {
  auto& pixels = tonuino.getNeoPixels();
  current_time = 0xfffffff0UL;
  const uint32_t start = current_time;
  pixels.setTimedNightLight(1);
  current_time = static_cast<uint32_t>(start + 30000UL);
  pixels.loop();
  EXPECT_TRUE(pixels.timedNightLightActive());
  EXPECT_NE(Adafruit_NeoPixel::frame[5], 0u);
  EXPECT_EQ(Adafruit_NeoPixel::frame[0], 0u);
  current_time = static_cast<uint32_t>(start + 60000UL);
  pixels.loop();
  EXPECT_FALSE(pixels.timedNightLightActive());
  for (auto color : Adafruit_NeoPixel::frame) EXPECT_EQ(color, 0u);
  pixels.setNightLight(true);
  current_time += 60 * 60000UL;
  pixels.loop();
  EXPECT_EQ(pixels.getState(), Neo_pixels::State::night_light);
  EXPECT_NE(Adafruit_NeoPixel::frame[0], 0u);
}

TEST_F(pixel_box_fixture, timed_night_and_playback_modifier_reset_are_independent) {
  ASSERT_TRUE(tonuino.specialCard({0, pmode_t::timed_night_light, 30, 0}));
  EXPECT_TRUE(tonuino.getNeoPixels().timedNightLightActive());
  ASSERT_TRUE(tonuino.specialCard({0, pmode_t::sleep_timer, 30, 0}));
  EXPECT_TRUE(tonuino.getNeoPixels().timedNightLightActive());
  EXPECT_EQ(getModifier().getActive(), pmode_t::sleep_timer);
  tonuino.resetActiveModifier();
  EXPECT_TRUE(tonuino.getNeoPixels().timedNightLightActive());
  ASSERT_TRUE(tonuino.specialCard({0, pmode_t::timed_night_light, 30, 0}));
  EXPECT_FALSE(tonuino.getNeoPixels().timedNightLightActive());
  ASSERT_TRUE(tonuino.specialCard({0, pmode_t::timed_night_light, 30, 0}));
  ASSERT_TRUE(tonuino.specialCard({0, pmode_t::pixel_lively, 0, 0}));
  EXPECT_FALSE(tonuino.getNeoPixels().timedNightLightActive());
  EXPECT_EQ(tonuino.getNeoPixels().getLightMode(), Neo_pixels::LightMode::lively);
}

TEST_F(pixel_box_fixture, admin_and_animation_preview_remain_visible_over_timed_night) {
  auto& pixels = tonuino.getNeoPixels();
  pixels.setTimedNightLight(30);
  pixels.setNormalState(Neo_pixels::State::admin);
  pixels.loop();
  EXPECT_EQ(pixels.getState(), Neo_pixels::State::admin);
  const auto admin = Adafruit_NeoPixel::frame;
  EXPECT_NE(admin[0], admin[1]);
  pixels.previewPlayAnimation(Neo_pixels::PlayAnimation::comet);
  pixels.setNormalState(Neo_pixels::State::admin);
  EXPECT_EQ(pixels.getState(), Neo_pixels::State::play_preview);
  pixels.loop();
  EXPECT_NE(Adafruit_NeoPixel::frame, admin);
  pixels.setNormalState(Neo_pixels::State::idle);
  pixels.loop();
  EXPECT_EQ(pixels.getState(), Neo_pixels::State::night_light);
  for (auto color : Adafruit_NeoPixel::frame) EXPECT_EQ(color, Adafruit_NeoPixel::frame[0]);
}

TEST_F(pixel_box_fixture, sleep_display_contracts_symmetrically_and_waits_for_the_track_at_zero) {
  auto& pixels = tonuino.getNeoPixels();
  pixels.setSleepTimer(255, false);
  pixels.loop();
  for (auto color : Adafruit_NeoPixel::frame) EXPECT_NE(color, 0u);
  pixels.setSleepTimer(128, false);
  pixels.loop();
  for (unsigned int i = 0; i < 12; ++i)
    EXPECT_EQ(Adafruit_NeoPixel::frame[i] != 0, i >= 3 && i <= 8);
  const auto shows = Adafruit_NeoPixel::showCount;
  current_time += 50;
  pixels.loop();
  EXPECT_EQ(Adafruit_NeoPixel::showCount, shows);
  pixels.setSleepTimer(0, true);
  current_time += 100;
  pixels.loop();
  for (unsigned int i = 0; i < 12; ++i)
    EXPECT_EQ(Adafruit_NeoPixel::frame[i] != 0, i == 5 || i == 6);
  pixels.clearSleepTimer();
  pixels.loop();
  EXPECT_EQ(Adafruit_NeoPixel::frame[0] & 0x00ffff, 0u);
  EXPECT_NE(Adafruit_NeoPixel::frame[0], 0u);
}

TEST_F(pixel_box_fixture, recognized_card_moves_green_outward_and_returns_to_the_normal_scene) {
  auto& pixels = tonuino.getNeoPixels();
  pixels.call_on_feedback(Neo_pixels::Feedback::recognized);
  pixels.loop();
  for (unsigned int i = 0; i < 12; ++i)
    EXPECT_EQ((Adafruit_NeoPixel::frame[i] & 0x00ffff) != 0, i == 5 || i == 6);
  current_time += 450;
  pixels.loop();
  EXPECT_NE(Adafruit_NeoPixel::frame[2] & 0x00ffff, 0u);
  EXPECT_NE(Adafruit_NeoPixel::frame[9] & 0x00ffff, 0u);
  EXPECT_EQ(Adafruit_NeoPixel::frame[0] & 0x00ffff, 0u);
  current_time += 451;
  pixels.loop();
  for (auto color : Adafruit_NeoPixel::frame) EXPECT_EQ(color & 0x00ffff, 0u);
}

TEST_F(pixel_box_fixture, empty_error_and_locked_feedback_have_distinct_pulse_counts) {
  auto& pixels = tonuino.getNeoPixels();
  auto frameAt = [&](unsigned long elapsed) {
    current_time += elapsed;
    pixels.loop();
    return Adafruit_NeoPixel::frame;
  };
  pixels.call_on_feedback(Neo_pixels::Feedback::empty_card);
  const auto emptyOn = frameAt(0);
  const auto emptyOff = frameAt(700);
  EXPECT_NE(emptyOn, emptyOff);
  EXPECT_EQ(frameAt(300), emptyOn); // Second pulse.
  pixels.call_on_feedback(Neo_pixels::Feedback::read_error);
  const auto errorOn = frameAt(0);
  const auto errorOff = frameAt(300);
  EXPECT_NE(errorOn, errorOff);
  EXPECT_EQ(frameAt(200), errorOn);
  EXPECT_EQ(frameAt(200), errorOff);
  EXPECT_EQ(frameAt(200), errorOn); // Third pulse.
  pixels.call_on_feedback(Neo_pixels::Feedback::locked);
  EXPECT_EQ(frameAt(0), errorOn);
  EXPECT_EQ(frameAt(350), errorOff);
}

TEST_F(pixel_box_fixture, game_selection_has_neutral_halves_and_result_is_only_shown_on_feedback) {
  auto& pixels = tonuino.getNeoPixels();
  pixels.setState(Neo_pixels::State::play);
  pixels.setGameSelection(Neo_pixels::GameSelection::first);
  current_time += 50;
  pixels.loop();
  for (unsigned int i = 0; i < 12; ++i) EXPECT_EQ(Adafruit_NeoPixel::frame[i] != 0, i < 6);
  pixels.setGameSelection(Neo_pixels::GameSelection::second);
  current_time += 50;
  pixels.loop();
  const auto chosen = Adafruit_NeoPixel::frame;
  EXPECT_NE(chosen[0], chosen[6]);
  for (unsigned int i = 1; i < 6; ++i) EXPECT_EQ(chosen[i], chosen[0]);
  pixels.call_on_feedback(Neo_pixels::Feedback::game_good);
  current_time += 900;
  pixels.loop();
  for (auto color : Adafruit_NeoPixel::frame) EXPECT_EQ(color, Adafruit_NeoPixel::frame[0]);
  EXPECT_GT((Adafruit_NeoPixel::frame[0] >> 8) & 255, (Adafruit_NeoPixel::frame[0] >> 16) & 255);
  pixels.call_on_feedback(Neo_pixels::Feedback::game_bad);
  pixels.loop();
  const auto bad = Adafruit_NeoPixel::frame;
  current_time += 300;
  pixels.loop();
  EXPECT_EQ(Adafruit_NeoPixel::frame[0], bad[6]);
  EXPECT_EQ(Adafruit_NeoPixel::frame[6], bad[0]);
}

TEST_F(pixel_box_fixture, battery_warning_is_one_full_strip_pulse_and_shutdown_still_wins) {
  auto& pixels = tonuino.getNeoPixels();
  pixels.setNightLight(true);
  pixels.call_on_feedback(Neo_pixels::Feedback::battery);
  current_time += 800;
  pixels.loop();
  const auto warning = Adafruit_NeoPixel::frame;
  for (auto color : warning) EXPECT_EQ(color, warning[0]);
  EXPECT_GT((warning[0] >> 16) & 255, (warning[0] >> 8) & 255);
  current_time += 801;
  pixels.loop();
  EXPECT_NE(Adafruit_NeoPixel::frame, warning);
  pixels.call_on_feedback(Neo_pixels::Feedback::battery);
  pixels.shutdown();
  pixels.call_on_feedback(Neo_pixels::Feedback::game_good);
  EXPECT_TRUE(pixels.isShuttingDown());
  for (int i = 0; i < 6; ++i) { current_time += 100; pixels.loop(); }
  for (auto color : Adafruit_NeoPixel::frame) EXPECT_EQ(color, 0u);
}

#ifdef MEMORY_GAME
TEST_F(pixel_box_fixture, memory_pair_cards_update_halves_and_confirmed_match_turns_green) {
  card_in({2, pmode_t::memory_game, 0, 0}, 20);
  execute_cycle();
  getMp3().end_track();
  execute_cycle();
  execute_cycle_for_ms(dfPlayer_timeUntilStarts + 50);
  ASSERT_TRUE(SM_tonuino::is_in_state<Memory>());
  card_out();
  card_in({0, pmode_t::memory_game, 1, 0});
  execute_cycle_for_ms(1100);
  for (unsigned int i = 0; i < 12; ++i) EXPECT_EQ(Adafruit_NeoPixel::frame[i] != 0, i < 6);
  card_out();
  card_in({0, pmode_t::memory_game, 2, 0});
  execute_cycle_for_ms(1100);
  EXPECT_NE(Adafruit_NeoPixel::frame[0], Adafruit_NeoPixel::frame[6]);
  button_for_command(command::pause, state_for_command::play);
  execute_cycle_for_ms(700);
  for (auto color : Adafruit_NeoPixel::frame) EXPECT_EQ(color, Adafruit_NeoPixel::frame[0]);
  EXPECT_GT((Adafruit_NeoPixel::frame[0] >> 8) & 255, (Adafruit_NeoPixel::frame[0] >> 16) & 255);
}
#endif

#ifdef BAT_VOLTAGE_MEASUREMENT
TEST_F(pixel_box_fixture, low_battery_warning_is_reached_without_confusing_it_with_empty_battery) {
  pin_value[voltageMeasurementPin] = 575; // About 2.91 V: low, not empty.
  execute_cycle();
  current_time += 30001;
  execute_cycle();
  EXPECT_TRUE(tonuino.getBatVoltage().isLow());
  EXPECT_FALSE(tonuino.getNeoPixels().isShuttingDown());
  current_time += 800;
  execute_cycle();
  for (auto color : Adafruit_NeoPixel::frame) EXPECT_EQ(color, Adafruit_NeoPixel::frame[0]);
  EXPECT_GT((Adafruit_NeoPixel::frame[0] >> 16) & 255, (Adafruit_NeoPixel::frame[0] >> 8) & 255);
}
#endif


TEST_F(pixel_box_fixture, timed_night_prevents_early_standby_and_restarts_idle_timeout_after_light_ends) {
  getSettings().standbyTimer=1;
  tonuino.setStandbyTimer();
  ASSERT_TRUE(tonuino.specialCard({0, pmode_t::timed_night_light, 30, 0}));
  execute_cycle();
  EXPECT_EQ(tonuino.getRemainingStandbyTimer(), 0xffffffffUL);
  current_time += 2 * 60000UL;
  execute_cycle();
  EXPECT_FALSE(tonuino.getNeoPixels().isShuttingDown());
  current_time += 28 * 60000UL;
  execute_cycle();
  EXPECT_FALSE(tonuino.getNeoPixels().timedNightLightActive());
  for (auto color : Adafruit_NeoPixel::frame) EXPECT_EQ(color, 0u);
  EXPECT_EQ(pin_value[shutdownPin], HIGH);
  execute_cycle();
  EXPECT_LE(tonuino.getRemainingStandbyTimer(), 60000UL);
  EXPECT_GT(tonuino.getRemainingStandbyTimer(), 59000UL);
  EXPECT_FALSE(tonuino.getNeoPixels().isShuttingDown());
}

TEST_F(pixel_box_fixture, malformed_lighting_card_with_music_folder_is_rejected) {
  card_in({2, pmode_t::pixel_quiet, 0, 0});
  EXPECT_TRUE(SM_tonuino::is_in_state<Idle>());
  EXPECT_EQ(tonuino.getNeoPixels().getLightMode(), Neo_pixels::LightMode::normal);
  for (auto color : Adafruit_NeoPixel::frame) EXPECT_EQ(color, Adafruit_NeoPixel::frame[0]);
  EXPECT_NE(Adafruit_NeoPixel::frame[0] & 0x00ffff, 0u);
}

TEST_F(pixel_box_fixture, lighting_card_is_read_from_rfid_without_stopping_playback) {
  goto_play({2, pmode_t::album, 0, 0});
  card_out();
  card_in({0, pmode_t::pixel_quiet, 0, 0});
  EXPECT_TRUE(SM_tonuino::is_in_state<Play>());
  EXPECT_EQ(tonuino.getNeoPixels().getLightMode(), Neo_pixels::LightMode::quiet);
  EXPECT_EQ(getModifier().getActive(), pmode_t::none);
  EXPECT_TRUE(getMp3().is_playing_adv());
  execute_cycle_for_ms(1100);
  EXPECT_EQ(Adafruit_NeoPixel::frame[0], 0x020100u);
  card_out();
  card_in({0, pmode_t::pixel_quiet, 0, 0});
  EXPECT_EQ(tonuino.getNeoPixels().getLightMode(), Neo_pixels::LightMode::normal);
  EXPECT_TRUE(SM_tonuino::is_in_state<Play>());
}


#ifdef QUIZ_GAME
TEST_F(pixel_box_fixture, quiz_does_not_hint_the_answer_and_result_feedback_matches_the_spoken_result) {
  card_in({2, pmode_t::quiz_game, 2, 0}, 30);
  execute_cycle();
  getMp3().end_track(); execute_cycle();
  execute_cycle_for_ms(dfPlayer_timeUntilStarts + 50);
  ASSERT_TRUE(SM_tonuino::is_in_state<Quiz>());
  execute_cycle_for_ms(1000);
  getMp3().end_track(); execute_cycle();
  button_for_command(command::pause, state_for_command::play);
  execute_cycle_for_ms(200);
  const auto volume=getVolume();
  button_for_command(command::volume_up, state_for_command::play);
  EXPECT_EQ(getVolume(), volume);
  EXPECT_GT(Adafruit_NeoPixel::frame[0] & 255, Adafruit_NeoPixel::frame[0] >> 16);
  EXPECT_GT(Adafruit_NeoPixel::frame[6] & 255, (Adafruit_NeoPixel::frame[6] >> 8) & 255);
  getMp3().end_track(); execute_cycle();
  button_for_command(command::pause, state_for_command::play);
  execute_cycle_for_ms(650);
  const auto answer=getMp3().df_mp3_track;
  ASSERT_TRUE(answer==501 || answer==502);
  uint32_t brightest=0;
  for (auto color : Adafruit_NeoPixel::frame) brightest=max(brightest,color);
  if (answer==501) EXPECT_GT((brightest>>8)&255,(brightest>>16)&255);
  else EXPECT_GT((brightest>>16)&255,(brightest>>8)&255);
}
#endif

#endif
