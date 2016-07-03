
#ifndef _COMMON_H_
#define _COMMON_H_

#include <climits>
#include <iostream>
#include <QString>
#include <vector>
#include <fir/str/str.h>
//#include <boost/filesystem.hpp>
#include <json/json.h>
#include <fstream>
#include "AudioParams.h"

long long getCurrentTimeMs(void);
long long getCurrentTimeMs2(void);

void printMilliSecondsAsTime(std::ostream& _out, unsigned long _ms);
long millisecondsToVisualPosition( long _ms, double _timeconst );
long visualPositionToMilliseconds( long _pos, double _timeconst );
unsigned getFFTWinByIndex(int _i);
int getIndexByFFTWin(unsigned _win);


class QStringToCharString
{
public:
	QStringToCharString()
	{
		filename_w_tmp.resize( 1024 );
		filename_tmp.resize( 1024 );
	}

	const char* convert( const QString &_str )
	{
		_str.toWCharArray( &filename_w_tmp[0] );
	#ifdef WIN32
		// TODO: cp1251
		fir::str::conv_unicode_utf8( &filename_w_tmp[0], &filename_tmp[0] );
	#else
		fir::str::conv_unicode_utf8( &filename_w_tmp[0], &filename_tmp[0] );
	#endif

		return &filename_tmp[0];
	}

private:
	std::vector<wchar_t> filename_w_tmp;
	std::vector<char> filename_tmp;
};


void writeJsonToFile(const std::string &_filename, const Json::Value&);
void loadJsonFromFile(const std::string &_filename, Json::Value&);
void saveAudioParamsToJson(Json::Value &_node, const AudioParams &_params);
void loadAudioParamsFromJson(const Json::Value &_node, AudioParams &_params);

template < typename T >
class traitLimits
{
public:
	enum Limits { MIN = 0, MAX = 1 };
};

template < >
class traitLimits<int16_t>
{
public:
	enum Limits { MIN = SHRT_MIN, MAX = SHRT_MAX };
};

template < >
class traitLimits<int32_t>
{
	enum Limits { MIN = LONG_MIN, MAX = LONG_MAX };
};

template < >
class traitLimits<unsigned long>
{
	enum Limits { MIN = 0, MAX = ULONG_MAX };
};

#endif
