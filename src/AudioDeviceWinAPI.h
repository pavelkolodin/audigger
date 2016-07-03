#ifndef _AUDIODEVICE_WINAPI_H_
#define _AUDIODEVICE_WINAPI_H_


#include <Windows.h>
#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/condition_variable.hpp>

#include "AudioDevice.h"


/// \brief Cross-platform class for working with audio devices.
class AudioDeviceWinAPI : public AudioDevice
{
public:
	AudioDeviceWinAPI( Controller *_ctrl );
	//virtual ~AudioDeviceWinAPI();

	void _postDoneEvent();

private:
	// Disable copying of all sorts:
	AudioDeviceWinAPI(const AudioDeviceWinAPI& _other);
	AudioDeviceWinAPI& operator=(const AudioDeviceWinAPI&) { return *this; }

	// inherited from AudioDevice:
	virtual bool _validateParams( const AudioParams& ) const { return true; /* TODO */ }
	virtual bool _initAudio();
    virtual bool _startAudio();
    virtual void _stopAudio();
    virtual bool _waitAudio();
    virtual void _drainAudio();
    virtual bool _playAudio();


	boost::mutex m_mutex_state;
	boost::condition_variable m_cond_state;

	bool m_ready;
	double m_rate_mean;
	size_t m_rate_mean_datasize;
	unsigned m_position_old;

	long long m_lastsentperiod_time;
	size_t m_lastsentperiod_frame;
	size_t m_frames_sent;

	HWAVEOUT m_handler_winapi;
	WAVEHDR m_pwh[ 4 ];
	unsigned m_pwh_curr;
};

#endif
