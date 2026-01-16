#ifndef QM_FUNCTIONS_HPP
#define QM_FUNCTIONS_HPP

#include <vector>

#include "containers/array.hpp"
#include "math/helpers.hpp"

/**
 * @file functions.hpp
 * @brief Интерфейс пользовательских нелинейных функций F(x) для системы F(x)=0.
 *
 * @details
 * Пользователь определяет набор функций, возвращаемых get_system_functions().
 * Размерность системы n равна количеству функций в этом векторе.
 * Каждая функция принимает вектор x (miv::array<T>&) и возвращает значение T.
 * Тип FuncFloat задаёт точность вычислений ВСЕЙ системы: достаточно изменить typedef,
 * и все функции автоматически собираются под новую точность.
 */

// ============================================================================
//                           Типы точности (user)
// ============================================================================
// Эти алиасы должны существовать, чтобы пользователь мог легко выбрать точность.
// Если в проекте появятся реальные half/float128 типы — замените здесь.

using Float16 = float;        ///< Заглушка: если нет half/float16, используем float.
using Float32 = float;
using Float64 = double;
using Float80 = long double;  ///< Обычно соответствует расширенной точности (если поддерживается).
using Float128 = long double; ///< Заглушка: если нет 128-битного типа, используем long double.

/// Тип точности для всей системы: перепишите на Float32/Float64/Float80/Float128.
typedef Float128 FuncFloat;

/**
 * @brief Указатель на нелинейную функцию системы F(x)=0.
 *
 * @tparam T Тип чисел (FloatNumber)
 *
 * Каждая функция:
 *  - принимает miv::array<T>& x
 *  - возвращает значение T
 *  - НЕ должна изменять x (хотя параметр не const)
 */
template <miv::math::FloatNumber T>
using NonlinearFunction = T(*)(miv::array<T> &);

/**
 * @brief Возвращает список функций системы.
 *
 * Количество элементов в векторе = размерность системы n.
 *
 * @return std::vector<NonlinearFunction<FuncFloat>>
 */
std::vector<NonlinearFunction<FuncFloat>> get_system_functions();

#endif // QM_FUNCTIONS_HPP
