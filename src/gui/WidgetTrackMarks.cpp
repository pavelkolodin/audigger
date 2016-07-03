/*
 *  pavelkolodin@gmail.com
 */

#include <iostream>
#include <QtWidgets>
#include <sstream>
#include <ctime>
#include "WidgetTrackMarks.h"

#include "TrackMarks.h"

#include "Controller.h"
#include "Processor.h"

#include "WidgetTrackAbstract.h"
//#include "WidgetTracksContainer.h"

#include <fir/logger/logger.h>
#include "defines.h"
#include "common.h"

#define MIN_TEXTEDIT_HEIGHT 22


WidgetTrackMarks::WidgetTrackMarks( QWidget *_parent, WidgetTrackAbstract *_wtg, Controller *_ctrl, TrackMarks::Ptr _track )
	: WidgetTrackAbstract( _parent, _track.get(), _wtg, _ctrl )
	, m_track( _track )
	, m_interval( _track )
	, m_text_edit( 0 )
	, m_view_pos_frame( 0 )
	, m_mark_dragging_copying( 0 )
	, m_mark_editing_pos(0)
	, m_mark_editing(0)
	, m_new_mark_pos( 0 )
	, m_multiselect( 0 )
{
	//m_track->setMultiSelect( m_multiselect );

	// Receive mouseMoveEvent():
	setMouseTracking(true);
	setFocusPolicy( Qt::NoFocus );
	setAcceptDrops(true);

	m_text_edit = new WidgetTextEditEnter( this );
	m_text_edit->hide();

	this->setMinimumHeight(0);
	this->setMinimumWidth(0);

	this->resize(100, 100);

	//m_rangeregulator = new QPixmap(16, 16);

	//m_rangeregulator.convertFromImage( QImage(":/images/mark-regulator-updown-16x16.png"));
	//m_rangeregulator_selected.convertFromImage( QImage(":/images/mark-regulator-updown-red-16x16.png"));
	m_rangeregulator_knob.convertFromImage( QImage(":/images/mark-interval-knob-16x8.png"));

	connect (m_text_edit, SIGNAL(signalPressEnter()), this, SLOT(stopEditingMark()));
}

WidgetTrackMarks::~WidgetTrackMarks()
{

}

void WidgetTrackMarks::refresh( )
{
	m_interval.load( m_offset * m_step_frames, height() );
	m_revision = m_track->getRevision();
	update();
}

void WidgetTrackMarks::setViewPos(WidgetTrackAbstract::TrackPos _pos)
{
	if ( _pos < 0 )
		return;

	m_view_pos_frame = _pos * m_step_frames;
}

WidgetTrackAbstract::TrackPos WidgetTrackMarks::getViewPos()
{
	if ( ! m_step_frames )
		return 0;

	return m_view_pos_frame / m_step_frames;
}

void WidgetTrackMarks::setOffset(WidgetTrackAbstract::TrackPos _off)
{
	m_offset = _off;
	m_interval.setOffset( _off );
	refresh();
}

void WidgetTrackMarks::setStepFrames( unsigned _step )
{
//	if ( ! _step )
//		return;
//
//	if ( m_step_frames )
//	{
//		int diff = (m_view_pos_frame / m_step_frames) - m_offset;
//		TrackPosSigned val = (m_view_pos_frame / _step);
//		m_offset = (TrackPos) ((val > diff)?val - diff : 0);
//	}
//
//	m_step_frames = _step;
	WidgetTrackAbstract::setStepFrames( _step );

	if ( ! _step )
		return;
	m_interval.setStepFrames( _step );
}

void WidgetTrackMarks::clearSel()
{
	m_track->clearSelection();
//
//	// Why you not just call "refresh()" here?
//
//	//m_rm.unselect();
//	size_t len = m_rm.used();
//	for ( size_t i = 0; i < len; ++i )
//	{
//		if ( ! m_rm.getMarkGUI( i ).valid() )
//			continue;
//
//		m_rm.getMarkGUI(i).m_mark->m_selected = false;
//	}

	refresh();

	update();
}



