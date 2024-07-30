#include "nfa.h"

namespace algorithms
{
	const string NFA::pattern = "[[:misc:]][[:alpha:]][[:ALPHA:]][[:ascii:]][[:digit:]][[:hex:]][[:alphanum:]]";
	const string::const_iterator NFA::misc_b = NFA::pattern.begin();
	const string::const_iterator NFA::misc_e = NFA::misc_b + 10;
	const string::const_iterator NFA::alph_b = NFA::misc_e;
	const string::const_iterator NFA::alph_e = NFA::alph_b + 11;
	const string::const_iterator NFA::ALPH_b = NFA::alph_e;
	const string::const_iterator NFA::ALPH_e = NFA::ALPH_b + 11;
	const string::const_iterator NFA::ascii_b = NFA::ALPH_e;
	const string::const_iterator NFA::ascii_e = NFA::ascii_b + 11;
	const string::const_iterator NFA::num_b = NFA::ascii_e;
	const string::const_iterator NFA::num_e = NFA::num_b + 11;
	const string::const_iterator NFA::hex_b = NFA::num_e;
	const string::const_iterator NFA::hex_e = NFA::hex_b + 9;
	const string::const_iterator NFA::alphanum_b = NFA::hex_e;
	const string::const_iterator NFA::alphanum_e = NFA::pattern.end();
	
	NFA::NFA(const string& regexp) noexcept
	{
		if(_re_sanity_check(regexp) == false)
			return;
		
		_re.assign(regexp);
		
		_ops = new containers::stack<uint>;
		_orstack = new containers::stack<uint>;
		
		_fpp = new FP[TEST_FUNCTIONS];
		// Set up the character class function pointer array:
		// misc, lowercase, uppercase, ascii, numeric, hex, alphanumeric
		_fpp[0] = &NFA::_isMisc;
		_fpp[1] = &NFA::_isAlpha;
		_fpp[2] = &NFA::_isALPHA;
		_fpp[3] = &NFA::_isASCII;
		_fpp[4] = &NFA::_isDigit;
		_fpp[5] = &NFA::_isHex;
		_fpp[6] = &NFA::_isAlphaNum;
		
		_ranges = new arar;
		
		// Transform user provided RE operators/keywords into internally accepted RE operators.
		_process_re(_re);
		_M = _corrected_re.length();
		p_g = new digraph(_M+1);					// Accepting states between [0, _M]

		_build_graph();
		_V = p_g->V();
		_E = p_g->E();
		
		p_dfs = new directed_DFS_multi(*p_g);
		_match = new arui(_V);
		_reachable_states = new arui(_V);
		_initial_reachable_states = new arui(_V);
		
		// Get states reachable from the RE start state.  This is needed by ALL calculations.
		(*p_dfs)(0);
		for(uint v = 0; v < _V; v++)
			if(p_dfs->marked(v) == true)
				_initial_reachable_states->add(v);
		
		_initialized = true;
	}
	
	NFA::~NFA() noexcept
	{
		delete p_g;
		delete _ops;
		delete _match;
		delete _reachable_states;
		delete _initial_reachable_states;
		delete p_dfs;
		delete[] _fpp;
		delete _orstack;
		delete _ranges;
	} 
	
