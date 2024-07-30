#ifndef INDEX_M_PQ_H
#define INDEX_M_PQ_H

#include <mutex>
#include <condition_variable>
#include <type_traits>

#include "_containers.h"
#include "_containers_globals.h"
#include "_allocator.h"

namespace containers
{
	using namespace containers;
	
	template <typename key, bool lessThan>
	class IMPQ
	{
		/*
		 * Index Minimum/Maximum Priority Queue
		 * 
		 * Insert (Value, Key) ordered pairs into the container to obtain the sequence of increasing Values using their 
		 * respective Keys for the basis of comparison.  Values can be removed or have their Key changed.
		 * Note that we have a 1-to-1 mapping, thus there can be only one unique Value in the container and there is only
		 * one unique Key for that Value.  The array _keys may be sparse.  The Values are restricted to [0, _size).
		 * 
		 * The _heap array stores the sorted Values, with _heap[0] not used.  The _keys array stores the priorities of
		 * the Values:
		 * 		_keys[Value] = Key of Value = priority of Value.
		 * The _keys array is only changed by 1) the removal of a Value from the container via dequeue(), 
		 * or 2) a reprioritization of a Value via changeKey(), decreaseKey(), dequeue() or increaseKey().
		 * 
		 * Additionally, the _inverse_heap array is used in order to provide O(1) queries for Values.  For each Value,
		 * we write to _inverse_heap[Value] the new index of Value within _heap.  This occurs during any enqueue/dequeue
		 * operation, as enqueue() calls swim(), while dequeue calls both swim() and sink().  Note that although _heap[0]
		 * is not used, we can make use of _inverse_heap[0], if the Value of zero was added to the container!  See 
		 * enqueue(), and assume (0, SomeKey) is the first ordered pair to be enqueued to the container.  Also note that
		 * not providing for an inverse to the _heap array, will force one to do an O(n) search of _heap to determine 
		 * whether a particular Value is contained.
		 * 
		 * The relationship between _heap and _inverse_heap is:
		 * 		_inverse_heap[_heap[Value]] = Value = _heap[_inverse_heap[Value]]
		 * 
		 * Enqueueing:
		 * Initially, a Value is added to the end of the _heap array, that is at the index of the new size of the
		 * container, which is greater than or equal to 1.  Its Key is added to the _keys array at the location of Value,
		 * and the index of the initial insertion of the Value, the new size of the container, is added to the
		 * _inverse_heap array at the location the Value:
		 * 		_N++;
		 * 		_heap[_N] = Value;
		 * 		_inverse_heap[Value] = _N;
		 * 		_keys[Value] = Key;
		 * 
		 * Then, a swim() call is made, which calls exchange() appropriately, which changes the _heap and _inverse_heap to
		 * maintain the heap structure invariant, namely that every node k has two possible children, lesser/greater than itself,
		 * one at 2k, and another at 2k+1.  Lastly, as this relationship cannot be upheld if the index within the _heap is
		 * permitted to have the value of zero, the indexing within the PQ must therefore start at 1.
		 * 
		 * Note that there is a less efficient version of changing a Value's priority, namely changeKey().  This version
		 * calls both sink() and swim(), while increaseKey() only calls sink(), and decreaseKey() only calls swim().  
		 * Caveat emptor!  
		 * Additionally, the dequeue() also calls sink() and swim() to restore heap structure invariance.  Caveat emptor!
		 * 
		 * Searches:
		 * 		minKey(), minValue(), contains():  O(1)
		 * Changes:
		 * 		enqueue(), dequeue(), changeKey(), decreaseKey(), increaseKey():  O(log_2 n)
		 * The lessThan template parameter is used to define compare() as a less than or greater than operation.
		 */
		
		// Implicitly private members:
		
		static _allocator<key>* const pal_keys;			// Constant pointer to type key allocator.
		static _allocator<uint>* const pal_uints;		// Constant pointer to type uint allocator.
		constexpr static const uint _default_size = 100;
		
		uint* _heap = nullptr;				// Binary heap, with 1-based indexing, used to access the _keys array.
		uint* _inverse_heap = nullptr;		// Inverse of _heap:   _inverse_heap[_heap[i]] = _heap[_inverse_heap[i]] = i.
		key* _keys = nullptr;				// _keys[i] = priority of i.
		
		uint _N = 0;						// Number of items contained.
		uint _size = 0;						// Size of memory allocation.  Must be at least 1 larger than _N.  See insert().
		
		// With mutable, we can modify these variables even in a const setting, namely in our const getters.
		mutable std::mutex read_write_mutex;
		mutable std::mutex resize_mutex;
		
