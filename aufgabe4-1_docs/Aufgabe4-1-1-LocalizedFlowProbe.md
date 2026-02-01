# Aufgabe 4.1.1: Localized Flow Probe

## Ziel

Erweiterung der klassischen "Hedgehogs" (Vektorpfeile) um zusätzliche Visualisierung von:
- **Divergenz**: `div(v) = ∂v_x/∂x + ∂v_y/∂y + ∂v_z/∂z`
- **Rotation (Curl)**: `rot(v) = (∂v_z/∂y - ∂v_y/∂z, ∂v_x/∂z - ∂v_z/∂x, ∂v_y/∂x - ∂v_x/∂y)`
- **Krümmung**: Lokale Krümmung der Stromlinien `κ = ||dT/ds||` wobei `T = v/||v||`

## Mathematische Grundlagen

### Divergenz

Die Divergenz eines Vektorfeldes `v = (v_x, v_y, v_z)` ist ein Skalar, der die "Quellstärke" an einem Punkt beschreibt:

```
div(v) = ∂v_x/∂x + ∂v_y/∂y + ∂v_z/∂z
```

- **div(v) > 0**: Quelle (Feld divergiert)
- **div(v) < 0**: Senke (Feld konvergiert)
- **div(v) = 0**: Quellenfrei (z.B. inkompressible Strömung)

### Rotation (Curl)

Die Rotation beschreibt die Wirbelstärke eines Vektorfeldes:

```
rot(v) = (∂v_z/∂y - ∂v_y/∂z, ∂v_x/∂z - ∂v_z/∂x, ∂v_y/∂x - ∂v_x/∂y)
```

- **||rot(v)|| > 0**: Wirbel vorhanden
- **||rot(v)|| = 0**: Wirbelfrei (Potentialströmung)

### Krümmung

Die Krümmung einer Stromlinie beschreibt, wie stark sich die Richtung entlang der Linie ändert:

```
κ = ||dT/ds||
```

wobei `T = v/||v||` der normierte Tangentenvektor ist und `s` der Bogenlängenparameter.

## Implementierung

### Datei

`aufgabe4-1_src/plugin1/algos/LocalizedFlowProbe.cpp`

### Algorithmus-Struktur

**Basisklasse**: `DataAlgorithm`

**Optionen**:
- `Field<3, Vector3>` (Pflicht): Eingabe-Vektorfeld
- `Glyph Scale`: Skalierung der Glyphen (Standard: 1.0)
- `Show Divergence`: Divergenz visualisieren (Standard: true)
- `Show Rotation`: Rotation visualisieren (Standard: true)
- `Show Curvature`: Krümmung visualisieren (Standard: true)
- `Divergence Scale`: Skalierung für Divergenz-Darstellung (Standard: 1.0)
- `Rotation Scale`: Skalierung für Rotation-Darstellung (Standard: 1.0)
- `Curvature Scale`: Skalierung für Krümmungs-Darstellung (Standard: 1.0)
- `Step Size`: Schrittweite für numerische Differentiation (Standard: 1e-4)
- `Sample Count`: Anzahl der Sampling-Punkte pro Dimension (Standard: 10)
- `Time`: Zeitstempel für zeitabhängige Felder (Standard: 0.0)

**Ausgabe**:
- `Glyph Positions`: `PointSet<3>` mit Positionen der Glyphen
- `Vector Arrows`: `LineSet<3>` mit Vektorpfeilen (Hedgehogs)
- `Rotation Arrows`: `LineSet<3>` mit Rotations-Pfeilen

### Numerische Differentiation

Alle Ableitungen werden mit zentralen Differenzenquotienten berechnet:

```cpp
// Beispiel: ∂v_x/∂x
double dvx_dx = (v_xp[0] - v_xm[0]) / (2.0 * h);
```

wobei `v_xp` und `v_xm` die Feldwerte an den Punkten `p + (h,0,0)` und `p - (h,0,0)` sind.

### Krümmungs-Berechnung

Die Krümmung wird numerisch durch Approximation von `dT/ds` berechnet:

1. Berechne normierten Tangentenvektor `T = v/||v||` am Punkt `p`
2. Bewege kleine Schritte entlang der Stromlinie: `p ± h*T`
3. Berechne `T` an diesen Punkten
4. Approximiere `dT/ds ≈ (T_forward - T_backward) / (2*h)`
5. Krümmung: `κ = ||dT/ds||`

## Verwendung

### Pipeline-Aufbau

1. **Daten laden**: `Load/VTK` → Vektorfeld laden (z.B. `streamTest1.vtk`)
2. **Algorithmus ausführen**: `Aufgabe4-1/1 Localized Flow Probe`
   - Input: `Vector Field` → Vektorfeld verbinden
   - Parameter anpassen (siehe unten)
   - `Execute` ausführen
3. **Visualisierung**:
   - `Point Set/Show Point Set` → `Glyph Positions` verbinden
   - `Line Set/Show Line Set` → `Vector Arrows` oder `Rotation Arrows` verbinden

