/*
 *  pavelkolodin@gmail.com
 */

#ifndef _Selection_H_
#define _Selection_H_

#include <cstddef>

class Selection
{
public:
	struct Range
	{
		size_t pos;
		size_t len;
	};

	Selection();

	// Reset settings.
	void reset();

	// Remove selection
	void clear();

	// return true if this position touches us.
	bool mouseMove(int _x, int _y);

	// return true if selection pressed (border or body).
	// return false if mouse pressed outside of the selection
	bool mousePress();
	void mouseRelease();
	void startMove();
	void startSelectionBegin(int _dimension);
	void startSelectionEnd(int _dimension);

	// \return (0 == width || 0 == height )
	bool noSelection();

	// Is some border of _x or _y dimention touched?
	// return: 0 - false, !0 - true.
	//void touched(int *_x, int *_y);

	void disableX(bool _disable);
	void disableY(bool _disable);
	void setLimits( int _xmin, int _xmax, int _ymin, int _ymax );
	void setBorderFeelRangeX(unsigned);
	void setBorderFeelRangeY(unsigned);

	// enable / disale automatic creation of new selection
	// when mouse pressed outside the existing selection
	// If disabled, you can create selection only with setX(), setY()
	void enableAutocreation(bool);

	void setX(size_t _pos, size_t _len);
	void setY(size_t _pos, size_t _len);

	int x();
	int y();
	int width();
	int height();
	bool hit(int x, int y);

	bool mouseOverBeginX() { return m_touched_begin[0]; }
	bool mouseOverEndX() { return m_touched_end[0]; }

	bool mouseOverBeginY() { return m_touched_begin[1]; }
	bool mouseOverEndY() { return m_touched_end[1]; }
	bool moving() { return m_moving; }
	bool resizing() { return (m_selecting[0] || m_selecting[1]); }

private:
	// dimention = 2: X, Y
	int m_pos[2];

	int m_pos_press_off[2];

	// Where selection started.
	int m_pos_press[2];

	// Current selection borders.
	// Guaranteed: BEGIN < END
	// * End points to the position AFTER last pixel!
	int m_sel_begin[2], m_sel_end[2];

	// *** Not implemented : TODO ***
	// Limiting values.
	// [0]: xmin, xmax, [1]:ymin, ymax
	int m_limits[2][2];

	// distance between border and mouse cursor
	// where the border can be taken by mouse.
	unsigned m_border_feel_range[2];

	// default: false
	bool m_limits_enabled;
	bool m_autocreation;

	// Automated axis or dimention is a dimention that doesn't affected by selection tool at creation time.
//	int m_auto_begin, m_auto_end; // only for 1 dimention.
//	bool m_auto[2];
	bool m_disabled[2];

	// m_selecting - what dimentions are in progress of changing by selection tool.
	bool m_selecting[2];

	// "Is begin border touched by mouse?"
	// Used by display functions to display special cursor.
	bool m_touched_begin[2];

	// "Is end border touched by mouse?"
	// Used by display functions to display special cursor.
	bool m_touched_end[2];

	// Mouse selection button pressed.
	bool m_pressed;
	bool m_moving;

};

#endif

