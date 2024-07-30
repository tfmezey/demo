#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <iostream>

/*
 * Adapted from C++ High Performance, p154.  This is a scoped timer.
 *  Usage:
 *       {   // Start of some context.
 *           utilities::timer Timer("Timer1");        // Can provide an optional second argument:  timer::output::stdoutput or timer::output::stderror.
 *          ...
 *       }
 *
*/

namespace utilities
{
	constexpr const bool accumulate = true;
	
    class timer
    {
    public:
        enum output : char {stdoutput, stderror};
        using tpoint = std::chrono::steady_clock::time_point;

		timer() = delete;
        timer(const char* context, output directTo = stdoutput) { _context = context; _start = std::chrono::steady_clock::now(); _directTo = directTo; }
        // Prevent copy/move construction/assigment.
        timer(const timer&) = delete;
        timer(timer&&) = delete;
        timer& operator=(const timer&) = delete;
        timer& operator=(timer&&) = delete;
        ~timer();
    private:
        const char* _context = nullptr;
        tpoint _start{};
        output _directTo = stdoutput;
    };
	
	class silent_timer
    {
    public:
        
		enum measurement : bool { single=false, accumulating};
		using tpoint = std::chrono::steady_clock::time_point;

		/*
		 * Modifed timer() class that takes an unsigned long reference and an optional local enumeration type to
		 * either assign the duration to the reference or add the duration to the provided refrence.
		*/
		
		silent_timer() = delete;
        silent_timer(int& duration, measurement accumulate=single) : _duration(duration) { _start = std::chrono::steady_clock::now();  _accumulate = accumulate; }
        // Prevent copy/move construction/assigment.
        silent_timer(const timer&) = delete;
        silent_timer(silent_timer&&) = delete;
        silent_timer& operator=(const silent_timer&) = delete;
        silent_timer& operator=(silent_timer&&) = delete;
        ~silent_timer();
    private:
		int& _duration;
		tpoint _start{};
		measurement _accumulate = single;
    };
}

#endif
