#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <float.h>		// DBL_MAX, DBL_MIN
#include <stdlib.h>
#include <limits.h>		// INT_MIN, INT_MAX, LONG_MIN, LONG_MAX.

#include "_algorithms_exceptions.h"

namespace algorithms
{
	extern bool _DEBUG;
	
	using uchar = unsigned char;
	using uint = unsigned int;
	using ulong = unsigned long;
	
	extern double _inititalize_real_to;
	extern uint _inititalize_uint_to;
	extern char _inititalize_char_to;
	
	static constexpr const char undefined_char = SCHAR_MIN;
	static constexpr const int undefined_int = INT_MAX-1;
	static constexpr const uint undefined_uint = UINT_MAX-1;
	static constexpr const long undefined_long = ULONG_MAX-1;
	static constexpr const uint default_size = 10;
	static constexpr const float inf_f = FLT_MAX - 1.0;
	static constexpr const double inf = DBL_MAX - 1.0;
	static constexpr const float neginf_f = FLT_MIN + 1.0;
	static constexpr const double neginf = DBL_MIN + 1.0;
	
	// Used by file_read and derived classes
	enum process_file_state : uchar {OK, READ_FILE_ERROR, REACHED_EOF, OVERFLOW, UNDERFLOW, INVALID};
	
}

#endif
