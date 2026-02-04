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
Berechnet Geschwindigkeit, Jacobi-Matrix, Beschleunigung, Divergenz und Krümmungsvektor (de Leeuw & van Wijk).

- **Name:** `Aufgabe4-1/1 Localized Flow Probe`
- **Inputs:** Vector Field, Step Size, Sample Count, Time
- **Outputs:**
  - `Probe Points` (PointSet)
  - `Velocity` (Function<Vector3>)
  - `Acceleration` (Function<Vector3>)
  - `Gradient` (Function<Tensor<double,3,3>>)
  - `Divergence` (Function<double>)
  - `Curvature` (Function<Vector3>) – Krümmungsvektor für Schmiegkreis/Frenet

### 3.2 Visualisierung: Flow Probe Rendering (VisAlgorithm)
Rendert die Paper-Primitiven: Schaft als Bogen des Schmiegkreises, Tube mit Torsion-Streifen, Beschleunigungs-Membran, Divergenz-Paraboloid (Linse), Shear-Ring, Pfeilspitze. Ein Grafik-Output „Flow Probes“ (Compound aus Linien und Dreiecken).

- **Inputs:** Probe Points, Velocity (Pflicht); optional Acceleration, Gradient, Divergence, Curvature
- **Parameter:** Glyph Scale, Tube Length, Ring Size, Line Width
