#include "file_input.h"

namespace algorithms
{
	using namespace std;

	file_input::file_input()
	{
		 _buffer = new char[READ_BUFFER_SIZE];
		 
		 for(int i = 0; i < READ_BUFFER_SIZE; i++)
			_buffer[i] = 0;
	}
	
	file_input::~file_input()
	{
		delete[] _buffer;
	}
	
	// This is called internally, when class is ready.
	bool file_input::open_file(int flag)
	{
		int flag_test = O_RDONLY | O_WRONLY | O_RDWR | O_APPEND | O_TRUNC | O_CREAT | O_EXCL;
		 
		if( (flag | flag_test) == flag)
		{
			cerr << "open_file():  Received an invalid flag argument." << endl;
			return false;
		}
		
		if(filename == "")
		{
			cerr << "open_file():  Filename provided is an empty string." << endl;
			return false;
		}
		
		fd = open(filename.c_str(), flag);
	
		if(fd == -1)
		{
			int error = errno;	// Immediately preserve the error state, in case cerr changes it.
			cerr << "open_file():  Failed to open file " << filename << "." << endl; 
			strerror(error);
			return false;
		}
		
		return true;
	}
	
	// This is an alternate verion of open_file() with user provided extra modes.  If used, it should be
	// called if the class is ready.
	bool file_input::open_file(int flag, int mode)
	{
		int flag_test = O_RDONLY | O_WRONLY | O_RDWR | O_APPEND | O_TRUNC | O_CREAT | O_EXCL;
		int mode_test = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH;
		 
		if( (flag | flag_test) == flag)
		{
			cerr << "open_file():  Received an invalid flag argument." << endl;
			return false;
		}
		
		if( (mode | mode_test) == mode)
		{
			cerr << "open_file():  Received an invalid mode argument." << endl;
			return false;
		}
		
		if(filename == "")
		{
			cerr << "open_file():  Filename provided is an empty string." << endl;
			return false;
		}
		
		fd = open(filename.c_str(), flag, mode);
	
		if(fd == -1)
		{
			int error = errno;	// Immediately preserve the error state, in case cerr changes it.
			cerr << "failed to open file " << filename << endl; 
			strerror(error);
			return false;
		}
		
		return true;
	}
	
	void file_input::close_file()
	{
		// Don't close standard input, output or error.

		if(fd > 2)
		{
			int result = close(fd);
			if(result != 0)
			{
				int error = errno;
				cerr << "process_db():  Close failed." << endl;
				strerror(error);
			}
			
			fd = 0;
		}
		else
			cerr << "close_file() called on invalid file descriptor.  Exiting." << endl;
	}
	
	void file_input::error_cleanup(int error)
	{
		strerror(error);
		close_file();
	}
	
	int file_input::operator()(const std::string& filename, containers::array<algorithms::line>& lines)
	{
		
		if(filename.length() == 0)
		{	cerr << "file_input::operator()():  Empty file name provided." << endl;
			return -1;
		}
		
		this->filename = filename;
		
		if(read_file(lines) == false)
		{
			cerr << "read_file() returned false.  File not (completely) read." << endl;
			return -1;
		}
		
		return 0;
	}
	
	// This is called internally, if class is ready.
	bool file_input::read_file(containers::array<algorithms::line>& lines)
	{
		int result = 0;
		int error = 0;
		int start = 0;
		int end = 0;
		int linecount = 0;
		int filesize = 0;
		
		// uint asize = 0;
		
		algorithms::line line;
		algorithms::line::iterator it;
		
		// Open the file
		if(open_file(FileFlag) == false)
		{
			cerr << "open_file() failed.  Returning without reading any entries." << endl;
			return false;
		}
		
		while( (result = read(fd, _buffer, READ_BUFFER_SIZE)) > 0)
		{
			if(_DEBUG == true)
				cout << "read_file():  read " << result << " bytes." << endl;
			
			filesize += result;
			
			while(end <= result)
			{
				// end is the location of the newline.
				end = get_line(start);
				
				if(start == end)
					break;
				
				if(_DEBUG == true)
				{
				
					cout << "line " << linecount << ":  [" << start << ", " << end << "] = ";
					for(int i = start; i < end; i++)
					{
						cout << _buffer[i];
					}
					cout << " ";
				}
				
				line.set(&_buffer[start], end-start+1);
				
				if(_DEBUG == true)
				{
					cout << "length = " << line.length() << ":  \"";
					for(it = line.begin(); it != line.end(); it++)
						cout << *it;
					
					cout << "\"" << endl;
				}
				
				start = end + 1;
				
				// lines.addAt(linecount, line);
				lines.add(line);
				linecount++;
			}
			
			// Reset for next iteration/read.
			start = 0;
			end = 0;
		}
		
		if(result == -1)
		{
			error = errno;
			error_cleanup(error);
			return false;
		}
		
		close_file();
		
        if(_DEBUG == true)
			cout << "read_file():  Finished reading " << filesize << " bytes from the file." << endl;
		
		return true;
	}
	
	int file_input::get_line(int start)
	{
		int end = start;
		
		for(int i = start; i < READ_BUFFER_SIZE; i++)
		{
			if(_buffer[i] == '\n')
			{
				end = i;
				break;
			}
		}
		
		return end;
	}

}
