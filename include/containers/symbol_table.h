#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <mutex>
#include <condition_variable>

#include "_containers.h"

#include "_containers_globals.h"
#include "_allocator.h"

namespace containers
{
    using uint = containers::uint;
    
    /*
     * This is a thread safe, templated symbol table class.
     * 
     * The class uses a non-recursive binary search algorithm in an ordered array to obtain the rank of
     * a value's key, primarily relying on the workhorse function rank(), which is O(log_2 N).
     * 
     * The range of indices is [0, N).  The put(), delete(), and deleteMin() are O(N), while the get(),
     * contains(), floor(), ceiling() are O(log_2 N) by virtue of using rank().
     * 
     * As put() is O(N), this container is not ideal for large data sets.  Caveat emptor!
     * 
    */
    
	template <typename Key, typename Value>
	class symbol_table
	{
    public:
        // The big 5 + 1:
        symbol_table();
        symbol_table(const uint&);
        symbol_table(const symbol_table&);
        symbol_table(symbol_table&&) noexcept;
        symbol_table<Key, Value>& operator=(const symbol_table&);
        symbol_table<Key, Value>& operator=(symbol_table&&) noexcept;
        ~symbol_table() noexcept;

        // getters:
        bool get(const Key&, Value&) const;
        Value get(const Key&) const;

        // setters:
        void put(const Key&, const Value&);
        bool remove(const Key&, Key&);
        bool removeMin(Key&);
        bool removeMax(Key&);
        void clear();
        
        // info:
        bool contains(const Key& key) const;
        bool empty() const;
        uint size() const;
        uint size(const Key&, const Key&) const;
        bool min(Key&) const;
        bool max(Key&) const;
        bool floor(const Key&, Key&) const;
        bool ceiling(const Key&, Key&) const;
        uint rank(const Key&) const;
        bool select(const uint&, Key&) const;
        
    private:

		static _allocator<Key>* const pal_keys;		// Constant pointer to type Key allocator.
		static _allocator<Value>* const pal_values;	// Constant pointer to type Value allocator.
		
        void _expand();

        bool _allocated = false;
        
        uint _size = 100;   						// Used for memory management.  Updated by some of the big 5, _initialize() and by _expand().
        uint N = 0;         						// elements in [0,N)
        Key* keys = nullptr;
        Value* values = nullptr;

        // With mutable, we can modify these variables even in a const setting, namely in our const getters.
		// Note that in order to prevent races, we employ 5 different mutexes.
		mutable std::mutex read_write_mutex;		// Used by getters and setters.
		mutable std::mutex resize_mutex;			// Used by the _resize() function.
		mutable std::mutex initialize_mutex;		// Used by _initialize().
        mutable std::mutex remove_mutex;            // Used by remove(), which calls contains() that uses the read_write_mutex.
        mutable std::mutex rank_mutex;              // Prevents double unique locks, as both remove() and put() use rank(), and lock the read_write_mutex.
    public:                                               
        // Embedded iterator class for our symbol table class, iterating over the keys array.  Note that this is not thread safe.  To ensure thread safety,
        // the caller must coordinate its threads.
		template <typename KeyType>
		struct st_iterator
		{
			//  iterators must be constructable, copy-constructable, copy-assignable, destructible and swappable.
			using iterator_category = std::random_access_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = KeyType;
			using pointer = KeyType*;
			using reference = KeyType&;
			
			// This custom constructor satisfies the constructable requirement.  All others are created by the compiler.  However,
			// we will need a constant iterator, will require returning constant types and constant references.  These can be
			// created from iterators and constant iterators, but non-constant iterators cannot be created from constant iterators.
			// Thus we will need copy constructor and a copy assignment constructor.
			st_iterator(pointer ptr) { m_ptr = ptr; }
			st_iterator(const st_iterator& it) { m_ptr = it.m_ptr; }
			st_iterator& operator=(const st_iterator& it) { m_ptr = it.m_ptr; return *this; }
			st_iterator() {};
			
			// Access operators:
			reference operator*() const { return *m_ptr; }
			pointer operator->() const { return m_ptr; }
			reference operator[](const difference_type& i) const { return m_ptr[i]; }
			
			// Note that std::sort needs the pre/post de/in-crement operators, the difference_type
			// returning addition/subtraction operators, along with the boolean comparison operators.
			
			// Arithmetic operators:
			// prefix increment:  fetch st_iterator reference before incrementation of index
			st_iterator& operator++() { ++m_ptr; return *this; }
			// postfix increment:  increment index and fetch copy st_iterator
			st_iterator operator++(int) { return st_iterator(m_ptr++); }
			
			// prefix decrement:  decrement first, then return st_iterator
			st_iterator& operator--() { --m_ptr; return *this; }
			// postfix decrement:  return value, then decrement, calls --st_iterator
			st_iterator operator--(int) { return st_iterator(m_ptr--); }
			
