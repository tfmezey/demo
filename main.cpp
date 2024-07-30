#include <iostream>
#include <fstream>
#include <string>

#include <sys/ioctl.h>		// ioctl() and TIOCGWINSZ
#include <unistd.h>			// STDOUT_FILENO
#include <term.h>
#include <getopt.h>

#include "utilities.h"
#include "algorithms.h"
#include "graphs.h"
#include "containers.h"

#include "rk4solver.h"
#include "gravity.h"

namespace containers
{
	bool _DEBUG = false;
}

using namespace std;

int test_read_tokens_RE();
int test_acyclic_SP();
int test_acyclic_LP();
int test_acyclic_paths();
int test_rk4_solver(long, bool saveLastTrial=false);
int test_topological();
int test_rkf_solver(long, bool saveLastTrial=false);

int run_solver_rkf_debug();

bool ProcessArgs(int argc, char* argv[], long& trialCount)
{
	// --help, -h, --trialCount, --debug
	string optional_rename_arg("");
	const string usage = "Usage:  " + string(argv[0]) + " --debug --trialCount=<value>" ;
	const int OptionsCount = 4;
	utilities::ExpandPath ep;
	algorithms::readNumber rn;

	int option = 0;
	struct option long_options[OptionsCount] = {
		{ "debug", no_argument, nullptr, 'b' },
		{ "trialCount", required_argument, nullptr, 't'},
		{ "help", no_argument, nullptr, 'h' },
		{nullptr,0,nullptr,0}								// Mandatory "null" termination.
	};

	/*
	 * getopt_long() returns a '?' when an option not in the option string is encountered.  If an
	 * option requiring an argument is missing, a ':' is returned.  Lastly, when all options are
	 * processed, a -1 is returned.
	 * 
	 * Additionally, with the option string starting with '+', we stop processing arguments once we
	 * obtain a non-option argument.  This is unsafer, as we may continue with an error.  However,
	 * including it prevents detailing the error, as -1 is returned on any unrecognized argument, and
	 * our default statement in never gets executed.
	 * 
	 * Lastly, any other character given that is not an option (namely anything not prefixed with '-')
	 * will be ignored in the sans '+' case.
	*/
	
	while( (option = getopt_long(argc, argv, "bt:h", long_options, nullptr)) != -1 )
	{
		switch(option)
		{
			case 'b':
			{	
				// -b, --debug
				algorithms::_DEBUG = true;
				graphs::_DEBUG = true;
				break;
			}
			case 't':
			{
				// trialCount = optarg;
				cout << "optarg = " << optarg << endl;
				algorithms::readNumber::token result = rn(optarg);
				if(rn.isNatural() == true)
					trialCount = rn.getLong();
				break;
			}
			case 'h': 			// -h, --help
				cerr << usage << endl;
				return false;
			case '?':
				cerr << "Unrecognized parameter given." << endl << usage << endl;
				return false;
			case ':':			
				cerr << "Missing required parameter for option." << endl << usage << endl;
				return false;
				
			default:
				cerr << usage << endl;
				return false;
		}
	}
	
	if(trialCount == 0)
		trialCount = 100000;
	
	return true;
}

int main(int argc, char *argv[])
{
    using namespace std;
	algorithms::_DEBUG = false;
	graphs::_DEBUG = false;

	long rkTrials = 0;
	if(ProcessArgs(argc, argv, rkTrials) == false)
		return -1;
	
	// Regular Expression demo:
    // string bad_double = "41.0 ";			// Does the regular expression for a double allow for the space ending numerical string to pass?
	string bad_double = "-1.e-8 ";
	string good_double = "41.0";
	string good_int = "12345";
	string bad_int = "123456789012345678";
    string double_re = "(-?[[:digit:]]+.?[[:digit:]]*((e|E)-?[[:digit:]]{1,3})?)";
	string int_re = "(-?[[:digit:]]{1,10})";
	string long_re = "(-?[[:digit:]]{1,20})";
	
    algorithms::NFA dre(double_re);
	algorithms::NFA ire(int_re);
	algorithms::NFA lre(long_re);
	
	//Doubles
    if(dre.recognizes(bad_double) == true)
        cout << "RE recognizes \"" << bad_double << "\" as a double." << endl;
    else
		cout << "RE does not recognize \"" << bad_double << "\" as a double." << endl;
	
	if(dre.recognizes(good_double) == true)
        cout << "RE recognizes \"" << good_double << "\" as a double." << endl;
    else
		cout << "RE does not recognize \"" << good_double << "\" as a double." << endl;
	
	// Int
	if(ire.recognizes(bad_int) == true)
		cout << "RE recognizes\"" << bad_int << "\" as a int." << endl;
    else
	{
		cout << "RE does not recognize bad_int = \"" << bad_int << "\" as a int." << endl;
		if(lre.recognizes(good_int) == true)
			cout << "However, RE recoginizes \"" << bad_int << "\" as long." << endl;
		else
			cout << "\"" <<  bad_int << "\" also fails the long RE." << endl;
	}
	
	if(dre.recognizes(good_int) == true)
		cout << "RE recognizes \"" << good_int << "\" as a int." << endl;
    else
		cout << "RE does not recognize \"" << good_int << "\" as a int." << endl;
	
	cout << endl;
	
	test_acyclic_paths();
	
	test_rk4_solver(rkTrials);
	
	test_rkf_solver(rkTrials, true);
	
    return 0;
}



