#include "directed_cycle.h"

namespace graphs
{
	directed_cycle::directed_cycle(const digraph& dg)
	{
		uint V = dg.V();
		
		_initialize(V);

		for(int v = 0; v < V; v++)
			if((*_marked)[v] == false)
				dfs(dg, v);
	}
	
	directed_cycle::directed_cycle(const edge_weighted_digraph& dg)
	{
		usingEdgeWeightedDigraph = true;

		uint V = dg.V();
		_initialize(V);
		
		for(int v = 0; v < V; v++)
			if((*_marked)[v] == false)
				dfs(dg, v);
	}
	
	void directed_cycle::_initialize(const uint& V)
	{
		if(_marked == nullptr)
			_marked = new containers::array<bool>(V);
		else
		{
			if(_marked->size() != V)
				_marked->reserve(V);
			
			_marked->clear();
		}
			
		if(_edgeTo == nullptr)
			_edgeTo = new containers::array<unsigned int>(V);
		else
		{
			if(_edgeTo->size() != V)
				_edgeTo->reserve(V);
			_edgeTo->clear();
		}
		
		if(_edgeTo_e == nullptr)
			_edgeTo_e = new containers::array<edge>(V);
		else
		{
			if(_edgeTo_e->size() != V)
				_edgeTo_e->reserve(V);
			_edgeTo_e->clear();
		}
		
		if(_onStack == nullptr)
			_onStack = new containers::array<bool>(V);
		else
		{
			if(_onStack->size() != V)
				_onStack->reserve(V);
			_onStack->clear();
		}
		
		if(p_cycle == nullptr)
			p_cycle = new cyclic_path;
		else
			p_cycle->clear();
		
		// directed_edge containers for edge_directed_digraph.
		if(p_cycle_e == nullptr)
			p_cycle_e = new cyclic_path_e;
		else
			p_cycle_e->clear();
		
		_size = V;
	}
	
	void directed_cycle::operator()(const digraph& dg)
	{
		usingEdgeWeightedDigraph = false;

		uint V = dg.V();
		_initialize(V);
		
		for(int v = 0; v < V; v++)
			if((*_marked)[v] == false)
				dfs(dg, v);
	}
	
	void directed_cycle::operator()(const edge_weighted_digraph& dg)
	{
		usingEdgeWeightedDigraph = true;
		
		uint V = dg.V();
		_initialize(V);
		
		for(int v = 0; v < V; v++)
			if((*_marked)[v] == false)
				dfs(dg, v);
	}
	
	void directed_cycle::dfs(const digraph& dg, const uint& v)
	{
		(*_onStack)[v] = true;
		(*_marked)[v] = true;
		
		adj_iter begin, end;
		dg.adj(v, begin, end);
		for(adj_iter W = begin; W != end; W++)
		{
			uint w = *W;
			
			if(hasCycle() == true)
				return;
			// Have we been here before?
			else if((*_marked)[w] == false)
			{
				(*_edgeTo)[w] = v;			// Record the first vertex that lead us to w.
				dfs(dg, w);
			}
			// If we encountered *w before, then we have a cycle.
			else if((*_onStack)[w] == true)
			{
				/*
				 * The v->w edge confirmed that we have a cycle.  Thus starting with v, trace the reverse of the path
				 * (via the x = _edgeTo[x] assignment) that got us here, skipping w for now.  Once done, add w and v,
				 * producing v<-[interior verticies not w]<-(w<-v), where (w<-v) is the final edge on our path to finish
				 * the loop. This is the mirror image of our actual path in the loop, which then is reversed once the
				 * LIFO stack's elements are popped off.
				*/
				
				for(int x = v; x != w; x = (*_edgeTo)[x])
				{
					// See notes in _graphs.h.
					if(x == undefined_uint)
						break;
					
					p_cycle->add(x);
				}
				
				p_cycle->add(w);
				p_cycle->add(v);
			}
		}
		
		(*_onStack)[v] = false;
	}
	
	void directed_cycle::dfs(const edge_weighted_digraph& dg, const uint& v)
	{
		(*_onStack)[v] = true;
		(*_marked)[v] = true;
		
		adje_citer begin, end, E;
		dg.adj(v, begin, end);
		for(E = begin; E != end; E++)
		{
			edge const& e = *E;
			uint w = e.to();
			
			if(hasCycle() == true)
				return;
			
			// Have we been here before?
			else if((*_marked)[w] == false)
			{
				(*_edgeTo)[w] = v;			// Record the first vertex that lead us to w.
				(*_edgeTo_e)[w] = e;
				dfs(dg, w);
			}
			
			// If we encountered w before, then we have a cycle.
			else if((*_onStack)[w] == true)
			{
				/*
				 * The v->w edge confirmed that we have a cycle.  Thus starting with v, trace the reverse of the path
				 * (via the x = _edgeTo[x] assignment) that got us here, skipping w for now.  Once done, add w and v,
				 * producing v<-[interior verticies not w]<-(w<-v), where (w<-v) is the final edge on our path to finish
				 * the loop. This is the mirror image of our actual path in the loop, which then is reversed once the
				 * LIFO stack's elements are popped off.
				 */
				
				for(int x = v; x != w; x = (*_edgeTo)[x])
				{
					// See notes in _graphs.h, and fuck you Java!
					if(x == undefined_uint)
						break;
					
					p_cycle_e->add((*_edgeTo_e)[x]);
					p_cycle->add(x);
				}
				
				p_cycle->add(w);
				p_cycle->add(v);
				p_cycle_e->add(e);
			}
		}
		
		(*_onStack)[v] = false;
	}
}
