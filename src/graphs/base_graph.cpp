#include "base_graph.h"

namespace graphs
{
	using namespace std;
	
	base_graph::base_graph() noexcept
	{
		_adj = new aradj(default_size, default_size);
		_size = default_size;
	}
	
	base_graph::base_graph(const uint& V) noexcept
	{
		_V = V;
		_adj = new aradj(V, V);
		_size = V;
	}
	
	base_graph::base_graph(const base_graph& g) noexcept
	{
		_V = g._V;
		_E = g._E;
		
		_adj = new aradj(_V);
		// _adj = new aradj(*g._adj);
		
		for(int v = 0; v < _V; v++)
			_adj->addAt(v, g._adj->get(v));
		
		_size = g._size;
	}
	
	base_graph::base_graph(base_graph&& g) noexcept
	{
		uint V = _V;
		_V = g._V;
		g._V = V;
		
		uint E = _E;
		_E = g._E;
		g._E = E;
		
		uint size = _size;
		_size = g._size;
		g._size = size;
		
		aradj* temp = _adj;
		_adj = g._adj;
		g._adj = temp;
	}
	
	base_graph& base_graph::operator=(const base_graph& g) noexcept
	{
		_V = g._V;
		_E = g._E;
		
		if(_adj != nullptr)
			delete _adj;
		
		_adj = new aradj(_V);
		_size = g._size;
		
		for(int v = 0; v < _V; v++)
			_adj->addAt(v, g._adj->get(v));
		
		return *this;
	}
	
	base_graph& base_graph::operator=(base_graph&& g) noexcept
	{
		uint V = _V;
		_V = g._V;
		g._V = V;
		
		uint E = _E;
		_E = g._E;
		g._E = E;
		
		uint size = _size;
		_size = g._size;
		g._size = size;
		
		aradj* temp = _adj;
		_adj = g._adj;
		g._adj = temp;
		
		return *this;
	}
	
	base_graph::~base_graph() noexcept
	{
		delete _adj;
	}
	
	uint base_graph::degree(base_graph& g, const uint& v)
	{
		uint degree = 0;
		
		for(auto w = g._adj[v].begin(); w != g._adj[v].end(); w++)
			degree++;
		
		return degree;
	}
	
	uint base_graph::maxDegree(base_graph& g)
	{
		uint max = 0;
		for(uint v = 0; v < g.V(); v++)
			if(degree(g, v) > max)
				max = degree(g, v);
		
		return max;
	}
	
	uint base_graph::avgDegree(base_graph& g)
	{
		return 2 * g.E() / g.V();
	}
	
	uint base_graph::numberOfSelfLoops(base_graph& g)
	{
		uint count = 0;
		adj_citer begin, end, w;
		for(uint v = 0; v < g.V(); v++)
		{
			g._adj->get(v).get_citers(begin, end);
			for(w = begin; w != end; w++)
			if(v == *w)
				count++;
		}
		
		return count/2;		// Each edge was counted twice
	}
	
	std::string base_graph::str() const
	{
		ostringstream o;
		string indentation = "     ";
		
		o << indentation << _V << endl << _E << endl;
		
		adj_citer begin, end, it;
		for(uint v = 0; v < _V; v++)
		{
			o << indentation << "g[" << v << "] = ";
			_adj->get(v).get_citers(begin, end);
			for(it = begin; it != end; it++)
				o << (*it) << " -> ";
			
			o << endl;
		}
		
		return o.str();
	}
	
	void base_graph::adj(const uint& v, adj_iter& begin, adj_iter& end) const
	{
		_adj->get(v).get_iters(begin, end);
		
		return;
	}
	
	void base_graph::adj(const uint& v, adj_citer& begin, adj_citer& end) const
	{
		_adj->get(v).get_citers(begin, end);
		
		return;
	}
	
	void base_graph::clear()
	{
		for(int v = 0; v < _V; v++)
			_adj->get(v).clear();
		
		_adj->clear();
		_V = 0;
		_E = 0;
	}
}
