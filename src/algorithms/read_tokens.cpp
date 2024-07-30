#include "read_tokens.h"

namespace algorithms
{
	const string read_tokens::_double_re = "(-?[[:digit:]]+.?[[:digit:]]*((e|E)-?[[:digit:]]{1,3})?)";
	const string read_tokens::_integer_re = "(-?[[:digit:]]+)";
	const string read_tokens::_operator_re = "(->)";
	const int read_tokens::_operator_re_size = _operator_re.size() - 2;
	
	read_tokens::read_tokens(const string& filename, const char& delimiter)
	{
		if(delimiter != 0)
			this->delimiters.push_back(delimiter);
		
		_initialize();
		
		if(_DEBUG)
			printfile();
		
		_file_read_result = (*p_fileinput)(filename, *p_lines);
		p_lines->get_citers(file_begin, file_end);
		
		_dre = new NFA(_double_re);
		_ire = new NFA(_integer_re);
		_ore = new NFA(_operator_re);
		if(_dre->ready() == false or _ire->ready() == false or _ore->ready() == false)
			cerr << "read_tokens():  Failed to initialize one or more of the NFAs." << endl;
		else
			_re_initialized = true;
	}
	
	read_tokens::read_tokens(const string& filename, const string& delimiters)
	{
		if(delimiters.length() != 0)
			this->delimiters += delimiters;
		
		_initialize();
		_file_read_result = (*p_fileinput)(filename, *p_lines);
		
		if(_DEBUG)
			printfile();
		
		p_lines->get_citers(file_begin, file_end);
		
		_dre = new NFA(_double_re);
		_ire = new NFA(_integer_re);
		_ore = new NFA(_operator_re);
		if(_dre->ready() == false or _ire->ready() == false or _ore->ready() == false)
			cerr << "read_tokens():  Failed to initialize one or more of the NFAs." << endl;
		else
			_re_initialized = true;
	}
	
	read_tokens::~read_tokens()
	{
		delete _result_str;
		delete[] p_digits;
		delete p_fileinput;
		delete p_lines;
		delete _result_operator;
		delete _dre;
		delete _ire;
		delete _ore;
	}
	
	void read_tokens::_initialize()
	{
		// As doubleCharLength > intCharLength, use it for intergers as well.
		p_digits = new char[doubleCharLength];
		for(int i = 0; i < doubleCharLength; i++)
			p_digits[i] = '\0';
		
		_result_str = new string {};
		_result_operator = new string {};
		p_lines = new arli;
		p_fileinput = new algorithms::file_input();
	}
	
	void read_tokens::reset()
	{
		 _previous_line = 0;
		 _previous_char = 0;
		 result = NONE;
		 _EOL = false;
		 _EOF = false;
		 p_lines->get_citers(file_begin, file_end);
		 current_line = file_begin;
	}
	
	bool read_tokens::_isOperatorToken(li_citer tokenStart, li_citer& tokenEnd, const li_citer& lineEnd)
	{
		if(_EOF == true)
			return false;
		
		result = NONE;
		li_citer c = tokenStart;
			
		if(c+_operator_re_size == lineEnd)		// We need 2 characters for "->"
		{
			tokenEnd = tokenStart;
			result = NONE;
			return false;
		}
		else if(_ore->recognizes(tokenStart, tokenStart + _operator_re_size) == true)
		{
			tokenEnd = tokenStart + _operator_re_size;
			result = OPERATOR;
			return true;
		}
		else
		{
			tokenEnd = tokenStart;
			result = NONE;
			return false;
		}
	}
	
	bool read_tokens::_isIntToken(li_citer tokenStart, li_citer& tokenEnd, const li_citer& lineEnd)
	{
		if(_EOF == true)
			return false;
		
		result = NONE;
		
		uint negative_sign = *tokenStart == '-' ? 1 : 0;
		int charcount = negative_sign;
		for(li_citer l = tokenStart + negative_sign; l != lineEnd; l++)
		{
			switch(*l)
			{
				case '0' ... '9':
				{
					charcount++; break;
				}
				case '.':
				case 'e':
				case 'E':
				{
					// Flag for possible real.
					result = REAL;
					charcount++;
					goto EXIT;
				}
				default:
					goto EXIT;
			}
		}
		
		EXIT:
		
		// A double is a superset to an integer, so we flagged REAL after any non-digit character acceptable by the double RE.  In doing so,
		// we ensured that the token can be claimed as a double, instead of and incorrectly, an integer made from a partial double token.
		if(result != REAL and _ire->recognizes(tokenStart, tokenStart+charcount) == true)
		{
			result = INT;
			tokenEnd = tokenStart + charcount;
			return true;
		}
		else
		{
			tokenEnd = tokenStart;
			result = NONE;
			return false;
		}
	}
	
