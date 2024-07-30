#ifndef DIGRAPH_H
#define DIGRAPH_H

#include "_graphs.h"
#include "base_graph.h"

namespace graphs
{	
	// Data container class for directed graphs.
	
	using namespace std;
	
	class digraph : public base_graph
	{
		
	public:
		digraph() noexcept : base_graph() {};
		digraph(const uint& V) noexcept : base_graph(V) {};
		digraph(const digraph& g) noexcept : base_graph(g) {};
		digraph(digraph&& g) noexcept : base_graph(g) {};
		digraph& operator=(const digraph& g) noexcept { base_graph::operator=(g); return *this; };
		digraph& operator=(digraph&& g) noexcept { base_graph::operator=(g); return *this; };
		virtual ~digraph() noexcept {};
		
		void addEdge(const uint&, const uint&);
		// Produce a graph obtained by reversing the direction of edges:  v->w => v<-w
		digraph reverse() const;
	};
}

#endif