		// Evaluated at compile time.  This boolean is used to define compare() as either a less than or a greater than function.
		constexpr static auto usingLessThan = lessThan == true;
		bool compare(uint i, uint j) const
		{
			if constexpr (usingLessThan == true)
				return _keys[_heap[i]] > _keys[_heap[j]];
			else
				return _keys[_heap[i]] < _keys[_heap[j]];
		}
		
		void exchange(uint i, uint j);
		void resize();
		void sink(uint);
		void swim(uint);
		void validateIndex(const uint&) const;
		
	public:
		IMPQ();
		IMPQ(const uint&);
		IMPQ(const IMPQ&);
		IMPQ(IMPQ&&) noexcept;
		IMPQ& operator=(const IMPQ&);
		IMPQ& operator=(IMPQ&&) noexcept;
		~IMPQ() noexcept;
		
		// Setters
		void clear();
		uint dequeue();
		void dequeue(const uint& Value);
		void enqueue(const uint& Value, const key& Key);
		void changeKey(const uint& Value, const key& Key);
		void decreaseKey(const uint& Value, const key& Key);
		void increaseKey(const uint& Value, const key& Key);
		
		// Getters
		bool contains(const uint& Value) const { validateIndex(Value); return _inverse_heap[Value] != undefined_uint; }  
		bool empty() const { return _N == 0; }
		key keyOf(const uint&) const;
		uint size() const { return _N; }
		
		// Peek at the minimum value in the IMiPQ.
		template<typename key1 = key, bool usingLessThan = lessThan>
		typename std::enable_if<usingLessThan == true, uint>::type minValue() const { if(_N == 0) throw NoSuchElementException(); else return _heap[1]; }
		// Peek at the minimum key in the IMiPQ.
		template<typename key1 = key, bool usingLessThan = lessThan>
		typename std::enable_if<usingLessThan == true, uint>::type minKey() const { if(_N == 0) throw NoSuchElementException();  else return _keys[_heap[1]]; }
		
		// Peek at the minimum in the IMaPQ.
		template<typename key1 = key, bool usingLessThan = lessThan>
		typename std::enable_if<usingLessThan == false, uint>::type maxValue() const { if(_N == 0) throw NoSuchElementException(); else return _heap[1]; }
		// Peek at the maximum key in the IMaPQ.
		template<typename key1 = key, bool usingLessThan = lessThan>
		typename std::enable_if<usingLessThan == false, uint>::type maxKey() const { if(_N == 0) throw NoSuchElementException();  else return _keys[_heap[1]]; }
		
	};
	
	template <typename key, bool lessThan>
	_allocator<key>* const IMPQ<key, lessThan>::pal_keys = &al<key>;
	
	template <typename key, bool lessThan>
	_allocator<uint>* const IMPQ<key, lessThan>::pal_uints = &al<uint>;
	
	template <typename key>
	using IMiPQ = IMPQ<key, min>;
	
	template <typename key>
	using IMaPQ = IMPQ<key, max>;
	
	template <typename key, bool lessThan>
	IMPQ<key, lessThan>::IMPQ()
	{
		_size = _default_size;
		_inverse_heap = pal_uints->allocate(_size);
		_keys = pal_keys->allocate(_size);
		_heap = pal_uints->allocate(_size);
	}
	
	template <typename key, bool lessThan>
	IMPQ<key, lessThan>::IMPQ(const uint& n)
	{
		_size = n;
		_inverse_heap = pal_uints->allocate(_size);
		_keys = pal_keys->allocate(_size);
		_heap = pal_uints->allocate(_size);
	}
	
	template <typename key, bool lessThan>
	IMPQ<key, lessThan>::IMPQ(const IMPQ<key, lessThan>& pq)
	{
		_size = pq._size;
		_N = pq._N;
		
		_inverse_heap = pal_uints->copyFromResize(pq._inverse_heap, _inverse_heap);
		_keys = pal_keys->copyFromResize(pq._keys, _keys);
		_heap = pal_uints->copyFromResize(pq._heap, _heap);
	}
	
	template <typename key, bool lessThan>
	IMPQ<key, lessThan>::IMPQ(IMPQ<key, lessThan>&& pq) noexcept
	{
		int size = _size;
		_size = pq._size;
		pq._size = size;
		
		int N = _N;
		_N = pq._N;
		pq._N = N;
		
		_inverse_heap = pq._inverse_heap;
		_keys = pq._keys;
		_heap = pq._heap;
		
		pq._inverse_heap = nullptr;
		pq._keys = nullptr;
		pq._heap = nullptr;
	}
	
	template <typename key, bool lessThan>
	IMPQ<key, lessThan>& IMPQ<key, lessThan>::operator=(const IMPQ<key, lessThan>& pq)
	{
		{
			// Lock the mutex
			std::lock_guard<std::mutex> l(read_write_mutex);
			
			_N = pq._N;
			_size = pq._size;
			
			_inverse_heap = pal_uints->copyFromResize(pq._inverse_heap, _inverse_heap);
			_keys = pal_keys->copyFromResize(pq._keys, _keys);
			_heap = pal_uints->copyFromResize(pq._heap, _heap);
			
			return *this;
		}
	}
	
