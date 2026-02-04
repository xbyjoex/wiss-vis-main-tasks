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

1. **Daten laden**: `Load/VTK` → z. B. `JetVelocity.vtk` oder anderes 3D-Vektorfeld.

2. **Berechnung**: `Aufgabe4-1/1 Localized Flow Probe`
   - `Vector Field`: Geladenes Feld verbinden
   - **Sample Count: 2 oder 3** (weniger = klare Pfeile wie im Paper; 5+ = sehr dicht)
   - `Execute`

3. **Visualisierung**: `Aufgabe4-1/1 Flow Probe Rendering`
   - `Probe Points` + `Velocity` (Pflicht); optional `Acceleration`, `Gradient`, `Divergence`, `Curvature` vom ersten Algorithmus verbinden
   - Ausgabe **Flow Probes** in die Szene ziehen (Auge)

**Was Sie sehen (Paper-Primitiven):**
- **Graue Linien:** Schaft (Bogen des Schmiegkreises), Pfeilspitze (8 Strahlen), Shear-Ring am Fuß
- **Rote Flächen:** Divergenz > 0 (Quelle); **blaue Flächen:** Divergenz < 0 (Senke)
- **Flache Scheibe an der Spitze:** Beschleunigungs-Membran (Wölbung = Beschleunigung in Strömungsrichtung)
- **Gewölbte Linse am Fuß:** Oskulierendes Paraboloid (Konvergenz/Divergenz)
- **Röhre mit Streifen:** Tube mit Torsion (Candy stripes)

**Wenn zu viel überlappt:** Sample Count auf 2 setzen; **Glyph Scale** verkleinern (z. B. 0.3–0.5); **Show Tube**, **Show Membrane**, **Show Lens** einzeln aus- und anstellen, um nur „Pfeil“ (Schaft + Spitze + Ring) zu sehen.

**Welcher Strich gehört zu welchem Pfeil?** **Color by Probe ID** (Standard: an) färbt jede Probe in einer eigenen Farbe: Schaft, Spitze, Ring, Tube, Membran und Linse einer Probe haben dieselbe Farbe. So erkennt man sofort, welche Linien zu einem Pfeil gehören. Bei **Color by Probe ID** aus wird wieder nach Divergenz (rot/blau) eingefärbt.

#### Test 2: Superquadric Tensor Glyphs

1. **Daten laden**: `Load/VTK` → 
   ```
   /Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/Wissenschaftliche Visualisierung Hauptaufgaben/TestDataHauptaufgaben/JetStrainTensors.vtk
   ```

2. **Algorithmus**: `Aufgabe4-1/2 Superquadric Tensor Glyphs`
   - `Tensor Field`: Geladenes Feld verbinden
   - `Sample Count`: 3
   - `Execute`

3. **Visualisieren**: `Aufgabe4-1/3 Superquadric Rendering` → Inputs **Grid**, **Color**, **Normals** mit den Ausgaben von Schritt 2 verbinden; Ausgabe **Glyphs** in die Szene ziehen.

**Parameter Superquadric (Kindlmann 2004):**

| Parameter | Bedeutung | Standard | Empfehlung |
|-----------|------------|----------|------------|
| **Tensor Field** | Eingabe (symmetrisches Tensorfeld) | – | Pflicht |
| **Glyph Scale** | Globale Größe der Glyphen | 1.0 | 0.3–1.0 je nach Datensatz; bei Überlappung verkleinern |
| **Sharpness Parameter γ** | Kantenschärfe (1 = weich/ellipsoid, 2–3 = Paper) | 2.5 | 2–3 für typische Superquadriken |
| **Use Kindlmann Shape** | Form aus Anisotropie (α/β); aus = runde Querschnitte | true | An für Paper-Treue |
| **Resolution Theta / Phi** | Mesh-Auflösung pro Glyphe | 20 | 15–30 (höher = glatter, mehr Dreiecke) |
| **Sample Count** | Stützstellen pro Achse (Anzahl Glyphen ∝ (N+1)³) | 10 | 2–5 für Übersicht; 3–5 für JetStrainTensors |
| **Time** | Zeit für zeitabhängige Felder | 0.0 | Bei Bedarf anpassen |
| **Normalize to cell** | Glyphen so skalieren, dass sie in ihre Zelle passen (kein Überlapp) | false | An für klare Gitterdarstellung wie in Referenzbildern; aus = Größe ∝ Tensorstärke (Paper) |
| **Cell fill** | Anteil der Zellgröße bei Normalisierung (0.5–1.0) | 0.8 | 0.7–0.9 für leichten Abstand; 1.0 = Zelle voll genutzt |

**Typische Sets:** JetStrainTensors: Sample Count 3–5, Glyph Scale 0.5–1.0, γ = 2.5. DTI/Brain: Sample Count 2–4, Glyph Scale 0.3–0.5.

---

## Outputs (was wohin verbinden)

### Localized Flow Probe

| Algorithmus | Output | Typ | Wohin verbinden |
|-------------|--------|-----|------------------|
| **Aufgabe4-1/1 Localized Flow Probe** | Probe Points | PointSet | Flow Probe Rendering: „Probe Points“ (Pflicht) |
| | Velocity | Function Vector3 | Flow Probe Rendering: „Velocity“ (Pflicht) |
| | Acceleration | Function Vector3 | Flow Probe Rendering: „Acceleration“ (optional) |
| | Gradient | Function Tensor 3×3 | Flow Probe Rendering: „Gradient“ (optional, Ring/Scherung) |
| | Divergence | Function double | Flow Probe Rendering: „Divergence“ (optional, Farbe/Linse) |
| | Curvature | Function Vector3 | Flow Probe Rendering: „Curvature“ (optional) |
| **Aufgabe4-1/1 Flow Probe Rendering** | Flow Probes | Grafik | In die 3D-Szene ziehen (Auge) |

### Superquadric Tensor Glyphs

| Algorithmus | Output | Typ | Wohin verbinden |
|-------------|--------|-----|------------------|
| **Aufgabe4-1/2 Superquadric Generation** | Glyph Mesh | Grid | Superquadric Rendering: „Grid“ (Pflicht) |
| | Color | Function Color | Superquadric Rendering: „Color“ (optional) |
| | Normals | Function Vector3 | Superquadric Rendering: „Normals“ (empfohlen für Beleuchtung) |
| **Aufgabe4-1/3 Superquadric Rendering** | Glyphs | Grafik | In die 3D-Szene ziehen (Auge) |

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
