#ifndef FILE_INPUT_H
#define FILE_INPUT_H

#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>		// Needed for fstat(), which we need to determine file size.
#include "string.h"			// Needed for strerror().

#include <iostream>
#include <string>

#include "_algorithms.h"
#include "containers.h"
#include "line.h"

namespace algorithms
{
	/*
	 * Read in a text file, one line at a time and store contents in a user provided containers::array<algorithms::line>
	 * array.
	*/
	
	class file_input
	{
	public:
		file_input();
		~file_input();
		
		// Prevent copy construction.
		file_input(const file_input&) = delete;
		file_input& operator=(const file_input&) = delete;

		// Main interface function.  Client provides filename reference and a containers::array<algorithms::line>& reference.
		int operator()(const std::string&, containers::array<algorithms::line>&);
		
	private:
		bool open_file(int);
		bool open_file(int, int);
		bool read_file(containers::array<algorithms::line>&);
		void close_file();
		int get_line(int);
		void error_cleanup(int);
		
		char* _buffer = nullptr;
		std::string filename = {};
		const int FileFlag = O_RDONLY;
		int fd = 0;
		bool ready = false;
		
		static const size_t READ_BUFFER_SIZE = 1024;		// Size of the buffer to hold one line.
	};
	
}

#endif
