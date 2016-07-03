/*
 *  pavelkolodin@gmail.com
 */

#ifndef _WidgetTrackTimeLine_H_
#define _WidgetTrackTimeLine_H_

#include <boost/noncopyable.hpp>
#include "WidgetTrackAbstract.h"
#include "TrackTimeLine.h"

class WidgetTrackTimeLine : public WidgetTrackAbstract, boost::noncopyable
{
	Q_OBJECT
	public:

		WidgetTrackTimeLine ( QWidget *_parent, WidgetTrackAbstract *_wtg, Controller *_ctrl, TrackTimeLine::Ptr _track );
		virtual ~WidgetTrackTimeLine ( );

		virtual TrackType getType() { return TYPE_TIMELINE; }
		virtual void refresh();
		virtual void setCursorPos(TrackPos _pos);
		virtual void setOffset( TrackPos _off );
		virtual void setStepFrames( unsigned _step );

	protected:
		virtual void paintEvent(QPaintEvent *);

	private:

		void updateData__()
		{
			// Usually, WidgetTrackTimeLine is added before any audio tracks added to group.
			// So, at that time group has no framerate and other audio parameters,
			// so we unable to calculate time.
			// With following code we trying to calculate time again.
			if ( ! m_interval_ms )
				setStepFrames( m_step_frames );
		}

		TrackPos m_pos;
		double m_ms_per_pixel;
		unsigned long m_interval_ms;
};

#endif