	bool read_tokens::_isRealToken(li_citer tokenStart, li_citer& tokenEnd, const li_citer& lineEnd)
	{
		if(_EOF == true)
			return false;
		
		result = NONE;
		
		uint negative_sign = *tokenStart == '-' ? 1 : 0;
		int charcount = negative_sign;
		for(li_citer l = tokenStart + negative_sign; l != lineEnd; l++)
		{
			switch(*l)
			{
				case '0' ... '9':
				case 'e':
				case 'E':
				case '.':
				case '-':
				{
					charcount++; break;
				}
				default:
					goto EXIT;
			}
		}
		
		EXIT:
		
		if(_dre->recognizes(tokenStart, tokenStart+charcount) == true)
		{
			result = REAL;
			tokenEnd = tokenStart + charcount;
			return true;
		}
		else
		{
			tokenEnd = tokenStart;
			result = NONE;
			return false;
		}
	}
	
	bool read_tokens::_isStrToken(li_citer tokenStart, li_citer& tokenEnd, const li_citer& lineEnd)
	{
		if(_EOF == true)
			return false;
		
		// The default token type.  Therefore, use as last test case.  Break on delimiters.
		result = STRING;
		char c = 0;
		int charcount = 0;
		
		for(li_citer l = tokenStart; l != lineEnd; l++)
		{
			c = *l;
			if(isDelim(c) == true)
				goto EXIT;
			
			charcount++;
		}
		
		EXIT:
		
		tokenEnd = tokenStart + charcount;
		
		return true;
	}
	
	read_tokens::tokenType read_tokens::_nextToken(li_citer tokenStart, li_citer& tokenEnd, const li_citer& lineEnd)
	{
		/* Private member function performing the task of identifying a token.
		 * Set tokenEnd and return the identified token type.
		 * 
		 * Caller should advance iteration to first non-delimiter character after previous token, if any.
		*/
		
		if(_EOF == true)
			return NONE;
		
		result = NONE;
		li_citer c;

		for(c = tokenStart; c != lineEnd; c++)
		{
			switch(*c)
			{
				case '0' ... '9':
				{
					if(_isIntToken(tokenStart, tokenEnd, lineEnd) == true)
						return result;
					if(_isRealToken(tokenStart, tokenEnd, lineEnd) == true)
						return result;
				}
				case '-':
				{
					if(_isIntToken(tokenStart, tokenEnd, lineEnd) == true)
						return result;
					if(_isRealToken(tokenStart, tokenEnd, lineEnd) == true)
						return result;
					if(_isOperatorToken(tokenStart, tokenEnd, lineEnd) == true)
						return result;
				}
				default:
				{
					if(_isStrToken(tokenStart, tokenEnd, lineEnd) == true)
						return result;
				}
			}
		}
		
		tokenEnd = c;
		
		// We must by default have a string.
		if(result == NONE)
			result = STRING;
		
		return result;
	}
	
	void read_tokens::_setToken(li_citer begin, li_citer end)
	{
		// Pre-empt the client retrieving value of token.
		switch(result)
		{
			case INT:
				_set_int_token(begin, end); break;
			case REAL:
				_set_real_token(begin, end); break;
			case STRING:
				_set_str_token(begin, end); break;
			case OPERATOR:
				_set_op_token(begin, end); break;
			default:
				break;
		}
	}
	
	bool read_tokens::_set_int_token(li_citer begin, li_citer end)
	{
		// Called internally to convert a token into an integer.  Note strtol produces a long.
		errno = 0;
		int i = 0;
		
		_result_int = algorithms::undefined_int;
		for(li_citer c = begin; c < end; i++, c++)
			p_digits[i] = *c;
		p_digits[i] = '\0';
		
		long temp = strtol(p_digits, &endptr, base_10);
		if(errno != 0)
		{
			_conversion_error = errno;
			switch(_conversion_error)
			{
				case ERANGE:
				{
					// With ERANGE, we have either an overflow or underflow.  Note that LONG_MIN and LONG_MAX are 
					// legitimate outside of ERANGE error.
					
					if(temp == LONG_MIN)
						cerr << "strtol():  underflow error occurred" << endl;
					else
						cerr << "strtol():  overflow error occurred" << endl;
					
					break;
				}
				case EINVAL:
					cerr << "strtol():  invalid input" << endl;
				default:
					cerr << "strtol():  Unknown error " << _conversion_error << endl;
			}
			
			return false;
		}
		
		_result_int = (int)temp;
		
		return true;
	}
	