	bool NFA::_process_re(const string& re)
	{
		char c = 0;
		_rangecount = 0;
		
		// Process RE for character classes, '+', '?' and ranges.
		for(int i = 0; i < re.length() - 1; i++)
		{
			c = re[i];
			
			if(re[i] == _escape)
			{
				_corrected_re.push_back(_escape);
				_corrected_re.push_back(re[i+1]);
				i++;
				continue;
			}
			
			if(re[i] == '(')
			{
				_corrected_re.push_back('(');
				
				// Do we have a user defined, perhaps complemented, character class as well?
				if(i+3 != re.length() and (re[i+2] == '-' || re[i+3] == '-'))
				{
					char b = 0;
					char e = 0;
					bool complement = false;
					int complement_sign = 0;
					int rangeID = _rangestart + _rangecount;
					
					if(re[i+1] == '^')
					{
						b = re[i+2];
						e = re[i+4];
						complement = true;
						complement_sign = 1;
					}
					else
					{
						b = re[i+1];
						e = re[i+3];
					}
					
					if(_create_user_defined_set(b, e, rangeID, complement) == false)
					{
						cerr << "Invalid range:  " << re[i+1] << "-" << re[i+3] << endl;
						i += 4 + complement_sign;
						goto Quit;
					}
					else
					{
						_rangecount++;
						_corrected_re.push_back(rangeID);
						_corrected_re.push_back(')');
						i += 4 + complement_sign;
					}
				}
			}
			// Test for character classes
			else if(re[i] == '[' and re[i+1] == '[')
			{
				uint size = 0;
				charclass result;
				bool test = _getClass(re.begin()+i, re.end(), result, size);
				if(test == false)
				{
					cerr << "Could not find valid character class at " << i << endl;
					goto Quit;
				}
				else
				{
					if(_DEBUG)
						cout << "Found " << print_charclass(result) << " at " << i << endl;
					
					_corrected_re.push_back(result);
					i += size;
				}
			}
			// One or more applied to a character
			else if(re[i-1] != ')' and  c == '+')
			{
				// The operand to '+' has already been added to corrected_re.  So, add another along with a '*'.
				_corrected_re.push_back(_corrected_re.back());
				_corrected_re.push_back('*');
			}
			else if(c == ')')
			{
				_corrected_re.push_back(c);
				// One or more applied to parenthesized group, ending at k, inclusive.
				if(re[i+1] == '+')
				{
					// Add parenthesized group: corrected_re[LP, k] + '*'
					_group.clear();
					int j = 0;
					int RP_count = 0;
					int LP_count = 0;
					for(j = _corrected_re.length() - 1; j >= 0; j--)
					{
						if(_corrected_re[j] == ')')
							RP_count++;
						
						if(_corrected_re[j] == '(' and ++LP_count == RP_count)
							break;
					}
					// We need to copy corrected_re[j, length()-1] to group.
					for(int n = j; n <= _corrected_re.length()-1; n++)
						_group.push_back(_corrected_re[n]);
					
					// One copy of the group has already been added, so add the final + '*':
					_corrected_re += _group;
					_corrected_re.push_back('*');
					i++;        // Skip the '+'
				}
				
				// Range applied to a group
				else if(re[i+1] == '{')
				{
					// We know that there exists a '}', so find it.
					uint n1 = 0, n2 = 0, temp = 0, range_str_length = 0, range_index = i+2;
					for(int k = i; k < re.length(); k++)
					{
						if(re[k] == '}')
							break;
						
						range_str_length++;
					}
					
					if(_isDigit(re[range_index]) == false)
					{
						cerr << "Invalid range provided starting at " << i + 1 << endl;
						goto Quit;
					}
					
					temp = re[range_index] - '0';
					if(_isDigit(re[range_index+1]) == true)
					{
						n1 = temp*10 + re[range_index+1] - '0';
						range_index += 2;
					}
					else
					{
						n1 = temp;
						range_index++;
					}
					
					// Skip any separation characters.
					for(uint k = range_index; k < range_index + range_str_length; k++)
					{
						if(_isDigit(re[k]) == true)
							break;
						
						range_index++;
					}
					
					temp = re[range_index] - '0';
					if(_isDigit(re[range_index+1]) == true)
					{
						n2 = temp*10 + re[range_index+1] - '0';
						range_index += 2;
					}
					else
					{
						n2 = temp;
						range_index++;
					}
					
					if(n2 < n1 or (n1 == 0 and n2 == 0) or n2 > maxRange)
					{
						cerr << "Invalid range provided, at " << i << endl;
						goto Quit;
					}
					
					// We need to propagate back one group.  Unfortunately, corrected_re already contains the
					// group, so we need to do surgery.
					_group.clear();
					int j = 0;
					int RP_count = 0;
					int LP_count = 0;
					for(j = _corrected_re.length() - 1; j >= 0; j--)
					{
						if(_corrected_re[j] == ')')
							RP_count++;
						
						if(_corrected_re[j] == '(' and ++LP_count == RP_count)
							break;
					}
					// We need to copy corrected_re[j, length()-1] to group.
					for(int n = j; n <= _corrected_re.length()-1; n++)
						_group.push_back(_corrected_re[n]);
					// Now erase the group already present in corrected_re:
					for(int l = 0; l < _group.length(); l++)
						_corrected_re.pop_back();
					
					// {0, n2} case.
					if(n1 == 0)
					{
						// Then add a starting '(', followed by the OR separated group concatenations, and the final ')'.
						_corrected_re.push_back('(');
						_corrected_re.push_back('|');
						for(int n = 1; n <= n2; n++)
						{
							for(int p = 1; p <= n; p++)
							{
								for(int q = 0; q < _group.length(); q++)
									_corrected_re.push_back(_group[q]);
							}
							
							if(n < n2)
								_corrected_re.push_back('|');
						}
						_corrected_re.push_back(')');
						i += range_str_length;
					}
					// {n1>0, n2} case.
					else
					{
						// This is one less than the minimum repetitions of the group, as corrected_re already contains a copy.
						for(int n = 1; n < n1; n++)
							_corrected_re += _group;
						
						_corrected_re.push_back('(');
						_corrected_re.push_back('|');
						
						for(int n = 0; n < n2 - n1; n++)
						{
							for(int p = 0; p <= n; p++)
								_corrected_re += _group;
							
							if(n < n2 - n1 - 1)
								_corrected_re.push_back('|');
						}
						
						_corrected_re.push_back(')');
						i += range_str_length;
					}
				}
				else if(re[i+1] == '?')
				{
					// Zero or one, applied to a group.
					_group.clear();
					int j = 0;
					int RP_count = 0;
					int LP_count = 0;
					for(j = _corrected_re.length() - 1; j >= 0; j--)
					{
						if(_corrected_re[j] == ')')
							RP_count++;
						
						if(_corrected_re[j] == '(' and ++LP_count == RP_count)
							break;
					}
					
					// We need to copy corrected_re[j, length()-1] to group
					for(int n = j; n <= _corrected_re.length()-1; n++)
						_group.push_back(_corrected_re[n]);
					
					// Now erase the group already present in corrected_re:
					for(int l = 0; l < _group.length(); l++)
						_corrected_re.pop_back();

					_corrected_re.push_back('(');
					_corrected_re.push_back('|');
					_corrected_re += _group;
					_corrected_re.push_back(')');
					i++;
				}
			}
			else if(c == '{')
			{
				// Range {n1,n2} applied to an individual character.
				// In case of a character class, retrieve char from _corrected_re.
				char temp_char = _corrected_re.back();		
				// We know that there exists a '}', so find it.
				uint n1 = 0, n2 = 0, temp = 0, range_str_length = 0, range_index = i+1;
				for(int k = i; k < re.length(); k++)
				{
					if(re[k] == '}')
						break;
					
					range_str_length++;
				}
				
				if(_isDigit(re[range_index]) == false)
				{
					cerr << "Invalid range provided starting at " << i + 1 << endl;
					goto Quit;
				}
				
				temp = re[range_index] - '0';
				if(_isDigit(re[range_index+1]) == true)
				{
					n1 = temp*10 + re[range_index+1] - '0';
					range_index += 2;
				}
				else
				{
					n1 = temp;
					range_index++;
				}
				
				// Skip any separation characters.
				for(uint k = range_index; k < range_index + range_str_length; k++)
				{
					if(_isDigit(re[k]) == true)
						break;
					
					range_index++;
				}
				
				temp = re[range_index] - '0';
				if(_isDigit(re[range_index+1]) == true)
				{
					n2 = temp*10 + re[range_index+1] - '0';
					range_index += 2;
				}
				else
				{
					n2 = temp;
					range_index++;
				}
				
				if(n2 < n1 or (n1 == 0 and n2 == 0))
				{
					cerr << "Invalid range provided, at " << i << endl;
					goto Quit;
				}

				if(n1 == 0)
				{
					_corrected_re.pop_back();
					_corrected_re.push_back('(');
					_corrected_re.push_back('|');
					for(int n = 1; n <= n2; n++)
					{
						for(int p = 1; p <= n; p++)
							_corrected_re.push_back(temp_char);
						
						if(n < n2)
							_corrected_re.push_back('|');
					}
					_corrected_re.push_back(')');
					i += range_str_length;
				}
				else
				{
					// The first copy of character is already in _corrected_re.
					for(int n = 1; n < n1; n++)
						_corrected_re.push_back(temp_char);
					
					_corrected_re.push_back('(');
					_corrected_re.push_back('|');
					for(int n = 0; n < n2 - n1; n++)
					{
						for(int p = 0; p <= n; p++)
							_corrected_re.push_back(temp_char);
						
						if(n < n2 - n1 - 1)
							_corrected_re.push_back('|');
					}
					
					_corrected_re.push_back(')');
					i += range_str_length;
				}
			}
			else if(c == '?')
			{
				// One or more applied to an individual character.
				// In case of a character class, use _corrected_re.
				char temp = _corrected_re.back();		
				_corrected_re.pop_back();
				_corrected_re.push_back('(');
				_corrected_re.push_back('|');
				_corrected_re.push_back(temp);
				_corrected_re.push_back(')');
			}
			// Any other character gets copied.
			else
				_corrected_re.push_back(re[i]);
		}
		
		// Last character from re.
		_corrected_re.push_back(re[re.length() - 1]);
		
		Quit:
		
		_ops->clear();
		
		return false;
	}
	
