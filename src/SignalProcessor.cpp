/*
 *  pavelkolodin@gmail.com
 */

#include "SignalProcessor.h"
#include "Controller.h"
#include <boost/foreach.hpp>

SignalProcessor :: SignalProcessor ( Controller *_ctrl )
: m_ctrl ( _ctrl )
{
	if ( ! _ctrl )
		throw std::runtime_error( "SignalProcessor::SignalProcessor(): Controller == NULL" );
}


SignalProcessor::~SignalProcessor ( )
{

}

void SignalProcessor::loadFFT( const ProcessorTaskInfo& _task )
{
	LOG_PURE ( "SignalProcessor::loadFFT(), "
				<< std::dec
				<< "tid " << _task.m_task_id << ", "
				<< "st " << _task.m_load_from << ", sz " << _task.m_load_size << ", "
				<< "step " << _task.m_step << ", "
				<< "win " << _task.m_fft_win << "\n" );

	readTrackForFFT__( _task );

	size_t &lines = m_fft_data.m_lines;

	if ( ! lines )
	{
		_task.m_callback->taskDone( _task.m_task_id, 0 );
		return;
	}

	if ( ! m_fft_data.m_audio_params.isS16LE() )
	{
		LOGERROR ( "LoadFFT: S16LE only" );
		_task.m_callback->taskDone( _task.m_task_id, 0 );
		return;
	}


	//
	// Calculate FFT
	//

	// User want us to load "m_load_size" lines from line "m_load_from".
	// 1 line visualize fft window
	// The step between fft windows is _task.m_step frames.

	TaskDataFFT taskdatafft; // structure for results
	bool drawok = false;

	for ( size_t line = 0; line < lines; ++line )
	{
		int16_t *signal = (int16_t*)(m_audio.ptr() + ( line * m_fft_data.m_step_in_buffer * m_fft_data.m_audio_params.frameSize() ) );

		// Apply Hann window to signal
		// and Convert LRLRLRLRLR to LLLLLRRRRR
		hann_LRLR_LLRR( m_complex.ptr(), signal, _task.m_fft_win, m_fft_data.m_audio_params.m_channels );

		// Channel 1
		CFFT::Forward(m_complex.ptr(), _task.m_fft_win);

		// Channel 2
		if ( m_fft_data.m_audio_params.m_channels > 1)
			CFFT::Forward(m_complex.ptr() + _task.m_fft_win, _task.m_fft_win);

		// return 1 line.
		taskdatafft.m_complex = m_complex.ptr();
		taskdatafft.m_channels = m_fft_data.m_audio_params.m_channels;
		taskdatafft.m_size = _task.m_fft_win/2;

		drawok = _task.m_callback->taskData(_task.m_task_id, taskdatafft );

		if ( ! drawok )
		{
			break;
		}
	}

	if ( drawok )
		_task.m_callback->taskDone( _task.m_task_id, 0 );
	else
	{
		LOG_ERROR( "! drawok" );
	}

	LOG_PURE( "Processor::_taskLoadFFT() OK" );
	//LOGLOGGERPKL( "\n~o" << _task.m_task_id << "\n" )
}

void SignalProcessor::loadWaveForm( const ProcessorTaskInfo& _task )
{
	LOG_PURE( "SignalProcessor::loadWaveForm" );
	TrackAudio::Ptr track = _task.m_track_audio;
	boost::unique_lock < boost::mutex > lock( _task.m_track_audio->getMutex() );

	TaskDataWaveForm taskdata;
	AudioParams ap = track->getAudioParams();

	if ( _task.m_channel >= track->getAudioParams().m_channels )
	{
		_task.m_callback->taskDone( _task.m_task_id, 0 );
		return;
	}

	size_t frames_load = _task.m_load_size * _task.m_step;
	size_t rd = 0;

	if ( ! m_audio.ensure_size( ap.frameSize() * frames_load ))
	{
		_task.m_callback->taskDone( _task.m_task_id, 0 );
		return;
	}

	rd = track->readFrom( m_audio.ptr(), frames_load, _task.m_load_from );

	if ( rd != frames_load )
	{
		LOGERROR( "Read only " << rd << "/" << frames_load << " frames from track " );
		_task.m_callback->taskDone( _task.m_task_id, 0 );
		return;
	}

	// *2, because each pixel = (min, max)

	if ( ! m_waveform.ensure_size( _task.m_load_size * 2 ) )
	{
		_task.m_callback->taskDone( _task.m_task_id, 0 );
		return;
	}

	if ( ap.isS16LE() )
	{
		calcWaveForm<int16_t> ( m_audio.ptr(), ap, _task.m_channel, m_waveform.ptr(), _task.m_step, _task.m_load_size );

		taskdata.m_minmax = m_waveform.ptr();
		taskdata.m_size = _task.m_load_size;
		taskdata.m_limits.min = traitLimits<int16_t>::MIN;
		taskdata.m_limits.max = traitLimits<int16_t>::MAX;

		_task.m_callback -> taskData( _task.m_task_id, taskdata );
		_task.m_callback -> taskDone( _task.m_task_id, _task.m_load_size );
	}
}

