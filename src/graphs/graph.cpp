#include "graph.h"

namespace graphs
{
	void graph::addEdge(const uint& v, const uint& w)
	{
		// Dynamically adjust the size of the _adj array of stacks as needed.
		int max = v < w ? w : v;
		if(max > _adj->size())
			_adj->reserve(max);
		
		_size = _adj->size();
		
		if(_V <= max)
			_V = max + 1;
		
		_adj->get(v).add(w);
		_adj->get(w).add(v);
		
		_E++;
	}
}
