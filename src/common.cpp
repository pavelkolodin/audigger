/*
 *  pavelkolodin@gmail.com
 */

#ifndef WIN32
	#include <sys/types.h>
	#include <unistd.h>
#endif
#include <boost/date_time/posix_time/posix_time.hpp> // used by getCurrentTimeMs()
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/thread/thread.hpp>
#include "common.h"
#include "defines.h"

long long getCurrentTimeMs(void)
{
	long long ret = 0;

#ifndef WIN32
	struct timespec ts;
	clock_gettime( CLOCK_MONOTONIC, &ts);
	ret = ts.tv_sec * 1000;
	ret += ts.tv_nsec / 1000000;

//	struct timeval tv;
//	gettimeofday( &tv, NULL );
//	ret = tv.tv_sec * 1000; // sec
//	ret += tv.tv_usec / 1000; // microsec

#else
	boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime	epoch_start(boost::gregorian::date(1970,1,1));
	boost::posix_time::time_duration dur = now - epoch_start;
	ret = dur.total_milliseconds();
#endif

	return ret;
}

long long getCurrentTimeMs2(void)
{
	return getCurrentTimeMs();
//	long long ret = 0;
//	boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
//	boost::posix_time::ptime	epoch_start(boost::gregorian::date(1970,1,1));
//	boost::posix_time::time_duration dur = now - epoch_start;
//	ret = dur.total_milliseconds();
//
//	return ret;
}


#define MS_DAY (60*60*24*1000)
#define MS_HR (60*60*1000)
#define MS_MIN (60*1000)
#define MS_SEC (1000)

void printMilliSecondsAsTime(std::ostream& _out, unsigned long _ms)
{
	// 60 - M
	// 60*60 - H
	// 60*60*24 - D

	unsigned long days = (_ms / MS_DAY);
	unsigned long hours = ((_ms - (days * MS_DAY)) / MS_HR);
	unsigned long mins = ((_ms - (days * MS_DAY + hours * MS_HR)) / MS_MIN);
	unsigned long secs = ((_ms - (days * MS_DAY + hours * MS_HR + mins * MS_MIN))/MS_SEC);

	unsigned long hundreds = _ms / 1000;

	unsigned long ms = _ms - hundreds * 1000;//_ms - (days * MS_DAY + hours * MS_HR + mins * MS_MIN + secs * MS_SEC);

	_out.width(2);
	_out.fill('0');
	if ( days )
	{
		_out.width(1);
		_out.fill('0');
		_out << days << ":";
	}

	if ( hours )
	{
		_out.width(2);
		_out.fill('0');
		_out << hours << ":";
	}

	_out.width(2);
	_out.fill('0');
	_out << mins << ":";

	_out.width(2);
	_out.fill('0');
	_out << secs;

	if ( ms )
	{
		_out << ".";
		_out.width(3);
		_out.fill('0');
		_out << ms;
	}
}

long millisecondsToVisualPosition( long _ms, double _timeconst )
{
	long res = (((double)_ms) / _timeconst);
	return res;
}

long visualPositionToMilliseconds( long _pos, double _timeconst )
{
	double pos = _pos;
	long result = pos * _timeconst;

	if ( millisecondsToVisualPosition( result, _timeconst ) < pos )
		++ result;

	return result;
}


unsigned getFFTWinByIndex(int _i)
{
	for ( int i = 0, w = FFT_WIN_MIN; w <= FFT_WIN_MAX; w *= 2, ++i )
		if ( _i == i )
			return w;
	return 0;
}

int getIndexByFFTWin(unsigned _win)
{
	for ( unsigned i = 0, w = FFT_WIN_MIN; w <= FFT_WIN_MAX; w *= 2, ++i )
		if ( _win == w )
			return i;
	return -1;
}


void writeJsonToFile(const std::string &_filename, const Json::Value &_json_value)
{
	Json::StyledWriter writer;
	std::string str_json = writer.write( _json_value );
	std::fstream f1( _filename.c_str(), std::ios_base::out | std::ios_base::binary);
	if ( ! f1.good() )
	{
		std::string err = "Can't create or overwrite file: ";
		err += _filename;
		throw std::runtime_error( err );
	}

	f1.write( str_json.c_str(), str_json.size() );
	f1.close();
}

void loadJsonFromFile(const std::string &_filename, Json::Value &_json_value)
{
	std::fstream f1( _filename.c_str(), std::ios_base::in | std::ios_base::binary );
	if ( ! f1.good() )
	{
		std::string err("Can't open file: ");
		err += _filename;
		throw std::runtime_error( err );
	}

	Json::Reader reader;
	if ( ! reader.parse( f1, _json_value ) )
	{
		std::string err("Can't parse json: ");
		err += _filename;
		throw std::runtime_error( err );
	}
}

void saveAudioParamsToJson(Json::Value &_node, const AudioParams &_params)
{
	_node["rate"] =			(Json::UInt)_params.m_rate;
	_node["bits"] =			(Json::UInt)_params.m_bits;
	_node["channels"] =		(Json::UInt)_params.m_channels;
	_node["frames"] =		(Json::UInt)_params.m_frames;
	_node["bigendian"] =	_params.m_bigendian;
	_node["signed"] =		_params.m_signed;
	_node["float"] =		_params.m_float;
}

void loadAudioParamsFromJson(const Json::Value &_node, AudioParams &_params)
{
	_params.m_rate = _node["rate"].asUInt();
	_params.m_bits = _node["bits"].asUInt();
	_params.m_channels = _node["channels"].asUInt();
	_params.m_frames = _node["frames"].asUInt();
	_params.m_bigendian = _node["bigendian"].asBool();
	_params.m_signed = _node["signed"].asBool();
	_params.m_float = _node["float"].asBool();
}

