#include "directed_dfs_multi.h"

namespace graphs
{
	directed_DFS_multi::directed_DFS_multi(const digraph& g, const uint& s)
	{
		_allocate(g);
	}
	
	directed_DFS_multi::~directed_DFS_multi()
	{
		delete[] _marked;
		delete[] _edgeTo;
		delete p_g;
	}
	
	void directed_DFS_multi::_allocate(const digraph& g)
	{
		if(p_g != nullptr and &g != p_g)
		{
			delete p_g;
			p_g = new digraph(g);
		}
		else if(p_g == nullptr)
			p_g = new digraph(g);
		
		_V = g.V();
		
		if(_edgeTo == nullptr)
			_edgeTo = new uint[_V];
		
		if(_marked == nullptr)
			_marked = new bool[_V];
		
		for(int v = 0; v < _V; v++)
		{
			_edgeTo[v] = undefined_uint;
			_marked[v] = false;
		}
		
		_initialized = true;
	}
	
	void directed_DFS_multi::_reset()
	{
		for(int v = 0; v < _V; v++)
		{
			_marked[v] = false;
			_edgeTo[v] = undefined_uint;
		}
		
		_count = 0;
	}
	
	void directed_DFS_multi::setGraph(const digraph& g)
	{
		_allocate(g);
	}
	
	void directed_DFS_multi::operator()(const uint& s)
	{
		_reset();
		dfs(s);
	}
	
	void directed_DFS_multi::operator()(const verteces_ar& sources)
	{
		_reset();
		for(int s = 0; s < sources.size(); s++)
			dfs(sources.get(s));
	}
	
	void directed_DFS_multi::dfs(const uint& v)
	{
		_count++;
		_marked[v] = true;
		stu_citer begin, end, w;
		p_g->adj(v, begin, end);
		for(w = begin; w != end; w++)
			if(_marked[*w] == false)
				dfs(*w);
	}
	
	bool directed_DFS_multi::marked(const uint& v) const
	{
		return _marked[v];
	}
}
