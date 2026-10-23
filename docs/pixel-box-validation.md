# Lokale Validierung

## Gemeinsames Nachtlicht vom 23. Oktober 2026

Die bisherigen drei Nachtlicht-Menüpunkte sind zu Option 12 zusammengefasst. Die folgenden Optionen sind 13 (Wiedergabeanimation wechseln), 14 (Grundfarben durchschalten) und 15 (feste Grundfarbe). Neue Nachtlichtkarten verwenden weiterhin Modus 202 mit 5, 15, 30, 60 Minuten oder 255 für unbegrenzt. Die bisherigen Modi 12 und 200 werden als unbegrenztes Nachtlicht behandelt. Alle drei Formate verwenden dieselbe Farbe, Nachtlichthelligkeit, Aktivierungsansage und Standby-Regel; sie belegen keinen Audio-Modifikationsplatz.

Vor der Umsetzung scheiterten drei neue Tests an den bisherigen Unterschieden: Die alte Nachtlichtkarte ersetzte den Einschlaftimer, Ruhelicht hatte eine andere Farbe/Helligkeit und hielt die Standby-Abschaltung nicht zurück, und das Kartenmenü bot weiterhin drei Einträge. Nach der Umsetzung bestehen alle 409 erneut ausgeführten Hosttests, jeweils in einem eigenen Prozess:

| Konfiguration | Bestandene Tests |
| --- | ---: |
| Pixel-Box mit den Firmware-Zusatzfunktionen | 77 |
| AiO+ mit Pixel Box | 158 |
| AiO+ ohne Pixel Box | 87 |
| Classic ohne Pixel Box | 87 |

Geprüft sind insbesondere die Erkennung der alten Karten, gemeinsame gelbe Ausgabe bei gespeicherter Nachtlichthelligkeit und blauer Grundfarbe, unbegrenzte Laufzeit über sechs simulierte Stunden, Erhalt eines Einschlaftimers, unverändertes EEPROM, gemeinsames Ausschalten über verschiedene alte Kartenformate und das Warten auf das Lichtende bei fälligem Standby. Menütests prüfen alle 15 Ansagen unabhängig von der Zuordnungsfunktion, die letzte Menügrenze sowie Schreiben und Auslesen der Nachtlicht-, Animations- und Farbkarten. Die bestehenden Prüfungen für Ausblendung, Uhrüberlauf, Wiedergabe und Abschalttaste bestehen weiter.

Alle drei Firmwarevarianten wurden gebaut:

| Build | Flash | Statisches RAM |
| --- | ---: | ---: |
| `PixelBox_AiOPlus_5` | 47.378 / 49.152 Byte | 1.264 / 6.144 Byte |
| `PixelBox_AiOPlus_3` | 46.884 / 49.152 Byte | 1.228 / 6.144 Byte |
| `PixelBox_AiOPlus_3x3` | 47.646 / 49.152 Byte | 1.245 / 6.144 Byte |

Die Vereinheitlichung spart gegenüber dem vorherigen Stand jeweils 356 Byte Flash und drei Byte statisches RAM. Die Menüansage 0982 wurde mit „Microsoft Katja“ neu erzeugt und mit FFmpeg fehlerfrei dekodiert. Der Audioprüfer bestätigt 680 eindeutige Soundnummern und 163 deutsche Katalognummern. Das vollständige SD-Paket wurde für alle 680 Dateien bytegenau mit den Quellen verglichen. Das aktualisierte Teilpaket `build/nightlight-prompts.zip` enthält die aktiven Nachtlichtansagen 0982, 1103 und 1107; alle drei Einträge wurden ebenfalls bytegenau geprüft.

Der Nutzer hat das Anschlussproblem der LEDs behoben und wieder funktionierende LEDs bestätigt. Die hier vereinheitlichte Firmware und die neue Menüansage wurden nicht auf die Box übertragen. Die neuen Menüabläufe und Kartenfunktionen sind auf der Hardware noch zu prüfen. Die folgenden Abschnitte dokumentieren frühere Prüfstände.

## Diagnose der Abschalttaste vom 22. Oktober 2026

Der Tastentest `button_shutdown_observes_release_instead_of_waiting_ten_seconds` reproduzierte nach dem Abschaltbefehl eine Wartezeit von exakt 10.000 ms trotz physisch losgelassener Taste. Die Warteschleife las nur den gespeicherten, noch gedrückten Tastenzustand. Sie aktualisiert den Zustand jetzt im Abstand von 100 ms. Der neue Test prüft den Einstieg in die Abschaltanimation in weniger als einer Sekunde und anschließend die Freigabe des Abschaltpins.

