#include <service.hpp>
#include <mcs.hpp>
#include <Eigen/Dense>

const uint DIM0 = 2;
const uint DIM1 = 2;
const int PORT = 5065;

using T = unsigned long long;

int main()
{
    mcs_load_setup();
    mcs_load_commit();

    Eigen::Matrix<T, DIM1, 1> x;

    for (int i = 0; i < DIM1; i++)
    {
        x(i) = 1;
    }

    Eigen::Matrix<T, DIM0, 1> y = request<Eigen::Matrix<T, DIM1, 1>, Eigen::Matrix<T, DIM0, 1>>(x, "127.0.0.1", PORT);

    for (int i = 0; i < DIM0; i++)
    {
        std::cout << y(i) << std::endl;
    }

    auto v = mcs_verify(DIM0, DIM1, x.data(), y.data());
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
