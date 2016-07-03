/*
 *  pavelkolodin@gmail.com
 */



#include <iostream>
#include <QtWidgets>
#include <sstream>
#include <fir/logger/logger.h>
#include "WindowMain.h"
#include "WindowAbout.h"
#include "ProjectManager.h"
#include "Processor.h"
#include "Controller.h"
#include "CommonGui.h"
#include <fir/str/str.h>
#include "defines.h"

#define HEIGHT_TOP_PANEL 60
#define WHEELRATE 64
#define TOOLBAR_INDENT 4
#define WIDTH_MENUBUTTON 26
#define HEIGHT_TABS 24
#define SPACE_MENUBUTTON_TABS 5
#define HEIGHT_TOOLBAR 24
#define INTERVAL_PANELS 4

unsigned getStepFramesByIndex(int _i)
{
	for ( int i = 0, s = FFT_STEP_MIN; s <= FFT_STEP_MAX; s *= 2, ++i )
		if ( _i == i )
			return s;
	return 0;
}

int getIndexByStepFrames(unsigned _step)
{
	for ( unsigned i = 0, s = FFT_STEP_MIN; s <= FFT_STEP_MAX; s *= 2, ++i )
		if ( _step == s )
			return i;
	return -1;
}





WidgetMainMenu::WidgetMainMenu(QWidget *_parent)
: QObject( _parent )
{
	//resize(0,0);

	m_menu = new QMenu (_parent);
	m_act_project_new = m_menu->addAction(QIcon(":/images/tools/new-24x24.png"), "New Project");
	m_act_new_trackaudio = m_menu->addAction(QIcon(":/images/tools/new-24x24.png"), "New Audio Track");
	m_act_new_trackmarks = m_menu->addAction(QIcon(":/images/tools/new-24x24.png"), "New Marks Track");
	m_act_new_tracktimeline = m_menu->addAction(QIcon(":/images/tools/new-24x24.png"), "New TimeLine");
	m_menu->addSeparator();

	m_act_project_open = m_menu->addAction(QIcon(":/images/tools/open-24x24.png"), "Open Project...");
	m_act_open_audio = m_menu->addAction(QIcon(":/images/tools/open-24x24.png"), "Open Audio");
	m_act_open_marks = m_menu->addAction(QIcon(":/images/tools/open-24x24.png"), "Open Marks");
	m_menu->addSeparator();


	m_act_project_save = m_menu->addAction(QIcon(":/images/tools/save-24x24.png"), "Save Project");
	m_act_project_save_as = m_menu->addAction(QIcon(":/images/tools/save-24x24.png"), "Save Project As...");
	//m_act_save_marks_tracks_as = m_menu->addAction(QIcon(":/images/tools/save-24x24.png"), "Save Marks Tracks As...");
	m_act_project_close = m_menu->addAction("Close Project");
	m_menu->addSeparator();

	m_act_export = m_menu->addAction("Export...");
	m_act_export->setDisabled( true );
	m_menu->addSeparator();

	m_act_project_new_tools_bank = m_menu->addAction("Tools Bank");
	m_menu->addSeparator();


	m_menu->addAction("Exit");
	m_menu->addAction("- close menu -");

	m_act_export->setEnabled( false );


	connect(m_act_project_new,			SIGNAL(triggered()), this, SLOT(slot_act_project_new()));
	connect(m_act_project_new_tools_bank,	SIGNAL(triggered()), this, SLOT(slot_act_project_new_tools_bank()));
	connect(m_act_project_open,			SIGNAL(triggered()), this, SLOT(slot_act_project_open()));
	connect(m_act_project_save,			SIGNAL(triggered()), this, SLOT(slot_act_project_save()));
	connect(m_act_project_save_as,		SIGNAL(triggered()), this, SLOT(slot_act_project_save_as()));
	connect(m_act_project_close,		SIGNAL(triggered()), this, SLOT(slot_act_project_close()));

	connect(m_act_new_trackaudio,		SIGNAL(triggered()), this, SLOT(slot_act_new_trackaudio()));
	connect(m_act_new_trackmarks,		SIGNAL(triggered()), this, SLOT(slot_act_new_trackmarks()));
	connect(m_act_new_tracktimeline,	SIGNAL(triggered()), this, SLOT(slot_act_new_tracktimeline()));
	connect(m_act_open_audio,			SIGNAL(triggered()), this, SLOT(slot_act_open_audio()));
	connect(m_act_open_marks,			SIGNAL(triggered()), this, SLOT(slot_act_open_marks()));

	//connect(m_act_save_marks_tracks_as,	SIGNAL(triggered()), this, SLOT(slot_act_save_marks_tracks_as()));
}

