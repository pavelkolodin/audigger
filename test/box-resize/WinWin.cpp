

#include <boost/foreach.hpp>
#include "WinWin.h"

#define WIDTH_SPINBOX 80
#define WIDTH_BUTTON 20
#define HEIGHT_BUTTON 24


WinWin::WinWin()
: m_selection_existed(0)
{
	setMouseTracking( true );
	resize( 640, 480 );

	m_bx = new QPushButton(this);
	m_bx->setCheckable(true);
	m_bx->setChecked(true);
	m_bx->setText( "x" );
	m_bx->move ( 10, 10 );
	m_bx->resize( WIDTH_BUTTON, HEIGHT_BUTTON );
	m_bx->show();

	m_by = new QPushButton(this);
	m_by->setCheckable(true);
	m_by->setChecked(true);
	m_by->setText( "y" );
	m_by->move ( 10, 10 + HEIGHT_BUTTON + 5 );
	m_by->resize( WIDTH_BUTTON, HEIGHT_BUTTON );
	m_by->show();

	connect( m_bx, SIGNAL(clicked()), this, SLOT(slotButtonClicked()) );
	connect( m_by, SIGNAL(clicked()), this, SLOT(slotButtonClicked()) );

	//
	// Spins
	//
	// 0 - xmin, 1 - xmax, 2 - ymin, 3 - ymax
	//
	for ( int i = 0; i < 4; ++i )
	{
		m_spin[i] = new QSpinBox(this);
		m_spin[i]->setAccelerated(true);
		m_spin[i]->setMaximum( 4000 );
		m_spin[i]->setMinimum( -4000 );
		m_spin[i]->resize(WIDTH_SPINBOX, HEIGHT_BUTTON);
		m_spin[i]->show();
		connect( m_spin[i], SIGNAL(valueChanged(int)), this, SLOT(slotSpin(int)) );
	}

	m_spin[0]->setValue( -4000 );
	m_spin[1]->setValue( 4000 );
	m_spin[2]->setValue( -4000 );
	m_spin[3]->setValue( 4000 );

	m_spin[0]->move( 10 + WIDTH_BUTTON + 5, 10 );
	m_spin[1]->move( 10 + WIDTH_BUTTON + 5 + WIDTH_SPINBOX, 10 );

	m_spin[2]->move( 10 + WIDTH_BUTTON + 5, 10 + HEIGHT_BUTTON + 5);
	m_spin[3]->move( 10 + WIDTH_BUTTON + 5 + WIDTH_SPINBOX, 10 + HEIGHT_BUTTON + 5);
}


void WinWin::paintEvent(QPaintEvent *)
{
	QPainter painter( this );

	painter.setPen( QColor(0x00, 0x00, 0x00, 0xff) );

    BOOST_FOREACH( const QRect& rect, m_rects )
    {
    	painter.drawRect( rect );
    }

    if (  m_sel.width() && m_sel.height() )
    {
    	painter.setPen( QColor(0x00, 0xaa, 0x00, 0xff) );
    	painter.drawRect( m_sel.x(), m_sel.y(), m_sel.width(), m_sel.height() );
    }
}

void WinWin::mouseDoubleClickEvent(QMouseEvent * event)
{

}

void WinWin::mouseMoveEvent(QMouseEvent * event)
{
	m_sel.mousePos( event->x(), event->y() );

    if (    (m_sel.mouseOverBeginX() && m_sel.mouseOverBeginY() ) ||
            (m_sel.mouseOverEndX() && m_sel.mouseOverEndY() ) )
        setCursor( Qt::SizeFDiagCursor );
    else
    if (    (m_sel.mouseOverBeginX() && m_sel.mouseOverEndY() ) ||
            (m_sel.mouseOverEndX() && m_sel.mouseOverBeginY() ) )
        setCursor( Qt::SizeBDiagCursor );
    else
    if ( m_sel.mouseOverBeginX() || m_sel.mouseOverEndX() )
        setCursor( Qt::SizeHorCursor );
    else
    if ( m_sel.mouseOverBeginY() || m_sel.mouseOverEndY() )
        setCursor( Qt::SizeVerCursor );
    else
    	setCursor( Qt::ArrowCursor );


    update();
}

void WinWin::mousePressEvent(QMouseEvent * event)
{
	if ( Qt::RightButton == event->button() )
		m_sel.startMove();
	else
		m_sel.mousePress();
}

void WinWin::mouseReleaseEvent(QMouseEvent * event)
{
	m_sel.mouseRelease();

	if ( ! m_selection_existed && ! m_sel.noSelection() )
	{
		m_rects.push_back( QRect(m_sel.x(), m_sel.y(), m_sel.width(), m_sel.height() ) );
	}

	m_selection_existed = ! m_sel.noSelection();
	//m_sel.clear();
}

void WinWin::slotButtonClicked()
{
	m_sel.disableX( ! m_bx->isChecked() );
	m_sel.disableY( ! m_by->isChecked() );

	if ( ! m_bx->isChecked() )
	{
		m_sel.setX( 100, 200 );
		m_sel.setY( 0, 0 );
	}

	if ( ! m_by->isChecked() )
	{
		m_sel.setY( 100, 200 );
		m_sel.setX( 0, 0 );
	}

}

void WinWin::slotSpin(int _val)
{
	// we don't care about "_val"

	m_sel.setLimits( m_spin[0]->value(), m_spin[1]->value(), m_spin[2]->value(), m_spin[3]->value() );
}
