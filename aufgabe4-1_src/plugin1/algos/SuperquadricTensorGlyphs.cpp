// JP, 01/2025 - Aufgabe 4.1: Superquadric Tensor Glyphs
// Tensorfeld-Visualisierung nach Kindlmann (2004) mit Superquadric-Geometrie
#include <algorithm>
#include <cmath>
#include <complex>
#include <fantom/algorithm.hpp>
#include <fantom/datastructures/DomainFactory.hpp>
#include <fantom/datastructures/interfaces/Field.hpp>
#include <fantom/datastructures/domains/Grid.hpp>
#include <fantom/math.hpp>
#include <fantom/register.hpp>
#include <fantom-plugins/utils/math/eigenvalues.hpp>
#include <memory>
#include <vector>

namespace aufgabe4_1
{
    using namespace fantom;

    namespace
    {
        // Minimum eigenvalue threshold
        constexpr double kMinEigenvalue = 1e-9;
        constexpr double kDefaultGamma = 3.0;

        // Westin metrics structure
        struct WestinMetrics
        {
            double c_l; // Linearität
            double c_p; // Planarität
            double c_s; // Sphärizität
        };

        // Form parameters for superquadric
        struct FormParameters
        {
            double alpha;
            double beta;
        };

        // Compute Westin metrics from sorted eigenvalues
        // λ₁ ≥ λ₂ ≥ λ₃ ≥ 0
        WestinMetrics computeWestinMetrics( double lambda1, double lambda2, double lambda3 )
        {
            double sum = lambda1 + lambda2 + lambda3;
            if( sum < kMinEigenvalue )
            {
                // All eigenvalues are zero - return default metrics
                return { 0.0, 0.0, 1.0 };
            }

            WestinMetrics m;
            m.c_l = ( lambda1 - lambda2 ) / sum; // Linearität
            m.c_p = 2.0 * ( lambda2 - lambda3 ) / sum; // Planarität
            m.c_s = 3.0 * lambda3 / sum; // Sphärizität
            return m;
        }

        // Compute form parameters α and β from Westin metrics
        FormParameters computeFormParameters( double c_l, double c_p, double gamma )
        {
            FormParameters params;
            if( c_l >= c_p )
            {
                // Linearer Typ
                params.alpha = std::pow( 1.0 - c_p, gamma );
                params.beta = std::pow( 1.0 - c_l, gamma );
            }
            else
            {
                // Planarer Typ
                params.alpha = std::pow( 1.0 - c_l, gamma );
                params.beta = std::pow( 1.0 - c_p, gamma );
            }
            return params;
        }

        // Signum function
        inline double sgn( double x )
        {
            return x >= 0.0 ? 1.0 : -1.0;
        }

        // Generate a point on the superquadric surface in unit space
        // θ ∈ [-π, π], φ ∈ [-π/2, π/2]
        Point3 superquadricPoint( double theta, double phi, double alpha, double beta )
        {
            double cos_phi = std::cos( phi );
            double sin_phi = std::sin( phi );
            double cos_theta = std::cos( theta );
            double sin_theta = std::sin( theta );

            double x = sgn( cos_phi ) * std::pow( std::abs( cos_phi ), alpha ) * 
                       sgn( cos_theta ) * std::pow( std::abs( cos_theta ), beta );
            double y = sgn( cos_phi ) * std::pow( std::abs( cos_phi ), alpha ) * 
                       sgn( sin_theta ) * std::pow( std::abs( sin_theta ), beta );
            double z = sgn( sin_phi ) * std::pow( std::abs( sin_phi ), alpha );

            return Point3( x, y, z );
        }

        // Transform a point from unit space to data space
        // Scales with eigenvalues and rotates into eigenvector basis
        Point3 transformToDataSpace( const Point3& unitPoint,
                                     double lambda1,
                                     double lambda2,
                                     double lambda3,
                                     const Vector3& v1,
                                     const Vector3& v2,
                                     const Vector3& v3,
                                     const Point3& center )
        {
            // Scale with eigenvalues
            Vector3 scaled( lambda1 * unitPoint[0], lambda2 * unitPoint[1], lambda3 * unitPoint[2] );

            // Rotate into eigenvector basis
            Vector3 transformed = scaled[0] * v1 + scaled[1] * v2 + scaled[2] * v3;

            // Translate to glyph center
            return center + transformed;
        }

        // Extract real part from complex number, return 0 if imaginary part is significant
        double extractRealEigenvalue( const std::complex< double >& val, double threshold = 1e-9 )
        {
            if( std::abs( val.imag() ) > threshold )
            {
                // Complex eigenvalue - return magnitude as fallback
                return std::abs( val );
            }
            return val.real();
        }

