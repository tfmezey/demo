#include "graphs/edge_weighted_digraph.h"

namespace graphs
{
	edge_weighted_digraph::edge_weighted_digraph() noexcept
	{
		_adj = new aradje(default_size, default_size);
		_size = default_size;
	}
	
	edge_weighted_digraph::edge_weighted_digraph(const uint& V) noexcept
	{
		_V = V;
		_adj = new aradje(V, V);
		_size = V;
	}
	
	edge_weighted_digraph::edge_weighted_digraph(const edge_weighted_digraph& g) noexcept
	{
		_V = g._V;
		_E = g._E;
		// _adj = new arstde(*g._adj);
		_adj = new aradje(_V);
		for(int v = 0; v < _V; v++)
			_adj->addAt(v, g._adj->get(v));
		
		_size = g._size;
	}
	
	edge_weighted_digraph::edge_weighted_digraph(edge_weighted_digraph&& g) noexcept
	{
		_V = g._V;
		_size = g._size;
		
		g._V = 0;
		g._E = 0;
		g._size = 0;
		
		_adj = g._adj;
		g._adj = nullptr;
	}
	
	edge_weighted_digraph& edge_weighted_digraph::operator=(const edge_weighted_digraph& g) noexcept
	{
		_V= g._V;
		_E = g._E;
		delete _adj;
		_adj = new aradje(_V);
		for(int v = 0; v < _V; v++)
			_adj->addAt(v, g._adj->get(v));
		
		_size = g._size;
		
		return *this;
	}

	edge_weighted_digraph& edge_weighted_digraph::operator=(edge_weighted_digraph&& g) noexcept
	{
		uint V = _V;
		_V= g._V;
		g._V = V;
		
		uint E = _E;
		_E = g._E;
		g._E = E;
		
		uint size = _size;
		_size = g._size;
		g._size = size;
		
		aradje* temp = _adj;
		_adj = g._adj;
		g._adj = temp;
		
		return *this;
	}
	
	void edge_weighted_digraph::addEdge(const edge& de)
	{
		// Only add from v to w.
		uint v = de.from();
		uint w = de.to();
		uint max = v > w ? v : w;
		
		// Account for default construction:  _V not set.
		if(max >= _adj->size())
		{
			_adj->reserve(max);
			_V = max + 1;
		}
		
		_adj->get(v).add(&de);
		_E++;
	}

	void edge_weighted_digraph::addEdge(edge&& de)
	{
		// Only add from v to w.
		uint v = de.from();
		uint w = de.to();
		uint max = v > w ? v : w;
		// Account for default construction:  _V not set.
		if(max >= _adj->size())
		{
			_adj->reserve(max);
			_V = max + 1;
		}
		
		_adj->get(de.from()).add(&de);
		_E++;
	}
	
	void edge_weighted_digraph::adj(const uint& v, adje_iter& b, adje_iter& e) const
	{
		_adj->get(v).get_iters(b, e);
	}
	void edge_weighted_digraph::adj(const uint& v, adje_citer& b, adje_citer& e) const
	{
		_adj->get(v).get_citers(b, e);
	}
	
	epath edge_weighted_digraph::getEdges_epath() const
	{
		// Return stack of all unique edges of the graph.
		epath result;
		
		for(int v = 0; v < _V; v++)
		{
			adje_citer begin, end, E;
			_adj->get(v).get_citers(begin, end);
			for(E = begin; E != end; E++)
			{
				edge const& e = *E;
				if(e.from() > v)		// if w < v, it would already been added.
					result.add(&e);
			}
		}
		
		return result;
	}
	
	erpath edge_weighted_digraph::getEdges_erpath() const
	{
		// Return stack of all unique edges of the graph.
		erpath result;
		
		for(int v = 0; v < _V; v++)
		{
			adje_citer begin, end, E;
			_adj->get(v).get_citers(begin, end);
			for(E = begin; E != end; E++)
			{
				edge const& e = *E;
				if(e.from() > v)		// if w < v, it would already been added.
					result.add(e);
			}
		}
		
		return result;
	}
	
	minpqe edge_weighted_digraph::getEdges_minpqe() const
	{
		// Return stack of all unique edges of the graph.
		minpqe result;
		
		for(int v = 0; v < _V; v++)
		{
			adje_citer begin, end, E;
			_adj->get(v).get_citers(begin, end);
			for(E = begin; E != end; E++)
			{
				edge const& e = *E;
				if(e.from() > v)		// if w < v, it would already been added.
					result.enqueue(e);
			}
		}
		
		return result;
	}
	
	void edge_weighted_digraph::clear()
	{
		for(int v = 0; v < _V; v++)
			_adj->get(v).clear();
		
		_adj->clear();
		_V = 0;
		_E = 0;
	}
	
	std::string edge_weighted_digraph::str() const
	{
		using namespace std;
		ostringstream o;
		string indentation = "     ";
		
		for(int v = 0; v < _V; v++)
		{
			adje_citer begin, end, E;
			o << indentation << "g[" << v << "] = ";
			_adj->get(v).get_citers(begin, end);
			for(E = begin; E != end; E++)
			{
				edge const& e = *E;
				o << e.str() << " ";
			}
			
			o << endl;
		}
		
		return o.str();
	}
}
