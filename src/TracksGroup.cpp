
// pavelkolodin

#include <stdexcept>
#include <fstream>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include "Controller.h"
#include "TracksGroup.h"
#include <json/json.h>

#include "common.h"

namespace fs = boost::filesystem;

TracksGroup::TracksGroup()
//: m_ctrl( _ctrl )
: m_params ( 0, false, false, false, 0, 0 )
, m_callback( NULL )
{

}

void TracksGroup::save(const std::string &_dirname)
{
	if ( fs::exists( _dirname ) )
		throw std::runtime_error("already exists");

	m_path = _dirname;
	save();
}

void TracksGroup::save()
{
	if ( m_path.string().size() < 2 )
		throw std::runtime_error("path is too short");

	if ( ! fs::exists( m_path ) )
		if ( ! fs::create_directories( m_path ) )
			throw std::runtime_error("Cannot create directory");

	if ( ! fs::is_directory( m_path ) )
		throw std::runtime_error("Cannot create directory: file with the same name exists.");

	unsigned id = 0;
	BOOST_FOREACH( TrackAudio::Ptr &track, m_tracks_audio )
	{
		if ( track->m_anchor )
		{
			track->setDBId( id );
			fs::path p( m_path );
			p /= fs::path( boost::lexical_cast<std::string>(id) );

			track->save( p.string() );

			++ id;
		}
	}

	BOOST_FOREACH( TrackMarks::Ptr &track, m_tracks_marks )
	{
		track->setDBId( id );
		fs::path p( m_path );
		p /= fs::path( boost::lexical_cast<std::string>(id) + ".json" );

		track->save( p.string() );

		++ id;
	}

	saveMetadata_();
}

void TracksGroup::saveMetadata_()
{
	Json::Value root;

	BOOST_FOREACH( TrackAudio::Ptr &track, m_tracks_audio )
	{
		Json::Value json_track;
		json_track["type"] = "audio";
		json_track["name"] = track->getName();
		if ( track->m_anchor )
		{
			json_track["id"] = (Json::UInt)track->getDBId();
		}
		else
		{
			json_track["external"] = track->getPath();
		}

		root["tracks"].append( json_track );
	}

	BOOST_FOREACH( TrackMarks::Ptr &track, m_tracks_marks )
	{
		Json::Value json_track;
		json_track["type"] = "marks";
		json_track["name"] = track->getName();
		json_track["id"] = (Json::UInt)track->getDBId();

		root["tracks"].append( json_track );
	}

	saveAudioParamsToJson(root["audio_params"], m_params);

	fs::path p( m_path );
	p /= "audigger_project.json";
	writeJsonToFile(p.string(), root);
}

void TracksGroup::load(const std::string &_dirname)
{
	//fs::path m_path = _dirname;
	m_path = _dirname;

	if ( m_path.string().size() < 2 )
		throw std::runtime_error("path is too short");

	if ( ! fs::is_directory( m_path ) )
		throw std::runtime_error("Cannot open directory.");

	Json::Value root;
	fs::path p( m_path );
	p /= "audigger_project.json";
	loadJsonFromFile(p.string(), root);
	loadAudioParamsFromJson(root["audio_params"], m_params);

	const Json::Value &json_tracks = root[ "tracks" ];
	for ( Json::Value::UInt i = 0; i < json_tracks.size(); ++i )
	{

		std::string track_filename;
		if ( json_tracks[i].isObject() && json_tracks[i].isMember("id") )
			track_filename = boost::lexical_cast<std::string>( json_tracks[i]["id"].asUInt() );

		std::string external;
		if ( json_tracks[i].isObject() && json_tracks[i].isMember("external") )
			external = json_tracks[i]["external"].asString();

		std::string type = json_tracks[i]["type"].asString();
		std::string name = json_tracks[i]["name"].asString();



		if ( 0 == type.compare("audio") )
		{
			if ( external.empty() )
			{
				fs::path path_track = m_path;
				path_track /= track_filename;

				TrackAudio::Ptr track = createTrackAudio();
				track->setName( name.c_str() );
				track->load( path_track.string() );
			}
			else
			{
				TrackAudio::Ptr track = createTrackAudio( external.c_str() );
				track->setName( name.c_str() );
			}
		}
		else
		if ( 0 == type.compare("marks") )
		{
			track_filename += ".json";
			fs::path path_track = m_path;
			path_track /= track_filename;

			TrackMarks::Ptr track = createTrackMarks();
			track->setName( name.c_str() );
			track->load( path_track.string() );
		}
	}
}

//bool TracksGroup::saveBase()
//{
//	touchBase_();
//
//	TracksBase::TracksBase *base = m_ctrl->getBase("local");
//	BOOST_FOREACH( VectorTracksMarks::value_type &track, m_tracks_marks )
//	{
//		if ( ! track->getDBId() )
//		{
//			LOG_PURE( "skip track with no DBId...")
//			continue;
//		}
//
//		std::string filename;
//		if ( base->getFilename( track->getDBId(), filename ) )
//		{
//			track->save( filename.c_str() );
//		}
//	}
//
//	return true;
//}

void TracksGroup::setCallback( ITracksGroupClient *_callback )
{
	m_callback = _callback;
}

void TracksGroup::setAudioParams( const AudioParams &_params )
{
	m_params = _params;
}

const AudioParams &TracksGroup::getAudioParams( ) const
{
	return m_params;
}

TrackAudio::Ptr TracksGroup::createTrackAudio()
{
	return createTrackAudio( m_params );
}

