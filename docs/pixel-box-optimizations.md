# Pixel-Box-Optimierungen vom 14. Oktober 2026

Die zehn Punkte aus der Codeprüfung sind umgesetzt. Die Animationen verwenden weiterhin einen Adafruit-Treiber für 12 LEDs am WS-Pin 18 und dessen vorhandenen 36-Byte-RGB-Puffer. Es gibt keinen zusätzlichen Animationspuffer.

| Punkt | Änderung | Warum das hilft |
| --- | --- | --- |
| 1. Quiz und Memory | `Tonuino::loop()` bestimmt die normale LED-Anzeige aus dem aktuellen TonUINO-Zustand. Quiz, Memory und ihre Startzustände verwenden die Wiedergabeanimation. Lautstärkefeedback folgt einer tatsächlichen Lautstärkeänderung. | Neue Spielzustände brauchen keine verstreuten LED-Aufrufe. Eine Quiz-Antworttaste löst kein Lautstärkefeedback aus, wenn sie nur eine Antwort auswählt. |
| 2. Farbzyklus | Nach genau `12 × 6 = 72` Bildern beginnt die nächste Runde. | Das identische Grenzbild wird nicht zweimal hintereinander ausgegeben. |
| 3. Vor/Zurück-Takt | Die Titelanimation verwendet ausdrücklich `cycleTime`, derzeit 50 ms. | Die alten 25 ms konnten im 50-ms-Hauptzyklus nicht erreicht werden. Das sichtbare Tempo bleibt erhalten. |
| 4. Farbauswahl auf AVR | Vorschau und Speichern verwenden gemeinsam `pixelHueFromOption()`: erst eins abziehen, dann ohne Vorzeichen mit 1.000 multiplizieren. | AVRs haben einen 16-Bit-`int`. Der alte Ausdruck konnte schon bei Option 33 überlaufen. Alle 65 Optionen reichen jetzt korrekt von 0 bis 64.000. |
| 5. Helligkeitsberechnung | `valueFromColor()` verwendet das Maximum der drei RGB-Bytes. Die Fließkommahelfer entfallen. | Das ergibt dieselbe HSV-Helligkeit und spart die dafür zuvor benötigte Software-Fließkommarechnung. |
| 6. EEPROM-Altformat | Die fünf alten LED-Bytes werden fest ab Adresse 140 gelesen. Die neuen Daten bleiben bei 148 bis 155 mit Kennung `PB` und Version 1. | Die Altadresse hängt nicht mehr von Größe und Padding des aktuellen `Settings`-Typs ab. Beim Import wird kein vollständiges zweites `Settings`-Objekt auf dem Stack angelegt. |
| 7. Adafruit-Verhalten im Test | Der Mock skaliert RGB-Werte wie der echte Treiber und bildet den verlustbehafteten Rücklesewert nach. Die Vorbereitung der Pulsanimation erkennt fehlenden Fortschritt. | Bei globaler Helligkeit 20 wird beispielsweise der angeforderte Wert 95 intern zu 7 und beim Lesen zu 85. Die Animation darf nicht auf einen exakt darstellbaren Zielwert warten. |
| 8. Weniger Pixelarbeit | Karten- und Titelanimation füllen die Grundfarbe einmal und schreiben Lichtspitze und Schweif einmal. Vor und Zurück teilen sich `animTrack()`. Die Pausenanimation verwendet `fill()`. | Die gleiche Lichtspitze wurde vorher innerhalb der Schleife zwölfmal geschrieben. Die bisherigen Schweiffarben bleiben erhalten, einschließlich der asymmetrischen Kartenanimation. |
| 9. Statische Anzeigen | Farbe und Helligkeit werden nur bei einem geänderten Bild gesendet. Der Lautstärkebalken wird nur bei geänderter LED-Anzahl neu gezeichnet; sein Zwei-Sekunden-Timer läuft weiter. | Ein unverändertes Menübild braucht keine neue Datenübertragung alle 50 ms. Animationen erhalten weiterhin ihre einzelnen Bilder. |
| 10. EEPROM-Schreibzugriffe | `EEPROM_update()` liest ein Byte vor dem Schreiben. Einstellungen, Fortschritt und zusätzliche Shortcuts verwenden den Helfer. Beim Löschen erfolgt ein gemeinsamer Commit. | AiO und AVR unterstützen unterschiedliche EEPROM-APIs. Der Helfer funktioniert mit beiden und vermeidet Schreibzugriffe auf unveränderte Bytes. Ein unverändertes Speichern schreibt null Bytes. |

