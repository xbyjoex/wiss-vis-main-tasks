#include <fantom/algorithm.hpp>
#include <fantom/datastructures/interfaces/Field.hpp>
#include <fantom/datastructures/domains/LineSet.hpp>
#include <fantom/register.hpp>
#include <fantom/math.hpp>
#include <cmath>
#include <algorithm>
#include <array>
#include <complex>

using namespace fantom;

namespace
{
    using Tensor33 = Tensor<double, 3, 3>;

    static inline double absd(double x) { return x < 0.0 ? -x : x; }

    static inline void normalizeSafe(Vector3 &v)
    {
        double n = norm(v);
        if (n > 1e-30)
            v /= n;
    }

    // Jacobi-Eigenzerlegung für symmetrische 3x3
    // Output: lam[0] <= lam[1] <= lam[2], evec[i] zu lam[i] (normiert)
    static bool eigenSymmetric3x3_Jacobi(const Tensor33 &A_in, double lam[3], Vector3 evec[3])
    {
        // Matrix kopieren
        double A[3][3] = {
            {(double)A_in(0, 0), (double)A_in(0, 1), (double)A_in(0, 2)},
            {(double)A_in(1, 0), (double)A_in(1, 1), (double)A_in(1, 2)},
            {(double)A_in(2, 0), (double)A_in(2, 1), (double)A_in(2, 2)}};

        // symmetrisieren (numerisch)
        A[0][1] = A[1][0] = 0.5 * (A[0][1] + A[1][0]);
        A[0][2] = A[2][0] = 0.5 * (A[0][2] + A[2][0]);
        A[1][2] = A[2][1] = 0.5 * (A[1][2] + A[2][1]);

        // V = I
        double V[3][3] = {
            {1, 0, 0},
            {0, 1, 0},
            {0, 0, 1}};

        const int maxIter = 60;
        const double eps = 1e-12;

        auto maxOffdiag = [&](int &p, int &q) -> double
        {
            p = 0;
            q = 1;
            double m = absd(A[0][1]);
            if (absd(A[0][2]) > m)
            {
                m = absd(A[0][2]);
                p = 0;
                q = 2;
            }
            if (absd(A[1][2]) > m)
            {
                m = absd(A[1][2]);
                p = 1;
                q = 2;
            }
            return m;
        };

        for (int it = 0; it < maxIter; ++it)
        {
            int p, q;
            double m = maxOffdiag(p, q);
            if (m < eps)
                break;

            double app = A[p][p];
            double aqq = A[q][q];
            double apq = A[p][q];

            double phi = 0.5 * std::atan2(2.0 * apq, (aqq - app));
            double c = std::cos(phi);
            double s = std::sin(phi);

            // Zeilen/Spalten rotieren
            for (int k = 0; k < 3; ++k)
            {
                double apk = A[p][k];
                double aqk = A[q][k];
                A[p][k] = c * apk - s * aqk;
                A[q][k] = s * apk + c * aqk;
            }
            for (int k = 0; k < 3; ++k)
            {
                double akp = A[k][p];
                double akq = A[k][q];
                A[k][p] = c * akp - s * akq;
                A[k][q] = s * akp + c * akq;
            }

            // Offdiag exakt nullen (Stabilität)
            A[p][q] = A[q][p] = 0.0;

            // Eigenvektoren: V = V * R
            for (int r = 0; r < 3; ++r)
            {
                double vrp = V[r][p];
                double vrq = V[r][q];
                V[r][p] = c * vrp - s * vrq;
                V[r][q] = s * vrp + c * vrq;
            }
        }

        lam[0] = A[0][0];
        lam[1] = A[1][1];
        lam[2] = A[2][2];

        // Spalten von V
        evec[0] = Vector3(V[0][0], V[1][0], V[2][0]);
        evec[1] = Vector3(V[0][1], V[1][1], V[2][1]);
        evec[2] = Vector3(V[0][2], V[1][2], V[2][2]);

        // sortieren nach lam
        std::array<int, 3> idx = {0, 1, 2};
        std::sort(idx.begin(), idx.end(), [&](int a, int b)
                  { return lam[a] < lam[b]; });

        double lamS[3] = {lam[idx[0]], lam[idx[1]], lam[idx[2]]};
        Vector3 eS[3] = {evec[idx[0]], evec[idx[1]], evec[idx[2]]};

        lam[0] = lamS[0];
        lam[1] = lamS[1];
        lam[2] = lamS[2];
        evec[0] = eS[0];
        evec[1] = eS[1];
        evec[2] = eS[2];

        normalizeSafe(evec[0]);
        normalizeSafe(evec[1]);
        normalizeSafe(evec[2]);

        return true;
    }

    class TensorLinesAlgorithm : public DataAlgorithm
    {
    public:
        struct Options : public DataAlgorithm::Options
        {
            Options(Control &control) : DataAlgorithm::Options(control)
            {
                add<Field<3, Tensor33>>(
                    "TensorField",
                    "3D 3x3 Tensorfeld (cell-centered)",
                    definedOn<Grid<3>>(Grid<3>::Cells));

                add<int>("Which", "0=major, 1=median, 2=minor", 0);

                add<double>("Step", "Euler Schrittweite h", 0.05);
                add<double>("Max Length", "Maximale Linienlänge", 200.0);
                add<int>("Max Steps", "Maximale Schrittanzahl", 1000);

                add<double>("Isotropy Eps", "Abbruch wenn Eigenwerte zu nah (Degeneration)", 1e-4);

                add<int>("Seed Stride", "Jeden k-ten Gitterpunkt als Seed", 5);
            }
        };

