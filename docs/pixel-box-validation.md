# Lokale Prüfung vom 14. Oktober 2026

Referenz: TonUINO-TNG `upstream/main` bei `d527ca5f3c5090a74ccdf5505202db352d7531c8`. Hardwarekonfiguration: AiO+, 12 LEDs, WS-Pin 18.

## Firmware

PlatformIO hat alle folgenden Varianten erfolgreich gebaut. Die Größen stammen aus den erzeugten ELF-Dateien (`avr-size`); Flash umfasst `.text` und `.data`, statisches RAM `.data` und `.bss`.

| Build | Flash | Statisches RAM |
| --- | ---: | ---: |
| `PixelBox_AiOPlus_5` | 42.182 / 49.152 Byte | 1.242 / 6.144 Byte |
| `PixelBox_AiOPlus_3` | 41.688 / 49.152 Byte | 1.206 / 6.144 Byte |
| `PixelBox_AiOPlus_3x3` | 42.450 / 49.152 Byte | 1.223 / 6.144 Byte |
| `ALLinONE_Plus_5f` mit offiziellem `NEO_RING` | 36.823 / 49.152 Byte | 1.237 / 6.144 Byte |
| `TonUINO_Classic_3` ohne Pixel Box | 29.444 / 30.720 Byte | 1.529 / 2.048 Byte |

Das dynamisch reservierte NeoPixel-Array und der Laufzeit-Stack sind in der statischen RAM-Zahl nicht enthalten. Die ursprünglichen Animationen bleiben in `src/neo_pixels.cpp`; `src/ring.cpp` entspricht weiterhin dem offiziellen Stand.

Die zehn Optimierungen einschließlich der nicht blockierenden Start- und Abschaltanimationen sparen gegenüber dem Stand mit Animationsvorschau 1.272 Byte Flash im Standardbuild. Es bleiben 6.970 Byte Flash frei. Das statische RAM steigt um 6 Byte für die gespeicherte Animationsauswahl und die Erkennung unveränderter Bilder. Die 12 RGB-LEDs benötigen im vorhandenen Adafruit-Puffer 36 Byte Heap zuzüglich Verwaltungsdaten. Die Änderungen legen keinen weiteren Puffer an und verwenden keine neuen EEPROM-Felder. Der Laufzeit-Stack ist weiterhin auf der Hardware zu beurteilen.

Der ELF-Abgleich mit `avr-nm -C` zeigt im Pixel-Box-Build keine `Ring::`- oder `OneRing::`-Symbole und zehn `Neo_pixels::`-Symbole. Der offizielle `ALLinONE_Plus_5f`-Build enthält zehn Ring-Symbole. Der Quellabgleich von `ring.cpp` und `ring.hpp` mit dem übernommenen Upstream-Commit ist unverändert. `NEO_RING` schließt den offiziellen Ring im Pixel-Box-Build über den Präprozessor aus. Die gemeinsame Adafruit-Bibliothek wird für den jeweils gewählten Treiber verwendet.

Die EEPROM-Optimierung benötigt in den offiziellen Vergleichsbuilds 40 Byte zusätzliches Flash für AiO+ und 48 Byte für Classic; das statische RAM bleibt dort unverändert. Die Details aller zehn Änderungen stehen in [pixel-box-optimizations.md](pixel-box-optimizations.md).

## Hosttests

Die Tests wurden unter Windows mit GCC 11.2.0, C++17, den vorhandenen Hardware-Mocks und GoogleTest 1.16.0 kompiliert. Jeder Test lief in einem eigenen Prozess, entsprechend `gtest_discover_tests` in der CMake-Konfiguration. CMake selbst wurde lokal nicht ausgeführt.

Alle zehn Hostkonfigurationen wurden nach den Codeoptimierungen erneut geprüft. Insgesamt bestehen 866 Tests. Die fünf Firmware-Builds wurden mit dem abschließenden Code erneut gebaut. Die Übersetzung für AVR prüft zusätzlich per `static_assert`, dass Farbauswahl 33 den Wert 32.000 und Auswahl 65 den Wert 64.000 ergibt.

| Testkonfiguration | Bestandene Tests |
| --- | ---: |
| Classic, 3 Tasten | 84 |
| Classic, 5 Tasten | 84 |
| Classic, 3×3 | 92 |
| AiO+ ohne Pixel Box | 84 |
| AiO+ ohne Pixel Box, 3×3 | 92 |
| AiO | 84 |
| AiO, 3×3 | 92 |
| AiO+ mit Pixel Box | 109 |
| AiO+ mit Pixel Box, 3×3 | 117 |
| Pixel-Box-Pfade mit allen Zusatzfunktionen des Standard-Builds | 28 |
| Gesamt | 866 |

