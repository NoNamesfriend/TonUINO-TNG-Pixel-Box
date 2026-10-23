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

Die erste Animation benötigt einen Helligkeitsunterschied zwischen Grundniveau und den hellen Punkten. Deshalb verwendet sie bei gespeicherten Werten über 95 intern ein Grundniveau von 95 und Lichtspitzen bis 255. Bei der festen globalen Helligkeit 20 entspricht das ungefähr RGB-Helligkeitswerten 7 für den Hintergrund und 20 für die Lichtspitzen. So bleibt auch bei Grundhelligkeitsstufe 16 ein deutlicher Unterschied. Werte bis 95, Farbe, Muster und Tempo bleiben erhalten. Die Begrenzung gilt sowohl für die Vorschau als auch für die Wiedergabe; sie verändert weder die gespeicherte Einstellung noch die Helligkeit anderer Animationen. Die feste Hardwaregrenze bleibt bei 20 von 255.

## Lichtkarten erstellen und verwenden

Wähle im Adminmenü Option 6, „Modifikationskarte“, und anschließend die gewünschte Option. Lege zum Schreiben eine Karte auf, sobald die Box dazu auffordert.

| Option innerhalb „Modifikationskarte“ | Funktion | Verhalten |
| --- | --- | --- |
| 12 | Nachtlicht | Gelb mit der gespeicherten Nachtlichthelligkeit. 5, 15, 30, 60 Minuten oder zuletzt unbegrenzt; 30 Minuten sind vorausgewählt |
| 13 | Wiedergabeanimation wechseln | Jedes erneute Auflegen wählt und speichert die nächste der fünf Play-Animationen |
| 14 | Grundfarben durchschalten | Jedes erneute Auflegen setzt die nächste von acht Farben |
| 15 | Feste Grundfarbe | Beim Erstellen wählst du mit Vorschau eine von 65 Farbstufen; beim Auflegen wird diese Farbe gesetzt |

„Nachtlicht“, „Ruhelicht“ und „Nachtlicht mit Zeitbegrenzung“ sind zu einer Funktion zusammengefasst. Nach der Auswahl von Nachtlicht änderst du die Dauer mit Hoch/Runter und bestätigst mit der Pausetaste.

Bereits geschriebene Nachtlicht- und Ruhelichtkarten ohne Dauer schalten dasselbe gelbe Nachtlicht unbegrenzt ein. Vorhandene zeitbegrenzte Karten behalten ihre Laufzeit. Jede dieser Karten schaltet ein laufendes Nachtlicht beim erneuten Auflegen aus; anschließend erscheint wieder die normale Anzeige. Karten müssen für die Vereinheitlichung nicht neu beschrieben werden.

Das Nachtlicht leuchtet unabhängig von der eingestellten Grundfarbe gelb. Die gespeicherte Nachtlichthelligkeit gilt für alle bisherigen Kartenarten, auch während der Ausblendung. Die frühere feste Ruhelichthelligkeit entfällt.

Bei einer Zeitbegrenzung leuchtet das Nachtlicht zunächst gleichmäßig. In der letzten Minute erlöschen symmetrische Paare von außen nach innen, bis die Mitte zuletzt ausgeht. Die Box wird dadurch nicht ausgeschaltet und die Musik läuft weiter. Nach dem Ende bleiben die LEDs dunkel. Erneutes Auflegen startet eine neue Laufzeit. Bei „Unbegrenzt“ bleibt das Licht ohne Ausblendung eingeschaltet, bis es mit einer Nachtlichtkarte oder anderweitig beendet wird.

Das Nachtlicht ist unabhängig von Audio-Modifikationen. Beispielsweise können Nachtlicht und Einschlaftimer gemeinsam aktiv sein. Das gilt auch für die bisherigen Nachtlicht- und Ruhelichtkarten: Sie belegen keinen Audio-Modifikationsplatz mehr. Aktivieren, Wechseln und Zurücksetzen einer Audio-Modifikation beendet das Nachtlicht nicht. Ein ausdrücklich eingestellter Einschlaftimer kann die Box weiterhin abschalten.

Im Idle-/Pausenzustand läuft die konfigurierte Standby-Zeit auch bei aktivem Nachtlicht weiter. Ist sie abgelaufen, merkt sich die Box die fällige Abschaltung und wartet, bis das Nachtlicht endet oder deaktiviert wird. Dann schaltet sie sich ohne neue Wartezeit aus. Endet das Licht vor der Standby-Frist, läuft nur deren Restzeit weiter. Bei 5 Minuten Standby und 30 Minuten Nachtlicht geht die Box ohne Musikwiedergabe deshalb nach etwa 30 Minuten aus; bei 60 Minuten Standby und 30 Minuten Nachtlicht nach etwa 60 Minuten.

