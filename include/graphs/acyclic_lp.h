#ifndef ACYCLIC_LP_H
#define ACYCLIC_LP_H

#include "_graphs.h"
#include "edge_weighted_digraph.h"
#include "edge.h"
#include "topological.h"

namespace graphs
{
	/* 
	 * Uses a modified acyclic_sp algorithm, where edges are either negated, or
	 * the optimiality condition is reversed, to obtain the longest path between
	 * two vertices.
	 * 
	 * Find the longest paths (negative weight edges permitted) for a DAG.
	 * Negate the weights, and the shortest path is the longest upon sign reversal.
	 * Alternatively, initialize weights to neginf, use < instead of > in relax().
	 * Also change hasPathTo() to use neginf.
	 * 
	 * O(E+V).  
	*/
	
	class acyclic_LP
	{
		
	private:
		
		double* _distTo = nullptr;
		edge* _edgeTo = nullptr;
		
		void relax(const edge_weighted_digraph&, const uint&);
		void _initialize(const uint& v);
		
	public:
		
		acyclic_LP() = delete;
		acyclic_LP(const acyclic_LP&) = delete;
		acyclic_LP(acyclic_LP&&) = delete;
		acyclic_LP& operator=(const acyclic_LP&) = delete;
		acyclic_LP& operator=(acyclic_LP&&) = delete;
		
		acyclic_LP(const edge_weighted_digraph&, const uint&);
		~acyclic_LP() { delete[] _distTo; delete[] _edgeTo; }
		
		void operator()(const edge_weighted_digraph&, const uint&);
		
		bool hasPathTo(const uint& v) const { return _distTo[v] != neginf; }
		double distance(const uint& v) const { return _distTo[v]; }
		epath getPathTo(const uint&) const;
	};
}

#endif
