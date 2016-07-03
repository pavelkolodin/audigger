/*
 *  pavelkolodin@gmail.com
 */

#ifndef _Settings_H_
#define _Settings_H_

#include <boost/thread/mutex.hpp>
#include <boost/foreach.hpp>
#include <vector>
#include <map>
#include "json/json.h"
#include "AudioParams.h"
#include "defines.h"

namespace Settings
{

class SettingsAbstract
{
public:
	virtual ~SettingsAbstract() { }
	virtual void clear() = 0;
	virtual void save( Json::Value& _node ) = 0;
	virtual void load( Json::Value& _node ) = 0;

	unsigned loadUIntValue(Json::Value& _node, const char* _key, unsigned _default)
	{
		unsigned ret = _default;
		if ( Json::intValue == _node[ _key ].type() )
			ret = (_node[ _key ].asInt() > 0) ? _node[ _key ].asInt() : _default;
		else
		if ( Json::uintValue == _node[ _key ].type() )
			ret = _node[ _key ].asUInt();

		return ret;
	}

	int loadIntValue(Json::Value& _node, const char* _key, int _default)
	{
		unsigned ret = _default;
		if ( Json::intValue == _node[ _key ].type() )
			ret = _node[ _key ].asInt();
		else
		if ( Json::uintValue == _node[ _key ].type() )
			ret = (int)( _node[ _key ].asUInt() );

		return ret;
	}
};

class SettingsGui : public SettingsAbstract
{
public:
	static const char *NAME;
	static const char *KEY_WIDTH;
	static const char *KEY_HEIGHT;
	static const char *KEY_X;
	static const char *KEY_Y;
	static const char *KEY_FULLSCREEN;

	SettingsGui();
	virtual void clear();
	void validate();
	virtual void save( Json::Value& _node );
	virtual void load( Json::Value& _node );

public:
	unsigned m_width;
	unsigned m_height;
	int m_x;
	int m_y;
	bool m_fullscreen;
};

class SettingsToolBar : public SettingsAbstract
{
public:
	static const char *NAME;
	typedef std::map<std::string, unsigned> StatesType;
	typedef std::vector<std::string> LayoutType;
//	struct Tool
//	{
//		Tool(const std::string &_name, unsigned _value)
//		: name ( _name )
//		, value( _value )
//		{
//
//		}
//		std::string name;
//		unsigned value;
//	};
	SettingsToolBar();
	virtual void clear();
	virtual void save( Json::Value& _node );
	virtual void load( Json::Value& _node );
	void print();

public:
	LayoutType m_layout;
	StatesType m_states;
	bool m_enabled;
};

class SettingsBases : public SettingsAbstract
{
public:
	static const char *NAME;
	struct Base
	{
		//std::string name;
		std::string path;
	};

	SettingsBases();
	virtual void clear();
	virtual void save( Json::Value& _node );
	virtual void load( Json::Value& _node );

public:
	std::map<std::string, Base > bases;
	std::string name_default;
};

class SettingsAudioParams: public SettingsAbstract
{
public:
	static const char *NAME;
	static const char *KEY_RATE;
	static const char *KEY_BITS;
	static const char *KEY_CHANNELS;

	SettingsAudioParams();
	virtual void clear();
	virtual void save( Json::Value& _node );
	virtual void load( Json::Value& _node );

public:
	AudioParams audioparams;
};

class SettingsMisc: public SettingsAbstract
{
public:
	static const char *NAME;
	static const char *KEY_DEFAULT_FFTWIN;
	static const char *KEY_DEFAULT_STEP_FRAMES;

	SettingsMisc();
	virtual void clear();
	virtual void save( Json::Value& _node );
	virtual void load( Json::Value& _node );

public:
	unsigned m_default_fft_win;
	unsigned m_default_step_frames;
};


class Settings
{		
	public:
		Settings ( );
		virtual ~Settings ( );

		void save(const char *_filename = NULL);
		void load(const char *_filename = NULL);
		SettingsToolBar &getToolBar() { return m_toolbar; }
		SettingsGui &getGui() { return m_gui; }
		SettingsBases &getBases() { return m_bases; }
		SettingsAudioParams &getAudioParams() { return m_audioparams; }
		SettingsMisc &getMisc() { return m_misc; }

	private:
        boost::mutex m_mutex;

        SettingsToolBar m_toolbar;
        SettingsGui m_gui;
        SettingsBases m_bases;
        SettingsAudioParams m_audioparams;
        SettingsMisc m_misc;

        std::string m_filename;
};

}

#endif

