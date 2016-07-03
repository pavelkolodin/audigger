/*
 *  pavelkolodin@gmail.com
 */

#include "common.h"
#include "AudioDeviceALSA.h"

#include "Controller.h"
#include "Processor.h"
#include "BinderBoostThread.h"

#include <fir/logger/logger.h>

static unsigned int buffer_time = 500000;               /* ring buffer length in us */
static unsigned int period_time = 100000;               /* period time in us */
static int period_event = 0;                            /* produce poll event after each period */


void fadeIn(short *_buff, size_t _fadelen)
{
	float volume = 0;
	float step = 1.0/ (float)_fadelen;
	for ( size_t i = 0; i < _fadelen; ++i, volume += step )
	{
		_buff[i*2] *= volume;
		_buff[i*2+1] *= volume;
	}
}

const char* ALSAStateStr(snd_pcm_state_t _state)
{
	switch( _state )
	{
		default:
			return "Unknown state";
		case SND_PCM_STATE_OPEN:
			return "SND_PCM_STATE_OPEN";
		case SND_PCM_STATE_SETUP:
			return "SND_PCM_STATE_SETUP";
		case SND_PCM_STATE_PREPARED:
			return "SND_PCM_STATE_PREPARED";
		case SND_PCM_STATE_RUNNING:
			return "SND_PCM_STATE_RUNNING";
		case SND_PCM_STATE_XRUN:
			return "SND_PCM_STATE_XRUN";
		case SND_PCM_STATE_DRAINING:
			return "SND_PCM_STATE_DRAINING";
		case SND_PCM_STATE_PAUSED:
			return "SND_PCM_STATE_PAUSED";
		case SND_PCM_STATE_SUSPENDED:
			return "SND_PCM_STATE_SUSPENDED";
		case SND_PCM_STATE_DISCONNECTED:
			return "SND_PCM_STATE_DISCONNECTED";
	}
}

AudioDeviceALSA :: AudioDeviceALSA ( Controller *_ctrl  )
	: AudioDevice ( _ctrl )
	, m_buffer_size(0)
	, m_period_size(0)
	, m_alsa_pcm_handle(0) // !!!
	, m_hw_params(0)
	, m_sw_params(0)
{
}

bool AudioDeviceALSA::_playAudio()
{
	++ m_cnt_play;
	int err = 0;
	snd_pcm_uframes_t avail = 0;
	//snd_pcm_state_t state;

	for ( 	avail = snd_pcm_avail_update(m_alsa_pcm_handle);
			avail >= m_period_size;
			avail = snd_pcm_avail_update(m_alsa_pcm_handle) )
	{
		size_t currsize = std::min<int> ( m_sizes_frm[m_currslot] - m_slot_off_frm, m_period_size );
		//LOG_PURE( "[" << currsize << "]")

		if ( 0 == currsize )
			break;

		err = snd_pcm_writei(m_alsa_pcm_handle, m_buff[m_currslot] + ( m_slot_off_frm * m_params.frameSize() ), currsize);

		if ( err < 0 )
			return false;

		m_slot_off_frm += currsize;
	} // for. Until we have space to play.
	return true;
}

//
// P R I V A T E
//

bool AudioDeviceALSA::_validateParams(const AudioParams& _params) const
{
	if ( _params.m_channels < 1 || _params.m_channels > 2 ||  ! _params.goodRate( ) || ! _params.goodBits( ))
		return false;

	return true;
}