Die Zusatztests prüfen EEPROM-Übernahme, Erhalt von Lautstärke, Shortcuts und Hörbuchfortschritt, erneutes Laden nach der Migration, gelöschte/beschädigte LED-Daten, Kartenanimation mit anschließender Pause, Lautstärkeanzeige bei gleichen Grenzwerten, Nachtlichtwechsel, fortlaufenden Farbzyklus und den 32-Bit-Überlauf der Animationstimer. Die Menütests prüfen Adminoptionen 15 bis 18 samt Einleitungen sowie Nachtlichtoption 12 mit Ansage 0982. Die Feature-Konfiguration prüft diese Pixel-Box-Pfade mit den tatsächlich gesetzten Zusatzfunktionen. Zusätzliche Tests lesen simulierte Quiz- und Memory-Karten ein, prüfen den Übergang zur Wiedergabeanimation und das Lautstärkefeedback. Bei Quiz wird zusätzlich geprüft, dass eine Antworttaste keine Lautstärkeanzeige auslöst. Das ersetzt keinen vollständigen Spieltest auf der Box.

Die Vorschautests prüfen den sichtbaren Animationswechsel, fortlaufende Frames ohne Neustart bei jedem Menüzyklus, unveränderte EEPROM-Daten vor Bestätigung, Wiederherstellung der bisherigen Animation beim Abbrechen und Speicherung der Auswahl bei Bestätigung.

Neue Regressionstests prüfen außerdem die feste Legacy-Adresse 140 statt eines Host-`offsetof`, Farbvorschau und Speicherung bis Auswahl 65, exakt 72 Farbzyklusbilder, den 50-ms-Takt und die bisherigen Schweifhelligkeiten für Vor/Zurück, ausbleibende Wiederholungsübertragungen bei statischen Anzeigen sowie null EEPROM-Schreibzugriffe für unveränderte Einstellungen. Der LED-Mock bildet Skalierung und verlustbehaftetes Rücklesen des Adafruit-Treibers nach. Die Pulsanimation wird bei Grundhelligkeiten 0, 1, 15, 95 und 255 geprüft.

Startanimation und Abschaltanforderung verändern die simulierte Uhr nicht. Die Abschalttests prüfen die symmetrischen Bilder, Nachtlichtvorrang, wiederholte Anforderungen, den 32-Bit-Überlauf sowie die Freigabe des Shutdown-Pins erst nach 600 ms. Standby und leerer Akku verwenden denselben geprüften Ablauf. Die bestehenden SleepTimer-Tests berücksichtigen, dass eine begonnene Abschaltung keine neuen Wiedergabebefehle mehr annimmt; nach der Abschaltung simulieren sie einen Neustart. Im Feature-Mock bekommt der Batterieeingang nach `setup()` einen normalen analogen Spannungswert, weil die vereinfachte Pinbelegung im Hosttest mit dem USB-Pin kollidiert.

## Audio und Paket

`python tools/validate_pixel_box_audio.py` besteht mit 668 eindeutigen Soundnummern und 151 deutschen Katalognummern. Die drei offiziellen Stopptanz-Textfragmente zählen dabei als eine gemeinsame Audionummer. Die neuen Grundhelligkeitsansagen 0916 und 0951 lassen sich mit FFmpeg fehlerfrei dekodieren.

`build/pixelbox_sdcard.zip` enthält genau 668 MP3-Dateien unter `mp3/` und `advert/`. Dateiliste, doppelte Archiveinträge und ZIP-Prüfsummen wurden geprüft. Das Paket wurde mit `python tools/package_pixel_box.py` erzeugt.

## Noch auf der Box prüfen

Firmware und SD-Karte wurden nicht auf die reale Box übertragen. Offen sind sichtbare Animationen, die Wiedergabe der neuen Ansagen über den DFPlayer, die EEPROM-Migration auf dem vorhandenen Gerät und das Neuprogrammieren alter Nachtlicht-Karten. Der Compiler prüft die AVR-Speichergrenzen; die Hosttests prüfen die Migration mit simuliertem EEPROM.

Prüfe nach dem Update Start/Abschalten, Kartenwechsel, beide Wiedergabeanimationen, Pause, Vor/Zurück, Lautstärke, LED-Einstellungen und Neustart. Prüfe außerdem die Nachtlicht-Aktivierung, Deaktivierung und den Wechsel auf eine andere Modifikationskarte. Für neue englische/französische Pixel-Box-Ansagen wurden keine Audiodateien erzeugt.
