/*
 *  pavelkolodin@gmail.com
 */

#include <boost/foreach.hpp>
#include <fir/logger/logger.h>

#include "WidgetTracksGroup.h"
#include "WidgetTrackSonogram.h"
#include "WidgetTrackWaveForm.h"
#include "WidgetTrackMarks.h"
#include "WidgetTrackTimeLine.h"
#include "defines.h"
#include "common.h"
#include "WidgetsCommon.h"
#include "CommonGui.h"


WidgetTracksGroup :: WidgetTracksGroup (QWidget *_parent, Controller *_ctrl, TracksGroup::Ptr _group)
	: WidgetTrackAbstract ( _parent, NULL, NULL, _ctrl )
	, m_group( _group )
	, m_headers(new WidgetHeaders( this ))
	, m_scrollbar(new QScrollBar(Qt::Vertical, this))
	, m_widgetsholder(new QWidget( this ))
	, m_widgettrack_current(0)
	, m_animation_start_pos(0)
	, m_index_header_menu(-1)
{
	m_headers->setSelectable( true );
	m_headers->setFitScreen( true );
	//m_headers->setHasButtonAdd( true );
	m_headers->move(0, 0);
	m_headers->resize(100, HEIGHT_HEADER);

	m_scrollbar->setRange(0, 0);
	m_scrollbar->setEnabled( false );

	m_widgetsholder->move(0, HEIGHT_HEADER);

//	m_widgetsholder->setAutoFillBackground( true );
//	QPalette p = m_widgetsholder->palette();
//	p.setColor(m_widgetsholder->backgroundRole(), Qt::red);
//	m_widgetsholder->setPalette(p);

	m_widgetsholder->show();



	connect (m_scrollbar, SIGNAL(valueChanged(int)), this, SLOT(slotScrollVerticalChanged(int)) );

	connect (m_headers, SIGNAL(signalSelected(unsigned)), this, SLOT(slotHeaderSelected(unsigned)));
	connect (m_headers, SIGNAL(signalClose(unsigned)), this, SLOT(slotHeaderClose(unsigned)));
	connect (m_headers, SIGNAL(signalResize(unsigned, unsigned)), this, SLOT(slotHeaderResize(unsigned, unsigned)));
	connect (m_headers, SIGNAL(signalMove(unsigned, unsigned)), this, SLOT(slotHeaderMove(unsigned, unsigned)));
	connect (m_headers, SIGNAL(signalRename(unsigned, const QString&)), this, SLOT(slotHeaderRename(unsigned, const QString &)));
	connect (m_headers, SIGNAL(signalMenu(unsigned, int, int)), this, SLOT(slotHeaderMenu(unsigned,int,int)));

	_group->setCallback( this );

	m_animation.reset( new QPropertyAnimation(this, "animparam") );
}


WidgetTracksGroup :: ~WidgetTracksGroup ( )
{

}

void WidgetTracksGroup::setAnimParam(int _param)
{
	if ( ! m_step_frames )
		return;

	// Elapsed milliseconds from animation start.
	long long elapsed_ms = m_animation_elapsed_timer.elapsed();
	if ( ! elapsed_ms )
		return;

	WidgetTrackAbstract::TrackPos add_pos = (m_group->getAudioParams().m_rate/1000) * (elapsed_ms);

	WidgetTrackAbstract::TrackPos pos = (m_animation_start_pos + add_pos) / m_step_frames;
	setCursorPos( pos );
	setOffset( offsetForPos ( pos ) );
	refresh();

	//	std::stringstream SS;
	//	printMilliSecondsAsTime( SS, elapsed_ms );
	//	LOG_PURE( SS.str() );
}

void WidgetTracksGroup::startAnimation()
{
	TrackAudio::Ptr ta = m_group->getTrackAudioSelectedFirst();
	if ( ! ta )
		return;

	m_animation_start_pos = ta->getCursorInfo().m_pos;
	m_animation_elapsed_timer.start();

	// The value of animated parameter DOES NOT MATTER
	// only the animation events matters

	m_animation->setCurrentTime(0);
	m_animation->setDuration( 1000 );
	m_animation->setStartValue( 0 );
	m_animation->setEndValue( 1000 );
	m_animation->setLoopCount( -1 ); // infinite

	m_animation->start();
	//LOG_PURE( "--- " << m_animation->state() );
	//m_animation->start();
}

