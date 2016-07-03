/*
 *  pavelkolodin@gmail.com
 */

#ifndef _AudioDevice_H_
#define _AudioDevice_H_
#include <iostream>

#define SOUNDDEVICE_SLOTS 4

// to lock callback;
#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/condition_variable.hpp>

#include <IProcessorClient.h>
#include "AudioParams.h"
#include "TracksGroup.h"

class Controller;

class AudioDevice : public Processor::IProcessorClient
{
	public:
		enum State
		{
			UNDEFINED = 0,
			PLAY_REQUESTED,
			PLAYING,	// playing blocks / requesting blocks
			STOP_REQUESTED,
			STOPPED,	// initial state
			LOADING,	// requesting first block of data
			DRAINING
		};
		AudioDevice ( Controller *_ctrl );
		virtual ~AudioDevice ( );

		///
		/// \return true - ok
		/// \return false - cannot init with this parameters.
		virtual bool init( const AudioParams & );
		bool start( TracksGroup::Ptr _track );
		void stop();
		State state();
		virtual double getPreciseSampleRate();
		void threadFunction( unsigned _thread_id );

		// IProcessorClient:
		// _task_id ignored:
		virtual bool taskStillValid(unsigned _task_id) { return true; }
		virtual void taskDone(const Processor::TaskAbstract &_task);


	protected:
        AudioDevice& operator= ( const AudioDevice& _other ) { return *this; }

    	void _initLogic();
		void _kickThread();
    	size_t _nextslotNumber();

        // Please implement these functions in your Audio device specific code.

    	// Is params good for this audio device?
    	virtual bool _validateParams( const AudioParams& ) const = 0;
        virtual bool _initAudio() = 0;
        virtual bool _startAudio() = 0;
        virtual void _stopAudio() = 0;
        virtual bool _waitAudio() = 0;
        virtual void _drainAudio() = 0;
        virtual bool _playAudio() = 0;	// play portion of data


        TracksGroup::Ptr m_group;
    	Controller *m_ctrl;
    	AudioParams m_params;
    	unsigned m_currslot; // index

    	State m_state;

    	// Buffers.
    	char *m_buff[SOUNDDEVICE_SLOTS];
    	size_t m_buff_size; // samples != frames !!!
    	size_t m_load_portion;
    	size_t m_sizes_frm[SOUNDDEVICE_SLOTS]; // loaded
    	size_t m_frames_loaded;
    	bool m_slot_loading;
    	bool m_track_eof;
    	//size_t m_slot_size;	// FRAMES. how many useful frames a single slot can contain. Depend on "m_period_frames".

    	// Playing data.
    	// Offset in current frame (frames)
    	size_t m_slot_off_frm;
    	unsigned m_cnt_play; // how many times "play()" called.

    	// Rate measurement.
    	long long m_time_start;
    	double m_rate_mean;
    	size_t m_rate_mean_datasize;

    	// Thread:
    	boost::thread m_thread;
    	boost::mutex m_mutex_thread;
    	boost::condition_variable m_cond_thread;
    	boost::condition_variable m_cond_back;
    	bool m_thread_stop;
};

#endif

