/*
 *  pavelkolodin@gmail.com
 */

#include <algorithm>
#include "audio_common.h"
#include "fir/math/math.h"
#include <fir/logger/logger.h>

void hannWeight(short *p, size_t _size, unsigned _channels, float _weight)
{
	if ( _size < 1 || _channels < 1 || _channels > 2 || 0 == _weight )
		return;
	//double alpha = 0.1;
	//double beta = 0.8;
	//alpha = constant of window
	//beta = constant of window. Generally, beta = (1 - alpha)
	for ( unsigned i = 0; i < _size; ++i)
	{
		//w[i] = alpha + beta*cos((M_PI*2*i)/nf);
		double w = _weight * (1.0 - cos( (M_PI*2*((float)i)) / (float)(_size - 1) ));

		p[i*_channels] *= w;
		if ( 2 == _channels )
			p[i*_channels + 1] *= w;
	}
}

// Left part of the hann window
// p - data
// _size - size of half of window.
void hannLeft(short *p, size_t _size, unsigned _channels)
{
	if ( _size < 1 || _channels < 1 || _channels > 2 )
		return;
	for ( unsigned i = 0; i < _size; ++i)
	{
		double w = 0.5 * (1.0 - cos( (M_PI*2*((float)i)) / (float)(_size*2 - 1) ));

		p[i*_channels] *= w;
		if ( 2 == _channels )
			p[i*_channels + 1] *= w;
	}
}

// Right part of the hann window
void hannRight(short *p, size_t _size, unsigned _channels)
{
	if ( _size < 1 || _channels < 1 || _channels > 2 )
		return;
	for ( unsigned i = 0; i < _size; ++i)
	{
		double w = 0.5 * (1.0 - cos( (M_PI*2*((float)i + _size)) / (float)(_size*2 - 1) ));

		p[i*_channels] *= w;
		if ( 2 == _channels )
			p[i*_channels + 1] *= w;
	}
}

void spectIHann(complex *_p, size_t _size)
{
	if ( _size < 1)
		return;
	complex *x;
	for ( unsigned i = 0; i < _size; ++i)
	{
		//double w = 0.5 * (1 + cos( (M_PI*2*(double)i) / (double)(_size - 1) ));
		double w = 0.001;
		x = &_p[ i ];
		*x = complex(x->re() * w, x->im() * w);

//		// Use this code simplification in "hann()" function (look before^^^).
//		if ( 2 == _channels)
//		{
//			++x;
//			*x = complex(x->re() * w, x->im()*w);
//		}
	}
}

// _p - spectrum
void spectAddValueHann(complex *_p, size_t _size, double _value)
{
	if ( _size < 1)
		return;

	complex *x;
	for ( unsigned i = 0; i < _size; ++i)
	{
		double w = 0.5 * (1 - cos( (M_PI*2*(double)i) / (double)(_size - 1) ));
		x = &_p[ i ];
		*x = complex(x->re() + _value * w, x->im());
		//double coeff = x->im()/x->re();
		//*x = complex(x->re() + w*_value, x->im() + coeff*w*_value);

//		// Use this code simplification in "hann()" function (look before^^^).
//		if ( 2 == _channels)
//		{
//			++x;
//			*x = complex(x->re() * w, x->im()*w);
//		}
	}
}

// _p - spectrum
void spectAddValue(complex *_p, size_t _size, unsigned _freq, double _value)
{
	if ( _freq > _size/2 || 0 == _freq )
		return;

	double arg, w;
	complex *x;
	// - _freq
	for ( size_t i = 1; i < _size/2; ++i)
	{
		arg = 10*_freq * (i - _freq); //(M_PI*2*(double)(_size/2)) * (double)((i - _size/4) - _freq)) ;
		if ( 0 == arg )
			w = 1;
		else
			w = sin( arg ) / arg;


		x = &_p[ i ];
		*x = complex(x->re() + _value * w, x->im());
		x = &_p[ (_size-1) - i ];
		*x = complex(x->re() + _value * w, x->im());

	}
}

void spectAddArray( complex *_p, size_t _win, float *_ftr )
{
	complex *x;
	for ( size_t i = 1; i < _win/2; ++i)
	{
		x = &_p[i];
		*x = complex( _ftr[i], 0);// - _ftr[i]*10 );
		x = &_p[ (_win-1) - (i-1) ];
		*x = complex( _ftr[i], 0);// + _ftr[i]*10 );
	}
}

