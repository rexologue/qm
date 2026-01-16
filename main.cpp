#include <iostream>

#include "containers/array.hpp"
#include "containers/matrix.hpp"

#include "math/linalg.hpp"
#include "math/equation_system.hpp"
#include "math/functional.hpp"

int main()
{
    miv::array<int> a = {1, 2, 3, 4, 5};
    std::cout << "a.size() = " << a.size() << "\n";
    std::cout << "contains(3) = " << (a.contains(3) ? "true" : "false") << "\n";

    miv::matrix<int> M(2, 3);
    M.fill(7);
    M(0, 1) = 42;

    std::cout << "M:\n";
    for (std::size_t r = 0; r < M.rows(); ++r)
    {
        for (std::size_t c = 0; c < M.cols(); ++c)
        {
            std::cout << M(r, c) << "\t";
        }
        std::cout << "\n";
    }

    miv::matrix<int> A{{1,2,3},{4,5,6}};
    miv::matrix<int> B{{7,8,9},{1,2,3}};

    auto C = miv::math::add(A, B);
    auto H = miv::math::hadamard(A, B);
    auto AT = miv::math::transpose(A);

    std::cout << "norm_l2(A)=" << miv::math::norm_l2(A) << "\n";
    std::cout << "min(A)=" << miv::math::min_element(A) << "\n";

    miv::matrix<double> v(1, 3);
    v(0,0)=3; v(0,1)=4; v(0,2)=0;

    auto vn = miv::math::normalize_l2(v);

    auto s = miv::math::sin(vn);
    auto c = miv::math::cos(vn);

    for (std::size_t c = 0; c < s.size(); ++c)
    {
        std::cout << s(0, c) << "\t";
    }
    std::cout << "\n";

    miv::matrix<double> A_sys{{1.0, 1.0}, {2.0, -1.0}};
    miv::matrix<double> b_sys{{3.0}, {0.0}};
    miv::math::equation_system<double> system(A_sys, b_sys);
    auto solution = system.solve_lup();

    std::cout << "Solution for system Ax=b:\n";
    for (std::size_t i = 0; i < solution.rows(); ++i)
    {
        std::cout << "x" << i << " = " << solution(i, 0) << "\n";
    }

    return 0;
}
