/*
 *  pavelkolodin@gmail.com
 */

#ifndef _WIDGETS_COMMON_H_
#define _WIDGETS_COMMON_H_

#include <QtWidgets>
#include "WidgetTrackAbstract.h"
#include "EventDistributor.h"

//unsigned getFFTWinByIndex(int _i);
//void buildUI_FFTWin(QComboBox *_qcb);
//int getIndexByFFTWin(unsigned _win);


class WidgetFFTRegulator : public QComboBox
{
	Q_OBJECT
public:
	WidgetFFTRegulator(QWidget *_parent, unsigned _initial_fft_win, WidgetTrackAbstract *_widget_track);

private slots:
	void slotIndexChanged(int);

private:
	WidgetTrackAbstract *m_widget_track;
//signals:
//	void newFFTWin(unsigned _fft_win);
};



class WidgetAnchor : public QWidget, public EventDistributor::EventUser
{
	Q_OBJECT

public:
	WidgetAnchor( QWidget *_parent, EventDistributor *_ED, TrackAudio::Ptr _track_ptr );
	virtual ~WidgetAnchor();

	virtual void trigger();

	virtual void enterEvent(QEvent *);
	virtual void leaveEvent(QEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);

	unsigned m_id;
protected:
	virtual void paintEvent(QPaintEvent *);
signals:
	void clicked(unsigned);

private:
	// The state of the widget (anchor / link) is stored in TrackAbstract::m_anchor.
	TrackAudio::Ptr m_track;
	boost::shared_ptr<QPixmap> m_pixmap_circle_red;
	boost::shared_ptr<QPixmap> m_pixmap_link;
	boost::shared_ptr<QPixmap> m_pixmap_anchor;
	bool m_under_mouse;
};


#endif

