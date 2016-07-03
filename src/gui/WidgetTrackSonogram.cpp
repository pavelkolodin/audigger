/*
 *  pavelkolodin@gmail.com
 */

#include <iostream>
#include <sstream>
#include <QtWidgets>
#include <limits>
#include <fir/logger/logger.h>
#include <fir/str/str.h>
#include "WidgetTrackSonogram.h"
#include "WidgetTrackAbstract.h"
//#include "WidgetTracksContainer.h"
#include "Controller.h"
//#include "Processor.h"
#include "LoggerPkl.h"
#include "defines.h"
#include "CommonGui.h"

// Height of canvas block.
#define HEIGHT_BLOCK 32 //128
// How many blocks of canvas (QPixmaps) is cached.
#define BLOCKS_MEM_SIZE (32768/HEIGHT_BLOCK)
#define BLOCKS_TASKS_SIZE 8
#define PADDING_LEFT 0

// How many selection images we can hangle?
// What is "selection image"? Is it block?
#define IMGSEL_MEM_SIZE 32

#define QCOLOR_SELECTION QColor( 0x42, 0xaa, 0xff, 0x40 )
#define QCOLOR_SELECTION_LINE QColor( 0x42, 0xaa, 0xff, 0xff )


WidgetTrackSonogram::WidgetTrackSonogram(QWidget *_parent, WidgetTrackAbstract *_wtg, Controller *_ctrl, TrackAudio::Ptr _track)
	: WidgetTrackAbstract( _parent, _track.get(), _wtg, _ctrl  )
	, m_track(_track)
	, m_fft_win(0)
	, m_scrollbar( new QScrollBar(this) )
	, m_widget_progress( new QProgressBar(this) )

	, m_block_table( BLOCKS_MEM_SIZE )
	, m_block_task( BLOCKS_TASKS_SIZE )

	, m_mouse_click_offset( 0 )
	, m_mouse_click_coord_time( 0 )
	, m_mouse_click_coord_freq( 0 )
{
	setMouseTracking(true);
	setFocusPolicy( Qt::NoFocus );

	m_scrollbar->setOrientation( Qt::Horizontal );
	m_scrollbar->hide();


	m_widget_progress->setRange(0,100);
	m_widget_progress->hide();

	// We need this signal-slot, because we work with QPainter.
	connect ( this, SIGNAL(signalDrawDone(unsigned)), this, SLOT(slotDrawDone(unsigned)), Qt::QueuedConnection );
	connect ( m_scrollbar.get(), SIGNAL(valueChanged(int)), this, SLOT(slotScrollBarHoriz(int)) );
	connect ( this, SIGNAL(signalProgressValue(int)), m_widget_progress, SLOT(setValue(int)) );
}

WidgetTrackSonogram::~WidgetTrackSonogram()
{

}

void WidgetTrackSonogram::setCanvasSize(unsigned _x, unsigned _y)
{
	// this function may be called twice with the same arguments:
	// for example, when you change stepframes:
	// 1. in setStepFrame()
	// 2. in refresh(), because setStepFrame() changed revision.
	boost::unique_lock<boost::mutex> lock( m_mutex_blocks );

	bool recreate_table = (m_canvas_width != _x || m_canvas_height != _y );
	m_canvas_width = _x;
	m_canvas_height = _y;
	actualizeScrollBarHoriz__();

	if ( recreate_table )
	{
		BlockFFT::Initializer bi( _x, HEIGHT_BLOCK );
		m_block_table.resize( _y/HEIGHT_BLOCK + 1 );
		m_block_table.init( bi );

		FFTDrawingTaskData::Initializer ti( _x, HEIGHT_BLOCK );
		m_block_task.init( ti );
	}

	// Set up selection engine.
	//setImageSelSize__( _x );
}

void WidgetTrackSonogram::refresh()
{
	// Update spectrogram if track's revision changed
	if ( getRevision() != m_track->getRevision() )
	{
		setRevision( m_track->getRevision() );

		m_block_table.clear();
		//m_canvas_height = 0;
		recalcParams__();
		drawScreen__();
	}

	// refresh cursor information:
	CursorInfo &cu = m_track->getCursorInfo();
	if ( cu.m_selfreq_len )
		m_sel.setX( cu.m_selfreq_start, cu.m_selfreq_len );
	else
		m_sel.setX( 0, m_canvas_width );

	m_sel.setY( cu.m_seltime_start, cu.m_seltime_len );
	m_cursor_pos = cu.m_pos / m_step_frames;

	update();
}

WidgetTrackAbstract::TrackPos WidgetTrackSonogram::getLenPixels()
{
	return m_canvas_height;
}