//
// PROTECTED
//
void WidgetTrackMarks::resizeEvent(QResizeEvent *_event)
{
	QWidget::resizeEvent( _event ); // TODO: wtf? Remove it?
	m_interval.resetTextSize();
	m_interval.resize(_event->size().width(), _event->size().height());
}


void WidgetTrackMarks::paintEvent(QPaintEvent *_event)
{
	static const QColor	color_white( 0xff, 0xff, 0xff, 0xff );

	QPainter pnt( this );
	int w = width();
	int h = height();

	pnt.fillRect( 0, 0, w, h, color_white );

	pnt.setPen( QColor( 0x00, 0x00, 0x00, 0xff ));

	size_t len = m_interval.used();
	for ( size_t i = 0; i < len; ++i )
	{
		if ( ! m_interval.getMarkGUI( i ).m_valid )
			continue;

		paintMarkGUI( pnt, m_interval.getMarkGUI( i ) );
	}


	// independent mark

	if ( m_interval.m_indep_markgui.m_valid )
	{
		paintMarkGUI( pnt, m_interval.m_indep_markgui );
	}

	pnt.setPen( QColor( 0xa0, 0xa0, 0xa0, 0xFF ) );
	pnt.drawLine(w-1, 0, w-1, h-1);
}

void WidgetTrackMarks::paintMarkGUI(QPainter &_pnt, const MarkGUI &_mg)
{
	static const QColor color_mark_unselected( 0xfa, 0xf0, 0xe6, 0xff );
	static const QColor color_mark_selected( 0x00, 0x33, 0x99, 0xff );
	static const QColor color_rangebar_unselected( 0x6b, 0xb7, 0xf3, 0xff );
	static const QColor color_rangebar_selected( 0x00, 0x8e, 0xff, 0xff );
	static const QColor color_blocked( 0xa0, 0xa0, 0xa0, 0xff );
	static const QColor color_knob( 0xa0, 0xa0, 0xa0, 0xff );

	int coord = (int)_mg.m_coord - (int)m_offset;
	if ( (coord + (int)_mg.m_len) <= 0 )
		return;

	int w = width();
	//int h = height();
	const QColor *color_mark = &color_mark_unselected;
	const QColor *color_rangebar = &color_rangebar_unselected;

	if ( _mg.m_place_bad )
	{
		color_mark = &color_blocked;
		color_rangebar = &color_blocked;
	}
	else
	if ( _mg.m_selected )
	{
		color_mark = &color_mark_selected;
		color_rangebar = &color_rangebar_selected;
	}



	//
	// Rangeregulator
	//

	// Line of rangeregulator
	if ( _mg.m_len )
	{
		_pnt.fillRect(	_mg.m_rangeregulator_x,
						coord + 1,
						RANGEREGULATOR_WIDTH,
						_mg.m_len - 1, // because "m_coord + 1"
						*color_rangebar );
	}

	// Knob:

	if ( _mg.m_rangeregulator_knob )
	{
		//_pnt.fillRect( _mg.m_rangeregulator_x, coord + _mg.m_len, )
		_pnt.drawPixmap( _mg.m_rangeregulator_x, coord + _mg.m_len, m_rangeregulator_knob );
	}

//	// Rangeregulator icon
//	_pnt.drawPixmap( _mg.m_rangeregulator_x,
//					_mg.m_rangeregulator_coord,
//					*rr_pixmap,
//					0,
//					_mg.m_rangeregulator_visible_start,
//					RANGEREGULATOR_SIZE,
//					_mg.m_rangeregulator_visible_size);

	//
	// Label
	//

	_pnt.drawLine( 0, coord, w-1, coord );

	if ( 0 == _mg.m_label_height )
	{
		// Fix size!
		QFontMetrics fm( _pnt.font() );
		QRect r = fm.boundingRect( 0, 0, _mg.m_label_width, 1, Qt::AlignLeft | Qt::TextWordWrap, _mg.m_string_cached ); //mark_label );
		const_cast<MarkGUI*>(&_mg)->m_label_height = r.height();
		if ( _mg.m_label_height < SIZE_TEXT_MIN )
			const_cast<MarkGUI*>(&_mg)->m_label_height = SIZE_TEXT_MIN;
	}

	_pnt.fillRect( 0, coord + 1, _mg.m_label_width, _mg.m_label_height, *color_mark );

	if ( _mg.m_selected )
		_pnt.setPen( QColor( 0xff, 0xff, 0xff, 0xff ));

	_pnt.drawText( MARK_TEXT_MARGIN_LEFT, coord + 1, _mg.m_label_width, _mg.m_label_height, Qt::TextWordWrap | Qt::AlignLeft, _mg.m_string_cached);

	if ( _mg.m_selected )
		_pnt.setPen( QColor( 0x00, 0x00, 0x00, 0xff ));
}


