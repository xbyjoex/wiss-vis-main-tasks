### FAnToM Session
### API Version: 20170511
### Used core version:    GITDIR-NOTFOUND (GITDIR-NOTFOUND)
### Used toolbox version: GITDIR-NOTFOUND (GITDIR-NOTFOUND)

################################################################
###                  Reset GUI                               ###
################################################################
fantom.ui.setCamera( 0, fantom.ui.Camera( fantom.math.Vector3(81.409, 99.5, 323.452), fantom.math.Vector3(81.409, 99.5, 81.4091), fantom.math.Vector3(-6.26179e-10, 1, 0), 1, 1.0472 ) )
fantom.ui.setCamera( 1, fantom.ui.Camera( fantom.math.Vector3(193.824, 12.2647, 0), fantom.math.Vector3(40.0611, 12.2647, 0), fantom.math.Vector3(0, 0, 1), 1, 1.0472 ) )
fantom.ui.setCamera( 2, fantom.ui.Camera( fantom.math.Vector3(40.0611, -141.499, -1.95096e-06), fantom.math.Vector3(40.0611, 12.2646, -2.27374e-13), fantom.math.Vector3(0, -1.26881e-08, 1), 0, 1.0472 ) )
fantom.ui.setCamera( 3, fantom.ui.Camera( fantom.math.Vector3(76.3268, 103.789, 409.282), fantom.math.Vector3(76.3268, 103.789, 77.2579), fantom.math.Vector3(0, 1, 0), 0, 1.0472 ) )

fantom.ui.setClippingPlane( fantom.ui.ClippingPlane( 0, fantom.math.Vector4( 1, 0, -2.23711e-17, 1 ), False ) )
fantom.ui.setClippingPlane( fantom.ui.ClippingPlane( 1, fantom.math.Vector4( -1, 0, -2.23711e-17, 1 ), False ) )
fantom.ui.setClippingPlane( fantom.ui.ClippingPlane( 2, fantom.math.Vector4( 0, 1, -2.23711e-17, 1 ), False ) )
fantom.ui.setClippingPlane( fantom.ui.ClippingPlane( 3, fantom.math.Vector4( 0, -1, -2.23711e-17, 1 ), False ) )
fantom.ui.setClippingPlane( fantom.ui.ClippingPlane( 4, fantom.math.Vector4( 0, 0, 1, 1 ), False ) )
fantom.ui.setClippingPlane( fantom.ui.ClippingPlane( 5, fantom.math.Vector4( -0, 0, -1, 1 ), False ) )

fantom.ui.setBackgroundColor( fantom.math.Color(1, 1, 1, 1) )

fantom.ui.setRotationCenter( fantom.ui.RotationCenter( fantom.math.Vector3(0, 0.762102, 0.69282), True, True, True ) )


################################################################
###                  Create algorithms                       ###
################################################################
Load_VTK = fantom.makeAlgorithm("Load/VTK")
Load_VTK.setName("Load/VTK")
Load_VTK.setAutoSchedule(True)
Load_VTK.setOption("Input File", "/Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/Wissenschaftliche Visualisierung Hauptaufgaben/TestDataHauptaufgaben/BrainTensors.vtk")
Load_VTK.setOption("Big Endian", True)
Load_VTK.setOption("Dimension", "3D")
Load_VTK.setOption("Time List", "")
fantom.ui.setAlgorithmPosition(Load_VTK, fantom.math.Vector2(0, 35))

# Inbound connections of this algorithm:

# Run the algorithm
Load_VTK.runBlocking()

Aufgabe41_2SuperquadricGeneration = fantom.makeAlgorithm("Aufgabe4-1/2 Superquadric Generation")
Aufgabe41_2SuperquadricGeneration.setName("Aufgabe4-1/2 Superquadric Generation")
Aufgabe41_2SuperquadricGeneration.setAutoSchedule(True)
Aufgabe41_2SuperquadricGeneration.setOption("Glyph Scale", 18000)
Aufgabe41_2SuperquadricGeneration.setOption("Sharpness Parameter γ", 0.5)
Aufgabe41_2SuperquadricGeneration.setOption("Use Kindlmann Shape", True)
Aufgabe41_2SuperquadricGeneration.setOption("Resolution Theta", 50)
Aufgabe41_2SuperquadricGeneration.setOption("Resolution Phi", 50)
Aufgabe41_2SuperquadricGeneration.setOption("Sample Count", 10)
Aufgabe41_2SuperquadricGeneration.setOption("Time", 0)
fantom.ui.setAlgorithmPosition(Aufgabe41_2SuperquadricGeneration, fantom.math.Vector2(0, 143))

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



