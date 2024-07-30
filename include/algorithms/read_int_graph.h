#ifndef READ_INT_GRAPH_H
#define READ_INT_GRAPH_H

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
	
	class read_int_graph
	{
		
	public:
		read_int_graph() noexcept {};
		read_int_graph(const std::string& filename, const bool& useDigraph) noexcept;
		read_int_graph(const std::string& filename, const char& delim, const bool& useDigraph) noexcept;
		~read_int_graph() noexcept { delete p_g; }
		
		void readfile(const std::string&, const char& delim, const bool& useDigraph);
		graph G() const;
		digraph DG() const;
		bool ready() const { return _initialized; }
		std::string str() const;
		uint V() const { return _V; }
		uint E() const { return _E; }
		
	private:
		void _readfile_int(const std::string&, const char& delim);
		void reset();
		
		base_graph* p_g = nullptr;
		
		uint _V = 0;
		uint _E = 0;
		
		bool _useDigraph = false;
		bool _initialized = false;
	};
}

#endif
