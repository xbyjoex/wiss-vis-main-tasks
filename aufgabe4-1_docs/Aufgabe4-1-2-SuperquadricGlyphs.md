# Aufgabe 4.1.2: Superquadric Tensor Glyphs

## Ziel

Implementierung von Superquadric Tensor Glyphs nach Kindlmann (2004) zur Visualisierung von Tensorfeldern.

## Referenz

**Kindlmann, G. (2004)**: "Superquadric Tensor Glyphs" - Visualisierungstechnik zur Darstellung von Diffusion Tensor Imaging (DTI) Daten und anderen symmetrischen Tensorfeldern.

## Mathematische Grundlagen

### Eigenwert/Eigenvektor-Zerlegung

Ein symmetrischer Tensor `T` kann zerlegt werden in:

```
T = λ₁ v₁ v₁ᵀ + λ₂ v₂ v₂ᵀ + λ₃ v₃ v₃ᵀ
```

wobei:
- `λ₁ ≥ λ₂ ≥ λ₃ ≥ 0` die sortierten Eigenwerte sind
- `v₁, v₂, v₃` die zugehörigen orthonormalen Eigenvektoren sind

### Westin-Metriken

Basierend auf den sortierten Eigenwerten werden drei Formfaktoren definiert:

**Linearität** (`c_l`):
```
c_l = (λ₁ - λ₂) / (λ₁ + λ₂ + λ₃)
```

**Planarität** (`c_p`):
```
c_p = 2(λ₂ - λ₃) / (λ₁ + λ₂ + λ₃)
```

**Sphärizität** (`c_s`):
```
c_s = 3λ₃ / (λ₁ + λ₂ + λ₃)
```

Eigenschaften:
- `c_l + c_p + c_s = 1`
- `0 ≤ c_l, c_p, c_s ≤ 1`
- Hohe Linearität → stäbchenförmige Glyphe
- Hohe Planarität → scheibenförmige Glyphe
- Hohe Sphärizität → kugelförmige Glyphe

### Formparameter α und β

Unter Verwendung eines Schärfeparameters `γ` (Vorschlag: `γ = 3`) werden die Exponenten für die Superquadric-Geometrie bestimmt:

**Fall `c_l ≥ c_p` (Linearer Typ)**:
```
α = (1 - c_p)^γ
β = (1 - c_l)^γ
```

**Fall `c_p > c_l` (Planarer Typ)**:
```
α = (1 - c_l)^γ
β = (1 - c_p)^γ
```

### Superquadric-Oberfläche

Die Oberflächenpunkte `S(θ, φ)` der Glyphe im Einheitsraum werden wie folgt berechnet:

```
x = sgn(cos φ) |cos φ|^α · sgn(cos θ) |cos θ|^β
y = sgn(cos φ) |cos φ|^α · sgn(sin θ) |sin θ|^β
z = sgn(sin φ) |sin φ|^α
```

wobei:
- `θ ∈ [-π, π]` (Azimut)
- `φ ∈ [-π/2, π/2]` (Elevation)
- `sgn(x) = 1` wenn `x ≥ 0`, sonst `-1`

### Transformation in Datenraum

Die finale Position `P_final` eines Oberflächenpunktes ergibt sich durch:

1. **Skalierung** mit den Eigenwerten:
   ```
   P_scaled = (λ₁ x, λ₂ y, λ₃ z)
   ```

2. **Rotation** in das Eigenvektor-Koordinatensystem:
   ```
   P_rotated = x·λ₁·v₁ + y·λ₂·v₂ + z·λ₃·v₃
   ```

3. **Translation** zum Glyphen-Zentrum:
   ```
   P_final = center + P_rotated
   ```

## Implementierung

### Datei

`aufgabe4-1_src/plugin1/algos/SuperquadricTensorGlyphs.cpp`

### Algorithmus-Struktur

**Basisklasse**: `DataAlgorithm`

**Optionen**:
- `Field<3, Tensor<double, 3, 3>>` (Pflicht): Eingabe-Tensorfeld
- `Glyph Scale`: Skalierung der Glyphen (Standard: 1.0)
- `Sharpness Parameter γ`: Schärfeparameter (Standard: 3.0)
- `Resolution Theta`: Anzahl der Schritte in θ-Richtung (Standard: 20)
- `Resolution Phi`: Anzahl der Schritte in φ-Richtung (Standard: 20)
- `Sample Count`: Anzahl der Sampling-Punkte pro Dimension (Standard: 5)
- `Time`: Zeitstempel für zeitabhängige Felder (Standard: 0.0)

