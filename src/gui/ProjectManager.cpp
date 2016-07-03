/*
 *  pavelkolodin@gmail.com
 */

#include <QtWidgets>
#include <boost/foreach.hpp>
#include "ProjectManager.h"
#include "Controller.h"
#include "WidgetAudioParams.h"
#include "defines.h"


ProjectManager :: ProjectManager ( QWidget *_parent, Controller *_ctrl )
: m_widget_parent( _parent )
, m_ctrl ( _ctrl )
, m_curr_project ( -1 )
{

}

ProjectManager :: ~ProjectManager ( )
{

}


bool ProjectManager::createProjectMedia()
{
	// Group guarantees all tracks has the same samplerate.
	// Group sets samplerate to all marks tracks.
	TracksGroup::Ptr group = m_ctrl->createTracksGroup();
	if ( ! group )
		return false;

	m_projects.push_back( ProjectUI( ProjectUI::TypeMedia(), m_widget_parent, m_ctrl, group) );
	m_curr_project = m_projects.size() - 1;
	showProjectCurrent__();
	return true;
}

bool ProjectManager::createProjectToolBank()
{
	m_projects.push_back( ProjectUI(  ProjectUI::TypeToolBank(), m_widget_parent ) );
	m_curr_project = m_projects.size() - 1;
	showProjectCurrent__();
	return true;
}

bool ProjectManager::addTrackAudioNew(WidgetTrackAbstract::TrackType _type)
{
	if ( m_curr_project < 0 )
		return false;

	if ( ! m_projects[m_curr_project].getWTG() )
		return false;

	AudioParams ap = m_projects[m_curr_project].getWTG()->getGroup()->getAudioParams();
	if ( ! ap.m_rate )
	{
		ap.clear();
		ap.set44100_2_S16LE();
	}

	DialogAudioParams dap(0, ap);
	if ( QDialog::Rejected == dap.exec() )
		return false;

	TrackAudio::Ptr track =	m_projects[m_curr_project].getWTG()->getGroup()->createTrackAudio( ap );
	if ( ! track )
		return false;

	m_projects[m_curr_project].getWTG()->addTrack( track, _type );
	return false;
}

bool ProjectManager::addTrackAudio(const char *_filename, WidgetTrackAbstract::TrackType _type)
{
	if ( m_curr_project < 0 )
		return false;

	if ( ! m_projects[m_curr_project].getWTG() )
		return false;

	TrackAudio::Ptr track =	m_projects[m_curr_project].getWTG()->getGroup()->createTrackAudio( _filename );
	if ( ! track )
		return false;

	m_projects[m_curr_project].getWTG()->addTrack( track, _type );
	return true;
}

bool ProjectManager::addTrackMarksNew()
{
	if ( -1 == m_curr_project )
		return false;

	if ( ! m_projects[m_curr_project].getWTG() )
		return false;

	TrackMarks::Ptr track = m_projects[m_curr_project].getWTG()->getGroup()->createTrackMarks();
	m_projects[m_curr_project].getWTG()->addTrack( track );
	return true;
}

bool ProjectManager::addTrackMarks(const char *_filename)
{
	if ( -1 == m_curr_project )
		return false;

	if ( ! m_projects[m_curr_project].getWTG() )
		return false;

	TrackMarks::Ptr track = m_projects[m_curr_project].getWTG()->getGroup()->createTrackMarks();
	track->load(_filename);
	m_projects[m_curr_project].getWTG()->addTrack( track );
	return true;
}

bool ProjectManager::addTrackTimeLine()
{
	if ( -1 == m_curr_project )
		return false;

	if ( ! m_projects[m_curr_project].getWTG() )
		return false;

	TrackTimeLine::Ptr track = m_projects[m_curr_project].getWTG()->getGroup()->createTrackTimeLine();
	track->setName("time");
	m_projects[m_curr_project].getWTG()->addTrack( track );
	return true;
}

void ProjectManager::exchange( unsigned _from, unsigned _to )
{
	if ( _from >= m_projects.size() || _to >= m_projects.size() )
		return;

	std::swap( m_projects[_from], m_projects[_to] );
}

void ProjectManager::changeIndexCurrent( unsigned _current )
{
	if ( _current >= m_projects.size() )
		return;

	m_curr_project = (int)_current;
	showProjectCurrent__();
}

void ProjectManager::deleteProject( unsigned _index )
{
	if ( _index >= m_projects.size() )
		return;

	// TODO
	//if ( m_playing_trackscontainer == m_projects[_index].getWTG() )
		//slotSndPause();

	if ( m_projects[_index].getWTG() )
		m_ctrl->deleteTracksGroup( m_projects[_index].getWTG()->getGroup().get() );

	m_projects.erase( m_projects.begin() + _index );

	if ( m_projects.empty() )
		m_curr_project = -1;
	else
		m_curr_project = 0;

	showProjectCurrent__();
}

void ProjectManager::deleteProjectCurrent( )
{
	if ( m_curr_project < 0 )
		return;

	this->deleteProject( m_curr_project );
}

WidgetTracksGroup *ProjectManager::getCurrWTG()
{
	if ( m_curr_project < 0 )
		return NULL;

	return m_projects[m_curr_project].getWTG();
}

ToolBar::WidgetToolBank *ProjectManager::getCurrWidgetToolBank()
{
	if ( m_curr_project < 0 )
		return NULL;

	return m_projects[m_curr_project].getWidgetToolBank();
}

void ProjectManager::move( unsigned _w, unsigned _h )
{
	BOOST_FOREACH( ProjectUI& project, m_projects )
	{
		project.move( _w, _h );
	}
}

void ProjectManager::resize( unsigned _w, unsigned _h )
{
	BOOST_FOREACH( ProjectUI& project, m_projects )
	{
		project.resize( _w, _h );
	}
}

void ProjectManager::scroll( int _energy)
{
	if ( m_curr_project < 0 )
		return;

	if ( ! m_projects[m_curr_project].getWTG() )
		return;

	m_projects[m_curr_project].getWTG()->scroll( _energy );
}

void ProjectManager::showProjectCurrent__( )
{
	BOOST_FOREACH( ProjectUI& project, m_projects )
	{
		project.hide();
	}

	if ( m_curr_project < 0 )
		return;

	m_projects[ m_curr_project ].show();
}
