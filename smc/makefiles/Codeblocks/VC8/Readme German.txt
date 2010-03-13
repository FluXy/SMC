Konfiguration Codeblocks 1.0, letzte Änderung 21.11.2006:

x Es muss noch Visual C++ Express geladen werden.


x Sowie das SDK für Windows Applikationen.


x Dann werden die VC6 - Development-Versionen von:

SDL
SDL_image
SDL_mixer
SDL_ttf

benötigt.

Die include & lib müssen in das entsprechende Verzeichnis beim Visual C++ kopiert werden.


x Ausserdem das SDK von CEGUI:

CEGUI_SDK 0.5.0-vc8

Die include & lib am besten in ein eigenes Verzeichnis entpacken (Übersichtlichkeit)

x In CodeBlocks muss nun alles eingestellt werden, es müssen die Compiler - Optionen (wenn nicht schon geschehen)

/EHs
/EHa
/MD

aktiviert werden. 

Ausserdem muss man die Sourcen manuell erst entfernen und dann erneut hinzufügen, da jeder andere Pfade auf der Festplatte zum auschecken nutzt.


x Nun muss der Compiler noch auf den 

Microsoft Visual C++ 2005

umgestellt werden. (wenn nicht schon geschehen)


x Nachdem hier nun noch die include & lib für SDL, CEGUI sowie das Windows SDK angegeben wurden, kann man SMC starten.

x TIPP: Am besten unter Settings / Compiler and Debugger einstellen, dann gilt es für alle Projekte ab dann. (Deswegen ist in dem Projekt auch nichts eingetragen)


________________

Warum Microsoft und kein MinGW nehmen?
- Das Problem ist, das CEGUI mit MinGW nicht kompatibel ist. Die Applikation lässt sich gar nicht linken.