#ifndef MIV_MATH_LUP_SOLVER_H
#define MIV_MATH_LUP_SOLVER_H

#include "math/equation_system.hpp"

namespace miv::math
{
    /**
     * @brief Совместимость: решение системы через LUP.
     *
     * Теперь основная реализация находится в equation_system.
     */
    template <FloatNumber T>
    inline miv::matrix<T> lup_solve(const equation_system<T> &system)
    {
        return system.solve_lup();
    }
}

#endif // MIV_MATH_LUP_SOLVER_H
