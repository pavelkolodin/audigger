/*
 * pavelkolodin@gmail.com
 *
 */

#ifndef _TOOLBAR_H_
#define _TOOLBAR_H_

#include <stddef.h>

namespace ToolBar
{
	static const int WIDTH_LABEL = 150;
	// TBA - tools bank account
	static const int TBA_PADDING = 2;
	static const int TBA_SPACING = 3;
	static const int WIDTH_TBA_TEXTMARGIN = 3;

	enum ToolType
	{
		TOOL_SEPARATOR
		, TOOL_NEW
		, TOOL_OPEN
		, TOOL_SAVE
		, TOOL_CLOSE
		, TOOL_COPY
		, TOOL_PASTE
		, TOOL_INSERT
		, TOOL_PLAY
		, TOOL_STOP
		, TOOL_HAND
		, TOOL_SELECT_TIME
		, TOOL_SELECT_BOTH
		, TOOL_BANDPASS
		, TOOL_BANDPASS_ZERO
		, TOOL_BANDSUPPRESS
		, TOOL_PEN
		, TOOL_ERASER
		, TOOL_INFO
		, TOOL_TIMESTEP
		, TOOL_CURSOR_ASYNC
		, CONTROL_TYPE_LIMIT
	};

	enum WidgetType
	{
		WT_NONE,
		WT_BUTTON,
		WT_DIAL
	};

struct ToolInfo
{
	ToolType m_type;
	WidgetType m_type_widget;
	unsigned m_default_value;

	// true: Tool can be in 2 states: checked, unchecked.
	// false: Tool cannot be checked/unchecked (ordinary button)
	const bool m_checkable;

	// true: Tool type: unsigned
	// false: Tool type: bool
	const bool m_regulator;

	// Only 1 button in the group can be in checked state.
	const unsigned m_group_id;

	// At least one button in group must be checked.
	const bool m_group_oneof;

	const char *m_icon_filename;
	const int m_icon_width;
	const int m_icon_height;

	// Text name of the toolbar.
	const char *m_label;

	// Name for settings file.
	const char *m_settings_name;
};



// Not used (as i know)
template < typename T_CONTAINER >
void deployWidgets( T_CONTAINER &_vector, int w, int h, int margin )
{
	int x = 0;
	int y = 0;

	typename T_CONTAINER::iterator it, ite;
	it = _vector.begin();
	ite = _vector.end();
	int max_height = 0;
	for ( ;it != ite; ++it )
	{
		if ( x && (x + (*it) -> width()) > w )
		{
			y += max_height;
			y += margin;
			max_height = 0;
			x = 0;
		}

		(*it) -> move( x, y );
		x += (*it) -> width();
		x += margin;
		if ( max_height < (*it) -> height() )
			max_height = (*it) -> height();

		//if ( y >= h)
			//break;
	}
}

}

#endif

