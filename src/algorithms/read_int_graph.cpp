#include "read_int_graph.h"

namespace algorithms
{
	read_int_graph::read_int_graph(const std::string& filename, const bool& useDigraph) noexcept
	{
		_useDigraph = useDigraph;
		_readfile_int(filename, ' ');
	}
	
	read_int_graph::read_int_graph(const std::string& filename, const char& delim, const bool& useDigraph) noexcept
	{
		_useDigraph = useDigraph;
		_readfile_int(filename, delim);
	}
	
	void read_int_graph::reset()
	{
		_initialized = false;
		
		_V = 0;
		_E = 0;
		
		delete p_g;
		p_g = nullptr;
	}
	
	void read_int_graph::readfile(const std::string& filename, const char& delim, const bool& useDigraph)
	{
		_useDigraph = useDigraph;
		_readfile_int(filename, delim);
	}
	
	void read_int_graph::_readfile_int(const std::string& filename, const char& delim)
	{
		reset();
		uint n1 = 0;
		uint n2 = 0;
		int line_count = 0;
		using Type = read_tokens::tokenType;
		Type result = Type::NONE;
		
		read_tokens in(filename, delim);
		if(in.ready() != true)
		{
			cerr << "Failed initializing read_int.  Exiting." << endl;
			return;
		}
		
		result = in.nextToken();
		if(result == Type::INT)
			_V = in.getIntToken();
		else
		{
			cerr << "Failed to read in V." << endl;
			return;
		}
		
		if(_useDigraph == true)
			p_g = new digraph(_V);
		else
			p_g = new graph(_V);
		
		line_count++;
		result = in.nextToken();
		if(result == Type::INT)
			_E = in.getIntToken();
		else
		{
			cerr << "Failed to read in V." << endl;
			return;
		}
		
		line_count++;
		while(in.eof() == false)
		{
			do
			{
				result = in.nextToken();
				if(result == Type::EOFTOKEN)
					goto FINISHED_FILE;
				else if(result == Type::INT)
					n1 = in.getIntToken();
				else
				{
					cerr << "Line " << line_count << ":  Failed to read in first vertex." << endl;
					return;
				}
				
				result = in.nextToken();
				if(result == Type::INT)
					n2 = in.getIntToken();
				else
				{
					cerr << "Line " << line_count << ":  Failed to read in second vertex." << endl;
					return;
				}
				
				if(_useDigraph == true)
					((digraph*)p_g)->addEdge(n1, n2);
				else
					((graph*)p_g)->addEdge(n1, n2);
			}
			while(in.eol() == false);
			
			line_count++;
		}
		
		FINISHED_FILE:
		
		if(p_g->ready() == true)
			_initialized = true;
		else
		{
			cerr << "Failed to read in graph." << endl;
			_initialized = false;
		}
		
		return;
	}
	
	graph read_int_graph::G() const
	{
		if(p_g == nullptr)
			return graphs::graph();
		else
			return *((graph*)p_g);
	}
	
	digraph read_int_graph::DG() const
	{
		if(p_g == nullptr)
			return graphs::digraph();
		else
			return *((digraph*)p_g);
	}
	
	std::string read_int_graph::str() const
	{
		if(_initialized == false)
			return std::string("");
		
		std::ostringstream o;
		
		adj_citer begin, end;
		for(int v = 0; v < _V; v++)
		{
			p_g->adj(v, begin, end);
			o << "g[" << v << "] = ";
			for(adj_citer i = begin; i != end; i++)
				o << *i << " -> ";
			
			o << endl;
		}
		
		return o.str();
	}
}
