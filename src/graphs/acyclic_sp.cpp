#include "acyclic_sp.h"

namespace graphs
{
	acyclic_SP::acyclic_SP(const edge_weighted_digraph& g, const uint& s)
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
	
	void acyclic_SP::_initialize(const uint& V)
	{
		if(_distTo != nullptr)
			delete[] _distTo;
		if(_edgeTo != nullptr)
			delete[] _edgeTo;
		
		_distTo = new double[V];
		_edgeTo = new edge[V];
		
		for(int i = 0; i < V; i++)
			_distTo[i] = inf;
	}
	
	void acyclic_SP::operator()(const edge_weighted_digraph& g, const uint& s)
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
	
	void acyclic_SP::relax(const edge_weighted_digraph& g, const uint& v)
	{
		/*
		 * The optimality condition states that _distTo[w] <= _distTo[v] + e.weight() for 
		 * shortest paths.  Any encountered violation of this condition, namely that
		 *		_distTo[w] > _distTo[v] + e.weight()
		 * is corrected by this method.  As this applies to all reachable vertices from s, the
		 * _disTo[w] becomes the distance of the shortest path from s to w.
		*/
		
		// Note that to save space, adje is a container of edge pointers.
		adje_citer begin, end, E;
		g.adj(v, begin, end);
		for(E = begin; E != end; E++)
		{
			edge const& e = *E;
			int w = e.to();
			if (_distTo[w] > _distTo[v] + e.weight)
			{
				_distTo[w] = _distTo[v] + e.weight;
				_edgeTo[w] = e;
			}
		}
	}
	
	epath acyclic_SP::getPathTo(const uint& v) const
	{
		if(hasPathTo(v) == false)
			return epath();
		
		epath path;
		for(edge e = _edgeTo[v]; e.valid() == true; e = _edgeTo[e.from()])
			path.add(e);
		
		return path;
	}
}
