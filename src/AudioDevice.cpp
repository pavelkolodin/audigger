/*
 *  pavelkolodin@gmail.com
 */

#include "AudioDevice.h"
#include "Controller.h"
#include "Processor.h"
#include "BinderBoostThread.h"
#include <boost/chrono.hpp>
#include <fir/logger/logger.h>

AudioDevice :: AudioDevice ( Controller *_ctrl  )
	: m_ctrl( _ctrl )
	, m_currslot( 0 )
	, m_state( STOPPED )
	, m_buff_size( 0 )
	, m_load_portion( 0 )
	, m_frames_loaded( 0 )
	, m_slot_loading( 0 )
	, m_track_eof( 0 )
	, m_slot_off_frm( 0 )
	, m_cnt_play( 0 )
	, m_time_start( 0 )
	, m_rate_mean( 0 )
	, m_rate_mean_datasize( 0 )
	, m_thread_stop( 0 )
{
	// Allocate memory:
	m_buff_size = 1024 * 128;

	for ( int i = 0 ; i < SOUNDDEVICE_SLOTS; ++i )
	{
		m_buff[i] = new char[ m_buff_size ];
		m_sizes_frm[i] = 0; // no data loaded to this buffer.
	}

	// Thread:
	BinderBoostThread< AudioDevice > callable( this, 0 ); // can copy itself
	m_thread_stop = 0;
	m_thread = boost::thread( callable );
}


AudioDevice :: ~AudioDevice ( )
{
	for ( int i = 0 ; i < SOUNDDEVICE_SLOTS; ++i )
	{
		if ( m_buff[i] )
			delete[] m_buff[i];
	}

	LOG_PURE("AudioDevice stopping (waiting thread)...")
	// Stop thread:
	m_thread_stop = true;
	this->_kickThread();
	m_thread.join();
	LOG_PURE("AudioDevice stopped.")
}

void AudioDevice::threadFunction( unsigned _thread_id )
{
	//
	// snd_* function calls ARE NOT PROTECTED because we access them only in this thread!!!
	//
	State curr_state = UNDEFINED;
	while( 1 )
	{
		{//LOCK
			boost::unique_lock<boost::mutex> lock( m_mutex_thread );
			while ( STOPPED == m_state && ! m_thread_stop )
				m_cond_thread.wait(lock);

			curr_state = m_state;

			if ( m_thread_stop )
				return;
		}//UNLOCK

		switch( curr_state )
		{
			default:
			case LOADING:
			case STOPPED:
			case UNDEFINED:
				break;

			case PLAY_REQUESTED:
				LOG_PURE("PLAY_REQUESTED");
				if ( ! _initAudio() )
				{
					boost::unique_lock<boost::mutex> lock( m_mutex_thread );
					m_state = STOPPED;
					break;
				}

				if ( ! _startAudio() )
				{
					boost::unique_lock<boost::mutex> lock( m_mutex_thread );
					m_state = STOPPED;
					break;
				}

				{
					boost::unique_lock<boost::mutex> lock( m_mutex_thread );
					m_state = PLAYING;
				}

				break;
			case PLAYING:
				// LOG_PURE("PLAYING");
				// Wait until SND devise is ready.
				if ( _waitAudio() )
				{
					boost::unique_lock<boost::mutex> lock( m_mutex_thread );

					// TODO protect working with variables.
					if ( ! _playAudio() )
						m_state = STOP_REQUESTED;

					// Load next block?
					if ( m_slot_off_frm > 0 ) // TODO magic 0
					{
						// Load next slot.
						// If this action is not activated yet and current buffer was full.
						if ( ! m_slot_loading && false == m_track_eof && 0 == m_sizes_frm[_nextslotNumber()] )
						{
							// Load next slot:
							// return true/false
							Processor::RequestData r(this, 0, m_buff[_nextslotNumber()], m_group, m_frames_loaded, m_buff_size/m_params.frameSize());
							m_slot_loading = m_ctrl->getProcessor()->request( r );


//							m_slot_loading = m_ctrl->getProcessor()->loadData( m_track,
//									this,
//									0,
//									m_buff[_nextslotNumber()],
//									m_frames_loaded,
//									m_buff_size / m_params.frameSize() );
						}
					}

					// Switch to another slot if current slot is drained.
					if ( m_slot_off_frm >= m_sizes_frm[m_currslot] )
					{
						LOG_PURE("\n(SWITCH)\n");

						// Kill current slot.
						m_slot_off_frm = 0;
						m_sizes_frm[m_currslot] = 0; // empty

						// Switch to next slot. Will not switch if next buffer is EMPTY. Be careful.
						// TODO: What if next buffer is empty???
						if ( 0 != m_sizes_frm[ _nextslotNumber() ] )
							m_currslot = _nextslotNumber();
						else
						{
							LOG_ERROR( "SO WHAT? There is nothing to switch to... Dieing..." );
							m_state = DRAINING;
							break;
						}
					}
				}
				else
				{
					LOG_ERROR( "_waitAudio() == false" );
					boost::unique_lock<boost::mutex> lock( m_mutex_thread );
					m_state = STOP_REQUESTED;
					break;
				}
				break;

			case DRAINING:
				LOG_PURE("DRAINING");
				_drainAudio();
				{
					boost::unique_lock<boost::mutex> lock( m_mutex_thread );
					m_state = STOP_REQUESTED;
				}
				break;

			case STOP_REQUESTED:
				LOG_PURE("STOP_REQUESTED");
				_stopAudio();
				{
					boost::unique_lock<boost::mutex> lock( m_mutex_thread );
					m_state = STOPPED;
					m_cond_back.notify_one();
				}
				break;
		} // SWITCH
	} // while(1) main
}

