#ifndef ARRAY_H
#define ARRAY_H

#include <mutex>
#include <condition_variable>
#include <iostream>
#include <type_traits>
#include <typeinfo>

#include "_allocator.h"
#include "_containers.h"
#include "_containers_globals.h"

namespace containers 
{
	using namespace std;
	using uint = containers::uint;
	
	// This is a thread safe templated array class meant to store unordered data.
	template <typename T>
	class array
	{
		
		// Implicitly private members:
		static _allocator<T>* const pal;		// Constant pointer to type T allocator.
		static constexpr const uint _max_size = 1024*1024;
		static constexpr const uint offset = 1;
		void _resize() { _resize(2*_size); }
		void _resize(uint);
		void _validateIndex(const uint&) const;
		
		// _D = {0, [offset, _count + offset], ... , _size}.  We expand when _count reaches _size.
		uint _size = default_size;
		uint _count = 0;			// _count = [offset, _size), where _resize() gets called whenever _count == _size.
		T* _D = nullptr;
		
		// With mutable, we can modify these variables even in a const setting, namely in our const getters.
		mutable std::mutex read_write_mutex;
		mutable std::mutex _resize_mutex;
		
	public:
		array();
		array(uint);
		array(uint, uint);
		array(const array<T>&);
		array(array<T>&&) noexcept;
		~array() noexcept;
		array<T>& operator=(const array<T>&);
		array<T>& operator=(array<T>&&) noexcept;
		
		// getters.
		const T& operator[](uint) const;
		T& get(uint) const;
		T& get(uint);
		T& get_last() const;

		// setters
		T& operator[](uint);
		void add(const T&);			
		void add(T&&);
		void add(const T*);
		
		// Use these for random additions [0, _count)
		void addAt(const uint&, const T&);			
		void addAt(const uint&, T&&);
		void addAt(const uint&, const T*);
		
		uint size() const;
		bool empty() const;
		void clear();
		void reserve(uint);
		void shrink_to_fit();
		
		// Embedded iterator class for our arrays.  Note that this is not thread safe.  To ensure thread safety, the caller must
		// coordinate its threads.
		template <typename Type>
		struct a_iterator
		{
			//  iterators must be constructable, copy-constructable, copy-assignable, destructible and swappable.
			using iterator_category = std::random_access_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = Type;
			using pointer = Type*;
			using reference = Type&;
			
			// This custom constructor satisfies the constructable requirement.  All others are created by the compiler.  However,
			// we will need a constant iterator, will require returning constant types and constant references.  These can be
			// created from iterators and constant iterators, but non-constant iterators cannot be created from constant iterators.
			// Thus we will need copy constructor and a copy assignment constructor.
			a_iterator(pointer ptr) { m_ptr = ptr; }
			a_iterator(const a_iterator& it) { m_ptr = it.m_ptr; }
			a_iterator& operator=(const a_iterator& it) { m_ptr = it.m_ptr; return *this; }
			a_iterator() {};
			
			// Access operators:
			reference operator*() const { return *m_ptr; }
			pointer operator->() const { return m_ptr; }
			reference operator[](const difference_type& i) const { return m_ptr[i]; }
			
			// Note that std::sort needs the pre/post de/in-crement operators, the difference_type
			// returning addition/subtraction operators, along with the boolean comparison operators.
			
			// Arithmetic operators:
			// prefix increment:  fetch a_iterator reference before incrementation of index
			a_iterator& operator++() { ++m_ptr; return *this; }
			// postfix increment:  increment index and fetch copy a_iterator
			a_iterator operator++(int) { return a_iterator(m_ptr++); }
			
			// prefix decrement:  decrement first, then return a_iterator
			a_iterator& operator--() { --m_ptr; return *this; }
			// postfix decrement:  return value, then decrement, calls --a_iterator
			a_iterator operator--(int) { return a_iterator(m_ptr--); }
			
			difference_type operator+(const a_iterator& it) { return m_ptr + it.m_ptr; }
			a_iterator operator+(const difference_type& n) { return a_iterator(m_ptr + n); }
			a_iterator& operator+=(const difference_type& n) { m_ptr += n; return *this; }
			
			difference_type operator-(const a_iterator& it) { return m_ptr - it.m_ptr; }
			a_iterator operator-(const difference_type& n) { return a_iterator(m_ptr - n); }
			a_iterator& operator-=(const difference_type& n) { m_ptr -= n; return *this; }
			
