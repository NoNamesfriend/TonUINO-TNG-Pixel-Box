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
  const int legacy = 100 + offsetof(Settings, neoPixelHue);
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
  for (int i = 100 + offsetof(Settings, neoPixelHue); i < 156; ++i) EEPROM.write(i, 0xff);
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

class pixel_box_fixture : public tonuino_fixture {};

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
  pixels.updateFromSettings();
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
#endif
