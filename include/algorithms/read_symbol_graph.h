#ifndef READ_SYMBOL_GRAPH_H
#define READ_SYMBOL_GRAPH_H

#include <iostream>
#include <sstream>
#include <string>

#include "_algorithms.h"
#include "read_tokens.h"

#include "graphs.h"
#include "containers.h"

namespace algorithms
{
	// This class that reads strings from file to build a symbol table, and then a graph/digraph.
	
	using namespace std;
	using namespace graphs;
	
	class read_symbol_graph
	{
		using values = containers::array<std::string>;
		using symbols = containers::symbol_table<std::string, uint>;
		using symbols_iter = containers::symbol_table<std::string, uint>::iterator;
		using symbols_citer = containers::symbol_table<std::string, uint>::const_iterator;
		
	public:
		read_symbol_graph(const std::string& filename, const bool& useDigraph=false) noexcept
		{
			_useDigraph = useDigraph;
			_readfile_str(filename);
			
		}
		read_symbol_graph(const std::string& filename, const char& delim, const bool& useDigraph=false) noexcept
		{
			_useDigraph = useDigraph;
			_readfile_str(filename, delim);
			
		}
		~read_symbol_graph() { delete p_st; delete p_keys; delete p_g; };
		
		void reset();
		void readfile(const std::string& filename, const char& delim=' ', const bool& useDigraph=false);
		bool ready() const { return _initialized; }
		std::string str() const;
		
		bool contains(const std::string& s) const;
		int index(const std::string& s) const;
		std::string name(const uint& v) const;
		graphs::graph G() const;
		graphs::digraph DG() const;
		uint V() const { return _V; }
		uint E() const { return _E; }
		
	private:
		
		void _readfile_str(const std::string&, const char& delim=' ');
		
		containers::symbol_table<std::string, uint>* p_st = nullptr;		// string -> index
		containers::array<std::string>* p_keys = nullptr;					// index -> string
		base_graph* p_g = nullptr;
		
		uint _V = 0;
		uint _E = 0;
		
		bool _useDigraph = false;
		bool _initialized = false;
		bool _allocated = false;
	};
}

#endif
