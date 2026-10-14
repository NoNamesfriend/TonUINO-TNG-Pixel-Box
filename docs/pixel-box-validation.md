# Lokale Prüfung vom 14. Oktober 2026

Referenz: TonUINO-TNG `upstream/main` bei `d527ca5f3c5090a74ccdf5505202db352d7531c8`. Hardwarekonfiguration: AiO+, 12 LEDs, WS-Pin 18.

## Firmware

PlatformIO hat alle folgenden Varianten erfolgreich gebaut. Die Größen stammen aus den erzeugten ELF-Dateien (`avr-size`); Flash umfasst `.text` und `.data`, statisches RAM `.data` und `.bss`.

| Build | Flash | Flash frei | Statisches RAM |
| --- | ---: | ---: | ---: |
| `PixelBox_AiOPlus_5` | 47.530 / 49.152 Byte | 1.622 Byte | 1.266 / 6.144 Byte |
| `PixelBox_AiOPlus_3` | 47.036 / 49.152 Byte | 2.116 Byte | 1.230 / 6.144 Byte |
| `PixelBox_AiOPlus_3x3` | 47.798 / 49.152 Byte | 1.354 Byte | 1.247 / 6.144 Byte |
| `ALLinONE_Plus_5f` mit offiziellem `NEO_RING` | 36.823 / 49.152 Byte | 12.329 Byte | 1.237 / 6.144 Byte |
| `TonUINO_Classic_3` ohne Pixel Box | 29.434 / 30.720 Byte | 1.286 Byte | 1.529 / 2.048 Byte |

Das dynamisch reservierte NeoPixel-Array und der Laufzeit-Stack sind in der statischen RAM-Zahl nicht enthalten. Die ursprünglichen Animationen bleiben in `src/neo_pixels.cpp`; `src/ring.cpp` entspricht weiterhin dem offiziellen Stand.

Die beiden Farbkarten benötigen gegenüber `7ae4182` 432 zusätzliche Byte Flash im Standardbuild; das statische RAM bleibt gleich. Sie verwenden die vorhandenen zwei Farbbytes im EEPROM und den bestehenden LED-Puffer. Die zuvor ergänzte Animations-Wechselkarte benötigt gegenüber `d42e096` 54 zusätzliche Byte Flash.

Die neuen ausgewählten Effekte und Lichtkarten benötigen gegenüber dem Optimierungsstand `e8a1dfb` zusätzlich 5.348 Byte Flash und 24 Byte statisches RAM im Standardbuild. Die damalige Einsparung von 1.272 Byte ist in [pixel-box-optimizations.md](pixel-box-optimizations.md) dokumentiert. Alle drei Pixel-Box-Varianten bleiben innerhalb des vom AiO+-Build vorgegebenen Flashlimits von 49.152 Byte. Die knappste Variante ist 3×3 mit 1.354 Byte Reserve; weitere Funktionen müssen erneut gemessen werden.

Die 12 RGB-LEDs benötigen im vorhandenen Adafruit-Puffer 36 Byte Heap zuzüglich Verwaltungsdaten. Die neuen Effekte legen keinen weiteren Puffer an, reservieren keinen zusätzlichen Heap und verwenden keine neuen EEPROM-Felder. Eine 32-Byte-Pulstabelle liegt im Flash. Der Laufzeit-Stack ist weiterhin auf der Hardware zu beurteilen.

Der ELF-Abgleich mit `avr-nm -C` zeigt im Pixel-Box-Build keine `Ring::`- oder `OneRing::`-Symbole und 18 `Neo_pixels::`-Symbole. Der offizielle `ALLinONE_Plus_5f`-Build enthält zehn Ring-Symbole. Der Quellabgleich von `ring.cpp` und `ring.hpp` mit dem übernommenen Upstream-Commit ist unverändert. `NEO_RING` schließt den offiziellen Ring im Pixel-Box-Build über den Präprozessor aus. Die gemeinsame Adafruit-Bibliothek wird für den jeweils gewählten Treiber verwendet.

