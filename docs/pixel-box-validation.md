# Lokale Prüfung vom 13. Oktober 2026

Referenz: TonUINO-TNG `upstream/main` bei `d527ca5f3c5090a74ccdf5505202db352d7531c8`. Hardwarekonfiguration: AiO+, 12 LEDs, WS-Pin 18.

## Firmware

PlatformIO hat alle folgenden Varianten erfolgreich gebaut. Die Größen stammen aus den erzeugten ELF-Dateien (`avr-size`); Flash umfasst `.text` und `.data`, statisches RAM `.data` und `.bss`.

| Build | Flash | Statisches RAM |
| --- | ---: | ---: |
| `PixelBox_AiOPlus_5` | 43.454 / 49.152 Byte | 1.236 / 6.144 Byte |
| `PixelBox_AiOPlus_3` | 42.958 / 49.152 Byte | 1.200 / 6.144 Byte |
| `PixelBox_AiOPlus_3x3` | 43.722 / 49.152 Byte | 1.217 / 6.144 Byte |
| `ALLinONE_Plus_5f` mit offiziellem `NEO_RING` | 36.783 / 49.152 Byte | 1.237 / 6.144 Byte |
| `TonUINO_Classic_3` ohne Pixel Box | 29.396 / 30.720 Byte | 1.529 / 2.048 Byte |

Das dynamisch reservierte NeoPixel-Array und der Laufzeit-Stack sind in der statischen RAM-Zahl nicht enthalten. Die ursprünglichen Animationen bleiben in `src/neo_pixels.cpp`; `src/ring.cpp` entspricht weiterhin dem offiziellen Stand.

Die ergänzte Animationsvorschau benötigt in allen drei Pixel-Box-Builds 56 Byte zusätzliches Flash und erhöht das statische RAM nicht. Im 5-Tasten-Build bleiben 5.698 Byte Flash frei. Die 12 RGB-LEDs benötigen im vorhandenen Adafruit-Puffer 36 Byte Heap zuzüglich Verwaltungsdaten. Die Vorschau legt keinen weiteren Puffer an und verwendet keine neuen EEPROM-Felder. Der Laufzeit-Stack ist weiterhin auf der Hardware zu beurteilen.

Der ELF-Abgleich mit `avr-nm -C` zeigt im Pixel-Box-Build keine `Ring::`- oder `OneRing::`-Symbole und sechs `Neo_pixels::`-Symbole. Der offizielle `ALLinONE_Plus_5f`-Build enthält zehn Ring-Symbole. Die Quelldateien für den offiziellen Ring bleiben erhalten; `NEO_RING` schließt sie im Pixel-Box-Build über den Präprozessor aus. Die gemeinsame Adafruit-Bibliothek wird für den jeweils gewählten Treiber verwendet.

## Hosttests

Die Tests wurden unter Windows mit GCC 11.2.0, C++17, den vorhandenen Hardware-Mocks und GoogleTest 1.16.0 kompiliert. Jeder Test lief in einem eigenen Prozess, entsprechend `gtest_discover_tests` in der CMake-Konfiguration. CMake selbst wurde lokal nicht ausgeführt.

Für die Animationsvorschau wurden die drei betroffenen Pixel-Box-Konfigurationen erneut geprüft: insgesamt 212 erfolgreiche Tests. Die sieben offiziellen Hostkonfigurationen in der Tabelle wurden bei der vorherigen Upstream-Zusammenführung geprüft; die fünf Firmware-Builds oben wurden nach Ergänzung der Vorschau erneut gebaut.

| Testkonfiguration | Bestandene Tests |
| --- | ---: |
| Classic, 3 Tasten | 84 |
| Classic, 5 Tasten | 84 |
| Classic, 3×3 | 92 |
| AiO+ ohne Pixel Box | 84 |
| AiO+ ohne Pixel Box, 3×3 | 92 |
| AiO | 84 |
| AiO, 3×3 | 92 |
| AiO+ mit Pixel Box | 96 |
| AiO+ mit Pixel Box, 3×3 | 104 |
| Pixel-Box-Pfade mit allen Zusatzfunktionen des Standard-Builds | 12 |
| Gesamt | 824 |

Die Zusatztests prüfen EEPROM-Übernahme, Erhalt von Lautstärke, Shortcuts und Hörbuchfortschritt, erneutes Laden nach der Migration, gelöschte/beschädigte LED-Daten, Kartenanimation mit anschließender Pause, Lautstärkeanzeige bei gleichen Grenzwerten, Nachtlichtwechsel, fortlaufenden Farbzyklus und den 32-Bit-Überlauf der Animationstimer. Die Menütests prüfen Adminoptionen 15 bis 18 samt Einleitungen sowie Nachtlichtoption 12 mit Ansage 0982. Die Feature-Konfiguration prüft diese Pixel-Box-Pfade mit den tatsächlich gesetzten Zusatzfunktionen; sie ersetzt keinen vollständigen Funktionstest von Quiz und Memory.

Die Vorschautests prüfen den sichtbaren Animationswechsel, fortlaufende Frames ohne Neustart bei jedem Menüzyklus, unveränderte EEPROM-Daten vor Bestätigung, Wiederherstellung der bisherigen Animation beim Abbrechen und Speicherung der Auswahl bei Bestätigung.

## Audio und Paket

`python tools/validate_pixel_box_audio.py` besteht mit 668 eindeutigen Soundnummern und 151 deutschen Katalognummern. Die drei offiziellen Stopptanz-Textfragmente zählen dabei als eine gemeinsame Audionummer. Die neuen Grundhelligkeitsansagen 0916 und 0951 lassen sich mit FFmpeg fehlerfrei dekodieren.

`build/pixelbox_sdcard.zip` enthält genau 668 MP3-Dateien unter `mp3/` und `advert/`. Dateiliste, doppelte Archiveinträge und ZIP-Prüfsummen wurden geprüft. Das Paket wurde mit `python tools/package_pixel_box.py` erzeugt.

## Noch auf der Box prüfen

Firmware und SD-Karte wurden nicht auf die reale Box übertragen. Offen sind sichtbare Animationen, die Wiedergabe der neuen Ansagen über den DFPlayer, die EEPROM-Migration auf dem vorhandenen Gerät und das Neuprogrammieren alter Nachtlicht-Karten. Der Compiler prüft die AVR-Speichergrenzen; die Hosttests prüfen die Migration mit simuliertem EEPROM.

Prüfe nach dem Update Start/Abschalten, Kartenwechsel, beide Wiedergabeanimationen, Pause, Vor/Zurück, Lautstärke, LED-Einstellungen und Neustart. Prüfe außerdem die Nachtlicht-Aktivierung, Deaktivierung und den Wechsel auf eine andere Modifikationskarte. Für neue englische/französische Pixel-Box-Ansagen wurden keine Audiodateien erzeugt.
