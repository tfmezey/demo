# demo
A representative cross-section of various projects of mine.

NFA Regular Expressions

The regular expression functionality is provided by the algorithms::NFA class.  The NFA can handle
following character classes, with self-explanatory names, using names adapted from Unix/Linux utility sed:

    • [[:ascii:]], [[:alpha:]], [[:ALPHA:]], [[:digit:]] 
    • [[:hex:]] targets characters that can represent a hexadecimal digit, [A-F], [a-f] and [0-9], excluding the ‘x’/’X’ characters.
    • [[:misc:]] targets any ASCII characters that aren't alphabetic nor numerical.
	
The NFA supports the following multiplicity modifiers, applicable to single characters or parenthesized groups of characters:

    • Kleene closure, ‘*’:  0 or more copies
    • ‘?’:  0 or one copy
    • ‘+’:  1 or more copies
    • \{n1,n2\}:  range of multiplicities n1 to n2 inclusive at both ends, arbitrarily up to 99, with n2 greater than or equal to n1.
    • „{0-9\}”, „\{A-Z\}” or „\{a-z\}” are user defined alphanumeric ranges, which support the complement operator ‘^’.
    
The NFA also supports the single-OR ‘|’ and multi-OR expression „(… |…|…)”, the latter of which can be modified with the above multiplicity modifiers, as it constitutes a parenthesized group.

Internally, a graph is constructed, with edges corresponding to the ε-transitions of Kleene closures or the single-OR or multi-OR expressions.  Note that all ranges etc., can be expressed
as a linear combination of the Kleene closure or one or more of the OR expression.

Starting with the initial reachable states of the regular expression being placed into an array of matched vertices, the first input character is tested against a character in the RE at the
location of each matched vertex.

Once all matched vertices are tested against, the algorithm runs a multi source depth first search algorithm, to produce the next set of reachable states/vertices.  These become the matched states
against which the next character is tested.

Once all the characters of the input string are consumed, the algorithm tests whether the final state (vertex) of the RE has been reached.  If so, the input string is matched by the RE, otherwise
it fails the RE.  

NFA Tests

As a demonstration of the NFA, we test several numbers to determine whether the NFA recognizes them as numbers.  Previously and early in the development, the author encountered a bug where a
numerical token with a trailing space was permitted to be read in, which the NFA also identified as a number.  

We test such scenarios for whole and real numbers, as well as a 32-bit integer overflow case.

Note that the overflow case is handled by the RE string of the NFA, having a multiplicity range modifier of „{1,10}”.  The passed number may still be an invalid 32-bit integer, and one would need to
do a string to numerical conversion, with subsequent limit tests, to truly determine the case.  In this test case however, if the integer RE fails, we apply the RE for a long integer, which has twice
the number of permitted digits to a 32-bit integer.

As another simple demonstration of the NFA, we have the test_acyclic_paths() function, which uses a rudimentary scanner to read in a graph from a file, and calculates shortest and longest paths from
two predetermined vertices.

The function uses the algorithms name space class, read_edge_weighted_digraph, which first reads the graph from a file into memory.  This class in turn has a private member of the same name space, named
read_tokens, which uses NFA-s and stateful iteration to retrieve tokens one at a time.  During this process, each token is attempted to be identified by use of one of the NFA-s, and the outer class uses
these to construct the graph via its edges, sans any error.

Runga-Kutta Order 4

This is a demonstration of ODE solving using the above named algorithm, implemented in C++ and in x86_64 assembly language.

To test the algorithm, we used the two body central force problem.  

This is a classic example of a stiff ODE, as the problem is sensitive to truncation error.  These errors manifest as changes in the total mechanical energy, which ought to remain constant in the absence of
a dissipative force.

The demo starts out by generating a random set of orbital parameters that ensure a bound system for each timed trial.  The trial consists of two parts, running equal trials of the C++ r4k() as well as the
assembly language written r4k_asm().

Note that the author observed that there were rare generated initial conditions that caused the algorithm to fail at preserving the total mechanical energy.  The reason for this is that in those circumstances,
the solution was excessively sensitive to the size of the time step, which was set to one day in all trials.  An adaptive solution is needed to mitigate such sensitivities.  More on that later.

In the end, the trials are then compared to one another.  So far, on the author’s computer, the C++ version runs consistently 30% faster than the assembly language version.

The assembly language implementation of the rk4 is noteworthy.  First, the C++ compiler, g++, did not honor requests to not mangle the names of assembly language written member functions for the algorithm’s
templated class, rk4solver.  

Instead, we chose to put the assembly functions into a non-templated class, and derive our main templated solver class from it.  

This was not the only solution, as the non-templated class could just as easily been a member of the solver class.  However, that route required passing two pointers to the initializing function (more on that
below): 1) the this pointer of the parent class required by the standard calling convention for member functions, and 2), the pointer to the rk4solver class.

The key to using the assembly language written member functions is the implicit this pointer, passed as the first parameter of these functions.  The static assembly function initialize(), first sets up the
this pointer to the parent class, and initializes pointers to the arrays used by the rk4_asm().  These pointers are shared by all assembly functions.

So long as the list of members within the rk4solver class remain unchanged, the offsets from the this pointer to the internally used arrays remains fixed.  Any changes to this require updating the list of
offsets provided to the initializing function.

Once the assembly portion of the class has been initialized, we set up a private member function pointer to point to either the C++ rk4() member function, or to the base class’ assembly language version
rk4_asm(), using a boolean.  The function call operator ends up de-referencing the pointer, which results in the function call.

Normally in C++, a function call of an object resolves into a call to the operator()() member function, which is anonymous and whose address is some compiler determined offset from the this pointer.  

In order to mimic this behavior using our assembly language written rk4_asm() function, we had to de-reference the this pointer twice in order to facilitate the call.  The reason for this is that we
deliberately chose the member function pointer to be the first member of the r4ksolver class.  Had it not been the case, then we would then needed to obtain the offset to that pointer, and add it to the
address of the de-referenced this pointer, and call from that location.

Runga-Kutta-Fehlberg:

This solver uses a 5th order solution to estimate the local truncation error, in order to allow scaling of the independent variable (here the time step), while generating a 4th order Runge Kutta solution.
As the time step changes, the solver can smoothly handle large local changes in the solution, such as the peaks and troughs of the kinetic and potential energies of the orbital problems.

The inescapable trade off comes from the increased evaluation of the state using the functor class.  This makes the solver slightly slower than the r4k solver, however, we obtain fewer data points, as well
as retain accuracy which the r4k cannot guarantee within tolerance.

In the test_rkf_solver() trials, we this time see that the assembly language version is slightly faster.  Additionally, the RKF algorithm is faster than the RK4 fixed time step algorithm in solving ODE.

In conclusion, the risk of having unsafe assembly language routines was worth the price in not just efficiency, but also accuracy even in those problems where the solution changes rapidly.
