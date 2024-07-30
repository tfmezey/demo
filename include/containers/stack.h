#ifndef STACK_H
#define STACK_H

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <sstream>

#include "_containers.h"
#include "_allocator.h"
#include "_containers_globals.h"

namespace containers
{
	using uint = containers::uint;
	
	// This is a sufficiently thread-safe LIFO stack.
	 
	template <typename T>
	class stack
	{

	// Implicitly private members:
		
		static _allocator<T>* const pal;			// Constant pointer to type T allocator.
		
		// _S = {0, [offset, _top - offset], _top, ..., _size}.  We expand when _top reaches _size.
		uint _size = _allocate;
		uint _top = 0;
		
		T* _S = nullptr;
		
		// With mutable, we can modify these variables even in a const setting, namely in our const getters.
		// Note that in order to prevent races, we employ three different mutexes.
		mutable std::mutex read_write_mutex;		// Used by push(), pop(), and the state querrying getters.
		mutable std::condition_variable notempty;	// Used by pop() on on an empty stack.
		mutable std::mutex resize_mutex;			// Used by the _resize() function.
		mutable std::mutex initialize_mutex;		// Used by _initialize().
		
	public:
		stack();
		stack(const uint&);
		stack(const T*, const uint&);
		stack(const stack<T>&);
		stack(stack<T>&&) noexcept;
		stack<T>& operator=(const stack<T>&);
		stack<T>& operator=(stack<T>&&) noexcept;
		~stack() noexcept; 
		
		std::string str() const;
		void operator()(const T*, const uint&);
		void clear();
		uint size() const;
		bool empty() const;
		
		void push(const T&);
		void push(const T*);
		void push(T&&);
		T pop();
	};
	
	template <typename T>
	_allocator<T>* const stack<T>::pal = &al<T>;
	
	template <typename T>
	stack<T>::stack()
	{
		_S = pal->allocate(_size);
	}
	
	template <typename T>
	stack<T>::stack(const uint& length)
	{
		_size = length;
		_S = pal->allocate(_size);
	}
	
	template <typename T>
	stack<T>::stack(const T* s, const uint& length)
	{
		// Construct from an external source.
		_size = length;
		_S = pal->allocate(_size);
		
		for(int i = 0; i < length; i++)
			_S[i] = s[i];
	}
	
	template <typename T>
	stack<T>::stack(const stack<T>& s)
	{
		_size = s._size;
		_top = s._top;
		_S = pal->copyFromResize(s._S, _S);
	}
	
	template <typename T>
	stack<T>::stack(stack<T>&& s) noexcept
	{
		int size = s._size;
		_size = s._size;
		s._size = size;
		
		int top = _top;
		_top = s._top;
		s._top = top;
		
		_S = s._S;
		s._S = nullptr;
	}
	
	template <typename T>
	stack<T>& stack<T>::operator=(const stack<T>& s)
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			
			if(this == &s)
				return *this;
			
			_size = s._size;
			_top = s._top;
			_S = pal->copyFromResize(s._S, _S);
			
			return *this;
		}
	}
	
	template <typename T>
	stack<T>& stack<T>::operator=(stack<T>&& s) noexcept
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			
			int size = s._size;
			_size = s._size;
			s._size = size;
			
			int top = _top;
			_top = s._top;
			s._top = top;
			
			T* temp = _S;
			_S = s._S;
			s._S = temp;
			
			return *this;
		}
	}
	
	template <typename T>
	stack<T>::~stack() noexcept
	{
		pal->deallocate(_S);
	}
	
	template <typename T>
	void stack<T>::clear()
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			
			pal->clear(_S);
			_top = 0;
		}
	}
	
	template <typename T>
	void stack<T>::operator()(const T *src, const uint& length)
	{
		// Copy from an external source, potentially resizing first.
		if(_size != length)
		{
			pal->deallocate(_S);
			_S = pal->allocate(length, length);
		}
		
		for(int i = 0; i < length; i++)
			_S[i] = src[i];
	}
	
	template <typename T>
	uint stack<T>::size() const
	{
		// _S = { 0, [offset, _top], ..., _size}
		uint size;
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			size = _top;
		}
		
		return size;
	}
	
	template <typename T>
	bool stack<T>::empty() const
	{
		bool result;
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			result = _top == 0;
		}
		
		return result;
	}
	
	template <typename T>
	void stack<T>::push(const T& t)
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);

			// _S = {0, [offset, _top - offset], _top, _size}.  Expand when _top reaches _size.
			if(_top == _size )
			{
				_S = pal->resizeTo(_S, _size*2);
				_size = _size * 2;
			}
			
			_S[_top] = t;
			_top++;
		}
		
		// If there was an empty queue previous to a pop() call, then there is a thread that is waiting.  Notify that thread.
		notempty.notify_one();
	}
// 	
	template <typename T>
	void stack<T>::push(const T* pt)
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			
			if(pt == nullptr)
				return;
			
			// _S = {0, [offset, _top - offset], _top, _size}.  Expand when _top reaches _size.
			
			if(_top == _size)
			{
				_S = pal->resizeTo(_S, _size*2);
				_size = _size * 2;
			}
			
			_S[_top] = *pt;
			_top++;
		}
		
		// If there was an empty queue previous to a pop() call, then there is a thread that is waiting.  Notify that thread.
		notempty.notify_one();
	}
	
	template <typename T>
	void stack<T>::push(T&& t)
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			
			// _S = {0, [offset, _top - offset], _top, _size}.  Expand when _top reaches _size.
			if(_top == _size)
			{
				_S = pal->resizeTo(_S, _size*2);
				_size = _size * 2;
			}
			
			_S[_top] = t;
			_top++;
		}
		
		// If there was an empty queue previous to a pop() call, then there is a thread that is waiting.  Notify that thread.
		notempty.notify_one();
	}
	
	template <typename T>
	T stack<T>::pop()
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			
			/* That is, don't pop an empty stack.  Instead wait until there is at least one item.  With
			 * std::condition_variable.wait(std::unique_lock), we release our unique lock on the mutex to
			 * allow a producer thread to add an item.  Once our condition is met, all other threads are
			 * blocked and we return to work on the queue.
			*/
			while(_top == 0)		// equivalent to notempty.wait(mlock, [] { return _count == 0; });
				notempty.wait(ul);	
			
			_top--;
			
			// [0, _top) --"_top--"-> [0, _top] with regards to popped data.  Post pop(), we will return to [0, _top).
			return _S[_top];
		}
	}

	template <typename T>
	std::string stack<T>::str() const
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);

			if(_top == 0)
				return "";

			std::ostringstream o;

			for(int i = _top; i >= 0; i--)
				o << _S[i] << ", ";

			return o.str();
		}
	}
	
}


#endif
