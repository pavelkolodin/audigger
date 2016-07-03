/*
 *  pavelkolodin@gmail.com
 */

#ifndef _AudioDeviceVIRT_H_
#define _AudioDeviceVIRT_H_
#include <iostream>
/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

// to lock callback;
#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/condition_variable.hpp>

#include <IProcessorClient.h>

#define SOUNDDEVICE_SLOTS 4

class Controller;

class AudioDeviceVirt : public IProcessorClient
{		
	public:
		enum State
		{
			UNDEFINED,
			STOPPED,	// initial state
			STOP_REQUESTED,
			LOADING,	// requesting first block of data
			PLAYING,		// playing blocks / requesting blocks
			DRAINING
		};
		AudioDeviceVirt ( Controller *_ctrl );
		virtual ~AudioDeviceVirt ( );

		void setTrackId( unsigned _track_id ) { m_track_id = _track_id; }
		// IProcessorClient:
		virtual void requestDone( size_t _size );

		// Interface:
		bool init(unsigned _bits, unsigned _rate, unsigned _channels);

		bool start( size_t _from_sample );
		void stop();

		// Play period
		void play();

		//size_t getSlotsSize() { return SOUNDDEVICE_SLOTS; }

		// Give first slot. Called once when starting playing.
		//bool getFirstSlot( short **_buff, size_t *_size);

		// Give an adress of next block, where data must be loaded.
		// Called each time when data must be preloaded.
		//bool getNextSlot( short **_buff, size_t *_size);

		// Called when slot loading is done.
		//void loadSlotDone(size_t _loaded_size);

		//bool isCapablePreciseSampleRate() { return false; }

		double getPreciseSampleRate();

		void _kickThread();
		void threadFunction( unsigned _thread_id );
	private:
		AudioDeviceVirt() { }
        AudioDeviceVirt ( const AudioDeviceVirt& _other ) { }
        AudioDeviceVirt& operator= ( const AudioDeviceVirt& _other ) { }
    	void _resetPlayingData();
    	size_t _nextslotNumber();
    	void _safeSetMode( State _mode );

    	Controller *m_ctrl;
    	unsigned m_rate;
    	unsigned m_bits;
    	unsigned m_channels;
    	unsigned m_currslot; // index
    	size_t m_slot_off;	// FRAMES
    	size_t m_slot_size;	// FRAMES. how many useful frames a single slot can contain. Depend on "m_period_frames".

    	short *m_buff[SOUNDDEVICE_SLOTS];
    	size_t m_buffsamplesize; // samples != frames !!!
    	size_t m_sizes[SOUNDDEVICE_SLOTS]; // loaded
    	bool m_slot_loading;
    	bool m_first_loading;

    	unsigned m_cnt_play; // how many times "play()" called.

    	long long m_time_start;
    	double m_rate_mean;
    	size_t m_rate_mean_datasize;

    	// Thread:
    	boost::thread m_thread;
    	boost::mutex m_mutex_thread;
    	boost::condition_variable m_cond_thread;

    	//boost::mutex m_mutex_back;
    	boost::condition_variable m_cond_back;
    	bool m_cannot_load_data;

    	bool m_thread_stop;

    	struct pollfd *m_poll_ufds;
    	int m_poll_count;
    	State m_state;
    	unsigned m_track_id;
    	size_t m_frames_loaded;
};

#endif

