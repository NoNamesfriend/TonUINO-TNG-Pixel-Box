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
| 5 | 4 | Lebhaft: zwei Lichtspitzen mit wechselnden Farben |

Die bisherigen vier Animationen behalten ihre IDs und ihre Bedeutung. Leuchtpunkt und Farbüberblendung nutzen die gespeicherte Grundfarbe als Ausgangspunkt; „Lebhaft“ wechselt die Farbe selbst. Das EEPROM-Format bleibt bei den bisherigen fünf LED-Datenbytes und der Kennung `PB`, Version 1. Zusätzliche Shortcuts und Hörbuchfortschritt werden nicht verschoben.

## Lichtkarten erstellen und verwenden

Wähle im Adminmenü Option 6, „Modifikationskarte“, und anschließend die gewünschte Option. Lege zum Schreiben eine Karte auf, sobald die Box dazu auffordert.

| Option innerhalb „Modifikationskarte“ | Funktion | Verhalten |
| --- | --- | --- |
| 12 | Bisheriges Nachtlicht | Dauerhaftes Nachtlicht mit der gespeicherten Nachtlichthelligkeit |
| 13 | Ruhelicht | Schwaches, warmes Dauerlicht ohne Bewegung; Wiedergabe und Audio-Modifikationen laufen weiter |
| 14 | Wiedergabeanimation wechseln | Jedes erneute Auflegen wählt und speichert die nächste der fünf Play-Animationen |
| 15 | Zeitbegrenztes Nachtlicht | Standardmäßig 30 Minuten; über Hoch/Runter sind 5, 15, 30 oder 60 Minuten wählbar, Pausetaste bestätigt |
| 16 | Grundfarben durchschalten | Jedes erneute Auflegen setzt die nächste von acht Farben |
| 17 | Feste Grundfarbe | Beim Erstellen wählst du mit Vorschau eine von 65 Farbstufen; beim Auflegen wird diese Farbe gesetzt |

Lege eine aktive Ruhelichtkarte erneut auf, um zur gespeicherten normalen Wiedergabeanimation zurückzukehren.

Die Animationskarte schaltet von der aktuell gespeicherten Auswahl weiter: `dotForwardPulse` → `colorCycle` → Leuchtpunkt mit Schweif → weiche Farbüberblendung → „Lebhaft“ → `dotForwardPulse`. Für den nächsten Wechsel nimmst du die Karte ab und legst sie erneut auf. Eine liegenbleibende Karte rotiert nicht fortlaufend. Die Auswahl wird sofort gespeichert und bleibt nach einem Neustart erhalten; das Adminmenü zeigt dieselbe Auswahl.

Die Animationskarte beendet Ruhelicht und Nachtlicht, damit während der Wiedergabe die neue Animation sichtbar wird. Musik und aktive Audio-Modifikationen laufen weiter. Auch im Idle-/Pausenzustand lässt sich die Auswahl wechseln; die Play-Animation erscheint beim Fortsetzen der Wiedergabe. Einschlaftimer-, Spiel- und kurze Rückmeldungen behalten ihre Priorität.

Die neuen Lichtkarten sind unabhängig von den bisherigen Audio-Modifikationen. Beispielsweise können Ruhelicht und Einschlaftimer gemeinsam aktiv sein. Das zeitbegrenzte Nachtlicht kann ebenfalls mit einer Audio-Modifikation kombiniert werden. Die alte Nachtlichtkarte verwendet weiterhin den bisherigen Nachtlicht-Modifikator.

Das zeitbegrenzte Nachtlicht leuchtet zunächst gleichmäßig. In der letzten Minute erlöschen symmetrische Paare von außen nach innen, bis die Mitte zuletzt ausgeht. Die Box wird dadurch nicht ausgeschaltet und die Musik läuft weiter. Nach dem Ende bleiben die LEDs dunkel. Erneutes Auflegen startet eine neue Laufzeit. Erneutes Auflegen während einer noch laufenden Laufzeit beendet das Nachtlicht vorzeitig.

