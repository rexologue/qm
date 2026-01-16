#include <cctype>
#include <cmath>
#include <format>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "containers/array.hpp"
#include "containers/matrix.hpp"
#include "math/equation_system.hpp"
#include "math/linalg.hpp"

#include "functions.hpp"

/**
 * @file main.cpp
 * @brief Консольное приложение для решения системы нелинейных уравнений F(x)=0.
 *
 * Реализованы методы:
 *  - Ньютон
 *  - Модифицированный Ньютон (замороженный Якобиан)
 *
 * По умолчанию Якобиан строится численно. Пользователь может ввести матрицу вручную.
 * Вся логика приложения находится в этом файле.
 */

namespace
{
    enum class Method
    {
        Newton = 1,
        ModifiedNewton = 2
    };

    enum class JacobianMode
    {
        Numeric = 1,
        Manual = 2
    };

    enum class NumericFormula
    {
        TwoPoint = 1,
        ThreePoint = 2
    };

    /**
     * @brief Печать массива в формате [a b c].
     */
    template <miv::math::FloatNumber T>
    std::string format_array(const miv::array<T> &x, int precision = 10)
    {
        std::ostringstream out;
        out << std::setprecision(precision) << std::fixed;
        out << "[";
        for (std::size_t i = 0; i < x.size(); ++i)
        {
            out << x[i];
            if (i + 1 < x.size())
            {
                out << " ";
            }
        }
        out << "]";
        return out.str();
    }

    /**
     * @brief Преобразование массива в матрицу-столбец (n x 1).
     */
    template <miv::math::FloatNumber T>
    miv::matrix<T> to_column_matrix(const miv::array<T> &x)
    {
        miv::matrix<T> out(x.size(), 1);
        for (std::size_t i = 0; i < x.size(); ++i)
        {
            out(i, 0) = x[i];
        }
        return out;
    }

    /**
     * @brief Преобразование матрицы-столбца (n x 1) в массив.
     */
    template <miv::math::FloatNumber T>
    miv::array<T> column_matrix_to_array(const miv::matrix<T> &v)
    {
        if (v.cols() != 1)
        {
            throw std::invalid_argument("Expected column matrix (n x 1)");
        }

        miv::array<T> out(v.rows());
        for (std::size_t i = 0; i < v.rows(); ++i)
        {
            out[i] = v(i, 0);
        }
        return out;
    }

    /**
     * @brief Вычислить вектор F(x) в виде матрицы-столбца (n x 1).
     */
    template <miv::math::FloatNumber T>
    miv::matrix<T> compute_F(miv::array<T> &x, const std::vector<NonlinearFunction<T>> &functions)
    {
        const std::size_t n = functions.size();
        miv::matrix<T> out(n, 1);

        for (std::size_t i = 0; i < n; ++i)
        {
            out(i, 0) = functions[i](x);
        }

        return out;
    }

    /**
     * @brief Численное построение Якобиана (двухузловая формула).
     */
    template <miv::math::FloatNumber T>
    miv::matrix<T> build_jacobian_two_point(
        miv::array<T> &x,
        const std::vector<NonlinearFunction<T>> &functions)
    {
        const std::size_t n = functions.size();
        miv::matrix<T> J(n, n);

        const auto fx = compute_F(x, functions);
        const T eps = std::numeric_limits<T>::epsilon();

        for (std::size_t j = 0; j < n; ++j)
        {
            const T h = std::sqrt(eps) * (static_cast<T>(1) + std::abs(x[j]));

            miv::array<T> x_plus = x;
            x_plus[j] += h;
            const auto f_plus = compute_F(x_plus, functions);

            for (std::size_t i = 0; i < n; ++i)
            {
                J(i, j) = (f_plus(i, 0) - fx(i, 0)) / h;
            }
        }

        return J;
    }

    /**
     * @brief Численное построение Якобиана (трёхузловая формула).
     */
    template <miv::math::FloatNumber T>
    miv::matrix<T> build_jacobian_three_point(
        miv::array<T> &x,
        const std::vector<NonlinearFunction<T>> &functions)
    {
        const std::size_t n = functions.size();
        miv::matrix<T> J(n, n);
        const T eps = std::numeric_limits<T>::epsilon();

        for (std::size_t j = 0; j < n; ++j)
        {
            const T h = std::sqrt(eps) * (static_cast<T>(1) + std::abs(x[j]));

            miv::array<T> x_plus = x;
            miv::array<T> x_minus = x;
            x_plus[j] += h;
            x_minus[j] -= h;

            const auto f_plus = compute_F(x_plus, functions);
            const auto f_minus = compute_F(x_minus, functions);

            for (std::size_t i = 0; i < n; ++i)
            {
                J(i, j) = (f_plus(i, 0) - f_minus(i, 0)) / (static_cast<T>(2) * h);
            }
        }

        return J;
    }

