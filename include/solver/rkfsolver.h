#ifndef RKF_SOLVER_H
#define RKF_SOLVER_H

#include <math.h>

#include "solver_functor_base.h"

namespace ode_solver_details
{
	struct assembly_functions_rkfsolver
	{
		/*
		 * Runge-Kutta-Fehlberg solver class.
		 * 
		 * This solver estimates the local truncation error between a 5th order RKF method and a fourth
		 * order RK variant, to obtain a time step scaling factor.  If the local truncation error of one
		 * of the dependent variables exceeds the default/user provided tolerance, we loop until we either
		 * obtain a satisfactory time step, or we reach one of the edges of the permitted range of the time
		 * step.
		 * 
		 * Either way, the time step is adjusted to a value within [δt_min, δt_max].  The operator()() uses a 
		 * reference for the time step to propagate the changed time step to client code.
		 * 
		 * Due to C++ name mangling of assembly language written member functions of templated classes,
		 * countermanding any and all asm() directives that override mangling (as in our case here),
		 * we needed to move our assembly functions into this non-templated container class, just to
		 * be able to link to them.
		 * 
		 * This class must not be used by client code!
		 * 
		 * initialize():
		 * rkfsolver<N>'s constructor must call initialize() with its own this pointer, the pointer to the passed
		 * ode_solver::base_f<N> derived functor, and the static array of offsets:  rkfsolver<N>::offsets.
		 * The two class pointers are cast to void*, to allow our assembly functions to make use of them.
		 * Lastly, making this function static allows us to pass only three pointers instead of four.
		 * 
		 * Once the necessary pointers are set, rkf_asm() can safely be called. Check results via asmReady(),
		 * which can be used to ensure that intialize() was indeed called.
		*/
		
		bool asmReady() const asm("solver_rkf_ready");
		void rkf_asm(double* Y, double t, double& δt) asm("solver_rkf_asm");
		static void initialize(void* parent, void* functor, const unsigned int* offsets, const double* constants) asm("solver_rkf_initialize");
		static void updateRestrictions(void* parent, const unsigned int* offsets) asm("solver_rfk_update_restrictions");
	};
}

namespace ode_solver
{
	template <unsigned int N>
	struct rkfsolver : public ode_solver_details::assembly_functions_rkfsolver
	{
		/*
		 * This is our templated rkf solver class.  Templating to an unsigned integer ensures that
		 * functors use the same sized state array as we have here.
		 * 
		 * Note that we make use of stack allocation and thus we restrict N to no more than 50.
		 * Internally, we will need 5 arrays of N doubles each, or 2000 bytes maximum.  Adjust
		 * MIN_N and MAX_N as desired.  See solver_functor_base.h.
		 * 
		*/
		
	public:
		
		static_assert(N>=MIN_N and N<=MAX_N, "ode_solver::state<N>:  N is outside the allowed range.");
		rkfsolver() = delete;
		rkfsolver(const rkfsolver&) = delete;
        rkfsolver(rkfsolver&&) = delete;
        rkfsolver& operator=(const rkfsolver&) = delete;
        rkfsolver& operator=(rkfsolver&&) = delete;
		
		rkfsolver(ode_solver::base_f<N>& f, bool useASM=false);
		rkfsolver(ode_solver::base_f<N>& f, double dtmin, double dtmax, double tol, bool useASM=false);
		~rkfsolver() {};
		
		void operator()(ode_solver::state<N>& Y, double t, double& dt) { (this->*_prkf)(Y, t, dt); };
		void setFunctor(ode_solver::base_f<N>& f) { _f = &f; initialize((void*)this, (void*)&f, offsets, asm_constants); }
		void setRKFVariant(bool useASM=false);
		bool ready() const { return asmReady(); }
		void setRestrictions(double δtmin, double δtmax, double tol);
		void getRestrictions(double& δtmin, double& δtmax, double& tol) const { δtmin = _δt_min; δtmax = _δt_max; tol = _tolerance; }
		
	private:
		
		void rkf(ode_solver::state<N>&, double t, double& dt);