	template <typename key, bool lessThan>
	IMPQ<key, lessThan>& IMPQ<key, lessThan>::operator=(IMPQ<key, lessThan>&& pq) noexcept
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
			
			uint* temp1 = _inverse_heap;
			key* temp2 = _keys;
			uint* temp3 = _heap;
			
			_inverse_heap = pq._inverse_heap;
			pq._inverse_heap = temp1;
			
			_keys = pq._keys;
			pq = temp2;
			
			_heap = pq._heap;
			pq._heap = temp3;
			
			return *this;
		}
	}
	
	template <typename key, bool lessThan>
	IMPQ<key, lessThan>::~IMPQ() noexcept
	{
		pal_uints->deallocate(_inverse_heap);
		pal_keys->deallocate(_keys);
		pal_uints->deallocate(_heap);
	}
	
	template <typename key, bool lessThan>
	void IMPQ<key, lessThan>::resize()
	{
		{
			// Lock the mutex
			std::lock_guard<std::mutex> l(resize_mutex);
			
			_size *= 2;
			_inverse_heap = pal_uints->resizeTo(_inverse_heap, _size);
			_keys = pal_keys->resizeTo(_keys, _size);
			_heap = pal_uints->resizeTo(_heap, _size);
		}
	}
	
	/*
	 * Change the key associated with Value, if present.
	 * Throws NoSuchElementException() when key is not present in the container.
	 */
	template <typename key, bool lessThan>
	void IMPQ<key, lessThan>::changeKey(const uint& Value, const key& Key)
	{
		{
			// Lock the mutex
			std::lock_guard<std::mutex> l(read_write_mutex);
			validateIndex(Value);
			
			if(contains(Value) == false)
				throw NoSuchElementException();
			
			_keys[Value] = Key;
			
			// Repair the tree:
			swim(_inverse_heap[Value]);		// Repair left children.
			sink(_inverse_heap[Value]);		// Repair right children.
		}
	}
	
	/*
	 * Empties the container.
	 */
	template <typename key, bool lessThan>
	void IMPQ<key, lessThan>::clear()
	{
		{
			// Lock the mutex
			std::lock_guard<std::mutex> l(read_write_mutex);
			
			pal_uints->clear(_inverse_heap);
			pal_keys->clear(_keys);
			pal_uints->clear(_heap);
			_N = 0;
		}
	}
	
	/*
	 * Decrease the Key associated with Value.
	 * Throws NoSuchElementException() when the key parameter is not in the container.
	 * Throws IllegalArgumentException() when key provided is less than or equal to existing key.
	 */
	template <typename key, bool lessThan>
	void IMPQ<key, lessThan>::decreaseKey(const uint& Value, const key& Key)
	{
		{
			// Lock the mutex
			std::lock_guard<std::mutex> l(read_write_mutex);
			
			validateIndex(Value);
			if(contains(Key) == false)
				throw NoSuchElementException();
			if(_keys[Value] <= Key)
				throw IllegalArgumentException();
			
			_keys[Value] = Key;
			swim(_inverse_heap[Value]);
		}
	}
	
	/*
	 * Remove the root of the tree and rebalance.  Returns Value.  Equivalent to delMin()/delMax().
	 * Throws NoSuchElementException() when the containers is empty.
	 */
	template <typename key, bool lessThan>
	uint IMPQ<key, lessThan>::dequeue()
	{
		{
			// Lock the mutex
			std::lock_guard<std::mutex> l(read_write_mutex);
			
			if(_N == 0)
				throw NoSuchElementException();
			
			uint maxValue = _heap[1];
			exchange(1, _N--);				// exchange(1, copy of _N) and decrement _N prior to call.
			sink(1);
			
			// Deletion
			_inverse_heap[maxValue] = undefined_uint;
			_keys[maxValue] = key{};
			_heap[_N+1] = undefined_uint;	// We have to add one as we previously decremented _N.
			
			return maxValue;
		}
	}
	
	/*
	 * Remove the (Value, Key) ordered pair.
	 * Throws NoSuchElementException() when the key parameter is not in the container.
	 * Throws InvalidIndexException() indirectly when the index is not in the container.
	 */
	template <typename key, bool lessThan>
	void IMPQ<key, lessThan>::dequeue(const uint& Value)
	{
		{
			// Lock the mutex
			std::lock_guard<std::mutex> l(read_write_mutex);
			
			validateIndex(Value);
			if(contains(Value) == false)
				throw NoSuchElementException();
			
			uint index = _inverse_heap[Value];
			exchange(index, _N--);
			
			// Repair the tree:
			swim(index);		// Repair left children.
			sink(index);		// Repair right children.
			
			_keys[Value] = key{};
			_inverse_heap[Value] = undefined_uint;
		}
	}
	
	/*
	 * Enqueue a unique key and value pair.
	 * Throws IllegalArgumentException() when the key is already present.
	 * Throws InvalidIndexException() indirectly, when the provided value is already present.
	*/
	template <typename key, bool lessThan>
	void IMPQ<key, lessThan>::enqueue(const uint& Value, const key& Key)
	{
		{
			// Lock the mutex
			std::lock_guard<std::mutex> l(read_write_mutex);
			if(_N + 1 == _size)				// _N \in [1, _size]
				resize();
			
			validateIndex(Value);
			
			if(contains(Value) == true)					
				throw IllegalArgumentException();
			
			// Place item at the bottom and swim up to proper location.
			_N++;
			_heap[_N] = Value;				// Initial priority of Value = _N, post increment.
			_inverse_heap[Value] = _N;		// Record the index of Value within _heap.
			_keys[Value] = Key;
			
			swim(_N);
		}
	}
	
	/*
	 * Exchange the indeces in _heap and _inverse_heap.  This is called internally by swim(), sink() and delMin().
	 */
	template <typename key, bool lessThan>
	void IMPQ<key, lessThan>::exchange(uint i, uint j)
	{
		// Called internally between thread locks.  So don't lock here.
		uint temp = _heap[i];
		_heap[i] = _heap[j];
		_heap[j] = temp;
		
		_inverse_heap[_heap[i]] = i;
		_inverse_heap[_heap[j]] = j;
	}
	
	/*
	 * Increase the Key associated with Value.
	 * Throws NoSuchElementException() when the key parameter is not in the container.
	 * Throws IllegalArgumentException() when key provided is greater than or equal to existing key.
	 */
	template <typename key, bool lessThan>
	void IMPQ<key, lessThan>::increaseKey(const uint& Value, const key& Key)
	{
		{
			// Lock the mutex
			std::lock_guard<std::mutex> l(read_write_mutex);
			validateIndex(Value);
			if(contains(Key) == false)
				throw NoSuchElementException();
			if(_keys[Value] >= Key)
				throw IllegalArgumentException();
			
			_keys[Value] = Key;
			sink(_inverse_heap[Value]);
		}
	}
	
	/*
	 * Return the Key associated with Value.
	 * Throws NoSuchElementException()
	 * Throws InvalidIndexException() indirectly when the provided value is not in the container.
	*/
	template <typename key, bool lessThan>
	key IMPQ<key, lessThan>::keyOf(const uint& Value) const
	{
		{
			// Lock the mutex
			std::lock_guard<std::mutex> l(read_write_mutex);
			
			validateIndex(Value);
			
			if(contains(Value) == false)
				throw NoSuchElementException();
			else
				return _keys[Value];
		}
	}

	/*
	 * Restores the heap invariant, the parent-children relationship, starting at index k.
	 * Called internally by changeKey(), delMin(), increaseKey() and dequeue(). 
	 */
	template <typename key, bool lessThan>
	void IMPQ<key, lessThan>::sink(uint k)
	{
		// Only called by delMin(), so there is no need for a dedicated mutex here.
		
		// Exchange parent with its largest child to restore heap order.
		while(2*k <= _N)
		{
			uint j =  2*k;					// j is the left child.
			if(j < _N && compare(j, j+1))	// While j < _N, j+1 is safe to compare against.  If left child is < right, switch to the right...
				j++;
			if(compare(k, j) == false)		// and test it against parent.  If parent k > larger child, then heap order has been restored.
				break;
			
			exchange(k, j);					// Else, we exchange parent with its larger child.
			k = j;							// Lastly, we prepare to evaluate the left child the same way.
		}
	}
	
	/*
	 * Restores the heap invariant, the parent-children relationship, starting at index k.
	 * Called internally by changeKey(), decreaseKey(), enequeue() and dequeue().
	 */
	template <typename key, bool lessThan>
	void IMPQ<key, lessThan>::swim(uint k)
	{
		// Only called by insert(), so there is no need for a dedicated mutex here.
		
		// Exchange parent with its left child to restore heap order.
		while(k > 1 && compare(k/2, k))		// Swim up while parent is smaller than child.
		{
			exchange(k/2, k);
			k = k/2;
		}
	}
	
	/*
	 * Validates the provided Value/index.
	 * Called internally by changeKey(), decreaseKey(), enqueue(), increaseKey(), keyOf() and dequeue().
	 * Throws InvalidIndexException() when the value is not present in the container.
	 */
	template <typename key, bool lessThan>
	void IMPQ<key, lessThan>::validateIndex(const uint& Value) const
	{
		if(Value >= _size)
			throw InvalidIndexException();
	}

}

#endif
