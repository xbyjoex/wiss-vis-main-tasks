# Troubleshooting - Häufige Probleme

## Problem: "Missing input for mandatory option 'Vector Field'"

### Symptom
```
ERROR [Aufgabe4-1/1 Localized Flow Probe] Missing input for mandatory option "Vector Field"
```

### Ursache
Das Vektorfeld wurde nicht korrekt mit dem Algorithmus verbunden. In FAnToM werden geladene Daten oft in einem "Data Object Bundle" verpackt.

### Lösung

#### Schritt 1: Daten laden
1. `Load/VTK` ausführen
2. Datei laden (z.B. `JetVelocity.vtk`)
3. **Wichtig**: In der Datenliste erscheint ein "Data Object Bundle"

#### Schritt 2: Vektorfeld extrahieren
Das Bundle enthält das eigentliche Vektorfeld. Sie müssen es extrahieren:

**Option A: Bundle öffnen**
1. Klicken Sie auf das Bundle in der Datenliste
2. Es öffnet sich ein Dialog mit den enthaltenen Feldern
3. Wählen Sie das Vektorfeld aus (z.B. "vectors" oder "velocity")

**Option B: Direkt verbinden**
1. Öffnen Sie `Aufgabe4-1/1 Localized Flow Probe`
2. Im Dropdown für `Vector Field` sollten Sie das Vektorfeld sehen
3. Falls nicht: Bundle zuerst öffnen (siehe Option A)

#### Schritt 3: Algorithmus ausführen
1. `Vector Field` ist jetzt verbunden
2. Parameter einstellen
3. `Execute` klicken

### Alternative: Bundle Helper verwenden
FAnToM hat manchmal einen "Bundle Chooser" oder ähnliches Tool, um Felder aus Bundles zu extrahieren. Suchen Sie in der Algorithmus-Liste nach "Bundle" oder "Extract".

## Problem: "Missing input for mandatory option 'Tensor Field'"

### Lösung
Gleiche Vorgehensweise wie oben, aber für Tensorfelder:
1. Bundle öffnen
2. Tensorfeld auswählen (z.B. "tensors" oder "strain")
3. Mit `Aufgabe4-1/2 Superquadric Tensor Glyphs` verbinden

## Problem: Keine Ausgabe nach Execute

### Mögliche Ursachen

1. **Alle Sampling-Punkte außerhalb der Domain**
   - Lösung: `Sample Count` erhöhen oder `Seed Center Point` anpassen

2. **Feld ist null**
   - Lösung: Prüfen Sie das Feld mit `Field/Show Hedgehogs` oder `Field Info`

3. **Falscher Feldtyp**
   - Lösung: Stellen Sie sicher, dass Sie ein Vektorfeld für Flow Probe bzw. Tensorfeld für Superquadric verwenden

### Debugging
- Prüfen Sie die Log-Ausgaben in FAnToM
- Schauen Sie nach Warnungen (WARNING) oder Fehlern (ERROR)
- Prüfen Sie, ob das Eingabefeld korrekt geladen wurde

## Problem: Plugin wird nicht gefunden

### Symptom
Algorithmus `Aufgabe4-1/...` erscheint nicht in der Liste

### Lösung
1. Prüfen Sie, ob Build erfolgreich war: `ls build/lib/fantom-plugins/aufgabe4-1/`
2. Prüfen Sie Log beim Starten von FAnToM - sollte zeigen: `Loading plugin: .../aufgabe4-1/libplugin1.dylib`
3. FAnToM komplett neu starten
4. Prüfen Sie, ob `make install` ausgeführt wurde

## Problem: Absturz bei Visualisierung

### Lösung
1. `Resolution Theta/Phi` reduzieren (z.B. auf 10)
2. `Sample Count` reduzieren (z.B. auf 2)
3. Prüfen Sie, ob Mesh-Daten korrekt sind

## Problem: Falsche Visualisierung

### Symptom
Glyphen erscheinen, aber sehen falsch aus

### Mögliche Ursachen
1. **Skalierung zu groß/klein**
   - Lösung: `Glyph Scale` anpassen

2. **Zu viele Glyphen**
   - Lösung: `Sample Count` reduzieren

3. **Falsche Orientierung**
   - Lösung: Prüfen Sie, ob Eigenvektoren korrekt berechnet werden (für Superquadric)

## Nützliche Tipps

### Datenstruktur verstehen
- **Data Object Bundle**: Container für mehrere Felder
- **Field**: Das eigentliche Feld (Vektor, Tensor, Skalar)
- **Grid**: Die Domain, auf der das Feld definiert ist

### FAnToM GUI
- **Graph View**: Zeigt die Pipeline
- **Data View**: Zeigt geladene Daten
- **Algorithm View**: Zeigt verfügbare Algorithmen

### Log-Ausgaben
- `DEBUG`: Detaillierte Informationen
- `INFO`: Normale Meldungen
- `WARNING`: Warnungen (nicht kritisch)
- `ERROR`: Fehler (kritisch)

## Hilfe bekommen

Wenn nichts hilft:
1. Prüfen Sie die vollständigen Log-Ausgaben
2. Prüfen Sie, ob Testdaten korrekt geladen wurden
3. Versuchen Sie mit einfacheren Parametern (niedrige Resolution, wenige Samples)
4. Prüfen Sie die Dokumentation in `BUILD_AND_TEST.md`