void spectToFrames( short *_frames_dst, complex *_spect_src, size_t _win, unsigned _channels )
{
	// Convert LLRR to LRLR:

	for ( unsigned ch = 0; ch < _channels; ++ch )
		for (size_t i = 0; i < _win; ++i )
			_frames_dst[i * _channels + ch] = limitShort( fir::math::intRound<double>( _spect_src[_win*ch + i].re() ) );

//	if (2 == _channels)
//		for (unsigned i = 0; i < _win; ++i )
//			//win[channels*i + 1] = win_x[_res + i].re();
//			_frames[i * _channels + 1] = fir::intRound<double>( win_x[_res + i].re() );
}

void spectShift( complex *_p, size_t _win, size_t _shift )
{
	if ( _shift <= 0 )
		return;

	if ( _shift > (_win/2 - 1))
		return;

	//complex *x;
	for ( size_t i = 0; i < (_win/2) - _shift; ++i )
		_p[i] = _p[i + _shift];

	for ( int i = (_win/2) - _shift; i <= (_win/2); ++i )
		_p[i] = 0;


	// Mirror.
	for ( int i = _win/2 + 1; i < (_win/2 + 1 + (_shift-1)); ++i)
		_p[i] = 0;

	for ( int i = (_win - 1); i >= (_win/2 + 1 + (_shift-1)); --i)
		_p[i] = _p[i - _shift];

	//for ( int i = (_win/2 + 1 + (_shift-1)); i < (_win - _shift) - 1; ++i)
		//_p[i + _shift] = _p[i];

}


// _start - band start
// _len - band len
// _zero_inside - nulls inside band
// ! _zero_inside - nulls outside band
void applyCurve(complex *_p, size_t _sz, size_t _start, size_t _len, const float *_curv, size_t _curv_size, bool _zero_inside )
{
	// Left: _start - _curv_size ... _start
	// Right:  _start + _len ... _start + _len + _curv_size

	// NO INPUT PARAMETERS PROTECTION.

	// --- Left side ---
	size_t curv_beg, c;
	if (_start < _curv_size)
	{
		curv_beg = 0;
		c = _curv_size - _start;
	}
	else
	{
		curv_beg = _start - _curv_size;
		c = 0;
	}

	// Left zeroes?
	if ( ! _zero_inside )
		for ( size_t i = 0; i < curv_beg; ++i )
			_p[i] = 0;

	// Left curve
	for ( size_t i = curv_beg; c < _curv_size; ++c, ++i )
		_p[i] *= _curv[c];


	// Middle zeroes?
	if ( _zero_inside )
	{
		for ( size_t i = _start; i < _start + _len; ++i )
			_p[i] = 0;
	}


	// --- Rigth side ---
	// Right
	int ilim = (_start + _len + _curv_size < _sz )?(_start + _len + _curv_size):_sz;
	c = _curv_size - 1;

	for ( int i = _start + _len; i < ilim; ++i, --c )
		_p[i] *= _curv[c];

	// Right zeroes?
	if ( ! _zero_inside )
		for ( size_t i = ilim; i < _sz; ++i )
			_p[i] = 0;
}


void spectBandSuppress(complex *_p, size_t _win, size_t _start, size_t _len, float _suppress)
{
	static const float curv_size = 4;
	static const float curv[] = {
	        0.9510565162951535,
	        0.8090169943749475,
	        0.5877852522924731,
	        0.30901699437494745};
	static const float curv_mirr[] = {
	        0.30901699437494745,
	        0.5877852522924731,
	        0.8090169943749475,
	        0.9510565162951535};


	if ( 0 == _len || _start + _len > _win/2 || _start < 0 || _start > _win/2 )
		return;

	applyCurve(_p, _win/2 + 1, _start, _len, curv, curv_size, true );

	// Mirror:
	if ( 0 == _start )
	{
		//++_start;
		--_len;
	}
	applyCurve(_p + _win/2 + 1, _win/2 - 1, (_win/2 - 1) - (_start + _len), _len, curv_mirr, curv_size, true );
}




void spectBandPass(complex *_p, size_t _win, size_t _start, size_t _len, float _suppress)
{
	static const float curv_size = 4;
	static const float curv_mirr[] = {
	        0.9510565162951535,
	        0.8090169943749475,
	        0.5877852522924731,
	        0.30901699437494745};
	static const float curv[] = {
	        0.30901699437494745,
	        0.5877852522924731,
	        0.8090169943749475,
	        0.9510565162951535};

	if ( 0 == _len || _start + _len > _win/2 || _start < 0 || _start > _win/2 )
		return;

	applyCurve(_p, _win/2 + 1, _start, _len, curv, curv_size, false );

	// Mirror:
	if ( 0 == _start )
	{
		//++_start;
		--_len;
	}
	applyCurve(_p + _win/2 + 1, _win/2 - 1, (_win/2 - 1) - (_start + _len), _len, curv_mirr, curv_size, false );
}

