/*
 *  pavelkolodin@gmail.com
 */

#include "common.h"
#include "AudioDeviceVirt.h"

#include "Controller.h"
#include "Processor.h"
#include "BinderBoostThread.h"

#include <fir/logger/logger.h>
#include <boost/date_time/posix_time/posix_time_duration.hpp>

AudioDeviceVirt :: AudioDeviceVirt ( Controller *_ctrl  )
	: m_ctrl( _ctrl )
	, m_rate(0)
	, m_channels(0)
	, m_currslot(0)
	, m_slot_off(0)
	, m_slot_size(0)
	, m_buffsamplesize(0)
	, m_slot_loading(0)
	, m_first_loading(0)
	, m_cnt_play(0)
	, m_time_start(0)
	, m_rate_mean(0)
	, m_rate_mean_datasize(0)
	, m_state( STOPPED )
	, m_track_id(0)
{
	for ( int i = 0 ; i < SOUNDDEVICE_SLOTS; ++i )
	{
		m_buff[i] = 0;
		m_sizes[i] = 0;
	}

	// Thread:
	BinderBoostThread< AudioDeviceVirt > callable( this, 0 ); // can copy itself
	m_thread_stop = 0;
	m_thread = boost::thread( callable );
}


AudioDeviceVirt :: ~AudioDeviceVirt ( )
{
	for ( int i = 0 ; i < SOUNDDEVICE_SLOTS; ++i )
	{
		if ( m_buff[i] )
			delete[] m_buff[i];
	}

	LOGPLACE
	// Stop thread:
	m_thread_stop = true;
	this->_kickThread();
	m_thread.join();
	LOGPLACE
}


void AudioDeviceVirt::threadFunction( unsigned _thread_id )
{
	int err;
	boost::posix_time::time_duration td_100ms = boost::posix_time::milliseconds(100);
	while( 1 )
	{
		{
			boost::unique_lock<boost::mutex> lock( m_mutex_thread );
			while (( PLAYING != m_state) && ( DRAINING != m_state ) && ! m_thread_stop )
				m_cond_thread.wait(lock);

			if ( m_thread_stop )
				return;
		}

		LOG_PURE("Playing started...");

		while( 1 )
		{
			{
				boost::unique_lock<boost::mutex> lock( m_mutex_thread );
				if ( PLAYING != m_state )
					break;

				this->play();
			}

			// POLLs
			boost::this_thread::sleep( td_100ms );
		}

		LOG_PURE("Playing Stopping...");

		// Release ::stop() method:
		{
			boost::unique_lock<boost::mutex> lock( m_mutex_thread );
			m_state = STOPPED;
			_resetPlayingData();
			m_cond_back.notify_one();
		}
	} // while(1) main
}

bool AudioDeviceVirt::start(size_t _from_sample)
{
	boost::unique_lock<boost::mutex> lock( m_mutex_thread );

	if ( ! m_ctrl )
		return false;

	if ( STOPPED != m_state )
		return false;

	_resetPlayingData();

	m_state = LOADING;
	m_frames_loaded = _from_sample;

	// Request first block of data and wait the data "m_cond_data".
	// Processor::loadData() responds by calling our ::requestDone().
	m_ctrl->getProcessor()->loadData( this, m_track_id, m_buff[0], m_frames_loaded, m_slot_size );

	// Waiting from requestDone().
	while ( 0 == m_sizes[0] && ! m_cannot_load_data)
		m_cond_back.wait( lock );

	return ( false == m_cannot_load_data );
}


void AudioDeviceVirt::stop()
{
	boost::unique_lock<boost::mutex> lock( m_mutex_thread );

	if ( PLAYING != m_state )
		return;

	m_state = STOP_REQUESTED;

	_kickThread();

	// Waiting from threadFunction().
	while ( STOPPED != m_state )
		m_cond_back.wait( lock );
}

void AudioDeviceVirt::requestDone( size_t _size )
{
	boost::unique_lock<boost::mutex> lock( m_mutex_thread );
	LOG_PURE( "size " << _size << ", nxt " << _nextslotNumber() << ", st " << m_state );
	if ( STOPPED == m_state )
		return;

	if ( _size > 0 )
	{
		m_frames_loaded += _size;
		m_slot_loading = false; // TODO: kill this flag

		if (LOADING == m_state)
		{
			m_sizes[ 0 ] = _size;
			m_state = PLAYING;

			// Start playing.
			_kickThread();

			// release start method:
			m_cond_back.notify_one();
		}
		else
			m_sizes[ _nextslotNumber() ] = _size;
	}
	else
	{
		m_cannot_load_data = true;
		m_cond_back.notify_one();
	}
}

