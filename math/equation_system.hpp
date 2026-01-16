#ifndef MIV_MATH_EQUATION_SYSTEM_H
#define MIV_MATH_EQUATION_SYSTEM_H

#include <string>
#include <utility>
#include <stdexcept>

#include "containers/matrix.hpp"
#include "math/helpers.hpp"   // Number, require_same_shape, etc.

namespace miv::math
{
    template <Number T>
    class equation_system
    {
    public:
        miv::matrix<T> A;
        miv::matrix<T> b;

        // x хранить не обязательно, его чаще ищут
        // но можно держать как опциональное поле
        // miv::matrix<T> x;

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
         * @brief Размерность системы (n).
         */
        std::size_t n() const { return A.rows(); }
    };
}

#endif // MIV_MATH_EQUATION_SYSTEM_H