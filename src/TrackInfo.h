// pavelkolodin

#ifndef _TRACKINFO_H_
#define _TRACKINFO_H_

#include "TrackAudio.h"
#include "TrackMarks.h"
#include <boost/thread/mutex.hpp>

class TrackInfo
{
public:
	enum TrackType
	{
		UNDEFINED = 0,
		PLACEHOLDER,
		AUDIO,
		MARKS
	};

	TrackInfo(TrackType _type = UNDEFINED)
	: m_type ( _type )
	, m_mutex_access( new boost::mutex )
	{
		clear();
	}

	void clear()
	{
		m_id = 0;

		if ( PLACEHOLDER == m_type )
			return;

		if ( TrackInfo::AUDIO == m_type )
			delete m_track_audio;

		if ( TrackInfo::MARKS == m_type )
			delete m_track_marks;

		m_type = UNDEFINED;
		m_track_audio = NULL;
		m_track_marks = NULL;

		m_cursor.clear();
	}

	bool isUsed()
	{
		return (UNDEFINED != m_type);
	}

	unsigned m_id;
	TrackType m_type;
	TrackAudio *m_track_audio;
	TrackMarks *m_track_marks;
	CursorInfo m_cursor;

	boost::shared_ptr< boost::mutex > m_mutex_access;
};

#endif
