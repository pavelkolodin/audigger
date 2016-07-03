/**
 *  pavelkolodin@gmail.com
 */

#include <iostream>
#include <fstream> // Processor::test()
#ifndef _USE_MATH_DEFINES
	#define _USE_MATH_DEFINES 1
#endif
#include <math.h>
#include <sndfile.h>
#include <boost/foreach.hpp>
#include <fir/logger/logger.h>

#include "Processor.h"
#include "Controller.h"
#include "TrackAudio.h"
#include "TrackMarks.h"
#include "TracksGroup.h"

#include "LoggerPkl.h"

#include "audio_common.h"

#define SIZE_BUFFER_COPYPASTE 1024*1024*4

unsigned getLimitedNumberCPU()
{
	unsigned cpus = boost::thread::hardware_concurrency();
	// cpus = cpus & 0x3f;
	if ( cpus > 64 )
		cpus = 64;
	if ( cpus < 1 )
		cpus = 1;

	return cpus;
}

namespace Processor
{


Processor::Processor( Controller *_ctrl )
: m_ctrl( _ctrl )
, m_stop_threads( 0 )
, m_threads_running( 0 )
{
	LOG_PURE ( "CPUs detected: " << getLimitedNumberCPU() );

	m_requests_data.resize( 4 );
	m_requests_fft.resize( 64 );
	m_requests_waveform.resize( 16 );
	m_requests_export.resize( 4 );

	// i don't know why i start +1 thread, it is like make -j(N+1) habit.
	_start_threads( getLimitedNumberCPU() + 1 );
}

Processor::~Processor()
{
	_stop_threads();
}

void Processor::threadFunction(unsigned _thread_id)
{
	while ( 1 )
	{
		{
			// LOCK
			boost::unique_lock<boost::mutex> lock( m_mutex_taskinfo );
			while(	(! m_stop_threads) &&
					m_requests_data.empty() &&
					m_requests_fft.empty() &&
					m_requests_waveform.empty() &&
					m_requests_export.empty() )
				m_cond_taskinfo.wait(lock);

			if ( m_stop_threads )
			{
				LOG_PURE ( "Processor::threadFunction(), thread " << _thread_id << " exit." );
				break;
			}

			if ( ! m_requests_data.empty() )
			{
				m_tasks_data[ _thread_id ].init( m_requests_data.back() );
				m_requests_data.pop();
			}
			else
			if ( ! m_requests_fft.empty() )
			{
				m_tasks_fft[ _thread_id ].init( m_requests_fft.back() );
				m_requests_fft.pop();
			}
			else
			if ( ! m_requests_waveform.empty() )
			{
				m_tasks_waveform[ _thread_id ].init( m_requests_waveform.back() );
				m_requests_waveform.pop();
			}
			else
			if ( ! m_requests_export.empty() )
			{
				m_tasks_export[ _thread_id ].init( m_requests_export.back() );
				m_requests_export.pop();
			}
		} // LOCK


		// Try to start tasks
		// If task is not initialized, its start() will return immideately.
		m_tasks_data[ _thread_id ].start();
		m_tasks_fft[ _thread_id ].start();
		m_tasks_waveform[ _thread_id ].start();
		m_tasks_export[ _thread_id ].start();
	} // while(1)
}

size_t Processor::getThreads()
{
	return m_threads_running;
}


bool Processor::request(const RequestFFT &_r)
{
	boost::unique_lock<boost::mutex> lock( m_mutex_taskinfo );
	if ( m_requests_fft.full() )
		return false;

	m_requests_fft.front() = _r;
	m_requests_fft.push();
	m_cond_taskinfo.notify_one();
	return true;
}

bool Processor::request(const RequestWaveform &_r)
{
	boost::unique_lock<boost::mutex> lock( m_mutex_taskinfo );
	if ( m_requests_waveform.full() )
		return false;

	m_requests_waveform.front() = _r;
	m_requests_waveform.push();
	m_cond_taskinfo.notify_one();
	return true;
}

bool Processor::request(const RequestData& _r)
{
	boost::unique_lock<boost::mutex> lock( m_mutex_taskinfo );
	if ( m_requests_data.full() )
		return false;

	m_requests_data.front() = _r;
	m_requests_data.push();
	m_cond_taskinfo.notify_one();
	return true;
}


bool Processor::request(const RequestExport& _r)
{
	boost::unique_lock<boost::mutex> lock( m_mutex_taskinfo );
	if ( m_requests_export.full() )
		return false;

	m_requests_export.front() = _r;
	m_requests_export.push();
	m_cond_taskinfo.notify_one();
	return true;
}


bool Processor::copy(TracksGroup::Ptr _group)
{
	TrackAudio::Ptr track = _group->getTrackAudioSelectedFirst();
	if ( ! track)
		return false;

	if ( ! track->getCursorInfo().m_seltime_len )
		return false;

	boost::unique_lock < boost::mutex > lock( track->getMutex() );

	// TODO: magic 4 MB
	m_ab_copypaste.reset ( new AudioBlock( track->getAudioParams(), true, SIZE_BUFFER_COPYPASTE ));

	track->seek( track->getCursorInfo().m_seltime_start, SEEK_SET );

	size_t written = transferFrames<AudioBlock, TrackAudio> ( m_ab_copypaste.get(),
			track.get(),
			track->getCursorInfo().m_seltime_len );

	if ( track->getCursorInfo().m_seltime_len != written )
		return false;

	return true;
}

bool Processor::paste(TracksGroup::Ptr _group, bool _insert)
{
	if ( ! m_ab_copypaste )
		return false;

	TrackAudio::Ptr track = _group->getTrackAudioSelectedFirst();
	if ( ! track)
		return false;

	boost::unique_lock < boost::mutex > lock( track->getMutex() );

	if ( ! track->getAudioParams().compartible( m_ab_copypaste->getAudioParams() ) )
	{
		LOG_ERROR( "Buffer and destination track are not compartible." );
		return false;
	}

	if ( ! track->insertData( track->getCursorInfo().m_pos, *m_ab_copypaste, _insert ) )
		return false;

	return true;
}

bool Processor::erase(TracksGroup::Ptr _group)
{
	TrackAudio::Ptr track = _group->getTrackAudioSelectedFirst();
	if ( ! track)
		return false;

	if ( ! track->getCursorInfo().m_seltime_len )
		return false;

	boost::unique_lock < boost::mutex > lock( track->getMutex() );

	if ( track->erase( track->getCursorInfo().m_seltime_start, track->getCursorInfo().m_seltime_len ) )
	{
		track->getCursorInfo().clearSelTime();
		return true;
	}
	return false;
}

// erase selected part of track
bool Processor::cut(TracksGroup::Ptr _group)
{
	if ( copy( _group ) )
		return erase ( _group );

	return false;
}

bool Processor::processDSP(TrackAudio::Ptr _track)
{
//	// 1. Call "copy()" (copy selected time interval into RAM).
//	// 2. Allocate memory for the interval: M1
//	// 3. Do DSP operation: copypaste buffer -> M1
//	// 4. Copy M1 to copypaste buffer.
//	// 5. Call "paste()" (paste the copypaste buffer into track, blocktype=RAM).
//
//	// TODO
//	// !!!
//	// TOO MUCH MEMORY CONSUMPTION: (1)copypaste buffer, (2)M1, (3)blocktype=RAM !!!
//
//	// 1. Remove copypaste memory buffer. Instead use special temporary track that can keep data on disk, if data >= 16MB.
//	// 2. Use the same temporary track for M1 replacement.
//	// 3. Process the data by blocks. Not the whole piece at a time.
//
//	// We use bufcopy as a temporary storage for processing data.
//	// TODO: this is definitely not a good approach :)
//	if ( ! this->copy( _track_id ) )
//		return false;
//
//	LOGVAR2( m_bufcopy_mi.m_frames, m_bufcopy_mi.m_channels );
//
//	try
//	{
//		m_bufwork.resize( m_bufcopy_mi.m_frames * m_bufcopy_mi.m_channels );
//	} catch (...)
//	{
//		return false;
//	}
//
//	LOGPLACE;
//	m_dsp->Restart();
//	m_dsp->setChannels( m_bufcopy_mi.m_channels );
//
//	size_t processed = m_dsp->Process16(&m_bufcopy[0], &m_bufwork[0], m_bufcopy_mi.m_frames );
//	LOGPLACE;
//	LOGVAR1( processed );
//	m_dsp->Flush16( &m_bufwork[processed * m_bufcopy_mi.m_channels] );
//
//	memcpy(&m_bufcopy[0], &m_bufwork[0], sizeof(short) * m_bufcopy_mi.m_frames * m_bufcopy_mi.m_channels );
//
//	//this->setSelTime( m_trackattr[_track_id].m_seltime_start, 0 );
//
//	this->paste(_track_id, m_tracks[ _track_id ].m_seltime_start, false ); // false == overwrite (
//	LOGPLACE;
	return true;
}

//
// P R I V A T E
//
void Processor::_start_threads(unsigned _num)
{
	if ( ! _num || m_threads_running >= _num )
		return;

	m_threads_info.resize( _num );

	for ( unsigned i = 0; i < _num; ++i )
	{
		m_threads_info[i].m_cond = new boost::condition_variable();
		m_threads_info[i].m_mutex = new boost::mutex();

		BinderBoostThread<Processor> b(this, i);
		m_Threads.create_thread( b );
	}
	m_threads_running = _num;
	m_tasks_data.resize( _num );
	m_tasks_fft.resize( _num );
	m_tasks_waveform.resize( _num );
	m_tasks_export.resize( _num );
}

void Processor::_stop_threads()
{
	{
		boost::unique_lock<boost::mutex> lock( m_mutex_taskinfo );
		m_stop_threads = true;
	}

	m_cond_taskinfo.notify_all();

	LOG_PURE("wait...");
	m_Threads.join_all();
	LOG_PURE("wait OK");
}


//void Processor::_taskExport( const ProcessorTaskInfo &_task )
//{
//	// TODO: mutex
//	TrackAudio::Ptr track = _task.m_track_audio;
//
//	if ( 0 == track->getAudioParams().m_frames )
//	{
//		LOGERROR ( "Empty audio track." );
//		return;
//	}
//
//	// How much frames we should export to a file?
//	// Copy channels, rate.
//	AudioParams ap = track->getAudioParams();
//	size_t todo = ap.m_frames;
//	size_t portion = 1024 * 128;
//	size_t offset = 0;
//
//	AudioFile file;
//
//	if ( ! file.open( _task.m_filename.c_str(), AudioFile::MODE_WRITE, &ap ) )
//	{
//		LOGERROR( "! _task.m_file->open()" );
//		_task.m_callback -> taskDone( _task.m_task_id, 0 );
//		return;
//	}
//
//	std::vector <char> buff;
//	buff.resize( portion * ap.frameSize() );
//
//	while( todo > 0 )
//	{
//		// Read from track
//
//		track->seek(_task.m_load_from + offset, SEEK_SET );
//		size_t rd = track->read( &buff[0], portion );
//		size_t wr = 0;
//
//		if ( ! rd )
//		{
//			LOGERROR( "Cannot read from track." );
//			break;
//		}
//		// Write to a file
//		wr = file.write( &buff[0], rd );
//		if ( wr != rd )
//		{
//			LOGERROR ( "Cannot write to a file." );
//			break; // TODO: error reporting: reason: cannot write to a file.
//		}
//		todo -= rd;
//		offset += rd;
//
//		// send percent:
//		double percent = ((double)offset) / (((double)ap.m_frames) / 100.0);
//
//		_task.m_callback->taskPercent( _task.m_task_id, (unsigned)percent );
//	}
//
//	// 0 - status
//	_task.m_callback->taskDone( _task.m_task_id, 0 );
//	LOGERROR ( "Export finished." );
//	file.close();
//}

}

