#ifndef DEPTH_FIRST_SEARCH
#define DEPTH_FIRST_SEARCH

#include "_graphs.h"
#include "base_graph.h"
#include "containers.h"

namespace graphs
{
	using namespace std;

	/*
	 * Depth First Search:
	 * 
	 * Create a bool array _marked[], and a vertex array _edgeTo[].
	 * Starting with vertex s,
	 * 		Mark vertex having been visited as true
	 * 		Obtain its adjacency vertices
	 * 			For each of their unmarked vertices w
	 * 				Mark their _edgeTo[] entry, designating that the edge v->w exists.
	 * 				Call dfs(g, w)
	 * 
	 * O(sum(degrees of the vertices), as ALL vertices are visited.
	 * Clients should call hasPathTo(v), to determine whether the constructor provided vertex
	 * s has a path to the vertex v.  If true, obtain the vertices of the path:
	* 		for(uint x = v; x. != x; x = _edgeTo[x])
	* 			stack.push(x);
	* The _edgeTo vertex entries are all assigned the value undefined_uint to terminate this loop.
	*/
	
	class depth_first_search
	{
	public:
		depth_first_search() = delete;
		depth_first_search(const depth_first_search&) = delete;
		depth_first_search(depth_first_search&&) = delete;
		depth_first_search& operator=(const depth_first_search&) = delete;
		depth_first_search& operator=(depth_first_search&&) = delete;
		
		depth_first_search(const base_graph&, const uint&);
		depth_first_search(const base_graph& g, const arui&);
		~depth_first_search() { delete[] _marked; delete[] _edgeTo; }
		
		bool marked(const uint& w) const { return _marked[w]; }
		bool hasPathTo(const uint& v) const { return _marked[v]; }
		int count() const { return _count; }
		void operator()(const base_graph&, const uint&);
		void operator()(const base_graph& g, const arui&);
		void pathTo(path&, const uint&, const uint&);
		
	private:
		void dfs(const base_graph&, const uint&);
		void dfs(const base_graph&, const arui&);
		
		bool* _marked = nullptr;
		uint* _edgeTo = nullptr;
		int _count = 0;
		uint _size = 0;
	};
}

#endif
