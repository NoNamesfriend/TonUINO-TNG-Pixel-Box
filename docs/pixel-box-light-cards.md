# Ausgewählte Animationen und Lichtkarten

Diese Erweiterung setzt die Auswahl aus der Animationsvorschau um. Die Box verwendet weiterhin zwölf LEDs am WS-Pin 18 des AiO+ und die feste maximale Adafruit-Helligkeit 20 von 255. Die Vorschauhelligkeit von 75 Prozent wird nicht als neue Geräteeinstellung übernommen; vorhandene Farbe, Grundhelligkeit und Nachtlichthelligkeit bleiben gespeichert.

## Wiedergabeanimation wählen

Im Adminmenü bleibt Option 17 die Auswahl der Wiedergabeanimation. Die Vorschau läuft auf dem Streifen; erst die Pausetaste speichert die Auswahl.

| Angesagte Auswahl | Gespeicherte ID | Animation |
| --- | ---: | --- |
| 1 | 0 | Bestehendes `dotForwardPulse` |
| 2 | 1 | Bestehendes `colorCycle` |
| 3 | 2 | Leuchtpunkt mit Schweif: ein Punkt mit drei abnehmenden Nachleuchtpunkten, 2,4 Sekunden pro Runde |
| 4 | 3 | Weiche Farbüberblendung: alle LEDs wechseln gemeinsam zwischen drei Farben, zwölf Sekunden pro Runde |

Die zwei bisherigen Animationen behalten ihre IDs und ihre Bedeutung. Beide neuen Animationen nutzen die gespeicherte Grundfarbe als Ausgangspunkt. Das EEPROM-Format bleibt bei den bisherigen fünf LED-Datenbytes und der Kennung `PB`, Version 1. Zusätzliche Shortcuts und Hörbuchfortschritt werden nicht verschoben.

## Lichtkarten erstellen und verwenden

Wähle im Adminmenü Option 6, „Modifikationskarte“, und anschließend die gewünschte Option. Lege zum Schreiben eine Karte auf, sobald die Box dazu auffordert.

| Option innerhalb „Modifikationskarte“ | Funktion | Verhalten |
| --- | --- | --- |
| 12 | Bisheriges Nachtlicht | Dauerhaftes Nachtlicht mit der gespeicherten Nachtlichthelligkeit |
| 13 | Ruhelicht | Schwaches, warmes Dauerlicht ohne Bewegung; Wiedergabe und Audio-Modifikationen laufen weiter |
| 14 | Lichtprofil „Lebhaft“ | Zwei Lichtspitzen mit wechselnden Farben während der Wiedergabe |
| 15 | Zeitbegrenztes Nachtlicht | Standardmäßig 30 Minuten; über Hoch/Runter sind 5, 15, 30 oder 60 Minuten wählbar, Pausetaste bestätigt |

Lege eine aktive Ruhelicht- oder Lebhaft-Karte erneut auf, um zur gespeicherten normalen Wiedergabeanimation zurückzukehren. Eine andere Lichtprofilkarte wechselt direkt das Profil. Beim Auflegen einer Profilkarte endet ein laufendes Nachtlicht.

Die neuen Lichtkarten sind unabhängig von den bisherigen Audio-Modifikationen. Beispielsweise können Ruhelicht und Einschlaftimer gemeinsam aktiv sein. Das zeitbegrenzte Nachtlicht kann ebenfalls mit einer Audio-Modifikation kombiniert werden. Die alte Nachtlichtkarte verwendet weiterhin den bisherigen Nachtlicht-Modifikator.

Das zeitbegrenzte Nachtlicht leuchtet zunächst gleichmäßig. In der letzten Minute erlöschen symmetrische Paare von außen nach innen, bis die Mitte zuletzt ausgeht. Die Box wird dadurch nicht ausgeschaltet und die Musik läuft weiter. Nach dem Ende bleiben die LEDs dunkel. Erneutes Auflegen startet eine neue Laufzeit. Erneutes Auflegen während einer noch laufenden Laufzeit beendet das Nachtlicht vorzeitig.

Ein aktives Nachtlicht verhindert eine vorzeitige Standby-Abschaltung. Sobald es endet oder deaktiviert wird, beginnt im Idle-/Pausenzustand die konfigurierte Standby-Zeit neu. Ein leerer Akku und ausdrücklich angefordertes Abschalten haben weiterhin Vorrang.

Die neuen Profile und Nachtlichtlaufzeiten werden auf der Modifikationskarte beziehungsweise nur für die aktuelle Sitzung verwaltet. Ein Neustart stellt die gespeicherte normale Animation wieder her.

## Rückmeldungen auf dem Streifen

