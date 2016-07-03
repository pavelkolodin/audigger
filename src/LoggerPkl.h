#ifndef _LOGGERFIR_H_
#define _LOGGERFIR_H_

#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include <sstream>
#include <iostream>

class LoggerPkl
{
	static LoggerPkl *_self;
protected:
	LoggerPkl() {}
	boost::mutex m_mutex;
public:
	static LoggerPkl *Instance()
	{
		if ( ! _self )
			_self = new LoggerPkl();

		return _self;
	}

	void log( const char *_str );
};

#define LOGLOGGERPKL( x ) {\
	std::stringstream SS; \
	SS << x; \
	LoggerPkl::Instance()->log( SS.str().c_str() ); \
	\
}

#endif
