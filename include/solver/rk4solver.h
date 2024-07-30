#ifndef RK4_SOLVER_H
#define RK4_SOLVER_H

#include "solver_functor_base.h"

namespace ode_solver_details
{
	struct assembly_functions
	{
		/* 
		* Due to C++ name mangling of assembly language written member functions of templated classes,
		* countermanding any and all asm() directives that override mangling (as in our case here),
		* we needed to move our assembly functions into this non-templated container class, just to
		* be able to link to them.
		* 
		* This class must not be used by client code!
		* 
		* initialize():
		* rk4solver<N>'s constructor must call initialize() with its own this pointer, the pointer to the passed
		* ode_solver::base_f<N> derived functor, and the static array of offsets:  rk4solver<N>::offsets.
		* The two class pointers are cast to void*, to allow our assembly functions to make use of them.
		* Lastly, making this function static allows us to pass only three pointers instead of four.
		* 
		* Once the necessary pointers are set, rk4_asm() can safely be called. Check results via asmReady(),
		* which can be used to ensure that intialize() was indeed called.
		*/
		
		bool asmReady() const asm("solver_ready");
		void rk4_asm(double* X, double t, double h) asm("solver_rk4_asm");
		static void initialize(void* parent, void* functor, const unsigned int*) asm("solver_initialize");
	};
}

namespace ode_solver
{
	template <unsigned int N>
	struct rk4solver : public ode_solver_details::assembly_functions
	{
		/*
		 * This is our templated rk4 solver class.  Templating to an unsigned integer ensures that
		 * functors use the same sized state array as we have here.
		 * 
		 * Note that we make use of stack allocation and thus we restrict N to no more than 50.
		 * Internally, we will need 5 arrays of N doubles each, or 2000 bytes maximum.  Adjust
		 * MIN_N and MAX_N as desired.  See solver_functor_base.h.
		 * 
		*/
		
	public:
		
		static_assert(N>=MIN_N and N<=MAX_N, "ode_solver::state<N>:  N is outside the allowed range.");
		rk4solver() = delete;
		rk4solver(const rk4solver&) = delete;
        rk4solver(rk4solver&&) = delete;
        rk4solver& operator=(const rk4solver&) = delete;
        rk4solver& operator=(rk4solver&&) = delete;
		
		rk4solver(ode_solver::base_f<N>& f, bool useASM=false);
		~rk4solver() {};
		
		void operator()(ode_solver::state<N>& Y, double t, double dt) { (this->*_prk4)(Y, t, dt); };
		void setFunctor(ode_solver::base_f<N>& f) { _f = &f; initialize((void*)this, (void*)&f, offsets); }
		void setRK4Variant(bool useASM=false);
		bool ready() const { return asmReady(); }
		
	private:
		
		void rk4(ode_solver::state<N>&, double t, double dt);

		ode_solver::state<N> _K1;
		ode_solver::state<N> _K2;
		ode_solver::state<N> _K3;
		ode_solver::state<N> _K4;
		ode_solver::state<N> _yTemp;
		
		ode_solver::base_f<N>* _f = nullptr;
		const unsigned int _N = N;
		using rk4type = void (rk4solver<N>::*)(ode_solver::state<N>&, double, double);
		rk4type _prk4 = nullptr;
		
		/* Calculate the offsets for the assembly functions, as the offsets change with each value of N.
		 * _K1 is at offset 0
		 * _K2 is at _K1 + N*sizeof(double) = N*sizeof(double)
		 * _K3 is at _K2 + N*sizeof(double) = 2*N*sizeof(double)
		 * _K4 is at _K3 + N*sizeof(double) = 3*N*sizeof(double)
		 * _yTemp is at _K4 + N*sizeof(double) = 4*N*sizeof(double)
		 * _f is at end of _yTemp = N*sizeof(double) + N*sizeof(double) = 5*N*sizeof(double)
		 * _N is at _f + 8 = 5*N*sizeof(double) + 8
		 * 
		 * Note that the order of declaring the members of this class matters.  Don't change it!
		*/
		
		constexpr static unsigned int offsets[7] = {
			0,								// rk4solver<N>::_K1
			N*sizeof(double), 				// rk4solver<N>::_K2
			2*N*sizeof(double),				// rk4solver<N>::_K3
			3*N*sizeof(double), 			// rk4solver<N>::_K4
			4*N*sizeof(double),				// rk4solver<N>::_yTemp
			5*N*sizeof(double),				// rk4solver<N>::_f
			8 + 5*N*sizeof(double)			// rk4solver<N>::_N
		};
	};
	
	template <unsigned int N>
	rk4solver<N>::rk4solver(ode_solver::base_f<N>& f, bool useASM)
	{
		if(useASM == true)
			_prk4 = (rk4type)&ode_solver_details::assembly_functions::rk4_asm;
		else
			_prk4 = &rk4solver<N>::rk4;
		
		_f = &f;
		initialize((void*)this, (void*)&f, offsets);
	}
	
	template <unsigned int N>
	void rk4solver<N>::setRK4Variant(bool useASM)
	{
		// Switch between the implicit and default C++ rk4() and rk4_asm().
		if(useASM == true)
			_prk4 = (rk4type)&ode_solver_details::assembly_functions::rk4_asm;
		else
			_prk4 = &rk4solver<N>::rk4;
	}
	
	template <unsigned int N>
	void rk4solver<N>::rk4(ode_solver::state<N>& Y, double t, double dt)
	{
		/* C++ variant of Runga Kutta Order Four.
		 * Initial value problem:  dy/dt = f(t, y), t in [a, b], y(a) = α.
		 * 
		 * Y_0 = α
		 * K1 = dt*f(t_i, Y_i)
		 * K2 = dt*f(t_i + 0.5*dt, Y_i + 0.5*K1) = dt*f(t_i + 0.5*dt, yTemp_i)
		 * K3 = dt*f(t_i + 0.5*dt, Y_i + 0.5*K2) = dt*f(t_i + 0.5*dt, yTemp_i)
		 * K4 = dt*f(t_i + dt, Y_i + K3) = dt*f(t_i + dt, yTemp_i)
		 * Y_{i+1} = Y_i + (K1 + 2*K2 + 2*K3 + K4)/6
		 * 
		 * We generalize Y to be a column vector of N dimensions.
		*/
		
		(*_f)(Y, _K1, t);										// Calculate K1/dt.
		
		for(int i = 0; i < N; i++)								// Solve for K1, and calculate Y_i + 0.5*K1.
		{ _K1[i] *= dt; _yTemp[i] = Y[i] + 0.5*_K1[i]; }
		(*_f)(_yTemp, _K2, t + 0.5*dt);							// Calculate K2/dt.
		
		for(int i = 0; i < N; i++)								// Solve for K2, and calculate Y_i + 0.5*K2.
		{_K2[i] *= dt; _yTemp[i] = Y[i] + 0.5*_K2[i]; }					
		(*_f)(_yTemp, _K3, t + 0.5*dt);							// Calculate K3/dt.
		
		for(int i = 0; i < N; i++)								// Solve for K3, and calculate Y_i + K3.
		{ _K3[i] *= dt; _yTemp[i] = Y[i] + _K3[i]; }
		(*_f)(_yTemp, _K4, t + dt);								// Calculate K4/dt.
		
		for(int i = 0; i < N; i++)								// Solve for K4 and calculate Y_{i+1}.
		{
			_K4[i] *= dt;
			Y[i] = Y[i] + (_K1[i] + 2.0*_K2[i] + 2.0*_K3[i] + _K4[i])/6;
		}
	}
}

#endif
