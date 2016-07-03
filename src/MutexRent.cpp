/*
 *  pavelkolodin@gmail.com
 */

#include "MutexRent.h"
#include <boost/thread/locks.hpp>

MutexRent::MutexRent ( unsigned _threads_num )
: m_vec ( _threads_num, MutexId() )
{

}

MutexRent::~MutexRent ( )
{

}

void MutexRent::enter( unsigned _id )
{
	MutexId *m;

	{
		boost::unique_lock<boost::mutex> lock( m_mutex_main );
		while ( NULL == (m = addMutex__( _id )) )
		{
			m_cond_main.wait( lock );
		}
	}

	m->m_mutex->lock();
}

void MutexRent::leave( unsigned _id )
{
	boost::unique_lock<boost::mutex> lock( m_mutex_main );

	for ( size_t i = 0; i < m_vec.size(); ++i )
	{
		if ( _id == m_vec[i].m_id && m_vec[i].m_counter )
		{
			-- m_vec[i].m_counter;
			m_cond_main.notify_all();
			m_vec[i].m_mutex -> unlock();
			break;
		}
	}
}

// PROTECTED

MutexId *MutexRent::addMutex__(unsigned _id)
{
	for ( size_t i = 0; i < m_vec.size(); ++i )
	{
		if ( _id == m_vec[i].m_id && m_vec[i].m_counter )
		{
			++ m_vec[i].m_counter;
			return &(m_vec[i]);
		}
	}

	for ( int i = 0; i < m_vec.size(); ++i )
	{
		if ( 0 == m_vec[i].m_counter )
		{
			++ m_vec[i].m_counter;
			m_vec[i].m_id = _id;
			return &(m_vec[i]);
		}
	}

	return NULL;
}

