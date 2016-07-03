/*
 * pavelkolodin@gmail.com
 */

#pragma once
#include <vector>
#include <boost/foreach.hpp>

namespace BlockTable
{

// Rules:
// TTD::init()

template < typename TB > struct TableEntry
{
	// <display?>: circumstance
	// 0: m_block == NULL
	// 0: m_block != NULL && m_busy
	// 1: m_block != NULL && ! m_busy

	void clear()
	{
		m_block = NULL;
		m_busy = false;
	}
	// pointer to allocated block
	TB *m_block;
	// building in progress
	bool m_busy;
};


// TB - block type
template < typename TB > class BlockTable
{
public:
	typedef std::vector<TB>				MemType;

	BlockTable(size_t _memsize);
	void init(typename TB::Initializer &_bi);
	void clear();
	void resize(size_t _table_size) { m_table.resize( _table_size ); }

	TB *newBlock(size_t _index);
	TB *getBlock(size_t _index);
	bool isAllocated(size_t _index);
	void enableBlock(size_t _index);
	void freeBlock(size_t _index);

private:
	MemType m_mem;
	// array of pointers to free blocks (stack)
	// Need because blocks can be freed in any order
	std::vector<TB*> m_mem_free_list;
	TB **m_mem_free_list_next;
	std::vector< TableEntry<TB> > m_table;
	size_t m_block_index_min;
	size_t m_block_index_max;
};

template <typename TB> BlockTable<TB>::BlockTable(size_t _memsize)
		: m_block_index_min ( std::numeric_limits<std::size_t>::max() )
		, m_block_index_max (0)
{
	m_mem.resize( _memsize );
	m_mem_free_list.resize( _memsize );
	m_mem_free_list_next = &m_mem_free_list[0];

	for( size_t i = 0; i < _memsize; ++i )
	{
		m_mem_free_list[i] = &m_mem[i];
	}
}

template <typename TB> void BlockTable <TB>::init(typename TB::Initializer &_bi)
{
	// Apply initializers.
	BOOST_FOREACH(TB &block, m_mem)
	{
		_bi( block );
	}
	clear();
}

template <typename TB> void BlockTable <TB>::clear()
{
	m_block_index_min = std::numeric_limits<std::size_t>::max();
	m_block_index_max = 0;

	BOOST_FOREACH( TableEntry<TB> &entry, m_table )
	{
		entry.m_block = NULL;
		entry.m_busy = false;
	}
	m_mem_free_list_next = &m_mem_free_list[0];
}

template <typename TB> TB* BlockTable <TB>::newBlock(size_t _index_dst)
{
	TB *b = NULL;

	if ( (size_t)(m_mem_free_list_next - &m_mem_free_list[0]) < m_mem.size() )
	{
		b = *m_mem_free_list_next;
		++ m_mem_free_list_next;
	}
	else
	{
		//
		// Re-use most distant among allocated blocks.
		//

		size_t d1 = std::abs( (int)_index_dst - (int)m_block_index_min );
		size_t d2 = std::abs( (int)_index_dst - (int)m_block_index_max );
		if ( d1 > d2 )
		{
		// MIN
			b = m_table[ m_block_index_min ].m_block;
			m_table[ m_block_index_min ].clear();

			// Find next allocated block.
			for ( size_t i = m_block_index_min + 1; i < m_table.size(); ++i )
				if ( m_table[ i ].m_block )
				{
					m_block_index_min = i;
					break;
				}
		}
		else
		{
		// MAX
			b = m_table[ m_block_index_max ].m_block;
			m_table[ m_block_index_max ].clear();
			// Find previous allocated block.

			if ( m_block_index_max )
				for ( int i = m_block_index_max - 1; i >= 0; --i )
					if ( m_table[ i ].m_block )
					{
						m_block_index_max = i;
						break;
					}
		}
	}

	if ( _index_dst < m_block_index_min )
		m_block_index_min = _index_dst;

	if ( _index_dst > m_block_index_max )
		m_block_index_max = _index_dst;

	b->clear();

	m_table[ _index_dst ].m_block = b;
	m_table[ _index_dst ].m_busy = true;
	return b;
}

template <typename TB> TB *BlockTable <TB>::getBlock(size_t _index)
{
	if ( _index >= m_table.size() )
		return NULL;

	if ( m_table[_index].m_busy )
		return NULL;

	return m_table[_index].m_block;
}

template <typename TB> bool BlockTable <TB>::isAllocated(size_t _index)
{
	if ( _index >= m_table.size() )
		return NULL;

	return (NULL != m_table[_index].m_block);
}

template <typename TB> void BlockTable <TB>::enableBlock(size_t _index)
{
	if ( _index >= m_table.size() )
		return;

	m_table[_index].m_busy = false;
}

template <typename TB> void BlockTable <TB>::freeBlock(size_t _index_table)
{
	if ( _index_table >= m_table.size() )
		return;

	if ( ! m_table[ _index_table ].m_block )
		return;

	-- m_mem_free_list_next;
	*m_mem_free_list_next = m_table[ _index_table ].m_block;

	m_table[ _index_table ].clear();

	if ( _index_table == m_block_index_min )
	{
		for ( size_t i = m_block_index_min + 1; i < m_table.size(); ++i )
			if ( m_table[ i ].m_block )
			{
				m_block_index_min = i;
				break;
			}
	}

	if ( _index_table == m_block_index_max )
	{
		if ( m_block_index_max )
			for ( int i = m_block_index_max - 1; i >= 0; --i )
				if ( m_table[ i ].m_block )
				{
					m_block_index_max = i;
					break;
				}
	}
}


}