			// boolean comparison operators
			bool operator==(const a_iterator& rhs) const { return m_ptr == rhs.m_ptr; }
			bool operator!=(const a_iterator& rhs) const { return m_ptr != rhs.m_ptr; }
			bool operator<(const a_iterator& rhs) const { return m_ptr < rhs.m_ptr; }
			bool operator>(const a_iterator& rhs) const { return m_ptr > rhs.m_ptr; }
			bool operator<=(const a_iterator& rhs) const { return m_ptr <= rhs.m_ptr; }
			bool operator>=(const a_iterator& rhs) const { return m_ptr >= rhs.m_ptr; }
			
		private:
			pointer m_ptr = nullptr;
		};

		// Embedded reverse iterator class for our arrays.  Note that this is not thread safe.  To ensure thread safety, the caller must
		// coordinate its threads.
		template <typename Type>
		struct a_riterator
		{
			//  iterators must be constructable, copy-constructable, copy-assignable, destructible and swappable.
			using iterator_category = std::random_access_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = Type;
			using pointer = Type*;
			using reference = Type&;

			// This custom constructor satisfies the constructable requirement.  All others are created by the compiler.  However,
			// we will need a constant iterator, will require returning constant types and constant references.  These can be
			// created from iterators and constant iterators, but non-constant iterators cannot be created from constant iterators.
			// Thus we will need copy constructor and a copy assignment constructor.
			a_riterator(pointer ptr) { m_ptr = ptr; }
			a_riterator(const a_riterator& it) { m_ptr = it.m_ptr; }
			a_riterator& operator=(const a_riterator& it) { m_ptr = it.m_ptr; return *this; }
			a_riterator() {};

			// Access operators:
			reference operator*() const { return *m_ptr; }
			pointer operator->() const { return m_ptr; }
			reference operator[](const difference_type& i) const { return m_ptr[i]; }

			// Note that std::sort needs the pre/post de/in-crement operators, the difference_type
			// returning addition/subtraction operators, along with the boolean comparison operators.

			// Arithmetic operators:
			// prefix increment:  fetch a_iterator reference before incrementation of index
			a_riterator& operator++() { --m_ptr; return *this; }
			// postfix increment:  increment index and fetch copy a_iterator
			a_riterator operator++(int) { return a_riterator(m_ptr--); }

			// prefix decrement:  decrement first, then return a_iterator
			a_riterator& operator--() { ++m_ptr; return *this; }
			// postfix decrement:  return value, then decrement, calls --a_iterator
			a_riterator operator--(int) { return a_riterator(m_ptr++); }

			difference_type operator+(const a_riterator& it) { return m_ptr - it.m_ptr; }
			a_riterator operator+(const difference_type& n) { return a_riterator(m_ptr - n); }
			a_riterator& operator+=(const difference_type& n) { m_ptr -= n; return *this; }

			difference_type operator-(const a_riterator& it) { return m_ptr + it.m_ptr; }
			a_riterator operator-(const difference_type& n) { return a_riterator(m_ptr + n); }
			a_riterator& operator-=(const difference_type& n) { m_ptr += n; return *this; }

			// boolean comparison operators
			bool operator==(const a_riterator& rhs) const { return m_ptr  == rhs.m_ptr; }
			bool operator!=(const a_riterator& rhs) const { return m_ptr  != rhs.m_ptr; }
			bool operator<(const a_riterator& rhs) const { return m_ptr > rhs.m_ptr; }
			bool operator>(const a_riterator& rhs) const { return m_ptr < rhs.m_ptr; }
			bool operator<=(const a_riterator& rhs) const { return m_ptr >= rhs.m_ptr; }
			bool operator>=(const a_riterator& rhs) const { return m_ptr <= rhs.m_ptr; }

		private:
			pointer m_ptr = nullptr;
		};
		
		using iterator = a_iterator<T>;
		using const_iterator = a_iterator<const T>;
		using citerator = a_iterator<const T>;
		using riterator = a_riterator<T>;
		using const_riterator = a_riterator<const T>;
		using criterator = a_riterator<const T>;
		
		// Integrate the above iterator into our class:
		iterator begin() { return iterator(_D+offset); }								// Return the first element in [offset,_count]
		iterator end() { return iterator(_D+_count+offset); }							// Return the last element in [offset,_count]
		const_iterator begin() const { return const_iterator(_D+offset); }				// Return the first element in [offset,_count]
		const_iterator end() const { return const_iterator(_D+_count+offset); }			// Return the last element in [offset,_count]
		const_iterator cbegin() const { return const_iterator(_D+offset); }				// Return the first element in [offset,_count]
		const_iterator cend() const { return const_iterator(_D+_count+offset); }		// Return the last element in [offset,_count]

