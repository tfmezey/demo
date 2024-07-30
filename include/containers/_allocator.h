#ifndef CONTAINERS_ALLOCATOR_H
#define CONTAINERS_ALLOCATOR_H

#include "_containers.h"

namespace containers
{
		
	template <typename T>
	class _allocator final
	{
		/*
		 * This is a templated allocator class for our containers, supporting a consistent API for all allocations within
		 * our containers.  The API provides a T* returning resizeTo(uint) function, a deallocate(T*) function, as well as
		 * a T* returning allocate(uint) function.  We also support copy construction/assignment via copyFromResize(), as well
		 * as a reinitialization of the allocation via the function clear().
		 * 
		 * We have a forward list of nodes that contain an allocation of type T, along with the allocation size.
		 *
		 * The resizeTo() operation entails making a new and larger allocation, copying the original content (hence the need
		 * to store the size of our allocations) into the new allocation, and default initializing the rest of the larger
		 * allocation.
		 * 
		 * The deallocate() operation removes a node from our forward list, after deallocating the stored allocation.
		 * 
		 * On error we throw the following exceptions, defined in _containers_exceptions.h:
		 * 		AllocationException, AllocationLimitException and IllegalArgumentException.
		 * 
		 * For now we support a isPresent(T*) querry, as well as the getSize() querry of the allocator.  This may be made
		 * private for debug purposes!
		 * 
		*/
		
		class node
		{
			public:
				node() {};
				node(T* p, uint size) { _p = p; _size = size; }
				~node() { if(_p != nullptr) delete[] _p; }
				T* _p = nullptr;
				node* _next = nullptr;
				uint _size = 0;
		};
		
		node* _first = nullptr;
		node* _last = nullptr;
		uint _allocations = 0;
		
		void _addNode(T*, uint);
		void _removeNode(T*);
		node* _getNode(const T* const&);			// Used internally to obtain the node of a confirmed to be present pointer.
		
	public:		
		_allocator(const _allocator&) = delete;
		_allocator(_allocator&&) = delete;
		_allocator& operator=(const _allocator&) = delete;
		_allocator& operator=(_allocator&&) = delete;
		_allocator() {};
		~_allocator();
		
		bool isPresent(const T* const & p) const;
		uint getSize() const { return _allocations; }
		
		void clear(T*&);
		T* allocate(uint size, uint initializeAt=0);		// Allocate with initialization of base types starting from second parameter.
		T* resizeTo(T*&, uint);								// Resizes an existing allocation.  Throws exceptions.
		T* copyFromResize(const T* const&, T*&);			// To support copy construction/assignment operatations.
		void deallocate(T*&);								// Deallocates an existing allocation.  Throws exceptions.
	};
	
	template <typename T>
	_allocator<T>::~_allocator()
	{
		if(_first == nullptr)
			return;
		
		node* current = _first;
		node* next = _first->_next;
		
		while(current != nullptr)
		{
			next = current->_next;
			delete current;
			current = next;
		}
	}
	
	template <typename T>
	void _allocator<T>::_addNode(T* p, uint size)
	{
		node* n = new node(p, size);
		if(_first == nullptr)
			_first = n;
		else
			_last->_next = n;
		
		_last = n;
	}
	
	template <typename T>
	void _allocator<T>::_removeNode(T* p)
	{
		node* current = _first;
		node* previous = _first;
		node* next = nullptr;
		while(current != nullptr)
		{
			next = current->_next;
			if(current->_p == p)
			{
				if(_first == current)
					_first = next;
				else if(_last == current)
				{
					_last = previous;
					_last->_next = nullptr;
				}
				else
					previous->_next = next;	
			
				delete current;
				_allocations--;
				
				if(_allocations == 0)
					_first = _last = nullptr;
				
				break;
			}
			else
			{
				previous = current;
				current = current->_next;
			}
		}
	}
	
	template <typename T>
	T* _allocator<T>::allocate(uint size, uint initializeAt)
	{
		/*
		 * After we allocate the "size" sized array, we will start initializing the allocation for base bool, 
		 * uint and double types at initializeAt.  If we are provided an invalid value, we initialize from the
		 * beginning, which is the default behaviour.
		 * 
		 * The purpose of this is to only initialize those values that will not immediately be filled with values
		 * from another allocation, as found in resize or copy construction operations.
		*/
		
		if(size > _max_size)
			throw AllocationLimitException();
		
		if(initializeAt > size)
			initializeAt = 0;
		
		T* t = nullptr;
		t = new T[size];
		
		if constexpr (std::is_same<T, bool>::value)
		{
			for(int i = initializeAt; i < size; i++)
				t[i] = false;
		}
		else if constexpr (std::is_same<T, char>::value)
		{
			for(int i = initializeAt; i < size; i++)
				t[i] = 0;
		}
		else if constexpr (std::is_same<T, unsigned int>::value)
		{
			for(int i = initializeAt; i < size; i++)
				t[i] = undefined_uint;
		}
		else if constexpr (std::is_same<T, double>::value)
		{
			for(int i = initializeAt; i < size; i++)
				t[i] = inf;
		}
		
		if(t == nullptr)
			throw AllocationException();
		_addNode(t, size);
		
		_allocations++;
		return t;
	}
	
