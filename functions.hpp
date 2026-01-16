#ifndef QM_FUNCTIONS_HPP
#define QM_FUNCTIONS_HPP

#include <vector>
#include <cstddef>

#if __has_include(<stdfloat>)
    #include <stdfloat>
#endif

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

// ---- Float16 ----
#ifdef __STDCPP_FLOAT16_T__
    using Float16 = std::float16_t;
#else
    using Float16 = float; // fallback
#endif

// ---- Float32 ----
#ifdef __STDCPP_FLOAT32_T__
    using Float32 = std::float32_t;
#else
    using Float32 = float; // fallback
#endif

// ---- Float64 ----
#ifdef __STDCPP_FLOAT64_T__
    using Float64 = std::float64_t;
#else
    using Float64 = double; // fallback
#endif

// ---- Float80 ----
// В стандарте нет std::float80_t, поэтому только long double.
using Float80 = long double;

// ---- Float128 ----
#ifdef __STDCPP_FLOAT128_T__
    using Float128 = std::float128_t;
#else
    using Float128 = long double; // fallback
#endif

/// Тип точности для всей системы: перепишите на Float32/Float64/Float80/Float128.
using FuncFloat = Float128;

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
