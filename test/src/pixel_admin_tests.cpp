#include "tonuino_fixture.hpp"

#ifdef NeoPixels
// Cover the fork menu independently of the official 14 options.
class pixel_admin_fixture : public tonuino_fixture {
public:
  void goto_pixel_setting(uint8_t option) {
    goto_admin();
    for (uint8_t i = 0; i < option; ++i) {
      button_for_command(command::next, state_for_command::admin);
      execute_cycle_for_ms(time_check_play);
      getMp3().end_track();
      execute_cycle();
    }
    button_for_command(command::select, state_for_command::admin);
    ASSERT_TRUE(SM_tonuino::is_in_state<Admin_SimpleSetting>());
    execute_cycle_for_ms(time_check_play);
    getMp3().end_track();
    execute_cycle();
  }
  void goto_animation_setting() {
    goto_pixel_setting(17);
    ASSERT_EQ(Admin_SimpleSetting::type, Admin_SimpleSetting::neoPixelPlayAnimation);
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
  pixels.loop();
  for (int i = 0; i < 3; ++i) { current_time += 100; pixels.loop(); }
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

TEST_F(pixel_admin_fixture, hue_preview_and_save_reach_options_33_and_65) {
  goto_pixel_setting(15);
  ASSERT_TRUE(SM_tonuino::is_in_state<Admin_SimpleSetting>());
  for (uint8_t option = 2; option <= 65; ++option) {
    button_for_command(command::next, state_for_command::admin);
    getMp3().end_track();
    execute_cycle();
    if (option == 33 || option == 65) {
      const auto actual = Adafruit_NeoPixel::frame;
      Adafruit_NeoPixel expected(1, 18, NEO_GRB + NEO_KHZ800);
      expected.setBrightness(NeoPixelBrightness);
      expected.setPixelColor(0, expected.ColorHSV((static_cast<uint32_t>(option) - 1) * 1000, 255, 255));
      expected.show();
      for (auto color : actual) EXPECT_EQ(color, Adafruit_NeoPixel::frame[0]);
      EXPECT_EQ(getSettings().neoPixelHue, 0);
    }
  }
  button_for_command(command::select, state_for_command::admin);
  EXPECT_EQ(getSettings().neoPixelHue, 64000);
  getSettings().neoPixelHue = 0;
  getSettings().loadSettingsFromFlash();
  EXPECT_EQ(getSettings().neoPixelHue, 64000);
}

TEST_F(pixel_admin_fixture, unchanged_hue_preview_does_not_send_frames_every_menu_cycle) {
  goto_pixel_setting(15);
  const auto shows = Adafruit_NeoPixel::showCount;
  execute_cycle_for_ms(1000);
  EXPECT_EQ(Adafruit_NeoPixel::showCount, shows);
}

class pixel_new_admin_fixture : public pixel_admin_fixture {
public:
  void write_and_decode(const folderSettings& expected) {
    if (!SM_writeCard::is_in_state<WriteCard>()) execute_cycle();
    ASSERT_TRUE(SM_writeCard::is_in_state<WriteCard>());
    execute_cycle();
    execute_cycle_for_ms(time_check_play);
    getMp3().end_track(); execute_cycle();
    card_in({1, pmode_t::album, 0, 0});
    execute_cycle();
    EXPECT_EQ(card_decode(), expected);
  }
  void goto_light_card_option(uint8_t option) {
    goto_admin();
    for (uint8_t i=0; i<6; ++i) {
      button_for_command(command::next, state_for_command::admin);
      execute_cycle_for_ms(time_check_play);
      getMp3().end_track(); execute_cycle();
    }
    button_for_command(command::select, state_for_command::admin);
    ASSERT_TRUE(SM_tonuino::is_in_state<Admin_ModCard>());
    execute_cycle_for_ms(time_check_play);
    getMp3().end_track(); execute_cycle();
    for (uint8_t i=1; i<=option; ++i) {
      button_for_command(command::next, state_for_command::admin);
      execute_cycle_for_ms(time_check_play);
      EXPECT_EQ(getMp3().df_mp3_track, pixelModifierSound(i));
      getMp3().end_track(); execute_cycle();
    }
    button_for_command(command::select, state_for_command::admin);
  }
};

TEST_F(pixel_new_admin_fixture, quiet_card_uses_option_13_prompt_1100_and_rfid_mode_200) {
  goto_light_card_option(13);
  EXPECT_EQ(SM_tonuino::folder.folder, 0);
  EXPECT_EQ(SM_tonuino::folder.mode, pmode_t::pixel_quiet);
  EXPECT_EQ(static_cast<uint8_t>(SM_tonuino::folder.mode), 200);
  EXPECT_EQ(SM_tonuino::folder.special, 0);
  write_and_decode({0, pmode_t::pixel_quiet, 0, 0});
}

TEST_F(pixel_new_admin_fixture, lively_card_uses_option_14_prompt_1101_and_rfid_mode_201) {
  goto_light_card_option(14);
  EXPECT_EQ(SM_tonuino::folder.folder, 0);
  EXPECT_EQ(SM_tonuino::folder.mode, pmode_t::pixel_lively);
  EXPECT_EQ(static_cast<uint8_t>(SM_tonuino::folder.mode), 201);
  write_and_decode({0, pmode_t::pixel_lively, 0, 0});
}

TEST_F(pixel_new_admin_fixture, timed_night_card_defaults_to_30_minutes_and_uses_mode_202) {
  goto_light_card_option(15);
  EXPECT_EQ(SM_tonuino::folder.mode, pmode_t::timed_night_light);
  execute_cycle_for_ms(time_check_play);
  EXPECT_EQ(getMp3().df_mp3_track, 1103);
  getMp3().end_track(); execute_cycle();
  button_for_command(command::select, state_for_command::admin);
  EXPECT_EQ(SM_tonuino::folder.special, 30);
  EXPECT_EQ(static_cast<uint8_t>(SM_tonuino::folder.mode), 202);
  execute_cycle();
  EXPECT_EQ(SM_writeCard::folder, (folderSettings{0, pmode_t::timed_night_light, 30, 0}));
  write_and_decode({0, pmode_t::timed_night_light, 30, 0});
}

TEST_F(pixel_new_admin_fixture, timed_night_duration_can_be_changed_to_15_minutes) {
  goto_light_card_option(15);
  execute_cycle_for_ms(time_check_play);
  getMp3().end_track(); execute_cycle();
  button_for_command(command::previous, state_for_command::admin);
  execute_cycle_for_ms(time_check_play);
  EXPECT_EQ(getMp3().df_mp3_track, 962);
  getMp3().end_track(); execute_cycle();
  button_for_command(command::select, state_for_command::admin);
  EXPECT_EQ(SM_tonuino::folder.special, 15);
  execute_cycle();
  EXPECT_EQ(SM_writeCard::folder, (folderSettings{0, pmode_t::timed_night_light, 15, 0}));
  write_and_decode({0, pmode_t::timed_night_light, 15, 0});
}

TEST_F(pixel_admin_fixture, both_new_effects_are_previewed_and_saved_only_after_confirmation) {
  for (uint8_t effect=2; effect<=3; ++effect) {
    getSettings().neoPixelPlayAnimation=0;
    getSettings().writeSettingsToFlash();
    goto_animation_setting();
    for (uint8_t i=0; i<effect; ++i) {
      button_for_command(command::next, state_for_command::admin);
      getMp3().end_track(); execute_cycle();
    }
    EXPECT_EQ(getSettings().neoPixelPlayAnimation, 0);
    EXPECT_EQ(EEPROM.read(151), 0);
    EXPECT_EQ(tonuino.getNeoPixels().getState(), Neo_pixels::State::play_preview);
    execute_cycle_for_ms(200);
    const auto first=Adafruit_NeoPixel::frame;
    execute_cycle_for_ms(1000);
    EXPECT_NE(Adafruit_NeoPixel::frame, first);
    getMp3().end_track(); execute_cycle();
    button_for_command(command::select, state_for_command::admin);
    EXPECT_EQ(getSettings().neoPixelPlayAnimation, effect);
    EXPECT_EQ(EEPROM.read(151), effect);
    goto_idle();
  }
}
#endif