void WidgetTracksGroup::stopAnimation()
{
	//LOG_PURE("---------- STOP");
	m_animation->stop();
}

void WidgetTracksGroup::undo()
{
	TrackAudio::Ptr track = m_group->getTrackAudioSelectedFirst();
	if ( ! track )
		return;

	track->undo();
	refresh();
}

void WidgetTracksGroup::redo()
{
	TrackAudio::Ptr track = m_group->getTrackAudioSelectedFirst();
	if ( ! track )
		return;

	track->redo();
	refresh();
}

void WidgetTracksGroup::load()
{
	while( m_widgets.size() )
	{
		slotHeaderClose( 0 );
	}

	TracksGroup::VectorTracksAudio tracks_audio;
	TracksGroup::VectorTracksMarks tracks_marks;
	m_group->getTracksAudio( tracks_audio );
	m_group->getTracksMarks( tracks_marks );

	BOOST_FOREACH( TrackMarks::Ptr track, tracks_marks )
	{
		addTrack( track );
	}

	BOOST_FOREACH( TrackAudio::Ptr track, tracks_audio )
	{
		addTrack( track, WidgetTrackAbstract::TYPE_SONOGRAM );
	}
}

void WidgetTracksGroup::addTrack( TrackAudio::Ptr _track, WidgetTrackAbstract::TrackType _type )
{
	QString name = QString::fromUtf8( _track->getName() );

	if ( WidgetTrackAbstract::TYPE_SONOGRAM == _type )
	{
		WidgetTrackSonogram *w = new WidgetTrackSonogram( m_widgetsholder, this, m_ctrl, _track );
		w->resize( m_ctrl->getSettings().getMisc().m_default_fft_win / 2, height() );
		w->setFFTWin( m_ctrl->getSettings().getMisc().m_default_fft_win );
		addTrack__( w, name );
		w->refresh();
	}
	else if ( WidgetTrackAbstract::TYPE_WAVE == _type )
	{
		WidgetTrackWaveForm *w = new WidgetTrackWaveForm( m_widgetsholder, this, m_ctrl, _track );
		w->resize( 128, height() );
		addTrack__( w, name );
		w->refresh();
	}

	++ _track->m_users;
}

void WidgetTracksGroup::addTrack( TrackMarks::Ptr _track )
{
	QString name = QString::fromUtf8( _track->getName() );

	WidgetTrackMarks *wt = new WidgetTrackMarks( m_widgetsholder, this, m_ctrl, _track );
	wt->resize(140, height());
	addTrack__ ( wt, name );
	wt->refresh();

	++ _track->m_users;
}

void WidgetTracksGroup::addTrack( TrackTimeLine::Ptr _track )
{
	QString name = QString::fromUtf8( _track->getName() );
	WidgetTrackTimeLine *wt = new WidgetTrackTimeLine( m_widgetsholder, this, m_ctrl, _track );
	addTrack__( wt, name );
	wt->refresh();
}

size_t WidgetTracksGroup::numTracks()
{
	return m_widgets.size();
}

WidgetTrackAbstract* WidgetTracksGroup::getTrack(WidgetTrackAbstract::TrackPos _index)
{
	if ( _index < 0 || _index >= m_widgets.size() )
		return NULL;

	return m_widgets[ _index ];
}

void WidgetTracksGroup::setCursorPos( TrackPos _pos )
{
//	long cur_async =
//		m_ctrl->getSettings().getToolBar().m_states[SETTINGS_TOOLBAR_TOOL_CURSOR_ASYNC];
//
//	if ( cur_async )
//	{
//		TrackAudio::Ptr driver_track = m_group->getTrackAudioSelectedFirst();
//		if ( ! driver_track )
//			return;
//
//		driver_track->getCursorInfo().m_pos = _pos * m_step_frames;
//	}
//	else
//	{
//		// set for all
//		m_group->setCursorPos( _pos * m_step_frames );
//	}
}

void WidgetTracksGroup::refresh()
{
	BOOST_FOREACH( WidgetTrackAbstract* t, m_widgets )
	{
		t->refresh();
	}
}

void WidgetTracksGroup::scroll( int _off )
{
	int old_scroll_value =  m_scrollbar->value();
	m_scrollbar->setValue( old_scroll_value - _off );
}

void WidgetTracksGroup::setOffset(WidgetTrackAbstract::TrackPos _pos)
{
	m_scrollbar->setValue ( _pos );
}

