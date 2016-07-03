/*
 * pavelkolodin@gmail.com
 */

#pragma once

#ifndef _WidgetTextEditEnter_h_
#define _WidgetTextEditEnter_h_

#include <QtWidgets>

// This class send "signalPressEnter" when user press Enter.
// TODO: rename it to "editingFinished()" (QLineEdit has method named so).
class WidgetTextEditEnter : public QTextEdit
{
	Q_OBJECT
public:
	//WidgetTextEditEnter() { }
	WidgetTextEditEnter( QWidget *_parent ) : QTextEdit(_parent)
	{
		this->setFrameStyle( QFrame::Sunken ); // Sunken!
	}

protected:
	virtual void keyPressEvent(QKeyEvent *_event)
	{
		if ( _event->key() == Qt::Key_Return || _event->key() == Qt::Key_Escape || _event->key() == Qt::Key_F2)
		{
			signalPressEnter();
			return;
		}
		else
			QTextEdit::keyPressEvent( _event );
	}

	virtual void paintEvent(QPaintEvent *_event)
	{
		QTextEdit::paintEvent( _event );
		QPainter pnt( viewport() );
		pnt.setPen( QColor( 0, 0, 0, 0xff ));
		pnt.drawRect( 0, 0, width()-1, height()-1);
	}

	signals:
		void signalPressEnter();
};

#endif