bool AudioDeviceVirt::init( unsigned _bits, unsigned _rate, unsigned _channels)
{
	//LOGVAR3 ( _rate, _bits, _channels )

	if ( _bits != 16 || _channels < 1 || ! goodRate( _rate ) )
	{
		LOG_PURE("(Bad settings)");
		return false;
	}

	this->stop();

	{
		boost::unique_lock<boost::mutex> lock( m_mutex_thread );
		m_rate = _rate;
		m_bits = _bits;
		m_channels = _channels;
		m_rate_mean = m_rate;

		int err = 0, dir = 0;
		unsigned int rrate = _rate;

		// Allocate memory:
		m_slot_size = 44100; // TODO magic 128
		size_t need_samplesize = m_slot_size * m_channels;

		if ( m_buffsamplesize < need_samplesize )
		{
			for ( int i = 0 ; i < SOUNDDEVICE_SLOTS; ++i )
			{
				if ( m_buff[i] )
					delete[] m_buff[i];

				m_buff[i] = new short[ need_samplesize ];
				m_sizes[i] = 0; // no data loaded to this buffer.
			}
		}
	}

	return true;
}



void AudioDeviceVirt::play()
{
	++ m_cnt_play;
	int err = 0;
	size_t m_period_size = 4410;

	size_t currsize = std::min<int> ( m_sizes[m_currslot] - m_slot_off, m_period_size );

		//err = snd_pcm_writei(m_alsa_pcm_handle, m_buff[m_currslot] + m_slot_off*m_channels, currsize);


		// Else : continue playing sound.
		m_slot_off += m_period_size;

		if ( m_slot_off > 4*m_period_size ) // MAGIC number 4
		{
			// Load next slot.
			// If current buffer was full and this action is not activated yet.
			if ( ! m_slot_loading && m_sizes[m_currslot] == m_slot_size && 0 == m_sizes[_nextslotNumber()] )
			{
				m_slot_loading = true;
				//!//m_ctrl->loadData();

				// Load next slot:

				m_ctrl->getProcessor()->loadData( this, m_track_id, m_buff[_nextslotNumber()], m_frames_loaded, m_slot_size );
			}
		}

		// Switch to another slot.
		if ( m_slot_off >= m_slot_size )
		{
			//#ifdef DBGCOUT
			LOG_PURE("\n(SWITCH)\n");

			//#endif
			m_slot_off = 0;

			m_sizes[m_currslot] = 0; // empty
			// Switch to next slot. Will not switch if next buffer is EMPTY. Be careful.
			// TODO: What if next buffer is empty???
			if ( 0 != m_sizes[ _nextslotNumber() ] )
				m_currslot = _nextslotNumber();
			else
			{
				LOG_ERROR( "SO WHAT? There is nothing to switch to... Dieing..." );
				//this->stop();
			}
		}
}

//
// P R I V A T E
//

void AudioDeviceVirt::_resetPlayingData()
{
	m_currslot = 0;
	m_slot_off = 0;
	m_cnt_play = 0;
	m_time_start = 0;
	m_rate_mean_datasize = 0;
	m_rate_mean = m_rate;

	for ( int i = 0 ; i < SOUNDDEVICE_SLOTS; ++i )
		m_sizes[i] = 0;
}

size_t AudioDeviceVirt::_nextslotNumber()
{
	if ( m_currslot+1 >= SOUNDDEVICE_SLOTS )
		return 0;
	else
		return m_currslot + 1;
}

double AudioDeviceVirt::getPreciseSampleRate()
{
	return m_rate_mean;
}

void AudioDeviceVirt::_kickThread()
{
	//boost::unique_lock<boost::mutex> lock( m_mutex_thread );
	m_cond_thread.notify_one();
}


void AudioDeviceVirt::_safeSetMode( State _state )
{
	boost::unique_lock<boost::mutex> lock( m_mutex_thread );
	m_state = _state;
}
