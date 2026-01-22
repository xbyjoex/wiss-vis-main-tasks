# Quick Start - Aufgabe 4.1

## Schnellstart (3 Schritte)

### 1. Build ausführen

```bash
cd /Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/aufgabe4-1_src
./build.sh
```

**Oder manuell**:
```bash
cd aufgabe4-1_src
mkdir -p build && cd build
cmake -D FANTOM_DIR="/Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/FAnToM_MacOS26" ../
make
make install
```

### 2. FAnToM starten

```bash
cd build
make run
```

### 3. Testen

#### Test 1: Localized Flow Probe

1. **Daten laden**: `Load/VTK` → 
   ```
   /Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/Wissenschaftliche Visualisierung Hauptaufgaben/TestDataHauptaufgaben/JetVelocity.vtk
   ```

2. **Algorithmus**: `Aufgabe4-1/1 Localized Flow Probe`
   - `Vector Field`: Geladenes Feld verbinden
   - `Sample Count`: 5
   - `Execute`

3. **Visualisieren**: `Line Set/Show Line Set` → `Vector Arrows`

#### Test 2: Superquadric Tensor Glyphs

1. **Daten laden**: `Load/VTK` → 
   ```
   /Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/Wissenschaftliche Visualisierung Hauptaufgaben/TestDataHauptaufgaben/JetStrainTensors.vtk
   ```

2. **Algorithmus**: `Aufgabe4-1/2 Superquadric Tensor Glyphs`
   - `Tensor Field`: Geladenes Feld verbinden
   - `Sample Count`: 3
   - `Execute`

3. **Visualisieren**: `Mesh/Show Mesh` → `Glyph Mesh`

## Verfügbare Testdaten

### Vektorfelder (für Localized Flow Probe):
- `JetVelocity.vtk` ⭐ (empfohlen)
- `wind.vtk`
- `windBorders.vtk`
- `gbk_velocity.vtk`
- `streamTest1.vtk`, `streamTest2.vtk`

### Tensorfelder (für Superquadric Glyphs):
- `JetStrainTensors.vtk` ⭐ (empfohlen)
- `BrainTensors.vtk`
- `2D-tensors.vtk`

**Pfad zu allen Testdaten**:
```
/Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/Wissenschaftliche Visualisierung Hauptaufgaben/TestDataHauptaufgaben/
```

## Häufige Probleme

### Plugin nicht gefunden?
- FAnToM neu starten
- Prüfen: `ls build/lib/fantom-plugins/` sollte Dateien enthalten

### Kompilierfehler?
- Prüfen Sie `FANTOM_DIR` ist korrekt gesetzt
- Prüfen Sie ob Eigen3 verfügbar ist

### Keine Ausgabe?
- Prüfen Sie ob Eingabefeld korrekt verbunden ist
- Reduzieren Sie `Sample Count`

## Detaillierte Anleitung

Siehe [BUILD_AND_TEST.md](BUILD_AND_TEST.md) für vollständige Anleitung.
