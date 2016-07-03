/*
 *  pavelkolodin@gmail.com
 */

#include "WidgetTrackWaveForm.h"
#include <boost/foreach.hpp>
#include "Processor.h"

#define HEIGHT_BLOCK 1024

WidgetTrackWaveForm::WidgetTrackWaveForm ( QWidget *_parent, WidgetTrackAbstract *_wtg, Controller *_ctrl, TrackAudio::Ptr _track )
: WidgetTrackAbstract( _parent, _track.get(), _wtg, _ctrl )
, m_track(_track)
, m_block_table( 64 )
, m_block_task( 8 )
{
	setMouseTracking(true);
	setFocusPolicy( Qt::NoFocus );

	BlockWaveForm::Initializer bi( HEIGHT_BLOCK );
	m_block_table.init( bi );

	connect(this, SIGNAL(signalTaskDone()), this, SLOT(slotTaskDone()) );
}


WidgetTrackWaveForm::~WidgetTrackWaveForm ( )
{

}

void WidgetTrackWaveForm::refresh()
{
	if ( m_track->getRevision() != m_revision )
	{
		setRevision( m_track->getRevision() );

		{
			boost::unique_lock<boost::mutex> lock( m_mutex_blocks );

			if ( ! m_step_frames )
				return;

			AudioParams ap = m_track->getAudioParams();
			//CursorInfo cu = m_track->getCursorInfo();

			// Calculate W, H.
			size_t track_height = 0;
			if ( ap.m_frames )
			{
				track_height = (ap.m_frames / m_step_frames);
				if ( ap.m_frames % m_step_frames )
					track_height += 1;
			}

			m_canvas_height = track_height;
			//m_block_table.resize( 0 );
			m_block_table.resize( (m_canvas_height / HEIGHT_BLOCK) + 1 );
			m_block_table.clear();

			TaskWaveForm::Initializer ti;
			m_block_task.init( ti );
		}
		drawScreen__();
	}

	// refresh cursor information:
	CursorInfo &cu = m_track->getCursorInfo();
//	if ( cu.m_selfreq_len )
//		m_sel.setX( cu.m_selfreq_start, cu.m_selfreq_len );
//	else
		m_sel.setX( 0, m_canvas_width );

	m_sel.setY( cu.m_seltime_start, cu.m_seltime_len );
	m_cursor_pos = cu.m_pos / m_step_frames;

	update();
}

WidgetTrackAbstract::TrackPos WidgetTrackWaveForm::getLenPixels()
{
	return m_canvas_height;
}

void WidgetTrackWaveForm::setOffset( TrackPos _off )
{
	{
		boost::unique_lock<boost::mutex> lock( m_mutex_blocks );
		m_offset = _off;
	}

	drawScreen__();
	update();
}


void WidgetTrackWaveForm::setMouseMode(WidgetTrackAbstract::MouseMode _mode)
{
	if ( WidgetTrackAbstract::MM_SELECTBOTH == _mode )
		_mode = WidgetTrackAbstract::MM_SELECTTIME;

	WidgetTrackAbstract::setMouseMode( _mode );
}


void WidgetTrackWaveForm::taskDone( const Processor::TaskAbstract& _t )
{
	boost::unique_lock<boost::mutex> lock( m_mutex_blocks );
	const Processor::TaskWaveForm &task = (const Processor::TaskWaveForm &)_t;

	TaskWaveForm *t = m_block_task.getTaskData( task.m_id );
	if ( ! t )
		return;

	m_block_table.enableBlock( t->m_block_index );
	BlockWaveForm *b = m_block_table.getBlock( t->m_block_index );
	b->init( task );

	m_block_task.freeTask( task.m_id );

	signalTaskDone();
}

void WidgetTrackWaveForm::slotTaskDone()
{
	update();
}

//
// PROTECTED
//