    /**
     * @brief Считывание строки и преобразование к числу.
     */
    template <typename T>
    bool parse_single_value(const std::string &line, T &value)
    {
        std::istringstream ss(line);
        ss >> value;
        if (ss.fail())
        {
            return false;
        }
        ss >> std::ws;
        return ss.eof();
    }

    /**
     * @brief Считать одно число (устойчивый ввод).
     */
    template <typename T>
    T read_number(const std::string &prompt)
    {
        while (true)
        {
            std::cout << prompt;
            std::string line;
            if (!std::getline(std::cin, line))
            {
                throw std::runtime_error("Ошибка чтения ввода");
            }

            T value{};
            if (parse_single_value(line, value))
            {
                return value;
            }

            std::cout << "Некорректный ввод. Попробуйте ещё раз.\n";
        }
    }

    /**
     * @brief Считать одно число с проверкой диапазона.
     */
    template <typename T>
    T read_number_in_range(const std::string &prompt, T min_value, T max_value)
    {
        while (true)
        {
            T value = read_number<T>(prompt);
            if (value >= min_value && value <= max_value)
            {
                return value;
            }

            std::cout << "Значение должно быть в диапазоне [" << min_value << ", " << max_value << "].\n";
        }
    }

    /**
     * @brief Считать положительное целое число.
     */
    std::size_t read_positive_size(const std::string &prompt)
    {
        while (true)
        {
            auto value = read_number<long long>(prompt);
            if (value > 0)
            {
                return static_cast<std::size_t>(value);
            }
            std::cout << "Число должно быть положительным.\n";
        }
    }

    /**
     * @brief Считать да/нет (русский интерфейс).
     */
    bool read_yes_no(const std::string &prompt)
    {
        while (true)
        {
            std::cout << prompt;
            std::string line;
            if (!std::getline(std::cin, line))
            {
                throw std::runtime_error("Ошибка чтения ввода");
            }

            for (auto &ch : line)
            {
                ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
            }

            if (line == "да" || line == "д" || line == "yes" || line == "y")
            {
                return true;
            }

            if (line == "нет" || line == "н" || line == "no" || line == "n")
            {
                return false;
            }

            std::cout << "Введите 'да' или 'нет'.\n";
        }
    }

    /**
     * @brief Считать строку из n чисел (вектор).
     */
    template <miv::math::FloatNumber T>
    miv::array<T> read_vector(std::size_t n, const std::string &prompt)
    {
        while (true)
        {
            std::cout << prompt;
            std::string line;
            if (!std::getline(std::cin, line))
            {
                throw std::runtime_error("Ошибка чтения ввода");
            }

            std::istringstream ss(line);
            miv::array<T> x(n);
            bool ok = true;
            for (std::size_t i = 0; i < n; ++i)
            {
                if (!(ss >> x[i]))
                {
                    ok = false;
                    break;
                }
            }
            ss >> std::ws;

            if (ok && ss.eof())
            {
                return x;
            }

            std::cout << "Ожидалось " << n << " чисел. Повторите ввод.\n";
        }
    }

    /**
     * @brief Считать матрицу n x n построчно.
     */
    template <miv::math::FloatNumber T>
    miv::matrix<T> read_matrix(std::size_t n, const std::string &prompt)
    {
        miv::matrix<T> J(n, n);
        std::cout << prompt << "\n";

        for (std::size_t r = 0; r < n; ++r)
        {
            while (true)
            {
                std::cout << "Строка " << (r + 1) << ": ";
                std::string line;
                if (!std::getline(std::cin, line))
                {
                    throw std::runtime_error("Ошибка чтения ввода");
                }

                std::istringstream ss(line);
                bool ok = true;
                for (std::size_t c = 0; c < n; ++c)
                {
                    if (!(ss >> J(r, c)))
                    {
                        ok = false;
                        break;
                    }
                }
                ss >> std::ws;

                if (ok && ss.eof())
                {
                    break;
                }

                std::cout << "Ожидалось " << n << " чисел. Повторите ввод строки.\n";
            }
        }

        return J;
    }

    /**
     * @brief Вывести матрицу (n x n).
     */
    template <miv::math::FloatNumber T>
    void print_matrix(const miv::matrix<T> &J, int precision = 10)
    {
        std::cout << std::setprecision(precision) << std::fixed;
        for (std::size_t r = 0; r < J.rows(); ++r)
        {
            std::cout << "[";
            for (std::size_t c = 0; c < J.cols(); ++c)
            {
                std::cout << J(r, c);
                if (c + 1 < J.cols())
                {
                    std::cout << " ";
                }
            }
            std::cout << "]\n";
        }
    }

