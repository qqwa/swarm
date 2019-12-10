# GPU Programmierung

## Build

Clone this repositiory:

```bash
git clone git@github.com:qqwa/swarm.git
cd swarm
git submodule update --init --recursive
```

Dependencies: C++ Compiler, CMake, GLEW, OpenGL and OpenCL should be installed.

By default the CPU is used to update the position of the swarm members, change
the key `use_cpu` to `false` in `cfg/default.toml` to use the GPU instead.

Execute `./run.sh` to build and run the project.

## Performance

System used for measurements: AMD Ryzen 7 1700X and RX Vega 56

Count      |perf_cpu   |perf_gpu   |perf_cpu_inc |perf_gpu_inc
-----------|-----------|-----------|-------------|-------------
1,000      |3.06ms     |2.47ms     |0.37ms       |0.52ms
10,000     |262.07ms   |20.57ms    |3.83ms       |0.65ms
100,000    |25785.15ms |1028.06ms  |34.36ms      |9.26ms
1,000,000  |-          |-          |363.67ms     |87.91ms

 + **perf_cpu** Implementation with CPU
 + **perf_gpu** Implementation with GPU
 + **perf_cpu_inc** Implementation with CPU, neighbours get updated incremental, so that it isn't o(n^2) anymore.
 + **perf_gpu_inc** Implementation with GPU, neighbours get updated incremental, so that it isn't o(n^2) anymore.

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

