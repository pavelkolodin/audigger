
#ifndef _MARK_GUI_H_
#define _MARK_GUI_H_

#include <QtWidgets>
#include "Selection.h"

// height of icon
#define RANGEREGULATOR_KNOB_HEIGHT 8
// width of icon
#define RANGEREGULATOR_WIDTH 16

#define MARK_TEXT_MARGIN_LEFT 1
#define MARK_TEXT_MARGIN_RIGHT 20
#define SIZE_TEXT_MIN 16


//#define LABEL_MARGIN_LEFT 2
#define LABEL_WIDTH_MIN 20

class MarkGUI
{
	// TODO: make these members private
public:
	Selection m_sel;
	QString m_string_cached;
	const Mark *m_mark;
	// one-dimensional coordinate
	int m_coord;
	// one-dimensional size
	unsigned m_len;
	unsigned m_label_width;
	unsigned m_label_height;
	unsigned m_width;

	//QRect m_rect_rangeregulator;
	int m_rangeregulator_x;

	// display rangeregulator knob or not?
	bool m_rangeregulator_knob;

	bool m_selected;
	// If this mark is over another, then place is busy
	bool m_place_bad;
	// this mark exists in TrackMarks
	// Used by "move logic" : when you start moving a mark
	// we delete it from TrackMarks to not conflict with itself
	// when asking for possibility of new place.
	// When you stop moving it, we add it to new place if possible,
	// or add it on old place (kept by MarksInterval, m_marks_mem).
	bool m_exists;

	// valid MarkGUI is visible mark gui.
	bool m_valid;

public:
	MarkGUI()
	: m_mark(0)
	, m_coord(0)
	, m_len(0)
	, m_label_width(0)
	, m_label_height(0)
	, m_width(0)
	, m_rangeregulator_x(0)
	, m_rangeregulator_knob(false)
	, m_selected(false)
	, m_place_bad(false)
	, m_exists(false)
	, m_valid(false)
	{
		m_sel.disableX(true);
		m_sel.enableAutocreation(false);
		setLen( 0 );
	}

	void init( const Mark& _mark, unsigned _step_frames )
	{
		m_mark = &_mark;

		if ( ! _step_frames )
			return;

		m_string_cached = QString::fromUtf8( _mark.m_text.c_str() );
		m_selected = _mark.m_selected;
		move (_mark.m_pos / _step_frames);

		// TODO: rangeregulator code
		setLen( _mark.m_len / _step_frames );

		m_sel.setY(_mark.m_pos / _step_frames, _mark.m_len / _step_frames );

		// We need to set appropriate size for a text.
		// We cannot do this here, because we need QPainter to calculate text size (in pixels).
		// So we set size = 0 here, and later paintEvent() will do the job!
		m_label_height = 0;
		m_exists = true;
		m_valid = true;
		m_place_bad = false;
	}

	void setLen(unsigned _len)
	{
		m_len = _len;
		m_rangeregulator_knob = ( m_len < RANGEREGULATOR_KNOB_HEIGHT );
	}

	void move(int _coord)
	{
		m_coord = _coord;
		setLen( m_len );
	}

	void setWidth(unsigned w)
	{
		m_width = w;
		m_sel.setX(0, w);

		m_label_width = w;
		if ( w >= (LABEL_WIDTH_MIN + RANGEREGULATOR_WIDTH) )
		{
			m_rangeregulator_x = (w - RANGEREGULATOR_WIDTH);
			m_label_width -= RANGEREGULATOR_WIDTH;
		}
		else
			m_rangeregulator_x = 0;

	}

	bool hitLabel(int x, int y)
	{
		if ( y >= m_coord && y < (m_coord + (int)m_label_height) &&
			x < (int)m_label_width )
			return true;

		return false;
	}

	bool hitKnob(int x, int y)
	{
		if ( ! m_rangeregulator_knob )
			return false;

		if ( ( x >= m_rangeregulator_x ) && ( x < m_rangeregulator_x + RANGEREGULATOR_WIDTH ) &&
				( y >= m_coord + (int)m_len ) && ( y < m_coord + (int)m_len + RANGEREGULATOR_KNOB_HEIGHT ) )
			return true;

		return false;
	}

	bool hit(int x, int y)
	{
		if ( hitLabel(x, y) || m_sel.hit(x, y) || hitKnob(x, y) )
			return true;

		return false;
	}

	bool changingInProgress()
	{
		if ( ! m_valid )
			return false;

		return (m_sel.moving() || m_sel.resizing());
	}

	// return if mouse hits any of the mark's part
	// of mouse affected the selection
	bool mousePress(int x, int y)
	{
		bool ret = false;


		if ( hitLabel(x, y) )
		{
			// If mouse hits the mark text, start moving the mark.
			m_sel.mouseMove(x, y);
			m_sel.startMove();
			ret = true;
		}
		else if ( hitKnob(x, y) )
		{
			// If mouse hits the knob, start resizing
			// the bottom border or the selection.
			m_sel.startSelectionEnd(1);
			ret = true;
		}
		else
			ret = m_sel.mousePress();

		return ret;
	}

	bool mouseMove(int x, int y)
	{
		if ( ! m_sel.moving() )
			if ( hitLabel(x, y) )
				return false;

		bool ret = m_sel.mouseMove(x, y);
		if ( ret )
		{
			m_coord = m_sel.y();
			setLen ( m_sel.height() );
		}
		return ret;
	}

	void mouseRelease()
	{
		m_sel.mouseRelease();
	}

	Qt::CursorShape getCursor()
	{
		if ( 	(m_sel.mouseOverBeginX() && m_sel.mouseOverBeginY() ) ||
				(m_sel.mouseOverEndX() && m_sel.mouseOverEndY() ) )
			return Qt::SizeFDiagCursor;
		else
		if ( 	(m_sel.mouseOverBeginX() && m_sel.mouseOverEndY() ) ||
				(m_sel.mouseOverEndX() && m_sel.mouseOverBeginY() ) )
			return Qt::SizeBDiagCursor;
		else
		if ( m_sel.mouseOverBeginX() || m_sel.mouseOverEndX() )
			return Qt::SizeHorCursor;
		else
		if ( m_sel.mouseOverBeginY() || m_sel.mouseOverEndY() )
			return Qt::SizeVerCursor;
		else
			return Qt::ArrowCursor;
	}

	void invalidate()
	{
		m_valid = false;
		m_sel.clear();
	}
};

#endif
