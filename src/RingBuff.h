
// pavelkolodin

#ifndef _RingBuff_h_
#define _RingBuff_h_

// Used for incoming tasks?
template < typename T > class RingBuff
{

public:
	void resize(size_t _size)
	{
		m_buff.resize( _size );
		m_size = _size;
		m_free = _size;
		m_front = 0;
		m_back = 0;
	}

	bool full()
	{
		return (0 == m_free);
	}

	// Any space?
	bool empty()
	{
		return (m_size == m_free);
	}

	const T& back()
	{
		return m_buff[m_back];
	}

	T& front()
	{
		return m_buff[m_front];
	}

	void pop()
	{
		if ( m_free == m_size )
			return;

		++ m_free;
		m_back = _incr(m_back);

	}
	void push()
	{
		if ( 0 == m_free )
			return;
		-- m_free;
		m_front = _incr(m_front);
	}
	size_t _incr( size_t _arg )
	{
		++ _arg;
		if ( _arg >= m_size )
			_arg = 0;
		return _arg;
	}
	size_t freeSize()
	{
		return m_free;
	}
private:
	std::vector < T > m_buff;
	size_t m_back;
	size_t m_front;
	size_t m_free, m_size;
};

#endif