void WidgetMainMenu::projectsExists(bool _flag)
{
	m_act_new_trackaudio->setEnabled( _flag );
	m_act_new_trackmarks->setEnabled( _flag );
	m_act_new_tracktimeline->setEnabled( _flag );
	m_act_project_save->setEnabled( _flag );
	m_act_project_save_as->setEnabled( _flag );
	m_act_project_close->setEnabled( _flag );

	m_act_open_marks->setEnabled( _flag );
	//->setEnabled( _flag );
	//->setEnabled( _flag );
}




WindowMain::WindowMain ( Controller *_ctrl )
	: m_ctrl( _ctrl )
	, m_tabs( new WidgetHeaders( this ) )
	, m_buttonmenu( new QPushButton( this ) )
	, m_toolbar( new ToolBar::WidgetToolBar( this, _ctrl->getSettings() ) )
	, m_main_menu( new WidgetMainMenu( this ))
	, m_statusbar( 0 )
	, m_label_freq( 0 )
	, m_label_position( 0 )
	, m_label_range( 0 )
	, m_label_range_len( 0 )
	, m_play_starttime( 0 )
	, m_play_from_line( 0 )
	, m_insert( true )
	, m_panelsheight( 0 )
	, m_statusheight( 0 )
	, m_scroll_pressed( 0 )
	, m_scroll_energy( 0 )
	, m_scroll_cnt( 0 )
	, m_pm ( this, _ctrl )
{
	QPalette pal = QApplication::palette();
	pal.setColor( QPalette::Window, QColor( 0xee, 0xee, 0xee, 0xff ) );
	QApplication::setPalette( pal );
	this->setWindowTitle( "Audigger " __DATE__);

	// Read geomerty settings.
	Settings::SettingsGui &s = m_ctrl->getSettings().getGui();
	resize( s.m_width, s.m_height );
	move( s.m_x, s.m_y );

	m_tabs->setSelectable( true );
	m_tabs->setTabLook( true );
	m_tabs->setHasButtonAdd( true );

	m_buttonmenu->setFocusPolicy( Qt::NoFocus );
	m_buttonmenu->resize(WIDTH_MENUBUTTON, HEIGHT_TABS);
	m_buttonmenu->setFlat( true );
	m_buttonmenu->setIconSize( QSize(24, 24) );
	m_buttonmenu->setIcon( QIcon(":/images/menu-3bars-24x24.png") );
	m_buttonmenu->show();

	m_timer_animation.setSingleShot( false );
	m_timer_animation.setInterval( 1000.0/61.0 );

	m_timer_animation.start();
	m_tabs->setAnimated( true );


	actualizeGui();
	//buildStatusBar__();

	// C O N N E C T
	connect(m_buttonmenu, SIGNAL(clicked()), m_main_menu, SLOT(slotExec()));
	connect(m_tabs, SIGNAL(signalSelected(unsigned)), this, SLOT(slotProjectCurrentChanged(unsigned)));
	connect(m_tabs, SIGNAL(signalMove(unsigned,unsigned)), this, SLOT(slotProjectsMove(unsigned,unsigned)));
	connect(m_tabs, SIGNAL(signalClose(unsigned)), this, SLOT(slotProjectCloseRequested(unsigned)));

	connect(m_toolbar, SIGNAL( signalAction(ToolBar::ToolType, unsigned)), this, SLOT( slotToolBarAction(ToolBar::ToolType, unsigned)));
	connect(&m_timer_animation, SIGNAL(timeout()), this, SLOT(slotTimerAnimation()));
	connect(&m_timer_animation, SIGNAL(timeout()), m_tabs, SLOT( animate()));


	connect(m_main_menu, SIGNAL(signalProjectNew()),			this, SLOT(slotProjectNew()));
	connect(m_main_menu, SIGNAL(signalProjectNewToolsBank()),	this, SLOT(slotProjectNewToolsBank()));
	connect(m_main_menu, SIGNAL(signalProjectOpen()),			this, SLOT(slotProjectOpen()));
	connect(m_main_menu, SIGNAL(signalProjectSave()),			this, SLOT(slotProjectSave()));
	connect(m_main_menu, SIGNAL(signalProjectSaveAs()),			this, SLOT(slotProjectSaveAs()));
	connect(m_main_menu, SIGNAL(signalProjectClose()),			this, SLOT(slotProjectClose()));

	connect(m_main_menu, SIGNAL(signalTrackMarksNew()),			this, SLOT(slotTrackMarksNew()));
	connect(m_main_menu, SIGNAL(signalTrackAudioNew()),			this, SLOT(slotTrackAudioNew()));
	connect(m_main_menu, SIGNAL(signalTrackTimeLineNew()),		this, SLOT(slotTrackTimeLineNew()));
	connect(m_main_menu, SIGNAL(signalTrackAudioOpen()),		this, SLOT(slotTrackAudioOpen()));
	connect(m_main_menu, SIGNAL(signalTrackMarksOpen()),		this, SLOT(slotTrackMarksOpen()));

	connect(m_main_menu, SIGNAL(signalTrackMarksSaveAllAs()),	this, SLOT(slotTrackMarksSaveAllAs()));


	updateLayout();
	updateSize();
}

