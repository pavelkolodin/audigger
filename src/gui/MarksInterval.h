
#pragma once

#ifndef _MarksInterval_H_
#define _MarksInterval_H_

#include "MarkGUI.h"
#include "TrackMarks.h"

// Time interval with marks placed in this interval
class MarksInterval
{
private:
	TrackMarks::Ptr m_track;
	std::vector <Mark> m_marks_mem;
	std::vector <MarkGUI> m_marks_gui_mem;

	//size_t m_size; // how many marks of the storage are used?
	size_t m_used;
	int m_width, m_height;
	unsigned m_step_frames;
	WidgetTrackAbstract::TrackPos m_offset;

public:
	// We need copy of moving/resizing mark, because user
	// may scroll track by wheel, which will cause refresh
	// and loose all the marks we have in current interval.
	Mark m_indep_mark;
	MarkGUI m_indep_markgui;

public:
	MarksInterval (TrackMarks::Ptr _track)
	: m_track( _track )
	, m_used(0)
	, m_width(0)
	, m_height(0)
	, m_step_frames(0)
	, m_offset(0)
	{
		m_indep_markgui.init( m_indep_mark, m_step_frames );
	}

	void setOffset(WidgetTrackAbstract::TrackPos _off)
	{
		m_offset = _off;
	}

	void setStepFrames( unsigned _step )
	{
		m_step_frames = _step;
	}

	void load(Mark::TimeValue _begin, size_t _size )
	{
		if ( ! m_step_frames )
			return;

		reserve ( _size );

		m_used = m_track->loadMarks( _begin, m_step_frames, &m_marks_mem[0], capacity() );

		for ( size_t i = 0; i < m_used; ++i )
		{
			m_marks_gui_mem[i].init(m_marks_mem[i], m_step_frames);
			m_marks_gui_mem[i].setWidth( m_width );
		}
	}

	void resize(int w, int h)
	{
		m_width = w;
		m_height = h;
		for ( size_t i = 0; i < m_used; ++i )
			m_marks_gui_mem[i].setWidth(w);
	}

	void reserve(size_t _size)
	{
		if ( _size < m_marks_mem.capacity() )
			return;

		m_marks_mem.resize( _size );
		m_marks_gui_mem.resize( _size );
	}

	size_t capacity()
	{
		return m_marks_mem.capacity();
	}

	size_t used()
	{
		return m_used;
	}

	void setUsed(size_t _used)
	{
		m_used = _used;
	}

//	void deselectAll()
//	{
//		for ( size_t i = 0; i < m_used; ++i )
//			m_marks_mem[i].m_selected = false;
//	}

	Mark& getMark(size_t i)
	{
		return m_marks_mem[i];
	}

	MarkGUI& getMarkGUI(size_t i)
	{
		return m_marks_gui_mem[i];
	}

	void resetTextSize()
	{
		for ( size_t i = 0; i < m_used; ++i )
			m_marks_gui_mem[ i ].m_label_height = 0;
	}

	MarkGUI *hit(int x, int y)
	{
		for ( size_t i = 0; i < m_used; ++i )
		{
			bool r = m_marks_gui_mem[ i ].hit( x, m_offset + y );
			if ( r )
				return &m_marks_gui_mem[ i ];
		}
		return NULL;
	}

	void setSelected(size_t i, bool _value)
	{
		m_marks_gui_mem[ i ].m_selected = _value;
		m_track->setMarkSelected( m_marks_mem[i].m_pos, _value );
	}

	MarkGUI *mouseMove(int x, int y)
	{
		// Moving, resizing mark.
		if ( m_indep_markgui.changingInProgress() )
		{
			m_indep_markgui.mouseMove(x, m_offset + y);

			Mark::TimeValue pos = m_indep_markgui.m_coord * m_step_frames;
			Mark::TimeValue len = m_indep_markgui.m_len * m_step_frames;
			m_indep_markgui.m_place_bad = (! m_track->canAddMark(pos, len) );

			// need for displaying purposes:
			const_cast<Mark*>(m_indep_markgui.m_mark)->m_pos = pos;
			const_cast<Mark*>(m_indep_markgui.m_mark)->m_len = len;

			return &m_indep_markgui;
		}

		// Search for a mark that user wants to move, resize:
		for ( size_t i = 0; i < m_used; ++i )
		{
			bool r = m_marks_gui_mem[ i ].mouseMove( x, m_offset + y );
			if ( r )
			{
				if ( m_marks_gui_mem[ i ].changingInProgress() )
				{
					m_indep_mark = m_marks_mem[i];
					m_indep_markgui = m_marks_gui_mem[i];
					m_indep_markgui.m_valid = true;

					// we delete mark from TrackMarks, but keep it in m_marks_mem
					// to be able to add it on old place if user releases button
					// at a position where mark cannot be added
					m_track->delMark( m_marks_mem[i].m_pos );

					m_marks_gui_mem[ i ].invalidate();
					return &m_indep_markgui;
				}
				return &m_marks_gui_mem[ i ];
			}
		}
		return NULL;
	}

	MarkGUI *mousePress(int x, int y, bool _allow_multiselect)
	{
		if ( ! _allow_multiselect )
		{
			m_track->clearSelection();
			for ( size_t i = 0; i < m_used; ++i )
			{
				m_marks_gui_mem[ i ].m_selected = false;
			}
		}

		for ( size_t i = 0; i < m_used; ++i )
		{
			bool r = m_marks_gui_mem[ i ].mousePress( x, m_offset + y );
			if ( r )
			{
				setSelected(i, true);
				return &m_marks_gui_mem[ i ];
			}
		}
		return NULL;
	}

	// we return void, because user can ask TrackMarks for revision
	// and decide to refresh a widget.
	void mouseRelease(int x, int y)
	{
		if ( m_indep_markgui.changingInProgress() )
		{
			Mark::TimeValue pos = m_indep_markgui.m_coord * m_step_frames;
			Mark::TimeValue len = m_indep_markgui.m_len * m_step_frames;

			// Try add mark at new place.
			// Add old mark, if failed adding new mark.
			if ( ! m_track->addMark(pos, len, m_indep_mark.m_text.c_str(), true) )
				m_track->addMark( m_indep_mark );

			m_indep_markgui.invalidate();
		}


		for ( size_t i = 0; i < m_used; ++i )
		{
			m_marks_gui_mem[ i ].mouseRelease();
		}
	}
};

#endif