void SignalProcessor::loadData( const ProcessorTaskInfo& _task )
{
	if ( ! _task.m_callback )
		return;

	TrackAudio::Ptr track = _task.m_track_audio;
	boost::unique_lock < boost::mutex > lock( _task.m_track_audio->getMutex() );

	LOG_PURE( " Processor::_taskLoadData(), fr "
				<< _task.m_load_from << ", sz " << _task.m_load_size );

	//LOGVAR4( (void*)_task.m_callback , _task.m_track_id, (void*)_task.m_buff, _task.m_load_size );

	if ( ! _task.m_buff || ! _task.m_load_size )
	{
		LOGERROR("Bad parameters");
		_task.m_callback->taskDone(_task.m_task_id, 0 );

		return;
	}

	size_t read_frames = track->readFrom( _task.m_buff, _task.m_load_size, _task.m_load_from );
	_task.m_callback->taskDone(_task.m_task_id, read_frames );

	LOG_PURE( "Processor::_taskLoadData() OK" );
}


bool SignalProcessor::readTrackForFFT__(const ProcessorTaskInfo &_task )
{
	TrackAudio::Ptr track = _task.m_track_audio;
	boost::unique_lock < boost::mutex > lock( _task.m_track_audio->getMutex() );

	m_fft_data.clear();

	if ( (! _task.m_load_size) || (! _task.m_fft_win) )
	{
		LOGERROR ( "Bad input parameters" );
		return false;
	}

	//LOG_PURE( _task.m_track_id << ", " << (void*) m_tracks_audio[ _task.m_track_id ] )
	unsigned channels = track->getAudioParams().m_channels;

	// 2 modes of in-memory data placement:
	// 1) step <= fft_win
	// 2) step > fft_win
	size_t step_in_buffer = ( _task.m_step <= _task.m_fft_win )?_task.m_step:_task.m_fft_win;

	size_t size_buffer_frames = _task.m_fft_win + step_in_buffer * (_task.m_load_size - 1);
	size_t start_track_frame = _task.m_load_from * _task.m_step;
	size_t read_iterations = ( _task.m_step <= _task.m_fft_win )?1:_task.m_load_size;
	size_t read_portion_frames = ( _task.m_step <= _task.m_fft_win )?size_buffer_frames:_task.m_fft_win;
	size_t lines = 0;

	if ( ! m_audio.ensure_size( size_buffer_frames * track->getAudioParams().frameSize() ) )
		return false;

	if ( ! m_complex.ensure_size( _task.m_fft_win * channels ) )
		return false;

	// Load data from track
	size_t frames_read = 0;

	for ( size_t i = 0; i < read_iterations; ++i, start_track_frame += _task.m_step )
	{
		size_t rd = track->readFrom(
				m_audio.ptr() + (step_in_buffer * i * track->getAudioParams().frameSize()),
				read_portion_frames,
				start_track_frame );
		if ( ! rd )
			break;

		frames_read += rd;
		if ( rd <  read_portion_frames )
			break;
	}

	// How much lines we can draw using all read data?
	if ( frames_read >= _task.m_fft_win )
	{
		lines = 1;
		size_t lines_add = (frames_read - _task.m_fft_win) / step_in_buffer;
		lines += lines_add;

		size_t zero_frames = (frames_read - _task.m_fft_win) - lines_add * step_in_buffer;

		if ( zero_frames )
		{
		//LOGVAR3( lines, lines_add, add_frames );
		memset( m_audio.ptr() + (frames_read * track->getAudioParams().frameSize() ),
				0,
				zero_frames * track->getAudioParams().frameSize() );
		}
	}

	m_fft_data.m_lines = lines;
	m_fft_data.m_step_in_buffer = step_in_buffer;
	m_fft_data.m_audio_params = track->getAudioParams();
	return true;
}