void WidgetTrackMarks::mouseMoveEvent ( QMouseEvent * _event )
{
	WidgetTrackAbstract::mouseMoveEvent( _event );
	MarkGUI *mg = m_interval.mouseMove( _event->x(), _event->y() );
	if ( mg )
	{
		if ( Qt::LeftButton & _event->buttons() )
		{
			m_wtg->getGroup()->setCursorPos( mg->m_mark->m_pos );
			m_wtg->getGroup()->setSelectionTime( mg->m_mark->m_pos, mg->m_mark->m_len );
			m_wtg->refresh();
		}

		setCursor( mg->getCursor() );
		update();
	}
	else
		setCursor( Qt::ArrowCursor );
}

void WidgetTrackMarks::mousePressEvent ( QMouseEvent * _event )
{
	stopEditingMark();

	// Send mouse press event to all Marks.
	// Get pointer to MarkGUI that is pressed.

	if ( Qt::LeftButton & _event->buttons() )
	{
		MarkGUI *mg = m_interval.mousePress( _event->x(), _event->y(), _event->modifiers() & Qt::ControlModifier );

		if ( mg )
		{
			m_wtg->getGroup()->setCursorPos( mg->m_mark->m_pos );
			m_wtg->getGroup()->setSelectionTime( mg->m_mark->m_pos, mg->m_mark->m_len );
			m_wtg->refresh();
		}
		else
		{
			m_wtg->getGroup()->setCursorPos( (m_offset + _event->pos().y()) * m_step_frames );
			m_wtg->getGroup()->setSelectionTime( 0, 0 );
			m_wtg->refresh();
		}
	}


	if ( m_interval.hit(_event->x(), _event->y()) && (Qt::RightButton & _event->buttons()) )
	{
		m_interval.hit(_event->x(), _event->y())->m_selected = true;
		update();

		QMenu men (this); // = new QMenu( this );
		QAction *act_del = men.addAction("Delete selected marks");
		act_del->setIcon( QIcon(":/images/remove-16x16.png") );
		connect(act_del, SIGNAL(triggered()), this, SLOT(slotMarkDelete()));
		men.addAction("Close menu");
		//men.popup(QWidget::mapToGlobal(QPoint(_event->x(), _event->y())));
		men.exec ( QCursor::pos() );
	}

	update();
}

void WidgetTrackMarks::mouseReleaseEvent( QMouseEvent *_event )
{
	m_interval.mouseRelease( _event->x(), _event->y() );
	if ( m_revision != m_track->getRevision() )
		// refresh updates the Widget's revision.
		refresh();
}