	bool NFA::_getClass(string::const_iterator b, string::const_iterator e, charclass& result, uint& size)
	{
		size = 0;
		if(_substr_equals(hex_b, hex_e, b, e) == true)
		{
			result = charclass::hex;
			size = 8;
			return true;
		}
		else if(_substr_equals(misc_b, misc_e, b, e) == true)
		{
			result = charclass::misc;
			size = 9;
			return true;
		}
		else if(_substr_equals(num_b, num_e, b, e) == true)
		{
			result = charclass::digit;
			size = 10;
			return true;
		}
		else if(_substr_equals(alph_b, alph_e, b, e) == true)
		{
			result = charclass::lcase;
			size = 10;
			return true;
		}
		else if(_substr_equals(ALPH_b, ALPH_e, b, e) == true)
		{
			result = charclass::ucase;
			size = 10;
			return true;
		}
		else if(_substr_equals(ascii_b, ascii_e, b, e) == true)
		{
			result = charclass::ascii;
			size = 10;
			return true;
		}
		else if(_substr_equals(alphanum_b, alphanum_e, b, e) == true)
		{
			result = charclass::alphanum;
			size = 14;
			return true;
		}
		
		return false;
	}
	
	bool NFA::_isCharClass(const char& c, charclass& result) const
	{
		if(c >= _class_start and c <= _class_end)
		{
			result = cc[c - _class_start];
			return true;
		}
		else
			return false;
	}
	
