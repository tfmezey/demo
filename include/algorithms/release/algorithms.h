#ifndef ALGORITHMS_H
#define ALGORITHMS_H

/*
 * Note that due to compiler and language limitation, the order of including header
 * files matters.  As a consequence, this file must be included before its dependecy
 * "graphs.h" in all client projects.
*/

#include "graphs.h"

#include "algorithms/_algorithms_exceptions.h"
#include "algorithms/_algorithms.h"
#include "algorithms/file_input.h"
#include "algorithms/line.h"
#include "algorithms/nfa.h"
#include "algorithms/readNumber.h"
#include "algorithms/read_edge_weighted_digraph.h"
#include "algorithms/read_edge_weighted_graph.h"
#include "algorithms/read_int_graph.h"
#include "algorithms/read_symbol_graph.h"
#include "algorithms/read_tokens.h"
#include "algorithms/weighted_union_find.h"

namespace algorithms
{
	extern bool _DEBUG;
	
	class file_input;
	class line;
	class NFA;
	class read_edge_weighted_digraph;
	class read_edge_weighted_graph;
	class read_int_graph;
	class read_symbol_graph;
	class read_tokens;
	class readNumber;
	class weighted_union_find;
	
	// Used by file_read and derived classes
	enum process_file_state : uchar {OK, READ_FILE_ERROR, REACHED_EOF, OVERFLOW, UNDERFLOW, INVALID};
}

#endif

