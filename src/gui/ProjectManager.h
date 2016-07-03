/*
 *  pavelkolodin@gmail.com
 */

#ifndef _ProjectManager_H_
#define _ProjectManager_H_

#include <boost/shared_ptr.hpp>
#include "WidgetToolBank.h"
#include "WidgetTracksGroup.h"

class QWidget;
class Controller;

class ProjectUI
{
public:
	struct TypeToolBank { };
	struct TypeMedia { };

	ProjectUI(TypeToolBank _xxx, QWidget *_parent )
	{
		m_wbank.reset( new ToolBar::WidgetToolBank( _parent ) );
	}

	ProjectUI(TypeMedia _xxx, QWidget *_parent, Controller *_ctrl, TracksGroup::Ptr _group)
	{

		m_wtg.reset( new WidgetTracksGroup(_parent, _ctrl, _group) );

		unsigned step_frames = _ctrl->getSettings().getToolBar().m_states[ SETTINGS_TOOLBAR_TOOL_TIMESTEP ];
		LOGVAR1( step_frames );
		m_wtg->setStepFrames( step_frames );
	}

	void move( int _x, int _y )
	{
		if ( m_wtg )
			m_wtg -> move( _x, _y );

		if ( m_wbank )
			m_wbank -> move( _x, _y );
	}

	void resize( int _x, int _y )
	{
		if ( m_wtg )
			m_wtg -> resize( _x, _y );

		if ( m_wbank )
			m_wbank -> resize( _x, _y );
	}

	void hide()
	{
		if ( m_wtg )
			m_wtg -> hide();

		if ( m_wbank )
			m_wbank -> hide();
	}

	void show()
	{
		if ( m_wtg )
			m_wtg -> show();

		if ( m_wbank )
			m_wbank -> show();
	}

	WidgetTracksGroup* getWTG()
	{
		return m_wtg.get();
	}

	ToolBar::WidgetToolBank* getWidgetToolBank()
	{
		return m_wbank.get();
	}

private:
	boost::shared_ptr<WidgetTracksGroup> m_wtg;
	boost::shared_ptr<ToolBar::WidgetToolBank> m_wbank;
};



class ProjectManager
{		
	public:
		ProjectManager ( QWidget *_parent, Controller *_ctrl );
		virtual ~ProjectManager ( );

		bool createProjectMedia();
		bool createProjectToolBank();

		// Add track to current project.
		bool addTrackAudioNew(WidgetTrackAbstract::TrackType _type = WidgetTrackAbstract::TYPE_SONOGRAM);
		bool addTrackAudio(const char *_filename, WidgetTrackAbstract::TrackType _type = WidgetTrackAbstract::TYPE_SONOGRAM);
		bool addTrackMarksNew();
		bool addTrackMarks(const char *_filename);
		bool addTrackTimeLine();

		void exchange( unsigned _old, unsigned _new );
		void changeIndexCurrent( unsigned _current );

		void deleteProject( unsigned _index );
		void deleteProjectCurrent( );
		WidgetTracksGroup *getCurrWTG();
		ToolBar::WidgetToolBank *getCurrWidgetToolBank();

		// GUI
		void move( unsigned _w, unsigned _h );
		void resize( unsigned _w, unsigned _h );
		void scroll( int );

	private:
        ProjectManager ( const ProjectManager& _other ) { }
        ProjectManager& operator= ( const ProjectManager& _other ) { return *this; }

        void showProjectCurrent__();

        QWidget *m_widget_parent;
        Controller *m_ctrl;
		std::vector< ProjectUI > m_projects;
		int m_curr_project;
};

#endif

