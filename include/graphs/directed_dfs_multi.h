#ifndef DIRECTED_DEPTH_FIRST_SEARCH_MULTI
#define DIRECTED_DEPTH_FIRST_SEARCH_MULTI

#include "_graphs.h"
#include "digraph.h"
#include "containers.h"

namespace graphs
{
	using namespace containers;
	
	/*
	 * Depth First Search, multiple source vertices, digraph variant, meant for
	 * the algorithms::NFA class.
	 * 
	 * For each vertex provided to operator()(path), run DFS on it.
	 * Querry results with marked().
	 * 
	 * Create a bool array _marked[], and a vertex array _edgeTo[], for each vertex 
	 * in the path passed to operator()().
	 * Starting with vertex s,
	 * 		Mark vertex having been visited as true
	 * 		Obtain its adjacency vertices
	 * 			For each of their unmarked vertices w
	 * 				Mark their _edgeTo[] entry, designating that the edge v->w exists.
	 * 				Call dfs(g, w)
	 * 
	 * O(sum(degrees of the vertices), as ALL vertices are visited.
	 */
	
	class directed_DFS_multi
	{
		using vertices_ar = containers::array<uint>;
		using vertices_fl = containers::forward_list<uint>;
		
	public:
		
		directed_DFS_multi() = delete;
		directed_DFS_multi(const directed_DFS_multi&) = delete;
		directed_DFS_multi(directed_DFS_multi&&) = delete;
		directed_DFS_multi& operator=(const directed_DFS_multi&) = delete;
		directed_DFS_multi& operator=(directed_DFS_multi&&) = delete;
		
		directed_DFS_multi(const digraph& g, const uint& s=0);
		~directed_DFS_multi();
		
		bool marked(const uint&) const;
		int count() const { return _count; }
		
		void setGraph(const digraph&);
		
		void operator()(const uint&);
		void operator()(const vertices_ar&);
		void operator()(const vertices_fl&);
		
	private:
		
		void dfs(const uint&);
		void _allocate(const digraph&);
		void _reset();

		bool* _marked = nullptr;
		uint* _edgeTo = nullptr;
		digraph* p_g = nullptr;

		uint _count = 0;
		uint _V = 0;
		bool _initialized = 0;
	};
}

#endif
