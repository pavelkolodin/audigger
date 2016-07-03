/*
 * pavelkolodin@gmail.com
 */


#include "WidgetToolBar.h"
#include <boost/foreach.hpp>
#include <fir/logger/logger.h>
#include "WidgetDial.h"
#include "stdint.h"
#include "defines.h"

namespace ToolBar
{

extern struct ToolInfo global_toolsinfo[];


WidgetToolBar::WidgetToolBar(QWidget *_parent, Settings::Settings &_settings)
: QWidget (_parent)
, m_settings( _settings )
, m_highlight_coord( -1 )
, m_highlight_index( -1 )
, m_mode_edit ( false )
, m_drg_tool ( const_cast<WidgetToolBar*>(this), DRAGGING_START_DISTANCE, -99, 99 )
{
	setAcceptDrops(true);

	m_info.resize( (size_t) ToolBar::CONTROL_TYPE_LIMIT, NULL );
	m_states.resize( (size_t) ToolBar::CONTROL_TYPE_LIMIT, 0);
	m_enabled.resize( (size_t) ToolBar::CONTROL_TYPE_LIMIT, 1);

	const ToolInfo* b = &global_toolsinfo[0];
	for ( ; b->m_type != CONTROL_TYPE_LIMIT; ++b )
	{
		//LOGCOUT( (int)b->m_type << ", " << b->m_icon_filename << ", " << b->m_icon_width << ", " << b->m_icon_height << "\n" );
		m_info[ (size_t) b->m_type ] = b;
		m_states[ (size_t) b->m_type ] = b->m_default_value;
	}

	Settings::SettingsToolBar &stb = m_settings.getToolBar();
	BOOST_FOREACH( const Settings::SettingsToolBar::LayoutType::value_type &str, stb.m_layout )
	{
		const ToolInfo* ti = findToolInfoByName__( str );
		if ( ! ti )
			continue;

		//LOGVAR3( str, ti->m_type, stb.m_states[ str ] );

		m_layoutplan.push_back( ti );
		setToolState__( ti->m_type, stb.m_states[ str ] );

		//insertTool( ti->m_type );
		//setToolState( ti->m_type, stb.m_states[ str ] );
	}

	rebuildLayer__( );
	applyStatusesToWidgets__();
}

WidgetToolBar::~WidgetToolBar()
{
	saveToolsState__();
	saveLayout__();
}

void WidgetToolBar::clear()
{
	m_layoutplan.clear();
	rebuildLayer__();
	update();
}

bool WidgetToolBar::insertTool(ToolBar::ToolType _type, int _index)
{
	const ToolInfo* ti = findToolInfo__( _type );
	if ( NULL == ti )
		return false;


	if ( _index < 0 )
	{
		m_layoutplan.push_back( ti );
	}
	else
	{
		m_layoutplan.insert( m_layoutplan.begin() + _index, ti );
	}

	rebuildLayer__( );
	applyStatusesToWidgets__();
	return true;
}

void WidgetToolBar::setModeEdit( bool _modeedit )
{
	m_mode_edit = _modeedit;

	std::vector < QWidget *> :: iterator it, ite;
	it = m_layout_widgets.begin();
	ite = m_layout_widgets.end();
	for ( ; it != ite; ++it )
	{
		if ( m_mode_edit )
			(*it)->hide();
		else
			(*it)->show();
	}
}

void WidgetToolBar::setToolState( ToolBar::ToolType _type, unsigned _state )
{
	setToolState__( _type, _state );
	applyStatusesToWidgets__();
}

unsigned WidgetToolBar::getToolState( ToolBar::ToolType _type )
{
	if ( (size_t) _type >= (size_t)CONTROL_TYPE_LIMIT )
		return 0;

	return m_states [ (size_t) _type ];
}

unsigned WidgetToolBar::dialUp( ToolBar::ToolType _type )
{
	const ToolInfo *ti = findToolInfo__( _type );
	if ( ! ti )
		return 0;

	if ( WT_DIAL != ti->m_type_widget )
		return 0;

	switch ( _type )
	{
	default:
		break;
	case TOOL_TIMESTEP:
		if ( m_states[ (size_t)ti->m_type ] < UINT_MAX/2 )
			m_states[ (size_t)ti->m_type ] *= 2;
		break;
	}

	applyStatusesToWidgets__();
	return m_states[ (size_t)ti->m_type ];
}

// Move the state of tool of dial type down.
// \return tool state.
unsigned WidgetToolBar::dialDown( ToolBar::ToolType _type )
{
	const ToolInfo *ti = findToolInfo__( _type );
	if ( ! ti )
		return 0;

	if ( WT_DIAL != ti->m_type_widget )
		return 0;

	switch ( _type )
	{
	default:
		break;
	case TOOL_TIMESTEP:
		if ( m_states[ (size_t)ti->m_type ] >= 2 )
			m_states[ (size_t)ti->m_type ] /= 2;
		break;
	}
	applyStatusesToWidgets__();
	return m_states[ (size_t)ti->m_type ];
}

void WidgetToolBar::enableTool( ToolBar::ToolType _type, bool _enabled )
{
	m_enabled[ (size_t)_type ] = _enabled;
	enableWidgets__();

	applyStatusesToWidgets__();
}

//
// PROTECTED
//

void WidgetToolBar::paintEvent(QPaintEvent *_event)
{
	if ( ! m_mode_edit )
		return;

	QPainter pnt(this);

	unsigned offset = 0;
	size_t len = m_layout_pic.size();
	for ( size_t i = 0; i < len; ++i )
	{
		pnt.drawPixmap( offset, 0, m_layout_pic[i] );
		offset += m_layout_pic[i].width();
	}

	if ( -1 != m_highlight_coord )
	{
		pnt.setPen( QColor( 0x00, 0x00, 0x00, 0xff ) );
		pnt.drawLine( m_highlight_coord, 0, m_highlight_coord, height()-1 );
		pnt.drawLine( m_highlight_coord+1, 0, m_highlight_coord+1, height()-1 );
	}
}


void WidgetToolBar::resizeEvent(QResizeEvent *_event)
{
	//deployWidgets(m_widgets, width(), height(), 3 );
}

void WidgetToolBar::mousePressEvent ( QMouseEvent *_event )
{
	int coord_offset;
	int index = whosePlace__( _event->x(), &coord_offset );

	if ( -1 == index )
		return;

	m_drg_tool.start( &WidgetToolBar::dragTool, index, _event->x(), 0 );
}

void WidgetToolBar::mouseMoveEvent ( QMouseEvent *_event )
{
	m_drg_tool.update( _event->x() );
}

void WidgetToolBar::mouseReleaseEvent( QMouseEvent *_event )
{
	m_drg_tool.stop( ); //&WidgetTrackMarks::dragToolFinish );
}

void WidgetToolBar::dragEnterEvent ( QDragEnterEvent *_event )
{
	if ( _event->mimeData()->hasFormat( "application/x-marker-tool" ) )
		_event->acceptProposedAction();
}

void WidgetToolBar::dragMoveEvent ( QDragMoveEvent *_event )
{
	m_highlight_index = whosePlace__( _event->pos().x(), &m_highlight_coord );
	update();
}

void WidgetToolBar::dragLeaveEvent ( QDragLeaveEvent * )
{
	m_highlight_coord = -1;
	update();
}

void WidgetToolBar::dropEvent ( QDropEvent *_event )
{
	if ( sizeof(uint32_t) != _event->mimeData()->data( "application/x-marker-tool" ).size() )
		return;

	const char *data = _event->mimeData()->data( "application/x-marker-tool" ).data();
	//LOGCOUT ( _event->mimeData()->data( "application/x-marker-tool" ).size() );
	uint32_t tool_index = *((uint32_t*)data);

	insertTool( (ToolBar::ToolType) tool_index, m_highlight_index );

	_event->acceptProposedAction();

	m_highlight_coord = -1;
	m_highlight_index = -1;
	update();
}

void WidgetToolBar::dragTool(int _index, int _coord)
{
	m_drg_tool.stop( );

    QMimeData *mimeData = new QMimeData;
    QByteArray b_array_toolcode;
	uint32_t code = (uint32_t) m_layoutplan[ _index ]->m_type;
	b_array_toolcode.append( (const char*)&code, sizeof(uint32_t) );
	mimeData->setData( "application/x-marker-tool", b_array_toolcode );

    QDrag *drag = new QDrag(this);
    drag->setPixmap( QPixmap(m_layoutplan[ _index ]->m_icon_filename) );
    drag->setMimeData(mimeData);

    // delete plan item
	m_layoutplan.erase( m_layoutplan.begin() + _index );
	rebuildLayer__();
	update();

    //Qt::DropAction dropAction =
	drag->exec(Qt::CopyAction | Qt::MoveAction);
}

void WidgetToolBar::dragToolFinish(int _index)
{

}

//
// PRIVATE
//

const ToolInfo* WidgetToolBar::findToolInfo__( ToolType _type )
{
	if ( (size_t) _type >= m_info.size() )
		return NULL;

	return m_info[ (size_t) _type ];
}

const ToolInfo* WidgetToolBar::findToolInfoByName__( const std::string &_name )
{
	//std::string needle ( _name );
	BOOST_FOREACH( VectorToolInfo::value_type &_ti, m_info )
	{
		if ( ! _name.compare( _ti->m_settings_name )  )
			return _ti;
	}

	return NULL;
}

void WidgetToolBar::rebuildLayer__()
{
	//saveSettings__();

	size_t len_plan = m_layoutplan.size();

	m_layout_pic.resize( len_plan );
	for ( size_t i = 0; i < len_plan; ++i )
	{
		//m_layout_pic.push_back( QPixmap );
		m_layout_pic[i].load( m_layoutplan[i]->m_icon_filename );
	}

	// delete all widgets
	size_t len = m_layout_widgets.size();
	for ( size_t i = 0; i < len; ++i )
		delete m_layout_widgets[i];

	m_layout_widgets.resize( len_plan );
	int width_offset = 0;
	for ( size_t i = 0; i < len_plan; ++i )
	{
		if ( ToolBar::WT_BUTTON == m_layoutplan[i]->m_type_widget )
		{
			QPushButton *b = new QPushButton(this);
			b->setFocusPolicy( Qt::NoFocus );
			b->setFlat( true );
			b->resize( m_layoutplan[i]->m_icon_width, m_layoutplan[i]->m_icon_height );
			b->move( width_offset, 0 );
			b->setIconSize( QSize(m_layoutplan[i]->m_icon_width, m_layoutplan[i]->m_icon_height) );
			b->setIcon( QIcon( m_layoutplan[i]->m_icon_filename ) );
			connectQObject( b, m_layoutplan[i]->m_type );
			m_layout_widgets[i] = b;

			if ( m_mode_edit )
				b->hide();
			else
				b->show();
		}
		else
		if ( ToolBar::WT_DIAL == m_layoutplan[i]->m_type_widget )
		{
			WidgetDial *obj = new WidgetDial(this);
			obj->setFocusPolicy( Qt::NoFocus );
			obj->move( width_offset, 0 );
			obj->resize(m_layoutplan[i]->m_icon_width, m_layoutplan[i]->m_icon_height);
			connectQObject( obj, m_layoutplan[i]->m_type );
			m_layout_widgets[i] = obj;

			if ( m_mode_edit )
				obj->hide();
			else
				obj->show();
		}


		width_offset += m_layoutplan[i]->m_icon_width;
	}

	m_highlight_coord = -1;
	m_highlight_index = -1;
	//update();

	enableWidgets__();

	LOG_PURE( "create..." );
}

void WidgetToolBar::enableWidgets__()
{
	for ( size_t i = 0; i < m_layoutplan.size(); ++i )
	{
		m_layout_widgets[ i ]->setEnabled(
				m_enabled[(size_t)m_layoutplan[i]->m_type]
				          );
	}
}

int WidgetToolBar::whosePlace__( unsigned _coord, int *_offset )
{
	unsigned offset = 0;
	size_t len = m_layoutplan.size();
	for ( size_t i = 0; i < len; ++i )
	{
		if ( _coord >= offset && _coord < (offset + m_layoutplan[i]->m_icon_width) )
		{
			*_offset = offset;
			return i;
		}

		offset += m_layoutplan[i]->m_icon_width;
	}

	*_offset = offset;
	return -1;
}

bool WidgetToolBar::toolClicked__( ToolBar::ToolType _type )
{
	// Change state of this tool and all dependent tools (groups, etc...)
	bool ret = setToolState__( _type, (unsigned)!(bool)m_states[ (size_t) _type ] );

	// Change visual state of tools.
	applyStatusesToWidgets__();

	saveToolsState__();

	return ret;
}


bool WidgetToolBar::setToolState__( ToolBar::ToolType _type, unsigned _state )
{
	if ( getToolState(_type) == _state )
		return false;

	const ToolInfo *ti = findToolInfo__( _type );

	if ( WT_DIAL == ti->m_type_widget )
	{
		m_states[ (size_t)ti->m_type ] = _state;
	}
	else
	if ( ti->m_checkable )
	{
		// m_group_oneof must not be true, if m_group_id is null.
		LOGVAR2( _state,  ti->m_group_oneof );

		if ( (0 == _state) && ti->m_group_oneof )
			return false;

		if ( ti->m_group_id )
		{
			BOOST_FOREACH( const ToolInfo* cti, m_info )
			{
				if ( cti->m_group_id == ti->m_group_id )
					m_states[ (size_t)cti->m_type ] = 0;
			}
		}

		m_states[ (size_t)ti->m_type ] = _state;
		return true;
	}

	// Not checkable tool: we changed the state, but it disappeared immediately,
	// because the nature of tool. :)
	// Used to decide that signal must be sent.
	return true;
}

void WidgetToolBar::applyStatusesToWidgets__( )
{
	size_t len = m_layoutplan.size();

	for( size_t i = 0; i < len; ++i )
	{
		const ToolInfo* cti = m_layoutplan[i];

		if ( WT_BUTTON == cti->m_type_widget )
		{
			if ( m_states[ (size_t) cti->m_type ] )
				((QPushButton*)m_layout_widgets[ i ]) -> setDown( true );
			else
				((QPushButton*)m_layout_widgets[ i ]) -> setDown( false );

		}
		else
		if ( WT_DIAL == cti->m_type_widget )
		{
			((WidgetDial*)m_layout_widgets[ i ])->setValue( m_states[ (size_t) cti->m_type ] );
		}
	}
}




void WidgetToolBar::saveToolsState__( )
{
	// Save tool's state
	Settings::SettingsToolBar &stb = m_settings.getToolBar();
	BOOST_FOREACH( const ToolInfo *cti, m_layoutplan )
	{
		stb.m_states[ cti->m_settings_name ] = m_states[ (size_t) cti->m_type ];
	}
}

void WidgetToolBar::saveLayout__( )
{
	// Save layout
	// Save each tool's state
	Settings::SettingsToolBar &stb = m_settings.getToolBar();
	stb.m_layout.clear();
	BOOST_FOREACH( const ToolInfo *cti, m_layoutplan )
	{
		stb.m_layout.push_back( cti->m_settings_name );
	}
}


// SLOTS

void WidgetToolBar::slot_TOOL_NEW()
{
    if ( toolClicked__( TOOL_NEW ) )
    	signalAction( TOOL_NEW, 1 );
}
void WidgetToolBar::slot_TOOL_OPEN()
{
    if ( toolClicked__( TOOL_OPEN ) )
        signalAction( TOOL_OPEN, m_states[ (size_t)TOOL_OPEN ] );
}
void WidgetToolBar::slot_TOOL_SAVE()
{
	if ( toolClicked__( TOOL_SAVE ) )
        signalAction( TOOL_SAVE, m_states[ (size_t)TOOL_SAVE ] );
}
void WidgetToolBar::slot_TOOL_CLOSE()
{
	if ( toolClicked__( TOOL_CLOSE ) )
        signalAction( TOOL_CLOSE, m_states[ (size_t)TOOL_CLOSE ] );
}
void WidgetToolBar::slot_TOOL_COPY()
{
	if ( toolClicked__( TOOL_COPY ) )
        signalAction( TOOL_COPY, m_states[ (size_t)TOOL_COPY ] );
}
void WidgetToolBar::slot_TOOL_PASTE()
{
	if ( toolClicked__( TOOL_PASTE ) )
        signalAction( TOOL_PASTE, m_states[ (size_t)TOOL_PASTE ] );
}
void WidgetToolBar::slot_TOOL_INSERT()
{
	if ( toolClicked__( TOOL_INSERT ) )
        signalAction( TOOL_INSERT, m_states[ (size_t)TOOL_INSERT ] );
}
void WidgetToolBar::slot_TOOL_PLAY()
{
	if ( toolClicked__( TOOL_PLAY ) )
        signalAction( TOOL_PLAY, m_states[ (size_t)TOOL_PLAY ] );
}
void WidgetToolBar::slot_TOOL_STOP()
{
	if ( toolClicked__( TOOL_STOP ) )
        signalAction( TOOL_STOP, m_states[ (size_t)TOOL_STOP ] );
}
void WidgetToolBar::slot_TOOL_INFO()
{
	if ( toolClicked__( TOOL_INFO ) )
        signalAction( TOOL_INFO, m_states[ (size_t)TOOL_INFO ] );
}
void WidgetToolBar::slot_TOOL_HAND()
{
	if ( toolClicked__( TOOL_HAND ) )
        signalAction( TOOL_HAND, m_states[ (size_t)TOOL_HAND ] );
}
void WidgetToolBar::slot_TOOL_SELECT_TIME()
{
	if ( toolClicked__( TOOL_SELECT_TIME ) )
        signalAction( TOOL_SELECT_TIME, m_states[ (size_t)TOOL_SELECT_TIME ] );
}
void WidgetToolBar::slot_TOOL_SELECT_BOTH()
{
	if ( toolClicked__( TOOL_SELECT_BOTH ) )
        signalAction( TOOL_SELECT_BOTH, m_states[ (size_t)TOOL_SELECT_BOTH ] );
}
void WidgetToolBar::slot_TOOL_BANDPASS()
{
	if ( toolClicked__( TOOL_BANDPASS ) )
        signalAction( TOOL_BANDPASS, m_states[ (size_t)TOOL_BANDPASS ] );
}
void WidgetToolBar::slot_TOOL_BANDPASS_ZERO()
{
	if ( toolClicked__( TOOL_BANDPASS_ZERO ) )
        signalAction( TOOL_BANDPASS_ZERO, m_states[ (size_t)TOOL_BANDPASS_ZERO ] );
}
void WidgetToolBar::slot_TOOL_BANDSUPPRESS()
{
	if ( toolClicked__( TOOL_BANDSUPPRESS ) )
        signalAction( TOOL_BANDSUPPRESS, m_states[ (size_t)TOOL_BANDSUPPRESS ] );
}
void WidgetToolBar::slot_TOOL_PEN()
{
	if ( toolClicked__( TOOL_PEN ) )
        signalAction( TOOL_PEN, m_states[ (size_t)TOOL_PEN ] );
}
void WidgetToolBar::slot_TOOL_ERASER()
{
	if ( toolClicked__( TOOL_ERASER ) )
        signalAction( TOOL_ERASER, m_states[ (size_t)TOOL_ERASER ] );
}

void WidgetToolBar::slot_TOOL_TIMESTEP(unsigned _value)
{
	m_states[ (size_t) ToolBar::TOOL_TIMESTEP ] = _value;
	signalAction( ToolBar::TOOL_TIMESTEP, _value );
}

void WidgetToolBar::slot_TOOL_CURSOR_SYNC()
{
	if ( toolClicked__( TOOL_CURSOR_ASYNC ) )
        signalAction( TOOL_CURSOR_ASYNC, m_states[ (size_t)TOOL_CURSOR_ASYNC ] );
}


void WidgetToolBar::connectQObject(QObject *_b, ToolType _t)
{
	switch ( _t )
	{
	default:
		break;
	case TOOL_NEW:
		connect( _b, SIGNAL( clicked() ), this, SLOT( slot_TOOL_NEW() ) );
		break;
	case TOOL_OPEN:
		connect( _b, SIGNAL( clicked() ), this, SLOT( slot_TOOL_OPEN() ) );
		break;		
	case TOOL_SAVE:
		connect( _b, SIGNAL( clicked() ), this, SLOT( slot_TOOL_SAVE() ) );
		break;		
	case TOOL_CLOSE:
		connect( _b, SIGNAL( clicked() ), this, SLOT( slot_TOOL_CLOSE() ) );
		break;		
	case TOOL_COPY:
		connect( _b, SIGNAL( clicked() ), this, SLOT( slot_TOOL_COPY() ) );
		break;		
	case TOOL_PASTE:
		connect( _b, SIGNAL( clicked() ), this, SLOT( slot_TOOL_PASTE() ) );
		break;		
	case TOOL_INSERT:
		connect( _b, SIGNAL( clicked() ), this, SLOT( slot_TOOL_INSERT() ) );
		break;		
	case TOOL_PLAY:
		connect( _b, SIGNAL( clicked() ), this, SLOT( slot_TOOL_PLAY() ) );
		break;		
	case TOOL_STOP:
		connect( _b, SIGNAL( clicked() ), this, SLOT( slot_TOOL_STOP() ) );
		break;		
	case TOOL_HAND:
		connect( _b, SIGNAL( clicked() ), this, SLOT( slot_TOOL_HAND() ) );
		break;		
	case TOOL_SELECT_TIME:
		connect( _b, SIGNAL( clicked() ), this, SLOT( slot_TOOL_SELECT_TIME() ) );
		break;		
	case TOOL_SELECT_BOTH:
		connect( _b, SIGNAL( clicked() ), this, SLOT( slot_TOOL_SELECT_BOTH() ) );
		break;		
	case TOOL_BANDPASS:
		connect( _b, SIGNAL( clicked() ), this, SLOT( slot_TOOL_BANDPASS() ) );
		break;		
	case TOOL_BANDPASS_ZERO:
		connect( _b, SIGNAL( clicked() ), this, SLOT( slot_TOOL_BANDPASS_ZERO() ) );
		break;		
	case TOOL_BANDSUPPRESS:
		connect( _b, SIGNAL( clicked() ), this, SLOT( slot_TOOL_BANDSUPPRESS() ) );
		break;		
	case TOOL_PEN:
		connect( _b, SIGNAL( clicked() ), this, SLOT( slot_TOOL_PEN() ) );
		break;		
	case TOOL_ERASER:
		connect( _b, SIGNAL( clicked() ), this, SLOT( slot_TOOL_ERASER() ) );
		break;
	case TOOL_INFO:
		connect( _b, SIGNAL( clicked() ), this, SLOT( slot_TOOL_INFO() ) );
		break;
	case TOOL_TIMESTEP:
		connect( _b, SIGNAL( valueChanged(unsigned)), this, SLOT( slot_TOOL_TIMESTEP(unsigned)));
		break;

	case TOOL_CURSOR_ASYNC:
		connect( _b, SIGNAL( clicked() ), this, SLOT( slot_TOOL_CURSOR_SYNC() ) );
		break;

	}
}


}