Nach dieser Korrektur bestehen 75 Feature-Tests, 156 AiO+-Pixel-Box-Tests, 87 AiO+-Tests ohne Pixel Box und 87 Classic-Tests, insgesamt 405 Tests. Die drei Pixel-Box-Builds bestehen mit 47.734, 47.240 und 48.002 Byte Flash für fünf Tasten, drei Tasten und 3×3. Statisches RAM bleibt bei 1.267, 1.231 und 1.248 Byte. Die Korrektur benötigt gegenüber dem unmittelbar vorherigen Stand 26 Byte Flash zusätzlich.

Die angeschlossene AiO+-Box auf COM3 wurde ausschließlich ausgelesen. Alle 47.708 Nutzbytes der installierten Firmware stimmen mit dem unmittelbar vorherigen Pixel-Box-Build ohne Tastenkorrektur überein. Das EEPROM enthält eine gültige Pixel-Box-Kennung, Grundfarbe 23.000, Grundhelligkeit 255, Animation 4 und Nachtlichthelligkeit 255. Flash und EEPROM sind im ignorierten Verzeichnis `build/box-readback-20261022-221005/` gesichert. Das Startprotokoll bestätigte diese Einstellungen sowie einen reagierenden DFPlayer mit 1.326 Titeln. Die Ursache der gemeldeten dunklen LEDs ist damit noch nicht geklärt; die Tastenkorrektur wurde zu diesem Diagnosezeitpunkt noch nicht auf die Box übertragen.

## Nachtlichtkorrektur vom 22. Oktober 2026

Das Nachtlicht verwendet beim Einschalten, während der Laufzeit und in der letzten Minute denselben gesättigten Gelbton. Die gespeicherte Nachtlichthelligkeit bleibt maßgeblich; die Grundfarbe hat keinen Einfluss. Die Dauerauswahl für Kartenmodus 202 bietet nach 60 Minuten „Unbegrenzt“ mit Kartenwert 255. Der bisherige Wert 0 und andere ungültige Werte bleiben beim Standard von 30 Minuten. Die bestehende Nachtlichtkarte mit Modus 12 bleibt erhalten.

Die drei neuen Regressionstests scheiterten vor der Korrektur am Farbwechsel, der fehlenden unbegrenzten Laufzeit und der bei 60 Minuten endenden Menüauswahl. Nach der Korrektur bestehen 74 Feature-Tests, 155 AiO+-Pixel-Box-Tests und 87 AiO+-Tests ohne Pixel Box, insgesamt 316 Tests. Jeder Test lief in einem eigenen Prozess. Geprüft wurden außerdem die gespeicherte Kartenkennung einschließlich Schreiben/Auslesen, die Ansage „Unbegrenzt“, unverändertes Gelb bei drei Nachtlichthelligkeiten und blauer Grundfarbe sowie das Abschalten durch erneutes Auflegen nach sechs simulierten Stunden.

Die Standby-Steuerung merkt sich nun eine während des Nachtlichts abgelaufene Leerlauf-/Pausenfrist. Die Box schaltet sich nach dem Ende des Lichts ohne neue Wartezeit ab. Eine noch nicht abgelaufene Frist behält ihre ursprüngliche Restzeit. Wiedergabestart und Fortsetzen löschen die Vormerkung. Vier gezielte Standby-Prüfungen scheiterten vor dieser Anpassung und bestehen danach: Warten bis zum Nachtlichtende, Erhalt einer längeren Standby-Frist, dauerhafte Vormerkung trotz großer Uhrsprünge bei unbegrenztem Licht und Löschung durch Wiedergabestart mit einer neuen Frist beim nächsten Pausieren.

Alle drei Pixel-Box-Firmwarevarianten wurden erfolgreich gebaut. Gegenüber dem unten dokumentierten Stand benötigen sie jeweils 162 Byte zusätzliches Flash und ein zusätzliches Byte statisches RAM. Davon entfallen 28 Byte Flash und das eine RAM-Byte auf die vorgemerkte Standby-Abschaltung.

