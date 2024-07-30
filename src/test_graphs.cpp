#include "algorithms.h"			// We must include this before graphs.h!
#include "graphs.h"
#include "containers.h"
#include "utilities.h"

using namespace std;

int test_acyclic_paths()
{
	using namespace graphs;
	using namespace algorithms;
	using ewdg = graphs::edge_weighted_digraph;
	
	string indentation = "     ";
	string filename = "test-cases/graph.txt";
	algorithms::read_edge_weighted_digraph rewdg(filename);
	if(rewdg.ready() == false)
	{
		cerr << "Failed reading " << filename << endl;
		return -1;
	}
	
	ewdg dg = rewdg.EWDG();
	cout << "Given the following graph:" << endl;
	cout << dg.str() << endl;
	
	graphs::directed_cycle dc(dg);
	if(dc.hasCycle() == true)
	{
		cerr << "which has a cycle: " << endl << "\t";
		cyclic_path cycle = dc.getCycle();
		int i = 0;
		for(auto i = cycle.cbegin(); i != cycle.cend(); i++)
		{
			cout << *i;
			if(i == cycle.cend() - 1)
				cout << ", ";
		}
		cout << endl << ", which makes it ineligible for the shortest path algorithm, forcing us to return with error." << endl;
		return -1;
	}
	else
		cout << "which is a DAG, ";

	uint s = 3;
	cout << "the longest paths from " << s << " are:" << endl;
	acyclic_LP aclp(dg, s);
	
	epath_citer begin, end, e;
	for(int v = 0; v < dg.V(); v++)
	{
		if(aclp.hasPathTo(v) == false)
			continue;
		
		epath Path = aclp.getPathTo(v);
		cout << indentation << s << " -> " << v << " (" << std::fixed << std::setprecision(2) << aclp.distance(v) << "): ";
		Path.get_citers(begin, end);
		for(e = begin; e != end; e++)
			cout << e->str() << " ";
		cout << endl;
	}   
	
	s = 2;
	acyclic_SP acsp(dg, s);
	cout << "while the shortest paths from " << s << " are:" << endl;
	for(int v = 0; v < dg.V(); v++)
	{
		if(acsp.hasPathTo(v) == false)
			continue;
		
		epath Path = acsp.getPathTo(v);
		cout << indentation << s << " -> " << v << " (" << std::fixed << std::setprecision(2) << acsp.distance(v) << "): ";
		Path.get_citers(begin, end);
		for(e = begin; e != end; e++)
			cout << e->str() << " ";
		cout << endl;
	}       
	
	return 0;
}