void spectFun(complex *_p, size_t _win, size_t _start, size_t _len)
{
	// Compress spectrum.

	for ( size_t i = 2; i < _win/4; ++i )
		_p[i] = _p[i*2];

	for ( size_t i = _win/4; i < (_win/2); ++i )
		_p[i] = 0;

	// MIrror
	for ( size_t i = _win-2, j = 1; i > (_win/2) + (_win/4); --i, j += 2 )
		_p[i] = _p[i-j];

	for ( size_t i = (_win/2) + (_win/4); i > (_win/2) + 1; --i )
		_p[i] = 0;
}

void spectTestF1(complex *_p, size_t _win, size_t _start, size_t _len, float _coeff)
{
	double max = 0;
	double thr = 0;
	// Find max.

	for ( int i = std::max<int>(1, _start); i < std::min<int>(_win/2, _start + _len); ++i )
	{
		double n = _p[i].norm();
		if ( max < n )
			max = n;
	}
	thr = max / _coeff;// - (max / 2.0); // TODO magic 10.0

	// Apply.
	for ( int i = std::max<int>(1, _start); i < std::min<int>(_win/2, _start + _len); ++i )
	{
		double n = _p[i].norm();
		if ( n < thr )
		{
			_p[i] = 0;
			_p[_win - i] = 0;
		}
	}
}

void spectTestF2(complex *_p, size_t _win, size_t _start, size_t _len, float _coeff)
{
	double max = 0;
	double thr = 0;
	// Find max.

	for ( int i = std::max<int>(1, _start); i < std::min<int>(_win/2, _start + _len); ++i )
	{
		double n = _p[i].norm();
		if ( max < n )
			max = n;
	}
	thr = max / _coeff ;// - (max / 2.0); // TODO magic 10.0

	// Apply.
	for ( int i = std::max<int>(1, _start); i < std::min<int>(_win/2, _start + _len); ++i )
	{
		double n = _p[i].norm();
		if ( n > thr )
		{
			_p[i] = 0;
			_p[_win - i] = 0;
		}
	}
}

void spectTestF3(complex *_p, size_t _win, size_t _start, size_t _len)
{
}

void spectTestF4(complex *_p, size_t _win, size_t _start, size_t _len)
{
}

void spectTestReverb(complex *_p, complex *_reverb, size_t _win, size_t _start, size_t _len, float _coeff)
{
	// _reverb - reverb memory
	// _p - working set

	// Take current signal:
	if ( _start < 1 )
		_start = 1;
	if ( _start >= _win/2 )
		return;
	if ( _start + _len >= _win/2 )
		return;

	// Collect reverb information:
	for ( size_t i = _start; i < _start+_len; ++i )
	{
		_reverb[i] += _p[i];
		_reverb[i] /= _coeff;
	}
	// Mirror
	for ( size_t i = _win - (_start + _len); i < (_win - _start); ++i )
	{
		_reverb[i] += _p[i];
		_reverb[i] /= _coeff;
	}

	// Apply reverb
	for ( size_t i = _start; i < _start+_len; ++i )
	{
		_p[i] += _reverb[i];
	}
	// Mirror
	for ( size_t i = _win - (_start + _len); i < (_win - _start); ++i )
	{
		_p[i] += _reverb[i];
	}
}


void sumSamples( short *_dst, short *_src, size_t _len, unsigned _channels )
{
	for ( size_t i = 0; i < _len; ++i )
		_dst[_channels*i] = limitShort((float)_dst[_channels * i] + (float)_src[_channels * i]);

	if (2 == _channels)
		for ( size_t i = 0; i < _len; ++i )
			_dst[_channels*i + 1] = limitShort((float)_dst[_channels * i + 1] + (float)_src[_channels * i + 1]);
}


bool goodFFTWin( size_t _w )
{
	for ( size_t i = 16; i <= 65535; i *= 2 )
		if ( _w == i )
			return true;

	return false;
}

void sumFrames( char *_dst, const char *_src, size_t _size, const AudioParams& _params)
{
	// 16 bit, LE, signed
	if ( (16 == _params.m_bits) && (! _params.m_float) && (_params.m_signed) && ( ! _params.m_bigendian) )
	{
		short *dst = (short*)_dst;
		const short *src = (const short*)_src;

		for ( size_t i = 0; i < _size * _params.m_channels; ++i, ++dst, ++src )
		{
			*dst = limitShort( ((long)*dst) + ((long)*src) );
		}
	}
}