        struct DataOutputs : public DataAlgorithm::DataOutputs
        {
            DataOutputs(fantom::DataOutputs::Control &control)
                : DataAlgorithm::DataOutputs(control)
            {
                add<LineSet<3>>("TensorLines");
            }
        };

        TensorLinesAlgorithm(InitData &data) : DataAlgorithm(data) {}

        void execute(const Algorithm::Options &options,
                     const volatile bool &abortFlag) override
        {
            auto field = options.get<Field<3, Tensor33>>("TensorField");

            auto lineSet = std::make_shared<LineSet<3>>();

            auto setEmptyAndReturn = [&]()
            {
                setResult("TensorLines", std::static_pointer_cast<const DataObject>(lineSet));
            };

            if (!field)
            {
                setEmptyAndReturn();
                return;
            }

            int which = options.get<int>("Which");
            which = std::max(0, std::min(2, which));

            const double h = options.get<double>("Step");
            const double maxLen = options.get<double>("Max Length");
            const int maxSteps = options.get<int>("Max Steps");
            const double isoEps = options.get<double>("Isotropy Eps");

            int stride = options.get<int>("Seed Stride");
            if (stride < 1)
                stride = 1;

            auto evaluator = field->makeEvaluator();
            const double time = 0.0;
            if (!evaluator->contains(time))
            {
                setEmptyAndReturn();
                return;
            }

            // Grid für Seeds (wie bei euch in Streamlines)
            auto function = options.get<Function<Tensor33>>("TensorField");
            auto grid = std::dynamic_pointer_cast<const Grid<3>>(function->domain());
            if (!grid)
            {
                setEmptyAndReturn();
                return;
            }

            const auto &seeds = grid->points();

            auto integrateOne = [&](const Point3 &seed)
            {
                std::vector<Point3> pts;
                pts.reserve((std::size_t)maxSteps + 1);

                Point3 x = seed;
                Vector3 prevDir(0, 0, 0);
                bool havePrev = false;
                double length = 0.0;

                // Seed muss evaluierbar sein
                try
                {
                    evaluator->reset(x, time);
                    (void)evaluator->value();
                }
                catch (...)
                {
                    return;
                }

                pts.push_back(x);

                for (int step = 0; step < maxSteps; ++step)
                {
                    if (abortFlag)
                        break;

                    Tensor33 T;
                    try
                    {
                        evaluator->reset(x, time);
                        T = evaluator->value();
                    }
                    catch (...)
                    {
                        break;
                    }

                    double lam[3];
                    Vector3 evec[3];
                    if (!eigenSymmetric3x3_Jacobi(T, lam, evec))
                        break;

                    // Isotropie/Entartung (Richtung nicht eindeutig/stetig definierbar)
                    const double d01 = absd(lam[1] - lam[0]);
                    const double d12 = absd(lam[2] - lam[1]);
                    if (which == 0 && d12 < isoEps)
                        break; // major unsicher
                    if (which == 2 && d01 < isoEps)
                        break; // minor unsicher
                    if (which == 1 && (d01 < isoEps || d12 < isoEps))
                        break; // median am empfindlichsten

                    Vector3 dir = (which == 0 ? evec[2] : (which == 1 ? evec[1] : evec[0]));
                    normalizeSafe(dir);
                    if (norm(dir) < 1e-12)
                        break;

                    // Richtungsfortsetzung (Vorzeichenstetigkeit)
                    if (havePrev)
                    {
                        const double dp = prevDir * dir;
                        if (dp < 0.0)
                            dir = -dir;
                    }
                    prevDir = dir;
                    havePrev = true;

                    // Euler Schritt
                    Point3 xNew = x + h * dir;

                    // Domain check (Preconditions erfüllen)
                    try
                    {
                        evaluator->reset(xNew, time);
                        (void)evaluator->value();
                    }
                    catch (...)
                    {
                        break;
                    }

                    x = xNew;
                    pts.push_back(x);

                    length += absd(h);
                    if (length >= maxLen)
                        break;
                }

                if (pts.size() < 2)
                    return;

                std::vector<std::size_t> idx;
                idx.reserve(pts.size());
                for (const auto &p : pts)
                    idx.push_back(lineSet->addPoint(p));
                lineSet->addLine(idx);
            };

            for (std::size_t i = stride; i + stride < seeds.size(); i += (std::size_t)stride)
            {
                if (abortFlag)
                    break;
                integrateOne(seeds[i]);
            }

            setResult("TensorLines", std::static_pointer_cast<const DataObject>(lineSet));
        }
    };

    AlgorithmRegister<TensorLinesAlgorithm> dummy("Custom/TensorLines", "Berechnet Tensorlinien (major/median/minor) via Euler (feste Schrittweite).");

} // namespace
