#ifndef READ_EDGE_WEIGHTED_DIGRAPH_H
#define READ_EDGE_WEIGHTED_DIGRAPH_H

#include <iostream>
#include <sstream>

#include "_algorithms.h"
#include "graphs.h"
#include "read_tokens.h"

namespace algorithms
{
	using namespace std;
	using namespace graphs;
	
	// This class reads integers from file to build a graph/digraph.
	
	class read_edge_weighted_digraph
	{
		using ewdg = graphs::edge_weighted_digraph;
		
	public:
		read_edge_weighted_digraph() noexcept {};
		read_edge_weighted_digraph(const std::string& filename) noexcept;
		read_edge_weighted_digraph(const std::string& filename, const char& delim) noexcept;
		~read_edge_weighted_digraph() noexcept { delete p_g; }
		
		void operator()(const std::string& filename);
		void operator()(const std::string& filename, const char& delim);
		
		ewdg EWDG() const;
		bool ready() const { return _initialized; }
		uint V() const { return _V; }
		uint E() const { return _E; }
		
		void clear();
		
	private:
		void reset();
		void _read_file(const std::string&, const char&);
		
		edge_weighted_digraph* p_g = nullptr;
		
		uint _V = 0;
		uint _E = 0;
		
		bool _initialized = false;
	};
}

#endif
