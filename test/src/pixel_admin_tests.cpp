#include "tonuino_fixture.hpp"

#ifdef NeoPixels
// Cover the fork menu independently of the official 14 options.
class pixel_admin_fixture : public tonuino_fixture {};

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
