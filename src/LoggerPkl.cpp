

#include "LoggerPkl.h"

LoggerPkl *LoggerPkl::_self = NULL;

void LoggerPkl::log(const char *_str)
{
	boost::unique_lock<boost::mutex> lock( m_mutex );
	std::cout << _str;
}
