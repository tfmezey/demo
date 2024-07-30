#ifndef REVERSE_LIST_H
#define REVERSE_LIST_H

#include <mutex>
#include <condition_variable>

#include "_containers.h"
#include "_containers_globals.h"

namespace containers 
{
	// This is a thread safe templated reverse_list class.  First in, last out.
	template <typename T>
	class reverse_list
	{

	// Implicitly private members and type:
		class node
		{
			public:
				node() {};
				node(T t) { this->t = t; }
				T t{};
				node* next = nullptr;
		};
		
		uint _count = 0;
		node* _first = nullptr;
		node* _current = nullptr;
		node* _last = nullptr;
		
		mutable std::mutex assignment;			// This mutex is used only by the copy assignment operator.
		mutable std::mutex mutex;				// This mutex is used by all other state changing public member functions.
		
	public:
		reverse_list() {};
		reverse_list(const reverse_list<T>&);
		reverse_list(reverse_list<T>&&) noexcept;
		~reverse_list() noexcept;
		reverse_list<T>& operator=(const reverse_list<T>&);
		reverse_list<T>& operator=(reverse_list<T>&&) noexcept;
		
		// getters
		T get(uint index=0) const;
		T get_first() const;
		T get_last() const;
		const T& operator[](uint) const;
		
		// setter
		void add(const T&);
		void add(const T*);
		void add(T&&);
		T& operator[](uint);
		
		uint size() const;
		bool empty() const;
		
		void clear();
		
		// Embedded iterator class for reverse_list objects.  Note that this is not thread safe.  To ensure that it is,
		// caller must coordinate its threads using a global mutex object.
		template <typename node_type, typename T2>
		struct re_iterator
		{
			//  iterators must be constructable, copy-constructable, copy-assignable, destructible and swappable.
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = T2;
			using pointer = node_type*;
			using reference = T2&;
			
			// This custom constructor satisfies the constructable requirement.  All others are created by the compiler.  However,
			// we will need a constant re_iterator, will require returning constant types and constant references.  These can be
			// created from iterators and constant iterators, but non-constant iterators cannot be created from constant iterators.
			// Thus we will need copy constructor and a copy assignment constructor.
			re_iterator() {};
			re_iterator(const pointer& ptr, const difference_type& i) { m_ptr = ptr; itemID = i; m_ptr_current = ptr; currentID = i; }
			re_iterator(const re_iterator& it) { m_ptr = it.m_ptr; itemID = it.itemID; m_ptr_current = it.m_ptr_current; currentID = it.currentID; }
			re_iterator& operator=(const re_iterator& it) { m_ptr = it.m_ptr; itemID = it.itemID; m_ptr_current = it.m_ptr_current; currentID = it.currentID; return *this; }
			
			// Note that our reverse_list class is a bag, and thus will not require modification of its
			// contents.  Nonetheless, std::sort needs the pre/post de/in-crement operators, the
			// difference_type returning addition/subtraction operators, along with the boolean
			// comparison operators.  We're providing these for completion and for future projects.
			
			// Access operators:
			reference operator*() const
			{
				// Return the reference to data at node of item number itemID the list.
				pointer pn = forward(itemID);
				
				return pn->t;
			}
			
			reference operator[](const difference_type& id) const
			{
				// Return the reference to data at node of item number id the list.
				pointer pn = forward(id);
				
				return pn->t;
			}
			
			T* operator->() const
			{
				// Return the node of item number itemID the list.
				pointer pn = forward(itemID);
				
				return &pn->t;
			}
			
			// Arithmetic operators:
			// prefix increment:  fetch re_iterator reference before incrementation of item
			re_iterator& operator++() { ++itemID; return *this; }
			// postfix increment:  increment item and fetch copy re_iterator
			re_iterator operator++(int) { return re_iterator(m_ptr, itemID++); }
			// prefix decrement:  decrement first, then return re_iterator
			re_iterator& operator--() { --itemID; return *this; }
			// postfix decrement:  return value, then decrement, calls --re_iterator
			re_iterator operator--(int) { return re_iterator(m_ptr, itemID--); }
			
			difference_type operator+(const re_iterator& rhs) { return itemID + rhs.itemID; }
			re_iterator operator+(const difference_type& rhs) { return re_iterator(m_ptr, itemID + rhs); }
			re_iterator& operator+=(const difference_type& rhs) { itemID += rhs; return *this; }
			
			difference_type operator-(const re_iterator& rhs) { return itemID - rhs.itemID; }
			re_iterator operator-(const difference_type& rhs) { return re_iterator(m_ptr, itemID - rhs); }
			re_iterator& operator-=(const difference_type& rhs) { itemID -= rhs; return *this; }
			
