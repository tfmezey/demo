#ifndef EDGE_WEIGHTED_GRAPH_H
#define EDGE_WEIGHTED_GRAPH_H

#include "_graphs.h"
#include "edge.h"

#include <sstream>

namespace graphs
{
	// Edge weighted graph container.
	
	/*
	 * As this is an undirected graph, every edge is shared by the vertices.  To cut down on
	 * storage space, we use an array of reverse list of edge pointers in order to cut down on
	 * memory used.  The edges are stored in the _edges[] array.  The _adj is the "stack"
	 * (reverse list) of edge pointers.  We save about 50% of storage.  Note that for the
	 * digraph variant, each vertex needs to store a copy of an edge, in the case that both
	 * point to one another (possibly forming a cycle).
	*/
	
	using namespace std;
	
	class edge_weighted_graph
	{
		
	public:
		edge_weighted_graph() noexcept;
		edge_weighted_graph(const uint&) noexcept;
		edge_weighted_graph(const edge_weighted_graph&) noexcept;
		edge_weighted_graph(edge_weighted_graph&&) noexcept;
		edge_weighted_graph& operator=(const edge_weighted_graph&) noexcept;
		edge_weighted_graph& operator=( edge_weighted_graph&&) noexcept;
		~edge_weighted_graph() noexcept { delete _adj; delete _edges; }
		
		uint V() const { return _V; }
		uint E() const { return _E; }
		void addEdge(const edge&);
		void addEdge(edge&&);
		void clear();
		
		void adj(const uint&, adjpe_iter&, adjpe_iter&) const;
		void adj(const uint&, adjpe_citer&, adjpe_citer&) const;
		
		epath getEdges_epath() const;
		erpath getEdges_erpath() const;
		minpqe getEdges_minpqe() const;
		std::string str() const;
		
	private:
		aradjpe* _adj = nullptr;
		are* _edges = nullptr;
		
		uint _V = 0;
		uint _E = 0;
	};
}

#endif