WidgetTrackAbstract::TrackPos WidgetTracksGroup::getLenPixels()
{
	WidgetTrackAbstract::TrackPos max = 0;
	BOOST_FOREACH( WidgetTrackAbstract* t, m_widgets )
	{
		if ( t->getLenPixels() > max )
			max = t->getLenPixels();
	}
	return max;
}

WidgetTrackAbstract::TrackPos WidgetTracksGroup::moveCursor( TrackPosSigned _pos_offset )
{
	long cur_async =
		m_ctrl->getSettings().getToolBar().m_states[SETTINGS_TOOLBAR_TOOL_CURSOR_ASYNC];

	TrackAudio::Ptr driver_track = m_group->getTrackAudioSelectedFirst();
	if ( ! driver_track )
		return 0;

	size_t newpos_frame = driver_track->getCursorInfo().moveCursor( _pos_offset * m_step_frames );
	TrackPos newpos_pixel =  newpos_frame/ m_step_frames;

	setOffset( offsetForPos( newpos_pixel ) );

	// In sync mode set the same cursor position to all tracks.
	// That is it!
	if ( ! cur_async )
	{
		m_group->setCursorPos( newpos_frame );
//		BOOST_FOREACH( WidgetTrackAbstract* t, m_widgets )
//		{
//			m_group->setCursorPos( newpos_frame );
//		}
	}



	// update cursor position
	refresh();

	// we don't know the cursor position, it may differ on all tracks.
	// so lest return offset :)
	return 0;
}


void WidgetTracksGroup::setMouseMode( WidgetTrackAbstract::MouseMode _mode)
{
	BOOST_FOREACH( WidgetTrackAbstract* t, m_widgets )
	{
		t->setMouseMode( _mode );
	}
}

void WidgetTracksGroup::setStepFrames( unsigned _value )
{
	if ( 0 == _value )
		return;

	m_step_frames = _value;
	bool offset_special_flag = false;
	WidgetTrackAbstract::TrackPos offset_special = 0;
	BOOST_FOREACH( WidgetTrackAbstract* t, m_widgets )
	{
		//t->resetRevision();
		t->setStepFrames( _value );

		if ( t->hasMouse() )
		{
			offset_special_flag = true;
			offset_special = t->getOffset();
		}
	}


	if ( offset_special_flag )
	{
		setOffset__( offset_special );
	}

	fixScrollBar__();

//	BOOST_FOREACH( WidgetTrackAbstract* t, m_widgets )
//	{
//		t->refresh();
//	}
}

void WidgetTracksGroup::setSelectedChild( WidgetTrackAbstract* _track, bool _selected )
{
	m_headers->setSelectedAll( false );
	m_widgettrack_current = NULL;

	// unselect all tracks in group
	m_group->setSelected( false );

	if ( ! _selected )
		return;

	m_widgettrack_current = const_cast<WidgetTrackAbstract*>(_track);

	if ( ! _track )
		return;

	int index = -1;
	int i = 0;
	BOOST_FOREACH( WidgetTrackAbstract* t, m_widgets )
	{
		if ( t == _track )
		{
			index = i;
			break;
		}
		++ i;
	}

	if ( index < 0 )
		return;


	m_headers->setSelected( index, true );
	// Pass changes to "backend".
	_track->getTrackAbstract()->getCursorInfo().m_selected = true;
}



void WidgetTracksGroup::trackCreated( TrackAudio::Ptr _track)
{
	addTrack( _track, WidgetTrackAbstract::TYPE_SONOGRAM );
}

void WidgetTracksGroup::trackCreated( TrackMarks::Ptr _track)
{
	addTrack( _track );
}

//
// PROTECTED
//

void WidgetTracksGroup::resizeEvent(QResizeEvent *_event)
{
	if ( _event->size().width() <= 0 || _event->size().height() <= 0 )
		return;

	int w = _event->size().width() - WIDTH_SCROLLBAR;
	int h = _event->size().height() - HEIGHT_HEADER;
	if ( w < 0 ) w = 0;
	if ( h < 0 ) h = 0;


	BOOST_FOREACH( WidgetTrackAbstract* t, m_widgets )
	{
		t->setFixedHeight( h );
	}

	m_scrollbar->resize( WIDTH_SCROLLBAR, _event->size().height() );
	m_scrollbar->move( _event->size().width() - WIDTH_SCROLLBAR, 0 );

	m_headers->resize( w, HEIGHT_HEADER );
	m_widgetsholder->resize( w, h );
}

