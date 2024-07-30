#ifndef ODE_SOLVER_H
#define ODE_SOLVER_H

#include "solver/solver_functor_base.h"
#include "solver/rk4solver.h"

namespace ode_solver
{
	template <unsigned int N>
	struct state;
	
	template <unsigned int N>
	class base_f;
	
	template <unsigned int N>
	struct rk4solver;
	
}

#endif
