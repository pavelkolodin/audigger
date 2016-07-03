/*
 *  pavelkolodin@gmail.com
 */

#ifndef _IMarksOperator_H_
#define _IMarksOperator_H_

#include <QtWidgets>
#include "Controller.h"
#include "Settings.h"
#include "Selection.h"

class WidgetTrackAbstract : public QWidget
{
	Q_OBJECT
	public:
		typedef size_t TrackPos;
		typedef long TrackPosSigned;
		enum TrackType
		{
			TYPE_UNDEFINED
			, TYPE_GROUP
			, TYPE_MARKS
			, TYPE_SONOGRAM
			, TYPE_WAVE
			, TYPE_TIMELINE
		};

		enum MouseMode
		{
			MM_DRAG
			, MM_SELECTTIME
			, MM_SELECTBOTH
			, MM_PEN
			, MM_ERASER
		};

		WidgetTrackAbstract( QWidget *_parent, TrackAbstract *_track, WidgetTrackAbstract *_container, Controller *_ctrl )
		: QWidget( _parent )
		, m_track_base ( _track )
		, m_wtg ( _container )
		, m_ctrl ( _ctrl )
		, m_cursor_pos(0)
		, m_offset(0)
		, m_view_pos(0)
		, m_canvas_width(0)
		, m_canvas_height(0)
		, m_step_frames(1)
		, m_revision(0)
		, m_mouse_curr_coord(-1)
		, m_mode( getMouseModeFromSettings() )
		, m_mouse_pressed_here(false)
		{

		}

		virtual TrackType getType() { return TYPE_UNDEFINED; }

		// Track will return non-zero value, group will return NULL
		virtual TrackAbstract* getTrackAbstract() { return m_track_base; }
		virtual TrackAudio::Ptr getTrackAudio() { return TrackAudio::Ptr(); }
		virtual TrackMarks::Ptr getTrackMarks() { return TrackMarks::Ptr(); }

		// Groups must return non-zero value, track will return NULL
		virtual TracksGroup::Ptr getGroup() { return TracksGroup::Ptr(); }

		virtual void resetRevision() { m_revision = 0; }
		virtual void setRevision(unsigned _rev) { m_revision = _rev; }
		virtual unsigned getRevision() { return m_revision; }

		virtual void refresh() { }
		virtual TrackPos getLenPixels() { return 0; }

		// ViewPos
		// the frame displayed at this potision now must be
		// displayed on this position always (after thanging zoom (StepFrames).
		virtual void setViewPos(TrackPos _pos) { m_view_pos = _pos; };
		virtual TrackPos getViewPos() { return m_view_pos; }
		virtual void setOffset( TrackPos _off ) { }
		virtual TrackPos getOffset() { return m_offset; }

		// Move cursor.
		virtual TrackPos moveCursor( TrackPosSigned _pixels ) { return 0; }


		// Refresh range of lines.
		virtual void refreshRange( TrackPos _from, TrackPos _len ) { }

		// Set mode of mouse cursor: dragging, time-selection, frequency-time-selection.
		virtual void setMouseMode(WidgetTrackAbstract::MouseMode _mode)
		{
			switch (_mode)
			{
			default:
				break;

			case WidgetTrackAbstract::MM_DRAG:
				setCursor( Qt::OpenHandCursor );
				break;

			case WidgetTrackAbstract::MM_SELECTTIME:
				setCursor( Qt::ArrowCursor );
				m_sel.setX(0, m_canvas_width);
				m_sel.disableX(true);
				break;

			case WidgetTrackAbstract::MM_SELECTBOTH:
				setCursor( Qt::ArrowCursor );
				m_sel.disableX(false);
				break;
			}

			m_mode = _mode;
		}

		// Used only by WidgetTrackSonogram or so.
		virtual void setFFTWin( unsigned _win ) { }

