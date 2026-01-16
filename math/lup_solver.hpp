#ifndef MIV_MATH_LUP_SOLVER_H
#define MIV_MATH_LUP_SOLVER_H

#include <cstddef>
#include <utility>
#include <concepts>
#include <algorithm>

#include "containers/array.hpp"
#include "containers/matrix.hpp"

#include "linalg.hpp"
#include "helpers.hpp"
#include "equation_system.hpp"

namespace miv::math
{
    template <Number T>
    inline void permutate_system(equation_system<T> &system)
    {
        miv::array<std::size_t> perm(system.n());

        for (std::size_t p = 0; p < system.n(); ++p)
        {
            T max = 0;
            std::iota(perm.begin(), perm.end(), 0);

            for (std::size_t k = p; k < system.n(); ++k)
            {
                T val = std::abs(system.A(k, p));

                if (val >= max)
                {
                    perm[p] = k;
                    perm[k] = p;
                    max = val;
                }
            }

            system.A.row_permute(perm);
            permute_vector(system.b, perm);
        }
    }

    template <Number T> requires std::is_floating_point_v<T>
    inline std::pair<miv::matrix<norm_t>, miv::matrix<norm_t>> lu_decompose(const miv::matrix<T> &A)
    {
        require_squarness(A);

        miv::matrix<norm_t> L = identity(A.rows());
        miv::matrix<norm_t> U(A);
        
        // iterate over all columns except the last one
        for (std::size_t i = 0; i < A.cols() - 1; ++i)
        {
            // Go down the column row by row
            for (std::size_t j = (i + 1); j < A.rows(); ++j)
            {
                // calc m coefficient
                auto m = U(j, i) / U(i, i);

                // zero elem in current column
                U(j, i) = 0;

                // update upper-diagonal values (U)
                for (std::size_t k = (i + 1); k < A.cols(); ++k)
                {
                    U(j, k) = U(j, k) - m * U(i, k);
                }

                // set lower diagonal value (L)
                L(j, i) = m;
            }
        }

        return std::make_pair(L, U);
    }

    template <typename T> requires std::is_floating_point_v<T>
    inline miv::matrix<norm_t> lup_solve(equation_system<T> &system)
    {
        // 1) Pivoting
        permutate_system(system);

        // 2) LU Decompose
        auto [L, U] = lu_decompose(system.A);

        // 3) Forward
        miv::matrix<norm_t> y(system.n(), 1);

        for (std::size_t i = 0; i < system.n(); ++i)
        {
            norm_t l_sum = 0;

            for (std::size_t j = 0; j < i; ++j)
            {
                l_sum += (L(i, j) * y(j, 0));
            }

            y(i, 0) = vector_at(system.b, i) - l_sum;
        }

        // 4) Backward
        miv::matrix<norm_t> x(system.n(), 1);

        for (std::size_t i = system.n(); i > 0; --i)
        {
            norm_t u_sum = 0;

            for (std::size_t j = i; j < system.n(); ++j)
            {
                u_sum += (U(i, j) * x(j, 0));
            }

            x(i - 1, 0) = (vector_at(y, i - 1) - u_sum) / U(i - 1, i - 1);
        }

        return x;
    }
}

#endif // MIV_MATH_LUP_SOLVER_H