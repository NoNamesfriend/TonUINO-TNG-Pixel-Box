# TonUINO - Pixel Box
Das ist ein Fork der TonUINO Musikbox.
Alle relevanten Informationen sind im Hauptrepositorie zu finden: https://github.com/tonuino/TonUINO-TNG

In diesem Fork wird die TonUINO Software mit der Integration von Adafruit NeoPixel erweitert.

# Features
Hier sind alle Features aufgelistet, die momentan ergänzt wurden.

- Animation beim Starten der Box
- Animation beim Abschalten der Box
- Animation beim Auflegen einer neuen Karte
- Animation während Song spielt
- Animation wenn Song pausiert
- Animation für nächster Song
- Animation für vorheriger Song

- Farbliche Darstellung der Lautstärke (Abhängig von Einstellungen Min/Max)
- Farbliche Darstellung wenn im Admin Menü
- Nachtlicht Modifier (Permanentes gelbes Licht und keine anderen Animationen)
- Erstellen einer neuen Nachtlicht Modifier-Karte

- Einstellung für Farbe der LEDs
- Einstellung für Grundhelligkeit der Animationen
- Einstellung für Auswahl einer Play-Animation
- Einstellung für Helligkeit des Nachtlichts

# Installation
In dem Ordner tools befindet sich ein Zip Verzeichnis. In diesem liegen die neu hinzugefügten mp3 Dateinen. Bitte in den den jeweiligen Ordner (mp3, advert) auf der SD-Karte kopieren.
Alternativ können die Sounddateien neu generiert werden.

In der constant.hpp gibt es 3 Werte die angepasst werden können.
NeoPixelPin: Standard Pin 18 (WS Pin auf Button Board AIO+).
NeoPixelCount: Anzahl der verwendeten Pixel auf dem LED Streifen oder Ring.
NeoPixelBrightness: Maximale Helligkeit der Pixel.

# Hinweis
Bis jetzt wurde diese Erwiterung nur mit der AIO+ getestet.
Es ist noch unklar, ob es mit den anderen Platinen möglich ist oder zu Problemen führt.