Die neuen Pixel-Effekte verändern den offiziellen AiO+-Vergleichsbuild nicht. Die gemeinsam verwendete Timer-Korrektur senkt den Classic-Vergleichsbuild gegenüber `e8a1dfb` um zehn Byte Flash; das statische RAM bleibt in beiden Vergleichsbuilds unverändert.

## Hosttests

Die Tests wurden unter Windows mit GCC 11.2.0, C++17, den vorhandenen Hardware-Mocks und GoogleTest 1.16.0 kompiliert. Jeder Test lief in einem eigenen Prozess, entsprechend `gtest_discover_tests` in der CMake-Konfiguration. CMake selbst wurde lokal nicht ausgeführt.

Die zwölf Hostkonfigurationen umfassen die sieben offiziellen Varianten und fünf Pixel-Box-Konfigurationen. Nach Ergänzung der Farbkarten wurden alle fünf Pixel-Box-Konfigurationen erneut geprüft; die sieben offiziellen Ergebnisse stammen aus dem vorherigen geprüften Stand `d42e096`. Die Pixel-Box-Änderung ist dort per Präprozessor ausgeschlossen. Insgesamt bestehen 1.138 Tests. Die fünf Firmware-Builds wurden mit dem abschließenden Code gebaut. Die Übersetzung für AVR prüft zusätzlich per `static_assert`, dass Farbauswahl 33 den Wert 32.000 und Auswahl 65 den Wert 64.000 ergibt.

| Testkonfiguration | Bestandene Tests |
| --- | ---: |
| Classic, 3 Tasten | 87 |
| Classic, 5 Tasten | 87 |
| Classic, 3×3 | 95 |
| AiO+ ohne Pixel Box | 87 |
| AiO+ ohne Pixel Box, 3×3 | 95 |
| AiO | 87 |
| AiO, 3×3 | 95 |
| AiO+ mit Pixel Box | 148 |
| AiO+ mit Pixel Box, 3×3 | 156 |
| Pixel-Box-Pfade mit allen Zusatzfunktionen, 5 Tasten | 67 |
| Pixel-Box-Pfade mit allen Zusatzfunktionen, 3 Tasten | 67 |
| Pixel-Box-Pfade mit allen Zusatzfunktionen, 3×3 | 67 |
| Gesamt | 1.138 |

Die Zusatztests prüfen EEPROM-Übernahme, Erhalt von Lautstärke, Shortcuts und Hörbuchfortschritt, erneutes Laden nach der Migration, gelöschte/beschädigte LED-Daten, Kartenanimation mit anschließender Pause, Lautstärkeanzeige bei gleichen Grenzwerten, Nachtlichtwechsel, fortlaufenden Farbzyklus und den 32-Bit-Überlauf der Animationstimer. Die Menütests prüfen Adminoptionen 15 bis 18 samt Einleitungen sowie Nachtlichtoption 12 mit Ansage 0982. Die Feature-Konfiguration prüft diese Pixel-Box-Pfade mit den tatsächlich gesetzten Zusatzfunktionen. Zusätzliche Tests lesen simulierte Quiz- und Memory-Karten ein, prüfen den Übergang zur Wiedergabeanimation und das Lautstärkefeedback. Bei Quiz wird zusätzlich geprüft, dass eine Antworttaste keine Lautstärkeanzeige auslöst. Das ersetzt keinen vollständigen Spieltest auf der Box.

Die Vorschautests prüfen den sichtbaren Animationswechsel, fortlaufende Frames ohne Neustart bei jedem Menüzyklus, unveränderte EEPROM-Daten vor Bestätigung, Wiederherstellung der bisherigen Animation beim Abbrechen und Speicherung der Auswahl bei Bestätigung.

