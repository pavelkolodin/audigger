/*
 * pavelkolodin@gmail.com
 *
 */

#include "ToolBar.h"
#include "Settings.h"
#include "defines.h"

namespace ToolBar
{
	struct ToolInfo global_toolsinfo[] =
	{
			{ ToolBar::TOOL_SEPARATOR,			ToolBar::WT_BUTTON, 0, false, false, 0, false, ":/images/tools/separator-16x24.png", 16, 24, "Separator", SETTINGS_TOOLBAR_TOOL_SEPARATOR }
			, { ToolBar::TOOL_NEW,				ToolBar::WT_BUTTON, 0, false, false, 0, false, ":/images/tools/new-24x24.png", 24, 24, "New", SETTINGS_TOOLBAR_TOOL_NEW}
			, { ToolBar::TOOL_OPEN,				ToolBar::WT_BUTTON, 0, false, false, 0, false, ":/images/tools/open-24x24.png", 24, 24, "Open", SETTINGS_TOOLBAR_TOOL_OPEN}
			, { ToolBar::TOOL_SAVE,				ToolBar::WT_BUTTON, 0, false, false, 0, false, ":/images/tools/save-24x24.png", 24, 24, "Save", SETTINGS_TOOLBAR_TOOL_SAVE}
			, { ToolBar::TOOL_CLOSE,			ToolBar::WT_BUTTON, 0, false, false, 0, false, ":/images/tools/remove-24x24.png", 24, 24, "Close", SETTINGS_TOOLBAR_TOOL_CLOSE}
			, { ToolBar::TOOL_COPY,				ToolBar::WT_BUTTON, 0, false, false, 0, false, ":/images/tools/copy-24x24.png", 24, 24, "Copy", SETTINGS_TOOLBAR_TOOL_COPY}
			, { ToolBar::TOOL_PASTE,			ToolBar::WT_BUTTON, 0, false, false, 0, false, ":/images/tools/paste-24x24.png", 24, 24, "Paste", SETTINGS_TOOLBAR_TOOL_PASTE}
			, { ToolBar::TOOL_INSERT,			ToolBar::WT_BUTTON, 1, true, false, 0, false, ":/images/tools/insert-24x24.png", 24, 24, "Insert mode", SETTINGS_TOOLBAR_TOOL_INSERT }
			, { ToolBar::TOOL_PLAY,				ToolBar::WT_BUTTON, 0, true, false, 1, true, ":/images/tools/play-24x24.png", 24, 24, "Play", SETTINGS_TOOLBAR_TOOL_PLAY}
			, { ToolBar::TOOL_STOP,				ToolBar::WT_BUTTON, 1, true, false, 1, true, ":/images/tools/stop-24x24.png", 24, 24, "Stop", SETTINGS_TOOLBAR_TOOL_STOP}
			, { ToolBar::TOOL_HAND,				ToolBar::WT_BUTTON, 0, true, false, 2, true, ":/images/tools/hand-24x24.png", 24, 24, "Drag", SETTINGS_TOOLBAR_TOOL_HAND }
			, { ToolBar::TOOL_SELECT_TIME,		ToolBar::WT_BUTTON, 1, true, false, 2, true, ":/images/tools/select-time-24x24.png", 24, 24, "Select time", SETTINGS_TOOLBAR_TOOL_SELECT_TIME }
			, { ToolBar::TOOL_SELECT_BOTH,		ToolBar::WT_BUTTON, 0, true, false, 2, true, ":/images/tools/select-both-24x24.png", 24, 24, "Select 2D", SETTINGS_TOOLBAR_TOOL_SELECT_BOTH }
			, { ToolBar::TOOL_BANDPASS,			ToolBar::WT_BUTTON, 0, true, false, 3, false, ":/images/tools/band-pass-24x24.png", 24, 24, "Bandpass filter", SETTINGS_TOOLBAR_TOOL_BANDPASS }
			, { ToolBar::TOOL_BANDPASS_ZERO,	ToolBar::WT_BUTTON, 0, true, false, 3, false, ":/images/tools/band-pass-zero-24x24.png", 24, 24, "FFT left shift", SETTINGS_TOOLBAR_TOOL_BANDPASS_ZERO }
			, { ToolBar::TOOL_BANDSUPPRESS,		ToolBar::WT_BUTTON, 0, true, false, 3, false, ":/images/tools/band-suppress-24x24.png", 24, 24, "Band suppress filter", SETTINGS_TOOLBAR_TOOL_BANDSUPPRESS }
			, { ToolBar::TOOL_PEN,				ToolBar::WT_BUTTON, 0, true, false, 4, false, ":/images/tools/pen-green-visualpharm-24x24.png", 24, 24, "Pen", SETTINGS_TOOLBAR_TOOL_PEN }
			, { ToolBar::TOOL_ERASER,			ToolBar::WT_BUTTON, 0, true, false, 4, false, ":/images/tools/eraser-24x24.png", 24, 24, "Eraser", SETTINGS_TOOLBAR_TOOL_ERASER }
			, { ToolBar::TOOL_INFO,				ToolBar::WT_BUTTON, 0, false, false, 0, false, ":/images/tools/info-24x24.png", 24, 24, "Info/help", SETTINGS_TOOLBAR_TOOL_INFO}
			, { ToolBar::TOOL_TIMESTEP,			ToolBar::WT_DIAL, DEFAULT_STEP_FRAMES, false, false, 0, false, ":/images/tools/timestep-100x24.png", 100, 24, "Time step", SETTINGS_TOOLBAR_TOOL_TIMESTEP }
			, { ToolBar::TOOL_CURSOR_ASYNC,		ToolBar::WT_BUTTON, 0, true, false, 0, false, ":/images/tools/cursor-sync-24x24.png", 24, 24, "Cursor sync", SETTINGS_TOOLBAR_TOOL_CURSOR_ASYNC }
			, { ToolBar::CONTROL_TYPE_LIMIT,	ToolBar::WT_NONE, 0, false, false, 0, false, "", 0, 0, "", NULL}
	};
}
