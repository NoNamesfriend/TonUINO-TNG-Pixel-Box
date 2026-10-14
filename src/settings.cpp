#include "settings.hpp"

#include "constants.hpp"
#include "logger.hpp"
#include <stddef.h>
#include "pixel_box.hpp"

namespace {

//  ############### EEPROM ################################
//  Address       Usage
//    0- 99       Folder Settings (Hoerbuch Fortschritt)
//  100-139       AVR AdminSettings (40 Byte)
//  140-147       reserved (ESP32/padding may use part of this)
//  148-155       Pixel Box settings (5 data bytes, PB signature, version)
//  156-255       extra Shortcuts (100 Byte, max. 25 Shortcuts)

// Nano:      2048 byte
// Nano Every: 256 byte
// AiO:        512 byte (with framework-lgt8fx@1.0.6 emulated)
// AiOplus:    256 byte
// Esp32 Nano  256 byte (configured in EEPROM.begin())

constexpr uint16_t startAddressFolderSettings =   0;
constexpr uint16_t startAddressAdminSettings  = 100;
constexpr uint16_t startAddressExtraShortcuts = 156;
constexpr uint16_t endAddress                 = 256;

#ifdef NeoPixels
constexpr uint16_t startAddressPixelSettings = 148;
constexpr uint16_t startAddressLegacyPixelSettings = 140; // Original AVR fork, fixed five-byte layout.
constexpr uint16_t storedSettingsSize = offsetof(Settings, neoPixelHue);
static_assert(startAddressAdminSettings + storedSettingsSize <= startAddressPixelSettings,
              "Upstream settings overlap Pixel Box EEPROM storage");
static_assert(startAddressPixelSettings + 8 <= startAddressExtraShortcuts,
              "Pixel Box settings overlap extra shortcuts");

void readPixelSettings(Settings& settings, uint16_t address) {
  settings.neoPixelHue = EEPROM.read(address) | (static_cast<uint16_t>(EEPROM.read(address + 1)) << 8);
  settings.neoPixelBaseValue = EEPROM.read(address + 2);
  settings.neoPixelPlayAnimation = EEPROM.read(address + 3);
  settings.neoPixelNightLightValue = EEPROM.read(address + 4);
}

void resetPixelSettings(Settings& settings) {
  settings.neoPixelHue = 0;
  settings.neoPixelBaseValue = 95;
  settings.neoPixelPlayAnimation = 0;
  settings.neoPixelNightLightValue = 255;
}

bool validPixelSettings(const Settings& settings) {
  return settings.neoPixelHue <= 64000
      && settings.neoPixelPlayAnimation < static_cast<uint8_t>(PixelPlayAnimation::NUMBER_OF_ANIMATIONS);
}
#else
constexpr uint16_t storedSettingsSize = sizeof(Settings);
#endif

#ifdef BUTTONS3X3
constexpr uint16_t maxExtraShortcuts = (endAddress - startAddressExtraShortcuts) / sizeof(folderSettings);
static_assert(buttonExtSC_buttons < maxExtraShortcuts, "Too many ExtraShortCuts");
#endif
}

void Settings::init() {
#ifdef TonUINO_Esp32
  bool res = EEPROM.begin(endAddress);
  LOG(settings_log, s_info, F("EEPROM begin: "), res);
#endif
}

void Settings::commit() {
#ifdef TonUINO_Esp32
  bool res = EEPROM.commit();
  LOG(settings_log, s_info, F("EEPROM commit: "), res);
#endif
}

void Settings::clearEEPROM() {
  LOG(settings_log, s_info, F("clEEPROM"));
  for (uint16_t i = startAddressFolderSettings; i < endAddress; ++i) {
    EEPROM_update(i, 0);
  }
  commit();
}

void Settings::writeSettingsToFlash() {
  static_assert(startAddressAdminSettings + storedSettingsSize < startAddressExtraShortcuts, "Settings too big");
  LOG(settings_log, s_debug, F("writeSettingsToFlash"));
  const byte* data = reinterpret_cast<const byte*>(this);
  for (uint16_t i = 0; i < storedSettingsSize; ++i)
    EEPROM_update(startAddressAdminSettings + i, data[i]);
#ifdef NeoPixels
  EEPROM_update(startAddressPixelSettings, neoPixelHue & 0xff);
  EEPROM_update(startAddressPixelSettings + 1, neoPixelHue >> 8);
  EEPROM_update(startAddressPixelSettings + 2, neoPixelBaseValue);
  EEPROM_update(startAddressPixelSettings + 3, neoPixelPlayAnimation);
  EEPROM_update(startAddressPixelSettings + 4, neoPixelNightLightValue);
  EEPROM_update(startAddressPixelSettings + 5, 'P');
  EEPROM_update(startAddressPixelSettings + 6, 'B');
  EEPROM_update(startAddressPixelSettings + 7, 1);
#endif
  commit();
}

void Settings::resetSettings() {
  LOG(settings_log, s_debug, F("resetSettings"));
  cookie               = cardCookie;
  version              =  2;
  spkMaxVolume         = 25;
  spkMinVolume         =  5;
  spkInitVolume        = 15;
  eq                   =  1;
  dummy                =  0;
  standbyTimer         =  0;
  invertVolumeButtons  =  1;
  shortCuts[0]         =  { 0, pmode_t::none, 0, 0 };
  shortCuts[1]         =  { 0, pmode_t::none, 0, 0 };
  shortCuts[2]         =  { 0, pmode_t::none, 0, 0 };
  shortCuts[3]         =  { 0, pmode_t::none, 0, 0 };
  adminMenuLocked      =  0;
  adminMenuPin[0]      =  1;
  adminMenuPin[1]      =  1;
  adminMenuPin[2]      =  1;
  adminMenuPin[3]      =  1;
  pauseWhenCardRemoved =  0;
  hpMaxVolume          = 25;
  hpMinVolume          =  5;
  hpInitVolume         = 15;
#ifdef NeoPixels
  resetPixelSettings(*this);
#endif

  writeSettingsToFlash();
}

