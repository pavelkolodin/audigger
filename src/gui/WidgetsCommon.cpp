/*
 *  pavelkolodin@gmail.com
 */

#include <sstream>
#include "defines.h"
#include "WidgetsCommon.h"
#include "common.h"

#define BUTTON_CLOSE_FILENAME ":/images/close-bold-sharp-8x8-14x14.png"
#define BUTTON_CLOSE_FILENAME_HL ":/images/close-bold-sharp-bg-d93b3b-8x8-14x14.png"

void buildUI_FFTWin(QComboBox *_qcb, int default_fft_win)
{
	std::stringstream SS;
	for ( int i = 0, w = FFT_WIN_MIN; w <= FFT_WIN_MAX; w *= 2, ++i )
	{
		SS.str("");
		SS << w;
		_qcb->addItem( SS.str().c_str() );
		if ( default_fft_win == w )
			_qcb->setCurrentIndex( i );
	}
}

WidgetFFTRegulator::WidgetFFTRegulator(QWidget *_parent, unsigned _initial_fft_win, WidgetTrackAbstract *_widget_track)
: QComboBox(_parent)
, m_widget_track( _widget_track )
{
	resize(64, 22);
	buildUI_FFTWin( this, _initial_fft_win );

	setFrame( false );
	setFocusPolicy(Qt::NoFocus);
	setContextMenuPolicy(Qt::NoContextMenu);

	connect ( this, SIGNAL(currentIndexChanged (int)), this, SLOT(slotIndexChanged(int)) );
}

void WidgetFFTRegulator::slotIndexChanged(int _index)
{
	if ( m_widget_track )
	{
		unsigned fft_win = getFFTWinByIndex( _index );
		m_widget_track->setFFTWin( fft_win );
		m_widget_track->resetRevision();
		m_widget_track->refresh();
	}
	//newFFTWin( getFFTWinByIndex( _index ) );
}




#define FILENAME_CIRCLE_RED		":/images/circle-d93b3b-14x14.png"
#define FILENAME_ICON_ANCHOR	":/images/anchor/anchor-14x14.png"
#define FILENAME_ICON_LINK		":/images/anchor/anchor-off-14x14.png"


WidgetAnchor::WidgetAnchor( QWidget *_parent, EventDistributor *_ed, TrackAudio::Ptr _track_ptr )
: QWidget ( _parent )
, EventDistributor::EventUser( _ed )
, m_id(0)
, m_track( _track_ptr )
, m_pixmap_circle_red( new QPixmap( ) )
, m_pixmap_link ( new QPixmap( ) )
, m_pixmap_anchor ( new QPixmap( ) )
, m_under_mouse(0)

{
	m_pixmap_circle_red->convertFromImage( QImage( FILENAME_CIRCLE_RED ) );
	m_pixmap_link->convertFromImage( QImage( FILENAME_ICON_LINK ) );
	m_pixmap_anchor->convertFromImage( QImage( FILENAME_ICON_ANCHOR ) );

	// use first pixmap size (all pixmaps must be the same size (i think - 14x14)).
	resize( m_pixmap_link->size() );

}

WidgetAnchor::~WidgetAnchor()
{

}

void WidgetAnchor::trigger()
{
	update();
}


void WidgetAnchor::enterEvent(QEvent *_event)
{
	m_under_mouse = true;
	QWidget::enterEvent(_event);
	update();
}


void WidgetAnchor::leaveEvent(QEvent *_event)
{
	m_under_mouse = false;
	QWidget::leaveEvent(_event);
	update();
}

void WidgetAnchor::mousePressEvent(QMouseEvent *_event)
{

}

void WidgetAnchor::mouseReleaseEvent(QMouseEvent *_event)
{
	if ( _event->x() < 0 || _event->x() >= width() || _event->y() < 0 || _event->y() >= height() )
		return;

	m_track->setAnchor( !m_track->getAnchor() );
	trigAll();

	update();
	clicked( m_id );
}

void WidgetAnchor::paintEvent(QPaintEvent *)
{
	QPainter pnt(this);

	if ( m_under_mouse )
		pnt.drawPixmap(0, 0, *m_pixmap_circle_red);


	if ( m_track->getAnchor() ) //m_anchor )
		pnt.drawPixmap(0, 0, *m_pixmap_anchor);
	else
		pnt.drawPixmap(0, 0, *m_pixmap_link);
}