Ein aktives Nachtlicht verhindert eine vorzeitige Standby-Abschaltung. Sobald es endet oder deaktiviert wird, beginnt im Idle-/Pausenzustand die konfigurierte Standby-Zeit neu. Ein leerer Akku und ausdrücklich angefordertes Abschalten haben weiterhin Vorrang.

Ruhelicht und das aktive Nachtlicht gelten nur für die aktuelle Sitzung. Die Nachtlichtdauer steht auf der Karte. Die Animationsauswahl bleibt im EEPROM gespeichert.

## Grundfarbe mit Karten setzen

Die Durchschaltkarte folgt der Reihenfolge Rot → Orange → Gelb → Grün → Türkis → Blau → Violett → Pink → Rot. Sie geht von der derzeit gespeicherten Grundfarbe aus. Liegt eine frei gewählte Farbe zwischen zwei Palettenfarben, folgt die nächste Farbe im Farbkreis. Eine liegenbleibende Karte schaltet nur einmal; nimm sie für den nächsten Wechsel ab und lege sie erneut auf.

Für eine feste Farbkarte wählst du innerhalb „Modifikationskarte“ Option 17. Die aktuell gespeicherte Grundfarbe ist vorausgewählt. Hoch/Runter wechseln durch dieselben 65 Farbstufen wie bei der bisherigen Farbeinstellung; der Streifen zeigt die Auswahl direkt. Die Pausetaste bestätigt die Farbe für die Karte, anschließend wird die Karte geschrieben. Beim Erstellen oder Abbrechen bleibt die Grundfarbe der Box erhalten. Erst das Auflegen der fertigen Karte setzt und speichert die Farbe.

Beide Varianten speichern im bestehenden Farbwert, der auch im Adminmenü unter Option 15 erscheint. Die Farbe bleibt nach einem Neustart erhalten. Wiederholtes Auflegen derselben festen Farbkarte setzt dieselbe Farbe; unveränderte EEPROM-Bytes werden nicht neu geschrieben. Animation, Helligkeit, Musik und aktive Audio-Modifikationen bleiben erhalten. Ruhelicht und Nachtlicht bleiben ebenfalls aktiv, ihre Laufzeit wird nicht neu gestartet.

Nach einer gültigen Farbkarte leuchten alle LEDs für 1,2 Sekunden in der gewählten Farbe. Die Vorschau wird bei Ruhelicht oder Nachtlicht gedämpft. Danach kehrt die vorherige Anzeige zurück. Eine weitere Farbkarte beginnt eine neue Vorschau; Abschalten hat Vorrang.

Die Grundfarbe wirkt auf Idle, Start, Pause, Vor/Zurück, Puls und Leuchtpunkt mit Schweif sowie den Hintergrund der Kartenrückmeldung. Sie ist die Ausgangsfarbe der weichen Farbüberblendung. Die grüne Bewegung bei „Karte erkannt“ behält ihre Bedeutung. Farbzyklus und „Lebhaft“ wechseln ihre Farben selbst. Ruhelicht, Nachtlicht, Einschlaftimer, Lautstärkeanzeige, Adminanzeige, Spielhälften und Status-/Warnsignale verwenden weiterhin ihre eigenen Farben.

## Rückmeldungen auf dem Streifen

| Ereignis | Ausgewählter Effekt |
| --- | --- |
| Karte erkannt | Grün breitet sich von der Mitte nach außen aus; Dauer 900 ms. Farbkarten verwenden stattdessen die Farbvorschau |
| Grundfarbe per Karte gesetzt | Der ganze Streifen zeigt die ausgewählte Farbe für 1,2 Sekunden |
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

Menüpositionen, gespeicherte Kartenmodi und Soundnummern sind getrennt. Die fünf neuen Karten verwenden Ordner 0 und folgende Modusnummern dieses Forks:

