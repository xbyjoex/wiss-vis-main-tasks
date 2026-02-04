# Report: Algorithmen-Umsetzung vs. Paper (Ist/Soll-Analyse)

**Stand:** Februar 2025  
**Gegenstand:** Abgleich der Implementierungen in `LocalizedFlowProbe.cpp` und `SuperquadricTensorGlyphs.cpp` mit den in `summary_papers.md` zusammengefassten Papern (de Leeuw & van Wijk 1993; Kindlmann 2004).

---

## 1. Localized Flow Probe (de Leeuw & van Wijk, 1993)

### 1.1 Soll-Zustand (Paper)

- **Taylor-Approximation:** \( u(x) = u(x_0) + J(x - x_0) \), \( J \) = Jacobi-Matrix (Geschwindigkeitsgradient).
- **Lokales Koordinatensystem:** Frenet-Rahmen: x-Achse = Geschwindigkeit \( u \), y-Achse parallel zum Krümmungsvektor \( c \).
- **Krümmungsvektor:**  
  \( c = \frac{J u (u \cdot u) - u (u \cdot J u)}{|u|^3} \).
- **Zerlegung:** Parallel zur Strömung (Beschleunigung, Scherung, Krümmung), senkrecht (Torsion, Konvergenz/Divergenz).
- **Geometrische Primitiven:**
  - **Geschwindigkeit & Krümmung:** Pfeil mit Schaftlänge \( l = u \, \Delta t \); Schaft als **Bogen des Schmiegkreises** (erste Ordnung der Stromlinie).
  - **Beschleunigung:** Membran senkrecht zur Strömung; Wölbung vorne/hinten für Beschleunigung/Verzögerung.
  - **Torsion:** „Candy stripes“ auf der Pfeiloberfläche (Rotation um die Geschwindigkeitsachse).
  - **Scherung:** „Shear-Ring“ – Änderung der Orientierung der Ebene senkrecht zur Strömung.
  - **Konvergenz/Divergenz:** **Oskulierendes Paraboloid** („Linse“) für Fokussierung/Ausbreitung.

### 1.2 Ist-Zustand (Code)

