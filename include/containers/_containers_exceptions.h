#ifndef _CONTAINERS_EXCEPTIONS_H
#define _CONTAINERS_EXCEPTIONS_H

#include <exception>

namespace containers
{
	class NoSuchElementException : public std::exception
	{
		virtual const char* what() const throw()
		{
			return "No such element exception occurred.";
		}
	};
	
	class InvalidIndexException : public std::exception
	{
		virtual const char* what() const throw()
		{
			return "Invalid index exception occurred.";
		}
	};
	
	class IllegalArgumentException : public std::exception
	{
		virtual const char* what() const throw()
		{
			return "Illegal argument exception occurred.";
		}
	};
	
	class EmptyContainerException : public std::exception
	{
		virtual const char* what() const throw()
		{
			return "Empty container exception occurred.";
		}
	};
	
	class AllocationException : public std::exception
	{
		virtual const char* what() const throw()
		{
			return "Allocation exception occurrred.";
		}
	};		
	
	class AllocationLimitException : public std::exception
	{
		virtual const char* what() const throw()
		{
			return "Allocation limit exception occurrred.";
		}
	};

	class NullPointerException : public std::exception
	{
		virtual const char* what() const throw()
		{
			return "Null pointer exception occurrred.";
		}
	};
	
	
}

#endif