| Karte | RFID-Modus | `special` | Menüansage | Aktivierungsansage |
| --- | ---: | --- | --- | --- |
| Ruhelicht | 200 | 0 | `mp3/1100_modifier_quiet_light.mp3` | `advert/0351_quiet_light.mp3` |
| Wiedergabeanimation wechseln | 201 | 0 | `mp3/1101_modifier_next_animation.mp3` | `advert/0352_next_animation.mp3` |
| Zeitbegrenztes Nachtlicht | 202 | Minuten; 0 oder ungültige Werte verwenden 30 | `mp3/1102_modifier_timed_night.mp3` | `advert/0353_timed_night_light.mp3` |
| Grundfarben durchschalten | 203 | 0 | `mp3/1104_modifier_next_color.mp3` | `advert/0354_color_selected.mp3` |
| Feste Grundfarbe | 204 | Farbauswahl 1 bis 65 | `mp3/1105_modifier_fixed_color.mp3` | `advert/0354_color_selected.mp3` |

`special2` bleibt 0. Die neue Einleitung für die Nachtlichtdauer ist `mp3/1103_night_duration_intro.mp3`. Die Zeitansagen 0961 bis 0964 werden wiederverwendet. Die Modusnummern 200 bis 204 sind Festlegungen dieses Forks; sie müssen bei späteren Upstream-Updates weiter abgeglichen werden. Musikordner mit diesen Licht-Modi werden als ungültige Karte zurückgewiesen. Eine feste Farbkarte mit `special` außerhalb 1 bis 65 zeigt einen Lesefehler und ändert keine Einstellung.

Für feste Farbkarten verwendet die Auswahl die neue Einleitung `mp3/1106_card_color_intro.mp3` und die vorhandenen Zahlenansagen 1 bis 65. Menüoption 16 verweist ausdrücklich auf 1104, damit sie nicht die Nachtlicht-Einleitung 1103 abspielt. Die Palette verwendet HSV-Farbwerte 0, 5.000, 11.000, 22.000, 32.000, 44.000, 50.000 und 58.000. Feste Farboptionen werden wie im bestehenden Adminmenü als `(Auswahl - 1) × 1.000` umgesetzt.

Eine bereits als „Lebhaft“ programmierte Karte mit Modus 201 wechselt nach diesem Update die Play-Animation. Die Karte muss dafür nicht neu beschrieben werden. Aktualisiere die Systemordner auf der SD-Karte vollständig, damit die ersetzten Ansagen 1101 und 0352 passen.

Das vollständige deutsche SD-Paket muss aktualisiert werden, damit die neuen Ansagen vorhanden sind. Erzeuge es mit `python tools/package_pixel_box.py`; der allgemeine Updateablauf steht in [pixel-box-update.md](pixel-box-update.md). Die elf zusätzlichen deutschen Ansagen wurden lokal mit „Microsoft Katja“ erzeugt. `tools/generate_pixel_box_prompts.ps1` kann sie unter Windows mit dieser installierten Stimme und FFmpeg erneut erstellen.

## Implementierung und Prüfung

Die neuen Bilder werden mit Ganzzahlrechnung aus Zeit und Zustand berechnet. Eine 32-Byte-Tabelle im Flash liefert die Pulsform. Es gibt keinen weiteren LED-Puffer und keine neuen Heap-Reservierungen für die Effekte. Die vorhandenen beiden Animationen bleiben in `neo_pixels.cpp`; die neuen Effekte stehen in `neo_pixels_effects.cpp`.

Die Minuten-/Sekundenrechnung für Einschlaftimer und Tanzpausen verwendet nun ausdrücklich 32 Bit. Der allgemeine `Timer` vergleicht Fristen über den 32-Bit-Überlauf hinweg. Seine zulässigen Zeitabstände liegen unter `2^31` Millisekunden; alle hier verwendeten Laufzeiten liegen darunter.

Die bestehende DFPlayer-Steuerung enthält weiterhin Wartezeiten und die bisherigen Schutzabläufe für verschiedene Player-Varianten. Die neuen LED-Effekte selbst enthalten kein `delay()`. Die Steuerung des DFPlayers wurde in dieser Erweiterung nicht durch eine neue Audio-Zustandsmaschine ersetzt.

Die konkreten Speichergrößen und lokalen Testergebnisse stehen in [pixel-box-validation.md](pixel-box-validation.md). Auf der realen Box bleiben Lichtwirkung, neue Ansagen und das Auflegen der geschriebenen Karten zu prüfen.
