#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include "_containers.h"

#include <mutex>
#include <condition_variable>

#include "_containers_globals.h"
#include "_allocator.h"

namespace containers
{
	using namespace std;
	
	/*
	 * A thread-safe priority queue:
	 * 
	 * Use array representation of binary tree.  Starting at node k=1, it will have children at 2k and 2k+1.
	 * The parent of node k is located at floor(k/2):  ie node 4 has parent at 2, node 3 has parent at 1.
	 * 
	 * Root is largest, and its children are less than or equal.  We might need to initialize to some number
	 * to distinguish between set items and not set items.
	 */
	
	template <typename key, bool lessThan>
	class PQ
	{
		// Implicitly private members:
		
		static _allocator<key>* const pal;			// Constant pointer to type T allocator.
		static constexpr const uint _default_size = 100;
		
		void resize();
		void exchange(uint i, uint j) { key temp = _keys[i]; _keys[i] = _keys[j]; _keys[j] = temp; }
		void sink(uint);
		void swim(uint);
		
		key* _keys = nullptr;		// keys in _keys[1, N], as _keys[0] is unused.
		uint _N = 0;				// Number of items contained.
		uint _size = 0;				// Size of memory allocation.  Must be at least 1 larger than _N.  See enqueue().
		
		// With mutable, we can modify these variables even in a const setting, namely in our const getters.
		mutable std::mutex read_write_mutex;
		mutable std::mutex resize_mutex;
		
		// Evaluated at compile time.  This boolean is used to define compare() as either a less than or a greater than function.
		constexpr static auto usingLessThan = lessThan == true;
		bool compare(uint i, uint j) const
		{
			if constexpr (usingLessThan == true)
				return _keys[i] < _keys[j];
			else
				return _keys[i] > _keys[j];
		}
		
	public:
		PQ();
		PQ(const uint&);
		PQ(const PQ<key, lessThan>&);
		PQ(PQ<key, lessThan>&&) noexcept;
		PQ<key, lessThan>& operator=(const PQ<key, lessThan>&);
		PQ<key, lessThan>& operator=(PQ<key, lessThan>&&) noexcept;
		~PQ() noexcept { pal->deallocate(_keys); }
		
		void enqueue(const key&);
		key dequeue();
		void clear();
		
		bool empty() const { return _N == 0; }
		
		// Peek at the minimum/maximum value in a IMiPQ/IMaPQ.
		template<typename key1 = key, bool usingLessThan = lessThan>
		typename std::enable_if<usingLessThan == true, uint>::type min() const { if(_N == 0) throw NoSuchElementException(); else return _keys[1]; }
		template<typename key1 = key, bool usingLessThan = lessThan>
		typename std::enable_if<usingLessThan == false, uint>::type max() const { if(_N == 0) throw NoSuchElementException(); else return _keys[1]; }
		uint size() const { return _N; }
		
	};
	
	template <typename key, bool lessThan>
	_allocator<key>* const PQ<key, lessThan>::pal = &al<key>;
	
	template <typename key, bool lessThan>
	PQ<key, lessThan>::PQ()
	{
		_size = _default_size;
		_keys = pal->allocate(_size);
	}
	
	template <typename key, bool lessThan>
	PQ<key, lessThan>::PQ(const uint& n)
	{
		_size = n;
		_keys = pal->allocate(_size);
	}
	
	template <typename key, bool lessThan>
	PQ<key, lessThan>::PQ(const PQ<key, lessThan>& pq)
	{
		_size = pq._size;
		_N = pq._N;
		
		_keys = pal->copyFromResize(pq._keys, _keys);
	}
	
	template <typename key, bool lessThan>
	PQ<key, lessThan>::PQ(PQ<key, lessThan>&& pq) noexcept
	{
		int size = _size;
		_size = pq._size;
		pq._size = size;
		
		int N = _N;
		_N = pq._N;
		pq._N = N;
		
		_keys = pq._keys;
		pq._keys = nullptr;
	}
	
