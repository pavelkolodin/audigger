/*
 * pavelkolodin@gmail.com
 * 2013-11-27 03:51
 */

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <json/json.h>
#include "common.h"
#include "SampleBank.h"
#include <fir/str/md5.h>

namespace fs = boost::filesystem;

void getMD5OfFragment(TrackAudio::Ptr _track, size_t _pos, size_t _len, char *_16bytes)
{
	std::vector<char> m_data;

	size_t todo = 0;
	if ( _track->getAudioParams().m_frames > _pos )
		todo = std::min<size_t>(_len, _track->getAudioParams().m_frames - _pos );

	m_data.resize( _track->getAudioParams().frameSize() * todo );
	_track->seek(_pos, SEEK_SET);
	_track->read( &m_data[0], todo);

	fir::str::md5buff(&m_data[0], _16bytes, todo);
}

SampleBank::SampleBank()
{

}

void SampleBank::process(const std::string &_dir_project, const std::string &_dir_output)
{
	fs::path m_path = _dir_output;

	if ( ! fs::exists( m_path ) )
		if ( ! fs::create_directories( m_path ) )
		{
			throw std::runtime_error( std::string ("Cannot create directory: ") + m_path.string() );
		}

	TracksGroup tg;
	tg.load( _dir_project );

	TracksGroup::VectorTracksAudio tracks_audio;
	TracksGroup::VectorTracksMarks tracks_marks;
	tg.getTracksAudio( tracks_audio );
	tg.getTracksMarks( tracks_marks );

	if ( ! tracks_audio.size() )
		throw std::runtime_error( std::string("No audio tracks in project!") );
	TrackAudio::Ptr &ta = tracks_audio[0];


	Json::Value json_root;


	BOOST_FOREACH(const TrackMarks::Ptr track, tracks_marks)
	{
		LOG_PURE("TrackMarks, size " << track->size() );
		TrackMarks::VectorMarks vector_marks;
		track->loadAll( vector_marks );

		BOOST_FOREACH(const Mark& mark, vector_marks )
		{
			if ( mark.m_len < 32 )
				continue;


			// Name
			char md5[16];
			char md5str[16*2 + 1];

			getMD5OfFragment(ta, mark.m_pos, mark.m_len, md5);
			fir::str::getHexRepr(md5, 16, md5str);
			md5str[32] = 0;

			std::string fileid = md5str;
			fileid += "-";
			fileid += boost::lexical_cast<std::string>( mark.m_pos );
			fileid += "-";
			fileid += boost::lexical_cast<std::string>( mark.m_len );

			fs::path p( _dir_output );
			p /= (fileid + ".wav");


			AudioFile af;
			if ( ! af.open( p.string().c_str(), AudioFile::MODE_WRITE, &ta->getAudioParams() ) )
			{
				std::string reason = "Cannot create file: ";
				reason += p.string();
				throw std::runtime_error( reason );
			}

			ta->seek(mark.m_pos, SEEK_SET);
			transferFrames( &af, ta.get(), mark.m_len );
			af.close();


			Json::Value json_mark;
			json_mark["text"] = mark.m_text;
			json_mark["len"] = (Json::UInt)mark.m_len;
			json_mark["id"] = fileid;
			json_mark["trackname"] = track->getName();

			json_root["marks"].append( json_mark );
		}
	}

	fs::path path_index_json( m_path );
	path_index_json /= "index.json";
	LOG_PURE("writing index...");
	writeJsonToFile(path_index_json.string(), json_root);
	LOG_PURE("writing index... OK");
}