### Parameterempfehlungen

#### streamTest1.vtk (Wirbelfeld)

- `Sample Count`: 5-10 (je nach Rechenleistung)
- `Glyph Scale`: 0.5-1.0
- `Step Size`: 1e-4 (Standard)
- `Show Rotation`: true (Wirbel sichtbar)
- `Rotation Scale`: 1.0-2.0

#### streamTest2.vtk (Kanalströmung)

- `Sample Count`: 8-12
- `Glyph Scale`: 0.3-0.5
- `Step Size`: 1e-4
- `Show Divergence`: true (Quellen/Senken sichtbar)

## Wissenschaftliche Aspekte

### Numerische Genauigkeit

- **Zentrale Differenzen**: O(h²) Genauigkeit
- **Schrittweite**: Zu klein → numerische Instabilität, zu groß → grobe Approximation
- **Empfehlung**: `Step Size` sollte proportional zur Gitterauflösung sein

### Visualisierungsstrategien

1. **Divergenz**: 
   - Farbcodierung der Glyphen
   - Zusätzliche Geometrie (z.B. Kugeln mit Radius ∝ |div|)

2. **Rotation**:
   - Zusätzliche Pfeile in Rotationsrichtung
   - Farbcodierung nach Rotationsstärke

3. **Krümmung**:
   - Farbcodierung
   - Bogen-Glyphen entlang der Stromlinie

### Anwendungsgebiete

- **Strömungsmechanik**: Analyse von Quellen, Senken und Wirbeln
- **Magnetfelder**: Visualisierung von Feldlinien und Wirbeln
- **Geschwindigkeitsfelder**: Identifikation von kritischen Punkten

## Offizielle Literatur und Parameterwahl

### Referenzpaper

- **W. C. de Leeuw, J. J. van Wijk:** *A probe for local flow field visualization*, IEEE Visualization, 1993 (IEEE Xplore: 398849).
- Die Glyphe (gebogene Röhre + Ring für Scherung/Divergenz) folgt diesem Konzept; die genaue geometrische Parametrisierung ist im frei zugänglichen Abstract/Übersichten nicht detailliert beschrieben.

### Was das Paper nicht vorgibt

- Im Volltext (IEEE Paywall) werden konkrete Formeln für **Rohrlänge**, **Ringradius** oder **Skalierung** in den zugänglichen Zusammenfassungen und Zitaten nicht genannt.
- Verwandte Arbeit (Post et al., *Future Generation Computer Systems* 15, 1999) nutzt bei Stromlinien-Rohren z.B. **Radius ∝ 1/√|v|**, damit starke Strömung dünnere Rohre bekommt; für die lokale Probe-Glyphe gibt es dort keine expliziten Parameter.

### Praktische Parameterwahl (Implementierung)

Ohne feste Vorgaben aus dem Paper werden die Parameter heuristisch und datenabhängig gewählt:

| Parameter | Rolle | Empfehlung |
|-----------|--------|-------------|
| **Glyph Scale** | Globale Größe der Glyphe im Weltkoordinatensystem | Ca. 0,1–0,5 × typische Domänenausdehnung; so wählen, dass Glyphen weder verschwinden noch sich stark überlappen. |
| **Tube Length** | Relative Länge der Stromlinien-Röhre (0,1–1) | 0,2–0,35: kürzere Röhren reduzieren Überlagerung; größere Werte zeigen mehr Verlauf. |
| **Ring Size** | Radius des Basis-Rings (Scherung/Divergenz) relativ zur Glyphen-Größe | 0,2–0,4: Ring gut sichtbar, aber nicht dominierend. |
| **Line Width** | Linienstärke in Pixel | 2–4 für bessere Sichtbarkeit. |
| **Sample Count** (Berechnung) | Anzahl Stützstellen pro Raumrichtung | 5–10: weniger Glyphen = übersichtlicher; mehr = feinere Abdeckung. |
| **Step Size** (Berechnung) | Schrittweite für numerische Ableitungen | In der Größenordnung der Gitterweite (z. B. 1e-4); nicht zu groß (ungenau), nicht zu klein (Rundungsfehler). |

**Grundprinzip:** Glyphen-Größe und -Dichte so wählen, dass die lokale Struktur (Richtung, Krümmung, Ring-Deformation) erkennbar bleibt und die Szene nicht überladen wirkt. Die Implementierung skaliert die Röhrenlänge bereits mit der lokalen Geschwindigkeitsmagnitude (stärkere Strömung = längere Röhre).

## Hinweise

- Die Implementierung verwendet uniformes Grid-Sampling. Für adaptives Sampling könnte man die Feldstärke als Dichte-Indikator verwenden.
- Divergenz und Krümmung werden berechnet; Divergenz steuert die Farbgebung, der Gradient (Jakobi-Matrix) die Ring-Verzerrung (Scherung).
- Die Rotation wird nicht mehr als separate Pfeile gezeichnet; die gebogene Röhre und der Ring kodieren Richtung, Krümmung und lokale Deformation.
