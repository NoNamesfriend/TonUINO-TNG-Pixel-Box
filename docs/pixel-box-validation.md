# Lokale Prüfung vom 13. Oktober 2026

Referenz: TonUINO-TNG `upstream/main` bei `d527ca5f3c5090a74ccdf5505202db352d7531c8`. Hardwarekonfiguration: AiO+, 12 LEDs, WS-Pin 18.

## Firmware

PlatformIO hat alle folgenden Varianten erfolgreich gebaut. Die Größen stammen aus den erzeugten ELF-Dateien (`avr-size`); Flash umfasst `.text` und `.data`, statisches RAM `.data` und `.bss`.

| Build | Flash | Statisches RAM |
| --- | ---: | ---: |
| `PixelBox_AiOPlus_5` | 43.398 / 49.152 Byte | 1.236 / 6.144 Byte |
| `PixelBox_AiOPlus_3` | 42.902 / 49.152 Byte | 1.200 / 6.144 Byte |
| `PixelBox_AiOPlus_3x3` | 43.666 / 49.152 Byte | 1.217 / 6.144 Byte |
| `ALLinONE_Plus_5f` mit offiziellem `NEO_RING` | 36.783 / 49.152 Byte | 1.237 / 6.144 Byte |
| `TonUINO_Classic_3` ohne Pixel Box | 29.396 / 30.720 Byte | 1.529 / 2.048 Byte |

Das dynamisch reservierte NeoPixel-Array und der Laufzeit-Stack sind in der statischen RAM-Zahl nicht enthalten. Die ursprünglichen Animationen bleiben in `src/neo_pixels.cpp`; `src/ring.cpp` entspricht weiterhin dem offiziellen Stand.

## Hosttests

Die Tests wurden unter Windows mit GCC 11.2.0, C++17, den vorhandenen Hardware-Mocks und GoogleTest 1.16.0 kompiliert. Jeder Test lief in einem eigenen Prozess, entsprechend `gtest_discover_tests` in der CMake-Konfiguration. CMake selbst wurde lokal nicht ausgeführt.

| Testkonfiguration | Bestandene Tests |
| --- | ---: |
| Classic, 3 Tasten | 84 |
| Classic, 5 Tasten | 84 |
| Classic, 3×3 | 92 |
| AiO+ ohne Pixel Box | 84 |
| AiO+ ohne Pixel Box, 3×3 | 92 |
| AiO | 84 |
| AiO, 3×3 | 92 |
| AiO+ mit Pixel Box | 94 |
| AiO+ mit Pixel Box, 3×3 | 102 |
| Pixel-Box-Pfade mit allen Zusatzfunktionen des Standard-Builds | 10 |
| Gesamt | 818 |

Die Zusatztests prüfen EEPROM-Übernahme, Erhalt von Lautstärke, Shortcuts und Hörbuchfortschritt, erneutes Laden nach der Migration, gelöschte/beschädigte LED-Daten, Kartenanimation mit anschließender Pause, Lautstärkeanzeige bei gleichen Grenzwerten, Nachtlichtwechsel, fortlaufenden Farbzyklus und den 32-Bit-Überlauf der Animationstimer. Die Menütests prüfen Adminoptionen 15 bis 18 samt Einleitungen sowie Nachtlichtoption 12 mit Ansage 0982. Die Feature-Konfiguration prüft diese Pixel-Box-Pfade mit den tatsächlich gesetzten Zusatzfunktionen; sie ersetzt keinen vollständigen Funktionstest von Quiz und Memory.

## Audio und Paket

`python tools/validate_pixel_box_audio.py` besteht mit 668 eindeutigen Soundnummern und 151 deutschen Katalognummern. Die drei offiziellen Stopptanz-Textfragmente zählen dabei als eine gemeinsame Audionummer. Die neuen Grundhelligkeitsansagen 0916 und 0951 lassen sich mit FFmpeg fehlerfrei dekodieren.

`build/pixelbox_sdcard.zip` enthält genau 668 MP3-Dateien unter `mp3/` und `advert/`. Dateiliste, doppelte Archiveinträge und ZIP-Prüfsummen wurden geprüft. Das Paket wurde mit `python tools/package_pixel_box.py` erzeugt.

## Noch auf der Box prüfen

Firmware und SD-Karte wurden nicht auf die reale Box übertragen. Offen sind sichtbare Animationen, die Wiedergabe der neuen Ansagen über den DFPlayer, die EEPROM-Migration auf dem vorhandenen Gerät und das Neuprogrammieren alter Nachtlicht-Karten. Der Compiler prüft die AVR-Speichergrenzen; die Hosttests prüfen die Migration mit simuliertem EEPROM.

Prüfe nach dem Update Start/Abschalten, Kartenwechsel, beide Wiedergabeanimationen, Pause, Vor/Zurück, Lautstärke, LED-Einstellungen und Neustart. Prüfe außerdem die Nachtlicht-Aktivierung, Deaktivierung und den Wechsel auf eine andere Modifikationskarte. Für neue englische/französische Pixel-Box-Ansagen wurden keine Audiodateien erzeugt.
