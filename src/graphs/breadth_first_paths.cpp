#include "breadth_first_paths.h"

namespace graphs
{
	breadth_first_paths::breadth_first_paths(const base_graph& g, uint s)
	{
		uint V = g.V();
		_size = V;
		_marked = new bool[V];
		_edgeTo = new uint[V];
		for(int i = 0; i < V; i++)
		{
			_marked[i] = false;
			_edgeTo[i] = undefined_uint;
		}
		
		bfs(g, s);
	}
	
	void breadth_first_paths::operator()(const base_graph& g, uint s)
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
		
		bfs(g, s);
	}
	
	void breadth_first_paths::bfs(const base_graph& g, uint s)
	{
		adj_iter begin, end, W;
		
		quui q;
		_marked[s] = true;
		q.enqueue(s);
		
		while(q.empty() == false)
		{
			uint v = q.dequeue();
			g.adj(v, begin, end);
			for(W = begin; W != end; W++)
			{
				uint w = *W;
				
				if(_marked[w] == false)			// For every unmarked adjacent vertex,
				{
					_edgeTo[w] = v;				// save last edge on a shortest path,
					_marked[w] = true;			// mark it because path is known,
					q.enqueue(w);				// and add it to the queue.
				}
			}
		}
	}
	
	path breadth_first_paths::pathTo(const uint& s, const uint& v) const
	{
		path result;
		
		if(hasPathTo(v) == false)
			return result;
		
		for(int x = v; x != s; x = _edgeTo[x])
		{
			if(x == undefined_uint)
				break;
			
			result.add(x);
		}
		
		result.add(s);
		
		return result;
	}
	
}