/*
void WidgetTrackMarks::dragMark(MarkGUI * _markgui, int _coord)
{
	//_mark->m_mark->print();

	// Copy
	if ( Qt::ControlModifier & QApplication::keyboardModifiers() )
	{
		m_mark_dragging_copying = true;

		QDrag *drag = new QDrag(this);
		QMimeData *mimeData = new QMimeData;
		QByteArray b_array_textplain;
		b_array_textplain.append( _markgui->m_mark->m_text.c_str() );

		QByteArray b_array_mark;
		uint32_t len = _markgui->m_mark->m_len;

		b_array_mark.append( (const char*)&len, sizeof(uint32_t) );
		b_array_mark.append( _markgui->m_mark->m_text.c_str() );

		mimeData->setData( "application/x-audigger-mark", b_array_mark );
		mimeData->setData( "text/plain", b_array_textplain );
		drag->setMimeData(mimeData);

		m_drg_mark.stop( &WidgetTrackMarks::dragMarkFinish );

		if ( m_markgui_indep.valid() )
		{
			m_markgui_indep.invalidate();
			refresh();
		}

		//Qt::DropAction dropAction =
		drag->exec(Qt::CopyAction | Qt::MoveAction);
	}
	else
	{
		m_wtg -> setSelTime( m_offset + _coord, _markgui->m_len  );
		m_wtg -> setCursorPos( m_offset + _coord );
		_markgui->move ( _coord );
		update();
	}
}

void WidgetTrackMarks::dragMarkFinish(MarkGUI * _markgui)
{
	if ( ! m_mark_dragging_copying )
	{
		Mark::TimeValue pos = (_markgui->m_coord + m_offset) * m_step_frames;
		if ( ! m_track->canAddMark(pos, _markgui->m_mark.len ) )
			return;

		Mark m = _markgui->m_mark;
		m.m_pos = pos;

		m_track->delMark( _markgui->m_mark.m_pos );
		m_track->addMark( m );

		m_markgui_indep.invalidate();
		refresh();
	}
}
*/

//void WidgetTrackMarks::dragRangeregulator(MarkGUI *_markgui, int _coord)
//{
//	int size = _coord - _markgui->m_coord;
//	if ( size < 0 )
//		size = 0;
//
//	_markgui->resize( size );
//
//	m_wtg -> setCursorPos(m_offset + _markgui->m_coord + size);
//	m_wtg -> setSelTime(m_offset + _markgui->m_coord, size);
//
//	update();
//}
//
//void WidgetTrackMarks::dragRangeregulatorFinish(MarkGUI *_markgui)
//{
//	Mark::TimeValue len = _markgui->m_len * m_step_frames;
//
//
//	m_track->setMarkLen( _markgui->m_mark->m_id, len);
//	m_markgui_indep.invalidate();
//	refresh();
//}

void WidgetTrackMarks::mouseDoubleClickEvent ( QMouseEvent * _event )
{
	if ( Qt::LeftButton != _event->button() )
		return;

	//m_rm.mouseDoubleClick(_event->x(), _event->y());

	MarkGUI *mg = m_interval.hit(_event->x(), _event->y());

	if ( mg )
	{
		startEditingMark( mg );
		return;
	}

	// Create new mark:
	Mark::TimeValue pos = (m_offset + _event->y()) * m_step_frames;
	if ( m_track->addMark( pos, 1, "", true ) )
	{
		refresh();

		// Start editing
		const Mark *m = m_track->getMark(pos);
		if ( m )
		{
			// auto object, will die after startEditingMark(), but it is OK.
			MarkGUI tmp;
			tmp.init( *m, m_step_frames );
			startEditingMark( &tmp );
		}
	}
}

void WidgetTrackMarks::keyPressEvent(QKeyEvent *_event)
{
	switch ( _event->key() )
	{
	default:
		QWidget::keyPressEvent( _event );
		break;
	case Qt::Key_F2:
		//if ( m_mark_selected )
			//startEditingMark( m_mark_selected );
		break;
	case Qt::Key_Delete:
		// Delete selected mark.
		slotMarkDelete();
		break;
	}
}

void WidgetTrackMarks::dragEnterEvent(QDragEnterEvent *event)
{
//	QStringList list = event->mimeData()->formats();
//	QStringList::iterator it, ite;
//	it = list.begin();
//	ite = list.end();
//	for ( ; it != ite; ++it )
//	{
//		LOGCOUT( it->toLocal8Bit().constData() );
//	}
//	LOGCOUT ( "" );

	if ( event->mimeData()->hasFormat( "application/x-audigger-mark" ) || event->mimeData()->hasFormat( "text/plain" ) )
		event->acceptProposedAction();
}

void WidgetTrackMarks::dragMoveEvent ( QDragMoveEvent * event )
{
	m_wtg->getGroup()->setCursorPos( (m_offset + event->pos().y()) * m_step_frames );
	m_wtg->refresh();
}

