#include "timer.h"

namespace utilities
{
    timer::~timer()
    {
        using namespace std::chrono;
        tpoint stop = std::chrono::steady_clock::now();
        auto duration = stop - _start;
        auto ms = std::chrono::duration_cast<milliseconds>(duration).count();
        if(_directTo == stdoutput)
            std::cout << ms << "ms " << _context << std::endl;
        else
            std::cerr << ms << "ms " << _context << std::endl;
    }
    
    silent_timer::~silent_timer()
    {
        using namespace std::chrono;
        tpoint stop = std::chrono::steady_clock::now();
        auto duration = stop - _start;
		if(_accumulate)
			_duration += std::chrono::duration_cast<milliseconds>(duration).count();
		else
			_duration = std::chrono::duration_cast<milliseconds>(duration).count();
    }
}