TrackAudio::Ptr TracksGroup::createTrackAudio(const AudioParams &_params)
{
	TrackAudio::Ptr p( new TrackAudio(_params) );

	if ( m_tracks_audio.empty() )
		m_params = p->getAudioParams();

	m_tracks_audio.push_back( p );

	return p;
}


TrackAudio::Ptr TracksGroup::createTrackAudio(const char *_filename)
{
	TrackAudio::Ptr p( new TrackAudio(_filename) );

	if ( m_tracks_audio.empty() )
		m_params = p->getAudioParams();

	m_tracks_audio.push_back( p );

	// Lest find out if we have some marks tracks assigned to this audio track.

//	TracksBase::TracksBase::VectorOfUnsigned result;
//	TracksBase::TracksBase *base = m_ctrl->getBase("local");
//	size_t num = base->findLinkedByHash( *p, result );
//
//	BOOST_FOREACH( TracksBase::TracksBase::VectorOfUnsigned::value_type &id, result )
//	{
//		//base->getRecord( id )->
//		LOG_PURE( "Linked filename: " << base->getRecord(id)->m_filename );
//
//		if ( m_callback )
//		{
//			TrackMarks::Ptr track = createTrackMarks();
//			// It is important to set DBId.
//			// If DBId is set, track is updated in DB,
//			// else a new copy of it will be added
//			track->setDBId( id );
//			track->setName( base->getRecord(id)->m_name.c_str() );
//
//			std::string filename;
//			if ( base->getFilename( id, filename ) )
//			{
//				try
//				{
//					track->load( filename.c_str() );
//					m_callback->trackCreated( track );
//				} catch ( ... )
//				{
//
//				}
//			}
//		}
//	}

	return p;
}

TrackMarks::Ptr TracksGroup::createTrackMarks()
{
	TrackMarks::Ptr p( new TrackMarks() );
	m_tracks_marks.push_back( p );
	return p;
}

TrackTimeLine::Ptr TracksGroup::createTrackTimeLine()
{
	TrackTimeLine::Ptr p( new TrackTimeLine() );
	m_tracks_timeline.push_back( p );
	return p;
}

bool TracksGroup::deleteTrack( TrackAbstract* _t )
{
	size_t len;

	len = m_tracks_audio.size();
	for ( size_t i = 0; i < len; ++i)
	{
		TrackAbstract *t = m_tracks_audio[i].get();

		if ( t == _t )
		{
			m_tracks_audio.erase( m_tracks_audio.begin() + i );
			return true;
		}
	}

	len = m_tracks_marks.size();
	for ( size_t i = 0; i < len; ++i)
	{
		TrackAbstract *t = m_tracks_marks[i].get();
		if ( t == _t )
		{
			m_tracks_marks.erase( m_tracks_marks.begin() + i );
			return true;
		}
	}
	return false;
}

TrackAudio::Ptr TracksGroup::getTrackAudioSelectedFirst( ) const
{
	BOOST_FOREACH( const TrackAudio::Ptr &t, m_tracks_audio )
	{
		if ( t->getCursorInfo().m_selected )
			return t;
	}

	return TrackAudio::Ptr();
}

size_t TracksGroup::getTracksAudio( VectorTracksAudio &_result ) const
{
	_result = m_tracks_audio;
	return _result.size();
}

size_t TracksGroup::getTracksMarks( VectorTracksMarks &_result ) const
{
	_result = m_tracks_marks;
	return _result.size();
}

void TracksGroup::setSelected(bool _value)
{
	BOOST_FOREACH( TrackAudio::Ptr &p, m_tracks_audio )
	{
		p->getCursorInfo().m_selected = _value;
	}

	BOOST_FOREACH( TrackMarks::Ptr &p, m_tracks_marks )
	{
		p->getCursorInfo().m_selected = _value;
	}
}

void TracksGroup::setCursorPos(size_t _pos)
{
	BOOST_FOREACH( TrackAudio::Ptr &p, m_tracks_audio )
	{
		p->getCursorInfo().m_pos = _pos;
	}
}

void TracksGroup::setSelectionTime(size_t _start, size_t _len)
{
	BOOST_FOREACH( TrackAudio::Ptr &p, m_tracks_audio )
	{
		p->getCursorInfo().m_seltime_start = _start;
		p->getCursorInfo().m_seltime_len = _len;
	}
}

void TracksGroup::setSelectionFreq(size_t _start, size_t _len)
{
	BOOST_FOREACH( TrackAudio::Ptr &p, m_tracks_audio )
	{
		p->getCursorInfo().m_selfreq_start = _start;
		p->getCursorInfo().m_selfreq_len = _len;
	}
}


//void TracksGroup::touchBase_()
//{
//	TracksBase::TracksBase *base = m_ctrl->getBase("local");
//
//	BOOST_FOREACH( const VectorTracksMarks::value_type &tm, m_tracks_marks )
//	{
//		unsigned id_db = tm->getDBId();
//		if ( id_db )
//		{
//			base->updateTrack( id_db, *tm );
//		}
//		else
//		{
//			id_db = base->addTrack( *tm );
//			tm->setDBId( id_db );
//		}
//
//		BOOST_FOREACH( const VectorTracksAudio::value_type &ta, m_tracks_audio )
//		{
//			base->addLinkedFilename( id_db, ta->getName() );
//			base->addLinkedHash( id_db, ta->getHash() );
//		}
//	}
//
//	base->save();
//}

//void TracksGroup::getTracks( std::vector<unsigned> &_output ) const
//{
//	_output.clear();
//	BOOST_FOREACH( unsigned id, m_tracks)
//	{
//		_output.push_back( id );
//	}
//}

