/*
 *  pavelkolodin@gmail.com
 */

#include "WidgetHeaders.h"
#include <iostream>
#include <fir/logger/logger.h>
#include <boost/foreach.hpp>
#include "defines.h"

#define TIMER_TABS_ANIMATION 1000/25
#define HEADER_BORDER_HALFWIDTH 4
#define HEADER_SIZE_MIN 30
#define SIZE_BUTTONADD 24
#define WIDTH_FFTWIDGET 71

//#define QCOLOR_SELECTED QColor( 0x92, 0xc0, 0xff, 0xff )
#define QCOLOR_SELECTED QColor(0x7c,0xaa, 0xff, 0xff) //QColor( 0xee, 0xee, 0xee, 0xff )
#define QCOLOR_ORDINARY QColor( 0xcc, 0xcc, 0xcc, 0xff )
#define ORDINARY_SMALLER 4


#define FILENAME_ICON_CLOSE		":/images/close-bold-sharp-8x8-14x14.png"
//#define FILENAME_ICON_CLOSE_HL	":/images/close-bold-sharp-bg-d93b3b-8x8-14x14.png"
#define FILENAME_CIRCLE_RED		":/images/circle-d93b3b-14x14.png"

#define HEADER_MARGIN_RIGHT 8
#define HEADER_WIDGETS_SPACING 4



WidgetLineEdit::WidgetLineEdit(QWidget *_p)
	: QLineEdit ( _p )
{

}

void WidgetLineEdit::focusOutEvent ( QFocusEvent *_event )
{
	editingFinished();
}

void WidgetLineEdit::keyPressEvent ( QKeyEvent *_event )
{
//	LOGCOUT("enter   " << (Qt::Key_Enter == _event->key()) );
//	LOGCOUT("return   " << (Qt::Key_Return == _event->key()) );
//	LOGCOUT("esc     " << (Qt::Key_Escape == _event->key()) );

	if ( Qt::Key_Escape == _event->key() || Qt::Key_Enter == _event->key() || Qt::Key_Return == _event->key())
	{
		clearFocus();
		editingFinished();
	}
	else
		QLineEdit::keyPressEvent( _event );
}






WidgetCloseButton::WidgetCloseButton( QWidget *_parent )
: QWidget ( _parent )
, m_id(0)
, m_pixmap_close ( new QPixmap( ) )
, m_pixmap_circle_red ( new QPixmap( ) )
, m_under_mouse(0)
{
	// tracking = false - mouseMove only when mouse is pressed
	// we need mousemove, because Qt doesn't send leaveEvent when mouse is pressed! :(
	//setMouseTracking( true );
	setFocusPolicy(Qt::NoFocus);
	setContextMenuPolicy(Qt::NoContextMenu);

	m_pixmap_close->convertFromImage( QImage( FILENAME_ICON_CLOSE ));
	m_pixmap_circle_red->convertFromImage( QImage( FILENAME_CIRCLE_RED ));

	resize( m_pixmap_close->size() );
}

WidgetCloseButton::~WidgetCloseButton()
{

}


void WidgetCloseButton::enterEvent(QEvent *_event)
{
	m_under_mouse = true;
	QWidget::enterEvent(_event);
	update();
}

void WidgetCloseButton::leaveEvent(QEvent *_event)
{
	m_under_mouse = false;
	update();
}

void WidgetCloseButton::mouseMoveEvent(QMouseEvent *_event)
{
	if ( _event->x() < 0 || _event->x() >= width() ||
			_event->y() < 0 || _event->y() >= height() )
		if ( m_under_mouse )
		{
			m_under_mouse = false;
			update();
		}
}

void WidgetCloseButton::mousePressEvent(QMouseEvent *_event)
{
	// just catch event
}

void WidgetCloseButton::mouseReleaseEvent(QMouseEvent *_event)
{
	if ( _event->x() < 0 || _event->x() >= width() ||
			_event->y() < 0 || _event->y() >= height() )
		return;

	clicked( m_id );
}


void WidgetCloseButton::paintEvent(QPaintEvent *)
{
	QPainter pnt(this);
//	int x = m_headers[ idx ]->m_pos + m_headers[ idx ]->m_width - (m_pixmap_close->width() + BUTTON_CLOSE_MARGIN_RIGHT);
//	int y = (height() - m_pixmap_close->height()) / 2;
//	y += _height_offset;

	if ( m_under_mouse )
		pnt.drawPixmap(0, 0, *m_pixmap_circle_red);

	pnt.drawPixmap(0, 0, *m_pixmap_close);
}



