#ifndef MIV_MATH_HELPERS_H
#define MIV_MATH_HELPERS_H

#include <cstddef>
#include <concepts>
#include <type_traits>
#include <stdexcept>
#include <string>

#include "containers/array.hpp"
#include "containers/matrix.hpp"

namespace miv::math
{
    /**
     * @brief Концепт "числовой тип" для математики.
     *
     * Под числами понимаем:
     * - арифметические типы (int/float/double/long double и т.д.)
     * - но НЕ bool (иначе появятся странные сценарии "математики над флагами")
     */
    template <typename T>
    concept Number = std::is_arithmetic_v<T> && (!std::same_as<T, bool>);

    /**
     * @brief Тип для норм, скалярных произведений и т.п.
     *
     * Даже если матрица int, норму разумно возвращать как вещественную величину.
     */
    using norm_t = long double;

    /**
     * @brief Проверка совпадения размеров матриц.
     *
     * @throws std::invalid_argument если размеры разные
     */
    template <Number T>
    inline void require_same_shape(const miv::matrix<T> &a, const miv::matrix<T> &b, const char *msg)
    {
        if (a.rows() != b.rows() || a.cols() != b.cols())
        {
            throw std::invalid_argument(std::string(msg) +
                " (left=" + std::to_string(a.rows()) + "x" + std::to_string(a.cols()) +
                ", right=" + std::to_string(b.rows()) + "x" + std::to_string(b.cols()) + ")");
        }
    }

    /**
     * @brief Проверка совместимости размеров для матричного произведения: A.cols == B.rows
     *
     * @throws std::invalid_argument если несовместимо
     */
    template <Number T>
    inline void require_mmul_compatible(const miv::matrix<T> &a, const miv::matrix<T> &b)
    {
        if (a.cols() != b.rows())
        {
            throw std::invalid_argument(
                "Matrix multiplication requires A.cols == B.rows "
                "(A=" + std::to_string(a.rows()) + "x" + std::to_string(a.cols()) +
                ", B=" + std::to_string(b.rows()) + "x" + std::to_string(b.cols()) + ")");
        }
    }

    /**
     * @brief Проверка, является ли представленная матрица квадратной
     *
     * @throws std::invalid_argument если не является
     */
    template <Number T>
    inline void require_squareness(const miv::matrix<T> &a)
    {
        if (a.cols() != a.rows())
        {
            throw std::invalid_argument(
                "Given matrix does not meet the requirement of squareness"
                "Shape(" + std::to_string(a.rows()) + "x" + std::to_string(a.cols()) + ")");
        }
    }

    /**
     * @brief Проверка: матрица является вектором (строка 1xN или столбец Nx1).
     */
    template <Number T>
    inline bool is_vector(const miv::matrix<T> &v)
    {
        return (v.rows() == 1) || (v.cols() == 1);
    }

    /**
     * @brief Длина вектора (если матрица интерпретируется как 1xN или Nx1).
     *
     * @throws std::invalid_argument если матрица не является вектором
     */
    template <Number T>
    inline std::size_t vector_length(const miv::matrix<T> &v)
    {
        if (v.rows() == 1) return v.cols();
        if (v.cols() == 1) return v.rows();
        throw std::invalid_argument("Expected a vector (1xN or Nx1)");
    }

    /**
     * @brief Доступ к i-ому элементу вектора (унифицировано для 1xN и Nx1).
     *
     * @throws std::invalid_argument если матрица не является вектором
     */
    template <Number T>
    inline T vector_at(const miv::matrix<T> &v, std::size_t i)
    {
        if (v.rows() == 1)
        {
            return v(0, i);
        }
        if (v.cols() == 1)
        {
            return v(i, 0);
        }
        throw std::invalid_argument("Expected a vector (1xN or Nx1)");
    }

    /**
     * @brief Ссылка на i-ый элемент вектора (унифицировано для 1xN и Nx1).
     *
     * @throws std::invalid_argument если матрица не является вектором
     */
    template <Number T>
    inline T &vector_at_ref(miv::matrix<T> &v, std::size_t i)
    {
        if (v.rows() == 1)
        {
            return v(0, i);
        }
        if (v.cols() == 1)
        {
            return v(i, 0);
        }
        throw std::invalid_argument("Expected a vector (1xN or Nx1)");
    }

    /**
     * @brief Перестановка вектора (унифицировано для 1xN и Nx1).
     *
     * @throws std::invalid_argument если матрица не является вектором
     */
    template <Number T>
    inline void permute_vector(miv::matrix<T> &v, const miv::array<std::size_t> &perm)
    {
        if (v.rows() == 1)
        {
            v.col_permute(perm);
        }
        else if (v.cols() == 1)
        {
            v.row_permute(perm);
        }
        else
        {
            throw std::invalid_argument("Expected a vector (1xN or Nx1)");
        }
    }

} // namespace miv::math

#endif // MIV_MATH_HELPERS_H
