#include "depth_first_order.h"

namespace graphs
{
	depth_first_order::depth_first_order(const digraph& dg)
	{
		uint V = dg.V();
		_initialize(V);
		
		for(int v = 0; v < V; v++)
			if(_marked[v] == false)
				dfs(dg, v);
	}
	
	depth_first_order::depth_first_order(digraph&& dg)
	{
		uint V = dg.V();
		_initialize(V);
		
		for(int v = 0; v < V; v++)
			if(_marked[v] == false)
				dfs(dg, v);
	}
	
	depth_first_order::depth_first_order(const edge_weighted_digraph& dg)
	{
		uint V = dg.V();
		_initialize(V);
		
		for(int v = 0; v < V; v++)
			if(_marked[v] == false)
				dfs(dg, v);
	}
	
	depth_first_order::depth_first_order(edge_weighted_digraph&& dg)
	{
		uint V = dg.V();
		_initialize(V);
		
		for(int v = 0; v < V; v++)
			if(_marked[v] == false)
				dfs(dg, v);
	}
	
	void depth_first_order::operator()(const edge_weighted_digraph& dg)
	{
		uint V = dg.V();
		_initialize(V);
		
		for(int v = 0; v < V; v++)
			if(_marked[v] == false)
				dfs(dg, v);
	}
	
	void depth_first_order::_initialize(const uint& V)
	{
		_size = V;
		if(_marked != nullptr)
			delete[] _marked;
			
		_marked = new bool[V];
		for(int v = 0; v < V; v++)
			_marked[v] = false;
		
		if(_pre != nullptr)
			_pre->clear();
		else
			_pre = new pre;
		
		if(_post != nullptr)
			_post->clear();
		else
			_post = new post;
		
		if(_reversePost != nullptr)
			_reversePost->clear();
		else
			_reversePost = new rpost;
	}
	
	void depth_first_order::operator()(const digraph& dg)
	{
		uint V = dg.V();
		_initialize(V);
		
		for(int v = 0; v < V; v++)
			if(_marked[v] == false)
				dfs(dg, v);
	}
	
	void depth_first_order::dfs(const digraph& dg, const uint& v)
	{
		// _pre = order of dfs calls
		// _post = order in which the vertices are done, that is when dfs(v) finishes.
		// _reversePost = mirror immage of _post.
		
		_pre->add(v);
		_marked[v] = true;
		
		adj_citer begin, end;
		dg.adj(v, begin, end);
		for(adj_citer W = begin; W != end; W++)
		{
			uint w = *W;
			if(_marked[w] == false)
				dfs(dg, w);
		}
		
		_post->add(v);
		_reversePost->add(v);
	}

	void depth_first_order::dfs(const edge_weighted_digraph& ewdg, const uint& v)
	{
		// _pre = order of dfs calls
		// _post = order in which the vertices are done, that is when dfs(v) finishes.
		// _reversePost = mirror immage of _post.
		
		_pre->add(v);
		_marked[v] = true;
		
		adje_citer begin, end, E;
		ewdg.adj(v, begin, end);
		for(E = begin; E != end; E++)
		{
			edge const& e = *E;
			uint w = e.to();
			if(_marked[w] == false)
				dfs(ewdg, w);
		}
		
		_post->add(v);
		_reversePost->add(v);
	}
}
