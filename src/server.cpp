#include <service.hpp>
#include <mcs.hpp>
#include <Eigen/Dense>

const uint DIM0 = 1000;
const uint DIM1 = 1000;
const int PORT = 5065;

using T = unsigned long long;

Mat<T> mat(DIM0, DIM1);

int main()
{
    std::cerr << "[server] Server using " << Eigen::nbThreads() << " threads" << std::endl;
    for (int i = 0; i < DIM0; i++)
    {
        for (int j = 0; j < DIM1; j++)
        {
            mat(i, j) = 1;
        }
    }

    mcs_gen_setup();
    mcs_load_setup();
    std::cerr << "[server] Generated setup in `setup.bin`" << std::endl;
    mcs_gen_commit(DIM0, DIM1, mat.data());
    std::cerr << "[server] Generated Commitment in `commit.bin`" << std::endl;

    std::cerr << "[server] Listening for requests..." << std::endl;
    loop<T>(mat, PORT);

    return 0;
}