/*
 *  pavelkolodin@gmail.com
 */

#include "TracksBase.h"

#include <fstream> // read/write json files
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <json/json.h>

namespace TracksBase
{

static const char *STR_TYPE_AUDIO = "audio";
static const char *STR_TYPE_MARKS = "marks";

static const char *STR_KEY_TRACKS = "tracks";
static const char *STR_KEY_TYPE = "type";
static const char *STR_KEY_NAME = "name";
static const char *STR_KEY_FILENAME = "file";
static const char *STR_KEY_LINKED_FILENAME = "linked_filename";
static const char *STR_KEY_LINKED_HASH = "linked_hash";

static const char *STR_BASE_MAIN_FILENAME= "base.json";


void jsonToSet( const Json::Value &_val, std::set<std::string> &_set )
{
	for ( Json::Value::UInt i = 0; i < _val.size(); ++i )
	{
		if ( Json::stringValue == _val[i].type() )
			_set.insert( _val[i].asString() );
	}
}

void setToJson( const std::set<std::string> &_set, Json::Value &_val )
{
	BOOST_FOREACH( std::set<std::string>::key_type val, _set )
	{
		_val.append( val );
	}
}


bool loadJsonValue( const Json::Value &_val, TracksBaseRecord &_dst )
{
	std::string str;
	_dst.m_linked_hash.clear();
	_dst.m_type = TrackAbstract::TYPE_UNDEFINED;

	if ( Json::nullValue == _val[STR_KEY_NAME].type() )
		return false;

	if ( Json::nullValue == _val[STR_KEY_FILENAME].type() )
		return false;

	if ( Json::stringValue == _val[STR_KEY_TYPE].type() )
	{
		if ( _val[STR_KEY_TYPE].asString().compare( STR_TYPE_AUDIO ) )
			_dst.m_type = TrackAbstract::TYPE_AUDIO;
		else if ( _val[STR_KEY_TYPE].asString().compare( STR_TYPE_MARKS ) )
			_dst.m_type = TrackAbstract::TYPE_MARKS;
	}

	_dst.m_name = _val[STR_KEY_NAME].asString();
	_dst.m_filename = _val[STR_KEY_FILENAME].asString();

	jsonToSet( _val[STR_KEY_LINKED_FILENAME], _dst.m_linked_filename );
	jsonToSet( _val[STR_KEY_LINKED_HASH], _dst.m_linked_hash );
	return true;
}

bool saveJsonValue(const TracksBaseRecord &_tbr, Json::Value &_val)
{
	std::string str_type;
	if ( TrackAbstract::TYPE_MARKS == _tbr.m_type )
		str_type = STR_TYPE_MARKS;
	else if ( TrackAbstract::TYPE_AUDIO == _tbr.m_type )
		str_type = STR_TYPE_AUDIO;
	else
		return false; // don't save UNDEFINED records.

	_val[STR_KEY_TYPE] = str_type;
	_val[STR_KEY_NAME] = _tbr.m_name;
	_val[STR_KEY_FILENAME] = _tbr.m_filename;

	if ( ! _tbr.m_linked_filename.empty() )
		setToJson( _tbr.m_linked_filename, _val[STR_KEY_LINKED_FILENAME] );
	if ( ! _tbr.m_linked_hash.empty() )
		setToJson( _tbr.m_linked_hash, _val[STR_KEY_LINKED_HASH] );

	return true;
}





void FilenameAllocator::clear()
{
	m_names.clear();
}

bool FilenameAllocator::add( const std::string &_name )
{
	if ( m_names.end() != m_names.find( _name ))
		return false;

	m_names.insert( _name );
	return true;
}

std::string FilenameAllocator::allocate()
{
	static char *chars =
			"abcdefghijklmnopqrstuvwxyz"
	        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	        "1234567890"
			;
			// 26*2 + 10 = 62

	std::string name;
	do
	{
		name.clear();

		boost::random::random_device rng;
		boost::random::uniform_int_distribution<> index_dist(0, 62-1 );
		for(int i = 0; i < 32; ++i)
		{
			name += chars[ index_dist(rng) ];
		}

	} while( m_names.end() != m_names.find( name ) );

	m_names.insert( name );
	return name;
}



TracksBase :: TracksBase ( )
: m_mode( CLOSED )
{
	// reserved ID 0
	m_tracks.push_back( TracksBaseRecordPtr( new TracksBaseRecord() ) );
}


TracksBase :: ~TracksBase ( )
{

}


void TracksBase::openBaseFS( const char *_path, const char *_name )
{
	if ( ! _path )
		throw std::runtime_error("TracksBase::openBaseFS, filename == NULL");

	// Mode fill be FILESYSTEM even if base is not exists yet.
	// It will be created when you save it.
	m_mode = FILESYSTEM;
	m_dir_base = _path;
	std::string filename;
	getFilenameBaseFS__( filename );
	LOG_PURE ( "TracksBase trying to open file \"" << filename << "\"" );

	std::fstream f1( filename.c_str(), std::ios_base::in | std::ios_base::binary );
	if ( ! f1.good() )
		throw std::runtime_error( "TracksBase can't read index." );

	Json::Reader reader;

	Json::Value root;
	if ( ! reader.parse( f1, root ) )
		throw std::runtime_error("TracksBase can't read json document");

	if ( Json::nullValue == root[STR_KEY_TRACKS].type() )
		throw std::runtime_error("TracksBase can't parse json document");


	const Json::Value &tracks = root[ STR_KEY_TRACKS ];
	for ( Json::Value::UInt i = 0; i < tracks.size(); ++i )
	{
		TracksBaseRecordPtr ptr;
		ptr.reset( new TracksBaseRecord() );

		if ( ! loadJsonValue( tracks[i], *ptr ) )
			continue;

		if ( ! ptr->m_filename.empty() )
			m_filename_alloc.add( ptr->m_filename );

	    if ( ! addTrack__( ptr ) )
	    	throw std::runtime_error("TracksBase::openBaseFS(), can't create record in memory.");
	}

	//printTracks__();
}

void TracksBase::save()
{
	if ( FILESYSTEM == m_mode )
		saveBaseFS__();
}

unsigned TracksBase::addTrack( const TrackAbstract& _t)
{
	if ( CLOSED == m_mode)
		return false;

	TracksBaseRecordPtr ptr;
	ptr.reset( new TracksBaseRecord() );
	buildRecordByTrack__( _t, *ptr );
	return addTrack__( ptr );
}

bool TracksBase::updateTrack( unsigned _id, const TrackAbstract& _track )
{
	if ( CLOSED == m_mode)
		return false;

	if ( bad_id ( _id ) )
		return false;


	buildRecordByTrack__( _track, *m_tracks[ _id ] );
	return true;
}

bool TracksBase::deleteTrack( unsigned _id )
{
	if ( CLOSED == m_mode)
		return false;

	if ( bad_id ( _id ) )
		return false;

	m_tracks[ _id ]->m_type = TrackAbstract::TYPE_UNDEFINED;
	return true;
}

bool TracksBase::addLinkedHash( unsigned _id, const char *_str )
{
	if ( bad_id ( _id ) )
		return false;

	if ( ! _str )
		return false;

	m_tracks[ _id ]->m_linked_hash.insert( _str );
	return true;
}

bool TracksBase::addLinkedFilename( unsigned _id, const char *_str )
{
	if ( bad_id ( _id ) )
		return false;

	if ( ! _str )
		return false;

	m_tracks[ _id ]->m_linked_filename.insert( _str );
	return true;
}


size_t TracksBase::findLinkedByHash( const TrackAbstract& _track, VectorOfUnsigned &_result_ids ) const
{
	if ( CLOSED == m_mode)
		return false;

	size_t found = 0;
	std::string hash = _track.getHash();
	if ( hash.empty() )
		return found;

	for ( size_t i = 0; i < m_tracks.size(); ++i )
	{
		if ( m_tracks[i]->hasLinkedHash( hash ) )
		{
			_result_ids.push_back( i );
			++ found;
		}
	}

	return found;
}

void TracksBase::buildRecordByTrack__( const TrackAbstract& _t, TracksBaseRecord &_rec)
{
	_rec.m_type = _t.getType();
	_rec.m_name = _t.getName();

	// If this track has record in DB, leave the same filename!!!
	if ( 0 == _t.getDBId() )
		_rec.m_filename = m_filename_alloc.allocate();
}

const TracksBaseRecord* TracksBase::getRecord( unsigned _id )
{
	if ( (0 == _id) || (_id >= m_tracks.size()) )
		return NULL;

	return m_tracks[ _id ].get();
}

bool TracksBase::getFilename( size_t _id, std::string &_return )
{
	const TracksBaseRecord* r = getRecord ( _id );
	if ( ! r )
		return false;

	if ( r->m_filename.empty() )
		return false;

	boost::filesystem::path p ( m_dir_base );
	p /= r->m_filename;

	_return = p.string();

	return true;
}



//
// PRIVATE
//

unsigned TracksBase::addTrack__(TracksBaseRecordPtr &ptr)
{
	m_tracks.push_back( ptr );
	return m_tracks.size() - 1;
}

void TracksBase::getFilenameByAudioId__(const char *_audio_id, std::string &_return)
{

}



void TracksBase::printTracks__()
{
	BOOST_FOREACH( const TypeVectorTracks::value_type &value, m_tracks )
	{
		value->print();
	}
}

void TracksBase::saveBaseFS__()
{
	Json::StyledWriter writer;
	Json::Value root;

	BOOST_FOREACH( const TypeVectorTracks::value_type &value, m_tracks )
	{
		Json::Value track;
		if ( ! saveJsonValue( *value, track ) )
			continue;

		root["tracks"].append( track );
	}

	std::string filename;
	getFilenameBaseFS__( filename );

	std::string str_json = writer.write( root );
	std::fstream f1( filename.c_str(), std::ios_base::out | std::ios_base::binary);
	if ( ! f1.good() )
	{
		std::string err = "Can't open file: ";
		err += filename;
		throw std::runtime_error( err );
	}

	f1.write( str_json.c_str(), str_json.size() );
	f1.close();
}

void TracksBase::getFilenameBaseFS__(std::string &_dst)
{
	boost::filesystem::path p = m_dir_base;
	p /= STR_BASE_MAIN_FILENAME;
	_dst = p.string();
}

}
