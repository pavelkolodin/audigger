/**
 *  pavelkolodin@gmail.com
 */

#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

#include <vector>
#include <iostream>
//#include <boost/threadpool.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/condition_variable.hpp>


#include <fir/logger/logger.h>
#include "BinderBoostThread.h"
#include "IProcessorClient.h"
#include "RingBuff.h"
#include "AudioFile.h"
#include "AudioParams.h"
#include "TracksGroup.h"
#include "ProcessorTasks.h"

class Controller;

namespace Processor
{


// ThreadInfo - special information for each thread.
class ThreadInfo
{
public:
	boost::condition_variable	*m_cond; // POINTER because we use std::vector<>::resize(), used to start thread's work
	boost::mutex				*m_mutex; // used to protect "m_cond" variable.
};



// Why Processor is a IProcessorClient?
class Processor // : public IProcessorClient
{
public:
	Processor(Controller *);
	virtual ~Processor();

	// need to be PUBLIC!
	void threadFunction(unsigned _thread_id);

	size_t getThreads();

	/// \return true - task accepted
	/// \return false - task rejected
	bool request(const RequestFFT &);
	bool request(const RequestWaveform &);
	bool request(const RequestData&);
	bool request(const RequestExport&);


	// Copy selected part of time of selected track of group "_group_id".
	// TODO: delete return value? (because of track's revision)
	bool copy(TracksGroup::Ptr _group);

	// Paste buffer to selected track
	// TODO: delete return value? (because of track's revision)
	bool paste(TracksGroup::Ptr _group, bool _insert);

	// Erase selected time interval from each selected track.
	// TODO: delete return value? (because of track's revision)
	bool erase(TracksGroup::Ptr _group);

	// erase selected part of track
	bool cut(TracksGroup::Ptr _group);

	// IProcessorClient
	// called when the exportTask done.
	//virtual void requestDone( size_t _size );

	bool processDSP(TrackAudio::Ptr _track);


private:
	void _start_threads(unsigned _num);
	void _stop_threads();
	//void _taskExport( const ProcessorTaskInfo &_task );
	//bool _ensureSignalSize( size_t _size );

	// Thread system data.
	Controller *m_ctrl;
	std::vector < ThreadInfo > m_threads_info;
	boost::thread_group m_Threads;
	bool m_stop_threads;
	unsigned m_threads_running;

	// thread N get Request and perform it via m_tasks_fft[N]
	// or via tasks of other types.
	RingBuff<RequestData> m_requests_data;
	RingBuff<RequestFFT> m_requests_fft;
	RingBuff<RequestWaveform> m_requests_waveform;
	RingBuff<RequestExport> m_requests_export;


	// 1 task per thread
	std::vector<TaskData> m_tasks_data;
	std::vector<TaskFFT> m_tasks_fft;
	std::vector<TaskWaveForm> m_tasks_waveform;
	std::vector<TaskExport> m_tasks_export;


	boost::mutex m_mutex_taskinfo;
	boost::condition_variable m_cond_taskinfo;


	//boost::shared_ptr<DSP> m_dsp;

//	// Used by _taskLoadData() to mix samples.
//	std::vector<char> m_buff_samples;
//	std::vector<char> m_buff_samples_2;

	boost::shared_ptr<AudioBlock> m_ab_copypaste;
};

} // ns

#endif