TrackHeaderInfo::TrackHeaderInfo( QWidget *_parent, const QString &_text, int _options )
	: m_parent( _parent )
	, m_text ( _text )
	//, m_index( 0 )
	, m_width ( 0 )
	, m_height ( 0 )
	, m_pos_goal(0)
	, m_pos(0)
	, m_text_x(8)
	, m_text_y(0)
	, m_selected(false)
	, m_options( _options )
{
	if ( HEADER_OPT_CLOSEABLE & _options )
	{
		m_close_button.reset( new WidgetCloseButton( _parent ) );
		//m_close_button->show();
		addWidget( m_close_button.get() );
		addWidget( new WidgetSpace( _parent, 5 ) );
	}

	updateWidgetsPositions( );
}

TrackHeaderInfo::~TrackHeaderInfo()
{
	for ( size_t i = 0; i < m_widgets.size(); ++i )
	{
		if ( m_close_button.get() != m_widgets[i] )
			delete m_widgets[i];
	}
}

void TrackHeaderInfo::textSize()
{
	QFontMetrics fm( QApplication::font() );
	QRect r = fm.boundingRect( 0, 0, m_width, m_height, Qt::AlignLeft | Qt::TextSingleLine, m_text );
	m_text_y = (m_height - r.height()) / 2;
}

void TrackHeaderInfo::rename( const QString &_text )
{
	m_text = _text;
	textSize();
}

void TrackHeaderInfo::resize( unsigned _width, unsigned _height )
{
	m_width = _width;
	m_height = _height;

	updateWidgetsPositions();

	// text size calculated after updating widgets positions,
	// because widgets affect text size.
	textSize();
}

void TrackHeaderInfo::setIndex(unsigned _index)
{
	if ( m_close_button )
		m_close_button->m_id = _index;
}

// return: true - animation is done, false - animation will be continued
bool TrackHeaderInfo::animate()
{
	if ( m_pos == m_pos_goal )
		return false;

	int sum = m_pos_goal - m_pos;
	if ( ! (-1 == sum || 1 == sum)  )
		sum /= 2;

	m_pos += sum;

	updateWidgetsPositions();

	return true;
}

// go to last animation frame
void TrackHeaderInfo::finishAnimation()
{
	m_pos = m_pos_goal;
	updateWidgetsPositions();
}

void TrackHeaderInfo::addWidget(QWidget *_w)
{
	if ( ! _w )
		return;

	//m_widgets.insert( m_widgets.begin(), _w );
	m_widgets.push_back( _w );
	_w->setParent( m_parent );
	_w->show();
	updateWidgetsPositions();
}

void TrackHeaderInfo::updateWidgetsPositions()
{
	unsigned margin = 0;
	margin += HEADER_MARGIN_RIGHT;

	bool hiding = false;
	BOOST_FOREACH(QWidget* w, m_widgets)
	{
		if ( hiding || (w->width() + margin > m_width) )
		{
			w->hide();
			hiding = true;
		}
		else
		{
			w->show();
			w->move(m_pos + ( m_width - (w->width() + margin) ), std::max<int>( (((int)m_height) - w->width()), 0 ) / 2 );
			margin += w->width();
			margin += HEADER_WIDGETS_SPACING;
		}
	}

	if ( m_width >= (margin + m_text_x) )
		m_text_width = m_width - (margin + m_text_x);
	else
		m_text_width = 0;
}







