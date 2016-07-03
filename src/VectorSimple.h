/*
 * pavelkolodin@gmail.com
 *
 */


#pragma once

// A class for allocating array of objects T
// and ensuring size of this array.

template< typename T >
class VectorSimple
{
	T* m_ptr;
	size_t m_size;
	void clear()
	{
		if ( m_ptr )
			delete[] m_ptr;
		m_ptr = NULL;
		m_size = 0;
	}
public:
	VectorSimple()
	: m_ptr(NULL)
	, m_size( 0 )
	{
	}
	~VectorSimple()
	{
		clear();
	}
	bool ensure_size(size_t _size)
	{
		// Any upper limit? No.
		if ( ! _size )
			return false;

		if ( _size > m_size )
		{
			clear();
			try
			{
				m_ptr = new T[ _size ];

			} catch ( ... )
			{
				return false;
			}

			m_size = _size;
		}
		return true;
	}
	T* ptr()
	{
		return m_ptr;
	}
};
