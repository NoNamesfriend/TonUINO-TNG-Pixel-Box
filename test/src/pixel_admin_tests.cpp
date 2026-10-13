#include "tonuino_fixture.hpp"

#ifdef NeoPixels
// Cover the fork menu independently of the official 14 options.
class pixel_admin_fixture : public tonuino_fixture {
public:
  void goto_animation_setting() {
    goto_admin();
    for (uint8_t i = 0; i < 17; ++i) {
      button_for_command(command::next, state_for_command::admin);
      execute_cycle_for_ms(time_check_play);
      getMp3().end_track();
      execute_cycle();
    }
    button_for_command(command::select, state_for_command::admin);
    ASSERT_TRUE(SM_tonuino::is_in_state<Admin_SimpleSetting>());
    ASSERT_EQ(Admin_SimpleSetting::type, Admin_SimpleSetting::neoPixelPlayAnimation);
    execute_cycle_for_ms(time_check_play);
    getMp3().end_track();
    execute_cycle();
  }
};

TEST_F(pixel_admin_fixture, animation_preview_switches_without_saving_and_cancel_restores_selection) {
  getSettings().neoPixelPlayAnimation = 0;
  getSettings().writeSettingsToFlash();
  goto_animation_setting();
  auto& pixels = tonuino.getNeoPixels();
  EXPECT_EQ(pixels.getState(), Neo_pixels::State::play_preview);
  const auto dotFrame = Adafruit_NeoPixel::frame;
  uint8_t savedEeprom[EEPROM.max_len];
  memcpy(savedEeprom, EEPROM.eeprom_mem, sizeof(savedEeprom));

  button_for_command(command::next, state_for_command::admin);
  execute_cycle_for_ms(150);
  EXPECT_NE(Adafruit_NeoPixel::frame, dotFrame);
  EXPECT_NE(Adafruit_NeoPixel::frame[0], Adafruit_NeoPixel::frame[1]);
  EXPECT_EQ(getSettings().neoPixelPlayAnimation, 0);
  EXPECT_EQ(memcmp(savedEeprom, EEPROM.eeprom_mem, sizeof(savedEeprom)), 0);

  const auto cycleFrame = Adafruit_NeoPixel::frame;
  execute_cycle_for_ms(150);
  EXPECT_NE(Adafruit_NeoPixel::frame, cycleFrame); // Menu polling must not restart the preview.
  button_for_command(command::adm_end, state_for_command::admin);
  ASSERT_TRUE(SM_tonuino::is_in_state<Idle>());
  EXPECT_EQ(pixels.getState(), Neo_pixels::State::idle);
  EXPECT_EQ(memcmp(savedEeprom, EEPROM.eeprom_mem, sizeof(savedEeprom)), 0);

  pixels.setState(Neo_pixels::State::play);
  execute_cycle_for_ms(250);
  // Persisted animation 0 uses the configured red hue; preview 1 used several hues.
  for (const auto color : Adafruit_NeoPixel::frame)
    EXPECT_EQ(color & 0x00ffff, 0u);
}

TEST_F(pixel_admin_fixture, animation_preview_is_saved_only_on_confirmation) {
  goto_animation_setting();
  button_for_command(command::next, state_for_command::admin);
  execute_cycle_for_ms(150);
  ASSERT_EQ(getSettings().neoPixelPlayAnimation, 0);
  getMp3().end_track();
  execute_cycle();
  button_for_command(command::select, state_for_command::admin);
  ASSERT_TRUE(SM_tonuino::is_in_state<Admin_Entry>());
  EXPECT_EQ(tonuino.getNeoPixels().getState(), Neo_pixels::State::admin);
  EXPECT_EQ(getSettings().neoPixelPlayAnimation, 1);
  getSettings().neoPixelPlayAnimation = 0;
  getSettings().loadSettingsFromFlash();
  EXPECT_EQ(getSettings().neoPixelPlayAnimation, 1);
}

TEST_F(pixel_admin_fixture, pixel_options_keep_audio_numbers_and_setting_types) {
  const Admin_SimpleSetting::Type types[] = {
    Admin_SimpleSetting::neoPixelHue, Admin_SimpleSetting::neoPixelBaseValue,
    Admin_SimpleSetting::neoPixelPlayAnimation, Admin_SimpleSetting::neoPixelNightLightValue
  };
  for (uint8_t option = 15; option <= 18; ++option) {
    goto_admin();
    for (uint8_t i = 1; i <= option; ++i) {
      button_for_command(command::next, state_for_command::admin);
      execute_cycle_for_ms(time_check_play);
      EXPECT_EQ(getMp3().df_mp3_track, 900 + i);
      getMp3().end_track();
      execute_cycle();
    }
    button_for_command(command::select, state_for_command::admin);
    ASSERT_TRUE(SM_tonuino::is_in_state<Admin_SimpleSetting>());
    EXPECT_EQ(Admin_SimpleSetting::type, types[option - 15]);
    execute_cycle_for_ms(time_check_play);
    EXPECT_EQ(getMp3().df_mp3_track, 950 + option - 15);
    getMp3().end_track();
    execute_cycle();
    button_for_command(command::adm_end, state_for_command::admin);
    goto_idle();
  }
}

TEST_F(pixel_admin_fixture, night_light_modifier_uses_option_12_and_track_982) {
  goto_admin();
  for (uint8_t i = 0; i < 6; ++i) {
    button_for_command(command::next, state_for_command::admin);
    execute_cycle_for_ms(time_check_play);
    getMp3().end_track();
    execute_cycle();
  }
  button_for_command(command::select, state_for_command::admin);
  ASSERT_TRUE(SM_tonuino::is_in_state<Admin_ModCard>());
  execute_cycle_for_ms(time_check_play);
  getMp3().end_track();
  execute_cycle();
  for (uint8_t i = 1; i <= 12; ++i) {
    button_for_command(command::next, state_for_command::admin);
    execute_cycle_for_ms(time_check_play);
    EXPECT_EQ(getMp3().df_mp3_track, 970 + i);
    getMp3().end_track();
    execute_cycle();
  }
  button_for_command(command::select, state_for_command::admin);
  EXPECT_EQ(SM_tonuino::folder.folder, 0);
  EXPECT_EQ(SM_tonuino::folder.mode, pmode_t::night_light);
  EXPECT_EQ(static_cast<uint8_t>(pmode_t::pause_aft_tr), 9);
}
#endif
