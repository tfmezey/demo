#ifndef DEPTH_FIRST_ORDER_H
#define DEPTH_FIRST_ORDER_H

#include "containers.h"
#include "_graphs.h"
#include "digraph.h"
#include "edge.h"
#include "edge_weighted_digraph.h"

namespace graphs
{
	class depth_first_order
	{
		/*
		 * Depth First Search Order:
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
		 * During the call, set up the following order of visited vertices:
		 * 		Pre = matches the dfs calls.
		 * 		Post = matches the order that dfs calls finish.
		 * 		reversePost = the reverse (via a stack) of the Post order.
		*/
		
	public:
		depth_first_order() = delete;
		depth_first_order(const depth_first_order&) = delete;
		depth_first_order(depth_first_order&&) = delete;
		depth_first_order& operator=(const depth_first_order&) = delete;
		depth_first_order& operator=(depth_first_order&&) = delete;
		
		depth_first_order(const digraph&);
		depth_first_order(digraph&&);
		depth_first_order(const edge_weighted_digraph&);
		depth_first_order(edge_weighted_digraph&&);
		~depth_first_order() { delete[] _marked; delete _pre; delete _post; delete _reversePost;};
		
		void operator()(const digraph&);
		void operator()(const edge_weighted_digraph&);
		pre Pre() const { return *_pre; }
		post Post() const { return *_post; }
		rpost ReversePost() const { return *_reversePost; }
		
	private:
		void dfs(const digraph&, const uint&);
		void dfs(const edge_weighted_digraph&, const uint&);
		
		void _initialize(const uint&);
		
		bool* _marked = nullptr;
		pre* _pre = nullptr;
		post* _post = nullptr;
		rpost* _reversePost = nullptr;
		
		uint _size = 0;
	};
}

#endif
