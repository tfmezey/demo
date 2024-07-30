#ifndef TOPOLOGICAL_H
#define TOPOLOGICAL_H

#include "_graphs.h"
#include "digraph.h"
#include "directed_cycle.h"
#include "depth_first_order.h"
#include "edge.h"
#include "edge_weighted_digraph.h"

#include "containers.h"

namespace graphs
{
	/*
	 * Topological Order
	 * 
	 * Topological sort:  Put the vertices in such an order, where all (directed) edges
	 * are from a vertex earlier in the order to a vertex later in the order.
	 * 
	 * A graph has a topological order only if it is a DAG.
	 * 
	 * Use directed_cycle (which uses a DFS routine) to determine whether the
	 * graph has a cycle.  If not, then proceed to obtain the topological order:
	 * 
	 * Using depth_first_order (which creates Pre, Post and reversePost vertex
	 * order based on the DFS routine excecution), obtain the reversePost order,
	 * which is the Topological order.
	 * 
	 * Running time order is proportional to the running time of DFS, namely O(V+E).
	*/
	
	class topological
	{
		
	public:
		topological(const topological&) = delete;
		topological(topological&&) = delete;
		topological& operator=(const topological&) = delete;
		topological& operator=(topological&&) = delete;
		
		topological() { _order = new path; };
		topological(const digraph&);
		topological(const edge_weighted_digraph&);
		~topological() { delete _order; };
		
		void operator()(const digraph&);
		void operator()(const edge_weighted_digraph&);

		bool isDAG() const { return _order->size() != 0; }		// If we have a cycle, then _order would be empty!
		path order() const { return *_order; }
		
	private:
		path* _order = nullptr;
	};
}

#endif
