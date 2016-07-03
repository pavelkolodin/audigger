
#ifndef _TRACK_H_
#define _TRACK_H_

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
//#include "TrackInfo.h"

class CursorInfo
{
public:
	CursorInfo()
	{
		clear();
	}

	void clear()
	{
		m_seltime_start = 0;
		m_seltime_len = 0;
		m_selfreq_start = 0;
		m_selfreq_len = 0;
		m_pos = 0;
		m_selected = 0;
	}

	void setSelTime(size_t _start_frame, size_t _len_frames)
	{
		m_seltime_start = _start_frame;
		m_seltime_len = _len_frames;
	}

	void clearSelTime()
	{
		m_seltime_start = 0;
		m_seltime_len = 0;
	}

	void setSelFreq(unsigned _start_fftbin, unsigned _len_fftbins)
	{
		m_selfreq_start = _start_fftbin;
		m_selfreq_len = _len_fftbins;
	}

	void setCursorPos(size_t _pos_frame)
	{
		m_pos = _pos_frame;
	}

	size_t moveCursor( long _signed_value )
	{
		bool minus = ( _signed_value < 0 );
		size_t off = (size_t)std::abs( _signed_value );
		if ( minus )
		{
			if ( off > m_pos )
				m_pos = 0;
			else
				m_pos -= off;
		}
		else
		{
			m_pos += off;
		}
		return m_pos;
	}

	size_t m_seltime_start; // frames (stereosamples)
	size_t m_seltime_len; // frames (stereosamples)

	size_t m_selfreq_start; // frames (stereosamples)
	size_t m_selfreq_len; // frames (stereosamples)

	// cursor position.
	size_t m_pos;
	bool m_selected;
};

class TrackAbstract
{
public:
	enum Type
	{
		TYPE_UNDEFINED,
		TYPE_MARKS,
		TYPE_AUDIO,
		TYPE_TIMELINE
	};

	TrackAbstract()
	: m_revision(0)
	, m_id_db(0)
	, m_users(0)
	, m_anchor(0)
	{
		m_mutex_access.reset( new boost::mutex() );
	}

	virtual Type getType() const { return TYPE_UNDEFINED; }

	virtual void setName (const char *_name)
	{
		m_name = _name;
	}
	virtual const char* getName() const
	{
		return m_name.c_str();
	}

	// Return string
	virtual const char* getHash() const = 0;

	CursorInfo& getCursorInfo()
	{
		return m_cursor;
	}

	boost::mutex &getMutex()
	{
		return *m_mutex_access.get();
	}

	virtual void resetRevision() { m_revision = 0; }
	virtual void setRevision(unsigned _rev) { m_revision = _rev; }
	virtual unsigned getRevision() { return m_revision; }

	// It is important to set DBId.
	// If DBId is set, track is updated in DB,
	// else a new copy of it will be added to DB.
	void setDBId(unsigned _id) { m_id_db = _id; }
	unsigned getDBId() const { return m_id_db; }

	// undo(), redo() must increment revision (if there is something to undo() or redo()).
	void undo();
	void redo();

	bool getAnchor()				{ return m_anchor; }
	void setAnchor(bool _value)	{ m_anchor = _value; }

protected:
	boost::shared_ptr< boost::mutex > m_mutex_access;
	std::string m_name;
	CursorInfo m_cursor;


public:
	unsigned m_revision;
	// Id if THE track in database.
	unsigned m_id_db;
	// Used by WidgetTrackGroup to count WidgetTracks for
	// this track to delete the track after any Widgets (FFT,WAVE) pointing to this track are closed.
	size_t m_users;

	// m_anchor == true if track is included in a project (Group).
	// m_anchor == false: track is not a part of a project and will not be saved when user saves the project
	bool m_anchor;
};

#endif
