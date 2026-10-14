# Pixel Box aktualisieren

Dieser Fork basiert auf TonUINO-TNG `d527ca5f3c5090a74ccdf5505202db352d7531c8` vom 18. September 2026. Der Abgleich mit `upstream/main` erfolgte am 13. Oktober 2026. Die offizielle Anleitung steht in [README-upstream.md](../README-upstream.md).

## Firmware und SD-Karte aktualisieren

1. Sichere die SD-Karte. Entferne vorhandene Nachtlicht-Modifikationskarten vor dem ersten Start mit der neuen Firmware: Ihr alter Modus 9 hat inzwischen eine andere Bedeutung.
2. Wähle die Tastenvariante deiner AiO+-Box. `pio run` baut standardmäßig `PixelBox_AiOPlus_5`. Für drei Tasten verwende `pio run -e PixelBox_AiOPlus_3`, für das 3×3-Tastenbrett `pio run -e PixelBox_AiOPlus_3x3`.
3. Übertrage die Firmware mit dem üblichen Verfahren für deine AiO+-Platine. Die erzeugte HEX-Datei liegt unter `.pio/build/<Build-Variante>/firmware.hex`. Mit PlatformIO ist beispielsweise `pio run -e PixelBox_AiOPlus_5 -t upload` möglich. Die offiziellen Online-Uploader installieren die offizielle Firmware; verwende für deine Animationen die Firmware dieses Forks.
4. Erzeuge mit `python tools/package_pixel_box.py` das Paket `build/pixelbox_sdcard.zip`. Es enthält `mp3/` und `advert/` direkt für das Wurzelverzeichnis der SD-Karte. Ersetze nach der Sicherung diese beiden Systemordner vollständig. Nur neue Dateien darüberzukopieren reicht nicht: Alte Dateien mit gleicher vierstelliger Nummer würden auf der Karte bleiben. Deine Musikordner `01` bis `99` bleiben erhalten.
5. Starte die Box neu. Die bisherigen gültigen LED-Einstellungen werden beim ersten Start übernommen. Prüfe Farbe, Grundhelligkeit, Wiedergabeanimation und Nachtlichthelligkeit im Adminmenü.
6. Programmiere vorhandene Nachtlicht-Karten über „Modifikationskarte“ neu. Das Nachtlicht ist dort Option 12. Alte Nachtlicht-Karten mit Modus 9 können nicht automatisch erkannt werden: Sie enthalten dieselben Daten wie die neue offizielle Karte „Pause nach jedem Track“. Andere vorhandene TonUINO-Karten behalten ihre Bedeutung.

Ein Zurücksetzen aller Einstellungen ist für dieses Update nicht erforderlich. Halte beim Start keine Tastenkombination zum Löschen des EEPROM gedrückt.

## Deine Hardware und Animationen

Die Pixel-Box-Builds verwenden die bestätigte Konfiguration: AiO+, 12 LEDs, WS-Pin 18 und maximale NeoPixel-Helligkeit 20 von 255. Pin, LED-Anzahl und maximale Helligkeit stehen einmalig in `src/constants.hpp` als `NeoPixelPin`, `NeoPixelCount` und `NeoPixelBrightness`.

Erhalten sind Start- und Abschaltanimation, beide Wiedergabeanimationen `dotForwardPulse` und `colorCycle`, Pause, nächster/vorheriger Titel, Lautstärkefarben, Adminanzeige und gelbes Nachtlicht. Die ausgewählte Kartenrückmeldung breitet sich jetzt grün von der Mitte nach außen aus. Die weiteren ausgewählten Effekte und Lichtkarten sind in [pixel-box-light-cards.md](pixel-box-light-cards.md) beschrieben. Kurze Rückmeldungen und das Adminmenü bleiben auch bei Nachtlicht sichtbar; danach wird das Nachtlicht fortgesetzt.

Im Adminmenü unter Option 17 („Neo Pixel Play Animation“) läuft die aktuell gewählte Animation direkt auf dem Streifen. Die Tasten wechseln zwischen 1 (`dotForwardPulse`), 2 (`colorCycle`), 3 (Leuchtpunkt mit Schweif), 4 (weiche Farbüberblendung) und 5 („Lebhaft“); die Zahl wird weiterhin angesagt. Die beiden bisherigen Animationen verwenden die gespeicherte Farbe und Grundhelligkeit. Leuchtpunkt und Farbüberblendung nutzen die gespeicherte Grundfarbe, „Lebhaft“ wechselt die Farbe selbst. Bestätige mit der Pausetaste, um die Auswahl zu speichern. Beim Abbrechen bleiben die bisherige Einstellung und die gespeicherte Wiedergabeanimation erhalten. Die Vorschau verwendet denselben LED-Puffer und dieselben Animationsfunktionen wie die Wiedergabe.

Die Pixel-Box-Builds behalten außerdem die bisherigen Optionen `DONT_ACCEPT_SAME_RFID_TWICE`, `RESUME_ON_SAME_RFID`, `REPLAY_ON_PLAY_BUTTON`, `QUIZ_GAME`, `MEMORY_GAME`, `STORE_LAST_CARD` und `BAT_VOLTAGE_MEASUREMENT` bei. Sie sind in `platformio.ini` auf diese Builds begrenzt.