## Start und Abschalten

Start und Abschalten bestehen weiterhin aus sechs symmetrischen LED-Bildern mit jeweils 100 ms Dauer. Beim Start wandern die leuchtenden Paare von außen nach innen. Beim Abschalten erlöschen sie von innen nach außen. Die Bilder entstehen in `loop()` mit Zeitvergleichen; die Animationen enthalten kein `delay()` mehr. Zeitdifferenzen werden als 32-Bit-Werte ausgewertet, damit der `millis()`-Überlauf funktioniert.

Während der Startanimation läuft die normale TonUINO-Steuerung weiter. Eine neue Karte oder eine Nachtlichtkarte kann ihren eigenen Effekt aktivieren. Der zuletzt angeforderte normale Zustand bestimmt die Anzeige nach dem Start.

Eine Abschaltanforderung startet die Abschaltanimation genau einmal. Wiederholte Anforderungen starten sie nicht neu, und das Abschalten hat auch gegenüber dem Nachtlicht Vorrang. Die Audiosteuerung wird während der sechs Bilder weiter bedient. Neue Karten und Tasten werden während der begonnenen Abschaltung nicht mehr verarbeitet, damit sie keine neue Wiedergabe starten. Erst nach der vollen Dauer des letzten Bildes beginnt die bestehende TonUINO-Stromabschaltung. Deren platinenabhängige Wartezeiten von 1.000 ms für den Verstärker und 500 ms am Shutdown-Pin bleiben erhalten.

## Anpassung an TonUINO

Normale Zustandsanzeige und kurze Rückmeldungen sind getrennt: `setNormalState()` merkt sich Wiedergabe, Pause, Idle oder Admin; `setState()` startet Karten- und Titelrückmeldungen. Eine wiederholte normale Zustandsmeldung unterbricht diese Rückmeldungen nicht. Die Animationsvorschau gehört bis zum Speichern oder Abbrechen dem Adminmenü. Nachtlicht unterdrückt die anderen Anzeigen, Abschalten beendet auch Nachtlicht.

Die LED-Komponente greift nicht mehr auf den globalen TonUINO-Singleton zu. `init()` und `updateFromSettings()` erhalten Einstellungen, `loop()` erhält Lautstärke und Grenzen. Dadurch lässt sich dieselbe Komponente unabhängig von RFID, Audio und dem Adminmenü testen.

`src/pixel_box.hpp` definiert die gespeicherten Animationsnummern ausdrücklich als 0 und 1. Menü und EEPROM-Prüfung verwenden dieselbe Anzahl. Neue Animationen müssen hinten ergänzt werden, damit vorhandene Einstellungen ihre Bedeutung behalten. Unbenutzte Deklarationen, Fließkommahelfer, das auskommentierte Animationstemplate und unnötige Includes wurden entfernt.

## Speicher und Prüfung

Im Standardbuild `PixelBox_AiOPlus_5` sinkt Flash von 43.454 auf 42.182 Byte: 1.272 Byte weniger, 6.970 Byte frei. Das statische RAM steigt durch gespeicherte Vorschauauswahl und die Erkennung unveränderter Bilder von 1.236 auf 1.242 Byte. Der bestehende LED-Puffer bleibt bei 36 Byte Heap zuzüglich Verwaltungsdaten; der Laufzeit-Stack ist in dieser Messung nicht enthalten.

Die EEPROM-Optimierung gilt auch für offizielle Varianten. Dafür benötigt `ALLinONE_Plus_5f` 40 Byte und `TonUINO_Classic_3` 48 Byte zusätzliches Flash; ihr statisches RAM bleibt gleich. Der offizielle Ring-Code entspricht weiter dem übernommenen Upstream-Stand und wird in Pixel-Box-Builds nicht eingebunden.

Messwerte, Testumfang und die noch erforderlichen Hardwareprüfungen stehen in [pixel-box-validation.md](pixel-box-validation.md).