    /**
     * @brief Решить линейную систему J s = -F(x) через LUP.
     */
    template <miv::math::FloatNumber T>
    miv::array<T> solve_step(const miv::matrix<T> &J, const miv::matrix<T> &fx)
    {
        miv::matrix<T> b(fx.rows(), 1);
        for (std::size_t i = 0; i < fx.rows(); ++i)
        {
            b(i, 0) = -fx(i, 0);
        }

        miv::math::equation_system<T> sys(J, b);
        const auto s_mat = sys.solve_lup();
        return column_matrix_to_array(s_mat);
    }

    /**
     * @brief Вывести лог одной итерации.
     */
    template <miv::math::FloatNumber T>
    void print_iteration_log(
        std::size_t k,
        const miv::array<T> &x,
        const miv::matrix<T> &fx,
        const miv::array<T> &step,
        T lambda,
        bool damping_enabled)
    {
        const auto fx_norm = miv::math::norm_l2(fx);
        const auto step_norm = miv::math::norm_l2(to_column_matrix(step));
        miv::array<T> x_next = x;
        for (std::size_t i = 0; i < x_next.size(); ++i)
        {
            x_next[i] += lambda * step[i];
        }
        const std::string header = std::format(" Итерация {} ", k);
        std::cout << std::format("\n{:-^70}\n", header);
        std::cout << std::format("{:<14}{}\n", "x^k:", format_array(x));
        std::cout << std::format("{:<14}{}\n", "F(x^k):", format_array(column_matrix_to_array(fx)));
        std::cout << std::format("{:<14}{}\n", "||F||:", fx_norm);
        std::cout << std::format("{:<14}{}\n", "s^k:", format_array(step));
        std::cout << std::format("{:<14}{}\n", "||s||:", step_norm);
        if (damping_enabled)
        {
            std::cout << std::format("{:<14}{}\n", "λ:", lambda);
        }
        std::cout << std::format("{:<14}{}\n", "x^{k+1}:", format_array(x_next));
        std::cout << std::format("{:-^70}\n", "");
    }
}

