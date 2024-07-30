#ifndef GRAVITY_H
#define GRAVITY_H

#include <math.h>

#include "solver_functor_base.h"

namespace orbital
{
	// Time independent gravity functor class for heavily asymmetric "one" body problems, ie N == 4.
	
	class gravity : public ode_solver::base_f<4>
	{
		using FP = base_f<4>::FP;
		
	public:
		gravity(const gravity&) = delete;
		gravity(gravity&&) = delete;
		gravity& operator=(const gravity&) = delete;
		gravity& operator=(gravity&&) = delete;

		gravity() : base_f<4>((FP)&gravity::gravity_f) {}		// Pass and cast the pointer to our private member function to base_f<N>::FP type.
		gravity(const double& GM) : base_f<4>((FP)&gravity::gravity_f) { _GM = GM; }
		~gravity() {};
		
		inline double getGM() const { return _GM; }
		inline void setGM(const double& GM) { _GM = GM; }
		
	private:
		void gravity_f(const ode_solver::state<4>& r, ode_solver::state<4>& dr, const double& t);
		double _GM = 4*ode_solver::pi*ode_solver::pi; 			// [GM] = au^3/yr^2
	};
	
}

#endif 