        // Extract real eigenvector from complex eigenvector
        Vector3 extractRealEigenvector( const Tensor< std::complex< double >, 3 >& vec, double threshold = 1e-9 )
        {
            Vector3 result;
            for( size_t i = 0; i < 3; ++i )
            {
                if( std::abs( vec( i ).imag() ) > threshold )
                {
                    // Complex component - use magnitude
                    result[i] = std::abs( vec( i ) );
                }
                else
                {
                    result[i] = vec( i ).real();
                }
            }
            return normalized( result );
        }
    } // namespace

    class SuperquadricTensorGlyphs : public DataAlgorithm
    {
    public:
        struct Options : public DataAlgorithm::Options
        {
            Options( fantom::Options::Control& control )
                : DataAlgorithm::Options( control )
            {
                add< Field< 3, Tensor< double, 3, 3 > > >(
                    "Tensor Field",
                    "Tensorfeld für die Superquadric Glyph Visualisierung.",
                    definedOn< Grid< 3 > >( Grid< 3 >::Points ),
                    Options::REQUIRED );

                add< double >( "Glyph Scale", "Skalierung der Glyphen.", 1.0 );
                add< double >( "Sharpness Parameter γ", "Schärfeparameter für Superquadric-Form (Vorschlag: 3.0).", kDefaultGamma );
                add< int >( "Resolution Theta", "Anzahl der Schritte in θ-Richtung (Auflösung der Superquadric-Oberfläche).", 20 );
                add< int >( "Resolution Phi", "Anzahl der Schritte in φ-Richtung (Auflösung der Superquadric-Oberfläche).", 20 );
                add< int >( "Sample Count", "Anzahl der Sampling-Punkte entlang jeder Dimension (für uniformes Grid).", 5 );
                add< double >( "Time", "Zeitstempel, an dem das Feld evaluiert wird.", 0.0 );
            }
        };

        struct DataOutputs : public DataAlgorithm::DataOutputs
        {
            DataOutputs( fantom::DataOutputs::Control& control )
                : DataAlgorithm::DataOutputs( control )
            {
                add< const Grid< 3 > >( "Glyph Mesh" );
            }
        };

        SuperquadricTensorGlyphs( InitData& data )
            : DataAlgorithm( data )
        {
        }

