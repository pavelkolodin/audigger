/*
 * WidgetTrackSonogramUtils.h
 *
 *  Created on: Nov 4, 2013
 *      Author: pavelkolodin
 */

#ifndef WIDGETTRACKSONOGRAMUTILS_H_
#define WIDGETTRACKSONOGRAMUTILS_H_

#include <QtWidgets>
#include "Processor.h"
#include "IProcessorClient.h"
#include <fir/logger/logger.h>

//#define COLOR_WALLPAPER 0xff87004D
//#define COLOR_WALLPAPER 0xffdddddd
#define COLOR_WALLPAPER 0xff000000


struct FFTDrawingTaskData
{
	class Initializer
	{
	public:
		Initializer(int _w, int _h)
		: m_w(_w), m_h(_h) { }

		void operator()(FFTDrawingTaskData &_object)
		{
			if ( _object.m_drawing )
				delete _object.m_drawing;
			_object.m_height_painted = 0;
			_object.m_block_index = 0;

			_object.m_drawing = new QImage( QSize(m_w, m_h), QImage::Format_RGB32 );
		}
	private:
		int m_w, m_h;
	};



	FFTDrawingTaskData() : m_drawing(0), m_height_painted(0), m_block_index(0) { }
	~FFTDrawingTaskData()
	{
		if ( m_drawing )
			delete m_drawing;
	}

	void clear() { }
	bool done(const Processor::TaskFFT& _task);
	void finalize__();

	QImage *m_drawing;
	unsigned m_height_painted;
	size_t m_block_index;
};

struct BlockFFT
{
	class Initializer
	{
	public:
		Initializer(int _w, int _h)
		: m_w(_w), m_h(_h) { }

		void operator()(BlockFFT &_b)
		{
			if ( _b.m_pixmap )
				delete _b.m_pixmap;

			_b.m_pixmap = new QPixmap( m_w, m_h );
		}
	private:
		int m_w, m_h;
	};



	BlockFFT() : m_pixmap(0) { }
	~BlockFFT()
	{
		if ( m_pixmap )
			delete m_pixmap;
	}
	void clear() { }
	void init( const FFTDrawingTaskData &_taskdata )
	{
		m_pixmap->convertFromImage( *(_taskdata.m_drawing), Qt::ColorOnly );
	}

	QPixmap *m_pixmap;
};

#endif /* WIDGETTRACKSONOGRAMUTILS_H_ */