		ode_solver::state<N> _K1;
		ode_solver::state<N> _K2;
		ode_solver::state<N> _K3;
		ode_solver::state<N> _K4;
		ode_solver::state<N> _K5;
		ode_solver::state<N> _K6;
		ode_solver::state<N> _Ytemp;				// This will hold the temporary states needed to obtain the K constants.
		ode_solver::state<N> _R;					// This will hold the local truncation error estimates.
		ode_solver::base_f<N>* _f = nullptr;
		const unsigned int _N = N;
		double _δt_min = 1e-6;						// The minimum value of the step size dt of the independent variable t.
		double _δt_max = 10.0;						// The maximum value of the step size dt of the independent variable t.
		double _tolerance = 1e-5;					// The tolerance value which the dt scaling factor is compared against.
		
		using rkftype = void (rkfsolver<N>::*)(ode_solver::state<N>&, double, double&);
		rkftype _prkf = nullptr;
		int counter = 0;
		
		/* Calculate the offsets for the assembly functions, as the offsets change with each value of N.
		 * _K1 is at offset 0
		 * _K2 is at _K1 + N*sizeof(double) = N*sizeof(double)
		 * _K3 is at _K2 + N*sizeof(double) = 2*N*sizeof(double)
		 * _K4 is at _K3 + N*sizeof(double) = 3*N*sizeof(double)
		 * _K5 is at _K4 + N*sizeof(double) = 4*N*sizeof(double)
		 * _K6 is at _K5 + N*sizeof(double) = 5*N*sizeof(double)
		 * _Ytemp is at _K6 + N*sizeof(double) = 6*N*sizeof(double)
		 * _R is at _Ytemp + N*sizeof(double) = 7*N*sizeof(double)
		 * _f is at end of _R + N*sizeof(double) = 8*N*sizeof(double)
		 * _N is at _f + 8 = 8*N*sizeof(double) + 8
		 * 
		 * Note that the order of declaring the members of this class matters.  Don't change it!
		*/
		
		constexpr static unsigned int offsets[13] = {
			0,								// rkfsolver<N>::_K1
			N*sizeof(double), 				// rkfsolver<N>::_K2
			2*N*sizeof(double),				// rkfsolver<N>::_K3
			3*N*sizeof(double), 			// rkfsolver<N>::_K4
			4*N*sizeof(double), 			// rkfsolver<N>::_K5
			5*N*sizeof(double), 			// rkfsolver<N>::_K6
			6*N*sizeof(double),				// rkfsolver<N>::_Ytemp
			7*N*sizeof(double),				// rkfsolver<N>::_R
			8*N*sizeof(double),				// rkfsolver<N>::_f
			8 + 8*N*sizeof(double),			// rkfsolver<N>::_N
			16 + 8*N*sizeof(double),		// rkfsolver<N>::_δt_min
			24 + 8*N*sizeof(double),		// rkfsolver<N>::_δt_max
			32 + 8*N*sizeof(double)			// rkfsolver<N>::_tolerance
		};
		
		// Coefficients needed for the 5th order local truncation error estimate.
		constexpr static const double K2_1 = 1./4.;
		constexpr static const double K3_1 = 3./32.;
		constexpr static const double K3_2 = 9./32.;
		constexpr static const double K4_1 = 1932.0/2197.0;
		constexpr static const double K4_2 = -7200.0/2197.0;
		constexpr static const double K4_3 = 7296.0/2197.0;
		constexpr static const double K5_1 = 439.0/216.0;
		constexpr static const double K5_2 = -8.0;
		constexpr static const double K5_3 = 3680.0/513.0;
		constexpr static const double K5_4 = -845.0/4104.0;
		constexpr static const double K6_1 = -8.0/27.0;
		constexpr static const double K6_2 = 2.0;
		constexpr static const double K6_3 = -3544.0/2565.0;
		constexpr static const double K6_4 = 1859.0/4104.0;
		constexpr static const double K6_5 = -11.0/40.0;
		
		// Coefficients needed for R.
		constexpr static const double R_1 = 1.0/360.0;
		constexpr static const double R_2 = -128.0/4275.0;
		constexpr static const double R_3 = -2197.0/75240.0;
		constexpr static const double R_4 = 1.0/50.0;
		constexpr static const double R_5 = 2.0/55.0;
		
		// Coefficients for the Runge-Kutta-Fehlberg RK4 variant.
		constexpr static const double RK4_1 = 25.0/216.0;
		constexpr static const double RK4_2 = 1408.0/2565.0;
		constexpr static const double RK4_3 = 2197.0/4104.0;
		constexpr static const double RK4_4 = -1.0/5.0;
		
