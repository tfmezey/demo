#ifndef NFA_H
#define NFA_H

#include <string>
#include <iostream>
#include <sstream>

#include "_algorithms.h"
#include "line.h"

#include "graphs.h"
#include "containers.h"

namespace algorithms
{
	using namespace graphs;
	
	/*
	 * NFA for a regular expression.
	 * 
	 * Input = std::string.  Expectation, minimum 3 characters, with a starting and ending parenthesis.
	 * 
	 * Match transition:
	 * A, where the regular expression A, whose first character is at i.
	 * 
	 * Note that all '(', ')' or '*' have a forward ε-transition to the next character!
	 * 
	 * ε-transitions:
	 * Single character Kleene closure:
	 * A*, where 'A' at i-1, '*' at i
	 * ε-transition from '*' back to 'A', from 'A' forward to '*' and from '*' to next character
	 * addEdge(i-1, i); addEdge(i, i-1); addEdge(i, i+1);
	 * 
	 * Kleene closure of parenthetical expression:
	 * ( ... )*, where '(' at LP, ')' at i, '*' at i+1
	 * ε-transitions from '(' to '*', from '*' back to '(' and from '*' to next character
	 * addEdge(LP, i+1); addEdge(i+1, LP); addEdge(i, i+1)
	 * 
	 * or-expression, always within parenthetes:
	 * ( ... | ... ), where '(' at LP, '|' at OR, ')' at i
	 * ε-transitions from opening parenthesis '(' to character after '|', from '|' to closing parenthesis ')'
	 * addEdge(lp, OR+1); addEdge(OR, i)
	 * 
	 * Character classes:  [[:misc:]], [[:alpha:]], [[:ALPHA:]], [[:ascii:]], [[:digit:]], and [[:hex:]].
	 * Single character or group ranges (...){n1,n2} or A{n1,n2}, with 0 <= n1,n2 <= 99.  Note that the separation
	 * of the numbers is arbtirary and not enforced.
	 * Alphanumeric user defined ranges, with range = {0-9,A-Z,a-z}.  These can be complemented with '^'.
	 * Multi-OR ( ... | ...| ... )
	 * One or more operator '+':  A+, or (...)+
	 * Zero or one: A?, or (...)?
	 * Escape sequence via '\' in console, or via '\\' in source file.
	 * 
	 * The algorithm constructs a directed graph, and uses a multiple source depth first search algorithm to obtain
	 * the available states at each character of the expanded regular expression state.  Once all characters are
	 * consumed, the algorithm determines whether the end of the RE has been reached, marking a successful match.
	 * 
	 * TODO -- UTF-8 characters.
	*/
	
	class NFA
	{
		class ascii_range
		{
		public:
			ascii_range() {};
			ascii_range(int id, char b, char e, bool c) { rangeID = id; begin = b ; end = e; complement = c; }
			int rangeID = 0;
			char begin = 0;
			char end = 0;
			bool complement = false;
		};
		
		using arui = containers::array<uint>;
		using arui_citer = arui::const_iterator;
		using stci = std::string::const_iterator;
		using arar = containers::array<ascii_range>;
		using arar_citer = arar::const_iterator;
		using lici = algorithms::line::const_iterator;
		
		// Set up the character class enumeration, which will be used as an index within the character class function pointer array.
		enum charclass : char {misc = -99, lcase, ucase, ascii, digit, hex, alphanum};
		static constexpr const charclass cc[] = {misc, lcase, ucase, ascii, digit, hex, alphanum};
		static constexpr const charclass _class_start = misc;
		static constexpr const charclass _class_end = alphanum;
		static constexpr const char _rangestart = -80;
		static constexpr const char _escape = '\\';
		static constexpr const char _anychar = '.';
		static const string pattern;					// 54, 54 bytes, vs 5*15 = 75 bytes.
		
		static const string::const_iterator misc_b;
		static const string::const_iterator misc_e;
		static const string::const_iterator alph_b;
		static const string::const_iterator alph_e;
		static const string::const_iterator ALPH_b;
		static const string::const_iterator ALPH_e;
		static const string::const_iterator ascii_b;
		static const string::const_iterator ascii_e;
		static const string::const_iterator num_b;
		static const string::const_iterator num_e;
		static const string::const_iterator hex_b;
		static const string::const_iterator hex_e;
		static const string::const_iterator alphanum_b;
		static const string::const_iterator alphanum_e;
		
		static constexpr const uint TEST_FUNCTIONS = 7;
		static constexpr const uint maxRange = 99;
		
		uint _V = 0;									// The number of vertices of the graph representation of the RE.
		uint _E = 0;									// The number of edges of the graph representation of the RE.
		uint _M = 0;									// The length of the corrected/expanded RE string.
		uint _rangecount = 0;
		bool _initialized = false;
		
		arar* _ranges = nullptr;						// Array of ascii_range objects.
		containers::stack<uint>* _ops = nullptr;		// Used for storing '('.
		containers::stack<uint>* _orstack = nullptr;	// Used for storing '|' during RE processing (building _corrected_re).
		arui* _match = nullptr;							// Storage for the matching states.
		arui* _reachable_states = nullptr;				// Storage for reachable states.
		arui* _initial_reachable_states = nullptr;		// As the RE[0] states are the same for all input strings, store them here.
		digraph* p_g = nullptr;							// This will hold the digraph representation of the RE.
		directed_DFS_multi* p_dfs = nullptr;			// We'll use this object and p_g to determine dfs paths needed by the algorithm.
		
		string _re = "";
		string _group = "";
		string _corrected_re = "";
		
		using FPP = bool (NFA::**)(const char&) const;
		using FP = bool (NFA::*)(const char&) const;
		FPP _fpp = nullptr;								// Function pointer array for the character class functions.
		
		bool _sanitize = true;
		
		bool _substr_equals(const stci, const stci, const stci, const stci) const;
		bool _process_re(const string&);
		bool _getClass(string::const_iterator, string::const_iterator, charclass&, uint&);
		bool _isCharClass(const char&, charclass&) const;
		
		bool _input_sanity_check(const string&) const;
		bool _input_sanity_check(const stci&, const stci&) const;
		bool _input_sanity_check(const lici&, const lici&) const;
		bool _re_sanity_check(const string&) const;
		void _build_graph();
		bool _create_user_defined_set(const char&, const char&, const int&, const bool&);
		
		// Character class functions.
		bool _isMatch(const char&, const char&);
		bool _isDigit(const char&) const;
		bool _isAlpha(const char&) const;
		bool _isALPHA(const char&) const;
		bool _isASCII(const char&) const;
		bool _isMisc(const char&) const;
		bool _isHex(const char&) const;
		bool _isAlphaNum(const char&) const;
		
	public:
		NFA(const string&) noexcept;
		~NFA() noexcept;
		
		string str() const;
		string str_symbol() const;
		string print_charclass(const charclass& cc) const;
		void print_status() const;
		void sanitize(const bool& Sanitize) { _sanitize = Sanitize; }
		bool ready() const { return _initialized == true; }
		bool recognizes(const string&);
		bool operator()(const string&);
		bool recognizes(const stci&, const stci&);
		bool operator()(const stci&, const stci&);
		bool recognizes(const lici&, const lici&);
		bool operator()(const lici&, const lici&);
	};
}

#endif
