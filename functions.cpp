#include "functions.hpp"

#include <cmath>
#include <vector>

/**
 * @file functions.cpp
 * @brief Пользовательские функции системы F(x)=0 и подробная документация.
 *
 * @details
 * ---------------------------------------------------------------------------
 * ПРАВИЛА
 * ---------------------------------------------------------------------------
 * 1) Размерность системы n = количеству функций в get_system_functions().
 * 2) Каждая функция читает вектор x и возвращает скаляр T.
 * 3) Тип FuncFloat задаёт точность вычислений всей системы:
 *      - Замените typedef FuncFloat на Float32/Float64/Float80/Float128 в functions.hpp.
 *      - Все функции автоматически соберутся под новую точность.
 *
 * Пример объявления функции:
 * @code
 * template <miv::math::FloatNumber T>
 * inline T func1(miv::array<T> &x)
 * {
 *     return std::cos(x[0]) + x[1] + std::pow(x[2], 2);
 * }
 * @endcode
 *
 * ВАЖНО:
 * - Функция принимает miv::array<T>& x и возвращает T.
 * - Функция НЕ должна менять x (параметр не const по соглашению).
 * - Пользователь сам отвечает за индексы x[i].
 *
 * ---------------------------------------------------------------------------
 * ПРИМЕРЫ (закомментированные; используйте как шаблон)
 * ---------------------------------------------------------------------------
 * 1) Классическая 2D система:
 * @code
 * template <miv::math::FloatNumber T>
 * inline T example2d_f1(miv::array<T> &x)
 * {
 *     return x[0] * x[0] + x[1] * x[1] - static_cast<T>(1);
 * }
 *
 * template <miv::math::FloatNumber T>
 * inline T example2d_f2(miv::array<T> &x)
 * {
 *     return x[0] - x[1];
 * }
 * @endcode
 *
 * 2) 3D система (пример с cos/sin):
 * @code
 * template <miv::math::FloatNumber T>
 * inline T example3d_f1(miv::array<T> &x)
 * {
 *     return std::cos(x[0]) + x[1] + x[2];
 * }
 *
 * template <miv::math::FloatNumber T>
 * inline T example3d_f2(miv::array<T> &x)
 * {
 *     return std::sin(x[1]) + x[0] - x[2];
 * }
 *
 * template <miv::math::FloatNumber T>
 * inline T example3d_f3(miv::array<T> &x)
 * {
 *     return x[0] * x[0] + x[1] * x[1] + x[2] * x[2] - static_cast<T>(1);
 * }
 * @endcode
 *
 * 3) Полиномиальная система:
 * @code
 * template <miv::math::FloatNumber T>
 * inline T poly_f1(miv::array<T> &x)
 * {
 *     return x[0] * x[0] * x[0] - x[1] + static_cast<T>(2);
 * }
 *
 * template <miv::math::FloatNumber T>
 * inline T poly_f2(miv::array<T> &x)
 * {
 *     return x[1] * x[1] - x[0] + static_cast<T>(1);
 * }
 * @endcode
 *
 * 4) Пример с разделением переменных:
 * @code
 * template <miv::math::FloatNumber T>
 * inline T sep_f1(miv::array<T> &x)
 * {
 *     return std::sin(x[0]) - static_cast<T>(0.5);
 * }
 *
 * template <miv::math::FloatNumber T>
 * inline T sep_f2(miv::array<T> &x)
 * {
 *     return std::exp(x[1]) - static_cast<T>(3);
 * }
 * @endcode
 *
 * ---------------------------------------------------------------------------
 * КАК ДОБАВИТЬ СВОЮ ФУНКЦИЮ
 * ---------------------------------------------------------------------------
 * 1) Создайте новую функцию в стиле template <FloatNumber T>.
 * 2) Проверьте индексы x[i].
 * 3) НЕ забудьте добавить её в return { ... } ниже.
 */

// ============================================================================
//                  Реальная система уравнений (пример)
// ============================================================================

/**
 * @brief Пример: уравнение 1 (2D система).
 *
 * f1(x, y) = x^2 + y^2 - 1
 */
template <miv::math::FloatNumber T>
inline T func1(miv::array<T> &x)
{
    return x[0] * x[0] + x[1] * x[1] - static_cast<T>(1);
}

/**
 * @brief Пример: уравнение 2 (2D система).
 *
 * f2(x, y) = x - y
 */
template <miv::math::FloatNumber T>
inline T func2(miv::array<T> &x)
{
    return x[0] - x[1];
}

/**
 * @brief Возвращает список функций системы.
 */
std::vector<NonlinearFunction<FuncFloat>> get_system_functions()
{
    // TODO: добавь свою функцию выше и не забудь включить её сюда:
    return { &func1<FuncFloat>, &func2<FuncFloat> };
}
