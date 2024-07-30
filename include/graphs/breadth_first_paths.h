#ifndef BREADTH_FIRST_PATHS
#define BREADTH_FIRST_PATHS

#include "_graphs.h"
#include "base_graph.h"
#include "containers.h"

namespace graphs
{
	/*
	 * Breadth-first search:
	 * Shortest path algorithm, alternative to the dfs algorithm.  Also establish whether a path even exists.
	 * 
	 * Starting with the constructor provided vertex s, put it on the queue.
	 * 
	 * In the main loop:
	 * 		pop a vertex off the queue.
	 * 		For each of its unmarked vertices,
	 * 			_edgeTo[w] = v;		// Establish the v->w edge.
	 * 			_marked[w] = true;
	 * 
	 * 			queue.enqueue(w);
	 * O(V+E).
	*/
	
	class breadth_first_paths
	{
	public:
		
		breadth_first_paths() = delete;
		breadth_first_paths(const breadth_first_paths&) = delete;
		breadth_first_paths(breadth_first_paths&&) = delete;
		breadth_first_paths& operator=(const breadth_first_paths&) = delete;
		breadth_first_paths& operator=(breadth_first_paths&&) = delete;
		
		breadth_first_paths(const base_graph&, uint);
		~breadth_first_paths() { delete[] _marked; delete[] _edgeTo; }
		
		bool hasPathTo(uint v) const { if(_marked == nullptr) return false; else return _marked[v]; }
		void operator()(const base_graph&, uint);
		path pathTo(const uint& s, const uint& v) const;
		
	private:
		void bfs(const base_graph&, uint);
		
		bool* _marked = nullptr;
		uint* _edgeTo = nullptr;
		uint _size = 0;
	};
}

#endif
