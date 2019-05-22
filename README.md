# GPU Programmierung

## Build

Clone this repositiory:

```bash
git clone git@git.ios.htwg-konstanz.de:gpu/swarm.git
cd swarm
git submodule update --init --recursive
```

Execute `./run.sh` to build and run the project.

## Themen

 + Schwarm mit vielen Mitgliedern

### OpenCV

 + Schwarm:
   + Gibt aktuelle Richtung/Ziel des Schwarmes vor
   + evtl. noch andere Parameter, wie allgemeine Geschwindigkeit,
     Sprich eher langsam oder schnell
 + Mitglieder:
   + jedes einzelne Mitglied versucht individuell das aktuelle
     Schwarmziel zu erreichen, orientiert sich aber an seinen 
     nächsten Nachbarn
   + Mitglieder des selben Schwarmes sollten nicht miteinander
     kollidieren
 + äußere Einflüsse (wirken auf jedes Mitglied):
   + Wind (global, stärke und richtung Variable)

### OpenGL

 + Mitglieder werden durch einfache Meshes dargestellt, die die
   aktuelle Richtung veranschaulichen
 + Meshes werden animiert
 + "schöne" Umgebung, durch die der Schwarm fliegen kann

