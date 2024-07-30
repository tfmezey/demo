#ifndef DIRECTED_CYCLE_H
#define DIRECTED_CYCLE_H

#include "_graphs.h"
#include "digraph.h"
#include "containers.h"
#include "edge.h"
#include "edge_weighted_digraph.h"

namespace graphs
{
	/*
	 * Directed Cycle:
	 * 
	 * Does graph contain a directed cycle?
	 * 
	 * Use two bool arrays and one uint array for the algorithm:  _marked[] _onStack[] and _marked[],
	 * with _marked[] and _edgeTo[] inherited from the original DFS implementation.
	 * 
	 * In the DFS routine, continue marking vertices as visited and place them on the stack, until we
	 * encounter a vertex already marked as on the stack.
	 * 
	 * If there is a cycle, trace the path back via the _edgeTo[] and the verticies in on the stack:
	 * 		for(int x = v; x != w; x = _edgeTo[x])
	 * 		{
	 * 			if(x == undefined_uint)
	 *	 			break;
	 * 
	 * 			stack.push(x).
	 * 		}
	 * As before, the _edgeTo[x] is initialized for undefined_uint to terminate the loop.
	*/
	
	class directed_cycle
	{
	public:
		directed_cycle() = delete;
		directed_cycle(const directed_cycle&) = delete;
		directed_cycle(directed_cycle&&) = delete;
		directed_cycle& operator=(const directed_cycle&) = delete;
		directed_cycle& operator=(directed_cycle&&) = delete;
		
		directed_cycle(const digraph&);
		directed_cycle(const edge_weighted_digraph&);
		~directed_cycle() { delete _marked; delete _edgeTo; delete _onStack; delete p_cycle; delete p_cycle_e; delete _edgeTo_e; }
		
		void operator()(const digraph&);
		void operator()(const edge_weighted_digraph&);
		
		bool hasCycle() const { if(usingEdgeWeightedDigraph == true) return p_cycle_e->size() != 0; else return p_cycle->size() != 0; }
		cyclic_path getCycle() const { return *p_cycle; }
		cyclic_path_e getCycle_e() const { return *p_cycle_e; }
		
	private:
		void dfs(const digraph&, const uint&);
		void dfs(const edge_weighted_digraph&, const uint&);
		void _initialize(const uint&);
		
		containers::array<bool>* _marked = nullptr;
		arui* _edgeTo = nullptr;
		are* _edgeTo_e = nullptr;
		containers::array<bool>* _onStack = nullptr;
		cyclic_path* p_cycle = nullptr;
		cyclic_path_e* p_cycle_e = nullptr;
		
		uint _size = 0;
		bool usingEdgeWeightedDigraph = false;
	};
}

#endif
