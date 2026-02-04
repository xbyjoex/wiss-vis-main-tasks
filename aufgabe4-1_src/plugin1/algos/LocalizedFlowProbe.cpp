// Localized flow probe (de Leeuw & van Wijk 1993). DataAlgorithm + VisAlgorithm.

#include <algorithm>
#include <cmath>
#include <fantom/algorithm.hpp>
#include <fantom/datastructures/interfaces/Field.hpp>
#include <fantom/datastructures/domains/Grid.hpp>
#include <fantom/datastructures/domains/PointSet.hpp>
#include <fantom/datastructures/DomainFactory.hpp>
#include <fantom/datastructures/Function.hpp>
#include <fantom/graphics.hpp>
#include <fantom/math.hpp>
#include <fantom/register.hpp>
#include <fantom-plugins/utils/Graphics/HelperFunctions.hpp> 
#include <memory>
#include <string>
#include <vector>

namespace aufgabe4_1
{
    using namespace fantom;

    namespace
    {
        constexpr double kMinDirectionNorm = 1e-9;
        constexpr double kDefaultStepSize = 1e-4;

        // Evaluate velocity at one point; returns zero if point is outside the field.
        Vector3 evaluateField( FieldEvaluator< 3, Vector3 >& evaluator, const Point3& position, double time )
        {
            evaluator.reset( position, time );
            if( !evaluator ) return Vector3( 0.0, 0.0, 0.0 );
            return evaluator.value();
        }

        // Jacobian J: how velocity changes in x, y, z. Built from central differences (sample ±h along each axis).
        Tensor< double, 3, 3 > computeGradient( FieldEvaluator< 3, Vector3 >& evaluator, const Point3& p, double time, double h )
        {
            // Central differences
            Vector3 v_xp = evaluateField( evaluator, p + Vector3( h, 0.0, 0.0 ), time );
            Vector3 v_xm = evaluateField( evaluator, p - Vector3( h, 0.0, 0.0 ), time );
            Vector3 dv_dx = ( v_xp - v_xm ) / ( 2.0 * h );

            Vector3 v_yp = evaluateField( evaluator, p + Vector3( 0.0, h, 0.0 ), time );
            Vector3 v_ym = evaluateField( evaluator, p - Vector3( 0.0, h, 0.0 ), time );
            Vector3 dv_dy = ( v_yp - v_ym ) / ( 2.0 * h );

            Vector3 v_zp = evaluateField( evaluator, p + Vector3( 0.0, 0.0, h ), time );
            Vector3 v_zm = evaluateField( evaluator, p - Vector3( 0.0, 0.0, h ), time );
            Vector3 dv_dz = ( v_zp - v_zm ) / ( 2.0 * h );

            // J = [dv/dx, dv/dy, dv/dz] (columns)
            return Tensor< double, 3, 3 >( { dv_dx[0], dv_dy[0], dv_dz[0],
                                           dv_dx[1], dv_dy[1], dv_dz[1],
                                           dv_dx[2], dv_dy[2], dv_dz[2] } );
        }

        // Divergence = sum of diagonal of J. Positive = flow spreads out, negative = converges.
        double computeDivergence( const Tensor< double, 3, 3 >& J )
        {
            return J(0,0) + J(1,1) + J(2,2);
        }

        // Rotation (curl): from the off-diagonal entries of J. Measures local spin of the flow.
        Vector3 computeRotation( const Tensor< double, 3, 3 >& J )
        {
            return Vector3( J(2,1) - J(1,2), J(0,2) - J(2,0), J(1,0) - J(0,1) );
        }

        // Curvature: take the part of acceleration perpendicular to velocity, divide by speed. Gives how much the streamline bends (used for the arc).
        Vector3 computeCurvature( const Vector3& u, const Vector3& a )
        {
            double uu = u * u;
            if( uu < kMinDirectionNorm * kMinDirectionNorm ) return Vector3( 0, 0, 0 );
            Vector3 aPerp = a - u * ( ( u * a ) / uu );
            return aPerp / std::sqrt( uu );
        }

        PointF<3> toPointF( const Point3& p ) { return PointF<3>( (float)p[0], (float)p[1], (float)p[2] ); }

