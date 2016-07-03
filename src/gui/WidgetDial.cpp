/*
 *  pavelkolodin@gmail.com
 */

#include <climits>
#include "WidgetDial.h"


void setupButton(QPushButton *b, int _w, int _h, const char *_filename_icon)
{
	b->setFocusPolicy( Qt::NoFocus );
	b->setFlat( true );
	b->resize( _w, _h );
	b->setIconSize( QSize(_w, _h) );
	b->setIcon( QIcon( _filename_icon ) );
}

WidgetDial :: WidgetDial ( QWidget *_parent )
: QAbstractSpinBox( _parent )
{
//	m_button_up = new QPushButton( this );
//	m_button_down = new QPushButton( this );
//
//	setupButton( m_button_up, 12, 12, ":/images/arrow-up-12x12.png" );
//	setupButton( m_button_down, 12, 12, ":/images/arrow-down-12x12.png" );
//
//	m_label = new QLabel ( this );

	m_value = 1;

	lineEdit()->setText( QString::number( m_value ) );
}


WidgetDial :: ~WidgetDial ( )
{

}

void WidgetDial::stepBy(int steps)
{
	//LOGCOUT( "stepBy " << steps );

	if ( steps > 0 )
	{
		if ( m_value < UINT_MAX/2 )
			m_value *= 2;
	}
	else
	{
		if ( m_value >= 2 )
			m_value /= 2;
	}

	valueChanged( m_value );
	lineEdit()->setText( QString::number( m_value ) );
}

void WidgetDial::setValue(unsigned _value)
{
	if ( _value > 0 )
	{
		m_value = _value;
		lineEdit()->setText( QString::number( m_value ) );
	}
}

unsigned WidgetDial::getValue()
{
	return m_value;
}

QAbstractSpinBox::StepEnabled	WidgetDial::stepEnabled() const
{
	return ( QAbstractSpinBox::StepUpEnabled | ((m_value > 1)?(QAbstractSpinBox::StepDownEnabled):QAbstractSpinBox::StepUpEnabled) );
}

//// SLOTS
//
//void WidgetDial :: setValue( unsigned _value )
//{
//
//}
//
////
//// PROTECTED
////
//
//void WidgetDial::paintEvent ( QPaintEvent *_event )
//{
//
//}
//
//void WidgetDial::resizeEvent(QResizeEvent *_event)
//{
//	m_button_up->move( _event->size().width() - 12, 0 );
//	m_button_down->move( _event->size().width() - 12, 12 );
//}
