#include "graphs/acyclic_lp.h"

namespace graphs
{
	acyclic_LP::acyclic_LP(const edge_weighted_digraph& g, const uint& s)
	{
		uint V = g.V();
		_initialize(V);
		
		topological top(g);
		path order = top.order();
		
		_distTo[s] = 0.0;
		path_citer begin, end, v;
		order.get_citers(begin, end);
		for(v = begin; v != end; v++)
			relax(g, *v);
	}
	
	void acyclic_LP::_initialize(const uint& V)
	{
		if(_distTo != nullptr)
			delete[] _distTo;
		if(_edgeTo != nullptr)
			delete[] _edgeTo;
		
		_distTo = new double[V];
		_edgeTo = new edge[V];
		
		for(int i = 0; i < V; i++)
			_distTo[i] = neginf;
	}
	
	void acyclic_LP::operator()(const edge_weighted_digraph& g, const uint& s)
	{
		uint V = g.V();
		_initialize(V);
		
		topological top(g);
		path order = top.order();
		
		_distTo[s] = 0.0;
		path_citer begin, end, v;
		order.get_citers(begin, end);
		for(v = begin; v != end; v++)
			relax(g, *v);
	}
	
	void acyclic_LP::relax(const edge_weighted_digraph& g, const uint& v)
	{
		// Negate the weights and reverse the comparison operator.
		adje_citer begin, end, E;
		g.adj(v, begin, end);
		for(E = begin; E != end; E++)
		{
			edge const& e = *E;
			int w = e.to();
			if (_distTo[w] < _distTo[v] + e.weight)
			{
				_distTo[w] = _distTo[v] + e.weight;
				_edgeTo[w] = e;
			}
		}
	}
	
	epath acyclic_LP::getPathTo(const uint& v) const
	{
		if(hasPathTo(v) == false)
			return epath();
		
		epath path;
		for(edge e = _edgeTo[v]; e.valid() == true; e = _edgeTo[e.from()])
			path.add(e);
		
		return path;
	}
}