int main()
{
    using T = FuncFloat;

    try
    {
        while (true)
        {
            std::cout << std::format("\n{:=^70}\n", " Меню ");
            std::cout << "  1) Метод Ньютона\n";
            std::cout << "  2) Модифицированный метод Ньютона (замороженный Якобиан)\n";
            std::cout << "  0) Выход\n";

            const auto method_choice = read_number_in_range<int>("Выберите метод: ", 0, 2);
            if (method_choice == 0)
            {
                std::cout << std::format("\n{}\n\n", "Выход.");
                return 0;
            }

            const Method method = (method_choice == 1) ? Method::Newton : Method::ModifiedNewton;

            const bool damping_enabled = read_yes_no("Использовать демпфирование шага? (да/нет): ");
            T lambda = static_cast<T>(1);
            if (damping_enabled)
            {
                std::cout << std::format("\n{}\n", "Подсказка: x_{k+1} = x_k + λ * s_k");
                while (true)
                {
                    std::cout << "Введите λ в диапазоне (0, 1], по умолчанию 1.0: ";
                    std::string line;
                    if (!std::getline(std::cin, line))
                    {
                        throw std::runtime_error("Ошибка чтения ввода");
                    }

                    if (line.empty())
                    {
                        lambda = static_cast<T>(1);
                        break;
                    }

                    T value{};
                    if (parse_single_value(line, value) && value > static_cast<T>(0) && value <= static_cast<T>(1))
                    {
                        lambda = value;
                        break;
                    }

                    std::cout << "λ должно быть в диапазоне (0, 1].\n";
                }
            }

            std::cout << std::format("\n{:-^70}\n", " Источник Якобиана ");
            std::cout << "  1) Численно (приближение)\n";
            std::cout << "  2) Ввести матрицу Якоби вручную\n";
            const auto jacobian_choice = read_number_in_range<int>("Выберите режим: ", 1, 2);
            const JacobianMode jacobian_mode = (jacobian_choice == 1) ? JacobianMode::Numeric : JacobianMode::Manual;

            NumericFormula numeric_formula = NumericFormula::TwoPoint;
            if (jacobian_mode == JacobianMode::Numeric)
            {
                std::cout << std::format("\n{:-^70}\n", " Численная формула ");
                std::cout << "  1) Двухузловая (односторонняя разность)\n";
                std::cout << "  2) Трёхузловая (центральная разность)\n";
                const auto formula_choice = read_number_in_range<int>("Выберите формулу: ", 1, 2);
                numeric_formula = (formula_choice == 1) ? NumericFormula::TwoPoint : NumericFormula::ThreePoint;
            }

            std::cout << std::format("\n{:-^70}\n", " Параметры остановки ");
            const std::size_t max_iter = read_positive_size("Введите max_iter: ");
            const T eps_F = read_number<T>("Введите eps_F: ");
            const T eps_x = read_number<T>("Введите eps_x: ");

            const auto functions = get_system_functions();
            const std::size_t n = functions.size();

            if (n == 0)
            {
                std::cout << "Список функций пуст. Добавьте функции в get_system_functions().\n";
                return 1;
            }

            std::cout << std::format(
                "\nОбнаружено уравнений: {}.\nТребуется начальное приближение x0 длины {}.\n",
                n,
                n);
            auto x = read_vector<T>(n, std::format("Введите x0 ({} чисел через пробел): ", n));

            miv::matrix<T> J_manual;
            if (jacobian_mode == JacobianMode::Manual)
            {
                J_manual = read_matrix<T>(n, "Введите матрицу Якоби (n x n) построчно:");
                std::cout << std::format("\n{}\n", "Введённая матрица Якоби:");
                print_matrix(J_manual);
            }

            miv::matrix<T> J_frozen;
            if (method == Method::ModifiedNewton)
            {
                if (jacobian_mode == JacobianMode::Numeric)
                {
                    J_frozen = (numeric_formula == NumericFormula::TwoPoint)
                        ? build_jacobian_two_point(x, functions)
                        : build_jacobian_three_point(x, functions);
                }
                else
                {
                    J_frozen = J_manual;
                }
            }

            bool converged = false;
            std::size_t iter_done = 0;

            for (std::size_t k = 0; k < max_iter; ++k)
            {
                iter_done = k + 1;
                auto fx = compute_F(x, functions);
                const auto fx_norm = miv::math::norm_l2(fx);

                if (fx_norm < static_cast<miv::math::norm_t>(eps_F))
                {
                    converged = true;
                    std::cout << std::format("\n{}\n", "Критерий ||F|| < eps_F выполнен.");
                    break;
                }

                miv::matrix<T> J;
                if (method == Method::Newton)
                {
                    if (jacobian_mode == JacobianMode::Numeric)
                    {
                        J = (numeric_formula == NumericFormula::TwoPoint)
                            ? build_jacobian_two_point(x, functions)
                            : build_jacobian_three_point(x, functions);
                    }
                    else
                    {
                        J = J_manual;
                    }
                }
                else
                {
                    J = J_frozen;
                }

                miv::array<T> step = solve_step(J, fx);
                const auto step_norm = miv::math::norm_l2(to_column_matrix(step));
                const auto x_norm = miv::math::norm_l2(to_column_matrix(x));

                print_iteration_log(k, x, fx, step, lambda, damping_enabled);

                const auto step_threshold =
                    static_cast<miv::math::norm_t>(eps_x) * (static_cast<miv::math::norm_t>(1) + x_norm);
                if (step_norm < step_threshold)
                {
                    converged = true;
                    std::cout << std::format("\n{}\n", "Критерий ||s|| < eps_x * (1 + ||x||) выполнен.");
                    break;
                }

                for (std::size_t i = 0; i < n; ++i)
                {
                    x[i] += lambda * step[i];
                }
            }

            auto fx_final = compute_F(x, functions);
            const auto fx_final_norm = miv::math::norm_l2(fx_final);

            std::cout << std::format("\n{:=^70}\n", " Итог ");
            std::cout << std::format("{:<24}{}\n", "Статус:", converged ? "сходимость достигнута" : "не сошлось");
            std::cout << std::format("{:<24}{}\n", "Итоговый x*:", format_array(x));
            std::cout << std::format("{:<24}{}\n", "||F(x*)||:", fx_final_norm);
            std::cout << std::format("{:<24}{}\n", "Итераций выполнено:", iter_done);
            std::cout << std::format(
                "{:<24}{}\n",
                "Метод:",
                (method == Method::Newton) ? "Ньютон" : "Модифицированный Ньютон");
            std::cout << std::format(
                "{:<24}{}\n",
                "Якобиан:",
                (jacobian_mode == JacobianMode::Numeric) ? "численный" : "ручной");
            if (jacobian_mode == JacobianMode::Numeric)
            {
                std::cout << std::format(
                    "{:<24}{}\n",
                    "Формула:",
                    (numeric_formula == NumericFormula::TwoPoint) ? "двухузловая" : "трёхузловая");
            }
            if (damping_enabled)
            {
                std::cout << std::format("{:<24}{}\n", "λ:", lambda);
            }
            std::cout << std::format("{:=^70}\n\n", "");
        }
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Ошибка: " << ex.what() << "\n";
        return 1;
    }
}
