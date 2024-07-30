#include <iostream>
#include <fstream>
#include <string>

#include "utilities.h"

#include "initial_conditions.h"
#include "rk4solver.h"
#include "rkfsolver.h"
#include "gravity.h"

using namespace std;

int test_rk4_solver(const long trials, bool saveLastTrial=false)
{
	// Solver demo:
	using namespace orbital;
	using namespace utilities;
	cout << endl << "Orbit demo with " << trials << " trials:" << endl;
	
	/*
	 * The unit of the graviational constant, G, in SI units is m^3/kg/s^2, having a value of 6.67430e-11.
	 * 1 astronomical unit = 149,597,870,700 m = x meters.
	 * 1 year = 365.24*24*3600 = 3.1556736e7 s = y seconds.
	 * Solar mass = 1.9884e30 kg = z kilograms.
	 * 
	 * We transform G to units of au^3/year^2/1 solar mass via dimensional analysis:
	 * G[SI] * (au/x meters)^3*(y seconds/1 year)^2*(z kg/1 solar mass) ~ 39.474 => ~4 pi^2 au^3/year^2/1 solar mass.
	 * 
	*/
	
	enum rk4_state : char { x=0, y, vx, vy };
	enum state_index : char { r=0, θ, t, ke, pe };
	enum rk4_variant : bool { useCPP, useASM };
	const int N = 4;										// Planar motion => 4 ode's.
	ode_solver::state<N> state;								// State vector needed by rk4.
			
	orbital::gravity g;
	ode_solver::rk4solver<N> s(g, useCPP);
	if(s.ready() == false)
	{
		cerr << "The solver is not ready.  Returning." << endl;
		return -1;
	}
	
	double GM = g.getGM();									// Solar mass times G:  4pi^2 [au^3/year^2].
	double mass = 0.0;
	double Tinit = 0.0;
	double Tfinal = 0.0;
	initial_conditions ic;									// Default G is already 4pi^2 [au^3/year^2].
	ic.setCycles(3.0);
	ic(state, mass, Tfinal);
	double dt = Tfinal / 365.24;
	
	// Array for the state vector.
	using state_ar = containers::array<ode_solver::state<5>>;
	using citer = state_ar::citerator;
	state_ar orbit(2000);
	ode_solver::state<5> point;								// The state vector consisting of position in polar coordinates, time, KE and PE.
	
	int trialCount = 0;
	int trial_duration = 0;
	int cppRun = 0;
	int asmRun = 0;
	
	{
		utilities::timer Timer1("C++ rk4");
		utilities::silent_timer Timer2(trial_duration, silent_timer::accumulating);
		for(trialCount = 0; trialCount < trials; trialCount++)
		{
			for(double T = Tinit; T <= Tfinal; T += dt) 
			{
				point[t] = T;
				point[r] = sqrt(state[x]*state[x] + state[y]*state[y]);
				point[θ] = atan2(state[y], state[x]);
				point[ke] = 0.5*mass*(state[vx]*state[vx] + state[vy]*state[vy]);
				point[pe] = -GM*mass/point[r];

				orbit.add(point);

				s(state, T, dt);							// Advance the state to T + dt.
			}
			
			orbit.clear();
			ic(state, mass, Tfinal);
		}
	}
		
	cppRun = trial_duration;
	cout << "Completed " << trialCount << " rk4 C++ trials, in " << trial_duration;
	cout << "ms.  Average trial time duration = " << (double)(trial_duration)/trialCount << "ms." << endl;
	
	s.setRK4Variant(useASM);
	trial_duration = 0;
	
	{
		utilities::timer Timer1("Assembly rk4");
		utilities::silent_timer Timer2(trial_duration, silent_timer::accumulating);
		for(trialCount = 0; trialCount < trials; trialCount++)
		{
			orbit.clear();
			ic(state, mass, Tfinal);
			if(trialCount == trials-1)
				cout << "Final set of initial conditions:" << endl << ic.print(state, mass) << endl;
			
			for(double T = Tinit; T <= Tfinal; T += dt) 
			{
				point[t] = T;
				point[r] = sqrt(state[x]*state[x] + state[y]*state[y]);
				point[θ] = atan2(state[y], state[x]);
				point[ke] = 0.5*mass*(state[vx]*state[vx] + state[vy]*state[vy]);
				point[pe] = -GM*mass/point[r];

				orbit.add(point);

				s(state, T, dt);								// Advance the state to T + dt.
			}
		}
	}
	
	asmRun = trial_duration;
	cout << "Completed " << trialCount << " rk4 assembly trials, in " << trial_duration;
	cout << "ms.  Average trial time duration = " << (double)(trial_duration)/trialCount << "ms." << endl;
	
	if(cppRun <= asmRun)
		cout << "cppRun is " << setfill('0') << setprecision(2) << ((double)(asmRun - cppRun)/cppRun)*100 << "% faster than asmRun." << endl;
	else
		cout << "asmRun is " << setfill('0') << setprecision(2) << ((double)(cppRun - asmRun)/asmRun)*100 << "% faster than cppRun." << endl;
	
	if(saveLastTrial)
	{
	
		// Document our efforts to evaluate with octave.
		ofstream rplot("rplot.txt", std::ios_base::trunc);
		ofstream θplot("thetaplot.txt", std::ios_base::trunc);
		ofstream tplot("time.txt", std::ios_base::trunc);
		ofstream keplot("kinetic.txt", std::ios_base::trunc);
		ofstream peplot("potential.txt", std::ios_base::trunc);
		
		citer begin, end;
		orbit.get_citers(begin, end);
		double min_E = orbit[0][ke] + orbit[0][pe];
		double max_E = min_E;
		for(citer s = begin; s != end; s++)
		{
			rplot << (*s)[r] << endl;
			θplot <<  (*s)[θ] << endl;
			tplot << (*s)[t] << endl;
			keplot << (*s)[ke] << endl;
			peplot << (*s)[pe] << endl;
			
			double E = (*s)[ke] + (*s)[pe];
			if(min_E > E)
				min_E = E;
			if(max_E < E)
				max_E = E;
		}
		
		double spread = max_E - min_E;
		cout << std::setprecision(8) << "E_min = " << min_E << ", E_max = " << max_E << ", spread = " << spread << ", which is ";
		cout << std::setprecision(4);
		cout << abs(100.0*spread/min_E) << "% of the minimum, and " << abs(100.0*spread/max_E) << "% of the maximum." << endl;
		
		rplot.close();
		θplot.close();
		tplot.close();
		keplot.close();
		peplot.close();
	}
	
	return 0;
}

