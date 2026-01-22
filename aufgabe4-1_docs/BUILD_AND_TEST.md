# Build- und Test-Anleitung für Aufgabe 4.1

## Schritt 1: Build vorbereiten

### 1.1 FAnToM-Verzeichnis finden

Zuerst müssen Sie den Pfad zu Ihrer FAnToM-Installation finden:

```bash
# Typischerweise:
FANTOM_DIR="/Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/FAnToM_MacOS26"
```

### 1.2 Build-Verzeichnis erstellen

```bash
cd /Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/aufgabe4-1_src
mkdir -p build
cd build
```

## Schritt 2: CMake konfigurieren

### 2.1 CMake ausführen

**WICHTIG**: Verwenden Sie `-D FANTOM_DIR=...` als CMake-Variable, nicht nur als Umgebungsvariable!

```bash
cmake -D FANTOM_DIR="/Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/FAnToM_MacOS26" ../
```

**Oder als Umgebungsvariable + CMake-Variable (beide setzen)**:
```bash
FANTOM_DIR="/Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/FAnToM_MacOS26" cmake -D FANTOM_DIR="/Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/FAnToM_MacOS26" ../
```

**Wichtig**: Ersetzen Sie den Pfad mit Ihrem tatsächlichen FAnToM-Verzeichnis!

### 2.2 Erfolgreiche Konfiguration prüfen

Sie sollten sehen:
```
-- Configuring done
-- Generating done
-- Build files have been written to: ...
```

## Schritt 3: Kompilieren

```bash
make
```

**Erwartete Ausgabe**:
- Keine Fehler
- Plugin wird kompiliert: `lib/fantom-plugins/aufgabe4-1_plugin1.*`

## Schritt 4: Installation

```bash
make install
```

Dies installiert das Plugin in Ihr FAnToM-Verzeichnis.

## Schritt 5: FAnToM starten

### Option A: Mit make run (empfohlen)

```bash
cd build
make run
```

### Option B: Manuell

```bash
/Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/FAnToM_MacOS26/bin/fantom -P build/lib/fantom-plugins
```

## Schritt 6: Testdaten vorbereiten

Die Testdaten befinden sich in:
```
Wissenschaftliche Visualisierung Hauptaufgaben/TestDataHauptaufgaben/
```

### Verfügbare Testdaten

#### Für Localized Flow Probe (Vektorfelder):
- `JetVelocity.vtk` - Geschwindigkeitsfeld eines Jets
- `wind.vtk` - Windfeld
- `windBorders.vtk` - Windfeld mit Grenzen

#### Für Superquadric Tensor Glyphs (Tensorfelder):
- `JetStrainTensors.vtk` - Deformationstensoren eines Jets
- `BrainTensors.vtk` - DTI-Tensoren (falls vorhanden)
- `2D-tensors.vtk` - 2D Tensorfeld

## Schritt 7: Testing in FAnToM

### Test 1: Localized Flow Probe mit JetVelocity.vtk

#### 7.1 Daten laden

1. In FAnToM: **Algorithmus auswählen** → `Load/VTK`
2. **File Path** eingeben:
   ```
   /Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/Wissenschaftliche Visualisierung Hauptaufgaben/TestDataHauptaufgaben/JetVelocity.vtk
   ```
3. **Execute** klicken
4. **Erfolg**: In der Datenliste sollte ein **Data Object Bundle** erscheinen

**WICHTIG**: FAnToM verpackt geladene Daten oft in einem "Data Object Bundle". Sie müssen das Vektorfeld daraus extrahieren!

#### 7.1a Vektorfeld aus Bundle extrahieren

**Option 1: Bundle in Datenliste öffnen**
1. Klicken Sie in der Datenliste auf das Bundle
2. Es öffnet sich ein Dialog mit den enthaltenen Feldern
3. Das Vektorfeld sollte sichtbar sein (z.B. "vectors" oder "velocity")

