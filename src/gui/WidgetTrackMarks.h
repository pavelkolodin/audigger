/*
 *  pavelkolodin@gmail.com
 */

#ifndef _WidgetTrackMarks_H_
#define _WidgetTrackMarks_H_

#include <QtWidgets>

#include "WidgetTrackAbstract.h"
#include "TrackMarks.h"
#include "Dragging.h"
#include "Selection.h"
#include "MarkGUI.h"
#include "WidgetTextEditEnter.h"
#include "MarksInterval.h"

/// \brief A widget responsible for single marks track (layer)
class WidgetTrackMarks : public WidgetTrackAbstract
{
	Q_OBJECT
public:
	WidgetTrackMarks(QWidget *_parent, WidgetTrackAbstract *_wtg, Controller *, TrackMarks::Ptr);
	virtual ~WidgetTrackMarks();

	//
	// WidgetTrackAbstract
	//
	virtual TrackType getType() { return TYPE_MARKS; }
	//virtual TrackAbstract* getTrackAbstract() { return m_track.get(); }
	virtual TrackMarks::Ptr getTrackMarks() { return m_track; }

	virtual void refresh();
	virtual void setViewPos(WidgetTrackAbstract::TrackPos _pos);
	virtual WidgetTrackAbstract::TrackPos getViewPos();
	virtual void setOffset(WidgetTrackAbstract::TrackPos _off); // offset to display marks from.
	virtual void setStepFrames( unsigned _step );
	virtual void clearSel();

protected:
	virtual void resizeEvent(QResizeEvent *_event);
	virtual void paintEvent(QPaintEvent *);
	virtual void mousePressEvent ( QMouseEvent * _event );
	virtual void mouseMoveEvent ( QMouseEvent * _event );
	virtual void mouseReleaseEvent( QMouseEvent *_event );
	virtual void mouseDoubleClickEvent ( QMouseEvent *_event );
	virtual void keyPressEvent ( QKeyEvent *_event);

	virtual void dragEnterEvent ( QDragEnterEvent *event );
	virtual void dragMoveEvent ( QDragMoveEvent *event );
	virtual void dropEvent ( QDropEvent *event );

private:
	// We use MarkGUI type, because it contains graphic size of mark's text ("m_label_height")
	// but this value is calculated only by paintEvent() and == 0 before that
	// so we use minimal QTextEdit size.
	void startEditingMark(const MarkGUI *);
	void paintMarkGUI(QPainter& _pnt, const MarkGUI &);

public slots:
	void slotMarkDelete();

private slots:
	void stopEditingMark();

private:
	TrackMarks::Ptr m_track;
	MarksInterval m_interval;

	WidgetTextEditEnter *m_text_edit;
	size_t m_view_pos_frame;
	bool m_mark_dragging_copying;
	QPixmap m_rangeregulator_knob;

	// absolute position of a mark that is editin
	Mark::TimeValue m_mark_editing_pos;
	bool m_mark_editing;

	unsigned m_new_mark_pos;
	bool m_multiselect;
};

#endif