int test_rkf_solver(const long trials, bool saveLastTrial=false)
{
	using namespace std;
	
	/*
	 * The unit of the graviational constant, G, in SI units is m^3/kg/s^2, having a value of 6.67430e-11.
	 * 1 astronomical unit = 149,597,870,700 m = x meters.
	 * 1 year = 365.24*24*3600 = 3.1556736e7 s = y seconds.
	 * Solar mass = 1.9884e30 kg = z kilograms.
	 * 
	 * We transform G to units of au^3/year^2/1 solar mass via dimensional analysis:
	 * G[SI] * (au/x meters)^3*(y seconds/1 year)^2*(z kg/1 solar mass) ~ 39.474 => ~4 pi^2 au^3/year^2/1 solar mass.
	 * 
	*/
	
	// Solver demo:
	double xinit = 1.0;				//au
	double yinit = 1.1;				//au
	double vxinit = ode_solver::pi;	//au/yr
	double vyinit = 0.0;			//au/yr
	double Tinit = 0.0;				//yr
	double Tfinal = 2.75;			//yr
	double mass = 0.025;			// in solar mass units.

	double dt = 1.0/365;			//yr
	constexpr const int N = 4;
	ode_solver::state<N> state;		// State vector needed by rk4.
	
	enum rkf_state : char { x=0, y, vx, vy };
	state[x] = xinit;
	state[y] = yinit;
	state[vx] = vxinit;
	state[vy] = vyinit;

	bool useASM = false;
	orbital::gravity g;
	ode_solver::rkfsolver<N> s(g, useASM);
	if(s.ready() == false)
	{
		cerr << "solver is not ready.  Returning." << endl;
		return -1;
	}
	
	// Array for the state vector.
	using namespace orbital;
	using state_ar = containers::array<ode_solver::state<5>>;
	using citer = state_ar::citerator;
	enum state_index : char { r=0, θ, t, ke, pe };
	state_ar orbit(400);
	ode_solver::state<5> point;				// This is the state vector consisting of position in polar coordinates, time, KE and PE.
	
	int trialCount = 0;
	int trial_duration = 0;
	int cppRun = 0;
	int asmRun = 0;
	using namespace utilities;
	
	{
		utilities::timer Timer1("C++ rkf");
		utilities::silent_timer Timer2(trial_duration, silent_timer::accumulating);
		for(trialCount = 0; trialCount < trials; trialCount++)
		{
			for(double T = Tinit; T <= Tfinal; T += dt) 
			{
				point[t] = T;
				point[r] = sqrt(state[x]*state[x] + state[y]*state[y]);
				point[θ] = atan2(state[y], state[x]);
				point[ke] = 0.5*mass*(state[vx]*state[vx] + state[vy]*state[vy]);
				point[pe] = -g.getGM()*mass/point[r];

				orbit.add(point);

				s(state, T, dt);			// Advance the state to T + dt.
			}

			orbit.clear();
			state[x] = xinit;
			state[y] = yinit;
			state[vx] = vxinit;
			state[vy] = vyinit;
		}
	}
	
	cppRun = trial_duration;
	cout << "Completed " << trialCount << " rkf C++ trials, in " << trial_duration;
	cout << "ms.  Average trial time duration = " << (double)(trial_duration)/trialCount << "ms." << endl;
	orbit.clear();
	
	s.setRKFVariant(true);
	trial_duration = 0;
	state[x] = xinit;
	state[y] = yinit;
	state[vx] = vxinit;
	state[vy] = vyinit;
	
	{
		utilities::timer Timer1("Assembly rkf");
		utilities::silent_timer Timer2(trial_duration, silent_timer::accumulating);
		for(trialCount; trialCount< trials; trialCount++)
		{
			for(double T = Tinit; T <= Tfinal; T += dt) 
			{
				point[t] = T;
				point[r] = sqrt(state[x]*state[x] + state[y]*state[y]);
				point[θ] = atan2(state[y], state[x]);
				point[ke] = 0.5*mass*(state[vx]*state[vx] + state[vy]*state[vy]);
				point[pe] = -g.getGM()*mass/point[r];

				orbit.add(point);

				s(state, T, dt);			// Advance the state to T + dt.
			}
			
			if(trialCount != trials - 1)
				orbit.clear();
			
			state[x] = xinit;
			state[y] = yinit;
			state[vx] = vxinit;
			state[vy] = vyinit;
		}
	}
	
	asmRun = trial_duration;
	cout << "Completed " << trialCount << " rkf assembly trials, in " << trial_duration;
	cout << "ms.  Average trial time duration = " << (double)(trial_duration)/trialCount << "ms." << endl;
	
	if(cppRun <= asmRun)
		cout << "cppRun is " << (double)(asmRun)/cppRun*100 << "% faster than asmRun." << endl;
	else
		cout << "asmRun is " << (double)(cppRun)/asmRun*100 << "% faster than cppRun." << endl;
	
	if(saveLastTrial)
	{
		// Document our efforts to evaluate with octave.
		ofstream rplot("rplot.txt", std::ios_base::trunc);
		ofstream θplot("thetaplot.txt", std::ios_base::trunc);
		ofstream tplot("time.txt", std::ios_base::trunc);
		ofstream keplot("kinetic.txt", std::ios_base::trunc);
		ofstream peplot("potential.txt", std::ios_base::trunc);
		
		citer begin, end;
		orbit.get_citers(begin, end);
		for(citer s = begin; s != end; s++)
		{
			rplot << (*s)[r] << endl;
			θplot <<  (*s)[θ] << endl;
			tplot << (*s)[t] << endl;
			keplot << (*s)[ke] << endl;
			peplot << (*s)[pe] << endl;
		}
		
		rplot.close();
		θplot.close();
		tplot.close();
		keplot.close();
		peplot.close();
	}

	return 0;
}