void WidgetTrackWaveForm::paintEvent(QPaintEvent *_event)
{
	boost::unique_lock<boost::mutex> lock( m_mutex_blocks );

	//buffered painting tested
	//QImage img( QSize(width(), height()), QImage::Format_RGB32 );
	//QPainter painter(&img);

	QPainter painter( this );
	painter.fillRect(0, 0, width(), height(), QColor(0xff, 0xff, 0xff, 0xff));

	// Center line
	painter.setPen( QColor(0xb0, 0xb0, 0xb0, 0xff) );
	painter.drawLine( width()/2, 0, width()/2, height()-1 );

	painter.setPen( QColor(0x30, 0x00, 0x70, 0xff ) );

	{//LOCK

		TrackPos index = m_offset/HEIGHT_BLOCK;
		int hb = 0;
		for ( int coord_start = 0; coord_start < height(); ++index, coord_start += hb )
		{
			int lines_skip = m_offset - index * HEIGHT_BLOCK;
			if ( lines_skip < 0 )
				lines_skip = 0;

			hb = HEIGHT_BLOCK - lines_skip;
			if ( lines_skip >= HEIGHT_BLOCK )
				continue;

//			std::stringstream SS;
//			SS << hb << ", " << lines_skip;
//			painter.setPen( QColor(0xF0, 0x00, 0x70, 0xff ) );
//			painter.drawLine(0, coord_start, width()/2, coord_start);
//			painter.drawText(0, coord_start + 16, SS.str().c_str() );
//			painter.setPen( QColor(0x30, 0x00, 0x70, 0xff ) );


			BlockWaveForm *b = m_block_table.getBlock(index);
			if ( b )
			{
				float scale = ((float)width()) / (float)( b->m_max - b->m_min );

				Processor::TaskWaveForm::MinMaxType *minmax_p = &b->m_waveform[ (HEIGHT_BLOCK - hb) * 2 ];
				size_t sz = std::min< size_t >( b->m_result_size - lines_skip, hb );

				for ( size_t line = 0; line < sz; ++line, minmax_p += 2 )
				{
					unsigned len = (scale * (float)(minmax_p[1] - minmax_p[0]));
					unsigned begin = (scale * (float)minmax_p[0]) + width()/2;

					if ( len )
						painter.drawLine( begin, line + coord_start, begin + len, line + coord_start);
					else
						painter.drawPoint( begin, line + coord_start);
				}
			}
		}//for
	}//LOCK


	//
	// Selection
	//
#define QCOLOR_SELECTION QColor( 0x42, 0xaa, 0xff, 0x40 )
#define QCOLOR_SELECTION_LINE QColor( 0x42, 0xaa, 0xff, 0xff )
#define PADDING_LEFT 0

	size_t sel_freq_begin = m_sel.x();
	size_t sel_freq_len = m_sel.width();
	size_t sel_freq_end = sel_freq_begin + sel_freq_len;

	size_t sel_time_begin = m_sel.y() / m_step_frames;
	size_t sel_time_len = m_sel.height() / m_step_frames;
	size_t sel_time_end = sel_time_begin + sel_time_len;

	if ( sel_time_len ) //&& sel_freq_len)
	{
		if ( (sel_time_begin <= m_offset + height()) && (sel_time_begin + sel_time_len >= m_offset))
		{
			//QColor color_sel( 0xff, 0x33, 0x00, 0x40 );
			//QColor color_sel_line( 0xff, 0x33, 0x00, 0xff );

			QColor color_sel = QCOLOR_SELECTION;
			QColor color_sel_line = QCOLOR_SELECTION_LINE;

			painter.setPen( color_sel_line );

			// Line, time, begin
			if ( sel_time_begin >= m_offset )
				painter.drawLine( PADDING_LEFT + sel_freq_begin, sel_time_begin-m_offset, PADDING_LEFT + sel_freq_end, sel_time_begin-m_offset);

			// Line, time, end.
			if ( sel_time_end < m_offset + height() )
				painter.drawLine( PADDING_LEFT + sel_freq_begin, sel_time_end-m_offset, PADDING_LEFT + sel_freq_end, sel_time_end-m_offset);


			// Limit
			if ( sel_time_begin < m_offset ) sel_time_begin = m_offset;
			if ( sel_time_end > m_offset + height() ) sel_time_end = m_offset + height()-1;

			if ( sel_time_begin >= m_offset && sel_time_end > m_offset )
			{
				painter.drawLine( PADDING_LEFT + sel_freq_begin, sel_time_begin-m_offset, PADDING_LEFT + sel_freq_begin, sel_time_end-m_offset);
				painter.drawLine( PADDING_LEFT + sel_freq_end, sel_time_begin-m_offset, PADDING_LEFT + sel_freq_end, sel_time_end-m_offset);

				painter.fillRect( PADDING_LEFT + sel_freq_begin, sel_time_begin - m_offset, sel_freq_end - sel_freq_begin, sel_time_end - sel_time_begin, color_sel );
			}
		}
	}

	//
	// Cursor
	//
	int y_center = m_cursor_pos - m_offset;
	painter.setPen( QColor(0, 0, 0, 0xff) );
	painter.drawLine(PADDING_LEFT, y_center, width()-1, y_center);


	// Line
	painter.setPen( QColor( 0xa0, 0xa0, 0xa0, 0xFF ) );
	painter.drawLine(width()-1, 0, width()-1, height()-1);


//	//buffered painting tested.
//	QPainter painter2(this);
//	QPixmap pixmap;
//	pixmap.convertFromImage( img, Qt::ColorOnly );
//	painter2.drawPixmap(0, 0, pixmap);

}

void WidgetTrackWaveForm::resizeEvent(QResizeEvent *_event)
{
	WidgetTrackAbstract::resizeEvent(_event);
	m_canvas_width = width();
	m_sel.setX(0, m_canvas_width);
}


void WidgetTrackWaveForm::drawScreen__()
{
	boost::unique_lock<boost::mutex> lock( m_mutex_blocks );

	// Optimal number of requests per time.
	// TODO: magic number 4 - - - ?
	unsigned lim = height()/HEIGHT_BLOCK;
	lim += 2;

	// hashid - index of first visible block.
	unsigned blockindex = m_offset/HEIGHT_BLOCK;
	for ( unsigned i = 0; i < lim && ((blockindex * HEIGHT_BLOCK) < m_canvas_height); ++i, ++blockindex )
	{
		if ( m_block_table.isAllocated( blockindex ) )
			continue;

		size_t task_id = 0;
		if ( ! m_block_task.newTask(&task_id) )
			continue;

		/*BlockWaveForm *b = */m_block_table.newBlock( blockindex );

		m_block_task.getTaskData( task_id )->m_block_index = blockindex;

		Processor::RequestWaveform r(this, task_id, m_track, -1, m_step_frames, blockindex * HEIGHT_BLOCK, HEIGHT_BLOCK );
		if ( ! m_ctrl->getProcessor()->request( r ) )
		{
			m_block_task.freeTask( task_id );
			break;
		}
	}
}