        // Distinct color per probe index (golden-ratio hue) so each probe is visually grouped
        Color probeColor( size_t index )
        {
            float h = std::fmod( index * 0.618033988749895f, 1.0f );
            float s = 0.88f, v = 1.0f;
            float c = v * s;
            float x = c * ( 1.0f - std::abs( std::fmod( h * 6.0f, 2.0f ) - 1.0f ) );
            float m = v - c;
            float r, g, b;
            if( h < 1.0f / 6.0f )      { r = c; g = x; b = 0.0f; }
            else if( h < 2.0f / 6.0f ) { r = x; g = c; b = 0.0f; }
            else if( h < 3.0f / 6.0f ) { r = 0.0f; g = c; b = x; }
            else if( h < 4.0f / 6.0f ) { r = 0.0f; g = x; b = c; }
            else if( h < 5.0f / 6.0f ) { r = x; g = 0.0f; b = c; }
            else                       { r = c; g = 0.0f; b = x; }
            return Color( r + m, g + m, b + m, 1.0f );
        }
    }

    class LocalizedFlowProbe : public DataAlgorithm
    {
    public:
        struct Options : public DataAlgorithm::Options
        {
            Options( fantom::Options::Control& control ) : DataAlgorithm::Options( control )
            {
                add< Field< 3, Vector3 > >( "Vector Field", "Input flow field", Options::REQUIRED );
                add< double >( "Step Size", "Finite difference step", kDefaultStepSize );
                add< int >( "Sample Count", "Probes per axis (2–3 = clear arrows; 5+ = dense)", 3 );
                add< double >( "Time", "Evaluation time", 0.0 );
            }
        };

        struct DataOutputs : public DataAlgorithm::DataOutputs
        {
            DataOutputs( fantom::DataOutputs::Control& control ) : DataAlgorithm::DataOutputs( control )
            {
                add< const PointSet< 3 > >( "Probe Points" );
                add< const Function< Vector3 > >( "Velocity" );
                add< const Function< Vector3 > >( "Acceleration" );
                add< const Function< Tensor< double, 3, 3 > > >( "Gradient" );
                add< const Function< double > >( "Divergence" );
                add< const Function< Vector3 > >( "Curvature" );
            }
        };

        LocalizedFlowProbe( InitData& data ) : DataAlgorithm( data ) {}