| Build | Flash | Statisches RAM |
| --- | ---: | ---: |
| `PixelBox_AiOPlus_5` | 47.708 / 49.152 Byte | 1.267 / 6.144 Byte |
| `PixelBox_AiOPlus_3` | 47.214 / 49.152 Byte | 1.231 / 6.144 Byte |
| `PixelBox_AiOPlus_3x3` | 47.976 / 49.152 Byte | 1.248 / 6.144 Byte |

Die ersetzten Ansagen 1102 und 1103 sowie die neue Ansage 1107 wurden mit „Microsoft Katja“ erzeugt und mit FFmpeg fehlerfrei dekodiert. Der Audioprüfer bestätigt 680 eindeutige Soundnummern und 163 deutsche Katalognummern. Das erzeugte SD-ZIP wurde für alle 680 Dateien bytegenau mit den Quelldateien verglichen. Firmware und SD-Dateien wurden noch nicht auf die Box übertragen; die sichtbare Wirkung auf der Hardware ist noch zu prüfen.

Die folgenden Abschnitte dokumentieren den bisherigen Prüfstand vom 14. Oktober 2026.

Referenz: TonUINO-TNG `upstream/main` bei `d527ca5f3c5090a74ccdf5505202db352d7531c8`. Hardwarekonfiguration: AiO+, 12 LEDs, WS-Pin 18.

## Firmware

PlatformIO hat alle folgenden Varianten erfolgreich gebaut. Die Größen stammen aus den erzeugten ELF-Dateien (`avr-size`); Flash umfasst `.text` und `.data`, statisches RAM `.data` und `.bss`.

| Build | Flash | Flash frei | Statisches RAM |
| --- | ---: | ---: | ---: |
| `PixelBox_AiOPlus_5` | 47.546 / 49.152 Byte | 1.606 Byte | 1.266 / 6.144 Byte |
| `PixelBox_AiOPlus_3` | 47.052 / 49.152 Byte | 2.100 Byte | 1.230 / 6.144 Byte |
| `PixelBox_AiOPlus_3x3` | 47.814 / 49.152 Byte | 1.338 Byte | 1.247 / 6.144 Byte |
| `ALLinONE_Plus_5f` mit offiziellem `NEO_RING` | 36.823 / 49.152 Byte | 12.329 Byte | 1.237 / 6.144 Byte |
| `TonUINO_Classic_3` ohne Pixel Box | 29.434 / 30.720 Byte | 1.286 Byte | 1.529 / 2.048 Byte |

Das dynamisch reservierte NeoPixel-Array und der Laufzeit-Stack sind in der statischen RAM-Zahl nicht enthalten. Die ursprünglichen Animationen bleiben in `src/neo_pixels.cpp`; `src/ring.cpp` entspricht weiterhin dem offiziellen Stand.

Die Korrektur der ersten Play-Animation bei maximaler Grundhelligkeit benötigt 16 Byte zusätzliches Flash gegenüber `762ce93`; das statische RAM bleibt gleich.

Die beiden Farbkarten benötigen gegenüber `7ae4182` 432 zusätzliche Byte Flash im Standardbuild; das statische RAM bleibt gleich. Sie verwenden die vorhandenen zwei Farbbytes im EEPROM und den bestehenden LED-Puffer. Die zuvor ergänzte Animations-Wechselkarte benötigt gegenüber `d42e096` 54 zusätzliche Byte Flash.

Die neuen ausgewählten Effekte und Lichtkarten benötigen gegenüber dem Optimierungsstand `e8a1dfb` zusätzlich 5.364 Byte Flash und 24 Byte statisches RAM im Standardbuild. Die damalige Einsparung von 1.272 Byte ist in [pixel-box-optimizations.md](pixel-box-optimizations.md) dokumentiert. Alle drei Pixel-Box-Varianten bleiben innerhalb des vom AiO+-Build vorgegebenen Flashlimits von 49.152 Byte. Die knappste Variante ist 3×3 mit 1.338 Byte Reserve; weitere Funktionen müssen erneut gemessen werden.

Die 12 RGB-LEDs benötigen im vorhandenen Adafruit-Puffer 36 Byte Heap zuzüglich Verwaltungsdaten. Die neuen Effekte legen keinen weiteren Puffer an, reservieren keinen zusätzlichen Heap und verwenden keine neuen EEPROM-Felder. Eine 32-Byte-Pulstabelle liegt im Flash. Der Laufzeit-Stack ist weiterhin auf der Hardware zu beurteilen.

