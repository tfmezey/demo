#ifndef ACYCLIC_SP_H
#define ACYCLIC_SP_H

#include "_graphs.h"
#include "edge_weighted_digraph.h"
#include "edge.h"
#include "topological.h"

namespace graphs
{
	/*
	 * Find the shortest path for a DAG.
	 * 
	 * Topological sort:  Put the vertices in such an order, where all (directed) edges
	 * are from a vertex earlier in the order to a vertex later in the order.
	 * 
	 * Use topoligical sort (the algorithm exclusive to DAGs) to obtain the topological
	 * order to relax the graph's vertices in that order, which causes edges to be relaxed
	 * only once due to the aforementioned nature of that order.
	 * 
	 * O(E+V)
	*/
	
	class acyclic_SP
	{
	private:
		void relax(const edge_weighted_digraph&, const uint&);
		void _initialize(const uint& v);
		
		double* _distTo = nullptr;
		edge* _edgeTo = nullptr;
		
	public:
		acyclic_SP() = delete;
		acyclic_SP(const acyclic_SP&) = delete;
		acyclic_SP(acyclic_SP&&) = delete;
		acyclic_SP& operator=(const acyclic_SP&) = delete;
		acyclic_SP& operator=(acyclic_SP&&) = delete;
		
		acyclic_SP(const edge_weighted_digraph&, const uint&);
		~acyclic_SP() { delete[] _distTo; delete[] _edgeTo; }
		
		void operator()(const edge_weighted_digraph&, const uint&);
		
		bool hasPathTo(const uint& v) const { return _distTo[v] != inf; }
		double distance(const uint& v) const { return _distTo[v]; }
		epath getPathTo(const uint&) const;
	};
}

#endif