		riterator rbegin() { return riterator(_D+_count); }								// Return the last element in [offset,_count]
		riterator rend() { return riterator(_D); }										// Return the first element in [offset,_count]
		const_riterator rbegin() const { return const_riterator(_D+_count); }			// Return the last element in [offset,_count]
		const_riterator rend() const { return const_riterator(_D); }					// Return the first element in [offset, _count]
		const_riterator crbegin() const { return const_riterator(_D+_count); }			// Return the last element in [offset,_count]
		const_riterator crend() const { return const_riterator(_D); }					// Return the first element in [offset,_count]
		
		void get_iters(iterator& b, iterator& e) const { b = iterator(_D+offset); e = iterator(_D+_count+offset); }
		void get_citers(citerator& b, citerator& e) const { b = citerator(_D+offset); e = citerator(_D+_count+offset); }

		void get_riters(riterator& b, riterator& e) const { b = riterator(_D+_count); e = riterator(_D); }
		void get_criters(criterator& b, criterator& e) const { b = criterator(_D+_count); e = criterator(_D); }
	};
	
	template <typename T>
	_allocator<T>* const array<T>::pal = &al<T>;
	
	template <typename T>
	array<T>::array()
	{
		_D = pal->allocate(_size);
	}
	
	template <typename T>
	array<T>::array(uint size)
	{
		// Client wants size number of items, so we need to allocate one more than that, unless we reach _max_size or if it is less than default_size.
		if(size != default_size)
			_size = size + 1 + offset;
		
		if(_size > _max_size)
			_size = _max_size;
		
		_D = pal->allocate(_size);
	}
	
	template <typename T>
	array<T>::array(uint size, uint amount)
	{
		// Client wants size number of items, so we need to allocate one more than that, unless we reach _max_size.
		// Also set count to amount.
		uint max = size > default_size ? size : default_size;
		_size = max > amount ? max + 1 + offset : amount + 1 + offset;
		
		if(_size > _max_size)
			_size = _max_size;
		
		_D = pal->allocate(_size);
		_count = amount;
	}
	
	template <typename T>
	array<T>::array(const array<T>& a)
	{
		_size = a._size;
		_count = a._count;
		_D = pal->copyFromResize(a._D, _D);
	}
	
	template <typename T>
	array<T>::array(array<T>&& rhs) noexcept
	{
		int size = _size;
		_size = rhs._size;
		rhs._size = size;
		
		int count = _count;
		_count = rhs._count;
		rhs._count = count;
		
		_D = rhs._D;
		rhs._D = nullptr;
	}
	
	template <typename T>
	array<T>& array<T>::operator=(const array<T>& rhs)
	{
		{
			// Lock the mutex
			std::lock_guard<std::mutex> l(read_write_mutex);
		
			if(this == &rhs)
				return *this;
			
			_count = rhs._count;
			_size = rhs._size;
			_D = pal->copyFromResize(rhs._D, _D);
		}
		
		return *this;
	}
	
	template <typename T>
	array<T>& array<T>::operator=(array<T>&& rhs) noexcept
	{
		{
			// Lock the mutex
			std::lock_guard<std::mutex> l(read_write_mutex);
			
			if(this == &rhs)
				return *this;
			
			int size = _size;
			_size = rhs._size;
			rhs._size = size;
			
			int count = _count;
			_count = rhs._count;
			rhs._count = count;
			
			T* temp = _D;
			_D = rhs._D;
			rhs._D = temp;
			
			return *this;
		}
	}
	
	template <typename T>
	array<T>::~array() noexcept
	{
		pal->deallocate(_D);
	}
	
	template <typename T>
	void array<T>::_resize(uint requestedSize)
	{
		// By default requestedSize = 0, and we normally double the previous allocation.  If it
		// is not zero, then it was called by reserve(), and we attemp to honour the request,
		// subject to the _max_size constraint.
		
		{
			// Our lock_guard is locked once and discarded (unlocked) by desctruction.
			std::lock_guard<std::mutex> ul(_resize_mutex);
			
			int newsize = 0;
			if(requestedSize != 0)
			{
				if(requestedSize <= _size)			// An erronous request, but recoverable.
					newsize = _size*2;
				else if(requestedSize > _max_size)
					throw AllocationLimitException();
				else
					newsize = requestedSize;		// An acceptable request.
			}
			else									// Default behaviour.
			{
				if(_size*2 > _max_size)
					throw AllocationLimitException();
				else
					newsize = _size*2;
			}
			
			_D = pal->resizeTo(_D, newsize);
			_size = newsize;
		}
	}
	
