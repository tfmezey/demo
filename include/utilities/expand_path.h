#ifndef EXPAND_PATH_H
#define EXPAND_PATH_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <string>

extern char** environ;

namespace utilities
{
	// Given a path starting with '~' or a '.', expand to absolute path.
	
	class ExpandPath
	{
	public:
		ExpandPath() {};
		~ExpandPath() {};
		
		std::string operator()(std::string&);
		std::string operator()(std::string&&);
		std::string operator()(const char*, std::size_t);
		
	private:
		std::string expand(std::string&);
	};
}

#endif
