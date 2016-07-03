/*
 *  pavelkolodin@gmail.com
 */

#ifndef _MutexRent_H_
#define _MutexRent_H_

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

class MutexId
{
public:
	MutexId( )
	: m_counter(0)
	, m_id(0)
	, m_mutex( new boost::mutex )
	{

	}
	size_t m_counter;
	unsigned m_id;
	boost::shared_ptr<boost::mutex> m_mutex;
};

class MutexRent
{		
	public:
		MutexRent ( unsigned _threads_num );
		virtual ~MutexRent ( );

		// lock mutex for _id
		void enter( unsigned _id );

		// unlock mutex for _id.
		void leave( unsigned _id );

	private:
		MutexId *addMutex__(unsigned _id);

		boost::mutex m_mutex_main;
		boost::condition_variable m_cond_main;

		std::vector< MutexId > m_vec;
};

#endif

