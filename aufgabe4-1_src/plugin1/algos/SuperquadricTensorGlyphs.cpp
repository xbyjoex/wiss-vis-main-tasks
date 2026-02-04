// Superquadric tensor glyphs (Kindlmann 2004). DataAlgorithm + VisAlgorithm.

#include <algorithm>
#include <cmath>
#include <complex>
#include <fantom/algorithm.hpp>
#include <fantom/datastructures/DomainFactory.hpp>
#include <fantom/datastructures/interfaces/Field.hpp>
#include <fantom/datastructures/domains/Grid.hpp>
#include <fantom/datastructures/Function.hpp>
#include <fantom/graphics.hpp>
#include <fantom/math.hpp>
#include <fantom/register.hpp>
#include <fantom-plugins/utils/math/eigenvalues.hpp>
#include <fantom-plugins/utils/Graphics/HelperFunctions.hpp>
#include <memory>
#include <vector>
#include <limits>

namespace aufgabe4_1
{
    using namespace fantom;

    namespace
    {
        constexpr double kMinEigenvalue = 1e-12;
        constexpr double kDefaultGamma = 2.5;

        struct WestinMetrics { double c_l, c_p, c_s; };

        struct FormParameters
        {
            double alpha;
            double beta;
        };

        // Westin metrics: three numbers in [0,1] that describe linear/planar/spherical anisotropy of the tensor.
        WestinMetrics computeWestinMetrics( double lambda1, double lambda2, double lambda3 )
        {
            double sum = lambda1 + lambda2 + lambda3;
            if( sum < kMinEigenvalue ) return { 0.0, 0.0, 1.0 };
            return { ( lambda1 - lambda2 ) / sum, 2.0 * ( lambda2 - lambda3 ) / sum, 3.0 * lambda3 / sum };
        }

        // Alpha and beta control shape (cylinder vs disc vs sphere). Kindlmann: from Westin; else round cross-section.
        FormParameters computeFormParameters( double c_l, double c_p, double c_s, double gamma, bool useKindlmann )
        {
            if( useKindlmann )
            {
                if( c_l >= c_p ) return { std::pow( 1.0 - c_p, gamma ), std::pow( 1.0 - c_l, gamma ) };
                return { std::pow( 1.0 - c_l, gamma ), std::pow( 1.0 - c_p, gamma ) };
            }
            return { std::pow( c_s, gamma ), 1.0 };
        }

        inline double sgn( double x ) { return x >= 0.0 ? 1.0 : -1.0; }

        // One point on the unit superquadric surface (theta, phi = angles; alpha, beta = shape exponents).
        Point3 superquadricPoint( double theta, double phi, double alpha, double beta )
        {
            double cp = std::cos( phi ), sp = std::sin( phi );
            double ct = std::cos( theta ), st = std::sin( theta );
            double x = sgn( cp ) * std::pow( std::abs( cp ), alpha ) * sgn( ct ) * std::pow( std::abs( ct ), beta );
            double y = sgn( cp ) * std::pow( std::abs( cp ), alpha ) * sgn( st ) * std::pow( std::abs( st ), beta );
            double z = sgn( sp ) * std::pow( std::abs( sp ), alpha );
            return Point3( x, y, z );
        }

        // Outward normal at that point (for lighting).
        Vector3 superquadricNormal( double theta, double phi, double alpha, double beta )
        {
            double cp = std::cos( phi ), sp = std::sin( phi );
            double ct = std::cos( theta ), st = std::sin( theta );
            double nx = sgn( cp ) * std::pow( std::abs( cp ), 2.0 - alpha ) * sgn( ct ) * std::pow( std::abs( ct ), 2.0 - beta );
            double ny = sgn( cp ) * std::pow( std::abs( cp ), 2.0 - alpha ) * sgn( st ) * std::pow( std::abs( st ), 2.0 - beta );
            double nz = sgn( sp ) * std::pow( std::abs( sp ), 2.0 - alpha );
            return Vector3( nx, ny, nz );
        }

        // Eigen solver can return complex numbers; for symmetric tensors we take the real part (or magnitude).
        double extractRealEigenvalue( const std::complex< double >& val, double threshold = 1e-9 )
        {
            return ( std::abs( val.imag() ) > threshold ) ? std::abs( val ) : val.real();
        }

