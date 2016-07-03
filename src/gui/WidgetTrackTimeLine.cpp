/*
 *  pavelkolodin@gmail.com
 */


#include "WidgetTrackTimeLine.h"
#include "TracksGroup.h"
#include "common.h"
#include <sstream>

// Array of values of millisecond time intervals
// used to display time markers on time line.
//
// For each m_step_frames, we choose the value allowing marks to be
// on "comfortable-for-reading" distance from each other.
static const unsigned long timetable_ms[] =
{
		1, // Fake. To be able to step backward from next element :)
		1,
		2,
		5,
		10,
		20,
		40,
		50,
		100,
		200,
		400,
		500,
		1 * 1000, // 1 second
		2 * 1000,
		5 * 1000,
		10 * 1000,
		20 * 1000,
		30 * 1000,
		60 * 1000, // 1 minute
		2 * 60 * 1000,
		5 * 60 * 1000,
		10 * 60 * 1000,
		15 * 60 * 1000,
		30 * 60 * 1000,
		60 * 60 * 1000, // 1 hr
		60 * 60 * 1000 + 30 * 60 * 1000, // 1.5 hr
		2 * 60 * 60 * 1000, // 2 hr
		3 * 60 * 60 * 1000, // 2 hr
		4 * 60 * 60 * 1000, // 2 hr
		8 * 60 * 60 * 1000, // 2 hr
		0
};



WidgetTrackTimeLine :: WidgetTrackTimeLine ( QWidget *_parent, WidgetTrackAbstract *_wtg, Controller *_ctrl, TrackTimeLine::Ptr _track )
: WidgetTrackAbstract( _parent, _track.get(), _wtg, _ctrl )
, m_pos(0)
, m_interval_ms(0)
{

}

WidgetTrackTimeLine :: ~WidgetTrackTimeLine ( )
{

}

//TrackType WidgetTrackTimeLine::getType() { return TYPE_TIMELINE; }

void WidgetTrackTimeLine::refresh()
{
	updateData__();
	update();
}

void WidgetTrackTimeLine::setCursorPos(TrackPos _pos)
{
	m_pos = _pos;
	update();
}

void WidgetTrackTimeLine::setOffset( TrackPos _off )
{
	updateData__();
	m_offset = _off;
	update();
}

void WidgetTrackTimeLine::setStepFrames( unsigned _step )
{
	if ( ! _step )
	{
		m_step_frames = 0;
		m_interval_ms = 0;
		return;
	}

	m_step_frames = _step;

	double rate = m_wtg->getGroup()->getAudioParams().m_rate;
	if ( 0 == rate )
		return;

	m_ms_per_pixel = 1000.0 * (((double)_step) / rate);
	//LOGVAR1( m_ms_per_pixel );


	// find time interval:
	static const unsigned pixels_min = 80;
	double min_interval =  m_ms_per_pixel * (double) pixels_min; // seconds
	//LOGVAR1( min_interval );
	size_t i = 1;
	for ( i = 1; timetable_ms[i] != 0; ++i )
	{
		double how_many_intervals = min_interval / (((double)timetable_ms[i]));
		if ( how_many_intervals < 1.0 )
			break;
	}
	-- i;

	m_interval_ms = timetable_ms[i];
	//LOGVAR1( m_interval_ms );
	update();
}


// PROTECTED:

void WidgetTrackTimeLine::paintEvent(QPaintEvent *)
{
	if ( ! m_interval_ms )
		return;

	// find first mark:
	unsigned long integer = (long)((m_ms_per_pixel * (double)m_offset) / (((double)m_interval_ms)) );
	unsigned long time_point_start_ms = m_interval_ms * (integer + 1);

	QPainter painter(this);
	painter.setPen( QColor(0x00, 0x00, 0x00, 0xff) );

	// draw time mark on each 50 pixels:

	std::stringstream SS;

	for ( unsigned long t = time_point_start_ms; ; t += m_interval_ms )
	{
		//LOGVAR1 ( t );
		TrackPos y = ((TrackPos)(((double)t) / m_ms_per_pixel)) - m_offset;
		if ( (int)y > height() )
			break;
		painter.setPen( QColor(0x00, 0x00, 0x00, 0xff) );
		painter.drawLine( 0, y, width()-1, y );

		SS.str("");
		printMilliSecondsAsTime( SS, t );

		QString qs ( SS.str().c_str() );
		QFontMetrics fm( painter.font() );
		QRect r = fm.boundingRect( 0, 0, width(), 10, Qt::AlignLeft, qs );

		painter.fillRect( 0, y + 1, width()-1, r.height() + 2, QColor(0xdd, 0xdd, 0xee, 0xff) );

		//painter.drawText( (width() - r.width()) / 2, y + 20, QString( SS.str().c_str() ) ); // TODO: magic 20
		painter.drawText( 1, y + r.height() + 1, QString( SS.str().c_str() ) ); // TODO: magic 20
	}
}