Die offiziellen Build-Varianten bleiben verfügbar. Beispielsweise aktiviert `ALLinONE_Plus_5f` den offiziellen `NEO_RING`. `NeoPixels` und `NEO_RING` können nicht gleichzeitig aktiviert werden; der Compiler meldet diese Konfiguration als Fehler. Das verhindert konkurrierende LED-Treiber.

Für Arduino IDE wähle in `src/constants.hpp` genau eine Platine (`ALLinONE_Plus`), aktiviere `NeoPixels` und die gewünschten Zusatzfunktionen. PlatformIO übergibt diese Auswahl bereits über die Build-Flags.

## Menü, Speicher und Soundnummern

Die offiziellen 14 Adminoptionen behalten ihre Nummern. Deine Einstellungen stehen weiterhin dahinter:

| Adminoption | Einstellung | Ansage | Einleitung |
| --- | --- | --- | --- |
| 15 | Neo Pixel Farbe | `mp3/0915…` | `mp3/0950…` |
| 16 | Neo Pixel Grundhelligkeit | `mp3/0916…` | `mp3/0951…` |
| 17 | Neo Pixel Play Animation | `mp3/0917…` | `mp3/0952…` |
| 18 | Helligkeit für Nachtlicht | `mp3/0918…` | `mp3/0953…` |

Die Nachtlicht-Modifikationskarte verwendet Modus 12 und Menüansage `mp3/0982_modifier_night_light.mp3`. Ihre Aktivierungsansage bleibt `advert/0350_night_light.mp3`. Die bisherige Datei `mp3/0326_mode_night_light.mp3` entfällt: 0326 ist inzwischen die offizielle Ansage für den Hörbuchmodus mit Titelbereich. Die offiziellen Modifikationskarten belegen 0971 bis 0981; die Ansagen für die Adminsperre sind nach 0985 bis 0989 verschoben. Die alten Dateien 0980 bis 0984 für die Adminsperre gehören deshalb nicht mehr auf die Karte.

Die deutschen Katalogeinträge wurden mit diesen Nummern abgeglichen. Die vorhandenen eigenen Sounddateien bleiben erhalten. Nur die Grundhelligkeitsansagen 0916 und 0951 wurden korrigiert: Sie hatten von „Farbe“ gesprochen. Die neuen Dateien wurden lokal mit der deutschen Windows-Stimme „Microsoft Katja“ erzeugt. Die Pixel-Box-Ansagen sind derzeit nur im deutschen SD-Paket enthalten; die offiziellen englischen und französischen Pakete enthalten diese Ergänzungen nicht.

Die neuen Modifikationskarten stehen unter Optionen 13 (Ruhelicht), 14 (Wiedergabeanimation wechseln) und 15 (zeitbegrenztes Nachtlicht, standardmäßig 30 Minuten). Ihre Menüansagen verwenden 1100 bis 1103, die Aktivierungsansagen 0351 bis 0353. Dadurch bleiben die offiziellen Adminsperr-Ansagen 0985 bis 0989 frei von Überschneidungen. Die Kartenmodi 200 bis 202 sind von den Menüpositionen getrennt. Bedienung und Kartenformat stehen in [pixel-box-light-cards.md](pixel-box-light-cards.md).

Im AVR-EEPROM bleiben TonUINO-Einstellungen bei Adresse 100 und zusätzliche Shortcuts ab 156. Die fünf LED-Datenbytes liegen separat bei 148 bis 152; 153 bis 155 enthalten Kennung `PB` und Formatversion 1. Der erste Start übernimmt plausible Werte des bisherigen AVR-Forks ab Adresse 140. Gelöschte oder ungültige Werte erhalten die bisherigen Standardwerte. Danach kennzeichnet der eigene Datenblock die migrierten Werte. Compilerprüfungen verhindern, dass zukünftige Erweiterungen diesen Speicherbereich überlappen.

## Prüfung und weitere Verbesserungen

`python tools/validate_pixel_box_audio.py` prüft doppelte vierstellige Nummern, fehlende Systemansagen, leere Dateien sowie den Abgleich zwischen Firmware und deutschem Textkatalog. `python tools/package_pixel_box.py` führt dieselbe Prüfung vor dem Packen aus. Die CI baut mit `tools/build_all_environments.py` weiterhin alle konfigurierten Firmware-Varianten.

Die vorhandenen Hosttests sind um EEPROM-Migration, beschädigte Einstellungen, Nachtlichtwechsel, Karten-/Pausenzustände, Lautstärkelimits, Animationstimer und die vier eigenen Menüoptionen erweitert. Mit CMake laufen sie über `cmake -S . -B build`, `cmake --build build` und `ctest --test-dir build --output-on-failure`. Voraussetzung ist GoogleTest. Jeder Test läuft dabei in einem eigenen Prozess.

Die lokale Prüfung ist in [pixel-box-validation.md](pixel-box-validation.md) dokumentiert. Die sichtbaren Effekte und die Ansagen müssen noch auf der realen Box geprüft werden; ein Build ersetzt diesen Hardwaretest nicht.

Die zehn Punkte aus der anschließenden Codeprüfung sowie die Start- und Abschaltanimation ohne Animations-`delay()` sind umgesetzt. Verhalten, Gründe und Speichervergleich stehen in [pixel-box-optimizations.md](pixel-box-optimizations.md). Beim Abschalten endet erst das letzte LED-Bild, bevor die bestehende Stromabschaltung beginnt.
