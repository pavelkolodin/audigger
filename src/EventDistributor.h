/*
 *  pavelkolodin@gmail.com
 */

#ifndef EVENTDISTRIBUTOR_H_
#define EVENTDISTRIBUTOR_H_

#include <set>
#include <boost/foreach.hpp>

class EventDistributor
{

public:

	class EventUser
	{
	public:
		EventUser( EventDistributor *_distributor )
		: m_distributor( _distributor )
		{
			if ( m_distributor )
				m_distributor->addeventuser( this );
		}

		virtual ~EventUser()
		{
			if ( m_distributor )
				m_distributor->unregister( this );
		}

		// Call this method to trigger all EventUsers.
		void trigAll()
		{
			if ( m_distributor )
				m_distributor->trigAll();
		}

		virtual void trigger() { }
		void orphanize() { m_distributor = NULL; }

	protected:
		EventDistributor* m_distributor;
	};




	EventDistributor()	{}
	virtual ~EventDistributor()
	{
		BOOST_FOREACH(EventUser* user, m_users)
		{
			user->orphanize();
		}
	}

	void addeventuser( EventUser *_eus )
	{
		m_users.insert( _eus );
	}

	void unregister( EventUser *_eus )
	{
		m_users.erase( _eus );
	}

	void trigAll()
	{
		BOOST_FOREACH(EventUser* user, m_users)
		{
			user->trigger();
		}
	}

protected:
	std::set<EventUser*> m_users;

};

#endif /* EVENTDISTRIBUTOR_H_ */