bool AudioDeviceALSA::_initAudio()
{
	int err = 0, dir = 0;
	unsigned int rrate = m_params.m_rate;
	const char *device_name = "default";
	//const char *device_name = "hw:1,0";
	m_hw_params = NULL;
	m_sw_params = NULL;
	m_buffer_size = 0; // get
	m_period_size = 0; // get

	m_rate_mean = m_params.m_rate;

	// Open the device.
	err = snd_pcm_open(&m_alsa_pcm_handle, device_name, SND_PCM_STREAM_PLAYBACK, 0);
	if ( err < 0 )
	{
		LOG_PURE ( "snd_pcm_open() failed" );
		m_alsa_pcm_handle = NULL;
		return false;
	}

	//snd_pcm_hw_params_malloc (&m_hw_params);
	snd_pcm_hw_params_alloca(&m_hw_params);
	snd_pcm_sw_params_alloca(&m_sw_params);

	err = snd_pcm_hw_params_any(m_alsa_pcm_handle, m_hw_params);
	if ( err < 0 ) { LOG_ERROR( snd_strerror(err) ); return false; }
	err = snd_pcm_hw_params_set_access (m_alsa_pcm_handle, m_hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if ( err < 0 ) { LOG_ERROR( snd_strerror(err) ); return false; }


	//err = snd_pcm_hw_params_set_rate_resample(m_alsa_pcm_handle, params, resample);
	//if ( err < 0 ) { std::cout << "ERROR: line: " << __LINE__ << ", " << snd_strerror(err) << "\n"; return false; }

	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

	if ( (16 == m_params.m_bits) && (! m_params.m_float) && (m_params.m_signed) && ( ! m_params.m_bigendian) )
		format = SND_PCM_FORMAT_S16_LE;
	else if ( (16 == m_params.m_bits) && (! m_params.m_float) && (! m_params.m_signed) && ( ! m_params.m_bigendian) )
		format = SND_PCM_FORMAT_U16_LE;
	else if ( (32 == m_params.m_bits) && (m_params.m_float) && (m_params.m_signed) && ( ! m_params.m_bigendian) )
		format = SND_PCM_FORMAT_FLOAT_LE;



	err = snd_pcm_hw_params_set_format(m_alsa_pcm_handle, m_hw_params, format);
	if ( err < 0 ) { LOG_ERROR( snd_strerror(err) ); return false; }

	err = snd_pcm_hw_params_set_channels(m_alsa_pcm_handle, m_hw_params, m_params.m_channels);
	if ( err < 0 ) { LOG_ERROR( snd_strerror(err) ); return false; }

	err = snd_pcm_hw_params_set_rate_near(m_alsa_pcm_handle, m_hw_params, &rrate, &dir);
	if ( err < 0 ) { LOG_ERROR( snd_strerror(err) ); return false; }

	err = snd_pcm_hw_params_set_buffer_time_near(m_alsa_pcm_handle, m_hw_params, &buffer_time, &dir);
	if ( err < 0 ) { LOG_ERROR( snd_strerror(err) ); return false; }

	err = snd_pcm_hw_params_get_buffer_size(m_hw_params, &m_buffer_size);
	if ( err < 0 ) { LOG_ERROR( snd_strerror(err) ); return false; }

	err = snd_pcm_hw_params_set_period_time_near(m_alsa_pcm_handle, m_hw_params, &period_time, &dir);
	if ( err < 0 ) { LOG_ERROR( snd_strerror(err) ); return false; }

	err = snd_pcm_hw_params_get_period_size(m_hw_params, &m_period_size, &dir);
	if ( err < 0 ) { LOG_ERROR( snd_strerror(err) ); return false; }

	err = snd_pcm_hw_params (m_alsa_pcm_handle, m_hw_params);
	if ( err < 0 ) { LOG_ERROR( snd_strerror(err) ); return false; }

	// SW
	err = snd_pcm_sw_params_current(m_alsa_pcm_handle, m_sw_params);
	if ( err < 0 ) { LOG_ERROR( snd_strerror(err) ); return false; }

	err = snd_pcm_sw_params_set_start_threshold(m_alsa_pcm_handle, m_sw_params, m_period_size);
	if ( err < 0 ) { LOG_ERROR( snd_strerror(err) ); return false; }

	//err = snd_pcm_sw_params_set_avail_min(m_alsa_pcm_handle, m_sw_params, period_event ? m_buffer_size : (m_period_size*3));
	err = snd_pcm_sw_params_set_avail_min(m_alsa_pcm_handle, m_sw_params, m_period_size );
	if ( err < 0 ) { LOG_ERROR( snd_strerror(err) ); return false; }

	if (period_event)
	{
		err = snd_pcm_sw_params_set_period_event(m_alsa_pcm_handle, m_sw_params, 1);
		if ( err < 0 ) { LOG_ERROR( snd_strerror(err) ); return false; }
	}

	err = snd_pcm_sw_params(m_alsa_pcm_handle, m_sw_params);
	if ( err < 0 ) { LOG_ERROR( snd_strerror(err) ); return false; }

//	err = snd_pcm_prepare(m_alsa_pcm_handle);
//	if ( err < 0 ) { std::cout << "ERROR: line: " << __LINE__ << ", " << snd_strerror(err) << "\n"; return false; }

	// Register CALLBACK:
	//err = snd_async_add_pcm_handler(&ahandler, m_alsa_pcm_handle, ALSA_async_callback, this);
	//if ( err < 0 ) { std::cout << "ERROR: line: " << __LINE__ << ", " << snd_strerror(err) << "\n"; return false; }

	m_poll_count = snd_pcm_poll_descriptors_count (m_alsa_pcm_handle);
	if (m_poll_count <= 0) { LOG_ERROR("m_poll_count <= 0"); return false; }

	m_poll_ufds = new struct pollfd[m_poll_count];
	if (m_poll_ufds == NULL) { LOG_ERROR( "m_poll_ufds == NULL" ); return false; }

	err = snd_pcm_poll_descriptors(m_alsa_pcm_handle, m_poll_ufds, m_poll_count);

	if ( err < 0 ) { LOG_ERROR( "snd_pcm_poll_descriptors, err < 0" ); return false; }

	LOG_PURE(" @@@@@@@@@@@@@@@@@@@ ")
	LOGVAR2( m_buffer_size, m_period_size )

	return true;
}

double AudioDeviceALSA::getPreciseSampleRate()
{
	return m_rate_mean;
}

bool AudioDeviceALSA::_startAudio()
{
	int err = snd_pcm_prepare(m_alsa_pcm_handle);

	if ( err < 0 )
	{
		LOG_ERROR ( snd_strerror(err) )
		return false;
	}

	if (SND_PCM_STATE_PREPARED != snd_pcm_state(m_alsa_pcm_handle))
		return false;

	// Call _playAudio 1 times to have some samples in buffer before snd_pcm_start.
	if ( ! _playAudio() )
	{
		LOG_ERROR( "Cannot feed frames before snd_pcm_start()." )
		return false;
	}


	LOG_PURE ("Starting...")
	//err = snd_pcm_start(m_alsa_pcm_handle);
	if ( err < 0 )
	{
		LOG_ERROR( snd_strerror(err) );
		return false;
	}

	LOG_PURE ("Started!")
	return true;
}

bool AudioDeviceALSA::_waitAudio()
{
	unsigned short revents;

	poll(m_poll_ufds, m_poll_count, 100); // 100 ms

	snd_pcm_poll_descriptors_revents(m_alsa_pcm_handle, m_poll_ufds, m_poll_count, &revents);
	if (revents & POLLERR)
	{
		LOG_ERROR( "POLLERR" );
		//return -EIO;
		return false;
	}

	//if ( ! (revents & POLLOUT) )
		//return false;

	return true;
}

void AudioDeviceALSA::_drainAudio()
{
	snd_pcm_drain(m_alsa_pcm_handle);
}

void AudioDeviceALSA::_stopAudio()
{
	snd_pcm_close(m_alsa_pcm_handle);
	m_alsa_pcm_handle = NULL;
}



