
#ifndef _AUDIO_PARAMS_H_
#define _AUDIO_PARAMS_H_

#include <stdexcept>
#include <sstream>
#include <fir/logger/logger.h>

typedef unsigned long TimeValueMs;



struct AudioParams
{
	unsigned m_rate;
	unsigned m_channels;
	unsigned m_bits;
	bool m_bigendian;
	bool m_signed;
	bool m_float;
	unsigned long m_frames;

	const static unsigned m_rates_all[];
	const static unsigned m_bits_all[];
	const static unsigned m_channels_all[];

	AudioParams()
	{
		clear();
	}

	AudioParams( unsigned _rate, unsigned _channels, unsigned _bits, bool _bigendian, bool _signed, bool _float, size_t _frames = 0)
	: m_rate(_rate)
	, m_channels(_channels)
	, m_bits(_bits)
	, m_bigendian(_bigendian)
	, m_signed(_signed)
	, m_float(_float)
	, m_frames(_frames)
	{
	}

//	void set (unsigned _rate,  unsigned _channels, unsigned _bits, bool _bigendian, bool _signed, bool _float, size_t _frames = 0 )
//	{
//		m_rate = _rate;
//		m_channels = _channels;
//		m_bits = _bits;
//		m_bigendian = _bigendian;
//		m_signed = _signed;
//		m_float = _float;
//		m_frames = _frames;
//	}

	bool compartible( const AudioParams& _other ) const
	{
		return ( _other.m_rate == m_rate &&
				 _other.m_channels == m_channels &&
				 _other.m_bits == m_bits &&
				 _other.m_bigendian == m_bigendian &&
				 _other.m_signed == m_signed &&
				 _other.m_float == m_float
				  );
	}

	size_t frameSize() const
	{
		return ((m_bits / 8) * m_channels);
	}

	void clear()
	{
		m_rate = 0;
		m_channels = 0;
		m_bits = 0;
		m_bigendian = 0;
		m_signed = 0;
		m_float = 0;
		m_frames = 0;
	}

	void print() const
	{
		LOG_PURE (
		"rate:     " << m_rate << "\n"
		<< "channels: " << m_channels << "\n"
		<< "bits:     " << m_bits << "\n"
		<< "BE:       " << m_bigendian << "\n"
		<< "signed:   " << m_signed << "\n"
		<< "float:    " << m_float << "\n"
		<< "frames:   " << m_frames << "\n"
		<< "ms:       " << audioLenMs( m_rate, m_frames )
		<< "\n" );
	}

	static double audioLenMs( unsigned _rate, size_t _frames )
	{
		return ((double)_frames) / (((double)_rate) / 1000.0);
	}

	bool isS16LE( ) const
	{
		return ( (16 == m_bits) && ( ! m_bigendian) && m_signed && (! m_float) );
	}

	void set44100_2_S16LE()
	{
		m_rate = 44100;
		m_channels = 2;
		m_bits = 16;
		m_bigendian = false;
		m_signed = true;
		m_float = false;

		//m_frames = 0;
	}

	bool goodRate( ) const
	{
		return (
					8000 == m_rate ||
					11025 == m_rate ||
					16000 == m_rate ||
					22050 == m_rate ||
					32000 == m_rate ||
					44100 == m_rate ||
					48000 == m_rate ||
					96000 == m_rate ||
					192000 == m_rate );
	}

	bool goodBits( ) const
	{
		return ( 8 == m_bits || 16 == m_bits );
	}


	void throwExceptionForBadAudioParams() const
	{
		#define SS_BAP std::stringstream sss;   sss << "Bad audio param: ";
		if ( ! goodBits( ) )
		{
			SS_BAP;

			sss << "bits " << m_bits;
			throw std::runtime_error( sss.str() );
		}

		if ( ( (m_channels < 1) || (m_channels > 2) ) )
		{
			SS_BAP;

			sss << "channels " << m_channels;
			throw std::runtime_error( sss.str() );
		}

		if ( ! goodRate() )
		{
			SS_BAP;

			sss << "rate " << m_rate;
			throw std::runtime_error( sss.str() );
		}
	}
};


#endif