**Ausgabe**:
- `Glyph Mesh`: `UnstructuredGrid<3>` mit triangulierten Superquadric-Oberflächen

### Eigenwert/Eigenvektor-Berechnung

Verwendung von `fantom-plugins/utils/math/eigenvalues.hpp`:

```cpp
auto eigensystem = fantom::math::getEigensystem<3>(tensor);
```

Die Funktion liefert:
- Sortierte Eigenwerte (absteigend nach Norm, reelle zuerst)
- Zugehörige Eigenvektoren (normalisiert)

**Hinweis**: Eigenwerte können komplex sein. Die Implementierung extrahiert den Realteil und verwendet den Betrag als Fallback.

### Mesh-Generierung

1. **Parameterraum-Sampling**: Generiere Gitter von (θ, φ) Werten
2. **Oberflächenpunkte**: Berechne Superquadric-Punkte im Einheitsraum
3. **Transformation**: Transformiere alle Punkte in Datenraum
4. **Triangulierung**: Erstelle Dreiecke aus benachbarten Punkten (Quads → 2 Triangles)
5. **Mesh-Erstellung**: Verwende `DomainFactory::makeUnstructuredGrid<3>()`

## Verwendung

### Pipeline-Aufbau

1. **Daten laden**: `Load/VTK` → Tensorfeld laden (z.B. `JetStrainTensors.vtk`)
2. **Algorithmus ausführen**: `Aufgabe4-1/2 Superquadric Tensor Glyphs`
   - Input: `Tensor Field` → Tensorfeld verbinden
   - Parameter anpassen (siehe unten)
   - `Execute` ausführen
3. **Visualisierung**: `Mesh/Show Mesh` → `Glyph Mesh` verbinden

### Parameterempfehlungen

#### JetStrainTensors.vtk

- `Sample Count`: 3-5 (weniger Glyphen für bessere Übersicht)
- `Glyph Scale`: 0.5-1.0
- `Resolution Theta`: 20-30
- `Resolution Phi`: 20-30
- `Sharpness Parameter γ`: 3.0 (Standard)

#### BrainTensors.vtk

- `Sample Count`: 2-4 (DTI-Daten sind oft dicht)
- `Glyph Scale`: 0.3-0.5
- `Resolution Theta`: 15-20
- `Resolution Phi`: 15-20

## Wissenschaftliche Aspekte

### Numerische Stabilität

- **Eigenwert-Berechnung**: Verwendet Eigen3-Bibliothek (robust)
- **Komplexe Eigenwerte**: Werden erkannt und behandelt (Realteil oder Betrag)
- **Orthonormalisierung**: Gram-Schmidt-Verfahren für Eigenvektoren

### Visualisierungsqualität

- **Auflösung**: Höhere `Resolution` → glattere Oberflächen, aber mehr Dreiecke
- **Sampling-Dichte**: `Sample Count` bestimmt Anzahl der Glyphen
- **Skalierung**: `Glyph Scale` sollte proportional zur Feldstärke sein

### Forminterpretation

- **Lineare Tensoren** (`c_l` hoch): Stäbchenförmige Glyphen → Hauptrichtung dominant
- **Planare Tensoren** (`c_p` hoch): Scheibenförmige Glyphen → zwei Hauptrichtungen
- **Sphärische Tensoren** (`c_s` hoch): Kugelförmige Glyphen → isotrop

### Anwendungsgebiete

- **Diffusion Tensor Imaging (DTI)**: Visualisierung von Nervenfaserbahnen
- **Deformationstensoren**: Analyse von Materialverformungen
- **Spannungstensoren**: Visualisierung von mechanischen Spannungen
- **Strain Tensors**: Analyse von Verzerrungen in Strömungen

## Hinweise

- Die Implementierung erwartet symmetrische Tensoren. Für nicht-symmetrische Tensoren sollte der symmetrische Anteil extrahiert werden.
- Bei sehr kleinen Eigenwerten können numerische Probleme auftreten. Die Implementierung verwendet Schwellenwerte.
- Die Triangulierung verwendet einfache Quads→Triangles Konvertierung. Für bessere Qualität könnte man adaptive Subdivision verwenden.

## Referenzen

- Kindlmann, G. (2004): "Superquadric Tensor Glyphs"
- Westin, C.F. et al. (2002): "Geometric measures for diffusion tensor MRI analysis"
- Basser, P.J. et al. (1994): "Estimation of the effective self-diffusion tensor from the NMR spin echo"