	bool NFA::_substr_equals(const stci b, const stci e, const stci strb, const stci stre) const
	{
		// Substring comparison function, used in identifying character classes within _re.
		string::const_iterator pattern_i = b;
		for(string::const_iterator i = strb; i != stre and pattern_i != e; i++)
		{
			// Compare characters between pattern substring [b, e - 1] and test string [strb, stre - 1]
			if(*i != *pattern_i)
				return false;
			
			pattern_i++;
		}
		
		return true;
	}
	
	void NFA::_build_graph()
	{
		char c = 0;
		uint LP = 0;		// store the '(' index once encountered, or once it is popped off the stack at a ')'.
		uint OR = 0;		// store the '|' index once popped off the stack.
		
		for(uint i = 0; i < _M-1; i++)
		{
			c = _corrected_re[i];
			
			if(c == _escape)
			{
				i++;
				continue;
			}
			
			if(c == '(')
			{
				// Push '(' and perform an ε-transition to the next character:  '('-ε->_re[i+1].
				_ops->push(i);
				p_g->addEdge(i, i+1);
			}
			// Kleene closure applied to single character at _re[i-1]:
			else if( c == '*' and _corrected_re[i-1] != ')' )
			{
				// Form the following edges:  '*'-ε->_re[i-1], _re[i-1]-ε->'*' and '*'-ε->_re[i+1].
				p_g->addEdge(i, i-1);
				p_g->addEdge(i-1, i);
				p_g->addEdge(i, i+1);
			}
			// OR expression:
			// We need the right ')' to form the edges.  For now, push '|' onto the ops stack and fully process it once a ')' is reached.
			else if(c == '|')
				_ops->push(i);
			// A ')' can mean the end of an OR expression or the argument of a Kleene closure.
			else if(c == ')')
			{
				// Kleene closure applied to parenthesized group, with ')' at i:
				if(_corrected_re[i+1] == '*')
				{
					// Form the following edges:  '*'-ε->'(', '('-ε->'*' and '*'-ε->_re[i+2].
					LP = _ops->pop();
					p_g->addEdge(i+1, LP);
					p_g->addEdge(LP, i+1);
					p_g->addEdge(i+1, i+2);
				}
				// Test for an OR expression between the parentheses, with ')' at i:
				else
				{
					do
					{
						OR = _ops->pop();
						if(_corrected_re[OR] == '|')
						{
							p_g->addEdge(OR, i);
							_orstack->push(OR);
						}
					}
					while(_corrected_re[OR] == '|');
					
					// At this point, if we had any '|', they're on the stack, and the '|'-ε->')' have been completed.
					if(_corrected_re[OR] == '(')
					{
						LP = OR;                        // The '(' was already popped.
						while(_orstack->size() != 0)
						{
							// For every OR expression, ...
							OR = _orstack->pop();
							// form the following edge:  '('-ε->_re[OR+1]:
							p_g->addEdge(LP, OR+1);
						}
					}
				}
				
				// Also add the an edge to the next character:  ')'-ε->_re[i+1]:
				p_g->addEdge(i, i+1);
			}
		}
		
		p_g->addEdge(_M - 1, _M);		// This is the final transition out of the RE.
	}
	