WindowMain::~WindowMain()
{
	Settings::SettingsGui &s = m_ctrl->getSettings().getGui();
	s.m_width = (width() > 100)?width():DEFAULT_GUI_WIDTH;
	s.m_height = height();
	s.m_x = pos().x();
	s.m_y = pos().y();
	s.m_fullscreen = isFullScreen();

	if ( width() < 100 || height() < 100 )
		m_ctrl->getSettings().getGui().clear();
}


//
// P R I V A T E
//

void WindowMain::paintEvent(QPaintEvent *_event)
{

}

void WindowMain::wheelEvent(QWheelEvent *_event)
{
	if ( Qt::ControlModifier == _event->modifiers() )
	{
		if ( ! m_pm.getCurrWTG() )
			return;

		// ZOOM
		if ( _event->delta() < 0 )
			m_toolbar->dialUp( ToolBar::TOOL_TIMESTEP );
		else
			m_toolbar->dialDown( ToolBar::TOOL_TIMESTEP );

		m_pm.getCurrWTG()->setStepFrames( m_toolbar->getToolState( ToolBar::TOOL_TIMESTEP ) );
	}
	else
	{
		if ( m_pm.getCurrWTG() )
			m_scroll_energy += (_event->delta()/120) * WHEELRATE;
	}
}

void WindowMain::resizeEvent(QResizeEvent *_event)
{
	QWidget::resizeEvent( _event );

	updateSize();
}