	template <typename key, bool lessThan>
	PQ<key, lessThan>& PQ<key, lessThan>::operator=(const PQ<key, lessThan>& pq)
	{
		{
			// Lock the mutex
			std::lock_guard<std::mutex> l(read_write_mutex);
			_size = pq._size;
			_N = pq._N;
			
			_keys = pal->copyFromResize(pq._keys, _keys);
			
			return *this;
		}
	}
	
	template <typename key, bool lessThan>
	PQ<key, lessThan>& PQ<key, lessThan>::operator=(PQ<key, lessThan>&& pq) noexcept
	{
		{
			// Lock the mutex
			std::lock_guard<std::mutex> l(read_write_mutex);
			
			int size = _size;
			_size = pq._size;
			pq._size = size;
			
			int N = _N;
			_N = pq._N;
			pq._N = N;
			
			key* temp = _keys;
			_keys= pq._keys;
			pq = temp;
			
			return *this;
		}
	}
	
	template <typename key, bool lessThan>
	void PQ<key, lessThan>::resize()
	{
		{
			// Lock the mutex
			std::lock_guard<std::mutex> l(resize_mutex);
			
			_size = _size * 2;
			_keys = pal->resizeTo(_keys, _size);
		}
	}
	
	template <typename key, bool lessThan>
	void PQ<key, lessThan>::sink(uint k)
	{
		// Only called by dequeue(), so there is no need for a dedicated mutex here.
		
		// Exchange parent with its largest child to restore heap order.
		while(2*k <= _N)
		{
			uint j =  2*k;					// j is the left child.
			if(j < _N && compare(j, j+1))		// While j < _N, j+1 is safe to compare against.  If left child is < right, switch to the right...
				j++;
			if(compare(k, j) == false)			// and test it against parent.  If parent k > larger child, then heap order has been restored.
				break;
			
			exchange(k, j);					// Else, we exchange parent with its larger child.
			k = j;							// Lastly, we prepare to evaluate the left child the same way.
		}
	}
	
	template <typename key, bool lessThan>
	void PQ<key, lessThan>::swim(uint k)
	{
		// Only called by enqueue(), so there is no need for a dedicated mutex here.
		
		// Exchange parent with its left child to restore heap order.
		while(k > 1 && compare(k/2, k))		// Swim up while parent is smaller than child.
		{
			exchange(k/2, k);
			k = k/2;
		}
	}
	
	template <typename key, bool lessThan>
	void PQ<key, lessThan>::enqueue(const key& k)
	{
		{
			// Lock the mutex
			std::lock_guard<std::mutex> l(read_write_mutex);
			if(_N + 1 == _size)
				resize();
			
			_keys[++_N] = k;		// Pre-incrementing N prior to array access, ensures that N > 1 when container is not emtpy => _keys[0] is unused.
			swim(_N);
		}
	}
	
	template <typename key, bool lessThan>
	key PQ<key, lessThan>::dequeue()
	{
		{
			// Lock the mutex
			std::lock_guard<std::mutex> l(read_write_mutex);
			
			key max = _keys[1];
			exchange(1, _N--);		// exchange(1, copy of _N) and decrement _N prior to call.
			_keys[_N+1] = key{};
			sink(1);
			
			return max;
		}
	}
	
	template <typename key, bool lessThan>
	void PQ<key, lessThan>::clear()
	{
		pal->clear(_keys);
		_N = 0;
	}
	
	template<typename key>
	using maxPQ = PQ<key, max>;
	
	template<typename key>
	using minPQ = PQ<key, min>;
	
	using bMinPQ = minPQ<bool>;
	using bMaxPQ = maxPQ<bool>;
	
	using uMinPQ = minPQ<uint>;
	using uMaxPQ = maxPQ<uint>;
	
	using dMinPQ = minPQ<double>;
	using dMaxPQ = maxPQ<double>;
	
	
}

#endif