        void execute( const Algorithm::Options& options, const volatile bool& abortFlag ) override
        {
            debugLog() << "Starting LocalizedFlowProbe Calculation..." << std::endl;
            
            // We need a vector field as input (e.g. from a file loader).
            auto field = options.get< Field< 3, Vector3 > >( "Vector Field" );
            auto function = options.get< Function< Vector3 > >( "Vector Field" );
            if( !field || !function ) {
                debugLog() << "Error: Input Vector Field is missing." << std::endl;
                clearResults(); return; 
            }

            // The field must be defined on a 3D grid so we know where we can sample.
            auto grid = std::dynamic_pointer_cast< const Grid< 3 > >( function->domain() );
            if( !grid ) {
                debugLog() << "Error: Field is not defined on a grid." << std::endl;
                 throw std::logic_error( "Field not on a grid." );
            }

            // Evaluator lets us ask "what is the velocity at this point?"
            auto evaluator = field->makeEvaluator();
            if( !evaluator ) { clearResults(); return; }

            double time = options.get< double >( "Time" );
            double stepSize = options.get< double >( "Step Size" );
            int sampleCount = std::max( 1, options.get< int >( "Sample Count" ) );

            // Find the box that contains all grid points (min and max x, y, z).
            const auto& gridPoints = grid->points();
            if( gridPoints.size() == 0 ) { clearResults(); return; }
            Point3 gridMin = gridPoints[0], gridMax = gridPoints[0];
            for( size_t i = 1; i < gridPoints.size(); ++i )
                for( int d = 0; d < 3; ++d ) {
                    gridMin[d] = std::min( gridMin[d], gridPoints[i][d] );
                    gridMax[d] = std::max( gridMax[d], gridPoints[i][d] );
                }
            
            debugLog() << "Grid Bounds: Min=" << gridMin << ", Max=" << gridMax << std::endl;

            // Distance between probe positions. If a dimension has zero size we don't sample along it.
            Vector3 gridSize = gridMax - gridMin;
            double maxDim = std::max( { gridSize[0], gridSize[1], gridSize[2] } );
            double spacing = maxDim / static_cast< double >( sampleCount + 1 );
            int countX = ( gridSize[0] < 1e-6 ) ? 0 : sampleCount;
            int countY = ( gridSize[1] < 1e-6 ) ? 0 : sampleCount;
            int countZ = ( gridSize[2] < 1e-6 ) ? 0 : sampleCount;
            
            debugLog() << "Sampling Grid: " << (countX+1) << "x" << (countY+1) << "x" << (countZ+1) << " probes. Spacing: " << spacing << std::endl;

            std::vector< Point3 > points;
            std::vector< Vector3 > velocity;
            std::vector< Vector3 > acceleration;
            std::vector< Tensor< double, 3, 3 > > gradient;
            std::vector< double > divergence;
            std::vector< Vector3 > curvature;

            // Sample grid; at each point get v, J, a, div, curvature (skip zero velocity).
            Algorithm::Progress progress( *this, "Sampling Field", (countX+1)*(countY+1)*(countZ+1) );
            size_t pIdx = 0;

            for( int i = 0; i <= countX; ++i )
                for( int j = 0; j <= countY; ++j )
                    for( int k = 0; k <= countZ; ++k )
                    {
                        progress = ++pIdx;
                        if( abortFlag ) return;

                        // Position of this probe in 3D.
                        Point3 p( gridMin[0] + i*spacing, gridMin[1] + j*spacing, gridMin[2] + k*spacing );

                        evaluator->reset( p, time );
                        if( !*evaluator ) continue;

                        Vector3 v = evaluator->value();
                        if( norm( v ) < kMinDirectionNorm ) continue;

                        // Gradient J and acceleration a = J*v (how velocity changes along the flow).
                        auto J = computeGradient( *evaluator, p, time, stepSize );
                        Vector3 a = J * v; 

                        points.push_back( p );
                        velocity.push_back( v );
                        acceleration.push_back( a );
                        gradient.push_back( J );
                        divergence.push_back( computeDivergence( J ) );
                        curvature.push_back( computeCurvature( v, a ) );
                    }

            debugLog() << "Generated " << points.size() << " valid probes." << std::endl;

            // If no valid probes (e.g. field is zero everywhere), stop.
            if( points.empty() ) {
                debugLog() << "Warning: No probes generated (field might be zero)." << std::endl;
                clearResults(); return; 
            }

            // Pack everything into a point set and attached functions so the renderer can use them.
            auto pointSet = DomainFactory::makePointSet< 3 >( std::move( points ) );
            setResult( "Probe Points", pointSet );
            setResult( "Velocity", fantom::addData( pointSet, PointSet< 3 >::Points, velocity ) );
            setResult( "Acceleration", fantom::addData( pointSet, PointSet< 3 >::Points, acceleration ) );
            setResult( "Gradient", fantom::addData( pointSet, PointSet< 3 >::Points, gradient ) );
            setResult( "Divergence", fantom::addData( pointSet, PointSet< 3 >::Points, divergence ) );
            setResult( "Curvature", fantom::addData( pointSet, PointSet< 3 >::Points, curvature ) );

            debugLog() << "Finished LocalizedFlowProbe Calculation." << std::endl;
        }
    };

    AlgorithmRegister< LocalizedFlowProbe > registerFlowProbe(
        "Aufgabe4-1/1 Localized Flow Probe",
        "Berechnet Velocity, Acceleration und Gradient für Flow Probes." );


