### FAnToM Session
### API Version: 20170511
### Used core version:    GITDIR-NOTFOUND (GITDIR-NOTFOUND)
### Used toolbox version: GITDIR-NOTFOUND (GITDIR-NOTFOUND)

################################################################
###                  Reset GUI                               ###
################################################################
fantom.ui.setCamera( 0, fantom.ui.Camera( fantom.math.Vector3(79.0709, 93.655, 242.767), fantom.math.Vector3(79.0709, 93.655, 81.7562), fantom.math.Vector3(0, 1, 0), 1, 1.0472 ) )
fantom.ui.setCamera( 1, fantom.ui.Camera( fantom.math.Vector3(345.662, 99.1793, 81.7562), fantom.math.Vector3(81.0161, 99.1793, 81.7562), fantom.math.Vector3(0, 0, 1), 0, 1.0472 ) )
fantom.ui.setCamera( 2, fantom.ui.Camera( fantom.math.Vector3(81.0161, -165.466, 81.7562), fantom.math.Vector3(81.0161, 99.1793, 81.7562), fantom.math.Vector3(0, -1.26881e-08, 1), 0, 1.0472 ) )
fantom.ui.setCamera( 3, fantom.ui.Camera( fantom.math.Vector3(81.0161, 99.1793, 346.402), fantom.math.Vector3(81.0161, 99.1793, 81.7562), fantom.math.Vector3(0, 1, 0), 0, 1.0472 ) )

fantom.ui.setClippingPlane( fantom.ui.ClippingPlane( 0, fantom.math.Vector4( 1, 0, -2.23711e-17, 1 ), False ) )
fantom.ui.setClippingPlane( fantom.ui.ClippingPlane( 1, fantom.math.Vector4( -1, 0, -2.23711e-17, 1 ), False ) )
fantom.ui.setClippingPlane( fantom.ui.ClippingPlane( 2, fantom.math.Vector4( 0, 1, -2.23711e-17, 1 ), False ) )
fantom.ui.setClippingPlane( fantom.ui.ClippingPlane( 3, fantom.math.Vector4( 0, -1, -2.23711e-17, 1 ), False ) )
fantom.ui.setClippingPlane( fantom.ui.ClippingPlane( 4, fantom.math.Vector4( 0, 0, 1, 1 ), False ) )
fantom.ui.setClippingPlane( fantom.ui.ClippingPlane( 5, fantom.math.Vector4( -0, 0, -1, 1 ), False ) )

fantom.ui.setBackgroundColor( fantom.math.Color(1, 1, 1, 1) )

fantom.ui.setRotationCenter( fantom.ui.RotationCenter( fantom.math.Vector3(0, 0, 0), True, True, True ) )


################################################################
###                  Create algorithms                       ###
################################################################
Load_VTK = fantom.makeAlgorithm("Load/VTK")
Load_VTK.setName("Load/VTK")
Load_VTK.setAutoSchedule(True)
Load_VTK.setOption("Input File", "/Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/Wissenschaftliche Visualisierung Hauptaufgaben/TestDataHauptaufgaben/BrainTensors.vtk")
Load_VTK.setOption("Big Endian", True)
Load_VTK.setOption("Dimension", "2D if third component is zero")
Load_VTK.setOption("Time List", "")
fantom.ui.setAlgorithmPosition(Load_VTK, fantom.math.Vector2(0, 35))

# Inbound connections of this algorithm:

# Run the algorithm
Load_VTK.runBlocking()

Aufgabe41_2SuperquadricGeneration = fantom.makeAlgorithm("Aufgabe4-1/2 Superquadric Generation")
Aufgabe41_2SuperquadricGeneration.setName("Aufgabe4-1/2 Superquadric Generation")
Aufgabe41_2SuperquadricGeneration.setAutoSchedule(True)
Aufgabe41_2SuperquadricGeneration.setOption("Glyph Scale", 25000)
Aufgabe41_2SuperquadricGeneration.setOption("Sharpness Parameter γ", 2.5)
Aufgabe41_2SuperquadricGeneration.setOption("Use Kindlmann Shape", True)
Aufgabe41_2SuperquadricGeneration.setOption("Resolution Theta", 20)
Aufgabe41_2SuperquadricGeneration.setOption("Resolution Phi", 20)
Aufgabe41_2SuperquadricGeneration.setOption("Sample Count", 20)
Aufgabe41_2SuperquadricGeneration.setOption("Time", 0)
Aufgabe41_2SuperquadricGeneration.setOption("Normalize to cell", True)
Aufgabe41_2SuperquadricGeneration.setOption("Cell fill", 1)
fantom.ui.setAlgorithmPosition(Aufgabe41_2SuperquadricGeneration, fantom.math.Vector2(0, 142))

# Inbound connections of this algorithm:
Load_VTK.connect("Fields", Aufgabe41_2SuperquadricGeneration, "Tensor Field")

# Run the algorithm
Aufgabe41_2SuperquadricGeneration.runBlocking()

Aufgabe41_3SuperquadricRendering = fantom.makeAlgorithm("Aufgabe4-1/3 Superquadric Rendering")
Aufgabe41_3SuperquadricRendering.setName("Aufgabe4-1/3 Superquadric Rendering")
Aufgabe41_3SuperquadricRendering.setAutoSchedule(True)
fantom.ui.setAlgorithmPosition(Aufgabe41_3SuperquadricRendering, fantom.math.Vector2(0, 270))
Aufgabe41_3SuperquadricRendering.setVisualOutputVisible('Glyphs', True)

# Inbound connections of this algorithm:
Aufgabe41_2SuperquadricGeneration.connect("Normals", Aufgabe41_3SuperquadricRendering, "Normals")
Aufgabe41_2SuperquadricGeneration.connect("Glyph Mesh", Aufgabe41_3SuperquadricRendering, "Grid")
Aufgabe41_2SuperquadricGeneration.connect("Color", Aufgabe41_3SuperquadricRendering, "Color")

# Run the algorithm
Aufgabe41_3SuperquadricRendering.runBlocking()



