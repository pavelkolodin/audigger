/*
 * pavelkolodin@gmail.com
 */

#ifndef PROCESSORTYPES_H_
#define PROCESSORTYPES_H_


#include "common.h"
#include "TracksGroup.h"
#include "TrackAudio.h"
#include "IProcessorClient.h"

namespace Processor
{

class AutoTaskFinisher
{
public:
	AutoTaskFinisher( TaskAbstract *_t, IProcessorClient *_c )
	: m_taskabstract(_t)
	, m_client(_c)
	{
	}

	virtual ~AutoTaskFinisher()
	{
		if ( ! ( m_client && m_taskabstract ) )
			return;


		m_client->taskDone( *m_taskabstract );
		// ! IMPORTANT
		m_taskabstract->m_callback = 0;
	}

	TaskAbstract *m_taskabstract;
	IProcessorClient *m_client;
};



struct RequestFFT : public RequestAbstract
{
	RequestFFT()
	: RequestAbstract( 0, 0, PTT_FFT )
	{

	}

	RequestFFT(IProcessorClient *_callback, size_t _id, TrackAudio::Ptr _track, unsigned _win, size_t _step, size_t _offset, size_t _size)
	: RequestAbstract( _callback, _id, PTT_FFT )
	, m_track(_track)
	, m_fft_win(_win)
	, m_step(_step)
	, m_offset(_offset)
	, m_size(_size)
	{

	}

	TrackAudio::Ptr m_track;
	unsigned m_fft_win;
	size_t m_step;
	size_t m_offset;
	size_t m_size;
};



struct RequestWaveform : public RequestAbstract
{
	RequestWaveform()
	: RequestAbstract( 0, 0, PTT_WAVEFORM )
	{
	}

	RequestWaveform(IProcessorClient *_callback, unsigned _id, TrackAudio::Ptr _track, int _channel, size_t _step, size_t _offset, size_t _size)
	: RequestAbstract( _callback, _id, PTT_WAVEFORM )
	, m_track(_track)
	, m_channel(_channel)
	, m_step(_step)
	, m_offset(_offset)
	, m_size(_size)
	{

	}


	TrackAudio::Ptr m_track;
	// -1 = all channels
	int m_channel;
	size_t m_step;
	size_t m_offset;
	size_t m_size;
};



struct RequestDSP : public RequestAbstract
{
};



struct RequestData : public RequestAbstract
{
public:
	RequestData()
	: RequestAbstract( 0, 0, PTT_DATA )
	, m_offset(0)
	, m_size(0)
	{
	}

	RequestData(IProcessorClient *_callback, unsigned _id, char *_buff_dst, TracksGroup::Ptr _group, size_t _offset, size_t _size)
	: RequestAbstract( _callback, _id, PTT_DATA )
	, m_buff_dst(_buff_dst)
	, m_group(_group)
	, m_offset(_offset)
	, m_size(_size)
	{
	}

	char *m_buff_dst;
	TracksGroup::Ptr m_group;
	size_t m_offset;
	size_t m_size;
};



class RequestExport : public RequestAbstract
{
public:
	RequestExport()
	: RequestAbstract( 0, 0, PTT_EXPORT )
	, m_start(0)
	, m_len(0)
	{
	}

	RequestExport(IProcessorClient *_callback, unsigned _task_id, const std::string &_filename, TracksGroup::Ptr _group, size_t _start = 0, size_t _size = 0)
	: RequestAbstract( _callback, _task_id, PTT_EXPORT )
	, m_filename( _filename )
	, m_group(_group)
	, m_start(_start)
	, m_len(_size)
	{
	}

	RequestExport(IProcessorClient *_callback, unsigned _task_id, const std::string &_filename, TrackAudio::Ptr _track, size_t _start = 0, size_t _size = 0)
	: RequestAbstract( _callback, _task_id, PTT_EXPORT )
	, m_filename( _filename )
	, m_track(_track)
	, m_start(_start)
	, m_len(_size)
	{
	}


	std::string m_filename;
	TracksGroup::Ptr m_group;
	TrackAudio::Ptr m_track;
	size_t m_start;
	size_t m_len;
};



//
// TASKS
//


class TaskFFT : public TaskAbstract
{
public:
	struct FFTData
	{
		size_t m_lines;
		size_t m_step_in_buffer;
		void clear()
		{
			m_lines = 0;
		}
	};


	TaskFFT()

	: m_fft_win(0)
	, m_step(0)
	, m_offset(0)
	, m_size(0)
	, m_size_result(0)
	{
	}

	virtual ~TaskFFT()
	{
	}

	void init(const RequestFFT& _r)
	{
		TaskAbstract::init( _r );
		m_track = _r.m_track;
		m_fft_win = _r.m_fft_win;
		m_step = _r.m_step;
		m_offset = _r.m_offset;
		m_size = _r.m_size;

		unsigned channels = m_track->getAudioParams().m_channels;
		size_t s = m_fft_win * channels * m_size;

		if ( m_result.size() < s )
			m_result.resize( s );
	}

	virtual void start();

	const complex* result() const
	{
		return &m_result[0];
	}

	size_t sizeResult() const
	{
		return m_size_result;
	}

	TrackAudio::Ptr track() const
	{
		return m_track;
	}

	size_t win() const
	{
		return m_fft_win;
	}

private:
	bool readSignal__();


	// track to load data from
	TrackAudio::Ptr m_track;
	unsigned m_fft_win;
	size_t m_step;
	// offset from track's begining
	size_t m_offset;
	// number of steps to go
	size_t m_size;
	size_t m_size_result;

