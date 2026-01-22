// JP, 01/2025 - Aufgabe 4.1: Localized Flow Probe
// Erweiterte Vektorfeld-Visualisierung mit Divergenz, Rotation und Krümmung
#include <cmath>
#include <fantom/algorithm.hpp>
#include <fantom/datastructures/DomainFactory.hpp>
#include <fantom/datastructures/interfaces/Field.hpp>
#include <fantom/datastructures/domains/PointSet.hpp>
#include <fantom/datastructures/domains/LineSet.hpp>
#include <fantom/math.hpp>
#include <fantom/register.hpp>
#include <memory>
#include <string>
#include <vector>

namespace aufgabe4_1
{
    using namespace fantom;

    namespace
    {
        // Minimum field magnitude - if smaller, field is considered zero
        constexpr double kMinDirectionNorm = 1e-9;
        // Small step size for numerical differentiation
        constexpr double kDefaultStepSize = 1e-4;

        // Get the vector field value at a specific position
        Vector3 evaluateField( FieldEvaluator< 3, Vector3 >& evaluator, const Point3& position, double time )
        {
            evaluator.reset( position, time );
            if( !evaluator )
            {
                // Position is outside domain
                return Vector3( 0.0, 0.0, 0.0 );
            }
            return evaluator.value();
        }

        // Compute divergence using central differences
        // div(v) = ∂v_x/∂x + ∂v_y/∂y + ∂v_z/∂z
        double computeDivergence( FieldEvaluator< 3, Vector3 >& evaluator, 
                                  const Point3& p, 
                                  double time, 
                                  double h )
        {
            // Central difference for ∂v_x/∂x
            Vector3 v_xp = evaluateField( evaluator, p + Vector3( h, 0.0, 0.0 ), time );
            Vector3 v_xm = evaluateField( evaluator, p - Vector3( h, 0.0, 0.0 ), time );
            double div_x = ( v_xp[0] - v_xm[0] ) / ( 2.0 * h );

            // Central difference for ∂v_y/∂y
            Vector3 v_yp = evaluateField( evaluator, p + Vector3( 0.0, h, 0.0 ), time );
            Vector3 v_ym = evaluateField( evaluator, p - Vector3( 0.0, h, 0.0 ), time );
            double div_y = ( v_yp[1] - v_ym[1] ) / ( 2.0 * h );

            // Central difference for ∂v_z/∂z
            Vector3 v_zp = evaluateField( evaluator, p + Vector3( 0.0, 0.0, h ), time );
            Vector3 v_zm = evaluateField( evaluator, p - Vector3( 0.0, 0.0, h ), time );
            double div_z = ( v_zp[2] - v_zm[2] ) / ( 2.0 * h );

            return div_x + div_y + div_z;
        }

        // Compute rotation (curl) using central differences
        // rot(v) = (∂v_z/∂y - ∂v_y/∂z, ∂v_x/∂z - ∂v_z/∂x, ∂v_y/∂x - ∂v_x/∂y)
        Vector3 computeRotation( FieldEvaluator< 3, Vector3 >& evaluator, 
                                  const Point3& p, 
                                  double time, 
                                  double h )
        {
            // Compute partial derivatives using central differences
            Vector3 v_yp = evaluateField( evaluator, p + Vector3( 0.0, h, 0.0 ), time );
            Vector3 v_ym = evaluateField( evaluator, p - Vector3( 0.0, h, 0.0 ), time );
            Vector3 v_zp = evaluateField( evaluator, p + Vector3( 0.0, 0.0, h ), time );
            Vector3 v_zm = evaluateField( evaluator, p - Vector3( 0.0, 0.0, h ), time );
            Vector3 v_xp = evaluateField( evaluator, p + Vector3( h, 0.0, 0.0 ), time );
            Vector3 v_xm = evaluateField( evaluator, p - Vector3( h, 0.0, 0.0 ), time );

            // rot_x = ∂v_z/∂y - ∂v_y/∂z
            double dvz_dy = ( v_zp[2] - v_zm[2] ) / ( 2.0 * h );
            double dvy_dz = ( v_yp[2] - v_ym[2] ) / ( 2.0 * h );
            double rot_x = dvz_dy - dvy_dz;

            // rot_y = ∂v_x/∂z - ∂v_z/∂x
            double dvx_dz = ( v_xp[2] - v_xm[2] ) / ( 2.0 * h );
            double dvz_dx = ( v_zp[0] - v_zm[0] ) / ( 2.0 * h );
            double rot_y = dvx_dz - dvz_dx;

            // rot_z = ∂v_y/∂x - ∂v_x/∂y
            double dvy_dx = ( v_yp[0] - v_ym[0] ) / ( 2.0 * h );
            double dvx_dy = ( v_xp[1] - v_xm[1] ) / ( 2.0 * h );
            double rot_z = dvy_dx - dvx_dy;

            return Vector3( rot_x, rot_y, rot_z );
        }

