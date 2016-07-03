#ifndef _BINDER_BOOST_THREAD_H_
#define _BINDER_BOOST_THREAD_H_

template <typename BASE_T> class BinderBoostThread
{
public:
	BinderBoostThread(BASE_T *_this, unsigned _thread_id)
	{
		m_this = _this;
		m_thread_id = _thread_id;
	}
	void operator()(void)
	{
		m_this -> threadFunction( m_thread_id );
	}

	// May be copied by the default copy constructor:
	BASE_T	*m_this;
	unsigned m_thread_id;
};

#endif