void WindowMain::keyPressEvent ( QKeyEvent * _event )
{
	switch ( _event->key() )
	{
	default:
		break;

	case Qt::Key_Q:
		if ( Qt::ControlModifier == _event->modifiers())
		{
			// Crash
			exit(0);
			volatile char *v = NULL;
			for ( int i = 0; i < 1024*1024; ++i )
				++ v[i];
		}
		break;

	case Qt::Key_Space:
		sndStartStopSwitch__();
		break;

	case Qt::Key_C:
		if ( Qt::ControlModifier == _event->modifiers())
			slotCopy();
		break;
	case Qt::Key_V:
		if ( Qt::ControlModifier == _event->modifiers())
			slotPaste();
		break;
	case Qt::Key_X:
		if ( Qt::ControlModifier == _event->modifiers())
			slotCut();
		break;
	case Qt::Key_Delete:
			slotDelete();
		break;

	case Qt::Key_A:
		if ( Qt::ControlModifier == _event->modifiers())
		{
			// select all
		}
		break;
	case Qt::Key_Insert:
		//slotButtonInsert();
		break;

	case Qt::Key_End:
		// Shift? (selection)
		if ( m_pm.getCurrWTG() )
		{
			WidgetTrackAbstract::TrackPos pos = m_pm.getCurrWTG()->getLenPixels();
			m_pm.getCurrWTG()->setCursorPos( pos );
			//WidgetTrackAbstract::TrackPos off =
			m_pm.getCurrWTG()->offsetForPos( pos );
			m_pm.getCurrWTG()->setOffset( pos );
			m_pm.getCurrWTG()->refresh();
		}
		break;

	case Qt::Key_Home:
		if ( m_pm.getCurrWTG() )
		{
			m_pm.getCurrWTG()->setCursorPos(0);
			m_pm.getCurrWTG()->setOffset(0);
			m_pm.getCurrWTG()->refresh();
		}
		break;

	case Qt::Key_PageUp:
		if ( m_pm.getCurrWTG() )
		{
			m_pm.getCurrWTG()->moveCursor( - height() * 1.3 );
			m_pm.getCurrWTG()->refresh();
		}
		break;

	case Qt::Key_PageDown:
		if ( m_pm.getCurrWTG() )
		{
			m_pm.getCurrWTG()->moveCursor( height() * 1.3 );
			m_pm.getCurrWTG()->refresh();
		}
		break;

	case Qt::Key_Up:
		if ( m_pm.getCurrWTG() )
		{
			if ( Qt::ControlModifier == _event->modifiers())
				m_pm.getCurrWTG()->moveCursor( -5 );
			else
				m_pm.getCurrWTG()->moveCursor( -1 );
		}
		break;

	case Qt::Key_Down:
		if ( m_pm.getCurrWTG() )
		{
			if ( Qt::ControlModifier == _event->modifiers())
				m_pm.getCurrWTG()->moveCursor( 5 );
			else
				m_pm.getCurrWTG()->moveCursor( 1 );
		}
		break;

	case Qt::Key_Z:
		if ( Qt::ControlModifier == _event->modifiers())
			slotButtonUndo();

		break;
	case Qt::Key_Y:
		if ( Qt::ControlModifier == _event->modifiers())
			slotButtonRedo();

		break;
	}
}

void WindowMain::keyReleaseEvent ( QKeyEvent * _event )
{
	if ( _event->isAutoRepeat() )
		return;

	m_scroll_pressed = false;
}

//
// P R I V A T E
//
void WindowMain::actualizeGui()
{
	bool project = (NULL != m_pm.getCurrWTG());

	m_main_menu->projectsExists( project );
	m_toolbar->enableTool(ToolBar::TOOL_SAVE, project );
	m_toolbar->enableTool(ToolBar::TOOL_CLOSE, project );
	m_toolbar->enableTool(ToolBar::TOOL_COPY, project );
	m_toolbar->enableTool(ToolBar::TOOL_PASTE, project );
	m_toolbar->enableTool(ToolBar::TOOL_INSERT, project );
	m_toolbar->enableTool(ToolBar::TOOL_PLAY, project );
	m_toolbar->enableTool(ToolBar::TOOL_STOP, project );
	m_toolbar->enableTool(ToolBar::TOOL_HAND, project );
	m_toolbar->enableTool(ToolBar::TOOL_SELECT_TIME, project );
	m_toolbar->enableTool(ToolBar::TOOL_SELECT_BOTH, project );
	m_toolbar->enableTool(ToolBar::TOOL_BANDPASS, project );
	m_toolbar->enableTool(ToolBar::TOOL_BANDPASS_ZERO, project );
	m_toolbar->enableTool(ToolBar::TOOL_BANDSUPPRESS, project );
	m_toolbar->enableTool(ToolBar::TOOL_PEN, project );
	m_toolbar->enableTool(ToolBar::TOOL_ERASER, project );
	m_toolbar->enableTool(ToolBar::TOOL_TIMESTEP, project );
	m_toolbar->enableTool(ToolBar::TOOL_CURSOR_ASYNC, project );

	// If project of type "ToolBank" selected,
	// then you will not see disabled widgets on ToolBar,
	// because ToolBar display a picture with widgets images.

	m_toolbar->setModeEdit( (bool)m_pm.getCurrWidgetToolBank() );
}