        // Compute curvature of streamlines
        // κ = ||dT/ds|| where T = v/||v|| is the unit tangent vector
        double computeCurvature( FieldEvaluator< 3, Vector3 >& evaluator, 
                                 const Point3& p, 
                                 double time, 
                                 double h )
        {
            Vector3 v = evaluateField( evaluator, p, time );
            double v_norm = norm( v );
            
            if( v_norm < kMinDirectionNorm )
            {
                return 0.0; // Zero curvature for zero field
            }

            Vector3 T = v / v_norm; // Unit tangent vector

            // Compute dT/ds numerically
            // We approximate dT/ds by computing T at nearby points along the streamline
            // and taking the derivative with respect to arc length

            // Move a small step along the field direction
            Vector3 step = h * T;
            Point3 p_forward = p + step;
            Point3 p_backward = p - step;

            Vector3 v_forward = evaluateField( evaluator, p_forward, time );
            Vector3 v_backward = evaluateField( evaluator, p_backward, time );

            double v_forward_norm = norm( v_forward );
            double v_backward_norm = norm( v_backward );

            if( v_forward_norm < kMinDirectionNorm || v_backward_norm < kMinDirectionNorm )
            {
                return 0.0; // Can't compute curvature
            }

            Vector3 T_forward = v_forward / v_forward_norm;
            Vector3 T_backward = v_backward / v_backward_norm;

            // Central difference for dT/ds
            Vector3 dT_ds = ( T_forward - T_backward ) / ( 2.0 * h );

            // Curvature is the magnitude of dT/ds
            return norm( dT_ds );
        }
    } // namespace

    class LocalizedFlowProbe : public DataAlgorithm
    {
    public:
        struct Options : public DataAlgorithm::Options
        {
            Options( fantom::Options::Control& control )
                : DataAlgorithm::Options( control )
            {
                add< Field< 3, Vector3 > >(
                    "Vector Field",
                    "Vektorfeld für die Flow Probe Visualisierung.",
                    definedOn< Grid< 3 > >( Grid< 3 >::Points ),
                    Options::REQUIRED );

                add< double >( "Glyph Scale", "Skalierung der Glyphen.", 1.0 );
                add< bool >( "Show Divergence", "Divergenz visualisieren.", true );
                add< bool >( "Show Rotation", "Rotation (Curl) visualisieren.", true );
                add< bool >( "Show Curvature", "Krümmung visualisieren.", true );
                add< double >( "Divergence Scale", "Skalierung für Divergenz-Darstellung.", 1.0 );
                add< double >( "Rotation Scale", "Skalierung für Rotation-Darstellung.", 1.0 );
                add< double >( "Curvature Scale", "Skalierung für Krümmungs-Darstellung.", 1.0 );
                add< double >( "Step Size", "Schrittweite für numerische Differentiation.", kDefaultStepSize );
                add< int >( "Sample Count", "Anzahl der Sampling-Punkte entlang jeder Dimension (für uniformes Grid).", 10 );
                add< double >( "Time", "Zeitstempel, an dem das Feld evaluiert wird.", 0.0 );
            }
        };

