# Aufgabe 4.1: Superquadric Tensor Glyphs & Localized Flow Probe - Übersicht

## Einleitung

Diese Wahlpflichtaufgabe umfasst die Implementierung fortgeschrittener Techniken zur Visualisierung von Vektor- und Tensorfeldern in FAnToM.

## Teilaufgaben

### 1. Localized Flow Probe

Erweiterung des klassischen "Hedgehogs" (Vektorpfeile) um zusätzliche Visualisierung von:
- **Divergenz**: Maß für Quellen/Senken im Vektorfeld
- **Rotation (Curl)**: Maß für Wirbel im Vektorfeld
- **Krümmung**: Lokale Krümmung der Stromlinien

### 2. Superquadric Tensor Glyphs

Implementierung einer Glyphen-Technik nach Kindlmann (2004) zur Visualisierung von Tensorfeldern:
- Abbildung der Tensorrichtungen und -stärken (Eigenwerte/-vektoren) auf eine einstellbare Geometrie
- Verwendung von Westin-Metriken zur Klassifikation der Tensorform
- Superquadric-Geometrie für kontinuierliche Formübergänge

## Projektstruktur

```
aufgabe4-1_src/
├── CMakeLists.txt
├── plugin1/
│   ├── FAnToM-Dependencies.txt
│   └── algos/
│       ├── LocalizedFlowProbe.cpp
│       └── SuperquadricTensorGlyphs.cpp
└── README

aufgabe4-1_docs/
├── Aufgabe4-1-Overview.md (dieses Dokument)
├── Aufgabe4-1-1-LocalizedFlowProbe.md
└── Aufgabe4-1-2-SuperquadricGlyphs.md
```

## Build & Installation

### Voraussetzungen

- FAnToM installiert (FANTOM_DIR muss gesetzt sein)
- CMake 3.20 oder höher
- C++ Compiler mit C++17 Support
- Eigen3 (für Eigenwert-Berechnung)

### Build-Schritte

```bash
cd aufgabe4-1_src
mkdir build
cd build
FANTOM_DIR=/path/to/FAnToM cmake ../
make
make install
```

### FAnToM starten

```bash
cd build
make run
```

## Verwendung in FAnToM

### Localized Flow Probe

1. Vektorfeld laden (z.B. mit `Load/VTK`)
2. Algorithmus `Aufgabe4-1/1 Localized Flow Probe` auswählen
3. Parameter einstellen:
   - `Vector Field`: Eingabe-Vektorfeld
   - `Glyph Scale`: Skalierung der Glyphen
   - `Show Divergence`, `Show Rotation`, `Show Curvature`: Visualisierungsoptionen
   - `Sample Count`: Anzahl der Sampling-Punkte
4. `Execute` ausführen
5. Outputs visualisieren:
   - `Glyph Positions`: Punktwolke der Glyphen-Positionen
   - `Vector Arrows`: Vektorpfeile (Hedgehogs)
   - `Rotation Arrows`: Rotations-Pfeile

### Superquadric Tensor Glyphs

1. Tensorfeld laden (z.B. mit `Load/VTK`)
2. Algorithmus `Aufgabe4-1/2 Superquadric Tensor Glyphs` auswählen
3. Parameter einstellen:
   - `Tensor Field`: Eingabe-Tensorfeld
   - `Glyph Scale`: Skalierung der Glyphen
   - `Sharpness Parameter γ`: Schärfeparameter (Standard: 3.0)
   - `Resolution Theta`, `Resolution Phi`: Auflösung der Oberfläche
   - `Sample Count`: Anzahl der Sampling-Punkte
4. `Execute` ausführen
5. Output `Glyph Mesh` mit `Mesh/Show Mesh` visualisieren

## Testdaten

### Für Localized Flow Probe

- `streamTest1.vtk`, `streamTest2.vtk` (Vektorfelder)
- `JetVelocity.vtk` (aus Hauptaufgaben)

### Für Superquadric Tensor Glyphs

- `JetStrainTensors.vtk` (aus Hauptaufgaben)
- `BrainTensors.vtk` (aus Hauptaufgaben)
- `2D-tensors.vtk` (aus Hauptaufgaben)

## Weitere Informationen

Siehe detaillierte Dokumentation:
- [Localized Flow Probe](Aufgabe4-1-1-LocalizedFlowProbe.md)
- [Superquadric Tensor Glyphs](Aufgabe4-1-2-SuperquadricGlyphs.md)