void WindowMain::buildStatusBar__()
{
	if ( m_statusbar )
		return;

	m_statusbar = new QStatusBar ( this );

    m_label_position = new QLabel();
    m_label_range = new QLabel();
    m_label_range_len = new QLabel();
    m_label_freq = new QLabel();


    QFontMetrics fm( m_label_position->font() );
    QRect r = fm.boundingRect("0:00:00:00.000");
    m_label_position->setFixedWidth( r.width() );
    m_label_position->setText("0:00:00:00.000");
    m_label_range_len->setFixedWidth( r.width() );
    m_label_range_len->setText("no selection");

    r = fm.boundingRect("00000 Hz");
    m_label_freq->setFixedWidth( r.width() );
    m_label_freq->setAlignment(Qt::AlignRight);

    r = fm.boundingRect("0:00:00:00.000 - 0:00:00:00.000");
    m_label_range->setFixedWidth( r.width() );
    m_label_range->setText("no selection");

	m_statusbar->insertPermanentWidget( 0, m_label_position );
	m_statusbar->insertPermanentWidget( 1, m_label_range );
	m_statusbar->insertPermanentWidget( 2, m_label_range_len );
	m_statusbar->insertPermanentWidget( 3, m_label_freq );

	m_statusbar->move(0, height() - m_statusbar->height());
}

void WindowMain::updateLayout( )
{
	bool b_toolbar = true;
	bool b_status = (m_statusbar != 0); //m_a_statusbar->isChecked();

	m_panelsheight = 0;
	m_statusheight = 0;

	// INTERVAL_PANELS
	m_buttonmenu->move(0, 0);
	m_tabs->move( WIDTH_MENUBUTTON + SPACE_MENUBUTTON_TABS, 0 );
	m_panelsheight += m_tabs->height();
	m_panelsheight += INTERVAL_PANELS;

	if ( b_toolbar )
	{
		m_toolbar -> move (0, m_panelsheight );
		m_panelsheight += m_toolbar->height();
		m_panelsheight += INTERVAL_PANELS;
	}

	if ( m_statusbar )
		m_statusbar->setVisible( b_status );
	if ( b_status )
		m_statusheight += m_statusbar->height();

	m_pm.move( 0, m_panelsheight );
}

void WindowMain::updateSize( )
{
	m_tabs->resize( width() - (WIDTH_MENUBUTTON + SPACE_MENUBUTTON_TABS), HEIGHT_TABS );
	m_toolbar->resize( width(), HEIGHT_TOOLBAR );

	m_pm.resize( width(), height() - (m_panelsheight + m_statusheight));

	if ( m_statusbar )
	{
		m_statusbar->resize( width(), m_statusbar->height() );
		m_statusbar->move( 0, height() - m_statusbar->height());
	}
}

void WindowMain::sndStartStopSwitch__()
{
	if ( m_toolbar->getToolState( ToolBar::TOOL_STOP ) )
	{
		m_toolbar->setToolState( ToolBar::TOOL_PLAY, 1 );
		slotSndStart();
	}
	else
	{
		m_toolbar->setToolState( ToolBar::TOOL_STOP, 1 );
		slotSndPause();
	}
}

void WindowMain::slotTimerAnimation()
{
	if ( m_scroll_energy != 0 )
		m_pm.scroll ( m_scroll_energy );

	if ( m_scroll_pressed )
	{
		if ( m_scroll_energy > 0 )
			m_scroll_energy += 1;
		else
			m_scroll_energy -= 1;
	}
	else
	{
		if ( m_scroll_energy != 0 )
			m_scroll_energy /= 2;
	}
}

