#ifndef MIV_MATH_EQUATION_SYSTEM_H
#define MIV_MATH_EQUATION_SYSTEM_H

#include <string>
#include <utility>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <numeric>

#include "containers/matrix.hpp"
#include "math/helpers.hpp"   // Number, require_same_shape, etc.
#include "math/linalg.hpp"    // identity

namespace miv::math
{
    /**
     * @brief Система линейных уравнений Ax = b.
     *
     * Работает только с квадратными матрицами A (n x n) и вектором b длины n.
     * Внутренний тип системы — только FloatNumber (float/double/long double).
     * Позволяет выполнять решение методами разложения (например, LUP).
     */
    template <FloatNumber T>
    class equation_system
    {
    public:
        using value_t = T;

        miv::matrix<T> A;
        miv::matrix<T> b;

        /**
         * @brief Создать систему Ax=b.
         *
         * Требования:
         *  - A квадратная (n x n)
         *  - b размерности (n x 1) или (1 x n) если ты хочешь разрешить строку-вектор
         */
        equation_system(miv::matrix<T> A_, miv::matrix<T> b_)
            : A(std::move(A_)), b(std::move(b_))
        {
            require_squareness(A);

            const std::size_t n = A.rows();

            // b должен быть вектором длины n
            if (!is_vector(b) || vector_length(b) != n)
            {
                throw std::invalid_argument(
                    "equation_system: b must be a vector of length n = " + std::to_string(n) +
                    ", but got shape " + std::to_string(b.rows()) + "x" + std::to_string(b.cols()));
            }
        }

        /**
         * @brief Конструктор с автоматическим приведением к типу T.
         *
         * Удобно, если исходные матрицы заданы целыми или другим float-типом.
         */
        template <Number U>
        equation_system(const miv::matrix<U> &A_, const miv::matrix<U> &b_)
            : A(cast_matrix(A_)), b(cast_matrix(b_))
        {
            require_squareness(A);

            const std::size_t n = A.rows();

            if (!is_vector(b) || vector_length(b) != n)
            {
                throw std::invalid_argument(
                    "equation_system: b must be a vector of length n = " + std::to_string(n) +
                    ", but got shape " + std::to_string(b.rows()) + "x" + std::to_string(b.cols()));
            }
        }

        /**
         * @brief Размерность системы (n).
         */
        std::size_t n() const { return A.rows(); }

        /**
         * @brief Решить систему методом LUP-разложения.
         *
         * Работает только с типами float/double/long double.
         */
        miv::matrix<T> solve_lup() const
        {
            auto A_work = A;
            auto b_work = b;

            auto [L, U] = lu_decompose_lup(A_work, b_work);

            const auto y = forward_substitution(L, b_work);
            return backward_substitution(U, y);
        }

    private:
        /**
         * @brief LUP-разложение с частичным выбором главного элемента.
         *
         * Функция изменяет A и b на месте (переставляет строки).
         *
         * @throws std::invalid_argument если матрица вырожденная.
         */
        static std::pair<miv::matrix<T>, miv::matrix<T>> lu_decompose_lup(
            miv::matrix<T> &A_work,
            miv::matrix<T> &b_work)
        {
            const std::size_t n = A_work.rows();
            const T eps = static_cast<T>(1e-18);

            miv::matrix<T> L = identity<T>(n);
            miv::matrix<T> U = A_work;

            for (std::size_t k = 0; k < n; ++k)
            {
                // Найдём опорный элемент в столбце k
                std::size_t pivot = k;
                T max_val = std::abs(U(k, k));

                for (std::size_t i = k + 1; i < n; ++i)
                {
                    const T val = std::abs(U(i, k));
                    if (val > max_val)
                    {
                        max_val = val;
                        pivot = i;
                    }
                }

                if (max_val <= eps)
                {
                    throw std::invalid_argument("equation_system::solve_lup(): matrix is singular or near-singular");
                }

                if (pivot != k)
                {
                    // Переставляем строки в U и b
                    const auto perm = build_swap_perm(n, k, pivot);
                    U.row_permute(perm);
                    permute_vector(b_work, perm);

                    // В L переставляем только уже вычисленные столбцы (0..k-1)
                    for (std::size_t j = 0; j < k; ++j)
                    {
                        std::swap(L(k, j), L(pivot, j));
                    }
                }

                for (std::size_t i = k + 1; i < n; ++i)
                {
                    const T m = U(i, k) / U(k, k);
                    L(i, k) = m;
                    U(i, k) = static_cast<T>(0);

                    for (std::size_t j = k + 1; j < n; ++j)
                    {
                        U(i, j) -= m * U(k, j);
                    }
                }
            }

            return std::make_pair(L, U);
        }

        /**
         * @brief Прямая подстановка для L y = b.
         */
        static miv::matrix<T> forward_substitution(
            const miv::matrix<T> &L,
            const miv::matrix<T> &b_work)
        {
            const std::size_t n = L.rows();
            miv::matrix<T> y(n, 1);

            for (std::size_t i = 0; i < n; ++i)
            {
                T sum = 0;
                for (std::size_t j = 0; j < i; ++j)
                {
                    sum += L(i, j) * y(j, 0);
                }
                y(i, 0) = vector_at(b_work, i) - sum;
            }

            return y;
        }

        /**
         * @brief Обратная подстановка для U x = y.
         */
        static miv::matrix<T> backward_substitution(
            const miv::matrix<T> &U,
            const miv::matrix<T> &y)
        {
            const std::size_t n = U.rows();
            miv::matrix<T> x(n, 1);

            for (std::size_t i = n; i > 0; --i)
            {
                const std::size_t row = i - 1;
                T sum = 0;

                for (std::size_t j = row + 1; j < n; ++j)
                {
                    sum += U(row, j) * x(j, 0);
                }

                x(row, 0) = (vector_at(y, row) - sum) / U(row, row);
            }

            return x;
        }

        /**
         * @brief Привести матрицу к типу T (float/double/long double).
         */
        template <Number U>
        static miv::matrix<T> cast_matrix(const miv::matrix<U> &src)
        {
            miv::matrix<T> out(src.rows(), src.cols());

            for (std::size_t i = 0; i < src.size(); ++i)
            {
                out.data()[i] = static_cast<T>(src.data()[i]);
            }

            return out;
        }

        /**
         * @brief Построить перестановку для обмена двух строк.
         */
        static miv::array<std::size_t> build_swap_perm(std::size_t n, std::size_t i, std::size_t j)
        {
            miv::array<std::size_t> perm(n);
            std::iota(perm.begin(), perm.end(), 0);
            std::swap(perm[i], perm[j]);
            return perm;
        }
    };
}

#endif // MIV_MATH_EQUATION_SYSTEM_H