Die neuen Tests prüfen die fünf stabilen Animations-IDs, Schweif und Umlaufdauer des Leuchtpunkts, gleichmäßige Farbüberblendung, zwei Lichtspitzen der Play-Animation „Lebhaft“, gedämpftes Ruhelicht und die Unabhängigkeit von Audio-Modifikationen. Neue Lichtkarten werden über das Adminmenü ausgewählt, mit den erwarteten Ansagen angeboten, auf eine simulierte RFID-Karte geschrieben und wieder ausgelesen. Für das Nachtlicht sind voreingestellte 30 Minuten und geänderte 15 Minuten abgedeckt. Die Animationskarte wird über alle fünf Animationen bis zum Umlauf geprüft, einschließlich erneutem Laden der Auswahl. Dabei ändert sich nur das bisherige Auswahlbyte im EEPROM. Ein RFID-Test prüft genau einen Wechsel je Auflegen bei weiterlaufender Wiedergabe; ein weiterer prüft den Wechsel aus Ruhelicht und im Idle-Zustand. Der Test für Rotation und Speicherung scheiterte mit dem vorherigen Profilverhalten und besteht mit der Wechselkarte.

Elf zusätzliche Tests prüfen die acht Palettenfarben einschließlich Umlauf und Übergang von einer frei gewählten Farbe, feste Farbauswahl bis 65, erneutes Laden und unveränderte andere EEPROM-Bereiche. Wiederholtes Setzen derselben Farbe schreibt kein EEPROM-Byte. RFID-Prüfungen belegen einen Wechsel je Auflegen, die eigene Farbvorschau und weiterlaufende Wiedergabe. Ungültige Farbwerte und Musikordner mit Farbmodi ändern keine Einstellung. Ruhelicht sowie unbegrenztes und zeitbegrenztes Nachtlicht bleiben erhalten; die Nachtlicht-Frist wird nicht neu gestartet. Die Farbvorschau dauert 1,2 Sekunden, wird gedämpft und startet bei einer weiteren Farbkarte neu. Abschalten hat Vorrang. Grundfarbe und grünes Erkennungssignal werden an Karten-, Titel- und Pausenbildern gemeinsam geprüft. Über das Adminmenü werden beide Kartenvarianten geschrieben und ausgelesen; Vorschau, Bestätigung bei Auswahl 65 und Abbrechen verändern die Grundfarbe der Box nicht.

Weitere Tests prüfen alle kurzen Rückmeldungen, neutrale Spielhälften, bestätigte Memory-Paare und Quiz-Antworten, den niedrigen Akku ohne Verwechslung mit leerem Akku sowie die Priorität der Abschaltung. Das Nachtlicht wird über 30 Minuten bis zum mittleren letzten LED-Paar und zum dunklen Endzustand geprüft, auch über den Uhrüberlauf. Es hält eine kürzere Standby-Frist an; nach seinem Ende startet die konfigurierte Standby-Zeit neu. Das unbegrenzte Nachtlicht bleibt unbegrenzt. Adminanzeige und Animationsvorschau bleiben während eines Nachtlichts bedienbar. Die drei gemeinsamen Timer-Tests prüfen den Überlauf von `millis()`, die Grenze des vorzeichenbehafteten Wertebereichs und den Vertrag für einen inaktiven Timer.

Neue Regressionstests prüfen außerdem die feste Legacy-Adresse 140 statt eines Host-`offsetof`, Farbvorschau und Speicherung bis Auswahl 65, exakt 72 Farbzyklusbilder, den 50-ms-Takt und die bisherigen Schweifhelligkeiten für Vor/Zurück, ausbleibende Wiederholungsübertragungen bei statischen Anzeigen sowie null EEPROM-Schreibzugriffe für unveränderte Einstellungen. Der LED-Mock bildet Skalierung und verlustbehaftetes Rücklesen des Adafruit-Treibers nach. Die Pulsanimation wird bei Grundhelligkeiten 0, 1, 15, 95 und 255 geprüft.