	std::vector<complex> m_result;
	std::vector<char> m_signal;
	FFTData m_fft_data;
};


class TaskWaveForm : public TaskAbstract
{
public:
	typedef int16_t MinMaxType;

	TaskWaveForm()

	: m_result_size(0)
	, m_channel(0)
	, m_offset(0)
	, m_step(0)
	, m_size(0)
	, m_min(0)
	, m_max(0)
	{

	}

	void init(const RequestWaveform& _r)
	{
		TaskAbstract::init( _r );
		m_track = _r.m_track;
		m_channel = _r.m_channel;
		m_offset = _r.m_offset;
		m_step = _r.m_step;
		m_size = _r.m_size;
	}

	//virtual ~TaskWaveForm();
	virtual void start()
	{
		if ( ! initialized() )
			return;

		AutoTaskFinisher atf(this, m_callback);

		if ( m_track->getAudioParams().isS16LE() )
			calcWaveForm__<int16_t>();
	}

	void copyResult( std::vector< MinMaxType > &_dst ) const
	{
		if (_dst.size() < m_result_size * 2)
			_dst.resize( m_result_size * 2 );

		memcpy( &_dst[0], &m_result[0], m_result_size * sizeof(MinMaxType) * 2 );
	}

	const MinMaxType* result() const
	{
		return &m_result[0];
	}

	size_t resultSize() const
	{
		return m_result_size;
	}

private:
	template < typename TSAMPLE > void calcWaveForm__();
	std::vector< char > m_src;
	std::vector< MinMaxType > m_result;
	size_t m_result_size;

	TrackAudio::Ptr m_track;
	// -1 = all channels
	int m_channel;
	size_t m_offset;
	size_t m_step;
	size_t m_size;

public:
	MinMaxType m_min;
	MinMaxType m_max;
};


template < typename TSAMPLE > void TaskWaveForm::calcWaveForm__()
{
	m_min = traitLimits<TSAMPLE>::MIN;
	m_max = traitLimits<TSAMPLE>::MAX;

	//
	// initialize
	//

	size_t src_buff_size = m_step * m_track->getAudioParams().frameSize();
	if ( m_src.size() < src_buff_size )
		m_src.resize( src_buff_size );

	// *2, because each line = MIN + MAX
	if( m_result.size() < m_size*2 )
		m_result.resize( m_size*2 );


	TSAMPLE *s = (TSAMPLE*)&m_src[0];
	MinMaxType *d = &m_result[0];
	const AudioParams &_ap = m_track->getAudioParams();

	unsigned channel_start = (-1 == m_channel )?0:m_channel;
	unsigned channel_num = (-1 == m_channel )?_ap.m_channels:1;

	m_result_size = 1;
	for ( 	size_t line = 0;
			line < m_size;
			++line, d += 2, ++m_result_size )
	{

		// Read data
		size_t frames_read = 0;
		{
			boost::unique_lock < boost::mutex > lock( m_track->getMutex() );
			frames_read = m_track->readFrom(&m_src[0], m_step, m_offset*m_step + line*m_step );
//			LOG_PURE("read: ");
//			LOGVAR3( m_step, m_offset, frames_read );
		}


		// *** min = MAX, max = MIN ***
		d[0] = traitLimits<TSAMPLE>::MAX;
		d[1] = traitLimits<TSAMPLE>::MIN;

		for ( size_t i = 0; i < frames_read /*m_step*/; ++i )
		{
			MinMaxType value = 0;
			for (unsigned ch = channel_start; ch < channel_start + channel_num; ++ch )
			{
				value += s[ i * _ap.m_channels + ch ]/channel_num;
			}


			if ( d[0] > value )
				d[0] = value;
			if ( d[1] < value )
				d[1] = value;
		}

		if ( frames_read < m_step )
			break;
	}


	if ( m_size < 2 )
		return;

	//
	// post-processing: Remove all gaps.
	//

	MinMaxType *first = &m_result[0];
	MinMaxType *second = &m_result[0] + 2; // - next minmax pair

	for ( size_t i = 0; i < (m_size-1); ++i )
	{
		if ( first[0] > second[1] )
		{
			MinMaxType diff_half = (first[0] - second[1])/2;
			first[0] -= diff_half;
			second[1] += diff_half;
		}
		else
		if ( first[1] < second[0] )
		{
			MinMaxType diff_half = (second[0] - first[1])/2;
			first[1] += diff_half;
			second[0] -= diff_half;
		}

		first += 2;
		second += 2;
	}
}



class TaskData : public TaskAbstract
{
public:
	TaskData()	{ }

	void init(const RequestData& _r)
	{
		TaskAbstract::init( _r );

		m_request = _r;
	}

	virtual void start();
	size_t resultSize() const
	{
		return m_result_size;
	}

private:
	void mixFrames();

	RequestData m_request;
	std::vector <char> m_buff_tmp;
	//std::vector <char> m_buff_mix;
	size_t m_result_size;
};


class TaskExport : public TaskAbstract
{
public:
	TaskExport() { }
	void init(const RequestExport& _r)
	{
		TaskAbstract::init( _r );
		m_track = _r.m_track;
		m_group = _r.m_group;
		m_filename = _r.m_filename;
		m_start = _r.m_start;
		m_len = _r.m_len;
	}

	virtual void start();

protected:
	TrackAudio::Ptr m_track;
	TracksGroup::Ptr m_group;
	std::string m_filename;
	size_t m_start;
	size_t m_len;
};


}



#endif /* PROCESSORTYPES_H_ */
