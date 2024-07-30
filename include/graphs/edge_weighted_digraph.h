#ifndef EDGE_WEIGHTED_DIGRAPH_H
#define EDGE_WEIGHTED_DIGRAPH_H

#include <iostream>
#include <sstream>

#include "_graphs.h"
#include "edge.h"

namespace graphs
{
	class edge_weighted_digraph
	{
	public:
		edge_weighted_digraph() noexcept;
		edge_weighted_digraph(const uint&) noexcept;
		edge_weighted_digraph(const edge_weighted_digraph&) noexcept;
		edge_weighted_digraph(edge_weighted_digraph&&) noexcept;
		edge_weighted_digraph& operator=(const edge_weighted_digraph&) noexcept;
		edge_weighted_digraph& operator=( edge_weighted_digraph&&) noexcept;
		~edge_weighted_digraph() noexcept { delete _adj; }
		
		uint V() const { return _V; }
		uint E() const { return _E; }
		void addEdge(const edge&);
		void addEdge(edge&&);
		void clear();
		
		void adj(const uint&, adje_iter&, adje_iter&) const;
		void adj(const uint&, adje_citer&, adje_citer&) const;
		
		epath getEdges_epath() const;
		erpath getEdges_erpath() const;
		minpqe getEdges_minpqe() const;
		std::string str() const;
		
	private:
		aradje* _adj = nullptr;
		
		uint _V = 0;
		uint _E = 0;
		uint _size = 0;
	};
}

#endif
