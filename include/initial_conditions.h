#ifndef INITIAL_CONDITIONS_H
#define INITIAL_CONDITIONS_H

#include <random>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <math.h>

#include "ode_solver.h"

/*
 * This class is used to generate random initial conditions for the bound Keplerian problem.
 * 
 * We assume that G = 4pi^2 [au^3/year^2/1 solar mass].  This can be modified outside the client
 * (see the setGM() member function), but for consistency, all other parameters will have to be
 * modified.  Users can change the aphelion range, along with the range of the orbiting mass, as
 * well as the number of requested orbital cycles, which defaults to one.
 * 
 * Algorithm:
 * We generate a random aphelion, xinit, which also determines the tangential position yinit, the radial
 * velocity vyinit, as well as the potential energy.  We then generate a random total mechanical energy,
 * in units of the potential energy, arbitrarily ranging from 25% to 75%.  This ensures that we will have
 * a bound system.  From it, we also obtain the final component of velocity, vyinit.
 * 
 * Lastly, we calculate the multiple of the orbital period, which defaults to 3.0, and can be set setGM().
 * 
*/

constexpr static const int orbitalParameters = 14;
using diag_orbit = ode_solver::state<orbitalParameters>;

class initial_conditions
{
	using urd = std::uniform_real_distribution<double>;
public:
	enum orbit : char {x=0, y, vx, vy, r, θ, v, h, pe, ke, a, e, τ, se};
	initial_conditions() { _initialize_rng(); _diag = new diag_orbit; }
	initial_conditions(double, double, double, double, double, double cycles=1.0);
	~initial_conditions() { delete _prd; delete _diag; }
	
	initial_conditions(const initial_conditions&) = delete;
	initial_conditions(initial_conditions&&) = delete;
	initial_conditions& operator=(const initial_conditions&) = delete;
	initial_conditions& operator=(initial_conditions&&) = delete;
	
	void operator()(ode_solver::state<4>& state, double&, double&);
	
	double getMinMass() const { return _min_mass; }
	double getMaxMass() const { return _max_mass; }
	double getMinAphelion() const { return _min_aphelion; }
	double getMaxAphelion() const { return _max_aphelion; }
	double getCycles() const { return _cycles; }
	double getGM() const { return _GM; }
	
	void setMinMass(double min_mass) { _min_mass = min_mass; _mass_range = urd(_min_mass, _max_mass);  }
	void setMaxMass(double max_mass) { _max_mass = max_mass; _mass_range = urd(_min_mass, _max_mass);  }
	void setMinAphelion(double min_aphelion) { _min_aphelion = min_aphelion; _radial_range = urd(_min_aphelion, _max_aphelion);  }
	void setMaxAphelion(double max_aphelion) { _max_aphelion = max_aphelion; _radial_range = urd(_min_aphelion, _max_aphelion);  }
	void setCycles(double cycles) { _cycles = cycles; }
	void setGM(double GM) { _GM = GM; }
	
	void getOrbitalParameters(const ode_solver::state<4>&, diag_orbit&, double) const;
	
	std::string print(const ode_solver::state<4>&, double) const;
	
private:
	
	void _initialize_rng();
	
	std::random_device* _prd = nullptr;				// cat /proc/cpuinfo | grep rdrand.  Declaring a pointer to it as there may not be support for it.
	std::default_random_engine generator;
	std::uniform_real_distribution<double> _radial_range;
	std::uniform_real_distribution<double> _mass_range;
	std::uniform_real_distribution<double> _total_E_range;
	
	constexpr static const double twopi = 2*ode_solver::pi;
	constexpr static const double max_eccentricity = 0.9;
	
	diag_orbit* _diag = nullptr;
	double _min_aphelion = 0.4;						// ~Mercury, in au.
	double _max_aphelion = 50.0;					// ~Pluto, in au.
	double _min_mass = 0.01;						// In solar mass units.
	double _max_mass = 0.25;						// In solar mass units.
	double _cycles = 1.0;							// How many orbits requested.
	double _GM = 4*ode_solver::pi*ode_solver::pi;	// G times the solar mass, [au^3/year^2].
};

#endif 
