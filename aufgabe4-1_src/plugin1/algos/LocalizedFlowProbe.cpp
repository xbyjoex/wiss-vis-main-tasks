// JP, 01/2025 - Aufgabe 4.1: Localized Flow Probe
// Enhanced Visualization: Curved Tube + Ring Probe (de Leeuw & van Wijk style)

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

        Vector3 evaluateField( FieldEvaluator< 3, Vector3 >& evaluator, const Point3& position, double time )
        {
            evaluator.reset( position, time );
            if( !evaluator ) return Vector3( 0.0, 0.0, 0.0 );
            return evaluator.value();
        }

        // Compute Gradient Tensor (Jacobian)
        Tensor< double, 3, 3 > computeGradient( FieldEvaluator< 3, Vector3 >& evaluator, const Point3& p, double time, double h )
        {
            Vector3 v = evaluateField( evaluator, p, time );
            
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

        double computeDivergence( const Tensor< double, 3, 3 >& J )
        {
            return J(0,0) + J(1,1) + J(2,2);
        }

        Vector3 computeRotation( const Tensor< double, 3, 3 >& J )
        {
            return Vector3( J(2,1) - J(1,2), J(0,2) - J(2,0), J(1,0) - J(0,1) );
        }

        PointF<3> toPointF( const Point3& p ) { return PointF<3>( (float)p[0], (float)p[1], (float)p[2] ); }
    }

    // ==========================================================================================
    // ALGORITHM 1: Calculation (DataAlgorithm)
    // Computes full Flow Probe Data (Velocity, Acceleration, Gradient/Jacobian)
    // ==========================================================================================
    class LocalizedFlowProbe : public DataAlgorithm
    {
    public:
        struct Options : public DataAlgorithm::Options
        {
            Options( fantom::Options::Control& control ) : DataAlgorithm::Options( control )
            {
                add< Field< 3, Vector3 > >( "Vector Field", "Input flow field", Options::REQUIRED );
                add< double >( "Step Size", "Finite difference step", kDefaultStepSize );
                add< int >( "Sample Count", "Grid sampling resolution", 10 ); // Low default for discrete probes
                add< double >( "Time", "Evaluation time", 0.0 );
            }
        };

        struct DataOutputs : public DataAlgorithm::DataOutputs
        {
            DataOutputs( fantom::DataOutputs::Control& control ) : DataAlgorithm::DataOutputs( control )
            {
                add< const PointSet< 3 > >( "Probe Points" );
                add< const Function< Vector3 > >( "Velocity" );
                add< const Function< Vector3 > >( "Acceleration" ); // For curvature
                add< const Function< Tensor< double, 3, 3 > > >( "Gradient" ); // Full Jacobian
                add< const Function< double > >( "Divergence" ); // Convenience
            }
        };

        LocalizedFlowProbe( InitData& data ) : DataAlgorithm( data ) {}

        void execute( const Algorithm::Options& options, const volatile bool& abortFlag ) override
        {
            debugLog() << "Starting LocalizedFlowProbe Calculation..." << std::endl;
            
            auto field = options.get< Field< 3, Vector3 > >( "Vector Field" );
            auto function = options.get< Function< Vector3 > >( "Vector Field" );
            if( !field || !function ) { 
                debugLog() << "Error: Input Vector Field is missing." << std::endl;
                clearResults(); return; 
            }

            auto grid = std::dynamic_pointer_cast< const Grid< 3 > >( function->domain() );
            if( !grid ) {
                debugLog() << "Error: Field is not defined on a grid." << std::endl;
                 throw std::logic_error( "Field not on a grid." );
            }

            auto evaluator = field->makeEvaluator();
            if( !evaluator ) { clearResults(); return; }

            double time = options.get< double >( "Time" );
            double stepSize = options.get< double >( "Step Size" );
            int sampleCount = std::max( 1, options.get< int >( "Sample Count" ) );

            // Bounding Box
            const auto& gridPoints = grid->points();
            if( gridPoints.size() == 0 ) { clearResults(); return; }
            Point3 gridMin = gridPoints[0], gridMax = gridPoints[0];
            for( size_t i = 1; i < gridPoints.size(); ++i )
                for( int d = 0; d < 3; ++d ) {
                    gridMin[d] = std::min( gridMin[d], gridPoints[i][d] );
                    gridMax[d] = std::max( gridMax[d], gridPoints[i][d] );
                }
            
            debugLog() << "Grid Bounds: Min=" << gridMin << ", Max=" << gridMax << std::endl;

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

            Algorithm::Progress progress( *this, "Sampling Field", (countX+1)*(countY+1)*(countZ+1) );
            size_t pIdx = 0;

            for( int i = 0; i <= countX; ++i )
                for( int j = 0; j <= countY; ++j )
                    for( int k = 0; k <= countZ; ++k )
                    {
                        progress = ++pIdx;
                        if( abortFlag ) return;

                        Point3 p( gridMin[0] + i*spacing, gridMin[1] + j*spacing, gridMin[2] + k*spacing );
                        
                        evaluator->reset( p, time );
                        if( !*evaluator ) continue;

                        Vector3 v = evaluator->value();
                        if( norm( v ) < kMinDirectionNorm ) continue;

                        // Compute Gradient Tensor J
                        auto J = computeGradient( *evaluator, p, time, stepSize );
                        
                        // Compute Acceleration a = (v . grad) v = J * v
                        Vector3 a = J * v; 

                        points.push_back( p );
                        velocity.push_back( v );
                        acceleration.push_back( a );
                        gradient.push_back( J );
                        divergence.push_back( computeDivergence( J ) );
                    }
            
            debugLog() << "Generated " << points.size() << " valid probes." << std::endl;

            if( points.empty() ) { 
                debugLog() << "Warning: No probes generated (field might be zero)." << std::endl;
                clearResults(); return; 
            }

            auto pointSet = DomainFactory::makePointSet< 3 >( std::move( points ) );
            setResult( "Probe Points", pointSet );
            setResult( "Velocity", fantom::addData( pointSet, PointSet< 3 >::Points, velocity ) );
            setResult( "Acceleration", fantom::addData( pointSet, PointSet< 3 >::Points, acceleration ) );
            setResult( "Gradient", fantom::addData( pointSet, PointSet< 3 >::Points, gradient ) );
            setResult( "Divergence", fantom::addData( pointSet, PointSet< 3 >::Points, divergence ) );
            
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
                add< Function< Tensor< double, 3, 3 > > >( "Gradient", "Gradient Field (for Shear/Ring)" );
                add< Function< double > >( "Divergence", "Divergence Field (for Color)" );
                
                add< double >( "Glyph Scale", "Overall Size", 1.0 );
                add< double >( "Tube Length", "Relative length of each probe (0.1–1). Shorter = less clutter.", 0.25 );
                add< double >( "Ring Size", "Relative Ring Radius", 0.3 );
                add< double >( "Line Width", "Line thickness in pixels", 2.0 );
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
            debugLog() << "Starting FlowProbeRenderer..." << std::endl;
            auto pointSet = options.get< PointSet< 3 > >( "Probe Points" );
            auto velFunc = options.get< Function< Vector3 > >( "Velocity" );
            auto accFunc = options.get< Function< Vector3 > >( "Acceleration" ); 
            auto gradFunc = options.get< Function< Tensor< double, 3, 3 > > >( "Gradient" );
            auto divFunc = options.get< Function< double > >( "Divergence" ); 
            
            if( !pointSet || !velFunc ) { 
                debugLog() << "Error: Probe Points or Velocity Field missing." << std::endl;
                clearGraphics( "Flow Probes" ); return; 
            }
            
            debugLog() << "Input: " << pointSet->points().size() << " probe points." << std::endl;
            if( gradFunc ) debugLog() << "Gradient Field provided (Shear enabled)." << std::endl;

            double scale = options.get< double >( "Glyph Scale" );
            double tubeLength = std::max( 0.05, std::min( 1.0, options.get< double >( "Tube Length" ) ) );
            double ringRelSize = options.get< double >( "Ring Size" );
            double lineWidthOpt = std::max( 0.5, options.get< double >( "Line Width" ) );
            
            debugLog() << "Settings: Scale=" << scale << ", TubeLength=" << tubeLength << ", RingSize=" << ringRelSize << std::endl;
            
            const auto& points = pointSet->points();
            size_t numPoints = points.size();

            // First pass: get max velocity for length scaling
            double maxVel = 0.0;
            for( size_t i = 0; i < numPoints && i < velFunc->values().size(); ++i )
            {
                double vLen = norm( velFunc->values()[i] );
                if( vLen > maxVel ) maxVel = vLen;
            }
            if( maxVel < 1e-9 ) maxVel = 1.0; // avoid div by zero

            std::vector< PointF< 3 > > vertices;
            std::vector< Color > colors;
            std::vector< unsigned int > indices;

            for( size_t i = 0; i < numPoints; ++i )
            {
                if( abortFlag ) break;
                Point3 p = points[i];

                // 1. Get Velocity
                if( i >= velFunc->values().size() ) continue;
                Vector3 v = velFunc->values()[i];
                double vLen = norm( v );
                if( vLen < kMinDirectionNorm ) continue;

                // Tube length proportional to velocity magnitude (stronger flow = longer tube)
                double dt = scale * tubeLength * ( vLen / maxVel );

                // 2. Get Acceleration (if available)
                Vector3 a(0,0,0);
                if( accFunc && i < accFunc->values().size() ) a = accFunc->values()[i];

                // 3. Get Gradient (if available)
                Tensor< double, 3, 3 > J({0,0,0,0,0,0,0,0,0}); // Zero matrix
                if( gradFunc && i < gradFunc->values().size() ) J = gradFunc->values()[i];

                // 4. Get Divergence (if available)
                double div = 0.0;
                if( divFunc && i < divFunc->values().size() ) div = divFunc->values()[i];

                // --- Geometry Construction ---

                // A. Curved Tube (Streamline Segment)
                // x(t) = p + v*t + 0.5*a*t^2; dt already set above (scaled by magnitude)
                
                int segments = 10;
                Point3 prevPos = p;
                
                // Color Map: Blue (neg) -> Grey (0) -> Red (pos)
                double divSat = std::tanh( div * 5.0 ); 
                Color baseColor = ( divSat > 0 ) ? Color( 1.0, 1.0 - divSat, 1.0 - divSat ) 
                                                 : Color( 1.0 + divSat, 1.0 + divSat, 1.0 );

                // Draw Curve
                for( int s = 1; s <= segments; ++s )
                {
                    double t = (double)s / segments * dt;
                    Point3 currPos = p + v * t + a * ( 0.5 * t * t );

                    size_t idx = vertices.size();
                    vertices.push_back( toPointF( prevPos ) );
                    vertices.push_back( toPointF( currPos ) );
                    colors.push_back( baseColor );
                    colors.push_back( baseColor );
                    indices.push_back( idx );
                    indices.push_back( idx + 1 );

                    prevPos = currPos;
                }
                Point3 tipPos = prevPos;
                Vector3 tipDir = normalized( v + a * dt ); // Tangent at end
                double drawLen = norm( tipPos - p );      // Actual tube length for arrow head

                // B. Arrow Head (proportional to tube length so it stays visible)
                {
                    double headSize = std::max( drawLen * 0.25, scale * 0.05 ); // 25% of tube or min size
                    double headRadius = headSize * 0.5;
                    
                    Vector3 up( 0, 0, 1 );
                    if( std::abs( tipDir[2] ) > 0.9 ) up = Vector3( 0, 1, 0 );
                    Vector3 right = normalized( cross( tipDir, up ) );
                    up = normalized( cross( right, tipDir ) );
                    
                    Point3 base = tipPos - tipDir * headSize;
                    
                    for( int k = 0; k < 8; ++k ) 
                    {
                        double ang = k * M_PI / 4.0;
                        Vector3 off = ( right * std::cos(ang) + up * std::sin(ang) ) * headRadius;
                        size_t hIdx = vertices.size();
                        vertices.push_back( toPointF( tipPos ) );
                        vertices.push_back( toPointF( base + off ) );
                        colors.push_back( baseColor );
                        colors.push_back( baseColor );
                        indices.push_back( hIdx );
                        indices.push_back( hIdx + 1 );
                    }
                }

                // C. Ring at Base (Shear/Deformation)
                // We start with a circle perpendicular to v.
                // We deform it using the Gradient Tensor J.
                // r_new = r + J * r * dt (Linearized deformation)
                {
                    double ringRad = scale * ringRelSize; 
                    Vector3 dir = normalized( v );
                    Vector3 up( 0, 0, 1 );
                    if( std::abs( dir[2] ) > 0.9 ) up = Vector3( 0, 1, 0 );
                    Vector3 right = normalized( cross( dir, up ) );
                    up = normalized( cross( right, dir ) );
                    
                    int ringSegs = 20;
                    Point3 ringCenter = p; // Center at probe origin
                    
                    Color ringColor( 0.2, 0.2, 0.2 ); // Dark grey
                    
                    // Function to deform point: p' = p + J*(p-center)*dt
                    auto deform = [&]( const Point3& pt ) {
                        Vector3 r = pt - ringCenter;
                        Vector3 displacement = J * r * (dt * 0.5); // Deform over half time step for ring
                        return pt + displacement;
                    };

                    Point3 firstPt = ringCenter + right * ringRad;
                    Point3 prevRingPt = gradFunc ? deform(firstPt) : firstPt;

                    for( int k = 1; k <= ringSegs; ++k )
                    {
                        double ang = k * 2.0 * M_PI / ringSegs;
                        Point3 currPt = ringCenter + ( right * std::cos(ang) + up * std::sin(ang) ) * ringRad;
                        
                        // Apply deformation if Gradient is available
                        Point3 defPt = gradFunc ? deform(currPt) : currPt;
                        
                        size_t rIdx = vertices.size();
                        vertices.push_back( toPointF( prevRingPt ) );
                        vertices.push_back( toPointF( defPt ) );
                        colors.push_back( ringColor );
                        colors.push_back( ringColor );
                        indices.push_back( rIdx );
                        indices.push_back( rIdx + 1 );
                        
                        prevRingPt = defPt;
                    }
                }
            }
            
            debugLog() << "Max Velocity Magnitude: " << maxVel << std::endl;
            debugLog() << "Generated Geometry: " << vertices.size() << " vertices." << std::endl;

            auto const& system = graphics::GraphicsSystem::instance();
            std::string resourcePath = PluginRegistrationService::getInstance().getResourcePath( "utils/Graphics" );
            
            // multiColor line shader requires vertex → geometry → fragment (gColor → fColor)
            debugLog() << "Loading line shader (vertex + geometry + fragment)..." << std::endl;
            
             auto drawable = system.makePrimitive(
                graphics::PrimitiveConfig{ graphics::RenderPrimitives::LINES }
                    .vertexBuffer( "in_vertex", system.makeBuffer( vertices ) )
                    .vertexBuffer( "in_color", system.makeBuffer( colors ) )
                    .indexBuffer( system.makeIndexBuffer( indices ) )
                    .boundingSphere( graphics::computeBoundingSphere( vertices ) )
                    .uniform( "u_lineWidth", static_cast< float >( lineWidthOpt ) ),
                system.makeProgramFromFiles( resourcePath + "shader/line/noShading/multiColor/vertex.glsl",
                                             resourcePath + "shader/line/noShading/multiColor/fragment.glsl",
                                             resourcePath + "shader/line/noShading/multiColor/geometry.glsl" )
            );
            setGraphics( "Flow Probes", drawable );
            debugLog() << "Finished FlowProbeRenderer." << std::endl;
        }
    };

    AlgorithmRegister< FlowProbeRenderer > registerFlowRenderer(
        "Aufgabe4-1/1 Flow Probe Rendering",
        "Visualisiert Flow Probes (Curved Tube, Ring)." );

} // namespace aufgabe4_1
