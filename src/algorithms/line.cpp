#include "line.h"


namespace algorithms
{
	char_allocator al;
	char_allocator* const line::pal = &al;
	
	line::line() noexcept
	{
		m_ptr = pal->allocate(_size);
	}
	
	line::line(uint size) noexcept
	{
		_size = size;
		m_ptr = pal->allocate(_size);
	}
	
	line::line(const line& rhs) noexcept
	{
		_size = rhs._size;
		_length = rhs._length;
		m_ptr = pal->copyFromResize(rhs.m_ptr, m_ptr);
	}
	
	line::line(line&& rhs) noexcept
	{
		_size = rhs._size;
		_length = rhs._length;
		
		m_ptr = rhs.m_ptr;
		rhs.m_ptr = nullptr;
	}
	
	line& line::operator=(const line& rhs) noexcept
	{
		m_ptr = pal->copyFromResize(rhs.m_ptr, m_ptr);
		_length = rhs._length;
		
		return *this;
	}
	
	line& line::operator=(line&& rhs) noexcept
	{
		if(this == &rhs)
			return *this;
		
		char* temp = m_ptr;
		m_ptr = rhs.m_ptr;
		rhs.m_ptr = temp;
		
		_size = rhs._size;
		_length = rhs._length;
		
		return *this;
	}
	
	line::~line() noexcept
	{
		pal->deallocate(m_ptr);
	}
	
	bool line::operator==(const line& rhs) const
	{
		if(_length != rhs._length)
			return false;
		else
		{
			for(int i = 0; i < _length; i++)
				if(m_ptr[i] != rhs.m_ptr[i])
					return false;
		}
		
		return true;
	}
	
	bool line::operator!=(const line& rhs) const
	{
		return !operator==(rhs);
	}
	
	bool line::operator<(const line& rhs) const
	{
		return _length < rhs._length;
	}
	
	bool line::operator<=(const line& rhs) const
	{
		return operator<(rhs) || operator==(rhs);
	}
	
	bool line::operator>(const line& rhs) const
	{
		return _length > rhs._length;
	}
	
	bool line::operator>=(const line& rhs) const
	{
		return operator>(rhs) || operator==(rhs);
	}
	
	void line::resize(uint size)
	{
		if(size == _size)
			return;
		
		m_ptr = pal->resizeTo(m_ptr, size);
	}
	
	void line::clear()
	{
		for(uint i = 0; i < _length+1; i++)
			m_ptr[i] = 0;
	}
	
	void line::set(const char* buffer, uint size)
	{
		//Get characters from buffer, where index in [0, size), and truncate before any newline character.
		
		// Leave one byte for the null termination of the string.
		if(size > _size)
			resize(size+1);

		_length = 0;
		for(uint i = 0; i < size; i++)
		{
			// Don't count the newline character.
			if(buffer[i] == '\n')
				break;
			else
			{
				m_ptr[i] = buffer[i];
				_length++;
			}
		}
			
		m_ptr[_length+1] = '\0';
	}

	
}
