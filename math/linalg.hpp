#ifndef MIV_MATH_LINALG_H
#define MIV_MATH_LINALG_H

#include <cstddef>
#include <concepts>
#include <type_traits>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "containers/matrix.hpp"
#include "math/helpers.hpp"

namespace miv::math
{
    // ============================================================
    //               Element-wise arithmetic: +, -
    // ============================================================

    /**
     * @brief Сложение матриц одинакового размера: C = A + B
     *
     * @param a Левая матрица
     * @param b Правая матрица
     * @return Новая матрица (результат)
     * @throws std::invalid_argument если размеры не совпадают
     */
    template <Number T>
    inline miv::matrix<T> add(const miv::matrix<T> &a, const miv::matrix<T> &b)
    {
        require_same_shape(a, b, "add(): matrices must have the same shape");

        miv::matrix<T> out(a.rows(), a.cols());

        for (std::size_t i = 0; i < a.size(); ++i)
        {
            out.data()[i] = a.data()[i] + b.data()[i];
        }

        return out;
    }

    /**
     * @brief Разность матриц одинакового размера: C = A - B
     *
     * @throws std::invalid_argument если размеры не совпадают
     */
    template <Number T>
    inline miv::matrix<T> sub(const miv::matrix<T> &a, const miv::matrix<T> &b)
    {
        require_same_shape(a, b, "sub(): matrices must have the same shape");

        miv::matrix<T> out(a.rows(), a.cols());

        for (std::size_t i = 0; i < a.size(); ++i)
        {
            out.data()[i] = a.data()[i] - b.data()[i];
        }

        return out;
    }

    /**
     * @brief Унарный минус: C = -A
     */
    template <Number T>
    inline miv::matrix<T> negate(const miv::matrix<T> &a)
    {
        miv::matrix<T> out(a.rows(), a.cols());

        for (std::size_t i = 0; i < a.size(); ++i)
        {
            out.data()[i] = -a.data()[i];
        }

        return out;
    }

    // ============================================================
    //               Scalar operations: A*k, A/k
    // ============================================================

    /**
     * @brief Умножение матрицы на скаляр: C = A * k
     */
    template <Number T>
    inline miv::matrix<T> mul_scalar(const miv::matrix<T> &a, T k)
    {
        miv::matrix<T> out(a.rows(), a.cols());

        for (std::size_t i = 0; i < a.size(); ++i)
        {
            out.data()[i] = a.data()[i] * k;
        }

        return out;
    }

    /**
     * @brief Деление матрицы на скаляр: C = A / k
     *
     * Для целых типов это будет целочисленное деление (как в C++).
     * Если нужно вещественное — используй матрицу float/double.
     *
     * @throws std::invalid_argument если k == 0 (для целых типов), либо k == 0.0 (для float)
     */
    template <Number T>
    inline miv::matrix<T> div_scalar(const miv::matrix<T> &a, T k)
    {
        if (k == T{})
        {
            throw std::invalid_argument("div_scalar(): division by zero");
        }

        miv::matrix<T> out(a.rows(), a.cols());

        for (std::size_t i = 0; i < a.size(); ++i)
        {
            out.data()[i] = a.data()[i] / k;
        }

        return out;
    }

    // ============================================================
    //                Matrix products: mmul, hadamard
    // ============================================================

    /**
     * @brief Матричное произведение: C = A * B
     *
     * Требование совместимости:
     * - A: (m x n)
     * - B: (n x k)
     * - C: (m x k)
     *
     * @throws std::invalid_argument если A.cols != B.rows
     */
    template <Number T>
    inline miv::matrix<T> matmul(const miv::matrix<T> &a, const miv::matrix<T> &b)
    {
        require_mmul_compatible(a, b);

        const std::size_t m = a.rows();
        const std::size_t n = a.cols();
        const std::size_t k = b.cols();

        miv::matrix<T> out(m, k);
        out.fill(T{});

        // Классический O(m*n*k)
        for (std::size_t i = 0; i < m; ++i)
        {
            for (std::size_t t = 0; t < n; ++t)
            {
                const T av = a(i, t);
                for (std::size_t j = 0; j < k; ++j)
                {
                    out(i, j) += av * b(t, j);
                }
            }
        }

        return out;
    }

    /**
     * @brief Произведение Адамара (Hadamard): C = A ⊙ B (поэлементное умножение)
     *
     * @throws std::invalid_argument если размеры не совпадают
     */
    template <Number T>
    inline miv::matrix<T> hadamard(const miv::matrix<T> &a, const miv::matrix<T> &b)
    {
        require_same_shape(a, b, "hadamard(): matrices must have the same shape");

        miv::matrix<T> out(a.rows(), a.cols());

        for (std::size_t i = 0; i < a.size(); ++i)
        {
            out.data()[i] = a.data()[i] * b.data()[i];
        }

        return out;
    }

