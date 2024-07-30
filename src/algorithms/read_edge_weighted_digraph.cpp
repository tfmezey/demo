#include "read_edge_weighted_digraph.h"

namespace algorithms
{
	read_edge_weighted_digraph::read_edge_weighted_digraph(const std::string& filename) noexcept
	{
		_read_file(filename, ' ');
	}
	
	read_edge_weighted_digraph::read_edge_weighted_digraph(const std::string& filename, const char& delim) noexcept
	{
		_read_file(filename, delim);
	}
	
	void read_edge_weighted_digraph::_read_file(const std::string& filename, const char& delim)
	{
		int n1 = 0;
		int n2 = 0;
		double w = 0.0;
		int linecount = 0;
		
		read_tokens rn(filename, delim);
		if(rn.ready() == false)
		{
			cerr << "Failed reading file " << filename << "." << endl;
			return;
		}
		
		int lines = rn.size();
		
		if(_DEBUG == true)
			cout << filename << ":  read " << lines <<  " lines." << endl;
		
		using Type = read_tokens::tokenType;
		Type result = Type::NONE;
		
		result = rn.nextToken();
		if(result == Type::INT)
			_V = rn.getIntToken();
		else
		{
			cerr << "Failed to read in V." << endl;
			return;
		}
		
		linecount++;
		result = rn.nextToken();
		if(result == Type::INT)
			_E = rn.getIntToken();
		else
		{
			cerr << "Failed to read in E." << endl;
			return;
		}
		
		if(_DEBUG == true)
			cout << "_V = " << _V << ", _E = " << _E << endl;
		
		p_g = new ewdg(_V);
		
		linecount++;
		while(rn.eof() == false)
		{
			// Read in two integers and a double in that order.
			result = rn.nextToken();
			if(result != Type::INT)
			{
				cerr << "Line " << linecount << ":  Failed reading first edge vertex." << endl;
				return;
			}
			else
				n1 = rn.getIntToken();
			
			result = rn.nextToken();
			if(result != Type::INT)
			{
				cerr << "Line " << linecount << ":  Failed reading second edge vertex." << endl;
				return;
			}
			else
				n2 = rn.getIntToken();
			
			result = rn.nextToken();
			if(result != Type::REAL)
			{
				cerr << "Line " << linecount << ":  Failed reading edge weight." << endl;
				return;
			}
			else
				w = rn.getRealToken();
			
			p_g->addEdge(edge(n1, n2, w));
			linecount++;
		}
		
		_initialized = true;
	}
	
	ewdg read_edge_weighted_digraph::EWDG() const
	{
		if(_initialized == false)
			return ewdg();
		else
			return *p_g;
	}
}