//
// S L O T S
//
void WindowMain::slotToolBarAction(ToolBar::ToolType _type, unsigned _value)
{
	switch(_type)
	{
	default:
		break;

    case ToolBar::TOOL_NEW:
        slotProjectNew();
        break;

    case ToolBar::TOOL_OPEN:
    	slotTrackAudioOpen();
        break;

    case ToolBar::TOOL_SAVE:
    	if ( ! m_pm.getCurrWTG() )
    		break;
    	m_pm.getCurrWTG()->getGroup()->save();
        break;

    case ToolBar::TOOL_CLOSE:
    	slotProjectClose();
        break;

    case ToolBar::TOOL_COPY:
    	slotCopy();
        break;

    case ToolBar::TOOL_PASTE:
    	slotPaste();
        break;

    case ToolBar::TOOL_INSERT:
    	// we keep INSERT value in toolbar.
        break;

	case ToolBar::TOOL_PLAY:
		slotSndStart();
		break;

	case ToolBar::TOOL_STOP:
		slotSndPause();
		break;

    case ToolBar::TOOL_HAND:
    	if ( ! m_pm.getCurrWTG() )
    		break;
    	m_pm.getCurrWTG()->setMouseMode( WidgetTrackAbstract::MM_DRAG );
        break;

    case ToolBar::TOOL_SELECT_TIME:
    	if ( ! m_pm.getCurrWTG() )
    		break;
    	m_pm.getCurrWTG()->setMouseMode( WidgetTrackAbstract::MM_SELECTTIME );
        break;

    case ToolBar::TOOL_SELECT_BOTH:
    	if ( ! m_pm.getCurrWTG() )
    		break;
    	m_pm.getCurrWTG()->setMouseMode( WidgetTrackAbstract::MM_SELECTBOTH );
        break;

    case ToolBar::TOOL_BANDPASS:
        break;

    case ToolBar::TOOL_BANDPASS_ZERO:
        break;

    case ToolBar::TOOL_BANDSUPPRESS:
        break;

    case ToolBar::TOOL_PEN:
        break;

    case ToolBar::TOOL_ERASER:
        break;

    case ToolBar::TOOL_INFO:
    	{
    		WindowAbout wa(this);
    		wa.move( pos().x() + (width()/2 - wa.width()/2),
    				 pos().y() + (height()/2 - wa.height()/2) );
    		wa.exec();
    	}
        break;

    case ToolBar::TOOL_TIMESTEP:
    	if ( m_pm.getCurrWTG() )
    		m_pm.getCurrWTG()->setStepFrames( _value );
        break;
    case ToolBar::TOOL_CURSOR_ASYNC:
		break;
	}
}

void WindowMain::slotProjectCurrentChanged(unsigned _index)
{
	m_pm.changeIndexCurrent( _index );

	// Display cursor/selection parameters for newly switched tab.
	slotVisualsChanged( m_pm.getCurrWTG() );
	actualizeGui();
}

void WindowMain::slotProjectsMove(unsigned _from, unsigned _to)
{
	m_pm.exchange( _from, _to );
}

void WindowMain::slotProjectCloseRequested(unsigned _index)
{
	m_pm.deleteProject( _index );
	m_tabs->delHeader( _index );
	actualizeGui();
}

void WindowMain::slotProjectNew()
{
	if ( m_pm.createProjectMedia() )
	{
		m_tabs->addHeader( "New Project", 128, HEADER_OPT_AUTORESIZE | HEADER_OPT_CLOSEABLE );
		updateLayout();
		updateSize();
	}
	actualizeGui();
}


void WindowMain::slotProjectNewToolsBank()
{
	if ( m_pm.createProjectToolBank() )
	{
		m_tabs->addHeader( "Tools Bank", 140, HEADER_OPT_AUTORESIZE | HEADER_OPT_CLOSEABLE );
		updateLayout();
		updateSize();
	}

	actualizeGui();
}

void WindowMain::slotProjectOpen()
{
	if ( ! m_pm.getCurrWTG() )
		return;

	std::vector<char> filename_tmp;
	if ( ! dialogOpenDir(this, filename_tmp) )
		return;

	try
	{
		m_pm.getCurrWTG()->getGroup()->load( &filename_tmp[0] );
		m_pm.getCurrWTG()->load();
	} catch ( const std::runtime_error &e)
	{
		QMessageBox msg( QMessageBox::Critical, QString("error"), e.what(), QMessageBox::Cancel, this );
		msg.exec();
	}

	actualizeGui();
}

void WindowMain::slotProjectSave()
{
	if ( ! m_pm.getCurrWTG() )
		return;

	try
	{
		m_pm.getCurrWTG()->getGroup()->save();

	} catch ( const std::runtime_error &e)
	{
		QMessageBox msg( QMessageBox::Critical, QString("error"), e.what(), QMessageBox::Cancel, this );
		msg.exec();
	}
}