	bool NFA::_re_sanity_check(const string& regexp) const
	{
		// Test for minimum length, equal number opening/closing parentheses, square brackets and braces.
		// Also, test for the begining '(' and and the ending ')'.
		
		if(regexp.length() < 3)			// Smallest RE is '(' + char + ')'
			return false;
		
		int square_brackets = 0;
		int parentheses = 0;
		int braces = 0;
		
		for(int i = 0; i < regexp.length(); i++)
		{
			// Skip any escaped characters.
			if(regexp[i] == _escape)
			{
				i++;     // Escape escape plus what it escapes.
				continue;
			}
			
			if(regexp[i] == '(')
				parentheses++;
			else if(regexp[i] == ')')
				parentheses--;
			else if(regexp[i] == '[')
				square_brackets++;
			else if(regexp[i] == ']')
				square_brackets--;
			else if(regexp[i] == '{')
				braces++;
			else if(regexp[i] == '}')
				braces--;
		}
		
		if(parentheses != 0 || square_brackets != 0 || braces != 0)
			return false;
		
		if(regexp[0] == '(' and regexp[regexp.length() - 1] == ')')
			return true;
		else
			return false;
	}
	
	bool NFA::_input_sanity_check(const string& text) const
	{
		for(int i = 0; i < text.length(); i++)
		{
			char c = text[i];
			if(c != '\\' and i != text.length() - 1)
			{
				char d = text[i+1];
				if(d == '(' or d == ')' or d == '*' or d == '?' or d == '^' or d == '|')
					return false;
			}
		}
		
		return true;
	}
	