Der ELF-Abgleich mit `avr-nm -C` zeigt im Pixel-Box-Build keine `Ring::`- oder `OneRing::`-Symbole und 18 `Neo_pixels::`-Symbole. Der offizielle `ALLinONE_Plus_5f`-Build enthält zehn Ring-Symbole. Der Quellabgleich von `ring.cpp` und `ring.hpp` mit dem übernommenen Upstream-Commit ist unverändert. `NEO_RING` schließt den offiziellen Ring im Pixel-Box-Build über den Präprozessor aus. Die gemeinsame Adafruit-Bibliothek wird für den jeweils gewählten Treiber verwendet.

Die neuen Pixel-Effekte verändern den offiziellen AiO+-Vergleichsbuild nicht. Die gemeinsam verwendete Timer-Korrektur senkt den Classic-Vergleichsbuild gegenüber `e8a1dfb` um zehn Byte Flash; das statische RAM bleibt in beiden Vergleichsbuilds unverändert.

## Hosttests

Die Tests wurden unter Windows mit GCC 11.2.0, C++17, den vorhandenen Hardware-Mocks und GoogleTest 1.16.0 kompiliert. Jeder Test lief in einem eigenen Prozess, entsprechend `gtest_discover_tests` in der CMake-Konfiguration. CMake selbst wurde lokal nicht ausgeführt.

Die zwölf Hostkonfigurationen umfassen die sieben offiziellen Varianten und fünf Pixel-Box-Konfigurationen. Nach der Korrektur der Pulsanimation wurden alle fünf Pixel-Box-Konfigurationen erneut geprüft; die sieben offiziellen Ergebnisse stammen aus dem vorherigen geprüften Stand `d42e096`. Die Pixel-Box-Änderung ist dort per Präprozessor ausgeschlossen. Insgesamt bestehen 1.143 Tests. Die fünf Firmware-Builds wurden mit dem abschließenden Code gebaut. Die Übersetzung für AVR prüft zusätzlich per `static_assert`, dass Farbauswahl 33 den Wert 32.000 und Auswahl 65 den Wert 64.000 ergibt.

| Testkonfiguration | Bestandene Tests |
| --- | ---: |
| Classic, 3 Tasten | 87 |
| Classic, 5 Tasten | 87 |
| Classic, 3×3 | 95 |
| AiO+ ohne Pixel Box | 87 |
| AiO+ ohne Pixel Box, 3×3 | 95 |
| AiO | 87 |
| AiO, 3×3 | 95 |
| AiO+ mit Pixel Box | 149 |
| AiO+ mit Pixel Box, 3×3 | 157 |
| Pixel-Box-Pfade mit allen Zusatzfunktionen, 5 Tasten | 68 |
| Pixel-Box-Pfade mit allen Zusatzfunktionen, 3 Tasten | 68 |
| Pixel-Box-Pfade mit allen Zusatzfunktionen, 3×3 | 68 |
| Gesamt | 1.143 |

Die Zusatztests prüfen EEPROM-Übernahme, Erhalt von Lautstärke, Shortcuts und Hörbuchfortschritt, erneutes Laden nach der Migration, gelöschte/beschädigte LED-Daten, Kartenanimation mit anschließender Pause, Lautstärkeanzeige bei gleichen Grenzwerten, Nachtlichtwechsel, fortlaufenden Farbzyklus und den 32-Bit-Überlauf der Animationstimer. Die Menütests prüfen Adminoptionen 15 bis 18 samt Einleitungen sowie Nachtlichtoption 12 mit Ansage 0982. Die Feature-Konfiguration prüft diese Pixel-Box-Pfade mit den tatsächlich gesetzten Zusatzfunktionen. Zusätzliche Tests lesen simulierte Quiz- und Memory-Karten ein, prüfen den Übergang zur Wiedergabeanimation und das Lautstärkefeedback. Bei Quiz wird zusätzlich geprüft, dass eine Antworttaste keine Lautstärkeanzeige auslöst. Das ersetzt keinen vollständigen Spieltest auf der Box.

Die Vorschautests prüfen den sichtbaren Animationswechsel, fortlaufende Frames ohne Neustart bei jedem Menüzyklus, unveränderte EEPROM-Daten vor Bestätigung, Wiederherstellung der bisherigen Animation beim Abbrechen und Speicherung der Auswahl bei Bestätigung.

