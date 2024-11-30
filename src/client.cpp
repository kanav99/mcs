#include <service.hpp>
#include <mcs.hpp>
#include <Eigen/Dense>

const uint DIM0 = 1000;
const uint DIM1 = 1000;
const uint BATCH_SIZE = 10;
const int PORT = 5065;

using T = unsigned long long;

int main(int argc, char *argv[])
{
    std::string ip = "127.0.0.1";
    if (argc > 1)
    {
        ip = argv[1];
    }
    mcs_load_setup();
    mcs_load_commit();

    Eigen::Matrix<T, DIM1, BATCH_SIZE> x;

    for (int i = 0; i < DIM1; i++)
    {
        for (int j = 0; j < BATCH_SIZE; j++)
        {
            x(i, j) = 1;
        }
    }

    Eigen::Matrix<T, DIM0, BATCH_SIZE> y = request<Eigen::Matrix<T, DIM1, BATCH_SIZE>, Eigen::Matrix<T, DIM0, BATCH_SIZE>>(x, ip, PORT);

    Eigen::Matrix<T, BATCH_SIZE, 1> rlc;
    // randomize rlc;
    for (int i = 0; i < BATCH_SIZE; i++)
    {
        rlc(i) = rand();
    }

    Eigen::Matrix<T, DIM0, 1> y_combined = y * rlc;
    Eigen::Matrix<T, DIM1, 1> x_combined = x * rlc;

    for (int i = 0; i < DIM0; i++)
    {
        for (int j = 0; j < BATCH_SIZE; j++)
        {
            std::cout << y(i, j) << " ";
        }
    }

    auto v = mcs_verify(DIM0, DIM1, x_combined.data(), y_combined.data());
    if (v == 1)
    {
        std::cout << "Verified" << std::endl;
    }
    else
    {
        std::cout << "Not Verified" << std::endl;
    }

    return 0;
}