void WidgetTrackSonogram::setOffset(WidgetTrackAbstract::TrackPos _off)
{
	m_offset = _off;

	if ( ! m_canvas_height )
		return;

	drawScreen__();
	update();
}


void WidgetTrackSonogram::refreshRange(WidgetTrackAbstract::TrackPos _from, WidgetTrackAbstract::TrackPos _len)
{
	{
		boost::unique_lock<boost::mutex> lock( m_mutex_blocks );

		// first block = _from
		if ( _from >= m_canvas_height )
			return;

		if ( _from + _len >= m_canvas_height )
			_len = m_canvas_height - _from;

		size_t index = _from / HEIGHT_BLOCK;
		size_t lim = ((_from + _len) / HEIGHT_BLOCK) + 1;

		for ( ; index < lim; ++index )
			m_block_table.freeBlock( index );
	}

	drawScreen__();
}


void WidgetTrackSonogram::setFFTWin( unsigned _win )
{
	m_fft_win = _win;
}

void WidgetTrackSonogram::setStepFrames( unsigned _step )
{
	WidgetTrackAbstract::setStepFrames( _step );

	// good
//	resetRevision();
//	refresh();

	// bad
	//recalcParams__();
	//drawScreen__();
}


//
// P R I V A T E
//
void WidgetTrackSonogram::resizeEvent(QResizeEvent *_event)
{
	m_widget_progress->resize(width(), 20);
	m_widget_progress->move(0, height()-m_widget_progress->height());

	actualizeScrollBarHoriz__();
}

// This function should be optimized in future. TODO
void WidgetTrackSonogram::paintEvent(QPaintEvent *_event)
{
	paintBlocks_( _event );
}

void WidgetTrackSonogram::paintBlocks_(QPaintEvent *_event)
{
	if ( ! m_canvas_width )
		return;

	unsigned offset_horiz = 0;
	if ( ! m_scrollbar->isHidden() )
	{
		offset_horiz = m_scrollbar->value();

		if ( offset_horiz >= m_canvas_width )
			offset_horiz = m_canvas_width-1;
	}
	int iw = (m_canvas_width)?std::min<int> ( m_canvas_width - offset_horiz, width() ) : width();


	QPainter painter( this );
	painter.fillRect(0, 0, width(), height(), QColor(0, 0, 0, 0xff) );
	painter.setPen( QColor(0xff, 0x00, 0x00, 0xa0) );


	{//LOCK
		boost::unique_lock<boost::mutex> lock( m_mutex_blocks );

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


			BlockFFT *b = m_block_table.getBlock(index);
			if ( b )
			{
				painter.drawPixmap( PADDING_LEFT, coord_start, *b->m_pixmap, offset_horiz, lines_skip, iw, hb );

				// DEBUG
				//painter.setPen( QColor(0xF0, 0x00, 0x70, 0xff ) );
				//painter.drawLine(0, coord_start, width()/2, coord_start);
			}
			else
			{
				painter.fillRect( PADDING_LEFT, coord_start, width(), hb, QColor( COLOR_WALLPAPER ));
				continue;
			}
		}
	}//UNLOCK

	//
	// Selection
	//

	size_t sel_freq_begin = m_sel.x() - offset_horiz;
	size_t sel_freq_len = m_sel.width();
	size_t sel_freq_end = sel_freq_begin + sel_freq_len;

	size_t sel_time_begin = m_sel.y() / m_step_frames;
	size_t sel_time_len = m_sel.height() / m_step_frames;
	size_t sel_time_end = sel_time_begin + sel_time_len;

	if ( sel_time_len && sel_freq_len)
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
	//painter.setPen( QColor(0x66, 0xff, 0xaa, 0xff) );
	painter.setPen( QColor(0xf4, 0xc4, 0x30, 0xff) );
	painter.drawLine(PADDING_LEFT, y_center, iw, y_center);

}

//void WidgetTrackSonogram::paintProgressBar_(QPaintEvent *_event)
//{
//	if ( m_task_percent > 100 )
//		return;
//
//	static const unsigned OFF_W = 2;
//	static const unsigned OFF_H = 2;
//	static const unsigned WIDTH = 200;
//	static const unsigned HEIGHT = 15;
//	static const unsigned BORDER = 10;
//
//	QPainter painter(this);
//
//	painter.setPen( QColor(0xff, 0xff, 0xff, 0xff) );
//	painter.drawRect( OFF_W, OFF_H, WIDTH, HEIGHT );
//
//	// background:
//	painter.fillRect( OFF_W + BORDER, OFF_H + BORDER, WIDTH - BORDER, HEIGHT - BORDER, QColor(0,0,0,0xff) );
//
//	// progress:
//	double percent_width = (((double) WIDTH ) / 100.0) * ((double) m_task_percent);
//
//
//	painter.fillRect( OFF_W + BORDER, OFF_H + BORDER, percent_width - BORDER, HEIGHT - BORDER, QColor(0,0xff,0,0xff) );
//}