Solange Musik läuft, bleibt die Standby-Abschaltung deaktiviert. Starten oder Fortsetzen der Wiedergabe verwirft auch eine bereits vorgemerkte Standby-Abschaltung; beim nächsten Pausieren oder Wiedergabeende beginnt die volle Standby-Zeit. Bei deaktiviertem Standby gibt es nach dem Nachtlicht keine automatische Abschaltung. Unbegrenztes Nachtlicht hält eine fällige Abschaltung bis zum Ausschalten des Lichts zurück. Ein leerer Akku, ausdrücklich angefordertes Abschalten und die Einschlaftimer-Karte haben weiterhin Vorrang.

Die Animationskarte schaltet von der aktuell gespeicherten Auswahl weiter: `dotForwardPulse` → `colorCycle` → Leuchtpunkt mit Schweif → weiche Farbüberblendung → „Lebhaft“ → `dotForwardPulse`. Für den nächsten Wechsel nimmst du die Karte ab und legst sie erneut auf. Eine liegenbleibende Karte rotiert nicht fortlaufend. Die Auswahl wird sofort gespeichert und bleibt nach einem Neustart erhalten; das Adminmenü zeigt dieselbe Auswahl.

Die Animationskarte beendet Nachtlicht, damit während der Wiedergabe die neue Animation sichtbar wird. Musik und aktive Audio-Modifikationen laufen weiter. Auch im Idle-/Pausenzustand lässt sich die Auswahl wechseln; die Play-Animation erscheint beim Fortsetzen der Wiedergabe. Einschlaftimer-, Spiel- und kurze Rückmeldungen behalten ihre Priorität.

Das aktive Nachtlicht gilt nur für die aktuelle Sitzung. Die Nachtlichtdauer steht auf der Karte. Die Animationsauswahl bleibt im EEPROM gespeichert.

## Grundfarbe mit Karten setzen

Die Durchschaltkarte folgt der Reihenfolge Rot → Orange → Gelb → Grün → Türkis → Blau → Violett → Pink → Rot. Sie geht von der derzeit gespeicherten Grundfarbe aus. Liegt eine frei gewählte Farbe zwischen zwei Palettenfarben, folgt die nächste Farbe im Farbkreis. Eine liegenbleibende Karte schaltet nur einmal; nimm sie für den nächsten Wechsel ab und lege sie erneut auf.

Für eine feste Farbkarte wählst du innerhalb „Modifikationskarte“ Option 15. Die aktuell gespeicherte Grundfarbe ist vorausgewählt. Hoch/Runter wechseln durch dieselben 65 Farbstufen wie bei der bisherigen Farbeinstellung; der Streifen zeigt die Auswahl direkt. Die Pausetaste bestätigt die Farbe für die Karte, anschließend wird die Karte geschrieben. Beim Erstellen oder Abbrechen bleibt die Grundfarbe der Box erhalten. Erst das Auflegen der fertigen Karte setzt und speichert die Farbe.

Beide Varianten speichern im bestehenden Farbwert, der auch im Adminmenü unter Option 15 erscheint. Die Farbe bleibt nach einem Neustart erhalten. Wiederholtes Auflegen derselben festen Farbkarte setzt dieselbe Farbe; unveränderte EEPROM-Bytes werden nicht neu geschrieben. Animation, Helligkeit, Musik und aktive Audio-Modifikationen bleiben erhalten. Das Nachtlicht bleibt ebenfalls aktiv, seine Laufzeit wird nicht neu gestartet.

Nach einer gültigen Farbkarte leuchten alle LEDs für 1,2 Sekunden in der gewählten Farbe. Die Vorschau wird bei Nachtlicht gedämpft. Danach kehrt die vorherige Anzeige zurück. Eine weitere Farbkarte beginnt eine neue Vorschau; Abschalten hat Vorrang.

Die Grundfarbe wirkt auf Idle, Start, Pause, Vor/Zurück, Puls und Leuchtpunkt mit Schweif sowie den Hintergrund der Kartenrückmeldung. Sie ist die Ausgangsfarbe der weichen Farbüberblendung. Die grüne Bewegung bei „Karte erkannt“ behält ihre Bedeutung. Farbzyklus und „Lebhaft“ wechseln ihre Farben selbst. Nachtlicht, Einschlaftimer, Lautstärkeanzeige, Adminanzeige, Spielhälften und Status-/Warnsignale verwenden weiterhin ihre eigenen Farben.

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

Bei Nachtlicht bleiben kurze Rückmeldungen gedämpft. Das Nachtlicht hat Vorrang vor der Einschlaftimer-Anzeige. Die normale Anzeige kehrt nach einer Rückmeldung automatisch zurück. Im Adminmenü sind Einstellungen und Animationsvorschau auch während eines Nachtlichts sichtbar; danach wird das Nachtlicht fortgesetzt.

## Karten- und Soundformat

Menüpositionen, gespeicherte Kartenmodi und Soundnummern sind getrennt. Alle Lichtkarten verwenden Ordner 0; die gespeicherten Modusnummern bleiben unverändert.