void WindowMain::slotProjectSaveAs()
{
	if ( ! m_pm.getCurrWTG() )
		return;

	std::vector<char> filename_tmp;
	if ( ! dialogSaveAsDir(this, filename_tmp) )
		return;

	try
	{
		m_pm.getCurrWTG()->getGroup()->save( &filename_tmp[0] );
	} catch ( const std::runtime_error &e)
	{
		QMessageBox msg( QMessageBox::Critical, QString("error"), e.what(), QMessageBox::Cancel, this );
		msg.exec();
	}
}

void WindowMain::slotProjectClose()
{
	m_pm.deleteProjectCurrent();
	actualizeGui();
}

void WindowMain::slotTrackAudioNew()
{
	m_pm.addTrackAudioNew();
	actualizeGui();
}


void WindowMain::slotTrackAudioOpen()
{
	std::vector<char> filename_tmp;
	if ( ! dialogOpenFile(this, DEFAULT_OPEN_PATH_SND, OPENFILES_MEDIA, filename_tmp) )
		return;

	if ( NULL == m_pm.getCurrWTG() )
		slotProjectNew();

	m_pm.addTrackAudio( &filename_tmp[0] );
	actualizeGui();
}

void WindowMain::slotTrackAudioExport()
{
	throw std::string("Not implemented.");
}

void WindowMain::slotTrackMarksNew()
{
	m_pm.addTrackMarksNew();
	actualizeGui();
}

void WindowMain::slotTrackMarksOpen()
{
	std::vector<char> filename_tmp;
	if ( ! dialogOpenFile(this, DEFAULT_OPEN_PATH_SND, OPENFILES_JSON, filename_tmp) )
		return;

	if ( NULL == m_pm.getCurrWTG() )
		slotProjectNew();

	m_pm.addTrackMarks( &filename_tmp[0] );
	actualizeGui();
}

void WindowMain::slotTrackMarksSaveAllAs()
{
	throw std::string("Not implemented.");
}

void WindowMain::slotTrackTimeLineNew()
{
	m_pm.addTrackTimeLine();
}


void WindowMain::slotButtonUndo()
{
	if ( ! m_pm.getCurrWTG() )
		return;

	m_pm.getCurrWTG()->undo();
}

void WindowMain::slotButtonRedo()
{
	if ( ! m_pm.getCurrWTG() )
		return;

	m_pm.getCurrWTG()->redo();
}


void WindowMain::slotCopy()
{
	if ( ! m_pm.getCurrWTG() )
		return;

	m_ctrl->getProcessor()->copy( m_pm.getCurrWTG()->getGroup() );
}

void WindowMain::slotPaste()
{
	if ( ! m_pm.getCurrWTG() )
		return;

	// TODO: insertion flag
	m_ctrl->getProcessor()->paste( m_pm.getCurrWTG()->getGroup(),
								(bool)m_toolbar->getToolState( ToolBar::TOOL_INSERT ) );

	m_pm.getCurrWTG()->refresh();
}

void WindowMain::slotDelete()
{
	if ( ! m_pm.getCurrWTG() )
		return;

	// TODO: insertion flag
	m_ctrl->getProcessor()->erase( m_pm.getCurrWTG()->getGroup() );

	m_pm.getCurrWTG()->refresh();
}

void WindowMain::slotCut()
{
	if ( ! m_pm.getCurrWTG() )
		return;

	// TODO: insertion flag
	m_ctrl->getProcessor()->cut( m_pm.getCurrWTG()->getGroup() );

	m_pm.getCurrWTG()->refresh();
}


void WindowMain::slotApply()
{
	//m_pm.getCurrWTG()->transferSel();

	//if ( m_ctrl->getProcessor()->processDSP( m_pm.getCurrWTG()->m_track_id ) )
		//m_pm.getCurrWTG()->refreshSelTime();
}

void WindowMain::slotInfo()
{
	WindowAbout *wa = new WindowAbout( this );
	wa->move( pos().x() + width()/2 - wa->width()/2, pos().y() + height()/2 -wa->height()/2 );
	wa->exec();

	delete wa;
}