Die neuen Tests prüfen die fünf stabilen Animations-IDs, Schweif und Umlaufdauer des Leuchtpunkts, gleichmäßige Farbüberblendung, zwei Lichtspitzen der Play-Animation „Lebhaft“, gedämpftes Ruhelicht und die Unabhängigkeit von Audio-Modifikationen. Neue Lichtkarten werden über das Adminmenü ausgewählt, mit den erwarteten Ansagen angeboten, auf eine simulierte RFID-Karte geschrieben und wieder ausgelesen. Für das Nachtlicht sind voreingestellte 30 Minuten und geänderte 15 Minuten abgedeckt. Die Animationskarte wird über alle fünf Animationen bis zum Umlauf geprüft, einschließlich erneutem Laden der Auswahl. Dabei ändert sich nur das bisherige Auswahlbyte im EEPROM. Ein RFID-Test prüft genau einen Wechsel je Auflegen bei weiterlaufender Wiedergabe; ein weiterer prüft den Wechsel aus Ruhelicht und im Idle-Zustand. Der Test für Rotation und Speicherung scheiterte mit dem vorherigen Profilverhalten und besteht mit der Wechselkarte.

Elf zusätzliche Tests prüfen die acht Palettenfarben einschließlich Umlauf und Übergang von einer frei gewählten Farbe, feste Farbauswahl bis 65, erneutes Laden und unveränderte andere EEPROM-Bereiche. Wiederholtes Setzen derselben Farbe schreibt kein EEPROM-Byte. RFID-Prüfungen belegen einen Wechsel je Auflegen, die eigene Farbvorschau und weiterlaufende Wiedergabe. Ungültige Farbwerte und Musikordner mit Farbmodi ändern keine Einstellung. Ruhelicht sowie unbegrenztes und zeitbegrenztes Nachtlicht bleiben erhalten; die Nachtlicht-Frist wird nicht neu gestartet. Die Farbvorschau dauert 1,2 Sekunden, wird gedämpft und startet bei einer weiteren Farbkarte neu. Abschalten hat Vorrang. Grundfarbe und grünes Erkennungssignal werden an Karten-, Titel- und Pausenbildern gemeinsam geprüft. Über das Adminmenü werden beide Kartenvarianten geschrieben und ausgelesen; Vorschau, Bestätigung bei Auswahl 65 und Abbrechen verändern die Grundfarbe der Box nicht.

Weitere Tests prüfen alle kurzen Rückmeldungen, neutrale Spielhälften, bestätigte Memory-Paare und Quiz-Antworten, den niedrigen Akku ohne Verwechslung mit leerem Akku sowie die Priorität der Abschaltung. Das Nachtlicht wird über 30 Minuten bis zum mittleren letzten LED-Paar und zum dunklen Endzustand geprüft, auch über den Uhrüberlauf. Es hält eine kürzere Standby-Frist an; nach seinem Ende startet die konfigurierte Standby-Zeit neu. Das unbegrenzte Nachtlicht bleibt unbegrenzt. Adminanzeige und Animationsvorschau bleiben während eines Nachtlichts bedienbar. Die drei gemeinsamen Timer-Tests prüfen den Überlauf von `millis()`, die Grenze des vorzeichenbehafteten Wertebereichs und den Vertrag für einen inaktiven Timer.

Neue Regressionstests prüfen außerdem die feste Legacy-Adresse 140 statt eines Host-`offsetof`, Farbvorschau und Speicherung bis Auswahl 65, exakt 72 Farbzyklusbilder, den 50-ms-Takt und die bisherigen Schweifhelligkeiten für Vor/Zurück, ausbleibende Wiederholungsübertragungen bei statischen Anzeigen sowie null EEPROM-Schreibzugriffe für unveränderte Einstellungen. Der LED-Mock bildet Skalierung und verlustbehaftetes Rücklesen des Adafruit-Treibers nach. Die Pulsanimation wird bei Grundhelligkeiten 0, 1, 15, 95, 191, 223, 239 und 255 auf auftretende Lichtpunkte geprüft. Dabei wird ein Zeitabschnitt beobachtet, da einzelne Bilder am unteren Ende eines Pulses gleichmäßig sein dürfen.