| Karte | RFID-Modus | `special` | Menüansage | Aktivierungsansage |
| --- | ---: | --- | --- | --- |
| Nachtlicht | 202 | Minuten 1–120; 255 ist unbegrenzt; 0 oder andere ungültige Werte verwenden weiterhin 30 | `mp3/0982_modifier_night_light.mp3` | `advert/0350_night_light.mp3` |
| Bisheriges Nachtlicht | 12 | Wird ignoriert; unbegrenzt | Kein eigener Menüpunkt | `advert/0350_night_light.mp3` |
| Bisheriges Ruhelicht | 200 | Wird ignoriert; unbegrenzt | Kein eigener Menüpunkt | `advert/0350_night_light.mp3` |
| Wiedergabeanimation wechseln | 201 | 0 | `mp3/1101_modifier_next_animation.mp3` | `advert/0352_next_animation.mp3` |
| Grundfarben durchschalten | 203 | 0 | `mp3/1104_modifier_next_color.mp3` | `advert/0354_color_selected.mp3` |
| Feste Grundfarbe | 204 | Farbauswahl 1 bis 65 | `mp3/1105_modifier_fixed_color.mp3` | `advert/0354_color_selected.mp3` |

`special2` bleibt 0. Die Einleitung für die Nachtlichtdauer ist `mp3/1103_night_duration_intro.mp3`. Die Zeitansagen 0961 bis 0964 werden wiederverwendet; die letzte Auswahl verwendet `mp3/1107_night_duration_unlimited.mp3`. Bereits programmierte Karten mit Modus 202 behalten ihre Dauer, insbesondere bedeutet der bisherige Standardwert 0 weiterhin 30 Minuten. Für eine andere Dauer beschreibst du die Karte über Option 12 neu.

Die Modusnummern 12 und 200 bis 204 sind Festlegungen dieses Forks; sie müssen bei späteren Upstream-Updates weiter abgeglichen werden. Historische Karten mit Modus 9 gehören nicht zu diesen kompatiblen Nachtlichtkarten: Dieser Modus steht für „Pause nach jedem Track“. Musikordner mit Licht-Modi werden als ungültige Karte zurückgewiesen. Eine feste Farbkarte mit `special` außerhalb 1 bis 65 zeigt einen Lesefehler und ändert keine Einstellung.

Für feste Farbkarten verwendet die Auswahl die Einleitung `mp3/1106_card_color_intro.mp3` und die vorhandenen Zahlenansagen 1 bis 65. Die vier Licht-Menüpunkte 12 bis 15 verweisen ausdrücklich auf 0982, 1101, 1104 und 1105. Die Palette verwendet HSV-Farbwerte 0, 5.000, 11.000, 22.000, 32.000, 44.000, 50.000 und 58.000. Feste Farboptionen werden wie im bestehenden Adminmenü als `(Auswahl - 1) × 1.000` umgesetzt.

Eine bereits als „Lebhaft“ programmierte Karte mit Modus 201 wechselt weiterhin die Play-Animation. Die Karte muss dafür nicht neu beschrieben werden.

Für dieses Update werden die neue Firmware und die ersetzte Menüansage 0982 benötigt. Die Ansagen 1103 und 1107 aus der vorherigen Erweiterung bleiben erforderlich. Die früheren Ansagen 1100, 1102, 0351 und 0353 bleiben als unbenutzte Dateien im Systempaket; die Firmware ruft sie nicht mehr auf. Erzeuge das vollständige deutsche SD-Paket mit `python tools/package_pixel_box.py`; der allgemeine Updateablauf steht in [pixel-box-update.md](pixel-box-update.md). `tools/generate_pixel_box_prompts.ps1` kann die eigenen Ansagen unter Windows mit der installierten Stimme „Microsoft Katja“ und FFmpeg erneut erstellen.

## Implementierung und Prüfung

Die neuen Bilder werden mit Ganzzahlrechnung aus Zeit und Zustand berechnet. Eine 32-Byte-Tabelle im Flash liefert die Pulsform. Es gibt keinen weiteren LED-Puffer und keine neuen Heap-Reservierungen für die Effekte. Die vorhandenen beiden Animationen bleiben in `neo_pixels.cpp`; die neuen Effekte stehen in `neo_pixels_effects.cpp`.

Die Minuten-/Sekundenrechnung für Einschlaftimer und Tanzpausen verwendet nun ausdrücklich 32 Bit. Der allgemeine `Timer` vergleicht Fristen über den 32-Bit-Überlauf hinweg. Seine zulässigen Zeitabstände liegen unter `2^31` Millisekunden; alle hier verwendeten Laufzeiten liegen darunter.

Die bestehende DFPlayer-Steuerung enthält weiterhin Wartezeiten und die bisherigen Schutzabläufe für verschiedene Player-Varianten. Die neuen LED-Effekte selbst enthalten kein `delay()`. Die Steuerung des DFPlayers wurde in dieser Erweiterung nicht durch eine neue Audio-Zustandsmaschine ersetzt.

Die konkreten Speichergrößen und lokalen Testergebnisse stehen in [pixel-box-validation.md](pixel-box-validation.md). Auf der realen Box bleiben Lichtwirkung, neue Ansagen und das Auflegen der geschriebenen Karten zu prüfen.
