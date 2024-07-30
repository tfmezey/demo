#include "readNumber.h"

namespace algorithms
{
	readNumber::readNumber()
	{
		_dre = new NFA(_double_re);
		_lre = new NFA(_long_re);
		
		p_digits = new char[doubleCharLength];
		for(int i = 0; i < doubleCharLength; i++)
			p_digits[i] = '\0';
	}
	
	readNumber::~readNumber()
	{
		delete _dre;
		delete _lre;
		delete[] p_digits;
	}
	
	readNumber::token readNumber::_getToken(const string& numstr)
	{
		// Private member function performing the task of identifying a numerical token.
		// Tokens must start wither either the minus sign, or an digit character.
		
		tokenType = NONE;
		c = numstr.cbegin();
		
		switch(*c)
		{
			case '0' ... '9':
			case '-':
			{
				if(_isIntToken(numstr) == true)
					break;
				if(_isRealToken(numstr) == true)
					break;
			}
			default:
				break;
		}
		
		return tokenType;
	}
	
	bool readNumber::_isIntToken(const string& numstr)
	{
		tokenType = NONE;
		
		uint negative_sign = numstr[0] == '-' ? 1 : 0;
		for(c = numstr.cbegin() + negative_sign; c != numstr.cend(); c++)
		{
			switch(*c)
			{
				case '0' ... '9':
					break;
				case '.':
				case 'e':
				case 'E':
				{
					// Flag for possible real.
					tokenType = REAL;
					goto EXIT;
				}
				default:
					goto EXIT;
			}
		}
		
		EXIT:
		
		// A double is a superset to an integer, so we flagged REAL after any non-digit character acceptable by the double RE.  In doing so,
		// we ensured that the token can be claimed as a double, instead of and incorrectly, an integer made from a partial double token.
		if(tokenType != REAL and _lre->recognizes(numstr) == true)
		{
			tokenType = NATURAL;
			return true;
		}
		else
		{
			tokenType = NONE;
			return false;
		}
	}
	
	bool readNumber::_isRealToken(const string& numstr)
	{
		tokenType = NONE;
		
		uint negative_sign = numstr[0] == '-' ? 1 : 0;
		for(c = numstr.cbegin() + negative_sign; c != numstr.cend(); c++)
		{
			switch(*c)
			{
				case '0' ... '9':
				case 'e':
				case 'E':
				case '.':
				case '-':
					break;
				default:
					goto EXIT;
			}
		}
		
		EXIT:
		
		if(_dre->recognizes(numstr) == true)
		{
			tokenType = REAL;
			return true;
		}
		else
		{
			tokenType = NONE;
			return false;
		}
	}
	
	void readNumber::_setToken(const string& numstr)
	{
		// Pre-empt the client retrieving value of token.
		switch(tokenType)
		{
			case NATURAL:
				_setIntToken(numstr); break;
			case REAL:
				_setRealToken(numstr); break;
			default:
				break;
		}
	}
	
	bool readNumber::_setIntToken(const string& numstr)
	{
		// Called internally to convert a token into an integer.  Note strtol produces a long.
		errno = 0;
		int i = 0;
		
		result.int64 = algorithms::undefined_long;
		for(c = numstr.cbegin(); c < numstr.cend(); i++, c++)
			p_digits[i] = *c;
		p_digits[i] = '\0';
		
		long temp = strtol(p_digits, nullptr, base_10);
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
		
		tokenType = LONG;
		result.int64 = temp;
		
		return true;
	}
	
	bool readNumber::_setRealToken(const string& numstr)
	{
		// Called internally to produce a double from a token.
		double temp = 0;
		errno = 0;
		
		int i = 0;
		result.int64 = algorithms::undefined_int;
		for(c = numstr.cbegin(); c < numstr.cend(); i++, c++)
			p_digits[i] = *c;
		p_digits[i] = '\0';
		
		temp = strtod(p_digits, nullptr);
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
		
		
		tokenType = DOUBLE;
		result.real64 = temp;
		
		return true;
	}
	
	readNumber::token readNumber::operator()(const string& numstr)
	{
		tokenType = NONE;
		result.reset();
		if(numstr.empty() or numstr.size() > doubleCharLength)
			return tokenType;
		
		tokenType = _getToken(numstr);		// Determine which token type we have, and ...
		if(tokenType != NONE)
			_setToken(numstr);				// set the appropriate numerical return value in result.
			
		return tokenType;
	}
}