    // ============================================================
    //                        Dot product
    // ============================================================

    /**
     * @brief Скалярное произведение векторов: <a, b>
     *
     * Вектором считается матрица:
     * - 1 x N (строка-вектор)
     * - N x 1 (столбец-вектор)
     *
     * Длины должны совпадать.
     *
     * @return Скаляр (long double)
     * @throws std::invalid_argument если входы не векторы или длины не равны
     */
    template <Number T>
    inline norm_t dot(const miv::matrix<T> &a, const miv::matrix<T> &b)
    {
        const std::size_t na = vector_length(a);
        const std::size_t nb = vector_length(b);

        if (na != nb)
        {
            throw std::invalid_argument("dot(): vectors must have the same length");
        }

        norm_t acc = 0;

        for (std::size_t i = 0; i < na; ++i)
        {
            acc += static_cast<norm_t>(vector_at(a, i)) * static_cast<norm_t>(vector_at(b, i));
        }

        return acc;
    }

    // ============================================================
    //                         Transpose
    // ============================================================

    /**
     * @brief Транспонирование матрицы: B = A^T
     *
     * (rows x cols) -> (cols x rows)
     */
    template <Number T>
    inline miv::matrix<T> transpose(const miv::matrix<T> &a)
    {
        miv::matrix<T> out(a.cols(), a.rows());

        for (std::size_t r = 0; r < a.rows(); ++r)
        {
            for (std::size_t c = 0; c < a.cols(); ++c)
            {
                out(c, r) = a(r, c);
            }
        }

        return out;
    }

    // ============================================================
    //                 Min/Max (elements + element-wise)
    // ============================================================

    /**
     * @brief Минимальный элемент матрицы.
     *
     * @throws std::invalid_argument если матрица пустая
     */
    template <Number T>
    inline T min_element(const miv::matrix<T> &a)
    {
        if (a.size() == 0)
        {
            throw std::invalid_argument("min_element(): matrix is empty");
        }

        return *std::min_element(a.begin(), a.end());
    }

    /**
     * @brief Максимальный элемент матрицы.
     *
     * @throws std::invalid_argument если матрица пустая
     */
    template <Number T>
    inline T max_element(const miv::matrix<T> &a)
    {
        if (a.size() == 0)
        {
            throw std::invalid_argument("max_element(): matrix is empty");
        }

        return *std::max_element(a.begin(), a.end());
    }

    /**
     * @brief Поэлементный минимум: C[i] = min(A[i], B[i])
     *
     * @throws std::invalid_argument если размеры не совпадают
     */
    template <Number T>
    inline miv::matrix<T> elementwise_min(const miv::matrix<T> &a, const miv::matrix<T> &b)
    {
        require_same_shape(a, b, "elementwise_min(): matrices must have the same shape");

        miv::matrix<T> out(a.rows(), a.cols());

        for (std::size_t i = 0; i < a.size(); ++i)
        {
            out.data()[i] = std::min(a.data()[i], b.data()[i]);
        }

        return out;
    }

    /**
     * @brief Поэлементный максимум: C[i] = max(A[i], B[i])
     *
     * @throws std::invalid_argument если размеры не совпадают
     */
    template <Number T>
    inline miv::matrix<T> elementwise_max(const miv::matrix<T> &a, const miv::matrix<T> &b)
    {
        require_same_shape(a, b, "elementwise_max(): matrices must have the same shape");

        miv::matrix<T> out(a.rows(), a.cols());

        for (std::size_t i = 0; i < a.size(); ++i)
        {
            out.data()[i] = std::max(a.data()[i], b.data()[i]);
        }

        return out;
    }

    // ============================================================
    //                             Norms
    // ============================================================

    /**
     * @brief L1-норма матрицы/вектора: sum(|x_i|)
     *
     * Для матрицы считается по всем элементам.
     */
    template <Number T>
    inline norm_t norm_l1(const miv::matrix<T> &a)
    {
        norm_t acc = 0;

        for (std::size_t i = 0; i < a.size(); ++i)
        {
            acc += std::abs(static_cast<norm_t>(a.data()[i]));
        }

        return acc;
    }

    /**
     * @brief L2-норма матрицы/вектора:
     * sqrt(sum(x_i^2))
     *
     * Для матриц это эквивалентно Фробениусовой норме.
     */
    template <Number T>
    inline norm_t norm_l2(const miv::matrix<T> &a)
    {
        norm_t acc = 0;

        for (std::size_t i = 0; i < a.size(); ++i)
        {
            const norm_t v = static_cast<norm_t>(a.data()[i]);
            acc += v * v;
        }

        return std::sqrt(acc);
    }

