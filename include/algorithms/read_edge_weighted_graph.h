#ifndef READ_EDGE_WEIGHTED_GRAPH_H
#define READ_EDGE_WEIGHTED_GRAPH_H

#include <iostream>

#include "_algorithms.h"
#include "graphs.h"
#include "read_tokens.h"

namespace algorithms
{
	using namespace std;
	using namespace containers;
	using namespace graphs;
	
	class read_edge_weighted_graph
	{
	public:
		read_edge_weighted_graph() noexcept {};
		read_edge_weighted_graph(const string&) noexcept;
		~read_edge_weighted_graph() { delete p_g; }
		
		void operator()(const string&);
		void clear();
		
		bool ready() const { return _initialized; }
		edge_weighted_graph EWG() const;
		
		uint V() const { return _V; }
		uint E() const { return _E; }
		
	private:
		edge_weighted_graph* p_g = nullptr;
		
		uint _V = 0;
		uint _E = 0;
		bool _initialized = false;
	};
}

#endif