		// Coefficient needed to calculate the dt scaling factor.
		constexpr static const double q_coeff = pow(2, -0.25);
		
		//12/13 do avoid doing a division.
		constexpr static const double twelve_thirteenths = 12.0/13.0;
		
		// Note that nasm cannot calculate double constants, so we must provide these to rkf_asm() via this constexpr static array.
		constexpr static const double asm_constants[26] = {
			K2_1, K3_1, K3_2, K4_1, K4_2, K4_3, K5_1, K5_2, K5_3, K5_4, \
			K6_1, K6_2, K6_3, K6_4, K6_5, R_1, R_2, R_3, R_4, R_5, \
			RK4_1,RK4_2, RK4_3, RK4_4, q_coeff, twelve_thirteenths
		};
	};
	
	template <unsigned int N>
	rkfsolver<N>::rkfsolver(ode_solver::base_f<N>& f, bool useASM)
	{
		if(useASM == true)
			_prkf = (rkftype)&ode_solver_details::assembly_functions_rkfsolver::rkf_asm;
		else
			_prkf = &rkfsolver<N>::rkf;
		
		_f = &f;
		initialize((void*)this, (void*)&f, offsets, asm_constants);
	}
	
	template <unsigned int N>
	rkfsolver<N>::rkfsolver(ode_solver::base_f<N>& f, double δtmin, double δtmax, double tol, bool useASM)
	{
		if(useASM == true)
			_prkf = (rkftype)&ode_solver_details::assembly_functions_rkfsolver::rkf_asm;
		else
			_prkf = &rkfsolver<N>::rkf;
		
		_f = &f;
		initialize((void*)this, (void*)&f, offsets, asm_constants);
		
		_δt_min = δtmin;
		_δt_max = δtmax;
		_tolerance = tol;
	}
	
	template <unsigned int N>
	void rkfsolver<N>::setRKFVariant(bool useASM)
	{
		// Switch between the implicit and default C++ rkf() and rkf_asm().
		if(useASM == true)
			_prkf = (rkftype)&ode_solver_details::assembly_functions_rkfsolver::rkf_asm;
		else
			_prkf = &rkfsolver<N>::rkf;
	}
	
	template <unsigned int N>
	void rkfsolver<N>::setRestrictions(double δtmin, double δtmax, double tol)
	{
		if(δtmin >= δtmax)
			return;
		
		_δt_min = δtmin; _δt_max = δtmax;
		_tolerance = tol;
		updateRestrictions(this, offsets);
		
	}
	