			difference_type operator+(const st_iterator& it) { return m_ptr + it.m_ptr; }
			st_iterator operator+(const difference_type& n) { return st_iterator(m_ptr + n); }
			st_iterator& operator+=(const difference_type& n) { m_ptr += n; return *this; }
			
			difference_type operator-(const st_iterator& it) { return m_ptr - it.m_ptr; }
			st_iterator operator-(const difference_type& n) { return st_iterator(m_ptr - n); }
			st_iterator& operator-=(const difference_type& n) { m_ptr -= n; return *this; }
			
			// boolean comparison operators
			bool operator==(const st_iterator& rhs) const { return m_ptr  == rhs.m_ptr; }
			bool operator!=(const st_iterator& rhs) const { return m_ptr  != rhs.m_ptr; }
			bool operator<(const st_iterator& rhs) const { return m_ptr < rhs.m_ptr; }
			bool operator>(const st_iterator& rhs) const { return m_ptr > rhs.m_ptr; }
			bool operator<=(const st_iterator& rhs) const { return m_ptr <= rhs.m_ptr; }
			bool operator>=(const st_iterator& rhs) const { return m_ptr >= rhs.m_ptr; }
			
		private:
			pointer m_ptr = nullptr;
		};
		
		using iterator = st_iterator<Key>;
		using const_iterator = st_iterator<const Key>;
        using citerator = st_iterator<const Key>;
        
		// Integrate the above iterator into our class:
		iterator begin() { return iterator(&keys[0]); }                       // Return the first element in [_head,_tail)
		iterator end() { return iterator(&keys[N]); }                         // Return the last element in [_head,_tail)
		const_iterator cbegin() const { return const_iterator(&keys[0]); }    // Return the first element in [_head,_tail)
		const_iterator cend() const { return const_iterator(&keys[N]); }      // Return the last element in [_head,_tail)
		
		void get_iters(iterator& b, iterator& e) const { b = iterator(&keys[0]); e = iterator(&keys[N]); }
		void get_citers(citerator& b, citerator& e) const { b = citerator(&keys[0]); e = citerator(&keys[N]); }
		
        // Return iterators for keys [low, high] or [low, high) if high is not present.
		void getkeys(const Key& low, const Key& high, iterator& begin, iterator& end) const;
		                                            
    };
	
	template <typename Key, typename Value>
	_allocator<Key>* const symbol_table<Key, Value>::pal_keys = &al<Key>;
	
	template <typename Key, typename Value>
	_allocator<Value>* const symbol_table<Key, Value>::pal_values = &al<Value>;

    template <typename Key, typename Value>
    symbol_table<Key, Value>::symbol_table()
    {
		keys = pal_keys->allocate(_size);
		values = pal_values->allocate(_size);
    }

    template <typename Key, typename Value>
    symbol_table<Key, Value>::symbol_table(const uint& size)
    {
		keys = pal_keys->allocate(_size);
		values = pal_values->allocate(_size);
    }

    template <typename Key, typename Value>
    symbol_table<Key, Value>::symbol_table(const symbol_table<Key, Value>& st)
    {
		_size = st._size;
		keys = pal_keys->copyFromResize(st.keys, keys);
		values = pal_values->copyFromResize(st.values, values);
    }

    template <typename Key, typename Value>
    symbol_table<Key, Value>::symbol_table(symbol_table<Key, Value>&& st) noexcept
    {
		int size = _size;
        _size = st._size;
		st._size = size;

        keys = st.keys;
        values = st.values;

        st.keys = nullptr;
        st.values = nullptr;
    }

    template <typename Key, typename Value>
    symbol_table<Key, Value>& symbol_table<Key, Value>::operator=(const symbol_table<Key, Value>& st)
    {
        {
            std::unique_lock<std::mutex> l(read_write_mutex);
            
			_size = st._size;
			keys = pal_keys->copyFromResize(st.keys, keys);
			values = pal_values->copyFromResize(st.values, values);

            return *this;
        }
    }

    template <typename Key, typename Value>
    symbol_table<Key, Value>& symbol_table<Key, Value>::operator=(symbol_table<Key, Value>&& st) noexcept
    {
        {
            std::unique_lock<std::mutex> l(read_write_mutex);
            
			int size = _size;
            _size = st._size;
			st._size = size;
            
            Key* temp1 = keys;
            keys = st.keys;
            st.keys = temp1;
            
			Value* temp2 = values;
            values = st.values;
            st.values = temp2;

            return *this;
        }
    }

    template <typename Key, typename Value>
    symbol_table<Key, Value>::~symbol_table() noexcept
    {
		pal_keys->deallocate(keys);
		pal_values->deallocate(values);
    }

