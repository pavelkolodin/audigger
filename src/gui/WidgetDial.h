/*
 *  pavelkolodin@gmail.com
 */

#ifndef _WidgetDial_H_
#define _WidgetDial_H_

#include <QtWidgets>
#include <boost/noncopyable.hpp>
#include <fir/logger/logger.h>

class WidgetDial : public QAbstractSpinBox //, public boost::noncopyable
{		
	Q_OBJECT

	public:
		WidgetDial ( QWidget *_parent );
		virtual ~WidgetDial ( );

		// generates signal
		// steps > 0 - move value up
		// steps < 0 - move value down
		virtual void stepBy(int _steps);

		// DOES NOT generate signal
		void setValue(unsigned _value);

		unsigned getValue();

	protected:
		virtual StepEnabled	stepEnabled() const;
		unsigned m_value;

	signals:
		void valueChanged(unsigned _value);


//		virtual void paintEvent(QPaintEvent *);
//		virtual void resizeEvent(QResizeEvent *);
//
//		QPushButton *m_button_up;
//		QPushButton *m_button_down;
//		QLabel *m_label;
};

#endif