WidgetHeaders::WidgetHeaders( QWidget *_parent )
	: QWidget ( _parent )
	, m_drg_header( const_cast<WidgetHeaders*>(this), DRAGGING_START_DISTANCE, 0, 9999 )
	, m_drg_border( const_cast<WidgetHeaders*>(this), DRAGGING_START_DISTANCE, 0, 9999 )
	, m_index_pressed( -1 )
	, m_rename_edit( 0 )
	, m_index_editing(  -1 )
	, m_buttonadd( 0 )
	, m_has_button_add(0)
	, m_tab_look(0)
	, m_fit_screen(0)
	, m_selectable(0)
	, m_animated(0)
{
	setMouseTracking( true );

	m_rename_edit = new WidgetLineEdit ( this );
	m_rename_edit->setFrame( false );
	m_rename_edit->setFocusPolicy(Qt::NoFocus);
	m_rename_edit->hide();

	m_buttonadd = new QPushButton( this );
	m_buttonadd->setFlat( true );
	m_buttonadd->setFocusPolicy( Qt::NoFocus );
	m_buttonadd->setIconSize( QSize(24, 24) );
	m_buttonadd->setIcon( QIcon(":/images/add-24x24.png") );
	m_buttonadd->resize( SIZE_BUTTONADD, SIZE_BUTTONADD );
	m_buttonadd->hide();

	connect( m_rename_edit, SIGNAL(editingFinished()), this, SLOT( slotRenameEditFinished() ));
	connect( m_buttonadd, SIGNAL(clicked()), this, SLOT(slotButtonAdd()) );

}

void WidgetHeaders::setHasButtonAdd(bool _f)
{
	m_has_button_add = _f;

	if ( m_has_button_add )
		m_buttonadd->show();
	else
		m_buttonadd->hide();
}


//
// PROTECTED
//

void WidgetHeaders::resizeEvent(QResizeEvent *_event)
{
	QWidget::resizeEvent( _event );

	m_rename_edit->setFixedHeight( _event->size().height() );

	if ( m_fit_screen )
	{
		fitHeaders( 0 );
		updateAllPositions( );
		finishAnimation();
	}
}

void WidgetHeaders::mousePressEvent ( QMouseEvent * _event )
{
	boost::unique_lock<boost::recursive_mutex> lock( m_mutex );

	int border = whoseRightBorder( _event->x() );
	int header = whosePlace ( _event->x() );


	if ( Qt::LeftButton == _event->button() )
	{
		m_index_pressed = header;

		if ( -1 != border )
		{
			m_drg_border.start( &WidgetHeaders::changeBorderPos, border, _event->x(), 0 );
			return;
		}

		if ( -1 != header )
		{
			m_drg_header.start ( &WidgetHeaders::changeHeaderPos, header, _event->x(),
								_event->x() - m_headers[header]->m_pos );

			if ( m_selectable )
			{
				// Select.
				setSelectedAll( false );
				m_headers[ header ]->m_selected = true;
				signalSelected ( header );
				update();
			}
			return;
		}
	}

	if ( Qt::RightButton == _event->button() )
	{
		if ( -1 != header )
		{
			signalMenu( header, _event->x(), _event->y() );
		}
	}
}

void WidgetHeaders::mouseReleaseEvent( QMouseEvent *_event )
{
	m_drg_border.stop( &WidgetHeaders::finishBorderPos );
	m_drg_header.stop( &WidgetHeaders::finishHeaderPos );
	m_index_pressed = -1;

	update();
}

void WidgetHeaders::mouseMoveEvent ( QMouseEvent * _event )
{
	boost::unique_lock<boost::recursive_mutex> lock( m_mutex );

	update();

	if ( !m_tab_look )
	{
		if ( Qt::NoButton == _event->buttons() )
		{
			if ( -1 == whoseRightBorder( _event->x() ) )
				setCursor( Qt::ArrowCursor );
			else
				setCursor( Qt::SizeHorCursor );

			return;
		}

		m_drg_border.update( _event->x() );
	}

	m_drg_header.update( _event->x() );
}

void WidgetHeaders::mouseDoubleClickEvent ( QMouseEvent *_event )
{
	if ( Qt::LeftButton == _event->button() )
	{
		boost::unique_lock<boost::recursive_mutex> lock( m_mutex );
		int i = whosePlace ( _event->x() );
		if ( -1 == i )
			return;

		m_index_editing = i;
		m_rename_edit->setText( m_headers[i]->m_text );
		m_rename_edit->move( m_headers[i]->m_pos + 5, 1 );
		m_rename_edit->setFixedWidth( m_headers[i]->m_width-10 );
		m_rename_edit->setFocus();
		m_rename_edit->show();
	}
}