    template <typename Key, typename Value>
    void symbol_table<Key, Value>::_expand()
    {
        // This is called internally, when we exceed the storage space.  Expand and copy.
		{
			// Our lock_guard is locked once and discarded (unlocked) by desctruction.
			std::lock_guard<std::mutex> l(resize_mutex);

			keys = pal_keys->resizeTo(keys, _size*2);
			values = pal_values->resizeTo(values, _size*2);
			_size = _size*2;
        }
    }

    template <typename Key, typename Value>
    void symbol_table<Key, Value>::clear()
    {
        {
			// Lock the mutex.  _initialize has its own, so no race occurs.
			std::unique_lock<std::mutex> ul(read_write_mutex);

			pal_keys->clear(keys);
			pal_values->clear(values);
		}
    }

    template <typename Key, typename Value>
    uint symbol_table<Key, Value>::size() const
    {
        {
            std::unique_lock<std::mutex> ul(read_write_mutex);
            return N;
        }
    }
    
    template <typename Key, typename Value>
    uint symbol_table<Key, Value>::size(const Key& low, const Key& high) const
    {
        {
			std::unique_lock<std::mutex> ul(read_write_mutex);
            
            if(N == 0)
                return 0;
            
            if(high < 0)
                return 0;
            else if(contains(high))
                return rank(high) - rank(low) + 1;
            else
                return rank(high) - rank(low);
        }
    }
    
    template <typename Key, typename Value>
    Value symbol_table<Key, Value>::get(const Key& key) const
    {
        /* This is a potentially dangerous version of get, where we return an
         * initialized Value in case the requested key is not in the symbol table.
         * We need to return an l-value to accommodate this erroneous case.
        */
        
        {
            std::unique_lock<std::mutex> ul(read_write_mutex);
            
            if(N == 0)
                return Value{};
            
            uint i = rank(key);
            if(i < N && key == keys[i])
                return values[i];
            else
            return Value{};
        }
    }

    template <typename Key, typename Value>
    bool symbol_table<Key, Value>::get(const Key& key, Value& result) const
    {
        {
            std::unique_lock<std::mutex> ul(read_write_mutex);
        
            if(N == 0)
                return false;
            
            uint i = rank(key);     // O(log_2 N) => get() == O(log_2 N)
            if(i < N && keys[i] == key)
            {
                result = values[i];
                return true;
            }
            else
                return false;
        }
    }
    
    template <typename Key, typename Value>
    void symbol_table<Key, Value>::put(const Key& key, const Value& value)
    {
        // Search for key.  Update the value if found; Grow table if new.
        // rank() tells us where to update the value if it is in the table, or where to go to add the value if not in the table.
        
        {
            std::unique_lock<std::mutex> ul(read_write_mutex);
        
            if(N == _size)
                _expand();
            
            uint i = rank(key);             // O(log_2 N)
            
            // Update the value.
            if(i < N  && keys[i] == key)
            {
                values[i] = value;
                return;
            }
            
            // Key was not found, so move over data by one towards the right: [i, N) -> [i+1, N].  Then insert at i.
            for(int j = N; j > i; j--)      // O(N), => put() == O(N), as O(N) > O(log_2 N)
            {
                keys[j] = keys[j-1];
                values[j] = values[j-1];
            }
            
            keys[i] = key;
            values[i] = value;
            N++;
            
            return;
        }
    }
    
    template <typename Key, typename Value>
    bool symbol_table<Key, Value>::contains(const Key& key) const
    {
        {
            std::unique_lock<std::mutex> ul(read_write_mutex);
            
            if(keys[rank(key)] == key)          // O(log_2 N) => contains() == O(log_2 N)
                return true;
            
            return false;
        }
    }
    
    template <typename Key, typename Value>
    uint symbol_table<Key, Value>::rank(const Key& key) const
    {
        {
            std::unique_lock<std::mutex> ul(rank_mutex);
         
            /* Use non-recursive binary search to return the number of keys smaller than key, namely the index of its ceiling.
             * If the key is present, then the number of keys smaller than key are in [0, rank(key) - 1].  Else return the place
			 * the key should be in.  
             * 
             * Decompose the interval [low, high] to [low, mid-1], [mid] and [mid+1, high]; and adjust low and high by +1/-1 respectively,
             * until we converge on the rank, located at mid or at low.  This occurs when key == keys[mid] or when low > high, by one.
             */
            
            if(N == 0)				// Empty container, so give the first spot to key.
                return 0;
            
            // Note that we need to use signed int here, as the condition in the while loop needs to be violated to exit.
            int low = 0;
            int high = N - 1;       // The rank of a key within the container can never equal N. Otherwise, it is N if key > max(keys).
            int mid = 0;
            
            while(low <= high)                  
            {
                mid = low + (high - low) / 2;
                if(key < keys[mid])
                    high = mid - 1;
                else if(key > keys[mid])
                    low = mid + 1;
                else
                    return mid;     // The key is within the container.
            }
            
            // The key is not in the container.  If key > max(keys), return N.  If key < min(keys) return 0.  Else return
            // the index it would have if key was in the container.
            return low;
        }
    }
    
