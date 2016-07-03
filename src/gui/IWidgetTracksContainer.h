/*
 *  pavelkolodin@gmail.com
 */

#ifndef IWidgetTracksContainer_H_
#define IWidgetTracksContainer_H_

#include "TrackAudio.h"
#include "TrackMarks.h"

class IWidgetTrack;

class IWidgetTracksContainer
{		
	public:
		virtual void trackChanged( IWidgetTrack *_whose ) { }
		//virtual void trackCreated( TrackAudio::Ptr ) = 0;
		//virtual void trackCreated( TrackMarks::Ptr ) = 0;
};

#endif

