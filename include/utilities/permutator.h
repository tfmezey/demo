#ifndef PERMUTATOR_H
#define PERMUTATOR_H

#include <random>
#include <chrono>
#include <iostream>

#include "containers.h"

namespace utilities
{
	/*
	 * Algorithm to find a permutation of the ordered sequence [0, N-1], with N > 1.
	 * 
	 * We will use an indexed minimum priority queue using containers::uint (value, key) pairs.
	 * 
	 * Each key is a random number, having an associated value equaling the order of generation.
	 * These (value, key) ordered pairs will be placed into the IMiPQ, which sorts the values using
	 * their respective key.
	 * 
	 * We then remove the the N minima to obtain a perumation of the N number of indices of the range
	 * [0, N-1].
	 * 
	 * Note as we are using a uniform distribution, all numbers occur with equal probability.  Thus if we
	 * have a sufficiently large range to pick a sufficiently small sample of numbers, we should then 
	 * minimize, if entirely avoid, collision of random number generation.
	 * 
	 * Ideally, the random number generation is hardware produced.  Assume O(1) for each number, or O(N) for
	 * the whole set.
	 * IMPQ:
	 *  enqueue():  O(log N)
	 *  dequeue():  O(log N)
	 * 
	 * Algorithm:  random number generation + N enqueue() + N dequeue() => O(N) + O(2N log N) => O(N log N).
	 * 
	 */
	
	template <containers::uint N>
	class permutator
	{
		using IMiPQ = containers::IMPQ<uint, true>;
		
		IMiPQ* _pPQ = nullptr;
		std::random_device* _prd = nullptr;					// cat /proc/cpuinfo | grep rdrand.  Declaring a pointer to it as there may not be support for it.
		std::default_random_engine generator;
		std::uniform_int_distribution<uint> random_range;
		uint _N = 0;
		constexpr static const uint Nmax = 1e6;
		constexpr static const uint _M = 1e9;				// This is the upper limit of the uniform random distribution, having range [0, _M].
		bool _pseudo = false;
		
	public:
		permutator();
		~permutator();
		void operator()(containers::array<uint>&);
		bool isHardwareRNG() const { return !_pseudo; }
		
	};
	
	template <containers::uint N>
	permutator<N>::permutator()
	{
		static_assert(N < Nmax, "permutator():  N exceeds maximum size.");
		
		_N = N;
		_pPQ = new containers::IMPQ<uint, true>(N);
		
		try
		{
			_prd = new std::random_device;							// Create the hardware random device, ...
			generator = std::default_random_engine((*_prd)());		// and use it as the seed for the DRE.
		}
		catch(const std::exception& e)
		{
			// Alternatively, if there is no hardware support for RNG, use a pRNG with a time based seed instead.
			unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
			generator = std::default_random_engine(seed);
			_pseudo = true;
		}
		
		random_range = std::uniform_int_distribution<uint>(0, _M);
	}
	
	template <containers::uint N>
	permutator<N>::~permutator()
	{
		delete _pPQ;
		delete _prd;
	}
	
	template <containers::uint N>
	void permutator<N>::operator()(containers::array<uint>& arui)
	{
		/*
		 * Clear the provided array and enqueue _N values in the range [0, _N),
		 * with corresponding _N random keys into the index minimum PQ.
		 * Note that enqueueing may result in an exception if the random key
		 * generated is already in the IMiPQ.
		 */
		
		arui.clear();
		uint key = 0;
		
		for(int i = 0; i < _N; i++)
		{
			try
			{
				key = random_range(generator);
				_pPQ->enqueue(i, key);
			}
			catch(const std::exception e)
			{
				std::cerr << "permutator:  Insert failed at (" << i << ", " << key << ")" << std::endl;
				break;
			}
		}
		
		while(_pPQ->size())
			arui.add(_pPQ->dequeue());
	}
}

#endif