        Vector3 extractRealEigenvector( const Tensor< std::complex< double >, 3 >& vec, double threshold = 1e-9 )
        {
            Vector3 res;
            for( size_t i = 0; i < 3; ++i ) res[i] = ( std::abs( vec( i ).imag() ) > threshold ) ? std::abs( vec( i ) ) : vec( i ).real();
            return normalized( res );
        }
        
        PointF<3> toPointF( const Point3& p ) { return PointF<3>( (float)p[0], (float)p[1], (float)p[2] ); }
        VectorF<3> toVectorF( const Vector3& v ) { return VectorF<3>( (float)v[0], (float)v[1], (float)v[2] ); }
    }

    class SuperquadricTensorGlyphs : public DataAlgorithm
    {
    public:
        struct Options : public DataAlgorithm::Options
        {
            Options( fantom::Options::Control& control ) : DataAlgorithm::Options( control )
            {
                add< Field< 3, Matrix< 3 > > >( "Tensor Field", "Input tensor field", Options::REQUIRED );
                add< double >( "Glyph Scale", "Scaling factor", 1.0 );
                add< double >( "Sharpness Parameter γ", "Edge sharpness (paper: ~2–3)", kDefaultGamma );
                add< bool >( "Use Kindlmann Shape", "Paper shape (alpha/beta from anisotropy); off = round cross-section", true );
                add< int >( "Resolution Theta", "Theta resolution", 20 );
                add< int >( "Resolution Phi", "Phi resolution", 20 );
                add< int >( "Sample Count", "Grid sampling resolution", 10 );
                add< double >( "Time", "Evaluation time", 0.0 );
                add< bool >( "Normalize to cell", "Scale each glyph to fit cell (no overlap)", false );
                add< double >( "Cell fill", "Fraction of cell size when normalized (0.5–1.0)", 0.8 );
            }
        };

        struct DataOutputs : public DataAlgorithm::DataOutputs
        {
            DataOutputs( fantom::DataOutputs::Control& control ) : DataAlgorithm::DataOutputs( control )
            {
                add< const Grid< 3 > >( "Glyph Mesh" );
                add< const Function< Color > >( "Color" );
                add< const Function< Vector3 > >( "Normals" );
            }
        };

        SuperquadricTensorGlyphs( InitData& data ) : DataAlgorithm( data ) {}

