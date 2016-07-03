/*
 * pavelkolodin@gmail.com
 */

#include "ProcessorTasks.h"
#include "fft.h"
#include "DSP.h"
#include "audio_common.h"
#include <fir/logger/logger.h>

namespace Processor
{

void TaskFFT::start()
{
	LOGVAR2( initialized(), m_callback );

	if ( ! initialized() )
		return;

	AutoTaskFinisher atf(this, m_callback);

	if ( ! m_callback->taskStillValid( m_id ) )
	{
		LOG_ERROR ( "! m_callback->taskStillValid( m_id )" );
		return;
	}

	readSignal__();

	LOG_PURE ( "SignalProcessor::loadFFT(), "
				<< std::dec
				<< "tid " << m_id << ", "
				<< "off " << m_offset << ", sz " << m_size << ", "
				<< "stp " << m_step << ", "
				<< "win " << m_fft_win << ", "
				<< "lns " << m_fft_data.m_lines );


	size_t &lines = m_fft_data.m_lines;
	m_size_result = lines;

	if ( ! m_track->getAudioParams().isS16LE() )
	{
		LOG_ERROR ( "LoadFFT: S16LE only" );
		return;
	}

	unsigned channels = m_track->getAudioParams().m_channels;
	//TaskDataFFT taskdatafft; // structure for results
	for ( size_t line = 0; line < lines; ++line )
	{
		int16_t *signal = (int16_t*)(&m_signal[0] + ( line * m_fft_data.m_step_in_buffer * m_track->getAudioParams().frameSize() ) );
		complex *result = &m_result[0] + line * m_fft_win * channels;

		// Apply Hann window to signal
		// and Convert LRLRLR to LLLRRR
		hann_LRLR_LLRR( result, signal, m_fft_win, channels );

		// Channel 1
		CFFT::Forward(result, m_fft_win);

		// Channel 2
		if ( channels > 1 )
			CFFT::Forward(result + m_fft_win, m_fft_win);
	}


	LOG_PURE( "Processor::_taskLoadFFT() OK" );
}

//void TaskFFT::finish()
//{
//	m_callback->taskDone( *this );
//	m_callback = NULL;
//}


bool TaskFFT::readSignal__()
{
	boost::unique_lock < boost::mutex > lock( m_track->getMutex() );

	m_fft_data.clear();
	unsigned channels = m_track->getAudioParams().m_channels;

	// 2 modes of in-memory data placement:
	// 1) step <= fft_win
	// 2) step > fft_win
	size_t step_in_buffer = ( m_step <= m_fft_win )?m_step:m_fft_win;

	size_t size_buffer_frames = m_fft_win + step_in_buffer * (m_size - 1);
	size_t start_track_frame = m_offset * m_step;
	size_t read_iterations = ( m_step <= m_fft_win )?1:m_size;
	size_t read_portion_frames = ( m_step <= m_fft_win )?size_buffer_frames:m_fft_win;
	size_t lines = 0;

	size_t signal_size = size_buffer_frames * m_track->getAudioParams().frameSize();
	if ( m_signal.size() < signal_size )
		m_signal.resize( signal_size );

	size_t result_size = m_fft_win * channels;
	if ( m_result.size() < result_size )
		m_result.resize( result_size );

	// Load data from track
	size_t frames_read = 0;

	for ( size_t i = 0; i < read_iterations; ++i, start_track_frame += m_step )
	{
		size_t rd = m_track->readFrom(
				&m_signal[0] + (step_in_buffer * i * m_track->getAudioParams().frameSize()),
				read_portion_frames,
				start_track_frame );
		if ( ! rd )
			break;

		frames_read += rd;
		if ( rd <  read_portion_frames )
			break;
	}

	// How much lines we can draw using all read data?
	if ( frames_read >= m_fft_win )
	{
		lines = 1;
		size_t lines_add = (frames_read - m_fft_win) / step_in_buffer;
		lines += lines_add;

		size_t zero_frames = (frames_read - m_fft_win) - lines_add * step_in_buffer;

		if ( zero_frames )
		{
			//LOGVAR3( lines, lines_add, add_frames );
			memset( &m_signal[0] + (frames_read * m_track->getAudioParams().frameSize() ),
					0,
					zero_frames * m_track->getAudioParams().frameSize() );
		}
	}

	m_fft_data.m_lines = lines;
	m_fft_data.m_step_in_buffer = step_in_buffer;
	return true;
}




void TaskData::start()
{
	if ( ! initialized() )
		return;

	AutoTaskFinisher atf(this, m_callback);

	TracksGroup::VectorTracksAudio tracks;
	if ( ! m_request.m_group->getTracksAudio(tracks) )
		return;

	if ( tracks.size() > 1 )
		m_buff_tmp.resize( m_request.m_size * m_request.m_group->getAudioParams().frameSize() );

	m_result_size = 0;
	size_t cnt_mixed = 0;
	for ( size_t i = 0; i < tracks.size(); ++i )
	{
		char *dst = ((0 == cnt_mixed)?m_request.m_buff_dst : &m_buff_tmp[0]);

		boost::unique_lock < boost::mutex > lock( tracks[i]->getMutex() );
		//if ( tracks[i]->getCursorInfo().m_pos >= tracks[i]->getAudioParams().m_frames )
			//continue;

		if ( ! m_request.m_group->getAudioParams().compartible( tracks[i]->getAudioParams() ) )
			continue;

		size_t oldpos = tracks[i]->getCursorInfo().m_pos;
		tracks[i]->seek( tracks[i]->getCursorInfo().m_pos + m_request.m_offset, SEEK_SET );
		size_t rd = tracks[i]->read( dst, m_request.m_size );
		tracks[i]->getCursorInfo().m_pos = oldpos;

		if ( ! rd )
			continue;



		if ( rd > m_result_size )
			m_result_size = rd;

		if ( cnt_mixed > 0 )
		{
			sumFrames(m_request.m_buff_dst, &m_buff_tmp[0], rd, m_request.m_group->getAudioParams() );
		}

		++ cnt_mixed;
	}
}

void TaskData::mixFrames()
{

}



void TaskExport::start()
{
	if ( ! initialized() )
		return;

	AutoTaskFinisher atf(this, m_callback);

//	for ( int i = 1; i < 100; ++i )
//	{
//		usleep( 10000 );
//		m_callback->taskPercent(*this, i);
//	}

	if ( m_filename.empty() || (! m_track ) )
	{
		return;
	}

	boost::unique_lock < boost::mutex > lock( m_track->getMutex() );

	size_t len = m_track->getAudioParams().m_frames;
	size_t start = 0;
	if ( m_len )
	{
		if ( (m_start + m_len) > m_track->getAudioParams().m_frames )
			return;

		len = m_len;
		start = m_start;
	}

	// If > 32MB to do, then split it by 100 parts.
	size_t len_todo = len;
	size_t max_portion = ((len * m_track->getAudioParams().frameSize()) > 1024*1024*32)?
						len/100:
						len;

	// Create file
	AudioFile af;
	af.open( m_filename.c_str(), AudioFile::MODE_WRITE, &m_track->getAudioParams() );

	m_track->seek(start, SEEK_SET);

	size_t todo = 0;
	for ( int percent = 1; len_todo; len_todo -= todo, ++percent)
	{
		todo =  std::min<size_t>(len_todo, max_portion);
		transferFrames<AudioFile, TrackAudio>(&af, m_track.get(), todo );
		m_callback->taskPercent(*this, percent);
	}
}



}


