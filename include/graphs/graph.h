#ifndef GRAPH_H
#define GRAPH_H

#include "_graphs.h"
#include "base_graph.h"

namespace graphs
{
	// Data container class for undirected graphs.
	
	using namespace std;
	
	class graph : public base_graph
	{
	public:
		graph() noexcept : base_graph() {};
		graph(const uint& V) noexcept : base_graph(V) {};
		graph(const graph& g) noexcept : base_graph(g) {};
		graph(graph&& g) noexcept : base_graph(g) {};
		graph& operator=(const graph& g) noexcept { base_graph::operator=(g); return *this; };
		graph& operator=(graph&& g) noexcept { base_graph::operator=(g); return *this; };
		virtual ~graph() noexcept {};
		
		void addEdge(const uint&, const uint&);
	};
}

#endif