void Settings::loadSettingsFromFlash() {
  LOG(settings_log, s_debug, F("loadSettings"));
  byte* data = reinterpret_cast<byte*>(this);
  for (uint16_t i = 0; i < storedSettingsSize; ++i)
    data[i] = EEPROM.read(startAddressAdminSettings + i);
  if (cookie != cardCookie) {
    resetSettings();
  }
#ifdef NeoPixels
  else {
    const bool hasPixelSettings = EEPROM.read(startAddressPixelSettings + 5) == 'P'
                              && EEPROM.read(startAddressPixelSettings + 6) == 'B'
                              && EEPROM.read(startAddressPixelSettings + 7) == 1;
    if (hasPixelSettings) {
      readPixelSettings(*this, startAddressPixelSettings);
    }
    else {
#ifndef TonUINO_Esp32
      // Fixed original AVR layout: read only the five LED fields.
      // Erased or upstream-only EEPROM does not have plausible legacy values.
      readPixelSettings(*this, startAddressLegacyPixelSettings);
      if (!validPixelSettings(*this) || neoPixelBaseValue < 15 || neoPixelNightLightValue < 15)
        resetPixelSettings(*this);
#else
      resetPixelSettings(*this);
#endif
    }
    const bool invalid = !validPixelSettings(*this);
    if (invalid)
      resetPixelSettings(*this);
    if (!hasPixelSettings || invalid)
      writeSettingsToFlash();
  }
#endif

  if (pauseWhenCardRemoved == 255) {
    pauseWhenCardRemoved = 0;
    writeSettingsToFlash();
  }

  if (hpMaxVolume == 255 || hpMaxVolume == 0) {
    hpMaxVolume          = 25;
    hpMinVolume          =  5;
    hpInitVolume         = 15;
    writeSettingsToFlash();
  }

  LOG(settings_log, s_info, F("Ver:"), version);
  LOG(settings_log, s_info, F("Vol:"), spkMaxVolume, F(" "), spkMinVolume, F(" "), spkInitVolume, F(" "), hpMaxVolume, F(" "), hpMinVolume, F(" "), hpInitVolume);
  LOG(settings_log, s_info, F("EQ:" ), eq);
  LOG(settings_log, s_info, F("ST:" ), standbyTimer);
  LOG(settings_log, s_info, F("IB:" ), invertVolumeButtons);
  LOG(settings_log, s_info, F("AL:" ), adminMenuLocked);
  LOG(settings_log, s_info, F("AP:" ), adminMenuPin[0], adminMenuPin[1], adminMenuPin[2], adminMenuPin[3]);
  LOG(settings_log, s_info, F("PCR:"), pauseWhenCardRemoved);
  #ifdef NeoPixels
  LOG(settings_log, s_info, F("NP_H:"), neoPixelHue);
  LOG(settings_log, s_info, F("NP_V:"), neoPixelBaseValue);
  LOG(settings_log, s_info, F("NP_PA:"), neoPixelPlayAnimation);
  LOG(settings_log, s_info, F("NP_NLV:"), neoPixelNightLightValue);
  #endif
}

void Settings::writeFolderSettingToFlash(uint8_t folder, uint8_t track) {
  if (folder < 100)
    EEPROM_put(folder, track);
}

uint8_t Settings::readFolderSettingFromFlash(uint8_t folder) {
  uint8_t ret = 0;
  if (folder < 100)
    EEPROM_get(folder, ret);
  return ret;
}

void Settings::writeExtShortCutToFlash (uint8_t shortCut, const folderSettings& value) {
  const int address = startAddressExtraShortcuts + shortCut * sizeof(folderSettings);
  EEPROM_put(address, value);
}

void Settings::readExtShortCutFromFlash(uint8_t shortCut,       folderSettings& value) {
  const int address = startAddressExtraShortcuts + shortCut * sizeof(folderSettings);
  EEPROM_get(address, value);
}


folderSettings Settings::getShortCut(uint8_t shortCut) {
#ifdef TonUINO_Esp32
  if (shortCut == 0)
    return cardFromWeb;
#endif
  if (shortCut > 0 && shortCut <= 4)
    return shortCuts[shortCut-1];
#ifdef BUTTONS3X3
  else if (shortCut >= buttonExtSC_begin && shortCut < buttonExtSC_begin + buttonExtSC_buttons) {
    folderSettings extShortCut;
    readExtShortCutFromFlash(shortCut-buttonExtSC_begin, extShortCut);
    return extShortCut;
  }
#endif

  return { 0, pmode_t::none, 0, 0 };
}

void Settings::setShortCut(uint8_t shortCut, const folderSettings& value) {
#ifdef TonUINO_Esp32
  if (shortCut == 0)
    cardFromWeb = value;
#endif
  if (shortCut > 0 && shortCut <= 4) {
    shortCuts[shortCut-1] = value;
    // writeSettingsToFlash(); -- will be done in state machine
  }
#ifdef BUTTONS3X3
  else if (shortCut >= buttonExtSC_begin && shortCut < buttonExtSC_begin + buttonExtSC_buttons) {
    writeExtShortCutToFlash(shortCut-buttonExtSC_begin, value);
  }
#endif
}