	template <typename T>
	bool array<T>::empty() const
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			return _count == 0;
		}
	}
	
	template <typename T>
	uint array<T>::size() const
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			return _count;
		}
	}
	
	template <typename T>
	T& array<T>::operator[](uint index)
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			
			// Sanity checks:
			if(index > offset && index >= _count)
				reserve(index);

			return _D[index + offset];
		}
	}
	
	template <typename T>
	const T& array<T>::operator[](uint index) const
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			
			// Sanity checks:
			if(index > offset && index > _count)
				throw InvalidIndexException();

			return _D[index + offset];
		}
	}
	
	template <typename T>
	T& array<T>::get(uint index)
	{
		// index \in [0, _count)
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			
			// Sanity checks:
			// if(index > offset && index >= _count)
			if(index >= _count)
				reserve(index);

			return _D[index + offset];
		}	
	}
	
	template <typename T>
	T& array<T>::get(uint index) const
	{
		// Note there is no error mitigation as this is a constant getter.
		// Thus, this function can be unsafe.
		
		// index \in [0, _count)
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			
			return _D[index + offset];
		}	
	}
	
	template <typename T>
	T& array<T>::get_last() const
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);

			// Sanity check.  Note in an empty array, index equals _count.
			if(_count == 0)
				return _D[offset];
			else
				return _D[_count - 1 + offset];
		}
	}
	
	template <typename T>
	void array<T>::clear()
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			pal->clear(_D);
			_count = 0;
		}
	}
	
	template <typename T>
	void array<T>::add(const T& t) 
	{
		// Add t at next location.  If the index is > _count, then we will treat it as an allocation request as well, 
		// setting _count to index, making the assignment at index, and then restoring index \in [0, _count) via _count++.
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			
			if(_count == _size - offset)
				_resize();
			
			_D[_count++ + offset] = t;
		}
	}
	
	template <typename T>
	void array<T>::add(T&& t) 
	{
		// Add t at given index.  If the index is > _count, then we will treat it as an allocation request as well, 
		// setting _count to index, making the assignment at index, and then restoring index \in [0, _count) via _count++.
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			
			if(_count == _size - offset)
				_resize();
			
			_D[_count++ + offset] = t;
		}
	}
	
	template <typename T>
	void array<T>::add(const T* pt) 
	{
		// Add t at given index.  If the index is > _count, then we will treat it as an allocation request as well, 
		// setting _count to index, making the assignment at index, and then restoring index \in [0, _count) via _count++.
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			
			if(_count == _size - offset)
				_resize();
			
			_D[_count++ + offset] = *pt;
		}
	}
	
	template <typename T>
	void array<T>::addAt(const uint& index, const T& t) 
	{
		// Add t at given index in [offset, _count].  If the index is > _count, then we will treat it as an allocation
		// request as well, setting _count to index, making the assignment at index, and then restoring
		// index \in [offset, _count] via _count++.

		// Add t at given index.  If the index is > _count, then we will treat it as an allocation request as well, 
		// setting _count to index, making the assignment at index, and then restoring index \in [0, _count) via _count++.
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			
			// Sanity checks:
			if(index > offset && index >= _count)
			{
				reserve(index);
				_D[index + offset] = t;
			}
			else
			{
				_D[index + offset] = t;
				_count++;
			}
		}
	}
	
	template <typename T>
	void array<T>::addAt(const uint& index, T&& t) 
	{
		// Add t at given index.  If the index is > _count, then we will treat it as an allocation request as well, 
		// setting _count to index, making the assignment at index, and then restoring index \in [0, _count) via _count++.
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			
			// Sanity checks:
			if(index > offset && index >= _count)
			{
				reserve(index);
				_D[index + offset] = t;
			}
			else
			{
				_D[index + offset] = t;
				_count++;
			}
		}
	}
	
	template <typename T>
	void array<T>::addAt(const uint& index, const T* pt) 
	{
		// Add t at given index.  If the index is > _count, then we will treat it as an allocation request as well, 
		// setting _count to index, making the assignment at index, and then restoring index \in [0, _count) via _count++.
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);
			
			// Sanity checks:
			if(index > offset && index >= _count)
			{
				reserve(index);
				_D[index + offset] = *pt;
			}
			else
			{
				_D[index + offset] = *pt;
				_count++;
			}
		}
	}

	template <typename T>
	void array<T>::reserve(uint amount)
	{
		// Request work area to be [0, amount].  Thus set _count to one larger.  Resize as needed.
		// Note at amount = 0, we have a clear() operation.
		if(amount + 1 >= _size)
			_resize();
		
		_count = amount + 1;
	}

	template <typename T>
	void array<T>::shrink_to_fit()
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(read_write_mutex);

			// Sanity check:
			if(_count == 0)
				return;

			T* temp = pal->allocate(_count);
			// T* temp = new T[_count];
			for(int i = 0; i <= _count; i++)		// data \in [offset, _count]
				// temp[i] = std::move(_D[i]);
				temp[i] = std::move_if_noexcept(_D[i]);

			// delete[] _D;
			pal->deallocate(_D);
			_D = temp;
		}
	}
}

#endif