    // ==========================================================================================
    // ALGORITHM 2: Visualization (VisAlgorithm)
    // Renders the Complex Flow Probe Glyph (Curved Tube + Ring)
    // ==========================================================================================
    class FlowProbeRenderer : public VisAlgorithm
    {
    public:
        struct Options : public VisAlgorithm::Options
        {
            Options( fantom::Options::Control& control ) : VisAlgorithm::Options( control )
            {
                add< PointSet< 3 > >( "Probe Points", "Sample points", Options::REQUIRED );
                add< Function< Vector3 > >( "Velocity", "Velocity Field", Options::REQUIRED );
                add< Function< Vector3 > >( "Acceleration", "Acceleration Field (for Curvature)" );
                add< Function< Tensor< double, 3, 3 > > >( "Gradient", "Gradient Field (Shear/Torsion)" );
                add< Function< double > >( "Divergence", "Divergence (lens/color)" );
                add< Function< Vector3 > >( "Curvature", "Curvature vector (optional; else from Velocity+Acceleration)" );
                add< double >( "Glyph Scale", "Overall size (smaller = less overlap)", 0.5 );
                add< double >( "Tube Length", "Shaft length factor (0.1–0.5)", 0.2 );
                add< double >( "Ring Size", "Ring/lens radius factor", 0.2 );
                add< double >( "Line Width", "Line width in pixels (thicker = easier to see which belongs to which)", 3.0 );
                add< bool >( "Show Tube", "Tube with torsion stripes", true );
                add< bool >( "Show Membrane", "Acceleration disc at tip", true );
                add< bool >( "Show Lens", "Divergence paraboloid at base", true );
                add< bool >( "Color by Probe ID", "One color per probe (arc/ring/head grouped); off = by divergence", true );
            }
        };

        struct VisOutputs : public VisAlgorithm::VisOutputs
        {
            VisOutputs( fantom::VisOutputs::Control& control ) : VisAlgorithm::VisOutputs( control )
            {
                addGraphics( "Flow Probes" );
            }
        };

        FlowProbeRenderer( InitData& data ) : VisAlgorithm( data ) {}