	bool NFA::_input_sanity_check(const stci& begin, const stci& end) const
	{
		for(stci i = begin; i != end; i++)
		{
			char c = *i;
			if(c != '\\' and i + 1 != end)
			{
				char d = *(i+1);
				if(d == '(' or d == ')' or d == '*' or d == '?' or d == '^' or d == '|')
					return false;
			}
		}
		
		return true;
	}
	
	bool NFA::_input_sanity_check(const lici& begin, const lici& end) const
	{
		for(lici i = begin; i != end; i++)
		{
			char c = *i;
			if(c != '\\' and i + 1 != end)
			{
				char d = *(i+1);
				if(d == '(' or d == ')' or d == '*' or d == '?' or d == '^' or d == '|')
					return false;
			}
		}
		
		return true;
	}
	
	bool NFA::_isDigit(const char& c) const
	{
		// the digits are in [48, 57] in the ASCII table
		if( c >= 48 && c <= 57 )
			return true;
		else
			return false;
	}
	
	bool NFA::_isAlpha(const char& c) const
	{
		// the a-z are in [97, 122] in the ASCII table
		if( c >= 97 && c <= 122 )
			return true;
		else
			return false;
	}
	
	bool NFA::_isALPHA(const char& c) const
	{
		// the A-Z are in [65, 90] in the ASCII table
		if( c >= 65 && c <= 90 )
			return true;
		else
			return false;
	}
	
	bool NFA::_isASCII(const char& c) const
	{
		// The entire range of ASCII characters.
		if( c >= 32 && c <= 126)
			return true;
		else
			return false;
	}
	
	bool NFA::_isMisc(const char& c) const
	{
		// These are all the other ASCII codes that aren't alphanumerical.
		bool test = c >= 32 && c <= 47;
		test |= c >= 58 && c <= 64;
		test |= c >= 91 && c <= 96;
		test |= c >= 123 && c <= 126;
		
		return test;
	}
	
	bool NFA::_isHex(const char& c) const
	{
		// In ASCII, the digits are in [48, 57].  The A-F are in [65-70], while a-f are in [97,102].
		bool test = c >= 48 && c <= 57;
		test |= c >= 65 && c <= 70;
		test |= c >= 97 && c <= 102;
		
		return test;
	}
	
	bool NFA::_isAlphaNum(const char& c) const
	{
		// [0-9] or [A-Z] or [a-z]
		bool test = c >= 48 && c <= 57;
		test |= c >= 65 && c <= 90;
		test |= c >= 97 && c <= 122;
		
		return test;
	}
	
	std::string NFA::str() const
	{
		using namespace std;
		ostringstream o;
		
		o << "\tV = " << _V << ", E = " << _E << endl;
		
		adj_citer begin, end, it;
		for(uint v = 0; v < _V - 1; v++)
		{
			if(v == _V - 2)
			{
				o << "\tg[" << v << "] = accepting" << endl;
				continue;
			}
			else
				o << "\tg[" << v << "] = ";
			
			p_g->adj(v, begin, end);
			for(it = begin; it != end; it++)
				o << (*it) << " ";
			
			o << endl;
		}
		
		o << "\tg[accepting]";
		
		return o.str();
	}
	
	std::string NFA::str_symbol() const
	{
		using namespace std;
		ostringstream o;
		
		o << _V << endl << _E << endl;
		
		adj_citer begin, end, it;
		for(uint v = 0; v < _V - 1; v++)
		{
			if(v == _V - 2)
			{
				o << "g[" << v << "] = accepting" << endl;
				continue;
			}
			else
				o << "g[" << v << ", " << _corrected_re[v] << "] = ";
			
			p_g->adj(v, begin, end);
			for(it = begin; it != end; it++)
				o << _corrected_re[v] << "->" << _corrected_re[(*it)] << " ";
			
			o << endl;
		}
		
		o << "g[accepting]" << endl;
		
		return o.str();
	}
	
