/*
 *  pavelkolodin@gmail.com
 */

#ifndef _SignalProcessor_H_
#define _SignalProcessor_H_

#include "ProcessorTaskInfo.h"
#include "IProcessorClient.h"
#include "fft.h"
#include "TrackInfo.h"
#include <climits>
#include "VectorSimple.h"

class Controller;
class TrackAudio;


struct TrackAudio_Cursor
{
	unsigned m_id;
	TrackAudio* m_track;
	CursorInfo m_cursor;
};


// perform single-thread signal processing
class SignalProcessor
{


	public:
		SignalProcessor ( Controller *_ctrl );
		virtual ~SignalProcessor ( );

		void loadFFT( const ProcessorTaskInfo& _task );
		void loadWaveForm( const ProcessorTaskInfo& _task );
		void loadData( const ProcessorTaskInfo& _task );

	private:
        SignalProcessor ( const SignalProcessor& _other ) { }
        SignalProcessor& operator= ( const SignalProcessor& _other ) { return *this; }

    	// Read data from track.
        // This procedure calculates the size of data that must be read from track
        // to provide data for several FFT calculations.
        // * We need separate function to isolate frames reading code
        // from FFT calculation code. The "frames reading code" is not very easy.
        bool readTrackForFFT__(const ProcessorTaskInfo &_task );

        Controller *m_ctrl;

        VectorSimple<char> m_audio;
        VectorSimple<complex> m_complex;
        VectorSimple< TaskDataWaveForm::ValueType > m_waveform;

    	//std::vector<char> m_buffer_samples;
    	//std::vector<char> m_buffer_samples_2;
    	//std::vector<complex> m_pSignal;
    	//std::vector< TaskDataWaveForm::ValueType > m_buffer_waveform;

    	FFTData m_fft_data;
};

template < typename T >
class traitLimits
{
public:
	enum Limits { MIN = 0, MAX = 1 };
};

template < >
class traitLimits<int16_t>
{
public:
	enum Limits { MIN = SHRT_MIN, MAX = SHRT_MAX };
};

template < >
class traitLimits<int32_t>
{
	enum Limits { MIN = -2147483647, MAX = 2147483647 };
};

template < typename T > void calcWaveForm( char *_src, const AudioParams& _ap, unsigned _channel, TaskDataWaveForm::ValueType *_dst, unsigned _step, size_t _pixels )
{
	T *s = (T*)_src;
	TaskDataWaveForm::ValueType *d = _dst;

	for ( 	size_t i = 0;
			i < _pixels;
			++i, s += _step * _ap.m_channels, d += 2 )
	{
		// min = MAX, max = MIN !!!
		TaskDataWaveForm::ValueType min = traitLimits<T>::MAX;
		TaskDataWaveForm::ValueType max = traitLimits<T>::MIN;

		for ( size_t j = 0; j < _step; ++j )
		{
			TaskDataWaveForm::ValueType value = s[ j * _ap.m_channels + _channel ];
			if ( min > value )
				min = value;
			if ( max < value )
				max = value;
		}

		d[0] = min;
		d[1] = max;
	}

	// post-processing.
	// Remove all gaps.
	if ( _pixels < 2 )
		return;

	TaskDataWaveForm::ValueType *first = _dst;
	TaskDataWaveForm::ValueType *second = _dst + 2;

	for ( size_t i = 0; i < (_pixels-1); ++i )
	{
		if ( first[0] > second[1] )
		{
			TaskDataWaveForm::ValueType diff_half = (first[0] - second[1])/2;
			first[0] -= diff_half;
			second[1] += diff_half;
		}
		else
		if ( first[1] < second[0] )
		{
			TaskDataWaveForm::ValueType diff_half = (second[0] - first[1])/2;
			first[1] += diff_half;
			second[0] -= diff_half;
		}

		first += 2;
		second += 2;
	}
}


#endif

