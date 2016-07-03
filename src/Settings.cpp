/*
 *  pavelkolodin@gmail.com
 */

#include <fstream>
#include <fir/file/file.h>

#include "Settings.h"
#include "json/json.h"

#define RETURN_IF_INVALID_CHARP(x, ret) if (NULL == x) return ret; if ('/' != x[0] ) return ret;



namespace Settings
{
static const char *FILENAME_DEFAULT = ".audigger.json";


// SettingsToolbar
const char *SettingsToolBar::NAME = "toolbar";

// SettingsBases
const char *SettingsBases::NAME = "bases";

// SettingsGui
const char *SettingsGui::NAME = "gui";
const char *SettingsGui::KEY_WIDTH = "width";
const char *SettingsGui::KEY_HEIGHT = "height";
const char *SettingsGui::KEY_X = "x";
const char *SettingsGui::KEY_Y = "y";
const char *SettingsGui::KEY_FULLSCREEN = "fullscreen";

// SettingsAudioParams
const char *SettingsAudioParams::NAME = "audio_params";
const char *SettingsAudioParams::KEY_RATE = "rate";
const char *SettingsAudioParams::KEY_BITS = "bits";
const char *SettingsAudioParams::KEY_CHANNELS = "channels";

// SettingsMisc
const char *SettingsMisc::NAME = "misc";
const char *SettingsMisc::KEY_DEFAULT_FFTWIN = "default_fftwin";
const char *SettingsMisc::KEY_DEFAULT_STEP_FRAMES = "default_step_frames";

//
// SettingsGui
//

SettingsGui::SettingsGui()
{
	clear();
}

void SettingsGui::clear()
{
	m_width = DEFAULT_GUI_WIDTH;
	m_height = DEFAULT_GUI_HEIGHT;
	m_x = DEFAULT_GUI_POS_X;
	m_y = DEFAULT_GUI_POS_Y;
	m_fullscreen = DEFAULT_GUI_FULLSCREEN;
}

void SettingsGui::validate()
{

}

void SettingsGui::save(Json::Value& node)
{
	node[ KEY_WIDTH ] = (Json::Value::UInt) m_width;
	node[ KEY_HEIGHT ] = (Json::Value::UInt) m_height;
	node[ KEY_X ] = (Json::Value::Int) m_x;
	node[ KEY_Y ] = (Json::Value::Int) m_y;
	node[ KEY_FULLSCREEN ] = (Json::Value::Int) m_fullscreen;
}


void SettingsGui::load(Json::Value& _node)
{

	if ( Json::nullValue == _node.type() )
	{
		clear();
		return;
	}

	m_width = loadUIntValue(_node, KEY_WIDTH, DEFAULT_GUI_WIDTH);
	m_height = loadUIntValue(_node, KEY_HEIGHT, DEFAULT_GUI_HEIGHT);
	m_x = loadIntValue(_node, KEY_X, DEFAULT_GUI_POS_X);
	m_y = loadIntValue(_node, KEY_Y, DEFAULT_GUI_POS_Y);
	m_fullscreen = (int)loadIntValue(_node, KEY_Y, (int) DEFAULT_GUI_FULLSCREEN );
}

//
// SettingsToolBar
//

SettingsToolBar::SettingsToolBar()
{
	clear();
}

void SettingsToolBar::clear()
{
	m_layout.clear();
	m_layout.push_back(SETTINGS_TOOLBAR_TOOL_NEW);
	m_layout.push_back(SETTINGS_TOOLBAR_TOOL_SAVE);
	m_layout.push_back(SETTINGS_TOOLBAR_TOOL_SEPARATOR);
	m_layout.push_back(SETTINGS_TOOLBAR_TOOL_HAND);
	m_layout.push_back(SETTINGS_TOOLBAR_TOOL_SELECT_TIME);
	m_layout.push_back(SETTINGS_TOOLBAR_TOOL_SELECT_BOTH);
	m_layout.push_back(SETTINGS_TOOLBAR_TOOL_SEPARATOR);
	m_layout.push_back(SETTINGS_TOOLBAR_TOOL_STOP);
	m_layout.push_back(SETTINGS_TOOLBAR_TOOL_PLAY);
	m_layout.push_back(SETTINGS_TOOLBAR_TOOL_SEPARATOR);
	m_layout.push_back(SETTINGS_TOOLBAR_TOOL_COPY);
	m_layout.push_back(SETTINGS_TOOLBAR_TOOL_PASTE);
	m_layout.push_back(SETTINGS_TOOLBAR_TOOL_INSERT);
	m_layout.push_back(SETTINGS_TOOLBAR_TOOL_SEPARATOR);
	m_layout.push_back(SETTINGS_TOOLBAR_TOOL_CURSOR_ASYNC);
	m_layout.push_back(SETTINGS_TOOLBAR_TOOL_SEPARATOR);
	m_layout.push_back(SETTINGS_TOOLBAR_TOOL_TIMESTEP);
	m_layout.push_back(SETTINGS_TOOLBAR_TOOL_SEPARATOR);
	m_layout.push_back(SETTINGS_TOOLBAR_TOOL_INFO);

	m_states[ SETTINGS_TOOLBAR_TOOL_TIMESTEP ] = DEFAULT_STEP_FRAMES;
	m_enabled = true;
}

void SettingsToolBar::save( Json::Value& node )
{
	node[ SETTINGS_TOOLBAR_ENABLED ] = m_enabled;

	BOOST_FOREACH( const std::string &t, m_layout)
	{
		//tool = t;
		node[SETTINGS_TOOLBAR_LAYOUT].append( Json::Value(t) );
	}

	BOOST_FOREACH( const StatesType::value_type &t, m_states)
	{
		node[ SETTINGS_TOOLBAR_STATES ][ t.first ] = (Json::UInt) t.second;
	}
}

void SettingsToolBar::load( Json::Value& _node)
{
	clear();
	if ( Json::nullValue == _node.type() )
		return;

	if ( Json::booleanValue == _node[ SETTINGS_TOOLBAR_ENABLED ].type() )
		m_enabled = _node[ SETTINGS_TOOLBAR_ENABLED ].asBool();


	if ( Json::arrayValue == _node[ SETTINGS_TOOLBAR_LAYOUT ].type() )
	{
		m_layout.clear();

		const Json::Value &layout = _node[ SETTINGS_TOOLBAR_LAYOUT ];
		for ( Json::UInt i = 0; i < layout.size(); ++i )
		{
			m_layout.push_back( layout[i].asString() );


			if ( Json::nullValue == _node[ SETTINGS_TOOLBAR_STATES ][layout[i].asString()].type() )
				continue;

			m_states[ layout[i].asString() ] = _node[ SETTINGS_TOOLBAR_STATES ][layout[i].asString()].asUInt();
		}
	}
}

void SettingsToolBar::print()
{
	BOOST_FOREACH ( const LayoutType::value_type &_v, m_layout )
	{
		if ( _v.size() < 80 )
		{
			LOG_PURE ( _v );
			StatesType::const_iterator itf = m_states.find( _v );
			if ( m_states.end() != itf )
				LOG_PURE ( "state " << itf->second );
		}
		else
		{
			LOG_PURE ( "size() >= 80" );
		}
	}
}

//
// SettingsBases
//

SettingsBases::SettingsBases()
{
	clear();
}

void SettingsBases::clear()
{
}

void SettingsBases::load( Json::Value& _node )
{

}

void SettingsBases::save( Json::Value& _node )
{

}

//
// SettingsAudioParams
//

SettingsAudioParams::SettingsAudioParams()
{
	clear();
}

void SettingsAudioParams::clear()
{
	audioparams.clear();
}

void SettingsAudioParams::load( Json::Value& _node )
{

}

void SettingsAudioParams::save( Json::Value& _node )
{

}

//
// SettingsMisc
//

SettingsMisc::SettingsMisc()
{
	clear();
}

void SettingsMisc::clear()
{
	m_default_fft_win = 2048;
	m_default_step_frames = 256;
}

void SettingsMisc::save(Json::Value& node)
{
	node[ KEY_DEFAULT_FFTWIN ] = (Json::Value::UInt) m_default_fft_win;
	node[ KEY_DEFAULT_STEP_FRAMES ] = (Json::Value::UInt) m_default_step_frames;
}

void SettingsMisc::load(Json::Value& _node)
{
	clear();
	if ( Json::nullValue == _node.type() )
		return;

	if ( Json::intValue == _node[ KEY_DEFAULT_FFTWIN ].type() )
		m_default_fft_win = _node[ KEY_DEFAULT_FFTWIN ].asUInt();

	if ( Json::intValue == _node[ KEY_DEFAULT_STEP_FRAMES ].type() )
		m_default_step_frames = _node[ KEY_DEFAULT_STEP_FRAMES ].asUInt();
}



Settings::Settings ( )
{
	fir::file::detectHomeDir( m_filename );
	m_filename += "/";
	m_filename += FILENAME_DEFAULT;

	try
	{
		load();
	} catch( ... )
	{
	}

	//m_toolbar.print();
}

Settings::~Settings ( )
{
	save();
}

void Settings::save(const char *_filename )
{
	if ( _filename )
		m_filename = _filename;

	Json::StyledWriter writer;
	Json::Value root;

	// ToolBar
	m_toolbar.save( root[ SettingsToolBar::NAME ] );
	m_gui.save( root[ SettingsGui::NAME ] );
	m_bases.save( root[SettingsBases::NAME] );
	m_audioparams.save( root[ SettingsAudioParams::NAME ] );
	m_misc.save( root[SettingsMisc::NAME] );

	std::string outputConfig = writer.write( root );
	std::fstream f1( m_filename.c_str(), std::ios_base::out | std::ios_base::binary);
	f1.write( outputConfig.c_str(), outputConfig.size() );
	f1.close();
}

void Settings::load(const char *_filename )
{
	if ( _filename )
		m_filename = _filename;

	Json::Reader reader;
	Json::Value root;

	std::fstream f1( m_filename.c_str(), std::ios_base::in | std::ios_base::binary );

	if ( ! reader.parse( f1, root ) )
		throw std::runtime_error("cannot write json document");

	m_toolbar.load( root[ SettingsToolBar::NAME ] );
	m_gui.load( root[ SettingsGui::NAME ] );
	m_bases.load( root[ SettingsBases::NAME ] );
	m_audioparams.load( root[SettingsAudioParams::NAME] );
	m_misc.load( root[SettingsMisc::NAME] );
}


}

