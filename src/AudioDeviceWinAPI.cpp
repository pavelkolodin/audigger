/**
  *  pavelkolodin@gmail.com
  */

#include <iostream>

#include <fir/logger/logger.h>
#include "AudioDeviceWinAPI.h"
#include <boost/chrono.hpp>

#include <fstream> // write log

//#define DBGCOUT 1
void CALLBACK waveOutProc(
  HWAVEOUT hwo,
  UINT uMsg,
  DWORD_PTR dwInstance,
  DWORD_PTR dwParam1,
  DWORD_PTR dwParam2
);

// Callback used by wavOut.
void CALLBACK waveOutProcMy(	HWAVEOUT _hwo,
								UINT uMsg,
								DWORD_PTR dwInstance,
								DWORD_PTR dwParam1,
								DWORD_PTR dwParam2 )
{
	AudioDeviceWinAPI *sd = (AudioDeviceWinAPI*)dwInstance;
	if ( ! sd )
	{
		LOG_PURE( "waveOutProcMy(), sd == NULL!" );
		return; // very very sad
	}
	
	switch ( uMsg )
	{
	default:
		break;
	case WOM_OPEN:
		LOG_PURE ( "(WOM_OPEN)" );
		break;
	case WOM_DONE:
		LOGPKL ( "(WOM_DONE)" );
		sd->_postDoneEvent();
		break;
	case WOM_CLOSE:
		LOG_PURE ( "(WOM_CLOSE)" );
		break;
	}
}


void _printRes( MMRESULT res )
{
	switch( res )
	{
	default:
		case MMSYSERR_NOERROR:
			LOG_PURE("(MMSYSERR_NOERROR)" );
			break;
		case MMSYSERR_INVALHANDLE:
			LOG_PURE("(MMSYSERR_INVALHANDLE)");
			break;
		case MMSYSERR_NODRIVER:
			LOG_PURE( "(MMSYSERR_NODRIVER)");
			break;
		case MMSYSERR_NOMEM:
			LOG_PURE( "(MMSYSERR_NOMEM)");
			break;
		case WAVERR_UNPREPARED:
			LOG_PURE( "(WAVERR_UNPREPARED)");
			break;
	}
}



AudioDeviceWinAPI::AudioDeviceWinAPI( Controller *_ctrl )
	: AudioDevice( _ctrl )
	, m_handler_winapi(0)
	, m_pwh_curr(0)
	, m_ready(false)
{
}

bool AudioDeviceWinAPI::_initAudio()
{
	//
	// WinAPI
	WAVEFORMATEX pwfx;
	pwfx.wFormatTag = WAVE_FORMAT_PCM;
	pwfx.nChannels = m_params.m_channels;
	pwfx.nSamplesPerSec = m_params.m_rate;

	pwfx.wBitsPerSample = m_params.m_bits;
		pwfx.nBlockAlign = pwfx.nChannels * pwfx.wBitsPerSample/8;
	pwfx.nAvgBytesPerSec = pwfx.nBlockAlign * pwfx.nSamplesPerSec;

	pwfx.cbSize = 0; // ifnored for "WAVE_FORMAT_PCM".

	MMRESULT res = waveOutOpen( &m_handler_winapi,
								WAVE_MAPPER,
								&pwfx,
								(DWORD_PTR)waveOutProcMy, //NULL, // dwCallback,
								(DWORD_PTR)this, // dwCallbackInstance,
								CALLBACK_FUNCTION );
	LOG_PURE( "m_handler_winapi " << m_handler_winapi);
	_printRes ( res );
	if ( MMSYSERR_NOERROR != res )
		return false;

	return true;
}

bool AudioDeviceWinAPI::_startAudio()
{
	m_ready = true;
	return true;
}

void AudioDeviceWinAPI::_stopAudio()
{
	MMRESULT res;
	LOG_PURE( "AudioDeviceWinAPI::stop()...");
	// Stop playing and reset playing position to 0.
	if ( m_handler_winapi )
	{
		res = waveOutReset(m_handler_winapi);
		_printRes( res );
	}
	LOG_PURE( "AudioDeviceWinAPI::stop() OK");
}

bool AudioDeviceWinAPI::_waitAudio()
{
	if ( m_cnt_play < 2 )
		return true;

	//LOGPLACE;
	boost::chrono::milliseconds ms( 50 );
	boost::unique_lock<boost::mutex> lock( m_mutex_state );
	if ( ! m_ready )
		m_cond_state.wait_for(lock, ms);
	//LOGPLACE
	return true;
}

void AudioDeviceWinAPI::_drainAudio()
{
	// nothing? TODO!
}

bool AudioDeviceWinAPI::_playAudio()
{
	//LOG_PURE( "AudioDeviceWinAPI::play() " << m_cnt_play << ", " << m_currslot );

	if ( ! m_ready && (m_cnt_play > 1) )
		return true;

	// Very important: first 2 steps must not empty buffer immediately, because Processor cannot load data so fast to next buffer
	// and you will stop with "cannot switch to next buffer".
	// There is a problem: if current slot has too few frames, _playAudio will divide it for 4 and
	// will not be able to put them with appropriate speed! Unsolved! TODO!
	size_t currsize = std::min<size_t> ( m_sizes_frm[m_currslot] - m_slot_off_frm, m_sizes_frm[m_currslot] / 4 );
	if ( ! currsize )
		return true;

	//LOGVAR4( getCurrentTimeMs(), m_currslot, m_slot_off, currsize );
	LOG_PURE ( "(P " << currsize << ")" );
	MMRESULT res;
	//res = waveOutUnprepareHeader(m_handler, &m_pwh, sizeof(WAVEHDR) );

	m_pwh[m_pwh_curr].lpData = (LPSTR)(m_buff[m_currslot] + m_slot_off_frm * m_params.frameSize() );
	m_pwh[m_pwh_curr].dwBufferLength = (DWORD)(currsize * m_params.frameSize() ); //m_period_frames * sizeof(short);
	m_pwh[m_pwh_curr].dwBytesRecorded = 0;
	m_pwh[m_pwh_curr].dwUser = 0;
	m_pwh[m_pwh_curr].dwFlags = 0;
	m_pwh[m_pwh_curr].dwLoops = 0;
	m_pwh[m_pwh_curr].lpNext = NULL;

	res = waveOutPrepareHeader(m_handler_winapi, &m_pwh[m_pwh_curr], sizeof(WAVEHDR) );

	#ifdef DBGCOUT
	LOG_PURE ( "AudioDeviceWinAPI::play(), waveOutPrepareHeader()" );
	_printRes( res );
	#endif

	if ( MMSYSERR_NOERROR != res )
		return false;

	res = waveOutWrite(m_handler_winapi, &m_pwh[m_pwh_curr], sizeof(WAVEHDR) );

	#ifdef DBGCOUT
		LOG_PURE( "AudioDeviceWinAPI::play(), waveOutWrite()");
		_printRes( res );
	#endif

	if ( MMSYSERR_NOERROR != res )
		return false;

	m_slot_off_frm += currsize;
	++ m_cnt_play;

	m_ready = false;

	++ m_pwh_curr;
	if ( 4 == m_pwh_curr )
		m_pwh_curr = 0;

	return true;
}

//
// PRIVATE
//

void AudioDeviceWinAPI::_postDoneEvent()
{
	boost::unique_lock<boost::mutex> lock( m_mutex_state );
	m_ready = true;
	m_cond_state.notify_one();
}