	template <unsigned int N>
	void rkfsolver<N>::rkf(ode_solver::state<N>& Y, double t, double& δt)
	{
		/*
		 * C++ variant of Runga-Kutta-Fehlberg Method of Order Five.
		 * Initial value problem:  dy/δt = f(t, y), t in [a, b] and y(a) = α.
		 * 
		 * This method uses the difference between a Runga-Kutta 5th order method and a
		 * Runga Kutta 4th order method to estimate the local truncation error, in order
		 * to scale the change in the independent variable adaptively and within tolerance ε.
		 * 
		 * RK5:  φ_{i+1} = φ_i + 16/135*K1 + 6656/12825*K3 + 28561/56430*K4 - 9/50*K5 + 2/55*K6
		 * RK4:  ω_{i+1} = ω_i + 25/216*K1 + 1408/2565*K3 + 2197/4104*K4 - 1/5*K5
		 * 
		 * where,
		 * 		Y_0 = α
		 * 		K1 = δt*f(t_i, Y_i)
		 * 		K2 = δt*f(t_i + 0.25*δt, Y_i + 0.25*K1) = δt*f(t_i + 0.25*δt, _Ytemp_i)
		 * 		K3 = δt*f(t_i + 3/8*δt, Y_i + 3/32*K1 + 9/32*K2) = δt*f(t_i + 3/8*δt, _Ytemp_i)
		 * 		K4 = δt*f(t_i + 12/13*δt, Y_i + 1932/2197*K1 - 7200/2197*K2 + 7296/2197*K3) = δt*f(t_i + 12/13*δt, _Ytemp_i)
		 * 		K5 = δt*f(t_i + δt, Y_i + 439/216*K1 - 8*K2 + 3680/513*K3 - 845/4104*K4) = δt*f(t_i + δt, _Ytemp_i)
		 * 		K5 = δt*f(t_i + 0.5*δt, Y_i - 8/27*K1 + 2*K2 - 3544/2565*K3 + 1859/4104*K4 - 11/40*K5) = δt*f(t_i + 0.5*δt, _Ytemp_i)
		 * 		Y_{i+1} = Y_i + 25/216*K1 + 1408/2565*K3 + 2197/4104*K4 - 1/5*K5
		 * 
		 * and where the δt scaling factor equals:
		 * 		q = (ε*δt/(2*R))^0.25
		 * with local truncation error R being:
		 * 		R = 1/δt*abs(φ_{i+1} - ω_{i+1})
		 * 
		 * We generalize Y to be a column vector of N dimensions.
		*/
		
		bool acceptable = true;
		double q = 1.0;
		
		// Loop until an acceptable δt scale factor is obtained, or when δt falls outside the [_δt_min, _δt_max] range.
		counter = 0;
		do
		{
			counter++;
			acceptable = true;
			
			(*_f)(Y, _K1, t);										// Calculate K1/δt.
			
			for(int i = 0; i < N; i++)								// Solve for K1, along with the new _Ytemp, and ...
			{ _K1[i] *= δt; _Ytemp[i] = Y[i] + K2_1*_K1[i]; }
			(*_f)(_Ytemp, _K2, t + 0.25*δt);						// calculate K2/δt.
			
			for(int i = 0; i < N; i++)								// Solve for K2, along with the new _Ytemp, and ...
			{_K2[i] *= δt; _Ytemp[i] = Y[i] + K3_1*_K1[i] + K3_2*_K2[i]; }					
			(*_f)(_Ytemp, _K3, t + 3.0/8.0*δt);						// calculate K3/δt.
			
			for(int i = 0; i < N; i++)								// Solve for K3, along with the new _Ytemp, and ...
			{ _K3[i] *= δt; _Ytemp[i] = Y[i] + K4_1*_K1[i] + K4_2*_K2[i] + K4_3*_K3[i]; }
			(*_f)(_Ytemp, _K4, t + twelve_thirteenths*δt);			// calculate K4/δt.
			
			for(int i = 0; i < N; i++)								// Solve for K4, along with the new _Ytemp, and ...
			{ _K4[i] *= δt; _Ytemp[i] = Y[i] + K5_1*_K1[i] + K5_2*_K2[i] + K5_3*_K3[i] + K5_4*_K4[i]; }
			(*_f)(_Ytemp, _K5, t + δt);								// calculate K5/δt.
			
			for(int i = 0; i < N; i++)								// Solve for K5, along with the new _Ytemp, and ...
			{ _K5[i] *= δt; _Ytemp[i] = Y[i] + K6_1*_K1[i] + K6_2*_K2[i] + K6_3*_K3[i] + K6_4*_K4[i] + K6_5*_K5[i]; }
			(*_f)(_Ytemp, _K6, t + 0.5*δt);							// calculate K6/δt.  Note that we will leave K6 in this state, until we calculate R[i].
			
			double RR = 0.0;
			for(int i = 0; i < N; i++)								// Calculate the R values, and update RR, square of the norm of R.
			{
				_R[i] = abs(R_1*_K1[i] + R_2*_K3[i] + R_3*_K4[i] + R_4*_K5[i] + δt*R_5*_K6[i])/δt;
				RR += _R[i]*_R[i];
				if(_R[i] > _tolerance)
					acceptable = false;
			}
			
			// Calculate and apply the δt scale factor, and terminate looping if we reach the requested minimum/maximum δt.
			
			// Note that as an efficiency, we defer to take the square root of RR until the calculation of the scaling factor q:
			// 		pow(_tolerance / R_norm, 0.25) == pow(_tolerance*_tolerance / RR, 0.25/2)
			q = q_coeff*pow(_tolerance*_tolerance / RR, 0.125);
			δt = q*δt;
			
			if(δt < _δt_min)
			{
				δt = _δt_min;
				acceptable = true;
				break;
			}
			else if(δt > _δt_max)
			{
				δt = _δt_max;
				acceptable = true;
				break;
			}
		}
		while(acceptable == false);

		// Update the state vector initially at Y_i, to Y_{i+1}.
		for(int i = 0; i < N; i++)
			Y[i] = Y[i] + RK4_1*_K1[i] + RK4_2*_K3[i] + RK4_3*_K4[i] + RK4_4*_K5[i];
	}
}

#endif
