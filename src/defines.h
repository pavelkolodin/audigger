/*
 *  pavelkolodin@gmail.com
 */

#ifndef _defaults_H_
#define _defaults_H_

#define DEFAULT_FFT_WIN 2048
#define DEFAULT_STEP_FRAMES 256

#define FFT_WIN_MIN 128
#define FFT_WIN_MAX 32768
#define FFT_STEP_MIN 64
#define FFT_STEP_MAX 32768

#define DEFAULT_GUI_WIDTH 1024
#define DEFAULT_GUI_HEIGHT 768
#define DEFAULT_GUI_POS_X 0
#define DEFAULT_GUI_POS_Y 0
#define DEFAULT_GUI_FULLSCREEN false

#define DRAGGING_START_DISTANCE 4

#ifdef WIN32
	#define DEFAULT_OPEN_PATH_SND "C:\\"
	#define OPENFILES_MEDIA "Audio files (*.wav *.mp3 *.flac *.htk *.mat *.wave);;WAV files (*.wav *.wave);;MP3 files (*.mp3);;FLAC files (*.flac);;HTK files (*.htk);;All files (*.*)"
#else
	#define DEFAULT_OPEN_PATH_SND "/"
	#define OPENFILES_MEDIA "Audio files (*.wav *.mp3 *.flac *.htk *.mat *.wave);;WAV files (*.wav *.wave);;MP3 files (*.mp3);;FLAC files (*.flac);;HTK files (*.htk);;All files (*.*)"
#endif

#define OPENFILES_JSON "JSON files (*.json);;All files (*.*)"
#define OPENFILES_WAV "WAV files (*.wav *.wave);;All files (*.*)"

#define SETTINGS_TOOLBAR "toolbar"
#define SETTINGS_TOOLBAR_ENABLED "enabled"
#define SETTINGS_TOOLBAR_LAYOUT "layout"
#define SETTINGS_TOOLBAR_STATES "states"

#define SETTINGS_TOOLBAR_TOOL_SEPARATOR "separator"
#define SETTINGS_TOOLBAR_TOOL_NEW "new"
#define SETTINGS_TOOLBAR_TOOL_OPEN "open"
#define SETTINGS_TOOLBAR_TOOL_SAVE "save"
#define SETTINGS_TOOLBAR_TOOL_CLOSE "close"
#define SETTINGS_TOOLBAR_TOOL_COPY "copy"
#define SETTINGS_TOOLBAR_TOOL_PASTE "paste"
#define SETTINGS_TOOLBAR_TOOL_INSERT "insert"
#define SETTINGS_TOOLBAR_TOOL_PLAY "play"
#define SETTINGS_TOOLBAR_TOOL_STOP "stop"
#define SETTINGS_TOOLBAR_TOOL_HAND "hand"
#define SETTINGS_TOOLBAR_TOOL_SELECT_TIME "select_time"
#define SETTINGS_TOOLBAR_TOOL_SELECT_BOTH "select_both"
#define SETTINGS_TOOLBAR_TOOL_BANDPASS "bandbass"
#define SETTINGS_TOOLBAR_TOOL_BANDPASS_ZERO "bandbass_zero"
#define SETTINGS_TOOLBAR_TOOL_BANDSUPPRESS "bandsuppress"
#define SETTINGS_TOOLBAR_TOOL_PEN "pen"
#define SETTINGS_TOOLBAR_TOOL_ERASER "eraser"
#define SETTINGS_TOOLBAR_TOOL_INFO "info"
#define SETTINGS_TOOLBAR_TOOL_TIMESTEP "timestep"
#define SETTINGS_TOOLBAR_TOOL_CURSOR_ASYNC "cursor_async"

#endif