	template <typename T>
	bool _allocator<T>::isPresent(const T* const& p) const
	{
		if(_first == nullptr)
			return false;
		
		node* current = _first;
		node* previous = _first;
		node* next = nullptr;
		
		while(current != nullptr)
		{
			next = current->_next;
			if(current->_p == p)
				return true;
			else
			{
				previous = current;
				current = current->_next;
			}
		}
		
		return false;
	}
	
	template <typename T>
	T* _allocator<T>::resizeTo(T*& original, uint size)
	{
		/*
		 * Resize the original allocation to size uints.
		 * Note that this should only be used by containers on themselves.  This also implies that
		 * we throw exceptions when original was not allocated by this allocator!  We also require
		 * that size > originalSize, as we don't support clear() operations, which occur when
		 * size == originalSize, or truncation operations, which occurr when size < originalSize.
		 * 
		 * We std::move_if_noexcept from [0, originalSize) of the original allocation to the
		 * new allocation.  Note that new[] calls the move constructor on objects, and we handle
		 * default initialization on bool, unsigned int and double types.
		 * 
		*/
		
		// We require the size of the new allocation to be greater than the previous.
		if(original == nullptr or _first == nullptr or isPresent(original) == false)
			throw IllegalArgumentException();
		
		if(size > _max_size)
			throw AllocationLimitException();
		
		// First, find the original allocation.
		node* current = _getNode(original);
		
		uint originalSize = current->_size;
		if(originalSize >= size)
			throw IllegalArgumentException();
		
		T* temp = new T[size];
		for(int i = 0; i < originalSize; i++)
			temp[i] = std::move_if_noexcept(original[i]);
		
		if constexpr (std::is_same<T, bool>::value)
		{
			for(int i = originalSize; i < size; i++)
				temp[i] = false;
		}
		else if constexpr (std::is_same<T, char>::value)
		{
			for(int i = originalSize; i < size; i++)
				temp[i] = 0;
		}
		else if constexpr (std::is_same<T, unsigned int>::value)
		{
			for(int i = originalSize; i < size; i++)
				temp[i] = undefined_uint;
		}
		else if constexpr (std::is_same<T, double>::value)
		{
			for(int i = originalSize; i < size; i++)
				temp[i] = inf;
		}
		
		// Now that t contains the copy of the original, we remove the node of the previous allocation.
		delete[] current->_p;
		current->_p = temp;
		current->_size = size;
		
		return temp;
	}
	
	template <typename T>
	void _allocator<T>::deallocate(T*& p)
	{
		// A null pointer is not necessarily an error.  We could have had R-value construction.  So ignore it, along with any foreign allocations.
		if(p == nullptr or isPresent(p) == false)
			return;

		_removeNode(p);
		p = nullptr;
	}
	
	template <typename T>
	typename _allocator<T>::node* _allocator<T>::_getNode(const T* const& p)
	{
		// This function is called internally and after a call to isPresent(p) returns true.
		node* current = _first;
		while(current != nullptr)
		{
			if(current->_p == p)
				break;
			else
				current = current->_next;
		}
		
		return current;
	}
	
	template <typename T>
	void _allocator<T>::clear(T*& p)
	{
		if(isPresent(p) == false)
			return;
		
		node* n = _getNode(p);
		if constexpr (std::is_same<T, bool>::value)
		{
			for(int i = 0; i < n->_size; i++)
				p[i] = false;
		}
		else if constexpr (std::is_same<T, char>::value)
		{
			for(int i = 0; i < n->_size; i++)
				p[i] = 0;
		}
		else if constexpr (std::is_same<T, unsigned int>::value)
		{
			for(int i = 0; i < n->_size; i++)
				p[i] = undefined_uint;
		}
		else if constexpr (std::is_same<T, double>::value)
		{
			for(int i = 0; i < n->_size; i++)
				p[i] = inf;
		}
	}
	
	template <typename T>
	T* _allocator<T>::copyFromResize(const T* const& source, T*& destination)
	{
		/* Copy from source to destination.  If destination is nullptr, allocate destination to be
		 * the same size as source.  If the destination has been allocated, and it's not the same 
		 * size as the source, deallocate and reallocate using the size of the source.
		 * 
		 * Once source and destination are the same size, copy from source to destination.
		 * 
		*/
		
		if(isPresent(source) == false)
			return nullptr;
		
		// Test against a foreign allocation.
		if(isPresent(destination) == false and destination != nullptr)
			return nullptr;
		
		node* n_source = _getNode(source);

		// Test if we have construction instead.
		if(destination == nullptr)
			destination = allocate(n_source->_size);
		else
		{	
			// We have copy assignment instead.  Call resize only if there is a size difference.
			node* n_destination = _getNode(destination);
			if(n_destination->_size != n_source->_size)
			{
				// If the sizes are not the same then we deallocate destination, and reallocate to soure size.
				deallocate(destination);
				destination = allocate(n_source->_size);
			}
		}
			
		// At this point, source and destination have the same size.  Perform the copy operation.
		for(int i = 0; i < n_source->_size; i++)
			destination[i] = source[i];
		
		return destination;
	}

}

#endif