//
// PRIVATE
//

void WidgetTracksGroup::addTrack__( WidgetTrackAbstract* _wt, const QString &_name)
{
	_wt->setOffset( m_offset );
	_wt->setStepFrames( m_step_frames );

	unsigned initial_fft_win = 0;
	int options = HEADER_OPT_CLOSEABLE;

	if ( _wt->getType() == WidgetTrackAbstract::TYPE_SONOGRAM )
		initial_fft_win = m_ctrl->getSettings().getMisc().m_default_fft_win;


	// MouseMode:
	_wt->setMouseMode( getMouseModeFromSettings() );
	_wt->show();

	// insert to widgets before inserting to headers, because m_headers::addHeader may send signal
	// with new size of our widget to fit it into screen.
	m_widgets.push_back( _wt );

	unsigned index = m_headers->addHeader(_name, _wt->width(), options );

	LOGVAR1( _wt->getType() );

	if (	_wt->getType() == WidgetTrackAbstract::TYPE_SONOGRAM ||
			_wt->getType() == WidgetTrackAbstract::TYPE_WAVE )
	{
		// WidgetAnchor is EventUser!
		//WidgetAnchor *wa = new WidgetAnchor(NULL, &m_eventdistr, _wt->getTrackAudio() );
		WidgetAnchor *wa = new WidgetAnchor(NULL, this, _wt->getTrackAudio() );
		// m_headers will take ownership on the widget.
		m_headers->addWidgetToHeader( index, wa );
	}


	if ( initial_fft_win )
	{
		WidgetFFTRegulator* wfft = new WidgetFFTRegulator(NULL, initial_fft_win, _wt);
		// m_headers will take ownership on the widget.
		m_headers->addWidgetToHeader( index, wfft );
	}

	// Recalc widgets position!
	recalcWidgetsPosition__();
	fixScrollBar__();

	update();
}

WidgetTrackAbstract* WidgetTracksGroup::findFirstSelected__()
{
	WidgetTrackAbstract *find = NULL;
	BOOST_FOREACH( WidgetTrackAbstract* t, m_widgets )
	{
		if ( t->getTrackAbstract() )
			if ( t->getTrackAbstract()->getCursorInfo().m_selected )
			{
				find = t;
				break;
			}
	}
	return find;
}

void WidgetTracksGroup::setOffset__(WidgetTrackAbstract::TrackPos _offset)
{
	m_offset = _offset;
	BOOST_FOREACH( WidgetTrackAbstract* t, m_widgets )
	{
		t->setOffset( _offset );
	}
}

void WidgetTracksGroup::recalcWidgetsPosition__()
{
	int offset = 0;
	BOOST_FOREACH( WidgetTrackAbstract* t, m_widgets )
	{
		t->move(offset, 0);
		t->setFixedHeight( height() - HEIGHT_HEADER );

		offset += t->width();
	}
}


void WidgetTracksGroup::fixScrollBar__( )
{
	disconnect (m_scrollbar, SIGNAL(valueChanged(int)), this, SLOT(slotScrollVerticalChanged(int)) );

	m_scrollbar->setDisabled( 0 == m_widgets.size() );

	// RANGE
	unsigned len_pixels = getLenPixels();

	m_scrollbar->setRange(0, len_pixels);

	// OFFSET
	if ( m_widgets.size() )
		m_scrollbar->setValue( m_offset );
		//m_scrollbar->setValue( m_widgets[0]->getOffset() );

	connect (m_scrollbar, SIGNAL(valueChanged(int)), this, SLOT(slotScrollVerticalChanged(int)) );
}

//
// SLOTS
//

void WidgetTracksGroup::slotScrollVerticalChanged(int _value)
{
	setOffset__( _value );
}

void WidgetTracksGroup::slotHeaderClose(unsigned _index)
{
	if ( _index >= m_widgets.size() )
		return;

	WidgetTrackAbstract *w = m_widgets[_index];
	-- w->getTrackAbstract()->m_users;

	if ( 0 == w->getTrackAbstract()->m_users )
	{
		bool result_delete = m_group->deleteTrack( w->getTrackAbstract() );
		LOG_PURE( "Delete track from group, result: " << result_delete );
	}

	m_widgets.erase( m_widgets.begin() + _index );
	// TODO: boost::shared_ptr
	delete w;


	// Recalc widgets position!
	recalcWidgetsPosition__();
	fixScrollBar__();

	// We call WidgetHeaders::delHeader() after WidgetTrackAbstract is deleted.
	// because delHeader() will send signalResize for each header after deleting a header
	// but this signal will operate with new indexes (because of deleted one).
	m_headers->delHeader( _index );

	update();
}

