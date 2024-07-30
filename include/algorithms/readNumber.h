#ifndef READ_NUMBER_H
#define READ_NUMBER_H

#include <string>
#include <sstream>

#include <stdlib.h>		// Needed for atoi() and strtol().
#include <limits.h>		// INT_MIN, INT_MAX, LONG_MIN, LONG_MAX.

#include <float.h>

#include "nfa.h"

using namespace std;

namespace algorithms
{
	class readNumber
	{
	public:
		
		union number
		{
			double real64;
			long int64;
			float real32;			// These are reserved for future versions.
			int int32;
			char byte;
			
			void reset() { this->real64 = 0.0; }
		};
		
		enum token : char { NONE=0, NATURAL, REAL, LONG, DOUBLE };
	private:
		// Regular expression for a fully qualified real.
		const string _double_re = "(-?[[:digit:]]+.?[[:digit:]]*((e|E)-?[[:digit:]]{1,3})?)";
		// Regular expression for a 64-bit signed integer.
		const string _long_re = "(-?[[:digit:]]{1,20})";
		
		NFA* _dre = nullptr;		// Pointer to RE for double.
		NFA* _lre = nullptr;		// Pointer to RE for integer
		char* p_digits = nullptr;	// Stores the digits returned by the conversion functions, strtol(), and strtod().
		string::const_iterator c;
		
		int _conversion_error = 0;
		token tokenType = NONE;
		number result = {0.0};
		
		// As longCharLength < doubleCharLength, we'll use doubleCharLength for the length of the p_digits array.
		static constexpr const int longCharLength = 20;
		// sign + 1 + '.' + (52 bit mantissa = 16 base10 digits) + 'e|E' + sign + 3 exponent digits.
		static constexpr const int doubleCharLength = 25;
		static constexpr const int base_10 = 10;
		
		token _getToken(const string& numstr);
		void _setToken(const string& numstr);
		bool _setRealToken(const string& numstr);
		bool _setIntToken(const string& numstr);
		bool _isRealToken(const string& numstr);
		bool _isIntToken(const string& numstr);
		
	public:
		
		readNumber();
		~readNumber();
		
		readNumber(const readNumber&) = delete;
		readNumber(readNumber&&) = delete;
		readNumber& operator=(const readNumber&) = delete;
		readNumber& operator=(readNumber&&) = delete;
		
		double getDouble() const { return result.real64; }
		long getLong() const { return result.int64; }
		
		readNumber::token operator()(const string&);
		number getNumber();
	};
}

#endif