        void execute( const Algorithm::Options& options, const volatile bool& abortFlag ) override
        {
            // Get the tensor field from options
            auto field = options.get< Field< 3, Tensor< double, 3, 3 > > >( "Tensor Field" );
            auto function = options.get< Function< Tensor< double, 3, 3 > > >( "Tensor Field" );

            if( !field || !function )
            {
                warningLog() << "Kein Tensorfeld ausgewählt." << std::endl;
                clearResult( "Glyph Mesh" );
                return;
            }

            // Check that the field is defined on a 3D grid
            auto grid = std::dynamic_pointer_cast< const Grid< 3 > >( function->domain() );
            if( !grid )
            {
                throw std::logic_error( "Das Tensorfeld ist nicht auf einem 3D-Gitter definiert." );
            }

            // Create evaluator
            auto evaluator = field->makeEvaluator();
            if( !evaluator )
            {
                warningLog() << "Tensorfeld liefert keinen Evaluator." << std::endl;
                clearResult( "Glyph Mesh" );
                return;
            }

            // Get parameters
            double time = options.get< double >( "Time" );
            double glyphScale = options.get< double >( "Glyph Scale" );
            double gamma = options.get< double >( "Sharpness Parameter γ" );
            int resolutionTheta = std::max( 4, options.get< int >( "Resolution Theta" ) );
            int resolutionPhi = std::max( 4, options.get< int >( "Resolution Phi" ) );
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
                clearResult( "Glyph Mesh" );
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

            // Storage for mesh data
            std::vector< Point3 > allVertices;
            std::vector< std::vector< size_t > > allTriangles;

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

                // Get tensor at this point
                Tensor< double, 3, 3 > tensor = evaluator->value();

                // Compute eigensystem
                auto eigensystem = fantom::math::getEigensystem< 3 >( tensor );
                const auto& eigenvalues = eigensystem.first;
                const auto& eigenvectors = eigensystem.second;

                if( eigenvalues.size() < 3 || eigenvectors.size() < 3 )
                {
                    continue; // Skip if eigensystem computation failed
                }

                // Extract real eigenvalues (sorted: λ₁ ≥ λ₂ ≥ λ₃)
                double lambda1 = extractRealEigenvalue( eigenvalues[0] );
                double lambda2 = extractRealEigenvalue( eigenvalues[1] );
                double lambda3 = extractRealEigenvalue( eigenvalues[2] );

                // Ensure non-negative eigenvalues
                lambda1 = std::max( 0.0, lambda1 );
                lambda2 = std::max( 0.0, lambda2 );
                lambda3 = std::max( 0.0, lambda3 );

                // Sort to ensure λ₁ ≥ λ₂ ≥ λ₃
                if( lambda1 < lambda2 )
                    std::swap( lambda1, lambda2 );
                if( lambda2 < lambda3 )
                    std::swap( lambda2, lambda3 );
                if( lambda1 < lambda2 )
                    std::swap( lambda1, lambda2 );

                // Skip if all eigenvalues are zero
                if( lambda1 < kMinEigenvalue )
                {
                    continue;
                }

                // Extract real eigenvectors
                Vector3 v1 = extractRealEigenvector( eigenvectors[0] );
                Vector3 v2 = extractRealEigenvector( eigenvectors[1] );
                Vector3 v3 = extractRealEigenvector( eigenvectors[2] );

                // Ensure orthonormal basis (Gram-Schmidt if needed)
                v1 = normalized( v1 );
                v2 = normalized( v2 - ( v2 * v1 ) * v1 );
                v3 = normalized( cross( v1, v2 ) );

                // Compute Westin metrics
                WestinMetrics metrics = computeWestinMetrics( lambda1, lambda2, lambda3 );

                // Compute form parameters
                FormParameters formParams = computeFormParameters( metrics.c_l, metrics.c_p, gamma );

                // Generate superquadric surface
                size_t vertexOffset = allVertices.size();
                std::vector< std::vector< size_t > > glyphTriangles;

                // Generate vertices
                for( int i = 0; i <= resolutionTheta; ++i )
                {
                    double theta = -M_PI + ( 2.0 * M_PI * i ) / resolutionTheta;
                    for( int j = 0; j <= resolutionPhi; ++j )
                    {
                        double phi = -M_PI / 2.0 + ( M_PI * j ) / resolutionPhi;

                        // Generate point in unit space
                        Point3 unitPoint = superquadricPoint( theta, phi, formParams.alpha, formParams.beta );

                        // Transform to data space
                        Point3 dataPoint = transformToDataSpace( unitPoint,
                                                                glyphScale * lambda1,
                                                                glyphScale * lambda2,
                                                                glyphScale * lambda3,
                                                                v1,
                                                                v2,
                                                                v3,
                                                                samplePoint );

                        allVertices.push_back( dataPoint );
                    }
                }

                // Generate triangles (quads split into two triangles)
                for( int i = 0; i < resolutionTheta; ++i )
                {
                    for( int j = 0; j < resolutionPhi; ++j )
                    {
                        size_t idx00 = vertexOffset + i * ( resolutionPhi + 1 ) + j;
                        size_t idx01 = vertexOffset + i * ( resolutionPhi + 1 ) + ( j + 1 );
                        size_t idx10 = vertexOffset + ( i + 1 ) * ( resolutionPhi + 1 ) + j;
                        size_t idx11 = vertexOffset + ( i + 1 ) * ( resolutionPhi + 1 ) + ( j + 1 );

                        // First triangle
                        allTriangles.push_back( { idx00, idx01, idx10 } );

                        // Second triangle
                        allTriangles.push_back( { idx01, idx11, idx10 } );
                    }
                }
            }

            // Create unstructured grid
            if( !allVertices.empty() && !allTriangles.empty() )
            {
                // Convert triangles to cell format
                std::vector< Point3 > points = allVertices;
                std::vector< size_t > cellIndices;
                size_t triangleCount = allTriangles.size();

                for( const auto& triangle : allTriangles )
                {
                    cellIndices.push_back( triangle[0] );
                    cellIndices.push_back( triangle[1] );
                    cellIndices.push_back( triangle[2] );
                }

                // Create cell counts structure
                std::pair< Cell::Type, size_t > cellCounts[] = { { Cell::TRIANGLE, triangleCount } };

                auto mesh = DomainFactory::makeGrid< 3 >( std::move( points ), 1, cellCounts, std::move( cellIndices ) );
                setResult( "Glyph Mesh", mesh );
            }
            else
            {
                clearResult( "Glyph Mesh" );
            }
        }
    };

    AlgorithmRegister< SuperquadricTensorGlyphs > registerGlyphs(
        "Aufgabe4-1/2 Superquadric Tensor Glyphs",
        "Tensorfeld-Visualisierung mit Superquadric-Glyphen nach Kindlmann (2004)." );
} // namespace aufgabe4_1
