#include <service.hpp>
#include <mcs.hpp>
#include <Eigen/Dense>
#include <chrono>

const uint DIM0 = 10000;
const uint DIM1 = 10000;
const uint BATCH_SIZE = 100;
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

    Mat<T> x(DIM1, BATCH_SIZE);
    Eigen::Matrix<T, BATCH_SIZE, 1> rlc;
    // randomize rlc;
    for (int i = 0; i < BATCH_SIZE; i++)
    {
        rlc(i) = rand();
    }

    for (int i = 0; i < DIM1; i++)
    {
        for (int j = 0; j < BATCH_SIZE; j++)
        {
            x(i, j) = 1;
        }
    }

    auto start = std::chrono::high_resolution_clock::now();
    Mat<T> y = request<T>(x, DIM0, ip, PORT);
    Eigen::Matrix<T, DIM0, 1> y_combined = y * rlc;
    Eigen::Matrix<T, DIM1, 1> x_combined = x * rlc;
    auto v = mcs_verify(DIM0, DIM1, x_combined.data(), y_combined.data());
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    Mat<T> mat(DIM0, DIM1);

    for (int i = 0; i < DIM0; i++)
    {
        for (int j = 0; j < DIM1; j++)
        {
            mat(i, j) = 1;
        }
    }

    start = std::chrono::high_resolution_clock::now();
    Mat<T> y_actual = mat * x;
    end = std::chrono::high_resolution_clock::now();

    std::cout << "Local Computation Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

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
