#include "depth_first_search.h"

namespace graphs
{
	depth_first_search::depth_first_search(const base_graph& g, const uint& s)
	{
		uint V = g.V();
		_size = V;
		
		_marked = new bool[V];
		_edgeTo = new uint[V];
		
		for(int v = 0; v < V; v++)
		{
			_marked[v] = false;
			_edgeTo[v] = undefined_uint;
		}
		
		dfs(g, s);
	}
	
	depth_first_search::depth_first_search(const base_graph& g, const arui& sources)
	{
		uint V= g.V();
		_size = V;
		
		_marked = new bool[V];
		_edgeTo = new uint[V];
		
		for(int v = 0; v < V; v++)
		{
			_marked[v] = false;
			_edgeTo[v] = undefined_uint;
		}
		
		for(arui_citer s = sources.cbegin(); s != sources.cend(); s++)
		{
			if(_marked[*s] == false)
				dfs(g, *s);
		}
	}
	
	void depth_first_search::operator()(const base_graph& g, const uint& s)
	{
		uint V = g.V();
		if(_size != V)
		{
			delete[] _marked;
			delete[] _edgeTo;
			
			_marked = nullptr;
			_edgeTo = nullptr;
			
			_size = V;
			
			_marked = new bool[V];
			_edgeTo = new uint[V];
			
			for(int v = 0; v < V; v++)
			{
				_marked[v] = false;
				_edgeTo[v] = undefined_uint;
			}
		}
		
		_count = 0;
		
		dfs(g, s);
	}
	
	void depth_first_search::operator()(const base_graph& g, const arui& sources)
	{
		uint V = g.V();
		if(_size != V)
		{
			delete[] _marked;
			delete[] _edgeTo;
			
			_marked = nullptr;
			_edgeTo = nullptr;
			
			_size = V;
			
			_marked = new bool[V];
			_edgeTo = new uint[V];
			
			for(int v = 0; v < V; v++)
			{
				_marked[v] = false;
				_edgeTo[v] = undefined_uint;
			}
		}
		
		_count = 0;
		
		for(arui_citer s = sources.cbegin(); s != sources.cend(); s++)
		{
			if(_marked[*s] == false)
				dfs(g, *s);
		}
	}
	
	void depth_first_search::dfs(const base_graph& g, const uint& v)
	{
		_marked[v] = true;
		_count++;
		
		adj_iter begin, end, W;
		g.adj(v, begin, end);
		for(W = begin; W != end; W++)
		{
			uint w = *W;
			
			// Have we been here before?
			if(_marked[w] == false)
			{
				_edgeTo[w] = v;			// Record the first vertex that lead us to w.
				dfs(g, w);
			}
		}
	}
	
	// Modify stack object reference.
	void depth_first_search::pathTo(path& p, const uint& s, const uint& v)
	{
		if(hasPathTo(v) == false)
			return;
		for(int x = v; x != s; x = _edgeTo[x])
		{
			if(x == undefined_uint)
				break;
			
			p.add(x);
		}
		
		p.add(s);
	}
}
