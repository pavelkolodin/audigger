
/**
 *  pavelkolodin, 2012-08-08
 *
 */

#include "DSP.h"
#include "audio_common.h"
#include "fft.h"
#include <fir/logger/logger.h>
#include <fir/math/math.h>

DSP::DSP()
: m_mode(DSP::MODE_BANDPASS)
, m_fft_win(0)
, m_fft_step(0)
, m_band_start(0)
, m_band_len(0)
, m_channels(0)
, m_initdone_16FFT(false)
, m_strv(0)
, m_prestep(0)
//, m_bufin(NULL)
//, m_bufin_size(0)
, m_buf1(0)
, m_buf2(0)
{

}

DSP::~DSP()
{

}


/// Set FFT params. Used in signal processing.
void DSP::setFFTParams(unsigned _win, unsigned _step)
{

	if ( (_win == m_fft_win) && (_step == m_fft_step) )
		return;

	m_initdone_16FFT = false;
	// TODO: check
	m_fft_win = _win;
	m_fft_step = _step;
}

/// Set mode.
void DSP::setMode(Mode _mode)
{
	if ( _mode == m_mode )
		return;

	m_initdone_16FFT = false;
	m_mode = _mode;
}

void DSP::setParam1( double _param1 )
{
	m_param1 = _param1;
}

/// Set band.
void DSP::setBand(unsigned _start_fft_bin, unsigned _len)
{
	// TODO: check.
	//m_initdone_16FFT = false;
	m_band_start = _start_fft_bin;
	m_band_len = _len;
	if ( m_band_start & 0x01 ) -- m_band_start;
	if ( m_band_len & 0x01 ) -- m_band_len;
}

void DSP::setChannels(unsigned _ch)
{
	if ( _ch == m_channels )
		return;

	m_initdone_16FFT = false;
	m_channels = _ch;
}

void DSP::Restart()
{
	m_initdone_16FFT = false;
}

/// The main function that performs all the processing
size_t DSP::Process16(const TypeSample16* _src, TypeSample16* _dst, size_t _frames, size_t _limitout)
{
	LOG_PURE( "DSP::Process()");

	if ( 	MODE_BANDPASS == m_mode		/// The band will be passed, all other frequencies will be reduced.
			|| MODE_BANDPASS_ZERO == m_mode
			|| MODE_BANDSUPPRESS == m_mode
			|| MODE_F1 == m_mode
			|| MODE_F2 == m_mode
			|| MODE_F3 == m_mode
			|| MODE_F4 == m_mode )
		return _process16FFT_Loop( _src, _dst, _frames, _limitout );

	LOG_PURE( "DSP::Process(), Wrong mode");
	return 0;
}


size_t DSP::Flush16(TypeSample16* _dst)
{
	size_t ret = 0;
	if ( ! (m_initdone_16FFT) )
		return ret;

	// !!!

	if ( m_prestep < 2 )
	{
		ret += Process16( NULL, _dst, m_fft_step );
		_dst += m_fft_step * m_channels;
	}


	if (m_strv < m_fft_step)
	{
		Process16( NULL, _dst, m_strv, m_fft_step - m_strv );
		ret += m_fft_step - m_strv;
	}

	this->Restart();
	return ret;
}


//
// PRIVATE
//

void DSP::_init16FFT()
{
	if ( ! m_initdone_16FFT )
	{
		m_bufin.resize( m_channels * m_fft_win, 0 );
		m_bufwork.resize( m_channels * m_fft_win, 0 );
		m_buf1.resize( m_channels * m_fft_win, 0 );
		m_buf2.resize( m_channels * m_fft_win, 0 );
		m_complex.resize( m_channels * m_fft_win, 0 );

		m_reverb.resize( m_channels * m_fft_win, 0 );

		m_face = &m_buf1[0];
		m_shadow = &m_buf2[0];

		m_initdone_16FFT = true;

		// Starvation. How many frames we need to put into "m_bufin" before we can start processing.
		// Frames comes to "m_bufin" in FIFO manner - new frames pushing at the end.
		m_strv = m_fft_step;
		m_prestep = (m_fft_win / m_fft_step);
	}
}