    template <typename Key, typename Value>
    bool symbol_table<Key, Value>::min(Key& result) const
    {
        {
            std::unique_lock<std::mutex> ul(read_write_mutex);  
            
            if(N == 0)
                return false;
            
            result = keys[0];
            return true;
        }
    }
    
    template <typename Key, typename Value>
    bool symbol_table<Key, Value>::max(Key& result) const
    {
        {
            std::unique_lock<std::mutex> ul(read_write_mutex);  
            
            if(N == 0)
                return false;
            
            result = keys[N-1];
            return true;
        }
    }
    
    template <typename Key, typename Value>
    bool symbol_table<Key, Value>::select(const uint& key, Key& result) const
    {
        {
            std::unique_lock<std::mutex> ul(read_write_mutex);  
            
            if(N == 0)
                return false;
            
            result = keys[key];
            
            return true;
        }
    }
    
    /*
     * ST container floor = largest key less than OR EQUAL to key within the container.
     * ST container ceiling = smallest key larger than OR EQUAL to key within the container.
     * Note that 0 has no floor, and that N has no ceiling.  Also, note that rank(key) returns
     * the number of keys less than the key, which is the index to the ceiling key if present.
     * Both floor() and ceiling() are O(log_2 N) by virtue of rank().
     */
    
    // Find the ceiling of key.  Set key_ceiling and return true, or just return false on error.
    template <typename Key, typename Value>
    bool symbol_table<Key, Value>::ceiling(const Key& key, Key& key_ceiling) const
    {
        {
            std::unique_lock<std::mutex> ul(read_write_mutex);  
            if(N == 0)
                return false;                   // Empty container!
            
            uint ceil = rank(key);              // rank(key) == number of keys less than key, which equals the index of the ceiling, if the key is present.
            key_ceiling = keys[ceil];
            return true;
        }
    }
    
    // Find the floor of key.  Set key_floor and return true, or just return false on error.
    template <typename Key, typename Value>
    bool symbol_table<Key, Value>::floor(const Key& key, Key& key_floor) const
    {
        {
            std::unique_lock<std::mutex> ul(read_write_mutex);
            if(N == 0)                  
                return false;                   // Empty container!
            
            uint ceiling = rank(key);           // rank(key) == number of keys less than key == index of ceiling key, if present.
            
            if(ceiling == 0 or ceiling == N)
                return false;                   // These two cases occur when the key is not in the container, and key < min(keys), or > max(keys), respectively.
            
            if(key == keys[ceiling])
                key_floor = keys[ceiling];      // This is the OR EQUAL part of "floor".
            else
                key_floor = keys[ceiling-1];    // This is the less than part of "floor".
            
            return true;
        }
    }
    
    template <typename Key, typename Value>
    bool symbol_table<Key, Value>::remove(const Key& key, Key& result)
    {
        {
            std::unique_lock<std::mutex> mlock(remove_mutex);
            
            // Check if the key pair is in the container.
            if(N == 0 or contains(key) == false)
                return false;
            
            // Find ceiling of key.
            Key k;
            get(key, k);
            uint i = rank(key);     // O(log_2 N)
            
            // From put():  i = rank(key), and move all keys of greater rank to the right:   [i, N) -> [i+1, N].  We must thus reverse this.
            for(int j = i; j < N; i++)      // [i, N) => O(N), => remove() == O(N), as O(N) > O(log_2 N)
            {
                keys[j - 1] = keys[j];
                values[j - 1] = values[j];
            }
            
            N--;
            
            result = k;
            return true;
        }
    }
    
    template <typename Key, typename Value>
    bool symbol_table<Key, Value>::removeMin(Key& result)
    {
        return remove(min(), result);
    }
    
    template <typename Key, typename Value>
    bool symbol_table<Key, Value>::removeMax(Key& result)
    {
        return remove(max(), result);
    }
    
    template <typename Key, typename Value>
    bool symbol_table<Key, Value>::empty() const
    {
        {
            std::unique_lock<std::mutex> ul(read_write_mutex);  
            
            return N == 0;
        }
    }
    
    template <typename Key, typename Value>
    void symbol_table<Key, Value>::getkeys(const Key& low, const Key& high, iterator& begin, iterator& end) const
    {
        // As iteration is not our responsibility, we won't worry about thread safety here.
        begin = iterator(low);
        if(contains(high) != true)
            end = iterator(high);                   // [end, high)
        else
            end = iterator(keys[rank(high)]);       // [end, high], with rank(high) = ceiling(high)
    }
}

#endif