        void execute( const Algorithm::Options& options, const volatile bool& abortFlag ) override
        {
            // Read the data produced by the Localized Flow Probe algorithm (points + velocity, etc.).
            auto pointSet = options.get< PointSet< 3 > >( "Probe Points" );
            auto velFunc = options.get< Function< Vector3 > >( "Velocity" );
            auto accFunc = options.get< Function< Vector3 > >( "Acceleration" );
            auto gradFunc = options.get< Function< Tensor< double, 3, 3 > > >( "Gradient" );
            auto divFunc = options.get< Function< double > >( "Divergence" );
            auto curvFunc = options.get< Function< Vector3 > >( "Curvature" );
            if( !pointSet || !velFunc ) { clearGraphics( "Flow Probes" ); return; }

            // User options: overall size, shaft length, ring size, line thickness, and which parts to show.
            double scale = options.get< double >( "Glyph Scale" );
            double tubeLength = std::max( 0.05, std::min( 1.0, options.get< double >( "Tube Length" ) ) );
            double ringRelSize = options.get< double >( "Ring Size" );
            double lineWidthOpt = std::max( 0.5, options.get< double >( "Line Width" ) );
            bool showTube = options.get< bool >( "Show Tube" );
            bool showMembrane = options.get< bool >( "Show Membrane" );
            bool showLens = options.get< bool >( "Show Lens" );
            bool colorByProbeId = options.get< bool >( "Color by Probe ID" );
            const auto& points = pointSet->points();
            size_t numPoints = points.size();

            // We need the maximum velocity to scale shaft length (faster flow = longer shaft).
            double maxVel = 0.0;
            for( size_t i = 0; i < numPoints && i < velFunc->values().size(); ++i )
            {
                double vLen = norm( velFunc->values()[i] );
                if( vLen > maxVel ) maxVel = vLen;
            }
            if( maxVel < 1e-9 ) maxVel = 1.0;

            // Number of segments for arc, tube cross-section, lens: more = smoother but heavier.
            constexpr double kappaEps = 1e-6;
            const int arcSegs = 16;
            const int tubeCirc = 10;
            const int paraboloidRings = 6;
            const int paraboloidSegs = 12;

            // We fill these with vertices and indices; lines for arc/head/ring, triangles for tube/membrane/lens.
            std::vector< PointF< 3 > > lineVerts;
            std::vector< Color > lineColors;
            std::vector< unsigned int > lineIndices;
            std::vector< PointF< 3 > > triVerts;
            std::vector< VectorF< 3 > > triNormals;
            std::vector< Color > triColors;
            std::vector< unsigned int > triIndices;

            for( size_t i = 0; i < numPoints; ++i )
            {
                if( abortFlag ) break;
                Point3 p = points[i];
                if( i >= velFunc->values().size() ) continue;
                Vector3 v = velFunc->values()[i];
                double vLen = norm( v );
                if( vLen < kMinDirectionNorm ) continue;

                // Shaft length in "time" units: scaled by speed so faster flow gets a longer arrow.
                double dt = scale * tubeLength * ( vLen / maxVel );
                Vector3 a( 0, 0, 0 );
                if( accFunc && i < accFunc->values().size() ) a = accFunc->values()[i];
                Tensor< double, 3, 3 > J( { 0,0,0, 0,0,0, 0,0,0 } );
                if( gradFunc && i < gradFunc->values().size() ) J = gradFunc->values()[i];
                double div = 0.0;
                if( divFunc && i < divFunc->values().size() ) div = divFunc->values()[i];
                Vector3 c = ( curvFunc && i < curvFunc->values().size() ) ? curvFunc->values()[i] : computeCurvature( v, a );

                // Color: by divergence (red = spreading, blue = converging) or one color per probe.
                double divSat = std::tanh( div * 5.0 );
                Color divColor = ( divSat > 0 ) ? Color( 1.0f, 1.0f - (float)divSat, 1.0f - (float)divSat )
                                                : Color( 1.0f + (float)divSat, 1.0f + (float)divSat, 1.0f );
                Color lineColor = colorByProbeId ? probeColor( i ) : divColor;
                Color baseColor = colorByProbeId ? lineColor : divColor;
                // Curl along flow direction: drives the torsion stripe pattern on the tube.
                Vector3 curlVec = computeRotation( J );
                double torsionProxy = ( vLen > 1e-12 ) ? ( curlVec * v ) / vLen : 0.0;

                // Shaft = arc of osculating circle (radius 1/kappa, length L); fallback to straight segment.
                Vector3 T = normalized( v );
                double kappa = norm( c );
                double L = vLen * dt;
                std::vector< Point3 > arcPoints;
                if( kappa < kappaEps || L < 1e-12 )
                {
                    arcPoints.push_back( p );
                    arcPoints.push_back( p + T * L );
                }
                else
                {
                    double R = 1.0 / kappa;
                    Vector3 N = normalized( c );
                    Point3 center = p + R * N;
                    double thetaMax = L / R;
                    for( int s = 0; s <= arcSegs; ++s )
                    {
                        double theta = ( s * thetaMax ) / arcSegs;
                        arcPoints.push_back( center + R * ( -std::cos( theta ) * N + std::sin( theta ) * T ) );
                    }
                }
                // Where the arrow ends and which way it points (for head and membrane).
                Point3 tipPos = arcPoints.back();
                Vector3 tipDir = ( arcPoints.size() >= 2 )
                    ? normalized( arcPoints.back() - arcPoints[ arcPoints.size() - 2 ] ) : T;
                double drawLen = L;

                // Draw the arc (shaft) and the arrow head as line segments (each segment = two vertices).
                for( size_t s = 0; s + 1 < arcPoints.size(); ++s )
                {
                    size_t idx = lineVerts.size();
                    lineVerts.push_back( toPointF( arcPoints[s] ) );
                    lineVerts.push_back( toPointF( arcPoints[s + 1] ) );
                    lineColors.push_back( lineColor );
                    lineColors.push_back( lineColor );
                    lineIndices.push_back( (unsigned int)idx );
                    lineIndices.push_back( (unsigned int)idx + 1 );
                }

                // Arrow head: small cone from tip backward. We need two perpendicular directions in the plane of the cone base.
                double headSize = std::max( drawLen * 0.25, scale * 0.05 );
                double headRad = headSize * 0.5;
                Vector3 up( 0, 0, 1 );
                if( std::abs( tipDir[2] ) > 0.9 ) up = Vector3( 0, 1, 0 );
                Vector3 right = normalized( cross( tipDir, up ) );
                up = normalized( cross( right, tipDir ) );
                Point3 base = tipPos - tipDir * headSize;
                for( int k = 0; k < 8; ++k )
                {
                    double ang = k * M_PI / 4.0;
                    Vector3 off = ( right * std::cos( ang ) + up * std::sin( ang ) ) * headRad;
                    size_t hIdx = lineVerts.size();
                    lineVerts.push_back( toPointF( tipPos ) );
                    lineVerts.push_back( toPointF( base + off ) );
                    lineColors.push_back( lineColor );
                    lineColors.push_back( lineColor );
                    lineIndices.push_back( (unsigned int)hIdx );
                    lineIndices.push_back( (unsigned int)hIdx + 1 );
                }

                // Shear ring in plane perpendicular to flow; J deforms it (Frenet frame if curvature available).
                double ringRad = scale * ringRelSize;
                Vector3 dir = normalized( v );
                Vector3 ringUp( 0, 0, 1 );
                if( std::abs( dir[2] ) > 0.9 ) ringUp = Vector3( 0, 1, 0 );
                Vector3 ringRight = normalized( cross( dir, ringUp ) );
                ringUp = normalized( cross( ringRight, dir ) );
                if( kappa >= kappaEps ) { ringRight = normalized( cross( dir, c ) ); ringUp = normalized( cross( ringRight, dir ) ); }
                Point3 ringCenter = p;
                Color ringColor = colorByProbeId ? lineColor : Color( 0.2f, 0.2f, 0.2f );
                auto deform = [&]( const Point3& pt ) {
                    Vector3 r = pt - ringCenter;
                    return pt + J * r * ( dt * 0.5 );
                };
                // Draw the ring as 20 line segments; each segment connects two points on the (possibly deformed) circle.
                int ringSegs = 20;
                Point3 prevRingPt = ringCenter + ringRight * ringRad;
                if( gradFunc ) prevRingPt = deform( prevRingPt );
                for( int k = 1; k <= ringSegs; ++k )
                {
                    double ang = k * 2.0 * M_PI / ringSegs;
                    Point3 currPt = ringCenter + ( ringRight * std::cos( ang ) + ringUp * std::sin( ang ) ) * ringRad;
                    if( gradFunc ) currPt = deform( currPt );
                    size_t rIdx = lineVerts.size();
                    lineVerts.push_back( toPointF( prevRingPt ) );
                    lineVerts.push_back( toPointF( currPt ) );
                    lineColors.push_back( ringColor );
                    lineColors.push_back( ringColor );
                    lineIndices.push_back( (unsigned int)rIdx );
                    lineIndices.push_back( (unsigned int)rIdx + 1 );
                    prevRingPt = currPt;
                }

                // Tube along arc; stripe phase from torsion proxy (curl·u) for candy-stripe effect.
                double tubeRad = std::max( drawLen * 0.04, scale * 0.02 );
                size_t tubeBaseIdx = triVerts.size();
                double arcLenSoFar = 0.0;
                if( showTube )
                {
                // At each point along the arc we add a circle of vertices (tube cross-section). Tangent and two perpendiculars define the circle.
                for( size_t s = 0; s < arcPoints.size(); ++s )
                {
                    Vector3 tangent = ( s + 1 < arcPoints.size() )
                        ? normalized( arcPoints[s + 1] - arcPoints[s] )
                        : ( s > 0 ? normalized( arcPoints[s] - arcPoints[s - 1] ) : T );
                    Vector3 tu( 0, 0, 1 );
                    if( std::abs( tangent[2] ) > 0.9 ) tu = Vector3( 0, 1, 0 );
                    Vector3 tx = normalized( cross( tangent, tu ) );
                    Vector3 ty = normalized( cross( tx, tangent ) );
                    for( int k = 0; k <= tubeCirc; ++k )
                    {
                        double phi = k * 2.0 * M_PI / tubeCirc;
                        Point3 pt = arcPoints[s] + ( tx * std::cos( phi ) + ty * std::sin( phi ) ) * tubeRad;
                        triVerts.push_back( toPointF( pt ) );
                        Vector3 n = normalized( tx * std::cos( phi ) + ty * std::sin( phi ) );
                        triNormals.push_back( VectorF<3>( (float)n[0], (float)n[1], (float)n[2] ) );
                        float stripe = 0.5f + 0.5f * std::cos( (float)( phi + torsionProxy * arcLenSoFar * 2.0 ) );
                        triColors.push_back( Color( stripe * baseColor.r(), stripe * baseColor.g(), stripe * baseColor.b(), 1.0f ) );
                    }
                    if( s + 1 < arcPoints.size() ) arcLenSoFar += norm( arcPoints[s + 1] - arcPoints[s] );
                }
                // Connect the tube circles into quads, each quad as two triangles (indices).
                for( size_t s = 0; s + 1 < arcPoints.size(); ++s )
                {
                    for( int k = 0; k < tubeCirc; ++k )
                    {
                        int k1 = ( k + 1 ) % ( tubeCirc + 1 );
                        unsigned int i00 = (unsigned int)( tubeBaseIdx + s * ( tubeCirc + 1 ) + k );
                        unsigned int i01 = (unsigned int)( tubeBaseIdx + s * ( tubeCirc + 1 ) + k1 );
                        unsigned int i10 = (unsigned int)( tubeBaseIdx + ( s + 1 ) * ( tubeCirc + 1 ) + k );
                        unsigned int i11 = (unsigned int)( tubeBaseIdx + ( s + 1 ) * ( tubeCirc + 1 ) + k1 );
                        triIndices.push_back( i00 ); triIndices.push_back( i01 ); triIndices.push_back( i10 );
                        triIndices.push_back( i01 ); triIndices.push_back( i11 ); triIndices.push_back( i10 );
                    }
                }
                }

                // Acceleration disc at tip; bulge along flow from a·u.
                double aAlongU = ( vLen > 1e-12 ) ? ( a * v ) / vLen : 0.0;
                if( showMembrane )
                {
                // Membrane: disc at tip. Center is shifted along flow by acceleration (bulge); ring of points around it.
                double memRad = scale * ringRelSize * 0.8;
                int memSegs = 16;
                size_t memBase = triVerts.size();
                double bulge = 0.15 * scale * std::tanh( aAlongU * 2.0 );
                triVerts.push_back( toPointF( tipPos + tipDir * bulge ) );
                triNormals.push_back( VectorF<3>( (float)tipDir[0], (float)tipDir[1], (float)tipDir[2] ) );
                triColors.push_back( baseColor );
                for( int k = 0; k <= memSegs; ++k )
                {
                    double ang = k * 2.0 * M_PI / memSegs;
                    Point3 pt = tipPos + ( right * std::cos( ang ) + up * std::sin( ang ) ) * memRad;
                    triVerts.push_back( toPointF( pt ) );
                    Vector3 n = normalized( -tipDir );
                    triNormals.push_back( VectorF<3>( (float)n[0], (float)n[1], (float)n[2] ) );
                    triColors.push_back( baseColor );
                }
                // Triangle fan: center to each consecutive pair of ring points.
                for( int k = 0; k < memSegs; ++k )
                {
                    triIndices.push_back( (unsigned int)memBase );
                    triIndices.push_back( (unsigned int)( memBase + k + 1 ) );
                    triIndices.push_back( (unsigned int)( memBase + ( k + 2 <= memSegs + 1 ? k + 2 : 1 ) ) );
                }
                }

                // Lens (paraboloid) at probe base: center vertex plus rings; height follows divergence (dome or bowl).
                if( showLens )
                {
                double lensRad = scale * ringRelSize;
                double kDiv = 0.08 * scale * std::tanh( div * 3.0 );
                size_t lensBase = triVerts.size();
                triVerts.push_back( toPointF( p ) );
                triNormals.push_back( VectorF<3>( (float)dir[0], (float)dir[1], (float)dir[2] ) );
                triColors.push_back( baseColor );
                for( int r = 1; r <= paraboloidRings; ++r )
                {
                    double rad = ( r * lensRad ) / paraboloidRings;
                    double z = kDiv * ( rad * rad ) / ( lensRad * lensRad + 1e-12 );
                    for( int seg = 0; seg < paraboloidSegs; ++seg )
                    {
                        double ang = seg * 2.0 * M_PI / paraboloidSegs;
                        Point3 pt = p + ( ringRight * std::cos( ang ) + ringUp * std::sin( ang ) ) * rad + dir * z;
                        triVerts.push_back( toPointF( pt ) );
                        double dzdr = ( lensRad > 1e-12 && rad > 1e-12 ) ? ( 2.0 * kDiv * rad / ( lensRad * lensRad ) ) : 0.0;
                        Vector3 radial = normalized( ringRight * std::cos( ang ) + ringUp * std::sin( ang ) );
                        Vector3 n = ( rad > 1e-12 ) ? normalized( radial - dir * dzdr ) : dir;
                        triNormals.push_back( VectorF<3>( (float)n[0], (float)n[1], (float)n[2] ) );
                        triColors.push_back( baseColor );
                    }
                }
                // Triangles from center to first ring.
                for( int seg = 0; seg < paraboloidSegs; ++seg )
                {
                    int seg1 = ( seg + 1 ) % paraboloidSegs;
                    triIndices.push_back( (unsigned int)lensBase );
                    triIndices.push_back( (unsigned int)( lensBase + 1 + seg ) );
                    triIndices.push_back( (unsigned int)( lensBase + 1 + seg1 ) );
                }
                // Triangles between consecutive rings.
                for( int r = 0; r < paraboloidRings - 1; ++r )
                {
                    for( int seg = 0; seg < paraboloidSegs; ++seg )
                    {
                        int seg1 = ( seg + 1 ) % paraboloidSegs;
                        unsigned int a0 = (unsigned int)( lensBase + 1 + r * paraboloidSegs + seg );
                        unsigned int a1 = (unsigned int)( lensBase + 1 + r * paraboloidSegs + seg1 );
                        unsigned int b0 = (unsigned int)( lensBase + 1 + ( r + 1 ) * paraboloidSegs + seg );
                        unsigned int b1 = (unsigned int)( lensBase + 1 + ( r + 1 ) * paraboloidSegs + seg1 );
                        triIndices.push_back( a0 ); triIndices.push_back( a1 ); triIndices.push_back( b0 );
                        triIndices.push_back( a1 ); triIndices.push_back( b1 ); triIndices.push_back( b0 );
                    }
                }
                }
            }

            // One drawable for lines (arc, head, ring), one for triangles (tube, membrane, lens); compound to single output.
            auto const& system = graphics::GraphicsSystem::instance();
            std::string resourcePath = PluginRegistrationService::getInstance().getResourcePath( "utils/Graphics" );
            if( !resourcePath.empty() && resourcePath.back() != '/' ) resourcePath += "/";

            std::vector< std::shared_ptr< graphics::Drawable > > drawables;
            // Lines (arc, head, ring): load line shader, create one drawable with vertex/color/index buffers.
            if( !lineVerts.empty() )
            {
                auto lineProg = system.makeProgramFromFiles(
                    resourcePath + "shader/line/noShading/multiColor/vertex.glsl",
                    resourcePath + "shader/line/noShading/multiColor/fragment.glsl",
                    resourcePath + "shader/line/noShading/multiColor/geometry.glsl" );
                auto lineDraw = system.makePrimitive(
                    graphics::PrimitiveConfig{ graphics::RenderPrimitives::LINES }
                        .vertexBuffer( "in_vertex", system.makeBuffer( lineVerts ) )
                        .vertexBuffer( "in_color", system.makeBuffer( lineColors ) )
                        .indexBuffer( system.makeIndexBuffer( lineIndices ) )
                        .boundingSphere( graphics::computeBoundingSphere( lineVerts ) )
                        .uniform( "u_lineWidth", static_cast< float >( lineWidthOpt ) ),
                    lineProg );
                drawables.push_back( lineDraw );
            }
            // Triangles (tube, membrane, lens): Phong shader with normals and colors; one drawable.
            if( !triVerts.empty() )
            {
                auto triProg = system.makeProgramFromFiles(
                    resourcePath + "shader/surface/phong/multiColor/vertex.glsl",
                    resourcePath + "shader/surface/phong/multiColor/fragment.glsl" );
                auto triDraw = system.makePrimitive(
                    graphics::PrimitiveConfig{ graphics::RenderPrimitives::TRIANGLES }
                        .vertexBuffer( "position", system.makeBuffer( triVerts ) )
                        .vertexBuffer( "normal", system.makeBuffer( triNormals ) )
                        .vertexBuffer( "color", system.makeBuffer( triColors ) )
                        .indexBuffer( system.makeIndexBuffer( triIndices ) )
                        .boundingSphere( graphics::computeBoundingSphere( triVerts ) ),
                    triProg );
                drawables.push_back( triDraw );
            }
            if( drawables.empty() ) { clearGraphics( "Flow Probes" ); return; }
            setGraphics( "Flow Probes", graphics::makeCompound( drawables ) );
        }
    };

    AlgorithmRegister< FlowProbeRenderer > registerFlowRenderer(
        "Aufgabe4-1/1 Flow Probe Rendering",
        "Visualisiert Flow Probes (Curved Tube, Ring)." );

} // namespace aufgabe4_1
