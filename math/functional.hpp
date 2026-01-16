#ifndef MIV_MATH_FUNCTIONAL_H
#define MIV_MATH_FUNCTIONAL_H

#include <cstddef>
#include <concepts>
#include <type_traits>
#include <cmath>
#include <stdexcept>

#include "math/helpers.hpp"

namespace miv::math
{
    /**
     * @brief Применить функцию к каждому элементу матрицы и вернуть новую матрицу.
     *
     * Функция f должна быть вызываема как:
     *   U f(T)
     *
     * Где U тоже должен быть числом (Number), иначе это уже не "математика матриц".
     *
     * @tparam T Тип элементов исходной матрицы
     * @tparam F Тип функции
     * @return matrix<U>
     */
    template <Number T, typename F>
    requires std::invocable<F, T> && Number<std::invoke_result_t<F, T>>
    inline miv::matrix<std::invoke_result_t<F, T>> apply(const miv::matrix<T> &a, F f)
    {
        using U = std::invoke_result_t<F, T>;

        miv::matrix<U> out(a.rows(), a.cols());

        for (std::size_t i = 0; i < a.size(); ++i)
        {
            out.data()[i] = static_cast<U>(f(a.data()[i]));
        }

        return out;
    }

    /**
     * @brief Применить функцию к каждому элементу матрицы на месте (in-place).
     *
     * f может быть:
     * - void f(T&)
     * - T f(T)
     * - T& f(T&)
     *
     * Мы применяем её аккуратно: если f возвращает значение — записываем его обратно.
     * Если f возвращает void — считаем, что оно само модифицировало T&.
     */
    template <Number T, typename F>
    requires std::invocable<F, T&>
    inline void apply_inplace(miv::matrix<T> &a, F f)
    {
        for (std::size_t i = 0; i < a.size(); ++i)
        {
            if constexpr (std::is_void_v<std::invoke_result_t<F, T&>>)
            {
                f(a.data()[i]);
            }
            else
            {
                a.data()[i] = static_cast<T>(f(a.data()[i]));
            }
        }
    }

    /**
     * @brief sin поэлементно.
     *
     * Работает только для float/double/long double (целые типы тут неуместны).
     */
    template <Number T>
    requires std::is_floating_point_v<T>
    inline miv::matrix<T> sin(const miv::matrix<T> &a)
    {
        return apply(a, [](T x) -> T { return static_cast<T>(std::sin(x)); });
    }

    /**
     * @brief cos поэлементно.
     *
     * Работает только для float/double/long double.
     */
    template <Number T>
    requires std::is_floating_point_v<T>
    inline miv::matrix<T> cos(const miv::matrix<T> &a)
    {
        return apply(a, [](T x) -> T { return static_cast<T>(std::cos(x)); });
    }

} // namespace miv::math

#endif // MIV_MATH_FUNCTIONAL_H
