/*
 *  pavelkolodin@gmail.com
 */

#ifndef _audio_common_H_
#define _audio_common_H_

#include <vector>
#include <stddef.h>
#include <stdint.h>
#ifndef _USE_MATH_DEFINES
	#define _USE_MATH_DEFINES 1
#endif
#include <math.h>
#include "complex.h"
#include "AudioParams.h"

void hannWeight(short *p, size_t _size, unsigned _channels, float _weight);

template <typename TDST, typename TSRC>
void hann( TDST *_dst, TSRC *_src, size_t _size, unsigned _channels )
{
	for ( size_t i = 0; i < _size; ++i )
	{
		float w = 0.5 * (1.0 - cos( (M_PI*2.0*((float)i)) / (float)(_size - 1) ));

		for (unsigned ch = 0; ch < _channels; ++ch)
		{
			_dst[i*_channels + ch] = (TDST)( ((float)_src[i*_channels + ch]) * w );
		}
	}
}

template<typename TDST, typename TSRC>
void hann_LRLR_LLRR( TDST *_dst, TSRC *_src, size_t _size, unsigned _channels)
{
	for ( size_t i = 0; i < _size; ++i )
	{
		float w = 0.5 * (1.0 - cos( (M_PI*2.0*((float)i)) / (float)(_size - 1) ));

		for (unsigned ch = 0; ch < _channels; ++ch)
		{
			// (C1,C2,C3),(C1,C2,C3) -> (CH1,CH1),(C2,C2),(C3,C3)
			_dst[(ch * _size) + i] = (TDST)( ((float)_src[i*_channels + ch]) * w );
		}
	}
}



// Left part of the hann window
// p - data
// _size - size of half of window.
void hannLeft(short *p, size_t _size, unsigned _channels);

// Right part of the hann window
void hannRight(short *p, size_t _size, unsigned _channels);

void spectIHann(complex *_p, size_t _size);

// _p - spectrum
void spectAddValueHann(complex *_p, size_t _size, double _value);

// _p - spectrum
void spectAddValue(complex *_p, size_t _size, unsigned _freq, double _value);

void spectAddArray(complex *_p, size_t _win, float *_ftr);

void spectToFrames(short *_frames_dst, complex *_spect_src, size_t _win, unsigned _channels);

void spectShift(complex *_p, size_t _win, size_t _shift);

void spectBandSuppress(complex *_p, size_t _win, size_t _start, size_t _len, float _suppress = 0);
void spectBandPass(complex *_p, size_t _win, size_t _start, size_t _len, float _suppress = 0);
void spectFun(complex *_p, size_t _win, size_t _start, size_t _len);

void spectTestF1(complex *_p, size_t _win, size_t _start, size_t _len, float _coeff);
void spectTestF2(complex *_p, size_t _win, size_t _start, size_t _len, float _coeff);
void spectTestF3(complex *_p, size_t _win, size_t _start, size_t _len);
void spectTestF4(complex *_p, size_t _win, size_t _start, size_t _len);

void spectTestReverb(complex *_p, complex *_reverb, size_t _win, size_t _start, size_t _len, float _coeff);


void sumSamples(short *_dst, short *_src, size_t _len, unsigned _channels);
//bool goodRate(unsigned _rate);
//bool goodBits( unsigned _bits );
bool goodFFTWin(size_t _win);

template < typename T > inline T limitShort( T _val )
{
	if ( _val > 32767 )
		return 32767;
	if ( _val < -32767 )
		return -32767;

	return _val;
}

void sumFrames( char *_dst, const char *_src, size_t _size, const AudioParams& _params);

template < typename TypeDst, typename TypeSrc >
size_t transferFrames( TypeDst *_dst, TypeSrc *_src, size_t _frames )
{
	if ( ! _dst->getAudioParams().compartible( _src->getAudioParams() ) )
		return 0;

	static const size_t frames_buff = 1024 * 128;
	size_t bytes_buff = _src->getAudioParams().frameSize() * frames_buff;
	std::vector< char > buff;
	buff.resize( bytes_buff );

	size_t written_frames = 0;
	do
	{
		size_t todo = std::min< size_t > ( frames_buff, _frames );
		_src->read( &buff[0], todo );
		size_t wf = _dst->write( &buff[0], todo );
		written_frames += wf;
		if ( wf != todo)
			break;

		_frames -= todo;
	} while( _frames );

	return written_frames;
}

#endif

