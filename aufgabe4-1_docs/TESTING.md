# Testing-Anleitung für Aufgabe 4.1

## Build-Verifikation

### Schritt 1: CMake-Konfiguration prüfen

```bash
cd aufgabe4-1_src
mkdir build
cd build
FANTOM_DIR=/path/to/FAnToM_MacOS26 cmake ../
```

**Erwartetes Ergebnis**: 
- Keine Fehler
- Plugin wird erkannt: `plugin1`

### Schritt 2: Kompilierung

```bash
make
```

**Erwartetes Ergebnis**:
- Keine Kompilierfehler
- Plugin-Bibliothek wird erstellt: `lib/fantom-plugins/aufgabe4-1_plugin1.*`

### Schritt 3: Installation

```bash
make install
```

**Erwartetes Ergebnis**:
- Plugin wird in FAnToM-Verzeichnis installiert

## Funktions-Tests

### Test 1: Localized Flow Probe - Basis-Funktionalität

**Vorbereitung**:
1. FAnToM starten: `cd build && make run`
2. Testdaten laden: `Load/VTK` → `TestData/streamTest1.vtk`

**Test-Schritte**:
1. Algorithmus auswählen: `Aufgabe4-1/1 Localized Flow Probe`
2. Input verbinden: `Vector Field` → geladenes Vektorfeld
3. Standard-Parameter verwenden
4. `Execute` ausführen

**Erwartetes Ergebnis**:
- Keine Fehler in Log
- Outputs verfügbar:
  - `Glyph Positions` (PointSet)
  - `Vector Arrows` (LineSet)
  - `Rotation Arrows` (LineSet, wenn Rotation aktiviert)

**Visualisierung testen**:
- `Point Set/Show Point Set` → `Glyph Positions`: Punkte sichtbar
- `Line Set/Show Line Set` → `Vector Arrows`: Pfeile sichtbar (Hedgehogs)
- `Line Set/Show Line Set` → `Rotation Arrows`: Rotations-Pfeile sichtbar

### Test 2: Localized Flow Probe - Divergenz/Rotation/Krümmung

**Test-Schritte**:
1. Gleiche Pipeline wie Test 1
2. Parameter anpassen:
   - `Show Divergence`: true
   - `Show Rotation`: true
   - `Show Curvature`: true
   - `Sample Count`: 5
3. `Execute` ausführen

**Erwartetes Ergebnis**:
- Rotation-Arrows sichtbar (wenn Rotation vorhanden)
- Keine Abstürze oder Fehler

### Test 3: Superquadric Tensor Glyphs - Basis-Funktionalität

**Vorbereitung**:
1. FAnToM starten
2. Tensorfeld laden: `Load/VTK` → `Wissenschaftliche Visualisierung Hauptaufgaben/TestDataHauptaufgaben/JetStrainTensors.vtk`

**Test-Schritte**:
1. Algorithmus auswählen: `Aufgabe4-1/2 Superquadric Tensor Glyphs`
2. Input verbinden: `Tensor Field` → geladenes Tensorfeld
3. Standard-Parameter verwenden:
   - `Sample Count`: 3
   - `Resolution Theta`: 20
   - `Resolution Phi`: 20
4. `Execute` ausführen

**Erwartetes Ergebnis**:
- Keine Fehler in Log
- Output verfügbar: `Glyph Mesh` (UnstructuredGrid)

**Visualisierung testen**:
- `Mesh/Show Mesh` → `Glyph Mesh`: Superquadric-Glyphen sichtbar
- Glyphen sollten verschiedene Formen zeigen (abhängig von Tensor-Eigenschaften)

### Test 4: Superquadric Tensor Glyphs - Parameter-Variation

**Test-Schritte**:
1. Gleiche Pipeline wie Test 3
2. Parameter variieren:
   - `Sharpness Parameter γ`: 1.0, 3.0, 5.0
   - `Resolution Theta/Phi`: 10, 20, 30
   - `Sample Count`: 2, 5, 10

**Erwartetes Ergebnis**:
- Alle Parameter-Kombinationen funktionieren
- Höhere Auflösung → glattere Oberflächen
- Höherer γ → schärfere Formübergänge

### Test 5: Edge Cases

**Test 5.1: Null-Vektorfeld**
- Testdaten mit Null-Vektoren
- Erwartung: Keine Glyphen an Null-Punkten

**Test 5.2: Null-Tensor**
- Tensorfeld mit Null-Tensoren
- Erwartung: Diese Punkte werden übersprungen

**Test 5.3: Sehr kleine Eigenwerte**
- Tensorfeld mit sehr kleinen Eigenwerten
- Erwartung: Numerische Stabilität, keine Abstürze

**Test 5.4: Komplexe Eigenwerte**
- Tensorfeld mit komplexen Eigenwerten (falls möglich)
- Erwartung: Realteil wird verwendet, keine Abstürze

## Performance-Tests

### Test 6: Große Datensätze

**Test-Schritte**:
1. Große Vektorfelder/Tensorfelder laden
2. `Sample Count` erhöhen (10, 20, 30)
3. Performance beobachten

**Erwartetes Ergebnis**:
- Algorithmus bleibt responsiv
- Keine Speicherprobleme
- Abort-Flag funktioniert

## Bekannte Einschränkungen

1. **Divergenz/Krümmung-Visualisierung**: Werden berechnet, aber aktuell nicht direkt als separate Geometrie visualisiert (nur gespeichert für zukünftige Erweiterungen)

2. **Uniformes Sampling**: Verwendet uniformes Grid-Sampling. Adaptives Sampling wäre für große Felder effizienter.

3. **Komplexe Eigenwerte**: Werden erkannt und behandelt, aber die Visualisierung verwendet nur den Realteil.

## Troubleshooting

### Problem: Plugin wird nicht gefunden

**Lösung**:
- Prüfen ob `FANTOM_DIR` korrekt gesetzt ist
- Prüfen ob Plugin kompiliert wurde: `ls build/lib/fantom-plugins/`
- FAnToM neu starten

### Problem: Kompilierfehler

**Lösung**:
- Prüfen ob Eigen3 verfügbar ist (für Superquadric)
- Prüfen ob FAnToM-Header gefunden werden
- Prüfen Compiler-Flags

### Problem: Keine Ausgabe

**Lösung**:
- Prüfen ob Eingabefeld korrekt verbunden ist
- Prüfen ob Sampling-Punkte in Domain liegen
- Log-Ausgaben prüfen

### Problem: Absturz bei Visualisierung

**Lösung**:
- `Resolution` reduzieren
- `Sample Count` reduzieren
- Prüfen ob Mesh-Daten korrekt sind

## Checkliste

- [ ] Build erfolgreich
- [ ] Localized Flow Probe: Basis-Funktionalität
- [ ] Localized Flow Probe: Divergenz/Rotation/Krümmung
- [ ] Superquadric Tensor Glyphs: Basis-Funktionalität
- [ ] Superquadric Tensor Glyphs: Parameter-Variation
- [ ] Edge Cases getestet
- [ ] Performance akzeptabel
- [ ] Dokumentation vollständig
