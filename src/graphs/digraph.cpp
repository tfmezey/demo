#include "digraph.h"

namespace graphs
{
	void digraph::addEdge(const uint& v, const uint& w)
	{
		// Dynamically adjust the size of the _adj array of stacks as needed.
		int max = v < w ? w : v;
		if(max > _adj->size())
			_adj->reserve(max);

		_size = _adj->size();
		if(_V <= max)
			_V = max + 1;
		
		_adj->get(v).add(w);
		_E++;
	}
	
	digraph digraph::reverse() const
	{
		digraph reversedG;
		
		adj_citer begin, end;
		for(int v = 0; v < _V; v++)
		{
			adj(v, begin, end);
			for(adj_citer w = begin; w != end; w++)
				reversedG.addEdge(*w, v);		// Reverse the v->w edge.
		}
		
		return reversedG;
	}
}