void WidgetTrackMarks::dropEvent(QDropEvent *_event)
{
	Mark::TimeValue pos = (m_offset + _event->pos().y()) * m_step_frames;

	do
	{
		if ( _event->mimeData()->hasFormat( "application/x-audigger-mark" ) )
		{
			const char *data = _event->mimeData()->data( "application/x-audigger-mark" ).data();
			uint32_t len = *((uint32_t*)data);

			const char *str = (data + sizeof(uint32_t));
			m_track->addMark( pos, len, str, true );
			break;
		}

		if ( _event->mimeData()->hasFormat( "text/plain" ) )
		{
			const char *str = _event->mimeData()->data( "text/plain" ).data();
			m_track->addMark( pos, 0, str, true );
			break;
		}

		if ( _event->mimeData()->hasFormat( "text/uri-list" ) )
		{
			const char *str = _event->mimeData()->data( "text/uri-list" ).data();
			m_track->addMark( pos, 0, str, true );
			break;

		}
	} while(0);


	const Mark *mark_new = m_track->getMark( pos );
	if ( mark_new )
	{
		m_track->setMarkSelected( mark_new->m_pos, true );
		refresh();
	}
}



//
// PRIVATE
//

//MarkGUI* WidgetTrackMarks::whosePlace(int x, int y)
//{
//	size_t len = m_rm.used();
//	// reversed traverse: important!
//	// Because more latest element goes in the list,
//	// the more first it seen on the screen.
//	for ( size_t i = len-1; i < len && i >= 0; --i )
//	{
//		if ( ! m_rm.getMarkGUI( i ).valid() )
//			continue;
//
//		if (	y >= (int)m_rm.getMarkGUI( i ).m_coord &&
//				y <= (int)(m_rm.getMarkGUI( i ).m_coord + m_rm.getMarkGUI( i ).m_size_text) &&
//				x < ((width()-1) - (RANGEREGULATOR_SIZE_2D + RANGEREGULATOR_BAR_SIZE_2D)) )
//		{
//			return &m_rm.getMarkGUI( i );
//		}
//	}
//	return 0;
//}

//MarkGUI* WidgetTrackMarks::whoseRangeregulator(int x, int y)
//{
//	int len = m_rm.used();
//	for ( int i = len - 1; i < len && i >= 0; --i )
//	{
//		if ( ! m_rm.getMarkGUI( i ).valid() )
//			continue;
//
//		if ( x > (int)((width()-1) - (RANGEREGULATOR_SIZE_2D + RANGEREGULATOR_BAR_SIZE_2D )) &&
//				x < (int)((width()-1) - RANGEREGULATOR_BAR_SIZE_2D) &&
//				y >= (int)m_rm.getMarkGUI( i ).m_rangeregulator_coord &&
//				y < (int)(m_rm.getMarkGUI( i ).m_rangeregulator_coord + m_rm.getMarkGUI( i ).m_rangeregulator_visible_size) )
//		{
//			return &m_rm.getMarkGUI( i );
//		}
//	}
//	return 0;
//}

void WidgetTrackMarks::startEditingMark( const MarkGUI* _mark )
{
	if ( ! _mark )
		return;

	m_mark_editing = true;
	m_mark_editing_pos = _mark->m_mark->m_pos;

	m_text_edit->setText ( _mark->m_string_cached );
	m_text_edit->move( 0, _mark->m_coord - m_offset );
	m_text_edit->resize( width(), _mark->m_label_height + MIN_TEXTEDIT_HEIGHT );

	m_text_edit->show();
	m_text_edit->setFocus();
}


//
// SLOTS
//


void WidgetTrackMarks::slotMarkDelete()
{
	m_track->delAllSelectedMarks();
	refresh();
}

void WidgetTrackMarks::stopEditingMark()
{
	if ( ! m_mark_editing )
		return;

	m_mark_editing = false;

	m_track->setMarkText( m_mark_editing_pos, m_text_edit->toPlainText().toUtf8().data() );

	m_text_edit -> clearFocus();
	m_text_edit -> hide();

	refresh();
}



