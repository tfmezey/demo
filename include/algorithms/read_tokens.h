#ifndef READ_NUMBERS_H
#define READ_NUMBERS_H

#include <stdlib.h>		// Needed for atoi() and strtol().
#include <limits.h>		// INT_MIN, INT_MAX, LONG_MIN, LONG_MAX.

#include <cfloat>		// Somehow the double min/max ranges used by strtod() ended up in the C++ <cfloat>.
#include <string>

#include "_algorithms.h"
#include "line.h"
#include "file_input.h"
#include "nfa.h"

namespace algorithms
{
	using namespace std;
	class NFA;

	// Read integer, real number, operator (currently on a "->"), and string tokens from a text file.
	
	class read_tokens
	{
	public:
		
		using uchar = unsigned char;
		using uint = unsigned int;
		using arli = containers::array<algorithms::line>;
		using arli_iter = containers::array<algorithms::line>::iterator;
		using arli_citer = arli::const_iterator;
		using li_iter = algorithms::line::iterator;
		using li_citer = algorithms::line::const_iterator;
		
		enum tokenType : uchar { NONE, STRING, REAL, INT, OPERATOR, EOFTOKEN };
		
		// Prevent implicit default and copy construction.
		read_tokens() = delete;
		read_tokens(const read_tokens&) = delete;
		read_tokens& operator=(const read_tokens&) = delete;
		
		read_tokens(const std::string&, const char&);
		read_tokens(const std::string&, const std::string&);
		~read_tokens();
		
		// Resets parsing of read file.
		void reset();

		// API:
		tokenType nextToken();
		int getIntToken();
		double getRealToken();
		string getStrToken();
		string getOpToken();
		
		// Client helpers:
		bool eof() const { return _EOF; }
		bool eol() const { return _EOL; }
		string str(const tokenType&) const;
		bool ready() const { return _file_read_result == 0 and _re_initialized; }
		void printfile() const;
		uint size() const { return p_lines->size(); }
		
	private:
		
		void _initialize();

		// Work horses:
		tokenType _nextToken(li_citer, li_citer&, const li_citer&);
		bool _isIntToken(li_citer, li_citer&, const li_citer&);
		bool _isOperatorToken(li_citer, li_citer&, const li_citer&);
		bool _isRealToken(li_citer, li_citer&, const li_citer&);
		bool _isStrToken(li_citer, li_citer&, const li_citer&);
		void _setToken(li_citer, li_citer);
		bool _set_int_token(li_citer, li_citer);
		bool _set_real_token(li_citer, li_citer);
		bool _set_str_token(li_citer, li_citer);
		bool _set_op_token(li_citer, li_citer);
		bool isDigit(const char&) const;
		bool isSign(const char&) const;
		bool isDelim(const char&) const;
		
		// Stateful iteration variables:
		uint _previous_line = 0;
		uint _previous_char = 0;
		tokenType result = NONE;
		arli_citer file_begin, file_end, current_line;
		li_citer lbegin, lend, tokenStart, tokenEnd, c;
		
		// Our results:
		int _result_int = 0;
		double _result_real = 0.0;
		string* _result_str = nullptr;
		string* _result_operator = nullptr;
		int _conversion_error = 0;
		bool _EOL = false;
		bool _EOF = false;
		
		// Token conversion variables:
		char* p_digits = nullptr;
		char* endptr = nullptr;
		static constexpr const int longCharLength = 10;
		// sign + 1 + '.' + (52 bit mantissa = 16 base10 digits) + 'e|E' + sign + 3 exponent digits.
		static constexpr const int doubleCharLength = 25;
		static constexpr const int base_10 = 10;

		static const string _double_re;
		static const string _integer_re;
		static const string _operator_re;
		NFA* _dre = nullptr;		// Pointer to RE for double.
		NFA* _ire = nullptr;		// Pointer to RE for integer.
		NFA* _ore = nullptr;		// Pointer to RE for "->" operator.
		static const int _operator_re_size;
		bool _re_initialized = false;
		
		// File I/O variables:
		int _file_read_result = 0;
		string delimiters = {" "};
		
		algorithms::file_input* p_fileinput = nullptr;
		arli* p_lines = nullptr;
	};
}

#endif