		// how much frames is in one line?
		virtual void setStepFrames( unsigned _step )
		{
			if ( ! _step )
				return;

			m_sel.setBorderFeelRangeY( _step * 4 );

			if ( m_step_frames )
			{
				// Hold view position:
				size_t pivot_frame = 0;
				if ( -1 != m_mouse_curr_coord )
					pivot_frame = m_offset * m_step_frames + m_mouse_curr_coord * m_step_frames ;
				else
					pivot_frame = m_view_pos * m_step_frames;

				TrackPosSigned diff = (pivot_frame / m_step_frames) - m_offset;
				TrackPosSigned val = (pivot_frame / _step);

				m_offset = (TrackPos) ((val > diff)?val - diff : 0);
			}

			m_step_frames = _step;
			resetRevision();
			refresh();
		}

		virtual unsigned getStepFrames( ) { return 0; }
		virtual bool hasMouse( )
		{
			return ( -1 != m_mouse_curr_coord );
		}

		//
		// group functionality (WidgetTracksContainer notification).
		//

		virtual void setSelectedChild( WidgetTrackAbstract* _track, bool _selected ) { }

		MouseMode getMouseModeFromSettings()
		{

			if ( m_ctrl->getSettings().getToolBar().m_states[ SETTINGS_TOOLBAR_TOOL_HAND ] )
				return MM_DRAG;

			if ( m_ctrl->getSettings().getToolBar().m_states[ SETTINGS_TOOLBAR_TOOL_SELECT_TIME ] )
				return MM_SELECTTIME;

			if ( m_ctrl->getSettings().getToolBar().m_states[ SETTINGS_TOOLBAR_TOOL_SELECT_BOTH ] )
				return MM_SELECTBOTH;

			return MM_SELECTTIME;
		}

		TrackPos offsetForPos(TrackPos _pos)
		{
			unsigned coord_min = height()/5;
			unsigned coord_max = height() - height()/5;

			TrackPos vp = (_pos - m_offset);

			if ( vp < coord_min )
			{
				if ( m_offset >= (coord_min - vp) )
					return m_offset - (coord_min - vp);

				return 0;
			}

			if ( vp > coord_max )
				return _pos - coord_max;

			return m_offset;
		}

protected:
		virtual void leaveEvent(QEvent * event)
		{
			m_mouse_curr_coord = -1;
		}

		virtual void mousePressEvent(QMouseEvent* _event)
		{
			m_mouse_pressed_here = true;
			if (Qt::LeftButton == _event->button())
			{
				if ( WidgetTrackAbstract::MM_PEN == m_mode )
				{
					//mousePen__( _event );
				}
				else
				{
					if ( WidgetTrackAbstract::MM_SELECTTIME == m_mode || WidgetTrackAbstract::MM_SELECTBOTH == m_mode )
					{
						// TODO: doubling code
						unsigned pos[2];
						getGlobalPos__( _event->x(), _event->y(), &pos[0], &pos[1] );
						m_cursor_pos = pos[1];
						sendCursorPos__( pos[1] );

						m_sel.mouseMove( pos[0], pos[1] * m_step_frames );
						m_sel.mousePress();

						sendSelection__();

						// m_wtg may be == NULL if this WidgetTrackAbstract is WidgetTracksGroup.
						if ( m_wtg )
							m_wtg->refresh();
					}
					else
					if ( WidgetTrackAbstract::MM_DRAG == m_mode )
					{
						// dragging.
						setCursor( Qt::ClosedHandCursor );
					}
				}
			}
		}

		virtual void mouseReleaseEvent(QMouseEvent *_event)
		{
			m_mouse_pressed_here = false;

			m_sel.mouseRelease();

			if ( WidgetTrackAbstract::MM_DRAG == m_mode )
			{
				setCursor( Qt::OpenHandCursor );
			}

			sendSelection__();
			if ( m_wtg )
			 m_wtg->refresh();
		}