| Ereignis | Ausgewählter Effekt |
| --- | --- |
| Karte erkannt | Grün breitet sich von der Mitte nach außen aus; Dauer 900 ms |
| Leere Karte | Zwei cyanfarbene Hinweise auf dem ganzen Streifen; die Kartenprogrammierung beginnt wie bisher |
| Lesefehler | Drei kurze bernsteinfarbene Hinweise auf dem ganzen Streifen |
| Gesperrte Taste | Ein kurzer bernsteinfarbener Hinweis auf dem ganzen Streifen |
| Einschlaftimer | Warme LED-Paare ziehen sich mit der verbleibenden Zeit zur Mitte zurück |
| Einschlaftimer wartet auf Titelende | Zwei mittlere LEDs pulsieren langsam; die Anzeige behauptet keinen weiteren Zeit-Countdown |
| Memory-Auswahl | Die erste Karte aktiviert die linke Hälfte, die zweite zusätzlich die rechte; beide haben unterschiedliche neutrale Farben |
| Quiz-Antwortwahl | Zwei neutrale Hälften, ohne Hinweis auf die richtige Antwort |
| Bestätigtes Spiel-Ergebnis | Bei Erfolg ein gemeinsamer grüner Puls; bei falscher Antwort oder fehlendem Paar wechseln die Hälften kurz in Bernstein |
| Akku niedrig | Ein gemeinsamer bernsteinfarbener Warnpuls; danach kehrt die vorherige Anzeige zurück |

Im Ruhelicht bleiben kurze Rückmeldungen gedämpft. Die Einschlaftimer-Anzeige verwendet ebenfalls eine geringere Helligkeit, wenn Ruhelicht aktiv ist. Die normale Anzeige kehrt nach einer Rückmeldung automatisch zurück. Im Adminmenü sind Einstellungen und Animationsvorschau auch während eines Nachtlichts sichtbar; danach wird das Nachtlicht fortgesetzt.

## Karten- und Soundformat

Menüpositionen, gespeicherte Kartenmodi und Soundnummern sind getrennt. Die drei neuen Karten verwenden Ordner 0 und folgende Modusnummern dieses Forks:

| Karte | RFID-Modus | `special` | Menüansage | Aktivierungsansage |
| --- | ---: | --- | --- | --- |
| Ruhelicht | 200 | 0 | `mp3/1100_modifier_quiet_light.mp3` | `advert/0351_quiet_light.mp3` |
| Lebhaft | 201 | 0 | `mp3/1101_modifier_lively_light.mp3` | `advert/0352_lively_light.mp3` |
| Zeitbegrenztes Nachtlicht | 202 | Minuten; 0 oder ungültige Werte verwenden 30 | `mp3/1102_modifier_timed_night.mp3` | `advert/0353_timed_night_light.mp3` |

`special2` bleibt 0. Die neue Einleitung für die Nachtlichtdauer ist `mp3/1103_night_duration_intro.mp3`. Die Zeitansagen 0961 bis 0964 werden wiederverwendet. Die Modusnummern 200 bis 202 sind Festlegungen dieses Forks; sie müssen bei späteren Upstream-Updates weiter abgeglichen werden. Musikordner mit diesen Licht-Modi werden als ungültige Karte zurückgewiesen.

Das vollständige deutsche SD-Paket muss aktualisiert werden, damit die neuen Ansagen vorhanden sind. Erzeuge es mit `python tools/package_pixel_box.py`; der allgemeine Updateablauf steht in [pixel-box-update.md](pixel-box-update.md). Die sieben neuen deutschen Ansagen wurden lokal mit „Microsoft Katja“ erzeugt. `tools/generate_pixel_box_prompts.ps1` kann sie unter Windows mit dieser installierten Stimme und FFmpeg erneut erstellen.

## Implementierung und Prüfung

Die neuen Bilder werden mit Ganzzahlrechnung aus Zeit und Zustand berechnet. Eine 32-Byte-Tabelle im Flash liefert die Pulsform. Es gibt keinen weiteren LED-Puffer und keine neuen Heap-Reservierungen für die Effekte. Die vorhandenen beiden Animationen bleiben in `neo_pixels.cpp`; die neuen Effekte stehen in `neo_pixels_effects.cpp`.

Die Minuten-/Sekundenrechnung für Einschlaftimer und Tanzpausen verwendet nun ausdrücklich 32 Bit. Der allgemeine `Timer` vergleicht Fristen über den 32-Bit-Überlauf hinweg. Seine zulässigen Zeitabstände liegen unter `2^31` Millisekunden; alle hier verwendeten Laufzeiten liegen darunter.

Die bestehende DFPlayer-Steuerung enthält weiterhin Wartezeiten und die bisherigen Schutzabläufe für verschiedene Player-Varianten. Die neuen LED-Effekte selbst enthalten kein `delay()`. Die Steuerung des DFPlayers wurde in dieser Erweiterung nicht durch eine neue Audio-Zustandsmaschine ersetzt.

Die konkreten Speichergrößen und lokalen Testergebnisse stehen in [pixel-box-validation.md](pixel-box-validation.md). Auf der realen Box bleiben Lichtwirkung, neue Ansagen und das Auflegen der geschriebenen Karten zu prüfen.