void WidgetTracksGroup::slotHeaderSelected( unsigned _index )
{
	if ( _index >= m_widgets.size() )
		return;

	setSelectedChild( m_widgets[ _index ], true );
}

void WidgetTracksGroup::slotHeaderMove( unsigned _from, unsigned _to )
{
	LOGVAR2( _from, _to );

	if ( _from >= m_widgets.size() )
		return;

	if ( _to >= m_widgets.size() )
		return;


	WidgetTrackAbstract *tmp = m_widgets[ _from ];

	m_widgets.erase( m_widgets.begin() + _from );
	m_widgets.insert( m_widgets.begin() + _to, tmp );

	//std::swap ( m_widgets[ _index ], m_widgets[ _index2 ]);

	recalcWidgetsPosition__();
	update();
}

void WidgetTracksGroup::slotHeaderResize( unsigned _index, unsigned _size )
{
	if ( _index >= m_widgets.size() )
		return;

	(*(m_widgets.begin() + _index))->setFixedWidth( _size );

	recalcWidgetsPosition__();
	update();
}

void WidgetTracksGroup::slotHeaderRename(unsigned _index, const QString &_text)
{
	if ( _index >= m_widgets.size() )
		return;

	QStringToCharString conv;
	const char *name = conv.convert( _text );

	m_widgets[_index]->getTrackAbstract()->setName( name );

	LOG_PURE( "OK! " << name );
}

void WidgetTracksGroup::slotHeaderMenu(unsigned _index, int x, int y)
{
	if ( _index >= m_widgets.size() )
		return;

	if ( ! m_widgets[_index]->getTrackAudio() )
		return;

	QMenu menu( this );

	QAction *act_new_FFT = menu.addAction("Show FFT");
	QAction *act_new_WaveForm = menu.addAction("Show WaveForm");
	menu.addSeparator();
	QAction *act_save_WAV = menu.addAction("Export Track...");

	connect( act_new_FFT, SIGNAL(triggered(bool)), this, SLOT(slotNewFFT()) );
	connect( act_new_WaveForm, SIGNAL(triggered(bool)), this, SLOT(slotNewWaveForm()) );
	//connect( act_save_WAV, SIGNAL(triggered(bool)), this, SLOT(slotSaveAsWav()) );
	connect( act_save_WAV, SIGNAL(triggered(bool)), m_widgets[_index], SLOT(slotExportTrack()) );

	m_index_header_menu = _index;
	menu.exec( QWidget::mapToGlobal(QPoint(x, y)) );
	m_index_header_menu = -1;
}


void WidgetTracksGroup::slotNewFFT()
{
	if ( m_index_header_menu < 0 )
		return;

	if ( (size_t)m_index_header_menu >= m_widgets.size() )
		return;

	if ( ! m_widgets[m_index_header_menu]->getTrackAudio() )
		return;

	addTrack(m_widgets[m_index_header_menu]->getTrackAudio(), WidgetTrackAbstract::TYPE_SONOGRAM );
}


void WidgetTracksGroup::slotNewWaveForm()
{
	if ( m_index_header_menu < 0 )
		return;

	if ( (size_t)m_index_header_menu >= m_widgets.size() )
		return;

	if ( ! m_widgets[m_index_header_menu]->getTrackAudio() )
		return;

	addTrack(m_widgets[m_index_header_menu]->getTrackAudio(), WidgetTrackAbstract::TYPE_WAVE );
}

void WidgetTracksGroup::slotSaveAsWav()
{
	std::vector<char> filename_tmp;
	if ( ! dialogSaveAsFile(this, filename_tmp) )
		return;

	try
	{
		LOG_PURE("Save track " << m_index_header_menu << "...");

		m_widgets[m_index_header_menu]->getTrackAudio()->save( &filename_tmp[0] );

		LOG_PURE("Save track OK");
	}
	catch ( const std::runtime_error &e)
	{
		QMessageBox msg( QMessageBox::Critical, QString("error"), e.what(), QMessageBox::Cancel, this );
		msg.exec();
	}
}