void WindowMain::slotVisualsChanged(WidgetTracksGroup* _wtg)
{

//	m_tmp_SS.str("");
//
//
//	// Get Timeconst... Omg! Cache it!
//	unsigned track_id = _track->getId();
//	float rate = m_ctrl->getProcessor()->getAudioTrack( track_id)->getAudioParams().m_rate;
//	unsigned m_fft_win, m_fft_step;
//	_track->getFFTParams(&m_fft_win, &m_fft_step);
//	float timeconst = ((float)m_fft_step)/(rate/1000.0f);
//
//	size_t sel_from = 0, sel_len = 0;
//	if ( _track->getSelTimeCurr(&sel_from, &sel_len) )
//	{
//		long long selection_from_ms = 0, selection_len_ms = 0;
//		// Convert lines to MS.
//		selection_from_ms = ((float)sel_from)* timeconst;
//		selection_len_ms = ((float)sel_len) * timeconst;
//		printSecondsAsTime( m_tmp_SS, selection_from_ms );
//		m_tmp_SS << " - ";
//		printSecondsAsTime( m_tmp_SS, selection_from_ms + selection_len_ms );
//		m_label_range->setText( m_tmp_SS.str().c_str() );
//
//		m_tmp_SS.clear();
//		m_tmp_SS.str("");
//
//		printSecondsAsTime( m_tmp_SS, selection_len_ms );
//		m_label_range_len->setText( m_tmp_SS.str().c_str() );
//	}
//	else
//	{
//		m_label_range->setText("no selection");
//		m_label_range_len->setText("no selection");
//	}
//
//	// Ask about cursor position.
//	size_t cursor_pos = _track->getCursorPos();
//	long long cursor_pos_ms = ((float)cursor_pos * timeconst);
//	m_tmp_SS.clear();
//	m_tmp_SS.str("");
//	printSecondsAsTime( m_tmp_SS, cursor_pos_ms );
//	m_label_position->setText( m_tmp_SS.str().c_str() );
}




void WindowMain::slotSndStart()
{
	if ( ! m_pm.getCurrWTG() )
		return;

//	TrackAudio::Ptr track = m_pm.getCurrWTG()->getGroup()->getTrackAudioSelectedFirst();
//	if ( ! track )
//		return;

	// TODO: m_playing_wtc
	//m_pm.getCurrWTG() -> sendPosFrames();
	m_ctrl->sndStart( m_pm.getCurrWTG()->getGroup() );

	m_pm.getCurrWTG()->startAnimation();
}

void WindowMain::slotSndPause()
{
	LOG_PURE ( "WindowMain::slotSndStop()" );
	m_ctrl->sndStop();
	m_pm.getCurrWTG()->stopAnimation();
}

void WindowMain::slotStepFrames(int _index)
{
	//LOG_PURE("slotStepFrames(), " << _index);
	if ( ! m_pm.getCurrWTG() )
		return;

	unsigned step = getStepFramesByIndex( _index );
	LOGVAR2 (_index, step);

	if ( ! step )
		return;

	m_pm.getCurrWTG()->setStepFrames( step );
}


//void WindowMain::slotChangedSelection(long long _from_line, long long _len_lines)
//{
//	if ( ! _len_lines )
//	{
//		m_label_range->setText("no selection");
//		m_label_range_len->setText("no selection");
//		return;
//	}
//
//	// Convert lines to MS.
//	std::stringstream SS;
//	long long selection_from_ms, selection_len_ms;
//	//getSelectionMs( &selection_from_ms, &selection_len_ms );
//	selection_from_ms = ((float)_from_line)* m_timeconst;
//	selection_len_ms = ((float)_len_lines) * m_timeconst;
//	printSecondsAsTime( SS, selection_from_ms );
//	SS << "-";
//	printSecondsAsTime( SS, selection_from_ms + selection_len_ms );
//	m_label_range->setText( SS.str().c_str() );
//
//	SS.clear();
//	SS.str("");
//	printSecondsAsTime( SS, selection_len_ms );
//	m_label_range_len->setText( SS.str().c_str() );
//}

//void WindowMain::slotCursorSecond(long long _freq)
//{
//	char freq_txt[16];
//
//	float m_freqconst = ((float)(m_mfi.m_samplerate/2))/((float)m_windowlen/2);
//
//	size_t endpos = fir::itoa( freq_txt, ((float)_freq) * m_freqconst);
//	freq_txt[ endpos ] = 'H';
//	freq_txt[ endpos+1 ] = 'z';
//	freq_txt[ endpos+2 ] = 0;
//
//	m_label_freq -> setText( freq_txt );
//}