    /**
     * @brief L_inf-норма (бесконечная): max(|x_i|)
     */
    template <Number T>
    inline norm_t norm_linf(const miv::matrix<T> &a)
    {
        norm_t best = 0;

        for (std::size_t i = 0; i < a.size(); ++i)
        {
            best = std::max(best, std::abs(static_cast<norm_t>(a.data()[i])));
        }

        return best;
    }

    // ============================================================
    //                          Normalize
    // ============================================================

    /**
     * @brief Нормализация вектора по L2: v := v / ||v||_2
     *
     * Вектором считается матрица:
     * - 1xN (строка)
     * - Nx1 (столбец)
     *
     * Возврат:
     * - если T floating-point -> вернём matrix<T>
     * - если T integral       -> вернём matrix<long double>, иначе всё “обнулится”
     *
     * @param v Вектор-матрица
     * @param eps Защита от деления на ноль (если норма слишком мала)
     * @throws std::invalid_argument если v не вектор
     */
    template <Number T>
    inline auto normalize_l2(const miv::matrix<T> &v, norm_t eps = static_cast<norm_t>(1e-18))
    {
        // Проверим, что это именно вектор
        (void)vector_length(v);

        const norm_t n = norm_l2(v);

        if (n <= eps)
        {
            // Нулевой (или почти нулевой) вектор: нормализовать нельзя корректно
            throw std::invalid_argument("normalize_l2(): vector norm is too small (division by zero protection)");
        }

        if constexpr (std::is_floating_point_v<T>)
        {
            miv::matrix<T> out(v.rows(), v.cols());

            for (std::size_t i = 0; i < v.size(); ++i)
            {
                out.data()[i] = static_cast<T>(static_cast<norm_t>(v.data()[i]) / n);
            }

            return out;
        }
        else
        {
            miv::matrix<norm_t> out(v.rows(), v.cols());

            for (std::size_t i = 0; i < v.size(); ++i)
            {
                out.data()[i] = static_cast<norm_t>(v.data()[i]) / n;
            }

            return out;
        }
    }

    /**
     * @brief Нормализация вектора по L1: v := v / ||v||_1
     */
    template <Number T>
    inline auto normalize_l1(const miv::matrix<T> &v, norm_t eps = static_cast<norm_t>(1e-18))
    {
        (void)vector_length(v);

        const norm_t n = norm_l1(v);

        if (n <= eps)
        {
            throw std::invalid_argument("normalize_l1(): vector norm is too small (division by zero protection)");
        }

        if constexpr (std::is_floating_point_v<T>)
        {
            miv::matrix<T> out(v.rows(), v.cols());

            for (std::size_t i = 0; i < v.size(); ++i)
            {
                out.data()[i] = static_cast<T>(static_cast<norm_t>(v.data()[i]) / n);
            }

            return out;
        }
        else
        {
            miv::matrix<norm_t> out(v.rows(), v.cols());

            for (std::size_t i = 0; i < v.size(); ++i)
            {
                out.data()[i] = static_cast<norm_t>(v.data()[i]) / n;
            }

            return out;
        }
    }

    // ============================================================
    //                  Extra basics (useful additions)
    // ============================================================

    /**
     * @brief Сумма всех элементов матрицы.
     */
    template <Number T>
    inline norm_t sum(const miv::matrix<T> &a)
    {
        norm_t acc = 0;

        for (std::size_t i = 0; i < a.size(); ++i)
        {
            acc += static_cast<norm_t>(a.data()[i]);
        }

        return acc;
    }

    /**
     * @brief Среднее значение элементов матрицы.
     *
     * @throws std::invalid_argument если матрица пустая
     */
    template <Number T>
    inline norm_t mean(const miv::matrix<T> &a)
    {
        if (a.size() == 0)
        {
            throw std::invalid_argument("mean(): matrix is empty");
        }

        return sum(a) / static_cast<norm_t>(a.size());
    }

    /**
     * @brief След матрицы: trace(A) = sum(A[i,i])
     *
     * @throws std::invalid_argument если матрица не квадратная
     */
    template <Number T>
    inline norm_t trace(const miv::matrix<T> &a)
    {
        if (a.rows() != a.cols())
        {
            throw std::invalid_argument("trace(): matrix must be square");
        }

        norm_t acc = 0;

        for (std::size_t i = 0; i < a.rows(); ++i)
        {
            acc += static_cast<norm_t>(a(i, i));
        }

        return acc;
    }

    /**
     * @brief Единичная матрица I размера n x n.
     */
    template <Number T>
    inline miv::matrix<T> identity(std::size_t n)
    {
        miv::matrix<T> out(n, n);
        out.fill(T{});

        for (std::size_t i = 0; i < n; ++i)
        {
            out(i, i) = static_cast<T>(1);
        }

        return out;
    }

} // namespace miv::math

#endif // MIV_MATH_LINALG_H