void WidgetHeaders::changeBorderPos( int _border, int _coord )
{
	// Resize 2 columns.
	// Send signal.
	int newsize = _coord - m_headers[ _border ]->m_pos;
	if ( newsize < HEADER_SIZE_MIN )
		newsize = HEADER_SIZE_MIN;

	m_headers[ _border ]->resize ( newsize, height() );
	//m_headers[ m_border_pressed ].textSize( m_headers[ m_border_pressed ]->m_width, height() );

	if ( m_fit_screen)
		fitHeaders( _border + 1 );

	// Update goals for all headers (important for headers placed next to this header).
	// false = not animated
	updateAllPositions();
	finishAnimation();

	signalResize( _border, newsize );
	update();
}

void WidgetHeaders::changeHeaderPos( int _header, int _coord )
{
	m_headers[ _header ]->m_pos_goal = _coord;
	m_headers[ _header ]->finishAnimation();

	// Important: the "m_headers[ m_index_pressed ]->m_pos + m_press_offset" differs from "m_headers[ m_index_pressed ]->m_pos"
	// when mouse is outside the widget.
	// to depend on mouse potision, not on tab's begining position.
	int whose = whosePlace ( m_headers[ _header ]->m_pos + m_drg_header.offset() );
	//std::cout << "w " << whose << "\n";
	// compose pos_goal for every tab:
	unsigned offset = 0;
	int cnt = m_headers.size();
	for ( int i = 0; i < cnt; ++i )
	{
		if ( i == whose && i <= _header )
			offset += m_headers[ _header ]->m_width;

		if ( i != _header )
		{
			m_headers[i]->m_pos_goal = offset;
			offset += m_headers[i]->m_width;
		}

		if ( i == whose && i > _header )
			offset += m_headers[ _header ]->m_width;
	}
	if ( ! m_animated )
		finishAnimation();
	update();
}


void WidgetHeaders::finishHeaderPos( int _header )
{
	boost::unique_lock<boost::recursive_mutex> lock( m_mutex );

	int coord = m_headers[ _header ]->m_pos + m_drg_header.offset();

	int whose;
	if ( coord > 0 )
	{
		whose = whosePlace ( coord );
		if ( -1 == whose )
			whose = m_headers.size() - 1; // last one
	}
	else
		whose = 0;

	if ( whose != _header )
	{
		TrackHeaderInfo *tmp = m_headers[ _header ];
		m_headers.erase( m_headers.begin() + _header );
		m_headers.insert( m_headers.begin() + whose, tmp );

		enumerate();

		signalMove( _header, whose );
	}

	updateAllPositions();
}


bool WidgetHeaders::isSelected(unsigned _index)
{
	// TODO: return something good
	return false;
}


// PROTECTED

void WidgetHeaders::paintEvent(QPaintEvent *_event)
{
	QPainter pnt( this );

	// LOCK
	boost::unique_lock<boost::recursive_mutex> lock( m_mutex );

	if ( m_tab_look )
	{
		int cnt = m_headers.size();
		for ( int i = 0; i < cnt; ++i )
		{
			if ( i == m_index_pressed )
				continue;

			this->paintTab( pnt, i );
		}

		// Draw pressed tab.
		if ( -1 != m_index_pressed )
		{
			this->paintTab( pnt, m_index_pressed );
		}
	}
	else
	{
		int cnt = m_headers.size();
		for ( int i = 0; i < cnt; ++i )
		{
			if ( i == m_index_pressed )
				continue;

			this->paintHeader( pnt, i );
		}

		// Draw pressed tab.
		if ( -1 != m_index_pressed )
		{
			this->paintHeader( pnt, m_index_pressed );
		}
	}
}



void WidgetHeaders::paintHeader(QPainter &pnt, int  _idx)
{
	if ( m_headers[_idx]->m_selected )
	{
		pnt.fillRect ( m_headers[_idx]->m_pos, 0, m_headers[_idx]->m_width, height(), QCOLOR_SELECTED );
	}
	else
	{
		pnt.fillRect ( m_headers[_idx]->m_pos, 0, m_headers[_idx]->m_width, height(), QCOLOR_ORDINARY );
	}

	pnt.setPen( QColor( 0x00, 0x00, 0x00, 0xff ));



	pnt.drawText(	m_headers[_idx]->m_pos + m_headers[_idx]->m_text_x,
					m_headers[_idx]->m_text_y,
					m_headers[_idx]->m_text_width,
					m_headers[_idx]->m_height,
					Qt::TextSingleLine,
					m_headers[_idx]->m_text ,
					0 );

	pnt.setPen( QColor( 0xAA, 0xAA, 0xAA, 0xff ));
	pnt.drawLine( m_headers[_idx]->m_pos, 0, m_headers[_idx]->m_pos, height()-1 );
	//pnt.drawRect( m_headers[_idx]->m_pos, 0, m_headers[_idx]->m_width-1, height()-1 );
}

