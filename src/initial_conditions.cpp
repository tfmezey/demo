#include "initial_conditions.h"

initial_conditions::initial_conditions(double min_ap, double max_ap, double min_mass, double max_mass, double GM, double cycles)
{
	_min_aphelion = min_ap;
	_max_aphelion = max_ap;
	_min_mass = min_mass;
	_max_mass = max_mass;
	_cycles = cycles;
	_GM = GM;
	
	_initialize_rng();
	_diag = new diag_orbit;
}

void initial_conditions::_initialize_rng()
{
	try
	{
		_prd = new std::random_device;									// Create the hardware random device, ...
		generator = std::default_random_engine((*_prd)());				// and use it as the seed for the DRE.
	}
	catch(const std::exception& e)
	{
		// Alternatively, if there is no hardware support for RNG, use a pRNG with a time based seed instead.
		unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
		generator = std::default_random_engine(seed);
	}
		
	_radial_range = urd(_min_aphelion, _max_aphelion);					// Approximate [Mercury, Pluto] aphelion distances.
	_mass_range = urd(_min_mass, _max_mass);							// No more than 0.25 solar masses.
	_total_E_range = urd(0.25, 0.75);									// We will restrict the total energy to [0.25, 0.75] of the potential energy.
}

void initial_conditions::operator()(ode_solver::state<4>& state, double& mass, double& Tfinal)
{
	/*
	 * We first randomly generate the mass, in the range [_min_mass, _max_mass], having solar mass units.
	 * 
	 * We then generate the aphelion [au] in the range [_min_aphelion, _max_aphelion], which is a point along
	 * the x-axis.  This also gives us the potential energy at aphelion [au^2/year^2*1 solar mass].
	 * 
	 * With the potential energy, we chose the total mechanical energy to be between 25% and 75% of the
	 * potential energy.  This then leads to the kinetic energy and thus the velocity at aphelion, which is
	 * entirely tangential there.
	 * 
	 * Lastly, using Kepler's 3rd law, we calculate Tfinal to be _cycles times the calculated orbital period.
	 * 
	 * Units displayed in the comments are in the default system of units.
	 * 
	*/
	
	mass = _mass_range(generator);										// In solar mass units.
	state[x] = _radial_range(generator);								// At aphelion, in au.
	state[y] = 0.0;														// At aphelion, y = 0 au, and ...
	state[vx] = 0.0;													// we only have tangential velocity:  0.0 [au/year].
	double PE = -_GM*mass/state[x];										// Potential energy [au^2/year^2*1 solar mass].
	const double E_total = PE*_total_E_range(generator);
	state[vy] = sqrt(2/mass*(E_total - PE));							// Tangential velocity at aphelion [au/year].
	double E_specific = state[vy]*state[vy]/2 - _GM/state[x];			// Specific mechanical energy [au^2/year^2]
	double a = -_GM/2/E_specific;										// The semi-major axis in au.
	Tfinal = _cycles*twopi*sqrt(pow(a, 3.0)/_GM);						// Period = 2pi/sqrt(GM) a^(3/2), years.
}

void initial_conditions::getOrbitalParameters(const ode_solver::state<4>& state, diag_orbit& orbital, double mass) const
{
	orbital[x] = state[x];												// Position in cartesian coordinates, in au.
	orbital[y] = state[y];
	orbital[vx] = state[vx];											// Velocity in cartesian coordinates in au/year.
	orbital[vy] = state[vy];
	orbital[r] = sqrt(state[x]*state[x]+state[y]*state[y]);				// Position in radial coordinates, [r] = au, [θ] = radians/year.
	orbital[θ] = atan2(state[y], state[x]);
	orbital[v] = sqrt(state[vx]*state[vx]+state[vy]*state[vy]);			// Magnitude of velocity in au/year.
	orbital[ke] = 0.5*mass*orbital[v]*orbital[v];						// Kinetic energy [au^2/year^2*1 solar mass].
	orbital[pe] = -_GM*mass/orbital[r];									// Potential energy [au^2/year^2*1 solar mass].
	orbital[se] = orbital[v]*orbital[v]/2 - _GM/orbital[r];				// Specific mechanical energy [au^2/year^2].
	orbital[a] = -_GM/2/orbital[se];									// Semi-major axis [au].
	orbital[h] = state[x]*state[vy] - state[y]*state[vx];				// Magnitude h = r x v, in 2D.
	orbital[e] = sqrt(1 + 2*orbital[se]*orbital[h]*orbital[h]/_GM/_GM);	// The eccentricity.
	orbital[τ] = twopi*sqrt(pow(a, 3.0)/_GM);							// The orbital period.
}

std::string initial_conditions::print(const ode_solver::state<4>& state, double mass) const
{
	using namespace std;
	ostringstream o;
	string indentation = "     ";
	diag_orbit orbital;
	getOrbitalParameters(state, orbital, mass);
	
	o << indentation << "mass = " << std::setprecision(4) << mass << endl;
	o << indentation << "x = " << std::setprecision(4) << state[x] << endl;
	o << indentation << "y = " << std::setprecision(4) << state[y] << endl;
	o << indentation << "v.x = " << std::setprecision(4) << state[vx] << endl;
	o << indentation << "v.y = " << std::setprecision(4) << state[vy] << endl;
	o << indentation << "r = " << std::setprecision(4) << orbital[r] << endl;
	o << indentation << "θ = " << std::setprecision(4) << orbital[θ] << endl;
	o << indentation << "v = " << std::setprecision(4) << orbital[v] << endl;
	o << indentation << "h = " << std::setprecision(4) << orbital[h] << endl;
	o << indentation << "KE = " << std::setprecision(4) << orbital[ke] << endl;
	o << indentation << "PE = " << std::setprecision(4) << orbital[pe] << endl;
	o << indentation << "E_total = " << std::setprecision(4) << orbital[pe] + orbital[ke] << endl;
	o << indentation << "E_specific = " << std::setprecision(4) << orbital[se] << endl;
	o << indentation << "Semi-major axis = " << std::setprecision(4) << orbital[a] << endl;
	o << indentation << "Eccentricity = " << std::setprecision(4) << orbital[e] << endl;
	o << indentation << "Period = " << std::setprecision(4) << orbital[τ] << endl;
	
	return o.str();
}