			// Boolean comparison operators:
			bool operator==(const re_iterator& rhs) const { return itemID == rhs.itemID; }
			bool operator!=(const re_iterator& rhs) const { return itemID != rhs.itemID; }
			bool operator<(const re_iterator& rhs) const { return itemID < rhs.itemID; }
			bool operator>(const re_iterator& rhs) const { return itemID > rhs.itemID; }
			bool operator<=(const re_iterator& rhs) const { return itemID <= rhs.itemID; }
			bool operator>=(const re_iterator& rhs) const { return itemID >= rhs.itemID; }
				
		private:
			//m_ptr is set to reverse_list::_first, itemID is the item number within the list that is changed externally via
			// the arithmetic operators, and currentID/m_ptr_current are the id/pointer pair needed for stateful iteration.
			pointer m_ptr = nullptr;
			mutable difference_type itemID = 0;			// This is reset to 0 when id in forward(id), is 0.  Thus we need mutability.
			mutable difference_type currentID = 0;		// This is reset to 0 when id in forward(id), is 0.  Thus we need mutability.
			mutable pointer m_ptr_current = nullptr;	// This is reset to m_ptr when id in forward(id), is 0.  Thus we need mutability.
			
			pointer forward(const difference_type& id) const
			{
				using namespace std;
				
				int old_currentID = currentID;
				int old_itemID = itemID;
				
				node_type* pn = nullptr;
				
				if(id == 0)
				{
					// Either the iterator has recently been created, or we are iterating again.
					m_ptr_current = m_ptr;
					currentID = 0;
					itemID = 0;
					
					if(_DEBUG == true)
					{
						using namespace std;
						// Not all types are compatible with std::cout, so print out numerical types only.
						if constexpr (is_integral<T2>::value or is_floating_point<T2>::value)
							cout << endl << "\tforward(" << id << ") = " << m_ptr->t;
						else
							cout << endl << "\tforward(" << id << ") = &" << m_ptr;
						
						cout << ", currentID:  " << old_currentID << " -> " << currentID;
						cout << ", itemID:  " << old_itemID << " -> " << itemID << endl;
					}
					
					return m_ptr_current;
				}
				
				if(m_ptr_current == nullptr)
					m_ptr_current = m_ptr;
				
				pn = m_ptr_current;

				int i = currentID;
				int loopcounter = 0;
				
				while(pn->next != nullptr)
				{
					if(i == id)
						break;
					
					loopcounter++;
					pn = pn->next;
					i++;
				}
				
				currentID = i;
				m_ptr_current = pn;
				
				if(_DEBUG == true)
				{
					// Not all types are compatible with std::cout, so print out numerical types only.
					if constexpr (is_integral<T2>::value or is_floating_point<T2>::value)
						cout << endl << "\tforward(" << id << ") = " << pn->t;
					else
						cout << endl << "\tforward(" << id << ") = &" << m_ptr;
					
					cout << ", loopcounter = " << loopcounter << " ";
					cout << ", currentID:  " << old_currentID << " -> " << currentID;
					cout << ", itemID:  " << old_itemID << " -> " << itemID << endl;
				}
				
				return pn;
			}
		};
		
		using iterator = re_iterator<node, T>;
		using const_iterator = re_iterator<node, const T>; 
		using citerator = re_iterator<node, const T>;
		
		// For iteration:
		iterator begin() { return iterator(_first, 0); }						
		iterator end() { return iterator(_last, _count); }					
		const_iterator cbegin() const { return const_iterator(_first, 0); }
		const_iterator cend() const { return const_iterator(_last, _count); }
		
		void get_iters(iterator& b, iterator& e) const { b = iterator(_first, 0); e = iterator(_last, _count); }
		void get_citers(citerator& b, citerator& e) const { b = citerator(_first, 0); e = citerator(_last, _count); }
	
	};
	
	template <typename T>
	reverse_list<T>::reverse_list(const reverse_list& src)
	{
		_count = src._count;
		for(int i = 0; i < _count; i++)
		{
			node* pn = new node;
			pn->t = src.get(i);
			if(i == 0)
			{
				_first = pn;
				_current = _first;
				_last = _first;
			}
			else
			{
				_current->next = pn;
				_current = pn;
				_last = _current;
			}
		}
	}
	
	template <typename T>
	reverse_list<T>::reverse_list(reverse_list&& src) noexcept
	{
		_count = src._count;
		_first = src._first;
		_current = src._current;
		_last = src._last;
		
		src._first = nullptr;
		src._current = nullptr;
		src._last = nullptr;
	}
	
	template <typename T>
	reverse_list<T>& reverse_list<T>::operator=(const reverse_list<T>& rhs)
	{
		/*
		 * We have two ways to add values:  Internally here, using code
		 * identical to add(), or by  use of the add() member function.
		 * As add() uses the main class mutex, we need a dedicated mutex
		 * for ourselves here.
		*/
		
		{
			// Lock the mutex
			std::lock_guard<std::mutex> l(assignment);
		
			node* pn = nullptr;
			
			// Delete everything we might already have.
			for(int i = 0; i < _count; i++)
			{
				pn = (*_first).next;
				delete _first;
				_first = pn;
			}
			
			for(int i = rhs._count - 1; i >= 0; i--)
				add(rhs.get(i));
			
// 			for(int i = 0; i < rhs._count; i++)
// 			{
// 				pn = new node;
// 				pn->t = rhs.get(i);
// 				if(_count == 0)
// 				{
// 					_first = pn;
// 					_current = pn;
// 					_last = pn;
// 				}
// 				else
// 				{
// 					// Last was set at _count == 0, and does not change.
// 					pn->next = _current;
// 					_current = pn;
// 					_first = pn;
// 				}
// 					
// 				_count++;
// 			}
		
		}
		
		return *this;
	}
	