bool AudioDevice::start( TracksGroup::Ptr _group )
{
	m_group = _group;
	boost::unique_lock<boost::mutex> lock( m_mutex_thread );

	if ( ! m_ctrl )
		return false;

	if ( STOPPED != m_state )
		return false;

	LOG_PURE("[S]");

	_initLogic();

	m_state = LOADING;
	m_frames_loaded = 0; //m_track->getCursorInfo().m_pos;

	// Load first block of data.
	// requestDone() will set m_state to:
	// PLAY_REQUESTED - if block loaded
	// STOP - if block cannot be loaded
	//
	// requestDone() will also set flag m_eof when zero frames loaded.

	Processor::RequestData r(this, 0, m_buff[0], m_group, m_frames_loaded, m_buff_size/m_params.frameSize());
	m_ctrl->getProcessor()->request( r );
	//m_ctrl->getProcessor()->loadData( m_track, this, 0, m_buff[0], m_frames_loaded, m_buff_size/m_params.frameSize() );

	// Wait for data to be loaded.

	// If we cannot load data, requestDone() will be called by Processor with argument 0 (zero).
	// and requestDone() will change m_state to "STOPPED".
	while ( LOADING == m_state )
		m_cond_back.wait(lock);

	if ( PLAY_REQUESTED == m_state )
		return true;

	return false;
}


void AudioDevice::stop()
{
	boost::unique_lock<boost::mutex> lock( m_mutex_thread );
	if ( PLAYING != m_state )
		return;

	m_state = STOP_REQUESTED;

	_kickThread();

	// Wait AudioDevice to stop.
	while ( STOPPED != m_state )
		m_cond_back.wait(lock);
}

AudioDevice::State AudioDevice::state()
{
	return m_state;
}

void AudioDevice::taskDone( const Processor::TaskAbstract &_task )
{
	const Processor::TaskData &task = (const Processor::TaskData &)_task;
	// TODO
	unsigned _size = task.resultSize();
	// TODO

	boost::unique_lock<boost::mutex> lock( m_mutex_thread );
	LOG_PURE ( "\nAudioDevice, LOADED: size " << _size << ", nxt " << _nextslotNumber() << ", st " << m_state );

	if ( STOPPED == m_state )
		return;

	if (0 == _size)
	{
		LOG_PURE("AudioDevice::requestDone(), cannot load data.");
		m_track_eof = true;
		if ( LOADING == m_state )
		{
			m_state = STOPPED;
			m_cond_back.notify_one();
		}
		return;
	}

	m_frames_loaded += _size;
	m_slot_loading = false;		// TODO: kill this flag

	if ( LOADING == m_state)
	{
		m_sizes_frm[ 0 ] = _size;
		m_state = PLAY_REQUESTED;
		_kickThread();

		m_cond_back.notify_one();
	}
	else
		m_sizes_frm[ _nextslotNumber() ] = _size;
}

bool AudioDevice::init( const AudioParams &_params )
{
	if ( ! _validateParams(_params) )
	{
		LOG_PURE ("AudioDevice::init(AudioParams&), _validateParams(AudioParams&): bad settings.");
		return false;
	}

	m_params = _params;
	return true;
}


//
// P R I V A T E
//


void AudioDevice::_initLogic()
{
	m_currslot = 0;
	m_slot_off_frm = 0;
	m_cnt_play = 0;
	m_time_start = 0;
	m_rate_mean_datasize = 0;
	m_track_eof = false;

	for ( int i = 0 ; i < SOUNDDEVICE_SLOTS; ++i )
		m_sizes_frm[i] = 0;
}

size_t AudioDevice::_nextslotNumber()
{
	if ( m_currslot+1 >= SOUNDDEVICE_SLOTS )
		return 0;
	else
		return m_currslot + 1;
}

double AudioDevice::getPreciseSampleRate()
{
	return m_rate_mean;
}

void AudioDevice::_kickThread()
{
	//boost::unique_lock<boost::mutex> lock( m_mutex_thread );
	m_cond_thread.notify_one();
}




