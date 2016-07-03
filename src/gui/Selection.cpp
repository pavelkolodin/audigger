/*
 *  pavelkolodin@gmail.com
 */

#include "Selection.h"

#define SELECTION_BORDER_RANGE 4

Selection::Selection()
{
	reset();
	clear();
}

void Selection::reset()
{
	for ( int i = 0; i < 2; ++i )
	{
		m_disabled[i] = 0;
		m_limits[i][0] = 0;
		m_limits[i][1] = 0;
	}
	m_limits_enabled = false;
	m_autocreation = true;
	m_border_feel_range[0] = 4;
	m_border_feel_range[1] = 4;
}

void Selection::clear()
{
	for ( int i = 0; i < 2; ++i )
	{
		m_pos[i] = 0;
		m_pos_press_off[i] = 0;
		m_pos_press[i] = 0;
		m_selecting[i] = false;
		m_touched_begin[i] = false;
		m_touched_end[i] = false;

		// Remove only enabled dimension
		if ( m_disabled[i] )
			continue;

		m_sel_begin[i] = 0;
		m_sel_end[i] = 0;
	}
	m_pressed = false;
	m_moving = false;
}

bool Selection::mouseMove(int _x, int _y)
{
	bool ret = false;
	m_pos[0] = _x;
	m_pos[1] = _y;

	if ( m_moving )
	{
		for ( int i = 0; i < 2; ++i )
		{
			if ( m_disabled[i] )
				continue;

			int size = m_sel_end[i] - m_sel_begin[i];
			m_sel_begin[i] = m_pos[i] - m_pos_press_off[i];
			m_sel_end[i] = m_sel_begin[i] + size;
			ret = true;
		}

		return ret;
	}

	// Selecting
	if ( m_pressed )
	{
		for ( int i = 0; i < 2; ++i)
		{
			if ( m_disabled[i] )
				continue;

			if ( ! m_selecting[i] )
				continue;

			if ( m_pos[i] <= m_pos_press[i] )
			{
				m_sel_begin[i] = m_pos[i];
				m_sel_end[i] = m_pos_press[i];
			}
			else
			{
				m_sel_begin[i] = m_pos_press[i];
				m_sel_end[i] = m_pos[i];
			}
			ret = true;
		}//for

		return ret;
	}


	// DETECT touching of the border.

	for ( int i = 0; i < 2; ++i)
	{
		m_touched_begin[i] = false;
		m_touched_end[i] = false;

		// if w*h == 0, then
		if ( (m_sel_end[0] - m_sel_begin[0]) && (m_sel_end[1] - m_sel_begin[1]) )
		{
			int other = 1 - i; //(0 == i)?1:0;

			// Inside of the limits of the other dimention?
			if ( m_pos[other] >= m_sel_begin[other] && m_pos[other] <= m_sel_end[other]  )
			{
				if ( 	m_pos[i] >= (m_sel_begin[i] - (int)m_border_feel_range[i]) &&
						m_pos[i] <= (m_sel_begin[i] + (int)m_border_feel_range[i]) )
				{
					m_touched_begin[i] = true;
					ret = true;
				}
				else
				if (	m_pos[i] >= (m_sel_end[i] - (int)m_border_feel_range[i]) &&
						m_pos[i] <= (m_sel_end[i] + (int)m_border_feel_range[i]) )
				{
					m_touched_end[i] = true;
					ret = true;
				}
			}
		}
	}

	return ret;
}

