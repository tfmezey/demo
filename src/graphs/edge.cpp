#include "edge.h"

namespace graphs
{
	int edge::compareTo(const edge& that) const
	{
		if(weight < that.weight)
			return -1;
		else if(weight > that.weight)
			return 1;
		else
			return 0;
	}
	
	std::string edge::str() const
	{
		std::ostringstream o;
		
		o << v << "-" << w << " ";
		if(weight == inf)
			o << "inf";
		else if(weight == neginf)
			o << "neginf";
		else
			o << std::fixed << std::setprecision(2) << weight;
		
		return o.str();
	}
}