void WidgetHeaders::paintTab(QPainter &pnt, int  _idx)
{
	int elem_height, elem_y;
	if ( m_headers[_idx]->m_selected )
	{
		elem_height = height();
		elem_y = 0;
	}
	else
	{
		elem_height = height() - ORDINARY_SMALLER;
		elem_y = ORDINARY_SMALLER;
	}

	if ( m_headers[_idx]->m_selected )
	{
		pnt.fillRect ( m_headers[_idx]->m_pos, 0, m_headers[_idx]->m_width, height(), QCOLOR_SELECTED );
	}
	else
	{
		pnt.fillRect ( m_headers[_idx]->m_pos, ORDINARY_SMALLER, m_headers[_idx]->m_width, height()-ORDINARY_SMALLER, QCOLOR_ORDINARY );
	}

	pnt.setPen( QColor( 0x00, 0x00, 0x00, 0xff ));

	pnt.drawText( 	m_headers[_idx]->m_pos + m_headers[_idx]->m_text_x,
					m_headers[_idx]->m_text_y,
					m_headers[_idx]->m_text_width,
					m_headers[_idx]->m_height,
					Qt::TextSingleLine,
					m_headers[_idx]->m_text,
					0);

	pnt.drawLine( m_headers[_idx]->m_pos+1, elem_y, m_headers[_idx]->m_pos + m_headers[_idx]->m_width, elem_y );

	pnt.drawLine( m_headers[_idx]->m_pos, elem_y, m_headers[_idx]->m_pos, height()-1 );
	pnt.drawLine( m_headers[_idx]->m_pos + m_headers[_idx]->m_width, elem_y, m_headers[_idx]->m_pos + m_headers[_idx]->m_width, height()-1 );
}

void WidgetHeaders::leaveEvent( QEvent *_event )
{
	setCursor( Qt::ArrowCursor );
}

void WidgetHeaders::fitHeaders(unsigned _starting_from)
{
	if ( _starting_from >= m_headers.size() )
		return;

	int w = width();
	if ( m_has_button_add )
		w -= SIZE_BUTTONADD;

	if ( w <= 0 )
		return;

	unsigned sum_before = 0;
	unsigned sum_liquid = 0;

	for ( size_t i = 0; i < _starting_from; ++i )
		sum_before += m_headers[ i ]->m_width;

	int space = w - sum_before;
	if ( space <= 0 )
		return;

	size_t sz = m_headers.size();
	for ( size_t i = _starting_from; i < sz; ++i )
	{
		if ( m_headers[ i ]->m_options & HEADER_OPT_AUTORESIZE )
			sum_liquid += m_headers[ i ]->m_width;
		else
			space -= m_headers[ i ]->m_width;
	}

	if ( space <= 0 || 0 == sum_liquid)
		return;

	for ( size_t i = _starting_from; i < sz; ++i )
	{
		if ( m_headers[ i ]->m_options & HEADER_OPT_AUTORESIZE )
		{
			unsigned new_width = ( ( m_headers[ i ]->m_width / ((float)sum_liquid) ) * space );
			m_headers[ i ]->resize( new_width , height() );
			signalResize( i, new_width );
		}
	}

	updateAllPositions( );
	finishAnimation();
}

int WidgetHeaders::whosePlace(int _coord)
{
	int offset = 0;
	int cnt = m_headers.size();
	for ( int i = 0; i < cnt; ++i )
	{
		if ( _coord >= offset && _coord < offset + (int)m_headers[i]->m_width )
			return i;

		offset += m_headers[i]->m_width;
	}

	return -1;
}


int WidgetHeaders::whoseRightBorder(int _coord)
{
	int offset = 0;

	int len = m_headers.size();
	for ( int i = 0; i < len; ++i )
	{
		offset += m_headers[i]->m_width;
		if ( /*i < (len - 1) &&*/  _coord >= offset - HEADER_BORDER_HALFWIDTH && _coord <= offset + HEADER_BORDER_HALFWIDTH )
			return i;
	}
	return -1;
}

