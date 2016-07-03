/*
 *  pavelkolodin@gmail.com
 */

#ifndef _ITracksGroupClient_H_
#define _ITracksGroupClient_H_

#include "TrackAudio.h"
#include "TrackMarks.h"
#include "TrackTimeLine.h"

class IWidgetTrack;

class ITracksGroupClient
{		
	public:
		virtual void trackChanged( IWidgetTrack *_whose ) { }
		virtual void trackCreated( TrackAudio::Ptr ) = 0;
		virtual void trackCreated( TrackMarks::Ptr ) = 0;
};

#endif