Startanimation und Abschaltanforderung verändern die simulierte Uhr nicht. Die Abschalttests prüfen die symmetrischen Bilder, Nachtlichtvorrang, wiederholte Anforderungen, den 32-Bit-Überlauf sowie die Freigabe des Shutdown-Pins erst nach 600 ms. Standby und leerer Akku verwenden denselben geprüften Ablauf. Die bestehenden SleepTimer-Tests berücksichtigen, dass eine begonnene Abschaltung keine neuen Wiedergabebefehle mehr annimmt; nach der Abschaltung simulieren sie einen Neustart. Im Feature-Mock bekommt der Batterieeingang nach `setup()` einen normalen analogen Spannungswert, weil die vereinfachte Pinbelegung im Hosttest mit dem USB-Pin kollidiert.

## Regression bei maximaler Grundhelligkeit

Der Nutzer beobachtete auf der Box eine statische Farbe bei der ersten Play-Animation in der Vorschau und bestätigte Grundhelligkeitsstufe 16. Ein neuer Test öffnet die tatsächliche Adminvorschau mit dem gespeicherten und geladenen Wert 255, lässt die Farbvorbereitung vier Sekunden auslaufen und beobachtet anschließend zwölf Sekunden. Vor der Korrektur scheiterten sowohl die Prüfung auf veränderte Bilder als auch auf räumlich unterschiedliche Lichtpunkte. Die Helligkeitsanhebung wurde bereits vom Grundwert 255 begrenzt.

`dotForwardPulse()` verwendet jetzt `min(Grundhelligkeit, 95)` als internes Grundniveau und weiterhin Lichtspitzen bis 255. Der Nutzer bestätigte mit dem Zwischenstand `4bc025d`, dass die Vorschau pulsiert, empfand den Unterschied bei voller Grundhelligkeit aber als zu schwach. Das Grundniveau 191 ergab im skalierten LED-Puffer nur etwa 15 gegenüber 20 bei den Lichtspitzen. Das jetzige Grundniveau 95 ergibt etwa 7 gegenüber 20. Der Regressionstest verlangt deshalb zusätzlich, dass die Lichtspitzen in wenigstens einem Pulsbild mindestens den doppelten RGB-Helligkeitswert des Hintergrunds erreichen. Diese Prüfung scheitert beim Zwischenstand und besteht mit der stärkeren Abstufung. Einstellungen und EEPROM-Daten bleiben unverändert. Die erweiterte Wiedergabeprüfung fordert Lichtpunkte auch bei 255; der zuvor dafür ausgenommene Grenzfall ist damit abgedeckt. Vorschau und Wiedergabe nutzen weiterhin denselben Ablauf. Am 14. Oktober 2026 bestätigte der Nutzer nach dem Upload von `68c84f6` mit `PixelBox_AiOPlus_5`, dass der Kontrast der ersten Pulsanimation in der Vorschau bei voller Grundhelligkeit auf der realen Box passt. Die Wiedergabe nutzt denselben lokal geprüften Ablauf; die Hardwarebestätigung bezieht sich auf die Vorschau. Für diese Korrektur werden keine neuen SD-Dateien benötigt.

## Audio und Paket

`python tools/validate_pixel_box_audio.py` besteht mit 679 eindeutigen Soundnummern und 162 deutschen Katalognummern. Die drei offiziellen Stopptanz-Textfragmente zählen dabei als eine gemeinsame Audionummer. Die sieben Ansagen 1100–1103 und 0351–0353 wurden im vorherigen Stand geprüft. Die vier zusätzlichen Farbansagen 1104–1106 und 0354 lassen sich ebenfalls mit FFmpeg fehlerfrei dekodieren. Die Grundhelligkeitsansagen 0916 und 0951 wurden bereits im vorherigen Optimierungsstand geprüft.

`build/pixelbox_sdcard.zip` enthält genau 679 MP3-Dateien unter `mp3/` und `advert/` und ist 8.369.249 Byte groß. Die Dateiliste entspricht vollständig dem deutschen SD-Verzeichnis; doppelte Archiveinträge und ZIP-Prüfsummen wurden geprüft. Das Paket wurde mit `python tools/package_pixel_box.py` erzeugt.

## Tonausfall und Wiederherstellung auf der Box