		virtual void mouseMoveEvent(QMouseEvent *_event)
		{
			m_mouse_curr_coord = _event->y();

			if ( WidgetTrackAbstract::MM_SELECTTIME == m_mode || WidgetTrackAbstract::MM_SELECTBOTH == m_mode )
			{
				unsigned pos[2];
				getGlobalPos__( _event->x(), _event->y(), &pos[0], &pos[1] );
				m_sel.mouseMove( pos[0], pos[1] * m_step_frames );

				if ( 	(m_sel.mouseOverBeginX() && m_sel.mouseOverBeginY() ) ||
						(m_sel.mouseOverEndX() && m_sel.mouseOverEndY() ) )
					setCursor( Qt::SizeFDiagCursor );
				else
				if ( 	(m_sel.mouseOverBeginX() && m_sel.mouseOverEndY() ) ||
						(m_sel.mouseOverEndX() && m_sel.mouseOverBeginY() ) )
					setCursor( Qt::SizeBDiagCursor );
				else
				if ( m_sel.mouseOverBeginX() || m_sel.mouseOverEndX() )
					setCursor( Qt::SizeHorCursor );
				else
				if ( m_sel.mouseOverBeginY() || m_sel.mouseOverEndY() )
					setCursor( Qt::SizeVerCursor );
				else
					setCursor( Qt::ArrowCursor );

				if (_event->buttons() & Qt::LeftButton)
				{
					sendSelection__();
					//signalVisualsChanged();
					update();
				}
			}
		}

		void sendCursorPos__(TrackPos _pos)
		{
			if ( m_ctrl->getSettings().getToolBar().m_states[ SETTINGS_TOOLBAR_TOOL_CURSOR_ASYNC ] )
			{
				// Only for my track.

				// WidgetTracksGroup IS WidgetTrackAbstrack, but has no track, so check.
				if ( m_track_base )
					m_track_base->getCursorInfo().setCursorPos( _pos * m_step_frames );
			}
			else
			{
				// To all tracks in group.
				// WidgetTracksGroup IS WidgetTrackAbstrack
				if ( m_wtg )
					m_wtg->getGroup()->setCursorPos( _pos * m_step_frames );
			}
		}

		virtual void sendSelection__()
		{
			if ( ! m_track_base )
				return;

			// Time selection
			m_track_base->getCursorInfo().setSelTime( m_sel.y(), m_sel.height() );

			// Freq selection
			if ( WidgetTrackAbstract::MM_SELECTBOTH == m_mode )
				m_track_base->getCursorInfo().setSelFreq( m_sel.x(), m_sel.width() );
			else
				m_track_base->getCursorInfo().setSelFreq( 0, 0 );
		}

		inline void getGlobalPos__(int _x, int _y, unsigned *_X, unsigned *_Y)
		{
			*_X = ( _x >= 0 )?_x:0;

			if ( *_X >= m_canvas_width )
				*_X = m_canvas_width - 1;

			*_Y = (((int)m_offset + _y) >=0 ) ? m_offset + _y : 0;

			if ( *_Y >= m_canvas_height )
				*_Y = m_canvas_height - 1;
		}

		TrackAbstract* m_track_base;
		WidgetTrackAbstract *m_wtg;
		Controller *m_ctrl;

		// Cursor position (pixels), used for drawing only.
		WidgetTrackAbstract::TrackPos m_cursor_pos;
		// The track is displayed starting from m_offset pixel.
		WidgetTrackAbstract::TrackPos m_offset;
		WidgetTrackAbstract::TrackPos m_view_pos;

		unsigned m_canvas_width;
		unsigned m_canvas_height;

		// Amount of frames (multi-channel samples) in one pixel.
		unsigned m_step_frames;
		unsigned m_revision;
		int m_mouse_curr_coord;
		Selection m_sel;
		WidgetTrackAbstract::MouseMode m_mode;
		bool m_mouse_pressed_here;
};


#endif

