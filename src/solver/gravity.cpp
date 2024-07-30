#include "gravity.h"

namespace orbital
{
	void gravity::gravity_f(const ode_solver::state<4>& r, ode_solver::state<4>& dr, const double& t)
	{
		/*
		 * Time independent gravity function for heavily asymmetric "one" body problems, that
		 * is two body problems using center of mass coordinate system, with origin coincident
		 * with the asmymmetrically heavier mass.  Calculate state dr from r != r(t).  This
		 * function assumes that r = {x, y, vx, vy} and that dr = {vx, vy, ax, ay}.
		*/
		
		double x = r._s[0];
		double y = r._s[1];
		double vx = r._s[2];
		double vy = r._s[3];
  
		double distance = sqrt(x*x + y*y);
		double ax = -_GM*x/(distance*distance*distance);
		double ay = -_GM*y/(distance*distance*distance);
  
		dr._s[0] = vx;
		dr._s[1] = vy;
		dr._s[2] = ax;
		dr._s[3] = ay;
		
	}
}
