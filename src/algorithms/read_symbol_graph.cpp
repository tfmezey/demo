#include "read_symbol_graph.h"

namespace algorithms
{
	std::string read_symbol_graph::name(const uint& v) const
	{
		if(_initialized == false)
			return "";
		else
			return p_keys->get(v);
	}
	
	graphs::graph read_symbol_graph::G() const
	{
		if(_initialized == false)
			return graphs::graph();
		else
			return *((graph*)p_g);
	}
	
	graphs::digraph read_symbol_graph::DG() const
	{
		if(_initialized == false)
			return graphs::digraph();
		else
			return *((digraph*)p_g);
	}
	
	int read_symbol_graph::index(const std::string& s) const
	{
		uint result = 0;
		
		if(_initialized == false)
			return result;
		
		p_st->get(s, result);
		
		return result;
	}
	
	bool read_symbol_graph::contains(const std::string& s) const
	{
		if(p_st == nullptr)
			return false;
		else
			return p_st->contains(s);
	}
	
	void read_symbol_graph::reset()
	{
		// Return the class to the state it had after creation.  Progressive initialization is left to our
		// _readfile() member function.
		
		_initialized = false;
		_allocated = false;
		
		delete p_g;
		delete p_keys;
		delete p_st;
		
		p_g = nullptr;
		p_keys = nullptr;
		p_st = nullptr;
	}
	
	void read_symbol_graph::readfile(const std::string& filename, const char& delim, const bool& useDigraph)
	{
		_useDigraph = useDigraph;
		_readfile_str(filename, delim);
	}
	
	void read_symbol_graph::_readfile_str(const std::string& filename, const char& delim)
	{
		/* Initialize our graph object:
		 * 		1)	Read string symbols and place them in the symbol table, facilitating the string -> index mapping.
		 * 		2)	Using the symbol table, initialize the keys array to facilitate the index -> string mapping.
		 * 		3)	With the bi-directional mapping available, fill the graph object with edges, and implicit vertices.
		 */
	
		using namespace graphs;
		using namespace algorithms;
		reset();
		
		read_tokens in(filename, delim);
		
		if(in.ready() != true)
		{
			cerr << "Failed initializing read_int.  Exiting." << endl;
			return;
		}
		
		p_st = new symbols;
		symbols& st = *p_st;
		
		string str = "";
		int token_count = 0;
		using Type = read_tokens::tokenType;
		Type result = Type::NONE;
		
		while(in.eof() == false)
		{
			result = in.nextToken();
			if(result == Type::EOFTOKEN)
				goto FINISHED_FILE;
			else if(result == Type::STRING)
				str.assign(in.getStrToken());
			else
			{
				cerr << "Line " << token_count << ":  Failed reading token " << token_count + 1 << endl;
				return;
			}
			
			if(st.contains(str) == false)
			{
				st.put(str, st.size());
				token_count++;
			}
		}
		
		FINISHED_FILE:
		
		if(_DEBUG)
			cout << "Completed filling the symbol table with " << token_count << " entries:" << endl;
		
		// Now that we know the size of the symbol table, allocate and populate the keys array.  Note
		// that for graph purposes, this is not needed, except in our str() function.  All graph
		// processing is based on uint values in the graph class, and nothing more.
		token_count = 0;
		p_keys = new values(st.size());
		values& keys = *p_keys;

		for(symbols_iter name = st.begin(); name != st.end(); name++)
		{
			int value = st.get(*name);
			if(_DEBUG)
				cout << "\t" << *name << " -> " << value << endl;
			
			keys.get(value) = *name;
			token_count++;
		}
		
		_V = st.size();
		
		if(_DEBUG)
		{
			cout << endl << "Completed populating the keys array, with " << token_count << " entries:" << endl;
			for(int i = 0; i < keys.size(); i++)
				cout << "\t" << i << ":  " << keys.get(i) << endl;
			
			cout << endl;
		}

		// and allocate the graph object.
		if(_useDigraph == true)
			p_g = new digraph(st.size());
		else
			p_g = new graph(st.size());
		
		if(p_g->ready() == false)
			return;
		
		// We allocated everything.
		_allocated = true;

		if(_DEBUG)
			cout << "g is " << p_g->ready() << endl;

		// Second Pass -- Read string tokens from each line of the file. Obtain the vertex (symbol table index)
		// of the first vertex, and form edges from it to the vertices of the other tokens.
		
		in.reset();
		uint v, w;
		while(in.eof() == false)
		{			
			// Read first token from line, ...
			result = in.nextToken();
			if(result == Type::EOFTOKEN)
				goto SECOND_PASS_FINISH;
			else if(result == Type::STRING)
			{
				str.assign(in.getStrToken());
				v = st.get(str);
				
				if(_DEBUG)
					cout << "obtained " << str << ", ";
			}
			else
			{
				cerr << "Second Pass:  Failed reading token " << token_count + 1 << endl;
				return;
			}
			
			// then read rest of the tokens, getting their location w in the symbol table,
			// to form an edge with the first symbol's location v.
			do
			{
				result = in.nextToken();
				if(result == Type::EOFTOKEN)
					goto SECOND_PASS_FINISH;
				else if(result == Type::STRING)
				{
					str.assign(in.getStrToken());
					w = st.get(str);
					
					if(_DEBUG)
						cout << "obtained " << str << ", ";
				}
				else
				{
					cerr << "Second Pass:  Failed reading token " << token_count + 1 << endl;
					return;
				}
				
				if(_DEBUG)
					cout << "adding edge:  " << v << "->" << w << endl;
				
				if(_useDigraph == true)
					((digraph*)p_g)->addEdge(v, w);
				else 
					((graph*)p_g)->addEdge(v, w);
			}
			while(in.eol() == false);
		}
		
		SECOND_PASS_FINISH:
		
		// We populated the graph object with our edges.  We're finally done initializing.
		_V = p_g->V();
		_E = p_g->E();
		_initialized = true;
		
		return;
	}

	std::string read_symbol_graph::str() const
	{
		if(_initialized == false)
			return std::string("");
		
		ostringstream o;
		symbols& st = *p_st;
		string indentation = "     ";
		
		values& keys = *p_keys;
		adj_citer begin, end;
		for(symbols_citer name = st.cbegin(); name != st.cend(); name++)
		{
			o << indentation << *name << ":" << endl << "\t";
			int value = st.get(*name);
			p_g->adj(value, begin, end);
			for(adj_citer it = begin; it != end; it++)
				o << keys.get(*it) << " -> ";

			o << endl;
		}
		
		return o.str();
	}
}
