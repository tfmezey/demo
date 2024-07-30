#ifndef _CONTAINERS_H
#define _CONTAINERS_H

#include <float.h>

#include "_containers_exceptions.h"

namespace containers
{
	using uchar = unsigned char;
	using uint = unsigned int;
	using ulong = unsigned long;
	
	static constexpr const uint _allocate = 10;
	static constexpr const int undefined_int = -1;
	static constexpr const uint undefined_uint = 4294967295;
	static constexpr const uint default_size = 10;
	static constexpr const double inf = DBL_MAX/2;
	static constexpr const double neginf = DBL_MIN/2;
	
	static constexpr const bool min = true;
	static constexpr const bool max = false;	
	
	static constexpr const uint _max_size = 1024*1024;
}

#endif