	string NFA::print_charclass(const charclass& cc) const
	{
		ostringstream o;
		switch(cc)
		{
			case digit:
				o << "numeric"; break;
			case lcase:
				o << "lower case"; break;
			case ucase:
				o << "upper case"; break;
			case ascii:
				o << "ascii"; break;
			case misc:
				o << "misc"; break;
			case hex:
				o << "hex"; break;
			default:
				o << "unknown character class";
		}
		
		return o.str();
	}
	
	void NFA::print_status() const
	{
		if(_initialized == false)
			return;
		
		uint count = 0;
		for(string::const_iterator i = _corrected_re.begin(); i != _corrected_re.end(); i++)
		{
			char c = *i;
			charclass type;
			if(_isCharClass(c, type) == true)
				cout << count << ":  " << print_charclass(type) << endl;
			else
				cout << count << ":  " << c << endl;
			
			count++;
		}
		
		if(_ranges->size() != 0)
		{
			for(int i = 0; i < _ranges->size(); i++)
			{
				ascii_range ar = _ranges->get(i);
				cout << ar.rangeID - _rangestart  << ":  " << ar.begin << ", " << ar.end << " ";
				if(ar.complement == true)
					cout << "true" << endl;
				else
					cout << "false" << endl;
			}
		}
	}
	
	bool NFA::_create_user_defined_set(const char& begin, const char& end, const int& rangeID, const bool& complement)
	{
		// [0-9] < [A-Z] < [a-z].  Three ranges, instead of one, courtesy of ASCII.
		
		if(end < begin)
			return false;
		
		if(_isDigit(end) == true)
		{
			_ranges->add(ascii_range(rangeID, begin, end, complement));
			return true;
		}
		else if(_isALPHA(end) == true)
		{
			if(_isDigit(begin) == true)
			{
				_ranges->add(ascii_range(rangeID, begin, '9', complement));
				_ranges->add(ascii_range(rangeID, 'A', end, complement));
				return true;
			}
			else
			{
				_ranges->add(ascii_range(rangeID, begin, end, complement));
				return true;
			}
		}
		else if(_isAlpha(end) == true)
		{
			if(_isDigit(begin) == true)
			{
				_ranges->add(ascii_range(rangeID, begin, '9', complement));
				_ranges->add(ascii_range(rangeID, 'A', 'Z', complement));
				_ranges->add(ascii_range(rangeID, 'a', end, complement));
				return true;
			}
			else if(_isALPHA(begin) == true)
			{
				_ranges->add(ascii_range(rangeID, begin, 'Z', complement));
				_ranges->add(ascii_range(rangeID, 'a', end, complement));
				return true;
			}
			else
			{
				_ranges->add(ascii_range(rangeID, begin, end, complement));
				return true;
			}
		}
		
		return false;
	}
	
	bool NFA::_isMatch(const char& text, const char& re)
	{
		// Test for a direct match, user ranges, or built-in character classes:
		charclass result;
		
		if(re < 0)
		{
			// Test for character class:
			if(_isCharClass(re, result) == true)
				return (this->*_fpp[result - _class_start])(text);
			else
			{
				// We can only have ranges at this point, or an error.  The rangeID = re.
				arar_citer begin, end, r;
				_ranges->get_citers(begin, end);
				bool test = false;
				bool complement = false;
				for(r = begin; r != end; r++)
				{
					if(r->rangeID != re)
						continue;
					
					if(r->complement == true)
						complement = true;
					
					if(text >= r->begin and text <= r->end)
					{
						test = true; 
						break;
					}
				}
				
				return complement ? !test : test;
			}
			
			return false;
		}
		else
			return text == re;
	}
	
