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

#endif
