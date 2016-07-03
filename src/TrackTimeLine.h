/*
 *  pavelkolodin@gmail.com
 * 2013-08-12, 00:57
 */

#ifndef _TrackTimeLine_H_
#define _TrackTimeLine_H_

#include <boost/shared_ptr.hpp>
#include "TrackAbstract.h"

class TrackTimeLine : public TrackAbstract
{		
	public:
		typedef boost::shared_ptr<TrackTimeLine> Ptr;

		TrackTimeLine ( );
		virtual ~TrackTimeLine ( );

		virtual Type getType() const { return TYPE_TIMELINE; }
		virtual const char* getHash() const { return ""; }

	private:
        TrackTimeLine ( const TrackTimeLine& _other ) { }
        TrackTimeLine& operator= ( const TrackTimeLine& _other ) { return *this; }
};

#endif

