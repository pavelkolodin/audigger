/*
 *  pavelkolodin@gmail.com
 */

#ifndef _AudioDeviceALSA_H_
#define _AudioDeviceALSA_H_

#include "AudioDevice.h"

#include <iostream>
/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

/* All of the ALSA library API is defined
 * in this header */
#include <alsa/asoundlib.h>

#define SOUNDDEVICE_SLOTS 4
//#define SOUNDDEVICE_SLOT_SIZE_BYTES 44100*2 * 4



class Controller;

class AudioDeviceALSA : public AudioDevice
{
	public:
		AudioDeviceALSA ( Controller *_ctrl );
		virtual ~AudioDeviceALSA ( ) { }

		double getPreciseSampleRate();
	private:
//		AudioDeviceALSA() { }
//        AudioDeviceALSA ( const AudioDeviceALSA& _other ) { }
        AudioDeviceALSA& operator= ( const AudioDeviceALSA& _other ) { return *this; }

        virtual bool _validateParams(const AudioParams &_params) const;
        virtual bool _initAudio();
        virtual bool _startAudio();
        virtual void _stopAudio();
        virtual bool _waitAudio();
        virtual void _drainAudio();
        virtual bool _playAudio();

    	snd_pcm_uframes_t m_buffer_size; // what is this variable FOR???
    	snd_pcm_uframes_t m_period_size;
    	snd_pcm_t *m_alsa_pcm_handle;
    	snd_pcm_hw_params_t *m_hw_params;
    	snd_pcm_sw_params_t *m_sw_params;

    	struct pollfd *m_poll_ufds;
    	int m_poll_count;
};

#endif

