#include "expand_path.h"

namespace utilities
{
	using namespace std;
	
	std::string ExpandPath::operator()(std::string& path)
	{
		return expand(path);
	}
	
	std::string  ExpandPath::operator()(const char* str, std::size_t size)
	{
		std::string rval = std::string(str, size);
		return expand(rval);
	}

	std::string ExpandPath::operator()(std::string&& path)
	{
		return expand(path);
	}
	
	
	// Given a path starting with '~' or a '.', expand to absolute path.
	
	std::string ExpandPath::expand(std::string& path)
	{
		// A ~ requires us to expand,
		if(path[0] == '~')
		{
			char** env = environ;
			const string search_str = "HOME=";
			int offset = search_str.size();
			size_t result = 0;
			
			while(*env != nullptr)
			{
				string path_expanded = string(*env);
				result = path_expanded.rfind(search_str, 0); 
				
				// "HOME=" should be present only once and the result should be located at the 0th character
				if(result != string::npos && result == 0)
					return path_expanded.substr(result + offset, path_expanded.size());
				
				env++;
			}
			
			return string("");
		}
		// but so would a '.'.
		else if(path[0] == '.')
		{
			char* temp = new char[255];
			char* result = getcwd(temp, 255);
			
			if(result == nullptr)
			{
				int error = errno;
				cerr << "ExpandPath::expand():  Received error = " << error << " after the call to getcwd.  Returning." << endl;
				delete[] temp;
				return string("");
			}
			else
			{	
				path.assign(temp);
				delete[] temp;
				return path;
			}
		}
		else
			return path;
	}
}