        void execute( const Algorithm::Options& options, const volatile bool& abortFlag ) override
        {
            debugLog() << "Starting Superquadric Generation..." << std::endl;

            // Input: a 3x3 tensor at each point (e.g. diffusion tensor or stress).
            auto field = options.get< Field< 3, Matrix< 3 > > >( "Tensor Field" );
            auto function = options.get< Function< Matrix< 3 > > >( "Tensor Field" );
            if( !field || !function ) { 
                debugLog() << "Input Tensor Field missing." << std::endl;
                clearResults(); return; 
            }

            auto grid = std::dynamic_pointer_cast< const Grid< 3 > >( function->domain() );
            if( !grid ) throw std::logic_error( "Tensor field not on a 3D grid." );

            debugLog() << "Input Grid has " << grid->points().size() << " points." << std::endl;

            // Evaluator gives us the tensor at any point.
            auto evaluator = field->makeEvaluator();
            if( !evaluator ) { clearResults(); return; }

            double time = options.get< double >( "Time" );
            double glyphScale = options.get< double >( "Glyph Scale" );
            double gamma = options.get< double >( "Sharpness Parameter γ" );
            bool useKindlmann = options.get< bool >( "Use Kindlmann Shape" );
            int resTheta = std::max( 4, options.get< int >( "Resolution Theta" ) );
            int resPhi = std::max( 4, options.get< int >( "Resolution Phi" ) );
            int sampleCount = std::max( 1, options.get< int >( "Sample Count" ) );
            bool normalizeToCell = options.get< bool >( "Normalize to cell" );
            double cellFill = std::max( 0.01, std::min( 1.0, options.get< double >( "Cell fill" ) ) );

            debugLog() << "Parameters:" << std::endl;
            debugLog() << "  Time: " << time << std::endl;
            debugLog() << "  Glyph Scale: " << glyphScale << std::endl;
            debugLog() << "  Sharpness Gamma: " << gamma << std::endl;
            debugLog() << "  Use Kindlmann: " << (useKindlmann ? "Yes" : "No") << std::endl;
            debugLog() << "  Res Theta: " << resTheta << std::endl;
            debugLog() << "  Res Phi: " << resPhi << std::endl;
            debugLog() << "  Sample Count: " << sampleCount << std::endl;
            debugLog() << "  Normalize to cell: " << ( normalizeToCell ? "Yes" : "No" ) << std::endl;
            if( normalizeToCell ) debugLog() << "  Cell fill: " << cellFill << std::endl;

            // Bounding Box & Sampling
            const auto& gridPoints = grid->points();
            if( gridPoints.size() == 0 ) { clearResults(); return; }
            Point3 gridMin = gridPoints[0], gridMax = gridPoints[0];
            for( size_t i = 1; i < gridPoints.size(); ++i )
                for( int d = 0; d < 3; ++d ) {
                    gridMin[d] = std::min( gridMin[d], gridPoints[i][d] );
                    gridMax[d] = std::max( gridMax[d], gridPoints[i][d] );
                }
            
            debugLog() << "Grid Bounds: [" << gridMin << "] to [" << gridMax << "]" << std::endl;

            // Spacing and sample counts per dimension (0 if domain is degenerate in that axis).
            Vector3 gridSize = gridMax - gridMin;
            double maxDim = std::max( { gridSize[0], gridSize[1], gridSize[2] } );
            double spacing = maxDim / static_cast< double >( sampleCount + 1 );
            int countX = ( gridSize[0] < 1e-6 ) ? 0 : sampleCount;
            int countY = ( gridSize[1] < 1e-6 ) ? 0 : sampleCount;
            int countZ = ( gridSize[2] < 1e-6 ) ? 0 : sampleCount;

            // Fill (countX+1)×(countY+1)×(countZ+1) sample positions.
            std::vector< Point3 > samplePoints;
            for( int i = 0; i <= countX; ++i )
                for( int j = 0; j <= countY; ++j )
                    for( int k = 0; k <= countZ; ++k )
                        samplePoints.push_back( gridMin + Vector3( i*spacing, j*spacing, k*spacing ) );

            // We will append one mesh per glyph: vertices, normals, colors, and triangle indices.
            std::vector< Point3 > vertices;
            std::vector< Color > colors;
            std::vector< Vector3 > normals;
            std::vector< size_t > indices;

            size_t validTensors = 0;
            size_t skippedTensors = 0;

            double minEval = std::numeric_limits<double>::max();
            double maxEval = std::numeric_limits<double>::lowest();

            Algorithm::Progress progress( *this, "Generating Glyphs", samplePoints.size() );

            for( size_t i = 0; i < samplePoints.size(); ++i )
            {
                progress = i;
                const auto& p = samplePoints[i];

                if( abortFlag ) break;
                evaluator->reset( p, time );
                if( !*evaluator ) continue;

                // Tensor at this point. We need its eigenvalues (sizes) and eigenvectors (directions).
                Matrix< 3 > val = evaluator->value();
                Tensor< double, 3, 3 > tensor( val );
                auto eigensystem = fantom::math::getEigensystem< 3 >( tensor );

                // Get real eigenvalues and eigenvectors; sort by eigenvalue (largest first).
                std::vector< std::pair< double, Vector3 > > eigenPairs( 3 );
                for( int i = 0; i < 3; ++i )
                {
                    eigenPairs[i].first = extractRealEigenvalue( eigensystem.first[i] );
                    eigenPairs[i].second = extractRealEigenvector( eigensystem.second[i] );
                }

                // Sort by eigenvalue (descending)
                std::sort( eigenPairs.begin(), eigenPairs.end(), []( const auto& a, const auto& b ) {
                    return a.first > b.first;
                } );

                double l1 = std::max( 0.0, eigenPairs[0].first );
                double l2 = std::max( 0.0, eigenPairs[1].first );
                double l3 = std::max( 0.0, eigenPairs[2].first );

                minEval = std::min( minEval, l3 );
                maxEval = std::max( maxEval, l1 );

                // Skip degenerate tensors (all eigenvalues near zero).
                if( l1 < kMinEigenvalue ) {
                    skippedTensors++;
                    continue;
                }
                validTensors++;

                Vector3 v1 = eigenPairs[0].second;
                Vector3 v2 = eigenPairs[1].second;
                Vector3 v3 = eigenPairs[2].second;

                // Build orthonormal frame (v1 = main direction; v2, v3 perpendicular; right-handed).
                v1 = normalized( v1 );
                v2 = normalized( v2 - ( v2 * v1 ) * v1 ); // Gram-Schmidt to ensure orthogonality
                v3 = normalized( cross( v1, v2 ) );

                // Shape parameters from Kindlmann paper (alpha, beta) and color from main direction.
                WestinMetrics m = computeWestinMetrics( l1, l2, l3 );
                FormParameters fp = computeFormParameters( m.c_l, m.c_p, m.c_s, gamma, useKindlmann );
                Color glyphColor( std::abs( v1[0] ), std::abs( v1[1] ), std::abs( v1[2] ), 1.0f );

                // Optional: scale this glyph so it fits in its cell (no overlap with neighbors).
                double scaleFactor = 1.0;
                if( normalizeToCell && l1 >= kMinEigenvalue && spacing > 1e-12 )
                {
                    double targetRadius = 0.5 * spacing * cellFill;
                    scaleFactor = targetRadius / ( glyphScale * l1 );
                }

                size_t baseIndex = vertices.size();

                // Sample the unit superquadric with theta/phi; scale by l1,l2,l3; rotate to eigenframe. Each sample = one vertex + normal + color.
                for( int i = 0; i <= resTheta; ++i )
                {
                    double theta = -M_PI + ( 2.0 * M_PI * i ) / resTheta;
                    for( int j = 0; j <= resPhi; ++j )
                    {
                        double phi = -M_PI / 2.0 + ( M_PI * j ) / resPhi;

                        Point3 unitPos = superquadricPoint( theta, phi, fp.alpha, fp.beta );
                        Vector3 unitNorm = superquadricNormal( theta, phi, fp.alpha, fp.beta );
                        unitNorm = normalized( unitNorm );

                // Unit superquadric z-axis → principal eigenvector; scale by eigenvalues
                Vector3 scaledPos( l2 * unitPos[0], l3 * unitPos[1], l1 * unitPos[2] );
                        Vector3 scaledNorm( 
                            ( l2 > 1e-9 ) ? unitNorm[0] / l2 : unitNorm[0],
                            ( l3 > 1e-9 ) ? unitNorm[1] / l3 : unitNorm[1],
                            ( l1 > 1e-9 ) ? unitNorm[2] / l1 : unitNorm[2]
                        );

                        // Rotate into eigenframe (Z→v1)
                        Vector3 rotPos = scaledPos[0] * v2 + scaledPos[1] * v3 + scaledPos[2] * v1;
                        Vector3 rotNorm = scaledNorm[0] * v2 + scaledNorm[1] * v3 + scaledNorm[2] * v1;

                        vertices.push_back( p + scaleFactor * glyphScale * rotPos );
                        normals.push_back( normalized( rotNorm ) );
                        colors.push_back( glyphColor );
                    }
                }

                // Connect the vertex grid into triangles (each quad becomes two triangles).
                for( int i = 0; i < resTheta; ++i )
                {
                    for( int j = 0; j < resPhi; ++j )
                    {
                        size_t i00 = baseIndex + i * ( resPhi + 1 ) + j;
                        size_t i01 = baseIndex + i * ( resPhi + 1 ) + ( j + 1 );
                        size_t i10 = baseIndex + ( i + 1 ) * ( resPhi + 1 ) + j;
                        size_t i11 = baseIndex + ( i + 1 ) * ( resPhi + 1 ) + ( j + 1 );

                        indices.push_back( i00 ); indices.push_back( i01 ); indices.push_back( i10 );
                        indices.push_back( i01 ); indices.push_back( i11 ); indices.push_back( i10 );
                    }
                }
            }

            debugLog() << "Eigenvalue Range: Min=" << minEval << ", Max=" << maxEval << std::endl;
            
            // Size Check
            double maxGlyphRadius = maxEval * glyphScale;
            double sizeRatio = maxGlyphRadius / spacing;
            debugLog() << "Glyph Size Info: Max Radius=" << maxGlyphRadius << ", Grid Spacing=" << spacing << " (Ratio=" << sizeRatio << ")" << std::endl;
            
            if( sizeRatio < 0.05 )
            {
                debugLog() << "WARNING: Glyphs are very small (< 5% of spacing). They might be invisible!" << std::endl;
                debugLog() << "SUGGESTION: Increase 'Glyph Scale' to approx " << ( spacing * 0.5 / maxEval ) << "." << std::endl;
            }
            else if( sizeRatio > 1.0 )
            {
                debugLog() << "WARNING: Glyphs are larger than spacing. They might overlap significantly." << std::endl;
            }

            debugLog() << "Processed Tensors: " << validTensors << " valid, " << skippedTensors << " skipped (too small)." << std::endl;
            debugLog() << "Generated Mesh: " << vertices.size() << " vertices, " << indices.size() / 3 << " triangles." << std::endl;

            if( vertices.empty() ) {
                debugLog() << "No vertices generated!" << std::endl;
                clearResults(); return;
            }

            // Package vertices and indices as a single grid (mesh) for the renderer.
            std::pair< Cell::Type, size_t > cellCounts[] = { { Cell::TRIANGLE, indices.size() / 3 } };
            auto mesh = DomainFactory::makeGrid< 3 >( std::move( vertices ), 1, cellCounts, std::move( indices ) );
            setResult( "Glyph Mesh", mesh );

            // Attach per-vertex color and normal to the mesh so the renderer can shade it.
            setResult( "Color", fantom::addData( mesh, Grid< 3 >::Points, colors ) );
            setResult( "Normals", fantom::addData( mesh, Grid< 3 >::Points, normals ) );
        }
    };