void WidgetTrackSonogram::mouseMoveEvent(QMouseEvent *_event)
{
	WidgetTrackAbstract::mouseMoveEvent(_event);

	if ( WidgetTrackAbstract::MM_PEN == m_mode )
	{
//		if (_event->buttons() & Qt::LeftButton)
//			mousePen__( _event );
	}
	else
	if ( WidgetTrackAbstract::MM_DRAG == m_mode)
	{
		// DRAGGING
		if ( m_mouse_pressed_here && (_event->buttons() & Qt::LeftButton) )
		{
			int new_offset = m_mouse_click_offset - (_event->y() - m_mouse_click_coord_time);
			if ( new_offset < 0 )
			{
				// 
				m_mouse_click_offset -= new_offset;
				new_offset = 0;
			}

			// it will move vertical scrollbar of WTG that will issue SIGNAL
			// that will call SLOT in WTG that will call our setOffset().
			m_wtg->setOffset( new_offset );

			//
			if ( ! m_scrollbar->isHidden() )
			{
				int new_offset_2 = m_scrollbar->value() - (_event->x() - m_mouse_click_coord_freq);
				if ( new_offset_2 < 0 )
					new_offset_2 = 0;
				if ( new_offset_2 > m_scrollbar->maximum() )
					new_offset_2 = m_scrollbar->maximum();

				m_scrollbar->setValue( new_offset_2 );
				m_mouse_click_coord_freq = _event->x();
			}
		}
	}

}


void WidgetTrackSonogram::mousePressEvent(QMouseEvent* _event)
{
	m_wtg->setSelectedChild(this, true);

	m_mouse_click_offset = m_offset;
	m_mouse_click_coord_time = _event->y();
	m_mouse_click_coord_freq = _event->x();

	WidgetTrackAbstract::mousePressEvent( _event );

	if ( Qt::RightButton == _event->button() )
	{
		// If context menu invoked inside selected region?

		size_t frame = (_event->y() + m_offset) * m_step_frames;
		LOGVAR3(frame, m_sel.height(), m_sel.y() );

		if ( m_sel.height() &&
				( (int)frame >= m_sel.y() && (int)frame < (m_sel.y() + m_sel.height()) ) )
		{
				QMenu men( this );

				/*QAction *act_copy = */	men.addAction("Copy");
				/*QAction *act_cut = */		men.addAction("Cut");
				/*QAction *act_delete = */	men.addAction("Delete");
				men.addSeparator();
				QAction *act_export = 	men.addAction("Fragment Save As...");

//				men.addSeparator();
//
//				QMenu *menu_change = men.addMenu("Change");
//				menu_change->addAction("Revert");
//				menu_change->addSeparator();
//				menu_change->addAction("Silence");
//				menu_change->addAction("Noise");
//				menu_change->addAction("Tone");

				connect( act_export, SIGNAL(triggered(bool)), this, SLOT(slotExportTrackInterval()) );

				men.exec( QWidget::mapToGlobal(QPoint(_event->x(), _event->y())) );
		}
		else
		{
			// outsize selection:

			QMenu men( this );

			//QAction *act_paste = men.addAction("Paste Here...");

			QAction *act_show_cursor = men.addAction("Show Cursor");
			men.addSeparator();
			QAction *act_export = men.addAction("Export Track...");

			connect( act_show_cursor, SIGNAL(triggered(bool)), this, SLOT(slotShowCursor()) );
			connect( act_export, SIGNAL(triggered(bool)), this, SLOT(slotExportTrack()) );
			men.exec( QWidget::mapToGlobal(QPoint(_event->x(), _event->y())) );
		}
	}

	//sendSelection__();
}

//
// PROTECTED
//

void WidgetTrackSonogram::recalcParams__()
{
	if ( ! goodFFTWin( m_fft_win ) || 0 == m_step_frames )
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

	setCanvasSize( m_fft_win/2, track_height );
}

