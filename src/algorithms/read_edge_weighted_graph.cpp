#include "read_edge_weighted_graph.h"

namespace algorithms
{
	read_edge_weighted_graph::read_edge_weighted_graph(const string& filename) noexcept
	{
		int n1 = 0;
		int n2 = 0;
		double r1 = 0.0;
		int linecount = 0;

		read_tokens rn(filename, ' ');
		if(rn.ready() == false)
		{
			cerr << "Failed to read file " << filename << "." << endl;
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
		
		p_g = new edge_weighted_graph(_V);
		
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
				r1 = rn.getRealToken();
			
			p_g->addEdge(edge(n1, n2, r1));
			linecount++;
		}
		
		cout << p_g->str() << endl;
		_initialized = true;
	}
	
	edge_weighted_graph read_edge_weighted_graph::EWG() const
	{
		if(p_g == nullptr)
			return edge_weighted_graph();
		else
			return *p_g;
	}
	
	void read_edge_weighted_graph::clear()
	{
		_V = 0;
		_E = 0;
		p_g->clear();
		_initialized = false;
	}
	
	void read_edge_weighted_graph::operator()(const string& filename)
	{
		clear();
		
		int n1 = 0;
		int n2 = 0;
		double r1 = 0.0;
		int linecount = 0;
		
		read_tokens rn(filename, ' ');
		if(rn.ready() == false)
		{
			cerr << "Failed to read file " << filename << "." << endl;
			return;
		}
		
		int lines = rn.size();
		
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
		
		cout << "_V = " << _V << ", _E = " << _E << endl;
		p_g = new edge_weighted_graph(_V);
		
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
				r1 = rn.getRealToken();
			
			p_g->addEdge(edge(n1, n2, r1));
			linecount++;
		}
		
		_initialized = true;
	}
}