| Aspekt | Umsetzung | Bewertung |
|--------|-----------|-----------|
| **Jacobi-Matrix J** | Zentrale Differenzen, \( J = [\partial v/\partial x \mid \partial v/\partial y \mid \partial v/\partial z] \) | **Korrekt** |
| **Beschleunigung** | \( a = J \cdot v \) (konvektive Ableitung) | **Korrekt** |
| **Divergenz** | \( \operatorname{div}(v) = \operatorname{Spur}(J) \) | **Korrekt** |
| **Frenet-Rahmen** | Nicht implementiert (kein \( c \), keine Ausrichtung y ∥ \( c \)) | **Fehlt** |
| **Krümmungsvektor \( c \)** | Wird nicht berechnet | **Fehlt** |
| **Schaft-Geometrie** | Parabel \( x(t) = p + v t + \tfrac{1}{2} a t^2 \) (Liniensegmente) | **Abweichung:** Paper verlangt **Bogen des Schmiegkreises**, nicht Parabel |
| **Beschleunigung als Primitive** | Keine Membran, keine Wölbung; \( a \) nur zur Kurvenform genutzt | **Fehlt** |
| **Torsion** | Keine „Candy stripes“, keine Darstellung der Rotation um \( u \) | **Fehlt** |
| **Scherung** | Ring senkrecht zu \( v \), Deformation \( r' = r + J r \, \Delta t \) (linearisiert) | **Teilweise:** Idee „Deformation der Ebene ⊥ \( v \)“ getroffen, aber Paper spricht von Orientierungsänderung (Shear-Ring) |
| **Konvergenz/Divergenz** | Nur **Farbkodierung** (blau–grau–rot) auf Röhre/Ring | **Abweichung:** Paper verlangt **Paraboloid/Linse**, keine reine Farbcodierung |

### 1.3 Kritische Punkte und Verbesserungen

1. **Schaft als Schmiegkreis-Bogen (nicht Parabel)**  
   - **Soll:** Schaft = Bogen des Oskulationskreises (Krümmung \( \kappa \), Radius \( 1/\kappa \), Mittelpunkt in Richtung \( c \)).  
   - **Ist:** Parabel aus \( p + v t + \tfrac{1}{2} a t^2 \).  
   - **Empfehlung:** Krümmungsvektor \( c \) berechnen, \( \kappa = |c| \), Schaft als Kreisbogen mit Länge \( l = |u| \Delta t \) zeichnen.

2. **Frenet-Rahmen und Krümmung**  
   - **Soll:** Lokales System mit \( u \), \( c \); Zerlegung in parallel/senkrecht zur Strömung.  
   - **Ist:** Kein \( c \), kein Frenet-Rahmen.  
   - **Empfehlung:** \( c = \frac{J u (u \cdot u) - u (u \cdot J u)}{|u|^3} \) implementieren und für Schaft-Richtung sowie spätere Primitiven nutzen.

3. **Beschleunigung als eigenes Primitive**  
   - **Soll:** Membran ⊥ Strömung mit Wölbung vor/hinten.  
   - **Ist:** Keine Membran.  
   - **Empfehlung:** Ebene senkrecht zu \( u \) am Pfeilende (oder -anfang), Wölbung in \( u \)-Richtung proportional zu \( a \cdot u \) (vorwärts/rückwärts).

4. **Torsion**  
   - **Soll:** Candy stripes (Rotation um \( u \)).  
   - **Ist:** Nicht umgesetzt.  
   - **Empfehlung:** Torsion aus Frenet-Rahmen bzw. aus \( J \) ableiten und als Streifenmuster auf Röhre oder Ring kodieren.

5. **Konvergenz/Divergenz**  
   - **Soll:** Oskulierendes Paraboloid (Linse).  
   - **Ist:** Nur Divergenz-Farbe.  
   - **Empfehlung:** Zusätzlich zur Farbe ein Paraboloid (z. B. quadratisch in der Ebene ⊥ \( u \), Wölbung durch div) als Linse darstellen.

6. **Rotation (Curl)**  
   - Im Code berechnet (`computeRotation`), aber nirgends in der Visualisierung verwendet. Optional: Nutzung für Torsion oder separates Primitive dokumentieren bzw. einbauen.

---

## 2. Superquadric Tensor Glyphs (Kindlmann, 2004)

### 2.1 Soll-Zustand (Paper)

- **Westin-Metriken:**  
  \( c_l = \frac{\lambda_1 - \lambda_2}{\sum \lambda_i},\quad c_p = \frac{2(\lambda_2 - \lambda_3)}{\sum \lambda_i},\quad c_s = \frac{3\lambda_3}{\sum \lambda_i} \).
- **Formparameter:** Mit Schärfe \( \gamma \); für \( c_l \ge c_p \): \( \alpha = (1 - c_p)^\gamma \), \( \beta = (1 - c_l)^\gamma \); sonst vertauscht.
- **Geometrie:** Superquadrik im Einheitsraum, dann \( G_T = R \, \Lambda \, G \) (Rotation \( R \) aus Eigenvektoren, \( \Lambda \) Diagonalmatrix der Eigenwerte).
- **Eigenschaften:** Zylinder-ähnlich (linear), scheiben-ähnlich (planar), kugel-ähnlich (sphärisch); scharfe Kanten durch \( \gamma \) für bessere Orientierung.

### 2.2 Ist-Zustand (Code)

| Aspekt | Umsetzung | Bewertung |
|--------|-----------|-----------|
| **Westin-Metriken** | \( c_l, c_p, c_s \) wie im Paper | **Korrekt** |
| **Formparameter \( \alpha, \beta \)** | \( c_l \ge c_p \Rightarrow \alpha = (1-c_p)^\gamma, \beta = (1-c_l)^\gamma \); sonst getauscht | **Korrekt** |
| **Superquadrik-Punkt** | \( x = \operatorname{sgn}(\cos\phi)|\cos\phi|^\alpha \operatorname{sgn}(\cos\theta)|\cos\theta|^\beta \) (analog y, z) | **Korrekt** |
| **Superquadrik-Normale** | Exponenten \( (2-\alpha), (2-\beta) \) (Standard-Form) | **Korrekt** |
| **Skalierung & Rotation** | \( \texttt{scaledPos} = (l_2\,x, l_3\,y, l_1\,z) \), dann \( \texttt{rotPos} = \ldots v_2 + \ldots v_3 + \ldots v_1 \); Hauptachse (z) → \( \lambda_1, v_1 \) | **Korrekt** (Konvention: z = Hauptachse → größter Eigenwert) |
| **Normalen-Transformation** | Skalierung mit \( 1/l_2, 1/l_3, 1/l_1 \), dann Rotation | **Korrekt** (entspricht Skalierung der Fläche) |
| **Eigenwert-Sortierung** | Absteigend \( \lambda_1 \ge \lambda_2 \ge \lambda_3 \) | **Korrekt** |
| **Reelle Eigenwerte/Vektoren** | Fallback bei komplexen Eigenwerten (Realteil/Betrag) | **Sinnvoll** |
| **Rechtssystem** | Gram-Schmidt + \( v_3 = v_1 \times v_2 \) | **Korrekt** |
| **Default \( \gamma \)** | Code: 1.0; Doku: 3.0 empfohlen | **Klein:** Default weicht von typischer Empfehlung ab |

### 2.3 Kritische Punkte und Verbesserungen

1. **Default \( \gamma \)**  
   - **Soll (Doku):** \( \gamma = 3 \) für typische Superquadrik-Darstellung.  
   - **Ist:** \( \gamma = 1 \) (ellipsoidnah).  
   - **Empfehlung:** Default auf z. B. 2.0–3.0 setzen oder in UI/Doku klar machen, dass \( \gamma = 1 \) „weiche“ Glyphen liefert.

2. **Option „Use Kindlmann Shape“**  
   - Wenn aus: Formparameter weichen ab (runde Querschnitte). Das ist konsistent dokumentiert; für strikte Paper-Nachbildung sollte „Kindlmann“ standardmäßig an sein oder der Default so gewählt werden, dass das Paper-Verhalten vorherrschend ist.

3. **Skalierung der Glyphen-Größe**  
   - Glyphen werden mit \( \lambda_i \) skaliert; zusätzlich global mit `Glyph Scale`. Das ist in Ordnung; bei sehr unterschiedlichen Eigenwerten kann eine Normalisierung (z. B. nach Spur oder \( \lambda_1 \)) die Vergleichbarkeit verbessern (optional).

4. **Achsenkonvention**  
   - Code: Einheits-Superquadrik z → \( \lambda_1, v_1 \). Doku schreibt teilweise \( (\lambda_1 x, \lambda_2 y, \lambda_3 z) \) mit x als erstem Eintrag; inhaltlich ist die Zuordnung Hauptachse ↔ \( v_1 \) im Code korrekt. Evtl. Doku an Code anpassen („z-Achse der Einheits-Superquadrik entspricht \( v_1 \)“).

---

## 3. Zusammenfassung

### Localized Flow Probe

- **Korrekt:** Jacobi-Matrix, Beschleunigung \( Jv \), Divergenz, konzeptionell Ring mit \( J \)-Deformation, Farbcodierung nach Divergenz.
- **Falsch / fehlend:**  
  - Schaft als **Parabel** statt **Bogen des Schmiegkreises**;  
  - kein Frenet-Rahmen, kein Krümmungsvektor \( c \);  
  - keine **Membran** für Beschleunigung;  
  - keine **Torsion** (Candy stripes);  
  - Konvergenz/Divergenz nur als **Farbe**, nicht als **Paraboloid/Linse**.

### Superquadric Tensor Glyphs

- **Korrekt:** Westin-Metriken, Formparameter \( \alpha, \beta \), Superquadrik-Formel, Normalen, Skalierung/Rotation \( G_T = R \Lambda G \), Eigenwertbehandlung, Rechtssystem.
- **Verbesserungspotenzial:** Default \( \gamma \) (z. B. 2–3), Klarstellung „Kindlmann Shape“ als Standard für Paper-Treue, ggf. Doku zur Achsenkonvention.

---

## 4. Empfohlene Prioritäten (Localized Flow Probe)

1. **Hoch:** Krümmungsvektor \( c \) berechnen und Schaft als **Bogen des Schmiegkreises** (Länge \( |u|\Delta t \)) umsetzen.  
2. **Mittel:** Konvergenz/Divergenz als **Paraboloid/Linse** ergänzen (zusätzlich zur Farbe).  
3. **Mittel:** Beschleunigung als **Membran** mit Wölbung darstellen.  
4. **Niedrig:** Frenet-Rahmen vollständig nutzen; **Torsion** (Candy stripes) implementieren; ggf. Rotation (Curl) in der Visualisierung verwenden.

---

## 5. Nach Implementierung (Stand nach Plan-Umsetzung)

### Localized Flow Probe

- **DataAlgorithm:** Krümmungsvektor \( c = a_\perp/|u| \) wird berechnet; neuer Output **"Curvature"** (`Function<Vector3>`).
- **VisAlgorithm:** Alle Paper-Primitiven umgesetzt:
  - **Schaft:** Bogen des Schmiegkreises (Länge \( L = |u|\Delta t \), Radius \( R = 1/\kappa \)); bei \( \kappa < \varepsilon \) gerades Segment.
  - **Tube:** Zylinder entlang des Bogens mit **Torsion-Streifen** (Candy stripes) über \( (\nabla\times v)\cdot u \).
  - **Beschleunigung:** Membran senkrecht zur Strömung am Pfeilende, Wölbung in \( u \)-Richtung proportional zu \( a\cdot u \).
  - **Konvergenz/Divergenz:** Oskulierendes Paraboloid (Linse) in der Ebene \( \perp u \), Höhe \( \propto \mathrm{div}\,(x^2+y^2) \).
  - **Shear-Ring:** Ring \( \perp u \), mit \( J \)-Deformation; Frame optional Frenet-artig (Achse \( \parallel c \)).
  - **Pfeilspitze:** Unverändert.
- Ausgabe: ein Grafik-Output „Flow Probes“ als **Compound** aus LINES- und TRIANGLES-Drawable (FAnToM `graphics::makeCompound`).

### Superquadric Tensor Glyphs

- **Defaults:** \( \gamma = 2.5 \) (statt 1.0); „Use Kindlmann Shape“ standardmäßig **true** (Paper-konform).
- Kernlogik unverändert (Westin, \( \alpha/\beta \), Skalierung/Rotation); Kommentare auf Englisch, reduziert.

---

*Report Ende.*
