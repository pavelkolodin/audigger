/*
 * WidgetTrackSonogramUtils.cpp
 *
 *  Created on: Nov 4, 2013
 *      Author: pavelkolodin
 */


#include "WidgetTrackSonogramUtils.h"
#include <fir/logger/logger.h>

void longColor(double _val, unsigned &_r, unsigned &_g, unsigned &_b)
{
	_r = 0;
	_g = 0;
	_b = 0;
	if ( _val <= 255 )
	{
		_b = _val;
	}
	else if ( _val <= (double)(255.0 * 255.0) )
	{
		_b = 255;
		_g = _val/255;
	}
	else if ( _val <= (double)(255.0 * 255.0 * 255.0) )
	{
		_b = 255 - _val/ (double)(255.0 * 255.0);
		_g = 255;
	}
	else if ( _val <= (double)(255.0 * 255.0 * 255.0 * 255.0) )
	{
		_b = 0;
		_g = 255;
		_r = _val/ (double)(255.0 * 255.0 * 255.0);
	}
	else if ( _val <= (double)(255.0 * 255.0 * 255.0 * 255.0 * 255.0) )
	{
		_b = 0;
		_g = 255 - (_val / (double)(255.0 * 255.0 * 255.0 * 255.0) );
		_r = 255;
	}
	else
		_r = 255;
}

// TODO: http://www.fourmilab.ch/documents/specrend/specrend.c

bool FFTDrawingTaskData::done(const Processor::TaskFFT& _task)
{
	unsigned _channels = _task.track()->getAudioParams().m_channels;
	size_t _width = _task.win()/2;
	size_t _size = _task.sizeResult();

	if ( ! m_drawing )
		return false;

	m_height_painted = _size;

	for ( size_t line = 0; line < _size; ++line )
	{
		unsigned char* p = m_drawing->scanLine( line );
		//++ m_curr_y;

		if ( ! p )
			return false;

		QRgb *qrgb = (QRgb*)p;

		const complex *_array = _task.result() + line * _task.win() * _channels;
		// Draw line:
		for ( size_t x = 0; x < _width; ++x )
		{
			if ( 0 == _array[x].norm() )
			{
				*qrgb = qRgb(0, 0, 0);
				++qrgb;
				continue;
			}

			float green = 0, red = 0, blue = 0;
			green = log( _array[x].norm() );
			green *= green;
			green /= 3;
			//blue = green;

			if ( 2 == _channels )
			{
				blue = log( _array[x + _task.win()].norm() );
				blue *= blue;
				blue /= 3;
			}
			else
				blue = green;

			if ( green > 255 ) green = 255;
			if ( blue > 255 ) blue = 255;
			red = (green/2 + blue/2);

			//*qrgb = qRgb((red/2 + green/2), green, red);
			*qrgb = qRgb(blue, green, red);
			++qrgb;
		}
	}

	finalize__();
	return true;
}

void FFTDrawingTaskData::finalize__()
{
	// Fill remaining lines with background color. End of track.
	if ( (int)m_height_painted < m_drawing->height() )
	{
		QPainter pnt( m_drawing );
		pnt.fillRect( 0, m_height_painted, m_drawing->width(), m_drawing->height() - m_height_painted, QColor( COLOR_WALLPAPER ) );
	}
}
