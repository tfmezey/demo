#ifndef QUEUE_H
#define QUEUE_H

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <sstream>

#include "_containers.h"
#include "_containers_globals.h"
#include "_allocator.h"

namespace containers
{
	using uint = containers::uint;
	
	// This is a thread safe templated queue (FIFO) class.
	
	template <typename T>
	class queue
	{
	public:
		queue();
		queue(const uint&);
		queue(const T*, const uint&);
		queue(const queue<T>&);
		queue(queue<T>&&) noexcept;
		queue<T>& operator=(const queue<T>&);
		queue<T>& operator=(queue<T>&&) noexcept;
		~queue() noexcept;
		
		void operator()(const T*, const uint&);
		
		void enqueue(const T*);
		void enqueue(const T&);
		void enqueue(T&&);
		T dequeue();
		
		uint size() const;
		bool empty() const;
		std::string str() const;
		
		void clear();
		
	private:
		
		int _head = 0;		// Location of the oldest item.
		int _tail = 0;		// Location after the newest item.
		int _size = 100;	// Size of the array, initially set to 100.
		int _count = 0;		// Number of items in the queue, _count = _tail - _head
		T* _Q = nullptr;
		
		// With mutable, we can modify these variables even in a const setting, namely in our const getters.
		// Note that in order to prevent races, we employ three different mutexes.
		mutable std::mutex read_write_mutex;		// Used by dequeue(), enqueue(), and the state querrying getters.
		mutable std::condition_variable notempty;	// Used by dequeue() on on an empty stack.
		mutable std::mutex resize_mutex;			// Used by the _resize() function.
		mutable std::mutex initialize_mutex;		// Used by _initialize().
		
		static _allocator<T>* const pal;			// Constant pointer to type T allocator.
	};
	
	
	template <typename T>
	_allocator<T>* const queue<T>::pal = &al<T>;
	
	template <typename T>
	queue<T>::queue()
	{
		_Q = pal->allocate(_size);
	}
	
	template <typename T>
	queue<T>::queue(const uint& size)
	{
		_size = size;
		_Q = pal->allocate(_size);
	}
	
	template <typename T>
	queue<T>::queue(const T* src, const uint& size)
	{
		// Construct from an external source.
		_size = size;
		_Q = pal->allocate(_size);
	}
	
	template <typename T>
	queue<T>::queue(const queue<T>& q)
	{
		_count = q._count;
		_head = q._head;
		_size = q._size;
		_tail = q._tail;
		
		_Q = pal->copyFromResize(q._Q, _Q);
	}
	
	template <typename T>
	queue<T>::queue(queue<T>&& q) noexcept
	{
		int count = _count;
		_count = q._count;
		q._count = count;
		
		int head = _head;
		_head = q._head;
		q._head = head;
		
		int size = _size;
		_size = q._size;
		q._size = size;
		
		int tail = _tail;
		_tail = q._tail;
		q._tail = tail;
		
		_Q = q._Q;
		q._Q = nullptr;
	}
	
	template <typename T>
	queue<T>& queue<T>::operator=(const queue<T>& rhs)
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> mlock(read_write_mutex);
			
			if(this == &rhs)
				return *this;
			
			_count = rhs._count;
			_head = rhs._head;
			_size = rhs._size;
			_tail = rhs._tail;
			
			_Q = pal->copyFromResize(rhs._Q, _Q);
			
			return *this;
		}
	}
	
	template <typename T>
	queue<T>& queue<T>::operator=(queue<T>&& rhs) noexcept
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> mlock(read_write_mutex);
			
			int count = _count;
			_count = rhs._count;
			rhs._count = count;
			
			int head = _head;
			_head = rhs._head;
			rhs._head = head;
			
			int size = _size;
			_size = rhs._size;
			rhs._size = size;
			
			int tail = _tail;
			_tail = rhs._tail;
			rhs._tail = tail;
			
			T* temp = _Q;
			_Q = rhs._Q;
			rhs._Q = temp;
			
			return *this;
		}
	}
	
	template <typename T>
	queue<T>::~queue() noexcept
	{
		pal->deallocate(_Q);
	}
	
	template <typename T>
	void queue<T>::operator()(const T* src, const uint& length)
	{
		// Copy from an external source, potentially resizing first.
		if(_size != length)
		{
			pal->deallocate(_Q);
			_Q = pal->allocate(length, length);
		}
		
		for(int i = 0; i < length; i++)
			_Q[i] = src[i];
	}
	
	template <typename T>
	T queue<T>::dequeue()
	{
		{	// Employ a dedicated scope of excecution so that we can have an implicit call to the mutex's destructor at the end of the scope.
			
			// Lock the mutex, but test for spurious wakeup by testing this->_count;
			std::unique_lock<std::mutex> mlock(read_write_mutex);
			
			/* That is, don't dequeue an empty queue.  Instead wait until there is at least one item.  With
			 * std::condition_variable.wait(std::unique_lock), we release our unique lock on the mutex to
			 * allow a producer thread to add an item.  Once our condition is met, all other threads are
			 * blocked and we return to work on the queue.
			*/
			while(this->_count == 0)		// equivalent to notempty.wait(mlock, [] { return _count == 0; });
				notempty.wait(mlock);		
			
			T n = _Q[_head];
			_count--;
			_head++;
				
			return n;
		}	// The destructor is called at the end of the context and the mutex is unlocked.
	}
	
	template <typename T>
	void queue<T>::enqueue(const T* pn)
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> mlock(read_write_mutex);
			
			if(pn == nullptr)
				return;
			
			if(_tail == _size)
			{
				// Here, enqueue-ing n would exceed the internal size of the queue, so first expand it.
				_Q = pal->resizeTo(_Q, _size*2);
			}
			
			_Q[_tail] = *pn;
			_count++;
			_tail++;
		}	
		
		notempty.notify_one();
	}
	
	template <typename T>
	void queue<T>::enqueue(const T& n)
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> mlock(read_write_mutex);
		
			if(_tail == _size)
			{
				// Here, enqueue-ing n would exceed the internal size of the queue, so first expand it.
				_Q = pal->resizeTo(_Q, _size*2);
			}

			_Q[_tail] = n;
			_count++;
			_tail++;
		}	
		
		notempty.notify_one();
	}
	
	template <typename T>
	void queue<T>::enqueue(T&& n)
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> mlock(read_write_mutex);
		
			if(_tail == _size)
			{
				// Enqueue-ing n would exceed the internal size of the queue, so first expand it.
				_Q = pal->resizeTo(_Q, _size*2);
			}
			
			_Q[_tail] = n;
			_count++;
			_tail++;
		}
		
		notempty.notify_one();
	}
	
	template <typename T>
	uint queue<T>::size() const
	{
		uint size;
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			size = _count;
		}
		
		return size;
	}
	
	template <typename T>
	bool queue<T>::empty() const
	{
		bool result;
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			result = _count == 0;
		}
		
		return result;
	}
	
	template <typename T>
	void queue<T>::clear()
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			
			pal->clear(_Q);
			_count = 0;
			_head = 0;
			_tail = 0;
		}
	}
	
	template <typename T>
	std::string queue<T>::str() const
	{
		using namespace std;
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			
			if(_count == 0)
				return "";

			ostringstream o;
			for(int i = _head; i < _tail - 1; i++)
				o <<  _Q[i] << ", ";

			o << _Q[_tail-1] << endl;

			return o.str();
		}
	}
}

#endif
