# Implementierung: Getrennte Berechnung und Visualisierung

Auf Ihren Wunsch hin ("separate the visualization from the calculation") wurde die Implementierung in zwei unabhängige Algorithmen aufgeteilt. Dies gewährleistet, dass die Berechnung (Daten) immer korrekt ausgeführt wird und separat inspiziert werden kann, während die Visualisierung (Grafik) flexibel angepasst werden kann.

Zudem wurde umfangreicher **Debug-Output** hinzugefügt, den Sie in der FAnToM-Konsole (oder im Terminal) sehen können, um den Datenfluss zu überprüfen.

## 1. Algorithmus: Superquadric Generation (DataAlgorithm)

Dieser Algorithmus berechnet die Geometrie und die Eigenschaften der Glyphen und gibt sie als Standard-FAnToM-Datenobjekte aus.

- **Name:** `Aufgabe4-1/2 Superquadric Generation`
- **Typ:** `DataAlgorithm`
- **Input:** Tensor Field
- **Outputs:**
  - `Grid<3>` ("Glyph Mesh"): Das Gitter der Glyphen.
  - `Function<Color>` ("Color"): Farbfeld basierend auf Haupt-Eigenvektor.
  - `Function<Vector3>` ("Normals"): **Analytische Normalen** der Superquadrics (wichtig für korrekte Beleuchtung!).

**Debug-Output:**
- Gibt Anzahl der verarbeiteten Tensoren aus.
- Gibt Anzahl der generierten Vertices und Dreiecke aus.
- Meldet, wenn keine Tensoren gültig waren (zu klein).

## 2. Algorithmus: Superquadric Rendering (VisAlgorithm)

Dieser Algorithmus nimmt die Daten entgegen und rendert sie. Er ist speziell darauf ausgelegt, die analytischen Normalen zu verwenden, um das "Blob/Ball"-Problem zu lösen.
Die Implementierung folgt strikt den FAnToM-Richtlinien (`system.makePrimitive`, `setGraphics`).

- **Name:** `Aufgabe4-1/3 Superquadric Rendering`
- **Typ:** `VisAlgorithm`
- **Inputs:**
  - `Grid`: Verbinden mit "Glyph Mesh" aus Algo 1.
  - `Color` (optional): Verbinden mit "Color" aus Algo 1.
  - `Normals` (optional): Verbinden mit "Normals" aus Algo 1. **Wichtig für korrekte 3D-Form!**
- **Output:** Grafik im 3D-Fenster.

**Debug-Output:**
- Meldet empfangene Vertices, Normalen und Farben.
- Bestätigt das Laden der Shader und den Pfad.
- Meldet Erfolg beim Setzen der Grafik.

### Verwendung

1. **Load/VTK** (Tensor Field) → **Superquadric Generation** ("Tensor Field")
2. **Superquadric Generation** ("Glyph Mesh") → **Superquadric Rendering** ("Grid")
3. **Superquadric Generation** ("Color") → **Superquadric Rendering** ("Color")
4. **Superquadric Generation** ("Normals") → **Superquadric Rendering** ("Normals")

Durch diese Trennung können Sie auch debuggen: Verbinden Sie z.B. Algo 1 ("Glyph Mesh") mit einem Standard "Show Grid", um zu sehen, ob das Gitter überhaupt existiert, falls der Renderer Probleme macht.

## 3. Localized Flow Probe

Auch hier wurde die Berechnung von der Visualisierung getrennt, um maximale Flexibilität zu bieten.

### 3.1 Berechnung: Localized Flow Probe (DataAlgorithm)
Berechnet physikalische Eigenschaften (Divergenz, Rotation, Krümmung) an Sampling-Punkten.

- **Name:** `Aufgabe4-1/1 Localized Flow Probe`
- **Inputs:**
  - Vector Field
  - Step Size, Sample Count, Time
- **Outputs:**
  - `Probe Points` (PointSet)
  - `Velocity` (Function<Vector3>)
  - `Rotation` (Function<Vector3>)
  - `Divergence` (Function<double>)
  - `Curvature` (Function<double>)

### 3.2 Visualisierung: Flow Probe Rendering (VisAlgorithm)
Rendert Vektoren als 3D-Pfeile mit konfigurierbarer Farbe und Größe.

- **Name:** `Aufgabe4-1/1 Flow Probe Rendering`
- **Inputs:**
  - `Probe Points`: Verbinden mit "Probe Points" aus Algo 1.
  - `Vector Field`: Verbinden mit "Velocity" oder "Rotation".
  - `Scalar Field` (Optional): Verbinden mit "Divergence" oder "Curvature" zur Einfärbung.
- **Parameter:**
  - `Glyph Scale`: Skalierung der Pfeile.
  - `Arrow Head Size`: Relative Größe der Pfeilspitze.
  - `Normalize Vectors`: Wenn an, haben alle Pfeile gleiche Länge (nur Richtung).
  - `Scalar Max`: Wert für maximale Farbsättigung (wichtig für Scalar Coloring!).
  - `Default Color`: Farbe, falls kein Skalarfeld verbunden ist.
