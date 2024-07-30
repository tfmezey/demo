#ifndef LINE_H
#define LINE_H

#include <iterator>

#include "_algorithms.h"
#include "containers.h"

namespace algorithms
{
	/*
	 * This class is meant to be used to store a line of text, by default being 80 characters in length.
	*/
	
	using char_allocator = containers::_allocator<char>;
	extern char_allocator al;
	
	class line final
	{
	public:
		
		// Big 5 + 1
		line() noexcept;
		line(uint) noexcept;
		line(const line&) noexcept;
		line(line&&) noexcept;
		line& operator=(const line&) noexcept;
		line& operator=(line&&) noexcept;
		~line() noexcept;
		
		void set(const char*, uint);
		void clear();
		
		uint length() const { return _length; }
		
		bool operator==(const line&) const;
		bool operator!=(const line&) const;
		bool operator<(const line&) const;
		bool operator<=(const line&) const;
		bool operator>(const line&) const;
		bool operator>=(const line&) const;
		
		// Embedded iterator class for our arrays.  Note that this is not thread safe.  To ensure thread safety, the caller must
		// coordinate its threads using a global mutex object.
		template <typename Type>
		struct line_iterator
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
			line_iterator(pointer ptr) { m_ptr = ptr; }
			line_iterator(const line_iterator& it) { m_ptr = it.m_ptr; }
			line_iterator& operator=(const line_iterator& it) { m_ptr = it.m_ptr; return *this; }
			line_iterator() {};
			
			// Access operators:
			reference operator*() const { return *m_ptr; }
			pointer operator->() const { return m_ptr; }
			reference operator[](const difference_type& i) const { return m_ptr[i]; }
			
			// Note that std::sort needs the pre/post de/in-crement operators, the difference_type
			// returning addition/subtraction operators, along with the boolean comparison operators.
			
			// Arithmetic operators:
			// prefix increment:  fetch line_iterator reference before incrementation of index
			line_iterator& operator++() { ++m_ptr; return *this; }
			// postfix increment:  increment index and fetch copy line_iterator
			line_iterator operator++(int) { return line_iterator(m_ptr++); }
			
			// prefix decrement:  decrement first, then return line_iterator
			line_iterator& operator--() { --m_ptr; return *this; }
			// postfix decrement:  return value, then decrement, calls --line_iterator
			line_iterator operator--(int) { return line_iterator(m_ptr--); }
			
			difference_type operator+(const line_iterator& it) { return m_ptr + it.m_ptr; }
			line_iterator operator+(const difference_type& n) { return line_iterator(m_ptr + n); }
			line_iterator& operator+=(const difference_type& n) { m_ptr += n; return *this; }
			
			difference_type operator-(const line_iterator& it) { return m_ptr - it.m_ptr; }
			line_iterator operator-(const difference_type& n) { return line_iterator(m_ptr - n); }
			line_iterator& operator-=(const difference_type& n) { m_ptr -= n; return *this; }
			
			// boolean comparison operators
			bool operator==(const line_iterator& rhs) const { return m_ptr  == rhs.m_ptr; }
			bool operator!=(const line_iterator& rhs) const { return m_ptr  != rhs.m_ptr; }
			bool operator<(const line_iterator& rhs) const { return m_ptr < rhs.m_ptr; }
			bool operator>(const line_iterator& rhs) const { return m_ptr > rhs.m_ptr; }
			bool operator<=(const line_iterator& rhs) const { return m_ptr <= rhs.m_ptr; }
			bool operator>=(const line_iterator& rhs) const { return m_ptr >= rhs.m_ptr; }
			
		private:
			pointer m_ptr = nullptr;
		};
		
		using iterator = line_iterator<char>;
		using const_iterator = line_iterator<const char>;
		using citerator = line_iterator<const char>;
		
		// Integrate the above iterator into our class:
		iterator begin() { return iterator(m_ptr); }							// Return the first element in [0,_size)
		iterator end() { return iterator(&m_ptr[_length]); }					// Return the last element in [0, _size)
		const_iterator begin() const { return const_iterator(m_ptr); }			// Return the first element in [0,_size)
		const_iterator end() const { return const_iterator(&m_ptr[_length]); }	// Return the last element in [0, _size)
		const_iterator cbegin() const { return const_iterator(m_ptr); }			// Return the first element in [0,_size)
		const_iterator cend() const { return const_iterator(&m_ptr[_length]); }	// Return the last element in [0, _size)
		
		void get_iters(iterator& b, iterator& e) const { b = iterator(m_ptr); e = iterator(&m_ptr[_length]); }
		void get_citers(citerator& b, citerator& e) const { b = citerator(m_ptr); e = citerator(&m_ptr[_length]); }
		
		void resize(uint);
		
	private:
		static char_allocator* const pal;
		char* m_ptr = nullptr;
		constexpr static const uint DEFAULT_BUFFER_SIZE = 80;
		uint _size = DEFAULT_BUFFER_SIZE;
		uint _length = 0;
	};
	
}

#endif
