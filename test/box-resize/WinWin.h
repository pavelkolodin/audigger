
#pragma once

#include <QtWidgets>
#include <iostream>
#include "Selection.h"

#define LOGCOUT(x) { std::cout << __FILE__ << ":" << __LINE__ << " " << x << "\n"; }

class WinWin: public QWidget
{
	Q_OBJECT
	public:

		WinWin();

	protected:
		virtual void paintEvent(QPaintEvent *);
		virtual void mouseDoubleClickEvent(QMouseEvent * event);
		virtual void mouseMoveEvent(QMouseEvent * event);
		virtual void mousePressEvent(QMouseEvent * event);
		virtual void mouseReleaseEvent(QMouseEvent * event);

	public slots:
		void slotButtonClicked();
		void slotSpin(int _val);

	private:
		QPushButton *m_bx, *m_by;
		QSpinBox *m_spin[4];
		std::list < QRect > m_rects;
		bool m_selection_existed;

		Selection m_sel;
};