void WidgetTrackSonogram::drawScreen__()
{
	boost::unique_lock<boost::mutex> lock( m_mutex_blocks );

	unsigned blockindex = m_offset/HEIGHT_BLOCK;
	blockindex -= std::min<unsigned>(blockindex, 4);
	unsigned blockindex_end = std::min<unsigned>(m_canvas_height/HEIGHT_BLOCK + 1, blockindex + height()/HEIGHT_BLOCK + 8);

	for ( ; blockindex < blockindex_end; ++blockindex )
	{
		if ( m_block_table.isAllocated( blockindex ) )
			continue;

		size_t task_id = 0;
		if ( ! m_block_task.newTask(&task_id) )
			continue;

		m_block_table.newBlock( blockindex );

		m_block_task.getTaskData( task_id )->m_block_index = blockindex;

		Processor::RequestFFT r(this, task_id, m_track, m_fft_win, m_step_frames, blockindex * HEIGHT_BLOCK, HEIGHT_BLOCK );
		if ( ! m_ctrl->getProcessor()->request( r ) )
		{
			m_block_task.freeTask( task_id );
			break;
		}
	}
}

void WidgetTrackSonogram::actualizeScrollBarHoriz__()
{
	if ( width() < (int)m_canvas_width )
	{
		m_scrollbar->move( 0, height() - 20 );
		m_scrollbar->resize( width(), 20 );
		m_scrollbar->setRange(0, m_canvas_width - width() );
		m_scrollbar->show();
	}
	else
		m_scrollbar->hide();
}

bool WidgetTrackSonogram::taskStillValid( unsigned _task_id )
{
	boost::unique_lock<boost::mutex> lock( m_mutex_blocks );
	return ( NULL != m_block_task.getTaskData( _task_id ) );
}

void WidgetTrackSonogram::taskDone(const Processor::TaskAbstract &_task)
{
	boost::unique_lock<boost::mutex> lock( m_mutex_blocks );

	if ( Processor::PTT_FFT == _task.m_type )
	{

		FFTDrawingTaskData *td = m_block_task.getTaskData( _task.m_id );
		if ( ! td )
			return;

		td->done( (const Processor::TaskFFT&)_task );

		// TODO: delete this mutex, because call of Qt signal
		// must be thread-safe. But valgrind DRD disagree, so i add
		// this lock for experiments with valgrind.
		boost::unique_lock<boost::mutex> lock2(m_mutex_signal_draw_done);

		// We need slotDrawDone + signalDrawDone, because we want to change QPixmap.
		// QPixmap cannot be safely changed outside MAIN THREAD!!! X11 client crashes.
		signalDrawDone( _task.m_id );
	}
	else
	if ( Processor::PTT_EXPORT == _task.m_type )
	{
		// TODO: make it static: show(), hide()
		m_widget_progress->hide();
	}
}

void WidgetTrackSonogram::taskPercent(const Processor::TaskAbstract &_task, unsigned _percent)
{
	if ( Processor::PTT_EXPORT == _task.m_type )
	{
		signalProgressValue( _percent );
	}
}

void WidgetTrackSonogram::slotDrawDone(unsigned _task_id)
{
	{
		boost::unique_lock<boost::mutex> lock( m_mutex_blocks );

		FFTDrawingTaskData *td = m_block_task.getTaskData( _task_id );
		if ( td )
		{
			m_block_table.enableBlock( td->m_block_index );
			BlockFFT *b = m_block_table.getBlock( td->m_block_index );
			if ( b )
			{
				//LOGCOUT( "M" );
				b->init( *td );
			}
		}

		m_block_task.freeTask( _task_id );
	}
	update();
	drawScreen__();
}

void WidgetTrackSonogram::slotScrollBarHoriz(int _value)
{
	update();
}

void WidgetTrackSonogram::slotExportTrack()
{
	std::vector<char> filename_tmp;
	if ( ! dialogSaveAsFile(this, filename_tmp) )
		return;

	Processor::RequestExport r(this, 123, &filename_tmp[0], m_track);
	if ( m_ctrl->getProcessor()->request(r) )
	{
		m_widget_progress->show();
		m_widget_progress->setValue(0);
	}
}

void WidgetTrackSonogram::slotExportTrackInterval()
{
	std::vector<char> filename_tmp;
	if ( ! dialogSaveAsFile(this, filename_tmp) )
		return;

	size_t start = m_track->getCursorInfo().m_seltime_start;
	size_t len = m_track->getCursorInfo().m_seltime_len;

	Processor::RequestExport r(this, 123, &filename_tmp[0], m_track, start, len );
	if ( m_ctrl->getProcessor()->request(r) )
	{
		m_widget_progress->show();
		m_widget_progress->setValue(0);
	}
}

void WidgetTrackSonogram::slotShowCursor()
{
	m_wtg->setOffset( offsetForPos( m_cursor_pos ) );
}