bool Selection::mousePress()
{
	bool ret = false;
	m_pressed = true;
	m_moving = false;

	// Is border touched my mouse?
	// If yes, then we start selecting process: dragging the touched borders.
	for ( int i = 0; i < 2; ++i)
	{
		m_selecting[i] = false;
		if ( m_disabled[i] )
			continue;

		if ( m_touched_begin[i] )
		{
			m_pos_press[i] = m_sel_end[i];
			m_selecting[i] = true;
			ret = true;
		}
		else
		if ( m_touched_end[i] )
		{
			m_pos_press[i] = m_sel_begin[i];
			m_selecting[i] = true;
			ret = true;
		}
	}

	// If not selecting, then:
	//    * if outside selection - reset old selection and create new selection
	//    * if inside selection - start moving the selection
	if ( ! m_selecting[0] && ! m_selecting[1] )
	{
		// Inside box?
		if ( 	m_pos[0] >= m_sel_begin[0] && m_pos[0] < m_sel_end[0] &&
				m_pos[1] >= m_sel_begin[1] && m_pos[1] < m_sel_end[1] )
		{
			// Start moving process:
			m_pos_press_off[0] = m_pos[0] - m_sel_begin[0];
			m_pos_press_off[1] = m_pos[1] - m_sel_begin[1];
			m_moving = true;
			ret = true;
			return ret;
		}

		if ( m_autocreation )
		{
			// Not inside box - create new selection.
			for ( int i = 0; i < 2; ++i )
			{
				if ( m_disabled[i] )
					continue;

				m_selecting[i] = true;
				m_pos_press[i] = m_sel_begin[i] = m_sel_end[i] = m_pos[i];
				m_touched_begin[i] = m_touched_end[i] = false;
			}
		}
	}
	return ret;
}

bool Selection::hit(int x, int y)
{
	// Inside box?
	if ( 	x >= m_sel_begin[0] && x < m_sel_end[0] &&
			y >= m_sel_begin[1] && y < m_sel_end[1] )
	{
		return true;
	}

	return false;
}

void Selection::mouseRelease()
{
	m_pressed = false;
	m_moving = false;

	for ( int i = 0; i < 2; ++i )
	{
		m_selecting[i] = false;
	}
}

void Selection::startMove()
{
	m_pos_press_off[0] = m_pos[0] - m_sel_begin[0];
	m_pos_press_off[1] = m_pos[1] - m_sel_begin[1];
	m_moving = true;
}

void Selection::startSelectionBegin(int i)
{
	m_moving = false;
	m_pressed = true;
	m_touched_begin[i] = true;
	m_touched_end[i] = false;

	m_pos_press[i] = m_sel_end[i];
	m_selecting[i] = true;
}

void Selection::startSelectionEnd(int i)
{
	m_moving = false;
	m_pressed = true;
	m_touched_begin[i] = false;
	m_touched_end[i] = true;

	m_pos_press[i] = m_sel_begin[i];
	m_selecting[i] = true;
}

bool Selection::noSelection()
{
	return ( 0 == ( m_sel_end[0] - m_sel_begin[0] ) || 0 == ( m_sel_end[1] - m_sel_begin[1] ) );
}

//void Selection::touched(int *_x, int *_y)
//{
//	*_x = m_touched_begin[0] | m_touched_end[0];
//	*_y = m_touched_begin[1] | m_touched_end[1];
//}

void Selection::disableX(bool _disable)
{
	m_disabled[0] = _disable;
}

void Selection::disableY(bool _disable)
{
	m_disabled[1] = _disable;
}

void Selection::setLimits( int _xmin, int _xmax, int _ymin, int _ymax )
{
	m_limits_enabled = true;
	m_limits[0][0] = _xmin;
	m_limits[0][1] = _xmax;
	m_limits[1][0] = _ymin;
	m_limits[1][1] = _ymax;
}

void Selection::setBorderFeelRangeX(unsigned _val)
{
	m_border_feel_range[0] = _val;
}

void Selection::setBorderFeelRangeY(unsigned _val)
{
	m_border_feel_range[1] = _val;
}

void Selection::enableAutocreation(bool _enabled)
{
	m_autocreation = _enabled;
}

void Selection::setX(size_t _pos, size_t _len)
{
	m_sel_begin[0] = _pos;
	m_sel_end[0] = _pos + _len;
}

void Selection::setY(size_t _pos, size_t _len)
{
	m_sel_begin[1] = _pos;
	m_sel_end[1] = _pos + _len;
}

int Selection::x() { return m_sel_begin[0]; }
int Selection::y() { return m_sel_begin[1]; }
int Selection::width() { return m_sel_end[0] - m_sel_begin[0]; }
int Selection::height() { return m_sel_end[1] - m_sel_begin[1]; }
