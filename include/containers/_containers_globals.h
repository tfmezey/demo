#ifndef _CONTAINERS_GLOBALS_H
#define _CONTAINERS_GLOBALS_H

#include "_containers.h"
#include "_allocator.h"

namespace containers
{
	extern bool _DEBUG;
	template <typename T>
	_allocator<T> al;
}

#endif
