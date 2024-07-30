#ifndef BASE_GRAPH_H
#define BASE_GRAPH_H

#include <iterator>
#include <iostream>
#include <sstream>
#include <string>


#include "_graphs.h"

namespace graphs
{
	class base_graph
	{
		
	public:
		base_graph() noexcept;
		base_graph(const uint&) noexcept;
		base_graph(const base_graph&) noexcept;
		base_graph(base_graph&&) noexcept;
		base_graph& operator=(const base_graph&) noexcept;
		base_graph& operator=(base_graph&&) noexcept;
		virtual ~base_graph() noexcept;
		
		virtual void addEdge(const uint&, const uint&) = 0;
		
		// Return iterators at the requested vertex v.
		void adj(const uint& v, adj_iter&, adj_iter&) const;
		void adj(const uint& v, adj_citer&, adj_citer&) const;
		
		bool ready() const { return _V > 0; }
		std::string str() const;
		uint V() const { return _V; }
		uint E() const { return _E; }
		uint size() const { return _size; }
		
		void clear();		// Reset the graph.
		
		static uint degree(base_graph&, const uint&);
		static uint maxDegree(base_graph&);
		static uint avgDegree(base_graph&);
		static uint numberOfSelfLoops(base_graph&);
		
		aradj* _adj = nullptr;
		
		uint _size = 0;
		uint _V = 0;
		uint _E = 0;
	};
}

#endif
