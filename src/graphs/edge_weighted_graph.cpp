#include "edge_weighted_graph.h"

namespace graphs
{
	edge_weighted_graph::edge_weighted_graph() noexcept
	{
		_adj = new aradjpe(default_size, default_size);
		_edges = new are(2*default_size);
	}
	
	edge_weighted_graph::edge_weighted_graph(const uint& V) noexcept
	{
		_V = V;
		_adj = new aradjpe(_V, _V);
		_edges = new are(2*_V);
	}
	
	edge_weighted_graph::edge_weighted_graph(const edge_weighted_graph& g) noexcept
	{
		// Don't set _E, as it increases the array's _count variable.  addEdge() will take care of the correct value.
		_V = g._V;
		_E = g._E;
		
		_adj = new aradjpe(_V);
		_edges = new are(2*_V);
		
		for(int v = 0; v < default_size; v++)
			_adj->addAt(v, adjpe());
		
		for(are_citer e = g._edges->cbegin(); e != g._edges->cend(); e++)
			addEdge(*e);
	}
	
	edge_weighted_graph::edge_weighted_graph(edge_weighted_graph&& g) noexcept
	{
		_V = g._V;
		_E = g._E;
		
		_adj = g._adj;
		g._adj = nullptr;
		
		_edges = g._edges;
		g._edges = nullptr;
	}
	
	edge_weighted_graph& edge_weighted_graph::operator=(const edge_weighted_graph& g) noexcept
	{
		_V = g._V;
		_E= 0;					// Let addEdge() handle this.

		_adj->clear();
		_edges->clear();
		
		for(int v = 0; v < _V; v++)
			_adj->addAt(v, adjpe());
		
		for(are_citer e = g._edges->cbegin(); e != g._edges->cend(); e++)
			addEdge(*e);
		
		return *this;
	}
	
	edge_weighted_graph& edge_weighted_graph::operator=(edge_weighted_graph&& g) noexcept
	{
		int V = _V;
		_V = g._V;
		g._V = V;
		
		int E = _E;
		_E = g._E;
		g._E = _E;
		
		aradjpe* temp1 = _adj;
		are* temp2 = _edges;
		
		_adj = g._adj;
		g._adj = temp1;
		
		_edges = g._edges;
		g._edges = temp2;
		
		return *this;
	}
	
	void edge_weighted_graph::addEdge(const edge& e)
	{
		// Add edge, and adjust _E appropriately.  Leave _E at 0 in constructors.
		uint v = e.either();
		uint w = e.other(v);
		
		_edges->addAt(_E, e);
		edge& edge = _edges->get(_E);
		_adj->get(v).add(&edge);
		_adj->get(w).add(&edge);
		
		_E++;
		
		// Account for default construction:  _V not set.
		uint max = v > w ? v : w;
		
		if(_V < max)
			_V = max + 1;
	}
	
	void edge_weighted_graph::addEdge(edge&& e)
	{
		// Add edge, and adjust _E appropriately.  Leave _E at 0 in constructors.
		uint v = e.either();
		uint w = e.other(v);

		_edges->addAt(_E, e);
		edge& edge = _edges->get(_E);
		_adj->get(v).add(&edge);
		_adj->get(w).add(&edge);

		_E++;
		
		// Account for default construction:  _V not set.
		uint max = v > w ? v : w;
		
		if(_V < max)
			_V = max + 1;
	}

	void edge_weighted_graph::adj(const uint& v, adjpe_iter& begin, adjpe_iter& end) const
	{
		_adj->get(v).get_iters(begin, end);
	}
	
	void edge_weighted_graph::adj(const uint& v, adjpe_citer& begin, adjpe_citer& end) const
	{
		_adj->get(v).get_citers(begin, end);
	}
	
	epath edge_weighted_graph::getEdges_epath() const
	{
		// Return stack of all unique edges of the graph.
		epath result;
		
		for(int v = 0; v < _V; v++)
		{
			adjpe_citer begin, end, E;
			_adj->get(v).get_citers(begin, end);
			for(E = begin; E != end; E++)
			{
				edge& e = **E;
				if(e.other(v) > v)		// if w < v, it would already been added.
					result.add(e);
			}
		}
		
		return result;
	}
	
	erpath edge_weighted_graph::getEdges_erpath() const
	{
		// Return stack of all unique edges of the graph.
		erpath result;
		
		for(int v = 0; v < _V; v++)
		{
			adjpe_citer begin, end, e;
			_adj->get(v).get_citers(begin, end);
			for(e = begin; e != end; e++)
			{
				if((*e)->other(v) > v)		// if w < v, it would already been added.
					result.add(*(*e));
			}
		}
		
		return result;
	}
	
	minpqe edge_weighted_graph::getEdges_minpqe() const
	{
		// Return stack of all unique edges of the graph.
		minpqe result;
		
		for(int v = 0; v < _V; v++)
		{
			adjpe_citer begin, end, e;
			_adj->get(v).get_citers(begin, end);
			for(e = begin; e != end; e++)
			{
				if((*e)->other(v) > v)		// if w < v, it would already been added.
					result.enqueue(**e);
			}
		}
		
		return result;
	}
	
	std::string edge_weighted_graph::str() const
	{
		ostringstream o;
		string indentation = "      ";
		
		adjpe_citer cbegin, cend, E;
		for(int v = 0; v < _V; v++)
		{
			adjpe& vertex = _adj->get(v);
			o << indentation << "g[" << v << "] = ";
			vertex.get_citers(cbegin, cend);
			for(E = cbegin; E != cend; E++)
			{
				edge const& e = **E;
				o << e.str() << ", ";
			}
			
			o << endl;
		}
		
		return o.str();
	}
	
	void edge_weighted_graph::clear()
	{
		_V = 0;
		_E = 0;
		for(int v = 0; v < _V; v++)
			_adj->get(v).clear();
		
		_adj->clear();
		_edges->clear();
	}
}
