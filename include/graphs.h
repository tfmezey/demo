#ifndef GRAPHS_H
#define GRAPHS_H

#include <math.h>
#include "containers.h"

// Graphs header files meant for clients.
#include "graphs/acyclic_lp.h"
#include "graphs/acyclic_sp.h"
#include "graphs/base_graph.h"
#include "graphs/breadth_first_paths.h"
#include "graphs/depth_first_order.h"
#include "graphs/depth_first_search.h"
#include "graphs/directed_dfs_multi.h"
#include "graphs/digraph.h"
#include "graphs/directed_cycle.h"
#include "graphs/edge.h"
#include "graphs/edge_weighted_digraph.h"
#include "graphs/edge_weighted_graph.h"
#include "graphs/graph.h"
#include "graphs/topological.h"

namespace graphs
{
	extern bool _DEBUG;
	
	class edge;
	class graph;
	class digraph;
	class edge_weighted_digraph;
	class edge_weighted_graph;
	
	using ewdg = edge_weighted_digraph;
	using ewg = edge_weighted_graph;
	
	// This is the undirected, unweighted variant of adjacency list used by graph and digraph.
	using adj = containers::reverse_list<uint>;	
	using adj_citer = adj::citerator;
	using adj_iter = adj::iterator;
	
	/* 
	 * The following variant of the adjacency list is used exclusively by edge weighted
	 * undirected graphs.
	 * 
	 * In order to save space, we store the edges in an array, and then use a reverse list
	 * of edge pointers for each vertex of the edge.  With integers being 32-bit, addresses
	 * being 64-bit and doubles also being 64-bit, an edge takes up 128-bits.  We thus incurr
	 * a 50% storage savings.
	 */
	using adjpe = containers::reverse_list<edge*>;
	using adjpe_citer = adjpe::citerator;
	using adjpe_iter = adjpe::iterator;

	// This variant of the adjacency list is used by the ewdg class exclusively.
	using adje = containers::reverse_list<edge>;
	using adje_citer = adje::citerator;
	using adje_iter = adje::iterator;
	
	// Arrays of the various adjacency lists.
	using aradj = containers::array<adj>;
	using aradj_citer = aradj::citerator;
	using aradj_iter = aradj::iterator;
	
	using aradjpe = containers::array<adjpe>;
	using aradjpe_citer = aradjpe::citerator;
	using aradjpe_iter = aradjpe::iterator;
	
	using aradje = containers::array<adje>;
	using aradje_citer = aradje::citerator;
	using aradje_iter = aradje::iterator;
	
	// These are the uint containers, primarly used to return paths or list of vertices.
	
	using arui = containers::array<uint>;
	using arui_citer = arui::citerator;
	using arui_iter = arui::iterator;
	using quui = containers::queue<uint>;
	using stui = containers::stack<uint>;
	
	using flui = containers::forward_list<uint>;
	using flui_iter = flui::iterator;
	using flui_citer = flui::const_iterator;
	
	using rlui = containers::reverse_list<uint>;
	using rlui_iter = rlui::iterator;
	using rlui_citer = rlui::const_iterator;
	
	// Edge weighted graph aliases
	using are = containers::array<edge>;
	using are_citer = are::citerator;
	using are_iter = are::iterator;
	
	using fle = containers::forward_list<edge>;
	using fle_citer = fle::citerator;
	using fle_iter = fle::iterator;
	
	using rle = containers::reverse_list<edge>;
	using rle_citer = rle::citerator;
	using rle_iter = rle::iterator;
	
	using ste = containers::stack<edge>;
	using que = containers::queue<edge>;
	using mpqe = containers::IMiPQ<edge>;
	using minpqe = containers::minPQ<edge>;
	
	// Paths and results containers:
	using path = rlui;
	using path_citer = path::citerator;
	using path_iter = path::iterator;
	using rpath = flui;										// Returned by the depth_first_search and breadth_first_paths.
	using rpath_citer = rpath::citerator;
	using rpath_iter = rpath::iterator;
	using epath = rle;										// Path returned as edges, from acyclic_LP/SP etc.
	using epath_citer = epath::citerator;
	using epath_iter = epath::iterator;
	using erpath = rle;										// Path returned as edges, from acyclic_LP/SP etc.
	using erpath_citer = erpath::citerator;
	using erpath_iter = erpath::iterator;
		
	using cyclic_path = rpath;								// Returned by directed_cycle.
	using cyclic_path_iter = cyclic_path::iterator;
	using cyclic_path_citer = cyclic_path::citerator;
	using cyclic_path_e = rle;
	using cyclic_path_e_citer = cyclic_path_e::citerator;
	using cyclic_path_e_iter = cyclic_path_e::iterator;
	using neg_cyclic_path = rle;							// Returned by the bellman ford shorted path class.
	using neg_cyclic_path_citer = neg_cyclic_path::citerator;
	using neg_cyclic_path_iter = neg_cyclic_path::iterator;
	
	using pre = path;										// Returned by the DFS order, pre each dfs() call.
	using pre_citer = pre::citerator;
	using pre_iter = pre::iterator;
	using post = path;										// Returned by the DFS order, after a return by dfs() call.
	using post_citer = post::citerator;
	using post_iter = post::iterator;
	using rpost = rpath;									// Returned by the DFS order, reverse of post.
	using rpost_citer = rpost::citerator;
	using rpost_iter = rpost::iterator;
	
	using mst = fle;										// Returned by lazy_prim_mst and kruskal_mst.
	using mst_citer = mst::citerator;
	using mst_iter = mst::iterator;
	
}

#endif