	bool read_tokens::_set_real_token(li_citer begin, li_citer end)
	{
		// Called internally to produce a double from a token.
		double temp = 0;
		errno = 0;
		
		int i = 0;
		_result_int = algorithms::undefined_int;
		for(li_citer c = begin; c < end; i++, c++)
			p_digits[i] = *c;
		p_digits[i] = '\0';
		
		temp = strtod(p_digits, &endptr);
		if(errno != 0)
		{
			_conversion_error = errno;
			switch(_conversion_error)
			{
				case ERANGE:
				{
					// With ERANGE, we have either an overflow or underflow. 
					
					if(temp == FLT_MIN || temp == DBL_MIN)
						cerr << "strtod():  underflow error occurred" << endl;
					else
						cerr << "strtod():  overflow error occurred" << endl;
					
					break;
				}
				default:
					cerr << "strtod():  Unknown error " << _conversion_error << endl;
			}
			
			return false;
		}
		
		_result_real = temp;
		
		return true;
	}
	
	bool read_tokens::_set_str_token(li_citer begin, li_citer end)
	{
		// Called interally to produce a string (default from int/double) from a token.
		*_result_str = "";
		for(li_citer c = begin; c < end; c++)
			_result_str->push_back(*c);
		
		return true;
	}
	
	bool read_tokens::_set_op_token(li_citer begin, li_citer end) 
	{
		// Called internally to produce an operator (for now just arrow) from a token.
		*_result_operator = "";
		_result_operator->push_back(*begin);
		_result_operator->push_back(*(begin+1));
		
		return true;
	}
	
	read_tokens::tokenType read_tokens::nextToken()
	{
		for(current_line = file_begin + _previous_line; current_line != file_end; current_line++)
		{
			_EOL = false;
			result = NONE;
			current_line->get_citers(lbegin, lend);
			c = lbegin + _previous_char;
			while(c != lend)
			{
				// Skip any leading delimiters.
				while(c != lend && isDelim(*c) == true )
				{
					_previous_char++;
					c++;
				}
				
				tokenStart = c;
				result = _nextToken(tokenStart, tokenEnd, lend);
				_setToken(tokenStart, tokenEnd);
				_previous_char += tokenEnd - tokenStart;
				c = tokenEnd;
				
				// Skip any trailing delimiters.
				while(c != lend && isDelim(*c) == true )
				{
					_previous_char++;
					c++;
				}
				
				goto RETURN_TOKEN;
			}
		}
		
		RETURN_TOKEN:
		// We're leaving, but set up for next iteration, and test for EOL and EOF.
		if(c == lend)
		{
			_EOL = true;
			_previous_line++;
			_previous_char = 0;
			
			if(file_begin + _previous_line == file_end)
				_EOF = true;
		}

		return result;
	}
	
	int read_tokens::getIntToken()
	{
		int i = _result_int;
		_result_int = algorithms::undefined_int;
		
		return i;
	}
	
	double read_tokens::getRealToken()
	{
		double r = _result_real;
		_result_real = algorithms::inf;
		
		return r;
	}
	
	string read_tokens::getStrToken()
	{
		string s = *_result_str;
		*_result_str = "";
		
		return s;
	}
	
	string read_tokens::getOpToken()
	{
		string s = *_result_operator;
		*_result_operator = "";
		
		return s;
	}
	
	string read_tokens::str(const tokenType& t) const
	{
		ostringstream o;
		
		switch(t)
		{
			case STRING:
				o << "STRING"; break;
			case INT:
				o << "INT"; break;
			case REAL:
				o << "REAL"; break;
			default:
				o << "NONE"; break;
		}
		
		return o.str();
	}
	
	void read_tokens::printfile() const
	{
		using namespace std;
		
		containers::array<algorithms::line>::const_iterator a_iter;
		algorithms::line::const_iterator l_it;
		
		int linecount = 0;
		for(a_iter = p_lines->cbegin(); a_iter != p_lines->cend(); a_iter++)
		{
			cout << "line " << linecount << ":  ";
			for(l_it = a_iter->cbegin(); l_it != a_iter->cend(); l_it++)
				cout << *l_it;
			
			cout << endl;
			linecount++;
		}
	}
	
	bool read_tokens::isDigit(const char& c) const
	{
		// Digits are in [48, 57] in the ASCII table.
		if( c >= 48 && c <= 57 )
			return true;
		else
			return false;
	}
	
	bool read_tokens::isSign(const char& c) const
	{
		if( c == 43 || c == 45)
			return true;
		else
			return false;
	}
	
	bool read_tokens::isDelim(const char& c) const
	{
		bool result = false;
		for(int i = 0; i < delimiters.length(); i++)
		{
			if(c == delimiters[i])
				return true;
		}
		
		return result;
	}
}
