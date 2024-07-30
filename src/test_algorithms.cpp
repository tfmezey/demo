#include <string>
#include <iostream>
#include <iomanip>
#include "algorithms.h"
#include "graphs.h"

/*
 * This function reads integral and real tokens from a file, where the two numerial types
 * is defined via a regular expression defined by the algorithms::NFA class.  The numerical
 * tokens are evaluated and displayed.
 * 
 * The file test-re.txt describes an edge weighted digraph and has the following format:
 * 	First line is an integer, representing the number of vertices of the digraph.
 * 	The second line represents the number of edges present in the digraph.
 * 	Each subsequent line describes an edge, consisting of:
 * 		a vertex,
 * 		an "->" operator,
 * 		the vertex being pointed to,
 * 		and the real valued weight of the edge.
*/

using namespace std;

int test_read_tokens_RE()
{
	using namespace std;
	using namespace algorithms;
	
	string filename = "test-cases/test-re.txt";
	
	cout << "Reading file " << filename << endl;
	
	string delimiters = " ";
	int _V = 0;
	int _E = 0;
	int n1 = 0;
	int n2 = 0;
	double r = 0.0;
	int line_count = 0;
	
	using Type = read_tokens::tokenType;
	Type result = Type::NONE;
	
	read_tokens rn(filename, delimiters);
	if(rn.ready() == false)
	{
		cerr << "Failed rading file " << filename << " or initialzing the RE-s." << endl;
		return -1;
	}
	
	result = rn.nextToken();
	if(result == Type::INT)
	{
		_V = rn.getIntToken();
		cout << "V = " << _V << endl;
	}
	else
	{
		cerr << "Failed to read in V." << endl;
		return -1;
	}
	
	line_count++;
	result = rn.nextToken();
	if(result == Type::INT)
	{
		_E = rn.getIntToken();
		cout << "E = " << _E << endl;
	}
	else
	{
		cerr << "Failed to read in E." << endl;
		return -1;
	}
	
	line_count++;
	
	while(rn.eof() == false)
	{
		// For each line, read an integer, the "->" operator, another integer and a real.
		result = rn.nextToken();
		if(result != Type::INT)
		{
			cerr << "Line " << line_count << ":  Failed reading first edge vertex." << endl;
			return -1;
		}
		else
			n1 = rn.getIntToken();
		
		result = rn.nextToken();
		if(result != Type::OPERATOR)
		{
			cerr << "Line " << line_count << ":  Failed reading '->'" << endl;
			return -1;
		}
		
		result = rn.nextToken();
		if(result != Type::INT)
		{
			cerr << "Line " << line_count << ":  Failed reading second edge vertex." << endl;
			return -1;
		}
		else
			n2 = rn.getIntToken();
		
		result = rn.nextToken();
		if(result != Type::REAL)
		{
			cerr << "Line " << line_count << ":  Failed reading edge weight." << endl;
			return -1;
		}
		else
			r = rn.getRealToken();
		
		cout << n1 << ", " << n2 << ", weight = " << std::fixed << std::setprecision(2) << r << endl;
		
		line_count++;
	}
	
	cout << endl;
	
	return 0;
}