void WidgetHeaders::updateAllPositions( )
{
	unsigned offset = 0;
	int cnt = m_headers.size();
	for ( int i = 0; i < cnt; ++i )
	{
		m_headers[i]->m_pos_goal = offset;
		offset += m_headers[i]->m_width;
	}

	if ( ! m_animated )
		finishAnimation();

	if ( m_has_button_add )
	{
		int y = (height() - m_buttonadd->height()) / 2;
		if ( y < 0 )
			y = 0;

		m_buttonadd->move(offset, y);
	}
}

void WidgetHeaders::finishAnimation( )
{
	int cnt = m_headers.size();
	for ( int i = 0; i < cnt; ++i )
	{
		m_headers[i]->finishAnimation();
	}
}

void WidgetHeaders::enumerate()
{
	int cnt = m_headers.size();
	for ( int i = 0; i < cnt; ++i )
		m_headers[i]->setIndex( i );
}

//
// SLOTS
//

void WidgetHeaders::animate()
{
	//return;
	boost::unique_lock<boost::recursive_mutex> lock( m_mutex );
	bool changed = false;
	int sum = 0;
	int cnt = m_headers.size();
	for ( int i = 0; i < cnt; ++i )
	{
		if ( i == m_index_pressed )
			continue;

		changed |= m_headers[i]->animate();
	}

	if ( changed )
	{
		update();
	}
}


unsigned WidgetHeaders::addHeader(const QString &_name, int _width, int _options)
{
	boost::unique_lock<boost::recursive_mutex> lock( m_mutex );

	TrackHeaderInfo *thi = new TrackHeaderInfo( this, _name, _options );

	if ( thi->m_close_button )
		connect ( thi->m_close_button.get(), SIGNAL(clicked(unsigned)), this, SLOT(slotCloseButtonClicked(unsigned)) );

	thi->resize( _width, height() );
	m_headers.push_back( thi );

	setSelectedAll( false );
	m_headers.back()->m_selected = true;

	enumerate();
	if ( m_fit_screen )
		fitHeaders( 0 );

	updateAllPositions();
	return m_headers.size() - 1;
}

void WidgetHeaders::delHeader(unsigned _index)
{
	boost::unique_lock<boost::recursive_mutex> lock( m_mutex );

	if ( _index >= m_headers.size() )
		return;

	// TODO: boost::shared_ptr
	delete m_headers[_index];

	m_headers.erase( m_headers.begin() + _index );

	enumerate();
	if ( m_fit_screen)
		fitHeaders( 0 );

	updateAllPositions();
	update();
}


void WidgetHeaders::rename(unsigned _index, const QString& _name)
{
	boost::unique_lock<boost::recursive_mutex> lock( m_mutex );
	if ( m_headers.size() < _index + 1 )
		return;

	m_headers[_index]->rename ( _name );

	update();
}

void WidgetHeaders::setSelected(unsigned _index, bool _flag)
{
	boost::unique_lock<boost::recursive_mutex> lock( m_mutex );
	if ( (size_t)_index >= m_headers.size() )
		return;

	m_headers[ _index ]->m_selected = _flag;
	update();
}

void WidgetHeaders::setSelectedAll( bool _flag )
{
	boost::unique_lock<boost::recursive_mutex> lock( m_mutex );

	BOOST_FOREACH( TrackHeaderInfo* h, m_headers )
	{
		h->m_selected = _flag;
	}

	update();
}

void WidgetHeaders::addWidgetToHeader(unsigned _index, QWidget *_w)
{
	if ( (size_t)_index >= m_headers.size() )
	{
		// delete it, because we take ownerwhip.
		delete _w;
		return;
	}

	m_headers[_index]->addWidget( _w );
}

void  WidgetHeaders::slotRenameEditFinished()
{
	if ( -1 == m_index_editing )
		return;

	m_headers[ m_index_editing ]->rename( m_rename_edit->text() );
	m_rename_edit->hide();

	signalRename( m_index_editing, m_headers[ m_index_editing ]->m_text );

	m_index_editing = -1;
}

void WidgetHeaders::slotCloseButtonClicked(unsigned _index_tab)
{
	signalClose( _index_tab );
}

void WidgetHeaders::slotButtonAdd()
{
	signalAdd();
}


