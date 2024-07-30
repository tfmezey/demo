#ifndef _GRAPHS_EXCEPTIONS_H
#define _GRAPHS_EXCEPTIONS_H

#include <exception>

namespace graphs
{
	class ClassNotInitializedException : public std::exception
	{
		virtual const char* what() const throw()
		{
			return "Class is not initialized.";
		}
	};
	
	class InvalidIndexException : public std::exception
	{
		virtual const char* what() const throw()
		{
			return "Invalid index exception.";
		}
	};
}

#endif