Am 14. Oktober 2026 meldete der Nutzer nach kurzer Benutzung einen vollständigen Tonausfall und eine stehenbleibende Wiedergabeanimation. Das Startprotokoll über COM3 zeigte `Com to DFPlayer broken` und `track_count: 0`. Eine Musik-Karte wurde erkannt, die Ordnerabfrage lieferte aber `0 tr in folder 1`; der Ablauf wechselte anschließend von Wiedergabe zu Idle. Auch ein Vergleichsbuild mit derselben Audiokonfiguration und ohne LED-Treiber blieb laut Nutzer stumm und meldete den Kommunikationsfehler. Der ausgeführte NeoPixel-Code ist damit als alleinige Ursache ausgeschlossen.

Ein separates Diagnoseprogramm fragte den AiO+-Player direkt über UART3 bei 9600 Baud ab. Bereits vor seinem Software-Reset antwortete der Player mit Lautstärke 14, 1.310 Titeln auf der SD-Karte und elf Titeln im Ordner `01`. Der erste Diagnosebuild hatte einen eigenen Fehler: `digitalWrite(HIGH)` vor `pinMode(OUTPUT)` setzt im verwendeten MegaCoreX nur den Pull-up und aktiviert die Stromhaltefunktion nicht. Die Reihenfolge wurde ausschließlich im Diagnoseprogramm korrigiert; die normale Firmware verwendet bereits die richtige Reihenfolge. Nach dem Upload des korrigierten Diagnoseprogramms bestätigte der Nutzer die automatische Musikwiedergabe. Das Programm greift nicht auf das EEPROM zu.

Anschließend lud der Nutzer die unveränderte normale Firmware `PixelBox_AiOPlus_5` wieder hoch und bestätigte funktionierenden Ton und leuchtende LEDs. Dafür waren weder neue SD-Dateien noch ein EEPROM-Reset erforderlich. Der Diagnoseablauf enthielt erneute Initialisierung, Player-Reset und Firmwarewechsel. Welche Maßnahme die ursprüngliche Störung beseitigte und wodurch sie ausgelöst wurde, ist nicht geklärt; eine dauerhafte Codekorrektur ist daraus nicht abzuleiten. Neustarts, Titelwechsel und längere Wiedergabe bleiben zur Prüfung auf ein erneutes Auftreten offen. Die lokalen Diagnoseprogramme liegen außerhalb der veröffentlichten Firmware im ignorierten `build/`-Verzeichnis.

## Noch auf der Box prüfen

Am 14. Oktober 2026 wurde die Firmware auf der realen AiO+-Box mit fünf Tasten geprüft. Zunächst lief das offizielle Ziel `ALLinONE_Plus_5`, dessen ELF keine Pixel-Box-Symbole enthielt; im über COM3 gelesenen Startprotokoll fehlten entsprechend die vier `NP_*`-Einstellungszeilen. Nach dem vom Nutzer ausgeführten Upload mit dem ausdrücklich gewählten Ziel `PixelBox_AiOPlus_5` bestätigte er, dass die LEDs leuchten. Dafür waren weder ein SD-Update noch ein EEPROM-Reset erforderlich. Die Auswahl des richtigen Uploadziels ist in [pixel-box-update.md](pixel-box-update.md) beschrieben.

Die SD-Karte enthält auf der Box weiterhin die alten Systemansagen. Bestätigt sind leuchtende LEDs, die Puls-Vorschau bei voller Grundhelligkeit und die normale Musikwiedergabe nach der oben beschriebenen Diagnose. Weitere einzelne Animationsabläufe, die neuen Ansagen und Kartenfunktionen sind noch offen. Ebenso offen sind die Prüfung der übernommenen Einstellungen auf der realen Box und das Neuprogrammieren alter Nachtlicht-Karten. Der Compiler prüft die AVR-Speichergrenzen; die Hosttests prüfen die Migration mit simuliertem EEPROM.

Prüfe nach dem Update Start/Abschalten, Kartenwechsel, alle fünf Wiedergabeanimationen, Pause, Vor/Zurück, Lautstärke, LED-Einstellungen und Neustart. Prüfe außerdem Ruhelicht, Animations-Wechselkarte, Farb-Durchschaltkarte, feste Farbkarten mit Vorschau beim Erstellen, Nachtlicht-Aktivierung/Deaktivierung, den Ablauf des zeitbegrenzten Nachtlichts und das Zusammenspiel mit Einschlaftimer, Spielen und Standby. Die bestehenden DFPlayer-Wartezeiten können die LED-Bildfolge während Ansagen unterbrechen; die reale Wirkung ist deshalb auf der Box zu prüfen. Für neue englische/französische Pixel-Box-Ansagen wurden keine Audiodateien erzeugt.