	bool NFA::operator()(const string& text)
	{
		return recognizes(text);
	}
	
	bool NFA::operator()(const stci& begin, const stci& end)
	{
		return recognizes(begin, end);
	}
	
	bool NFA::operator()(const lici& begin, const lici& end)
	{
		return recognizes(begin, end);
	}
	
	bool NFA::recognizes(const string& text)
	{
		if(_sanitize == true)
		{
			if(_input_sanity_check(text) == false)
				return false;
		}
		
		arui_citer begin, end, v;
		*_reachable_states = *_initial_reachable_states;
		
		for(int i = 0; i < text.length(); i++)
		{
			_match->clear();
			char t = text[i];
			// Try to obtain matches from the first and subsequently, the previous character's reachable states.
			_reachable_states->get_citers(begin, end);
			for(v = begin; v != end; v++)
			{
				if(*v == _M)
					continue;
				
				if(_isMatch(t, _corrected_re[*v]) == true)
					_match->add(*v+1);
			}

			// Generate the reachable states of the current character, needed for the next iteration.
			_reachable_states->clear();
			(*p_dfs)(*_match);
			for(int v = 0; v < _V; v++)
			{
				if(p_dfs->marked(v) == true)
					_reachable_states->add(v);
			}
		}
		
		// With ALL the text characters having been consumed, check for success:
		_reachable_states->get_citers(begin, end);
		for(v = begin; v != end; v++)
			if(*v == _M)
				return true;
		
		return false;
	}
	
	bool NFA::recognizes(const stci& begin, const stci& end)
	{
		if(_sanitize == true)
		{
			if(_input_sanity_check(begin, end) == false)
				return false;
		}
		
		arui_citer vbegin, vend, v;
		*_reachable_states = *_initial_reachable_states;
		
		for(stci i = begin; i != end; i++)
		{
			_match->clear();
			char t = *i;
			// Try to obtain matches from the first and subsequently, the previous character's reachable states.
			_reachable_states->get_citers(vbegin, vend);
			for(v = vbegin; v != vend; v++)
			{
				if(*v == _M)
					continue;
				
				if(_isMatch(t, _corrected_re[*v]) == true)
					_match->add(*v+1);
			}
			
			// Generate the reachable states of the current character, needed for the next iteration.
			_reachable_states->clear();
			(*p_dfs)(*_match);
			for(int v = 0; v < _V; v++)
			{
				if(p_dfs->marked(v) == true)
					_reachable_states->add(v);
			}
		}
		
		// With ALL the text characters having been consumed, check for success:
		_reachable_states->get_citers(vbegin, vend);
		for(v = vbegin; v != vend; v++)
			if(*v == _M)
				return true;
		
		return false;
	}
	
	bool NFA::recognizes(const lici& begin, const lici& end)
	{
		if(_sanitize == true)
		{
			if(_input_sanity_check(begin, end) == false)
				return false;
		}

		arui_citer vbegin, vend, v;
		*_reachable_states = *_initial_reachable_states;
		
		for(lici i = begin; i != end; i++)
		{
			_match->clear();
			char t = *i;
			// Try to obtain matches from the first and subsequently, the previous character's reachable states.
			_reachable_states->get_citers(vbegin, vend);
			for(v = vbegin; v != vend; v++)
			{
				if(*v == _M)
					continue;
				
				if(_isMatch(t, _corrected_re[*v]) == true)
					_match->add(*v+1);
			}
			
			// Generate the reachable states of the current character, needed for the next iteration.
			_reachable_states->clear();
			(*p_dfs)(*_match);
			for(int v = 0; v < _V; v++)
			{
				if(p_dfs->marked(v) == true)
					_reachable_states->add(v);
			}
		}
		
		// With ALL the text characters having been consumed, check for success:
		_reachable_states->get_citers(vbegin, vend);
		for(v = vbegin; v != vend; v++)
			if(*v == _M)
				return true;
		
		return false;
	}
}

