### FAnToM Session
### API Version: 20170511
### Used core version:    GITDIR-NOTFOUND (GITDIR-NOTFOUND)
### Used toolbox version: GITDIR-NOTFOUND (GITDIR-NOTFOUND)

################################################################
###                  Reset GUI                               ###
################################################################
fantom.ui.setCamera( 0, fantom.ui.Camera( fantom.math.Vector3(-0.600933, -0.784496, 0.951023), fantom.math.Vector3(-0.600933, -0.784496, -1.00199), fantom.math.Vector3(0, 1, 0), 1, 1.0472 ) )
fantom.ui.setCamera( 1, fantom.ui.Camera( fantom.math.Vector3(0.545126, -0.63502, -0.282231), fantom.math.Vector3(0, -0.63502, -0.282231), fantom.math.Vector3(0, 0, 1), 0, 1.0472 ) )
fantom.ui.setCamera( 2, fantom.ui.Camera( fantom.math.Vector3(-0.675757, -0.353447, 0.209981), fantom.math.Vector3(-0.675757, -1.19209e-06, 0.209981), fantom.math.Vector3(0, -1.26881e-08, 1), 0, 1.0472 ) )
fantom.ui.setCamera( 3, fantom.ui.Camera( fantom.math.Vector3(0, 0, 2), fantom.math.Vector3(0, 0, 0), fantom.math.Vector3(0, 1, 0), 0, 1.0472 ) )

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
Load_VTK.setOption("Input File", "/Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/Wissenschaftliche Visualisierung Hauptaufgaben/TestDataHauptaufgaben/streamTest1.vtk")
Load_VTK.setOption("Big Endian", True)
Load_VTK.setOption("Dimension", "2D if third component is zero")
Load_VTK.setOption("Time List", "")
fantom.ui.setAlgorithmPosition(Load_VTK, fantom.math.Vector2(0, 35))

# Inbound connections of this algorithm:

# Run the algorithm
Load_VTK.runBlocking()

Aufgabe41_1LocalizedFlowProbe = fantom.makeAlgorithm("Aufgabe4-1/1 Localized Flow Probe")
Aufgabe41_1LocalizedFlowProbe.setName("Aufgabe4-1/1 Localized Flow Probe")
Aufgabe41_1LocalizedFlowProbe.setAutoSchedule(True)
Aufgabe41_1LocalizedFlowProbe.setOption("Step Size", 0.0001)
Aufgabe41_1LocalizedFlowProbe.setOption("Sample Count", 3)
Aufgabe41_1LocalizedFlowProbe.setOption("Time", 0)
fantom.ui.setAlgorithmPosition(Aufgabe41_1LocalizedFlowProbe, fantom.math.Vector2(0, 142))

# Inbound connections of this algorithm:
Load_VTK.connect("Fields", Aufgabe41_1LocalizedFlowProbe, "Vector Field")

# Run the algorithm
Aufgabe41_1LocalizedFlowProbe.runBlocking()

Aufgabe41_1FlowProbeRendering = fantom.makeAlgorithm("Aufgabe4-1/1 Flow Probe Rendering")
Aufgabe41_1FlowProbeRendering.setName("Aufgabe4-1/1 Flow Probe Rendering")
Aufgabe41_1FlowProbeRendering.setAutoSchedule(True)
Aufgabe41_1FlowProbeRendering.setOption("Glyph Scale", 0.5)
Aufgabe41_1FlowProbeRendering.setOption("Tube Length", 0.2)
Aufgabe41_1FlowProbeRendering.setOption("Ring Size", 0.1)
Aufgabe41_1FlowProbeRendering.setOption("Line Width", 2)
Aufgabe41_1FlowProbeRendering.setOption("Show Tube", True)
Aufgabe41_1FlowProbeRendering.setOption("Show Membrane", True)
Aufgabe41_1FlowProbeRendering.setOption("Show Lens", True)
Aufgabe41_1FlowProbeRendering.setOption("Color by Probe ID", False)
fantom.ui.setAlgorithmPosition(Aufgabe41_1FlowProbeRendering, fantom.math.Vector2(0, 333))
Aufgabe41_1FlowProbeRendering.setVisualOutputVisible('Flow Probes', True)

# Inbound connections of this algorithm:
Aufgabe41_1LocalizedFlowProbe.connect("Velocity", Aufgabe41_1FlowProbeRendering, "Velocity")
Aufgabe41_1LocalizedFlowProbe.connect("Probe Points", Aufgabe41_1FlowProbeRendering, "Probe Points")
Aufgabe41_1LocalizedFlowProbe.connect("Divergence", Aufgabe41_1FlowProbeRendering, "Divergence")
Aufgabe41_1LocalizedFlowProbe.connect("Acceleration", Aufgabe41_1FlowProbeRendering, "Acceleration")
Aufgabe41_1LocalizedFlowProbe.connect("Curvature", Aufgabe41_1FlowProbeRendering, "Curvature")
Aufgabe41_1LocalizedFlowProbe.connect("Gradient", Aufgabe41_1FlowProbeRendering, "Gradient")

# Run the algorithm
Aufgabe41_1FlowProbeRendering.runBlocking()