Startanimation und Abschaltanforderung verändern die simulierte Uhr nicht. Die Abschalttests prüfen die symmetrischen Bilder, Nachtlichtvorrang, wiederholte Anforderungen, den 32-Bit-Überlauf sowie die Freigabe des Shutdown-Pins erst nach 600 ms. Standby und leerer Akku verwenden denselben geprüften Ablauf. Die bestehenden SleepTimer-Tests berücksichtigen, dass eine begonnene Abschaltung keine neuen Wiedergabebefehle mehr annimmt; nach der Abschaltung simulieren sie einen Neustart. Im Feature-Mock bekommt der Batterieeingang nach `setup()` einen normalen analogen Spannungswert, weil die vereinfachte Pinbelegung im Hosttest mit dem USB-Pin kollidiert.

## Audio und Paket

`python tools/validate_pixel_box_audio.py` besteht mit 679 eindeutigen Soundnummern und 162 deutschen Katalognummern. Die drei offiziellen Stopptanz-Textfragmente zählen dabei als eine gemeinsame Audionummer. Die sieben Ansagen 1100–1103 und 0351–0353 wurden im vorherigen Stand geprüft. Die vier zusätzlichen Farbansagen 1104–1106 und 0354 lassen sich ebenfalls mit FFmpeg fehlerfrei dekodieren. Die Grundhelligkeitsansagen 0916 und 0951 wurden bereits im vorherigen Optimierungsstand geprüft.

`build/pixelbox_sdcard.zip` enthält genau 679 MP3-Dateien unter `mp3/` und `advert/` und ist 8.369.249 Byte groß. Die Dateiliste entspricht vollständig dem deutschen SD-Verzeichnis; doppelte Archiveinträge und ZIP-Prüfsummen wurden geprüft. Das Paket wurde mit `python tools/package_pixel_box.py` erzeugt.

## Noch auf der Box prüfen

Am 14. Oktober 2026 wurde die Firmware auf der realen AiO+-Box mit fünf Tasten geprüft. Zunächst lief das offizielle Ziel `ALLinONE_Plus_5`, dessen ELF keine Pixel-Box-Symbole enthielt; im über COM3 gelesenen Startprotokoll fehlten entsprechend die vier `NP_*`-Einstellungszeilen. Nach dem vom Nutzer ausgeführten Upload mit dem ausdrücklich gewählten Ziel `PixelBox_AiOPlus_5` bestätigte er, dass die LEDs leuchten. Dafür waren weder ein SD-Update noch ein EEPROM-Reset erforderlich. Die Auswahl des richtigen Uploadziels ist in [pixel-box-update.md](pixel-box-update.md) beschrieben.

Die SD-Karte enthält auf der Box weiterhin die alten Systemansagen. Die Hardwarebestätigung betrifft die leuchtenden LEDs nach dem Firmwarewechsel; einzelne Animationsabläufe, die neuen Ansagen und Kartenfunktionen sind noch offen. Ebenso offen sind die Prüfung der übernommenen Einstellungen auf der realen Box und das Neuprogrammieren alter Nachtlicht-Karten. Der Compiler prüft die AVR-Speichergrenzen; die Hosttests prüfen die Migration mit simuliertem EEPROM.

Prüfe nach dem Update Start/Abschalten, Kartenwechsel, alle fünf Wiedergabeanimationen, Pause, Vor/Zurück, Lautstärke, LED-Einstellungen und Neustart. Prüfe außerdem Ruhelicht, Animations-Wechselkarte, Farb-Durchschaltkarte, feste Farbkarten mit Vorschau beim Erstellen, Nachtlicht-Aktivierung/Deaktivierung, den Ablauf des zeitbegrenzten Nachtlichts und das Zusammenspiel mit Einschlaftimer, Spielen und Standby. Die bestehenden DFPlayer-Wartezeiten können die LED-Bildfolge während Ansagen unterbrechen; die reale Wirkung ist deshalb auf der Box zu prüfen. Für neue englische/französische Pixel-Box-Ansagen wurden keine Audiodateien erzeugt.
