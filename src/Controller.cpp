/*
 *  pavelkolodin@gmail.com
 */

#include <boost/date_time/posix_time/posix_time.hpp> // used by getCurrentTimeMs()
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/foreach.hpp>
#include <fir/logger/logger.h>
//#include <iostream>

#include "Controller.h"
#include "defines.h"
#include "common.h"
#include "Processor.h"

#ifdef AUDIO_VIRT
	#include "AudioDeviceVirt.h"
#else
	#ifdef WIN32
		#include "AudioDeviceWinAPI.h"
	#else
		#include "AudioDeviceALSA.h"
	#endif
#endif


Controller::Controller ( )
{
	m_proc.reset( new Processor::Processor( this ) );
	m_sounddevice.reset( new AudioDevice_t(this) );

	m_base.reset( new TracksBase::TracksBase() );

	try
	{
		m_base->openBaseFS( "/ar/snd", "local" );
	} catch ( ... )
	{
	}
}

Controller::~Controller ( )
{

}

TracksBase::TracksBase* Controller::getBase(const char *_name)
{
	return m_base.get();
}

TracksGroup::Ptr Controller::createTracksGroup()
{
	TracksGroup::Ptr p;
	try
	{
		p.reset( new TracksGroup() );
		m_groups.push_back( p );
	} catch(...)
	{
		p.reset();
	}
	return p;
}

bool Controller::deleteTracksGroup( const TracksGroup* _ptr )
{
	BOOST_FOREACH( TracksGroup::Ptr p, m_groups )
	{
		if ( (void*)p.get() == (void*)_ptr)
		{
			p.reset();
			return true;
		}
	}
	return false;
}

double Controller::getPreciseSampleRate()
{
	return m_sounddevice->getPreciseSampleRate();
}

bool Controller::sndStart(TracksGroup::Ptr _group)
{
	m_sounddevice->stop();

	if ( ! m_sounddevice->init( _group->getAudioParams() ) )
		return false;

	return m_sounddevice->start( _group );
}

void Controller::sndStop()
{
	//m_playing = false;
	LOG_PURE( "S" );
	m_sounddevice->stop();
	LOG_PURE ("S2");
}

/*
void Controller::timerFunction()
{
	if ( m_playing ) // TODO : atomic ?
	{
		double rate_precise = m_sounddevice->getPreciseSampleRate();
		//if ( rate_precise < 7000 || rate_precise > 193000 )
		//	return;

		long long currtime_ms = getCurrentTimeMs2();
		double timediff = (currtime_ms - (double)m_play_starttime);
		timediff /= 1000.0;
		double a1 = timediff * rate_precise; // using measured samplerate.
		a1 /= (m_windowlen/m_stepdiv);

		//Supermeganew method.
		//size_t lastframe;
		//long long lasttime;
		//long long currtime_ms = getCurrentTimeMs2();
		//m_sounddevice -> getLastSentPeriod ( &lastframe, &lasttime );

		if ( 0 == m_selection_len )
		{
			showCursorPos ( (int)(m_playing_offset + a1) );
		}
		else
		{
			// How many lines in our selected interval?
			unsigned n = m_selection_len/(m_windowlen/m_stepdiv);

			// How many intervals are already played out?
			unsigned m = a1 / n;

			// calculate new a1.
			a1 -= m * n;
			showCursorPos ( (int)(m_playing_offset + a1) );
		}
	}
}
*/