size_t DSP::_process16FFT_Loop(const TypeSample16* _src, TypeSample16* _dst, size_t _frames, size_t _limitout)
{

	// Make steps through "_src" buffer by m_fft_step
	// Do hann -> FFT -> process -> IFFT -> sum with previous m_fft_win samples.
	size_t ret = 0;
	if ( ! m_fft_win || ! m_fft_step || ! m_band_len || ! m_channels )
	{
		LOG_ERROR ( "Bad input parameters" );
		return 0;
	}

	if ( ! m_initdone_16FFT )
	{
		_init16FFT();
	}

	if ( ! _limitout || _limitout > m_fft_step)
		_limitout = m_fft_step;

	while ( 1 )
	{
		size_t todo = std::min<size_t> (_frames, m_strv );

		if ( ! todo )
			return ret;

		// FIFO
		// Shift
		memmove(&(m_bufin[0]), &(m_bufin[m_channels*todo]), sizeof(TypeSample16) * m_channels * (m_fft_win - todo) );

		// FIFO
		// Put new data
		if ( _src )
			memcpy(&(m_bufin[m_channels*(m_fft_win - todo)]), _src, sizeof(TypeSample16) * m_channels * todo );
		else
			memset(&(m_bufin[m_channels*(m_fft_win - todo)]), 0, sizeof(TypeSample16) * m_channels * todo );

		_frames -= todo;
		m_strv -= todo;
		if ( _src )
			_src += m_channels * todo;

		if ( m_strv )
		{
			// Ok, we will continue next time.
			return ret;
		}

		_process16FFT_Core();

		sumSamples(m_face, m_shadow + m_channels*m_fft_step, (m_fft_win - m_fft_step), m_channels );
		if ( m_prestep )
			-- m_prestep;

		if ( ! m_prestep )
		{
			memcpy( _dst, m_face, sizeof(TypeSample16) * m_channels * _limitout );
			_dst += m_channels * _limitout;
			ret += _limitout;
		}

		std::swap( m_face, m_shadow );
		m_strv = m_fft_step;
	}

	return ret;
}


void DSP::_process16FFT_Core()
{
	// Process
	memcpy(&(m_bufwork[0]), &(m_bufin[0]), sizeof(TypeSample16)*m_channels*m_fft_win);

	hann(&m_bufwork[0], &m_bufwork[0], m_fft_win, m_channels );

	// Convert LRLRLRLRLR to LLLLLRRRRR
	for ( unsigned i = 0; i < m_fft_win; ++i )
		m_complex[i] = m_bufwork[m_channels * i];

	if (2 == m_channels)
		for ( unsigned i = 0; i < m_fft_win; ++i )
			m_complex[m_fft_win + i] = m_bufwork[m_channels * i + 1];

	// Channel 1
	CFFT::Forward(&m_complex[0], m_fft_win);
	// Channel 2
	if (2 == m_channels)
		CFFT::Forward(&m_complex[m_fft_win], m_fft_win);


	switch (m_mode)
	{
	default:
		break;
	case MODE_BANDPASS:
		spectBandPass( &m_complex[0], m_fft_win, m_band_start, m_band_len);//, 0.001 );
		if (2 == m_channels)
		{
			spectBandPass( &m_complex[m_fft_win], m_fft_win, m_band_start, m_band_len);//, 0.001 );
		}
		break;

	case MODE_BANDPASS_ZERO:
		spectShift( &m_complex[0], m_fft_win, m_band_start);
		spectBandPass( &m_complex[0], m_fft_win, 1, m_band_len);//, 0.001 );
		if (2 == m_channels)
		{
			spectShift( &m_complex[m_fft_win], m_fft_win, m_band_start);
			spectBandPass( &m_complex[m_fft_win], m_fft_win, 1, m_band_len); //, 0.001 );
		}
		break;

	case MODE_BANDSUPPRESS:
		spectBandSuppress( &m_complex[0], m_fft_win, m_band_start, m_band_len );
		if (2 == m_channels)
		{
			spectBandSuppress( &m_complex[m_fft_win], m_fft_win, m_band_start, m_band_len );
		}
		break;
	case MODE_F1:
		spectTestF1( &m_complex[0], m_fft_win, m_band_start, m_band_len, m_param1 );
		if (2 == m_channels)
		{
			spectTestF1( &m_complex[m_fft_win], m_fft_win, m_band_start, m_band_len, m_param1 );
		}
		break;
	case MODE_F2:

		for ( int i = 0; i < 1; ++i )
		{
			spectTestF2( &m_complex[0], m_fft_win, m_band_start, m_band_len, m_param1 );
			if (2 == m_channels)
			{
				spectTestF2( &m_complex[m_fft_win], m_fft_win, m_band_start, m_band_len, m_param1 );
			}
		}
		break;
	case MODE_F3:
		spectTestReverb( &m_complex[0], &m_reverb[0], m_fft_win, m_band_start, m_band_len, m_param1 );
		if (2 == m_channels)
		{
			spectTestReverb( &m_complex[m_fft_win], &m_reverb[m_fft_win], m_fft_win, m_band_start, m_band_len, m_param1 );
		}
		break;
	}

	// Back FFT.
	CFFT::Inverse(&m_complex[0], m_fft_win);
	if (2 == m_channels)
		CFFT::Inverse(&m_complex[m_fft_win], m_fft_win);

	spectToFrames(m_face, &m_complex[0], m_fft_win, m_channels);
}
