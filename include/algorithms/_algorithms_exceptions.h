#ifndef _ALGORITHMS_EXCEPTIONS_H
#define _ALGORITHMS_EXCEPTIONS_H

#include <exception>

namespace algorithms
{
	class InvalidTokenCharacterException : public std::exception
	{
		virtual const char* what() const throw()
		{
			return "Illegal character read in token.";
		}
	};
}

#endif