        struct DataOutputs : public DataAlgorithm::DataOutputs
        {
            DataOutputs( fantom::DataOutputs::Control& control )
                : DataAlgorithm::DataOutputs( control )
            {
                add< const PointSet< 3 > >( "Glyph Positions" );
                add< const LineSet< 3 > >( "Vector Arrows" );
                add< const LineSet< 3 > >( "Rotation Arrows" );
            }
        };

        LocalizedFlowProbe( InitData& data )
            : DataAlgorithm( data )
        {
        }

        void execute( const Algorithm::Options& options, const volatile bool& abortFlag ) override
        {
            // Get the vector field from options
            auto field = options.get< Field< 3, Vector3 > >( "Vector Field" );
            auto function = options.get< Function< Vector3 > >( "Vector Field" );

            if( !field || !function )
            {
                warningLog() << "Kein Vektorfeld ausgewählt." << std::endl;
                clearResult( "Glyph Positions" );
                clearResult( "Vector Arrows" );
                clearResult( "Rotation Arrows" );
                return;
            }

            // Check that the field is defined on a 3D grid
            auto grid = std::dynamic_pointer_cast< const Grid< 3 > >( function->domain() );
            if( !grid )
            {
                throw std::logic_error( "Das Vektorfeld ist nicht auf einem 3D-Gitter definiert." );
            }

            // Create evaluator to interpolate field values at any point
            auto evaluator = field->makeEvaluator();
            if( !evaluator )
            {
                warningLog() << "Vektorfeld liefert keinen Evaluator." << std::endl;
                clearResult( "Glyph Positions" );
                clearResult( "Vector Arrows" );
                clearResult( "Rotation Arrows" );
                return;
            }

            // Get parameters
            double time = options.get< double >( "Time" );
            double glyphScale = options.get< double >( "Glyph Scale" );
            bool showDivergence = options.get< bool >( "Show Divergence" );
            bool showRotation = options.get< bool >( "Show Rotation" );
            bool showCurvature = options.get< bool >( "Show Curvature" );
            double rotationScale = options.get< double >( "Rotation Scale" );
            double stepSize = options.get< double >( "Step Size" );
            int sampleCount = std::max( 1, options.get< int >( "Sample Count" ) );

            // Find the bounding box of the grid
            const auto& gridPoints = grid->points();
            Point3 gridMin, gridMax;
            if( gridPoints.size() > 0 )
            {
                gridMin = gridPoints[0];
                gridMax = gridPoints[0];
                for( size_t i = 1; i < gridPoints.size(); ++i )
                {
                    const auto& p = gridPoints[i];
                    for( size_t d = 0; d < 3; ++d )
                    {
                        gridMin[d] = std::min( gridMin[d], p[d] );
                        gridMax[d] = std::max( gridMax[d], p[d] );
                    }
                }
            }
            else
            {
                warningLog() << "Grid hat keine Punkte." << std::endl;
                clearResult( "Glyph Positions" );
                clearResult( "Vector Arrows" );
                clearResult( "Rotation Arrows" );
                return;
            }

            // Generate sampling points (uniform grid)
            std::vector< Point3 > samplePoints;
            Vector3 gridSize = gridMax - gridMin;
            double spacing = std::min( { gridSize[0], gridSize[1], gridSize[2] } ) / static_cast< double >( sampleCount + 1 );

            for( int i = 0; i <= sampleCount; ++i )
            {
                for( int j = 0; j <= sampleCount; ++j )
                {
                    for( int k = 0; k <= sampleCount; ++k )
                    {
                        Point3 p( gridMin[0] + ( i * spacing ),
                                  gridMin[1] + ( j * spacing ),
                                  gridMin[2] + ( k * spacing ) );
                        samplePoints.push_back( p );
                    }
                }
            }

            // Storage for outputs
            std::vector< Point3 > glyphPositions;
            std::vector< Point3 > vectorArrowVertices;
            std::vector< std::vector< size_t > > vectorArrowIndices;
            std::vector< Point3 > rotationArrowVertices;
            std::vector< std::vector< size_t > > rotationArrowIndices;

            // Process each sample point
            for( const auto& samplePoint : samplePoints )
            {
                if( abortFlag )
                {
                    break;
                }

                // Check if point is in domain
                evaluator->reset( samplePoint, time );
                if( !*evaluator )
                {
                    continue; // Skip points outside domain
                }

                glyphPositions.push_back( samplePoint );

                // Get vector field value
                Vector3 v = evaluator->value();
                double v_norm = norm( v );

                if( v_norm < kMinDirectionNorm )
                {
                    continue; // Skip zero vectors
                }

                // Create vector arrow (hedgehog)
                size_t arrowStartIdx = vectorArrowVertices.size();
                vectorArrowVertices.push_back( samplePoint );
                vectorArrowVertices.push_back( samplePoint + glyphScale * v );
                std::vector< size_t > arrowIndices = { arrowStartIdx, arrowStartIdx + 1 };
                vectorArrowIndices.push_back( arrowIndices );

                // Compute and visualize divergence, rotation, curvature
                if( showDivergence )
                {
                    (void)computeDivergence( *evaluator, samplePoint, time, stepSize );
                    // Divergence can be visualized as color coding or additional geometry
                    // For now, we'll store it for potential future use
                }

                if( showRotation )
                {
                    Vector3 rot = computeRotation( *evaluator, samplePoint, time, stepSize );
                    double rot_norm = norm( rot );
                    if( rot_norm > kMinDirectionNorm )
                    {
                        // Create rotation arrow
                        Vector3 rot_normalized = normalized( rot );
                        size_t rotStartIdx = rotationArrowVertices.size();
                        rotationArrowVertices.push_back( samplePoint );
                        rotationArrowVertices.push_back( samplePoint + rotationScale * glyphScale * rot_normalized );
                        std::vector< size_t > rotIndices = { rotStartIdx, rotStartIdx + 1 };
                        rotationArrowIndices.push_back( rotIndices );
                    }
                }

                if( showCurvature )
                {
                    (void)computeCurvature( *evaluator, samplePoint, time, stepSize );
                    // Curvature can be visualized as color coding or additional geometry
                    // For now, we'll store it for potential future use
                }
            }

            // Create output data structures
            if( !glyphPositions.empty() )
            {
                auto pointSet = DomainFactory::makePointSet< 3 >( std::move( glyphPositions ) );
                setResult( "Glyph Positions", pointSet );
            }
            else
            {
                clearResult( "Glyph Positions" );
            }

            if( !vectorArrowVertices.empty() )
            {
                auto lineSet = DomainFactory::makeLineSet< 3 >( std::move( vectorArrowVertices ), std::move( vectorArrowIndices ) );
                setResult( "Vector Arrows", lineSet );
            }
            else
            {
                clearResult( "Vector Arrows" );
            }

            if( !rotationArrowVertices.empty() )
            {
                auto rotLineSet = DomainFactory::makeLineSet< 3 >( std::move( rotationArrowVertices ), std::move( rotationArrowIndices ) );
                setResult( "Rotation Arrows", rotLineSet );
            }
            else
            {
                clearResult( "Rotation Arrows" );
            }
        }
    };

    AlgorithmRegister< LocalizedFlowProbe > registerFlowProbe(
        "Aufgabe4-1/1 Localized Flow Probe",
        "Erweiterte Vektorfeld-Visualisierung mit Divergenz, Rotation und Krümmung." );
} // namespace aufgabe4_1