**Option 2: Direkt im Algorithmus verbinden**
1. Öffnen Sie `Aufgabe4-1/1 Localized Flow Probe`
2. Klicken Sie auf das Dropdown für `Vector Field`
3. Falls das Bundle sichtbar ist, können Sie es direkt auswählen (FAnToM extrahiert automatisch)
4. Falls nicht: Verwenden Sie Option 1

#### 7.2 Localized Flow Probe ausführen

1. **Algorithmus auswählen** → `Aufgabe4-1/1 Localized Flow Probe`
2. **Parameter einstellen**:
   - `Vector Field`: **WICHTIG** - Wählen Sie das Vektorfeld aus dem Bundle aus (nicht das Bundle selbst!)
   - `Glyph Scale`: `0.5` (für bessere Übersicht)
   - `Sample Count`: `5` (für schnelleres Testen)
   - `Show Divergence`: ✅ aktiviert
   - `Show Rotation`: ✅ aktiviert
   - `Show Curvature`: ✅ aktiviert
   - `Step Size`: `1e-4` (Standard)
3. **Execute** klicken
4. **Erfolg**: Outputs sollten verfügbar sein:
   - `Glyph Positions`
   - `Vector Arrows`
   - `Rotation Arrows`

#### 7.3 Visualisierung

1. **Vector Arrows visualisieren**:
   - Algorithmus: `Line Set/Show Line Set`
   - `Line set`: Wählen Sie `Vector Arrows` aus
   - `Render Type`: `Lines`
   - `Show Lines`: ✅ aktiviert
   - `Line Width`: `2`
   - **Execute** → Vektorpfeile sollten sichtbar sein

2. **Rotation Arrows visualisieren** (optional):
   - Algorithmus: `Line Set/Show Line Set`
   - `Line set`: Wählen Sie `Rotation Arrows` aus
   - Gleiche Einstellungen wie oben
   - **Execute** → Rotations-Pfeile sollten sichtbar sein

3. **Glyph Positions visualisieren** (optional):
   - Algorithmus: `Point Set/Show Point Set`
   - `Point set`: Wählen Sie `Glyph Positions` aus
   - **Execute** → Punkte sollten sichtbar sein

### Test 2: Superquadric Tensor Glyphs mit JetStrainTensors.vtk

#### 7.1 Daten laden

1. In FAnToM: **Algorithmus auswählen** → `Load/VTK`
2. **File Path** eingeben:
   ```
   /Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/Wissenschaftliche Visualisierung Hauptaufgaben/TestDataHauptaufgaben/JetStrainTensors.vtk
   ```
3. **Execute** klicken
4. **Erfolg**: In der Datenliste sollte ein **Data Object Bundle** erscheinen

#### 7.1a Tensorfeld aus Bundle extrahieren

**Option 1: Bundle in Datenliste öffnen**
1. Klicken Sie in der Datenliste auf das Bundle
2. Es öffnet sich ein Dialog mit den enthaltenen Feldern
3. Das Tensorfeld sollte sichtbar sein (z.B. "tensors" oder "strain")

**Option 2: Direkt im Algorithmus verbinden**
1. Öffnen Sie `Aufgabe4-1/2 Superquadric Tensor Glyphs`
2. Klicken Sie auf das Dropdown für `Tensor Field`
3. Falls das Bundle sichtbar ist, können Sie es direkt auswählen
4. Falls nicht: Verwenden Sie Option 1

#### 7.2 Superquadric Tensor Glyphs ausführen

1. **Algorithmus auswählen** → `Aufgabe4-1/2 Superquadric Tensor Glyphs`
2. **Parameter einstellen**:
   - `Tensor Field`: **WICHTIG** - Wählen Sie das Tensorfeld aus dem Bundle aus (nicht das Bundle selbst!)
   - `Glyph Scale`: `0.5` (für bessere Übersicht)
   - `Sample Count`: `3` (weniger Glyphen für bessere Übersicht)
   - `Resolution Theta`: `20`
   - `Resolution Phi`: `20`
   - `Sharpness Parameter γ`: `3.0` (Standard)
3. **Execute** klicken
4. **Erfolg**: Output `Glyph Mesh` sollte verfügbar sein

