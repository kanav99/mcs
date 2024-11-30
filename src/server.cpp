#include <service.hpp>
#include <mcs.hpp>
#include <Eigen/Dense>

const uint DIM0 = 10000;
const uint DIM1 = 10000;
const int PORT = 5065;

using T = unsigned long long;

Mat<T> mat(DIM0, DIM1);

int main(int argc, char *argv[])
{
    int gen_mcs = 0;
    if (argc > 1)
    {
        gen_mcs = atoi(argv[1]);
    }
    std::cerr << "[server] Server using " << Eigen::nbThreads() << " threads" << std::endl;
    for (int i = 0; i < DIM0; i++)
    {
        for (int j = 0; j < DIM1; j++)
        {
            mat(i, j) = 1;
        }
    }

    if (gen_mcs != 0)
    {
        mcs_gen_setup();
        mcs_load_setup();
        std::cerr << "[server] Generated setup in `setup.bin`" << std::endl;
        mcs_gen_commit(DIM0, DIM1, mat.data());
        std::cerr << "[server] Generated Commitment in `commit.bin`" << std::endl;
    }
    else
    {
        std::cerr << "[server] Skipping MCS" << std::endl;
    }

    std::cerr << "[server] Listening for requests..." << std::endl;
    loop<T>(mat, PORT);

    return 0;
}