    AlgorithmRegister< SuperquadricTensorGlyphs > registerGlyphs(
        "Aufgabe4-1/2 Superquadric Generation",
        "Berechnet Superquadric-Glyphen Geometrie, Farbe und Normalen." );


    class SuperquadricGlyphRenderer : public VisAlgorithm
    {
    public:
        struct Options : public VisAlgorithm::Options
        {
            Options( fantom::Options::Control& control ) : VisAlgorithm::Options( control )
            {
                add< Grid< 3 > >( "Grid", "The glyph mesh", Options::REQUIRED );
                add< Function< Color > >( "Color", "Color field" );
                add< Function< Vector3 > >( "Normals", "Normal field (analytic)" );
            }
        };

        struct VisOutputs : public VisAlgorithm::VisOutputs
        {
            VisOutputs( fantom::VisOutputs::Control& control ) : VisAlgorithm::VisOutputs( control )
            {
                addGraphics( "Glyphs" );
            }
        };

        SuperquadricGlyphRenderer( InitData& data ) : VisAlgorithm( data ) {}

        void execute( const Algorithm::Options& options, const volatile bool& abortFlag ) override
        {
            (void)abortFlag;
            debugLog() << "Starting Superquadric Rendering..." << std::endl;
            // Input: the mesh (vertices + triangles) produced by the Superquadric Generation algorithm.
            auto grid = options.get< Grid< 3 > >( "Grid" );
            if( !grid ) { 
                debugLog() << "No Grid input connected." << std::endl;
                clearGraphics( "Glyphs" ); return; 
            }

            std::vector< PointF< 3 > > vertices;
            std::vector< VectorF< 3 > > normals;
            std::vector< Color > colors;
            std::vector< unsigned int > indices;

            // Copy mesh vertices to float buffers (graphics API expects float).
            const auto& pts = grid->points();
            debugLog() << "Input Grid points: " << pts.size() << std::endl;
            vertices.reserve( pts.size() );
            for( size_t i = 0; i < pts.size(); ++i ) vertices.push_back( toPointF( pts[i] ) );

            // Normals: use the ones from the generator (for correct lighting) or fallback to a default.
            auto normalFunc = options.get< Function< Vector3 > >( "Normals" );
            if( normalFunc )
            {
                debugLog() << "Using provided analytic normals (" << normalFunc->values().size() << ")." << std::endl;
                // Use provided analytic normals
                normals.reserve( normalFunc->values().size() );
                for( size_t i = 0; i < normalFunc->values().size(); ++i ) 
                    normals.push_back( toVectorF( normalFunc->values()[i] ) );
            }
            else
            {
                debugLog() << "No normals provided. Using default up vector (Fallback)." << std::endl;
                // Fallback: Default up
                normals.resize( pts.size(), VectorF<3>(0,0,1) ); 
                // Note: Better fallback would be computing mesh normals, but we expect analytic ones here.
            }

            // Per-vertex color from the generator (or grey if not connected).
            auto colorFunc = options.get< Function< Color > >( "Color" );
            if( colorFunc )
            {
                debugLog() << "Using provided colors (" << colorFunc->values().size() << ")." << std::endl;
                colors.reserve( colorFunc->values().size() );
                for( size_t i = 0; i < colorFunc->values().size(); ++i )
                    colors.push_back( colorFunc->values()[i] );
            }
            else
            {
                debugLog() << "No colors provided. Using default grey." << std::endl;
                colors.resize( pts.size(), Color(0.8, 0.8, 0.8, 1.0) );
            }

            // Triangle list: each cell is three vertex indices.
            const auto& cells = grid->cells();
            indices.reserve( cells.size() * 3 );
            for( size_t i = 0; i < cells.size(); ++i )
            {
                auto cell = cells[i];
                if( cell.type() == Cell::TRIANGLE )
                {
                    indices.push_back( (unsigned int)cell.index( 0 ) );
                    indices.push_back( (unsigned int)cell.index( 1 ) );
                    indices.push_back( (unsigned int)cell.index( 2 ) );
                }
            }

            if( vertices.empty() ) { clearGraphics( "Glyphs" ); return; }

            // Log bounding box for debugging.
            PointF<3> minVert = vertices[0];
            PointF<3> maxVert = vertices[0];
            for( const auto& v : vertices ) {
                for( int d=0; d<3; ++d ) {
                    minVert[d] = std::min( minVert[d], v[d] );
                    maxVert[d] = std::max( maxVert[d], v[d] );
                }
            }
            debugLog() << "Vertex Bounds: Min=[" << minVert << "], Max=[" << maxVert << "]" << std::endl;

            // Load Phong shader and build one drawable (positions, normals, colors, indices).
            auto const& system = graphics::GraphicsSystem::instance();
            std::string resourcePath = PluginRegistrationService::getInstance().getResourcePath( "utils/Graphics" );
            if( !resourcePath.empty() && resourcePath.back() != '/' ) resourcePath += "/";

            debugLog() << "Loading shaders from: " << resourcePath << "shader/surface/phong/multiColor/" << std::endl;

            auto program = system.makeProgramFromFiles(
                resourcePath + "shader/surface/phong/multiColor/vertex.glsl",
                resourcePath + "shader/surface/phong/multiColor/fragment.glsl"
            );

            if( !program )
            {
                debugLog() << "Failed to create shader program!" << std::endl;
                clearGraphics( "Glyphs" );
                return;
            }

            auto bs = graphics::computeBoundingSphere( vertices );
            debugLog() << "Computed Bounding Sphere: Center=[" << bs.center() << "], Radius=" << bs.radius() << std::endl;

            // Single drawable: all glyphs in one triangle mesh, sent to the 3D view.
            auto drawable = system.makePrimitive(
                graphics::PrimitiveConfig{ graphics::RenderPrimitives::TRIANGLES }
                    .vertexBuffer( "position", system.makeBuffer( vertices ) )
                    .vertexBuffer( "normal", system.makeBuffer( normals ) )
                    .vertexBuffer( "color", system.makeBuffer( colors ) )
                    .indexBuffer( system.makeIndexBuffer( indices ) )
                    .boundingSphere( bs ),
                program
            );

            setGraphics( "Glyphs", drawable );
            debugLog() << "Graphics set. Drawing " << indices.size() / 3 << " triangles." << std::endl;
        }
    };

    AlgorithmRegister< SuperquadricGlyphRenderer > registerGlyphRenderer(
        "Aufgabe4-1/3 Superquadric Rendering",
        "Visualisiert die Superquadric-Glyphen mit analytischen Normalen." );

} // namespace aufgabe4_1