	template <typename T>
	reverse_list<T>& reverse_list<T>::operator=(reverse_list<T>&& rhs) noexcept
	{
		{
			// Lock the mutex
			std::lock_guard<std::mutex> l(assignment);
			
			int count = _count;
			_count = rhs._count;
			rhs._count = count;
			
			node* temp = _first;
			_first = rhs._first;
			rhs._first = temp;
			
			temp = _current;
			_current = rhs._current;
			rhs._current = temp;
			
			temp = _last;
			_last = rhs._last;
			rhs._last = temp;
		}
		
		return *this;
	}
	
	template <typename T>
	reverse_list<T>::~reverse_list() noexcept
	{
		clear();
	}
	
	template <typename T>
	void reverse_list<T>::add(const T& t)
	{
		// Add T to the front.
		
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(mutex);
			node* pn = new node(t);
			
			if(_count == 0)
			{
				_first = pn;
				_current = pn;
				_last = pn;
			}
			else
			{
				// Last was set at _count == 0, and does not change.
				pn->next = _current;
				_current = pn;
				_first = pn;
			}
			
			_count++;
		}
	}
	
	template <typename T>
	void reverse_list<T>::add(const T* pt)
	{
		// Add T to the front.
		
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(mutex);
			node* pn = new node(*pt);
			
			if(_count == 0)
			{
				_first = pn;
				_current = pn;
				_last = pn;
			}
			else
			{
				// Last was set at _count == 0, and does not change.
				pn->next = _current;
				_current = pn;
				_first = pn;
			}
			
			_count++;
		}
	}
	
	template <typename T>
	void reverse_list<T>::add(T&& t)
	{
		// Add T to the front.
		
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(mutex);
			node* pn = new node(t);
			
			if(_count == 0)
			{
				_first = pn;
				_current = pn;
				_last = pn;
			}
			else
			{
				// Last was set at _count == 0, and does not change.
				pn->next = _current;
				_current = pn;
				_first = pn;
			}
			
			_count++;
		}
	}
	
	template <typename T>
	void reverse_list<T>::clear()
	{
		{
			// Lock the mutex
				std::unique_lock<std::mutex> ul(mutex);
				
			if(_first == nullptr)
				return;
			
			node* pn = _first;
			int i = 0;
			while(pn != nullptr)
			{
				node* temp = pn->next;
				delete pn;
				pn = temp;
				i++;
			}
			
			_count = 0;
			_first = nullptr;
			_current = nullptr;
			_last = nullptr;
		}
	}
	
	template <typename T>
	bool reverse_list<T>::empty() const
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(mutex);
			return _count == 0;
		}
	}
	
	template <typename T>
	uint reverse_list<T>::size() const
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(mutex);
			return _count;
		}
	}
	
	template <typename T>
	T reverse_list<T>::get_first() const
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(mutex);
			
			if(_count == 0)
				throw InvalidIndexException();
			else
				return _first->t;
		}	
	}
	
	template <typename T>
	T reverse_list<T>::get(uint index) const
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(mutex);
			
			if(_first == nullptr || index > _count)
				throw InvalidIndexException();
			
			node* pn = _first;
			int i = 0;
			while(pn != nullptr)
			{
				if(i == index)
					break;
				else
				{
					i++;
					pn = pn->next;
				}
			}
			
			return pn->t;
		}
	}
	
	template <typename T>
	T reverse_list<T>::get_last() const
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(mutex);
			if(_count == 0)
				throw InvalidIndexException();
			else
				return _last->t;
		}
	}
	
	template <typename T>
	const T& reverse_list<T>::operator[](uint index) const
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(mutex);
			
			if(_first == nullptr || index > _count)
				throw InvalidIndexException();
			
			node* pn = _first;
			int i = 0;
			while(pn != nullptr)
			{
				if(i == index)
					break;
				else
				{
					i++;
					pn = pn->next;
				}
			}
			
			return pn->t;
		}
	}
	
	template <typename T>
	T& reverse_list<T>::operator[](uint index)
	{
		{
			// Lock the mutex
			std::unique_lock<std::mutex> ul(mutex);
			
			if(_first == nullptr || index > _count)
				throw InvalidIndexException();
			
			node* pn = _first;
			int i = 0;
			while(pn != nullptr)
			{
				if(i == index)
					break;
				else
				{
					i++;
					pn = pn->next;
				}
			}
			
			return pn->t;
		}
	}
	
}

#endif