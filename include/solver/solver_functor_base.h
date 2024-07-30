#ifndef SOLVER_FUNCTOR_BASE_H
#define SOLVER_FUNCTOR_BASE_H

namespace ode_solver
{
	/*
	 * Base class for functor classes representing initial value problems.
	 * 
	 * The key to our ode_solver namespace classes is the template value N.  It not only ensures
	 * that functors will have a state<N> state vector, but also that they can only be used within
	 * solver classes that can have, via the use of the template, pointers to these functors.
	 * 
	 * The solver<N> is a double array of N elements.  In the base_f<N> class, we define the API to
	 * use by client functors.  Note that we restrict the size of N not only here but also in the
	 * rk4solver<N> class in order to not exhaust stack space.  Adjust these accordingly.
	 * 
	*/
	
	constexpr static const int MIN_N = 2;
	constexpr static const int MAX_N = 50;
	constexpr static const double pi = 3.14159265358979323846;
	constexpr static const double e = 2.71828182845904523536;
	
	template <unsigned int N> class base_f;
	
	template <unsigned int N>
	struct state
	{
		state() { for(int i = 0; i < N; i++) _s[i] = 0.0; };
		state(const state& s) { for(int i = 0; i < N; i++) { _s[i] = s._s[i]; } }
		state& operator=(const state& s) { for(int i = 0; i < N; i++) { _s[i] = s._s[i];} return *this; }
		state(state&&) = delete;
		state& operator=(state&&) = delete;
		static_assert(N>=MIN_N and N<=MAX_N, "ode_solver::state<N>:  N is outside the allowed range.");
		double _s[N];
		const double& operator[](unsigned int i) const { return _s[i]; }		// Getter.
		double& operator[](unsigned int i) { return _s[i]; }					// Setter.
		operator double*() { return _s; }										// Conversion operator.
	};
	
	template <unsigned int N>
	class base_f
	{
		static_assert(N>=MIN_N and N<=MAX_N, "ode_solver::state<N>:  N is outside the allowed range.");
	protected:
		using FP = void (base_f::*)(const state<N>&, state<N>&, const double&);
	public:
		base_f() = delete;
		base_f(const base_f&) = delete;
        base_f(base_f&&) = delete;
        base_f& operator=(const base_f&) = delete;
        base_f& operator=(base_f&&) = delete;
		base_f(FP f) { _f = f; }
		~base_f() {};
		
		/* Prototype of functor:  Given the state r and time t, calculate state dr.  Client derived
		 * classes must provide an apppropriate member callback function, which must be passed to the
		 * base_f<N> constructor.
		*/
		
		void operator()(const state<N>& r, state<N>& dr, const double& t) { (this->*_f)(r, dr, t); };
	protected:
		FP _f = nullptr;
	};
}

#endif