#### 7.3 Visualisierung

1. **Glyph Mesh visualisieren**:
   - Algorithmus: `Mesh/Show Mesh`
   - `Mesh`: Wählen Sie `Glyph Mesh` aus
   - **Execute** → Superquadric-Glyphen sollten sichtbar sein
   - Die Glyphen sollten verschiedene Formen zeigen (abhängig von den Tensor-Eigenschaften)

## Schritt 8: Weitere Testdaten

### Alternative Vektorfelder für Localized Flow Probe

```bash
# wind.vtk
/Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/Wissenschaftliche Visualisierung Hauptaufgaben/TestDataHauptaufgaben/wind.vtk

# windBorders.vtk
/Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/Wissenschaftliche Visualisierung Hauptaufgaben/TestDataHauptaufgaben/windBorders.vtk
```

### Alternative Tensorfelder für Superquadric Glyphs

```bash
# 2D-tensors.vtk (falls vorhanden)
/Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/Wissenschaftliche Visualisierung Hauptaufgaben/TestDataHauptaufgaben/2D-tensors.vtk
```

## Troubleshooting

### Problem: Plugin wird nicht gefunden

**Symptom**: Algorithmus `Aufgabe4-1/...` erscheint nicht in der Liste

**Lösung**:
1. Prüfen Sie, ob Build erfolgreich war: `ls build/lib/fantom-plugins/`
2. Prüfen Sie, ob Installation erfolgreich war
3. FAnToM komplett neu starten
4. Prüfen Sie die Log-Ausgaben in FAnToM

### Problem: Kompilierfehler

**Symptom**: `make` schlägt fehl

**Lösung**:
1. Prüfen Sie, ob `FANTOM_DIR` korrekt gesetzt ist
2. Prüfen Sie, ob Eigen3 verfügbar ist (für Superquadric)
3. Prüfen Sie Compiler-Flags und Fehlermeldungen

### Problem: Keine Ausgabe

**Symptom**: Algorithmus läuft, aber keine Outputs

**Lösung**:
1. Prüfen Sie, ob Eingabefeld korrekt verbunden ist
2. Prüfen Sie Log-Ausgaben auf Warnungen
3. Reduzieren Sie `Sample Count` (vielleicht sind alle Samples außerhalb der Domain)
4. Prüfen Sie, ob Testdaten korrekt geladen wurden

### Problem: Absturz bei Visualisierung

**Symptom**: FAnToM stürzt ab beim Visualisieren

**Lösung**:
1. Reduzieren Sie `Resolution Theta/Phi` (z.B. auf 10)
2. Reduzieren Sie `Sample Count` (z.B. auf 2)
3. Prüfen Sie, ob Mesh-Daten korrekt sind

### Problem: Falsche Pfade

**Symptom**: Dateien werden nicht gefunden

**Lösung**:
- Verwenden Sie absolute Pfade (wie oben gezeigt)
- Oder navigieren Sie in FAnToM zum Verzeichnis und wählen Sie die Datei aus

## Schnellstart-Skript

Erstellen Sie eine Datei `build_and_test.sh`:

```bash
#!/bin/bash

# Konfiguration
FANTOM_DIR="/Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/FAnToM_MacOS26"
SOURCE_DIR="/Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/aufgabe4-1_src"

# Build
cd "$SOURCE_DIR"
mkdir -p build
cd build
FANTOM_DIR="$FANTOM_DIR" cmake ../
make
make install

echo "Build abgeschlossen! Starte FAnToM..."
"$FANTOM_DIR/bin/fantom" -P "$SOURCE_DIR/build/lib/fantom-plugins"
```

Ausführbar machen und ausführen:
```bash
chmod +x build_and_test.sh
./build_and_test.sh
```

## Nächste Schritte

Nach erfolgreichem Testen können Sie:
1. Parameter variieren und Ergebnisse vergleichen
2. Weitere Testdaten ausprobieren
3. Die Visualisierung optimieren (Farben, Skalierung, etc.)
4. Die Algorithmen für spezifische Anwendungsfälle anpassen
