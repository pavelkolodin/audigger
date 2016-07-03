/*
 *  pavelkolodin@gmail.com
 */

#include <iostream>
#include <cstring>
#include <exception>
#include <sstream>
#include <set>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <json/json.h>
#include <fstream>
#include <sstream>

#include "TrackAudio.h"
#include "AudioFile.h"
#include "AudioBlock.h"
#include <fir/logger/logger.h>
#include "common.h"

namespace fs = boost::filesystem;

TrackAudio::TrackAudio ( const AudioParams &_params )
	: m_pos(0)
	, m_ab_first(0)
	, m_audioblock_id(0)
	, m_audioblock_content_id(0)
{
	setRevision(1);
	_params.throwExceptionForBadAudioParams( );
//	if ( ( ! goodBits( _params.m_bits ) ) || ( (_params.m_channels < 1) || (_params.m_channels > 2) ) || ( ! goodRate(_params.m_rate) ) )
//	{
//
//		throw std::logic_error( "Bad audio parameters" );
//	}

	m_params = _params;
	m_params.m_frames = 0;

	m_mem_ab.reserve(32);
	clear();
}

TrackAudio::TrackAudio ( const char *_filename )
	: m_pos(0)
	, m_ab_first(0)
	, m_audioblock_id(0)
	, m_audioblock_content_id(0)
{
	setRevision(1);
	m_ab_first = new AudioBlock( _filename );
	m_ab_first->m_content_id = m_audioblock_content_id;
	++ m_audioblock_content_id;
	m_ab_first->m_id = m_audioblock_id;
	++ m_audioblock_id;

	m_mem_ab.push_back( m_ab_first );
	m_map_id_ab[ m_ab_first->m_id ] = m_ab_first;


	m_params = m_ab_first->getAudioParams();
	recalcSize__();
	setName( _filename );

	// to be able to save file path in a project file
	// when user excluded the track from the group (project)
	// by disabling "anchor".
	m_path = _filename;
}

TrackAudio::~TrackAudio ( )
{
	int len_ab = m_mem_ab.size();
	for ( int i = 0; i < len_ab; ++i )
		delete m_mem_ab[i];
	m_mem_ab.clear();
}

void TrackAudio::clear()
{
	BOOST_FOREACH( AudioBlock* b, m_mem_ab )
	{
		delete b;
	}

	m_mem_ab.clear();
	m_ab_first = NULL;
	m_params.m_frames = 0;
	m_audioblock_id = 0;
	m_audioblock_content_id = 0;
	m_history.clear();
}

void TrackAudio::save( const std::string &_filename )
{
	// Rewrite existing blocks, or treat them as our blocks that are already saved.
	// (block's content can't be changed; any editing operations adding new blocks or modificating old block's metadata)
	bool rewrite = ( m_path.string().compare( _filename ) );

	m_path = _filename;

	if ( m_path.string().size() < 2 )
		throw std::runtime_error("path is too short");

	if ( ! fs::exists( m_path ) )
		if ( ! fs::create_directories( m_path ) )
			throw std::runtime_error("Cannot create directory");

	if ( ! fs::is_directory( m_path ) )
		throw std::runtime_error("Cannot create directory: file with the same name exists.");

	saveBlocksData__( m_path, rewrite );
	saveBlocksMetadata__( m_path );
}

void TrackAudio::saveBlocksData__(const boost::filesystem::path &_path, bool _rewrite)
{
	std::set<unsigned> saved_content_ids;

	BOOST_FOREACH(AudioBlock *b, m_mem_ab)
	{
		AudioBlock block( *b );
		block.clearLimits();

		if ( saved_content_ids.end() != saved_content_ids.find( block.m_content_id ) )
			continue;

		saved_content_ids.insert( block.m_content_id );

		std::string filename_wav = boost::lexical_cast<std::string>( block.m_content_id );
		filename_wav += ".wav";

		fs::path p( _path );
		p /= filename_wav;

		if ( ! _rewrite )
			if ( fs::exists( p ) )
			{
				LOG_PURE(" ! rewrite ");
				continue;
			}

		AudioFile w;
		if ( ! w.open( p.string().c_str(), AudioFile::MODE_WRITE, &block.getAudioParams() ) )
		{
			std::string reason = "Cannot create file";
			reason += p.string();
			throw std::runtime_error( reason );
		}
		block.seek(0, SEEK_SET);
		transferFrames( &w, &block, block.getAudioParams().m_frames );
		w.close();
	}
}

void TrackAudio::saveBlocksMetadata__(const boost::filesystem::path &_path)
{
	Json::Value root;

	BOOST_FOREACH(AudioBlock *block, m_mem_ab)
	{
		Json::Value json_block;
		json_block["id"] = (Json::UInt) block->m_id;
		json_block["content_id"] = (Json::UInt) block->m_content_id;
		json_block["offset"] = (Json::UInt) block->m_offset;
		json_block["trailer"] = (Json::UInt) block->m_trailer;
		root["blocks"].append( json_block );
	}

	saveAudioParamsToJson(root["audio_params"], m_params );

	// History
	m_history.save( root["history"] );


	// Write file.
	boost::filesystem::path p( m_path );
	p /= "track.json";

	writeJsonToFile(p.string(), root);
}

void TrackAudio::load( const std::string &_dirname )
{
	m_path = _dirname;

	if ( m_path.string().size() < 2 )
		throw std::runtime_error("path is too short");

	if ( ! fs::is_directory( m_path ) )
		throw std::runtime_error("Cannot open directory.");

	clear();

	//
	// load metadata
	//
	{
		fs::path p( m_path );
		p /= "track.json";

		std::fstream f1( p.string().c_str(), std::ios_base::in | std::ios_base::binary );
		if ( ! f1.good() )
			throw std::runtime_error( "Can't open TrackAudio index." );

		Json::Reader reader;
		Json::Value root;
		if ( ! reader.parse( f1, root ) )
			throw std::runtime_error("Can't parse TrackAudio index (incorrect json?)");

		// Load all blocks
		const Json::Value &json_blocks = root["blocks"];
		for ( Json::Value::UInt i = 0; i < json_blocks.size(); ++i )
		{
			loadAudioBlockFromJson_( json_blocks[i] );
		}


		// Load history
		m_history.load( root["history"], m_map_id_ab );
		m_history.replay();

		if ( m_map_id_ab.size() )
			m_ab_first = m_map_id_ab.begin()->second;

		// Load audio params.
		loadAudioParamsFromJson(root[ "audio_params" ], m_params );
	}


}

//AudioBlock *TrackAudio::loadAudioBlock_(const Json::Value &val)
//{
//
//}

void TrackAudio::loadAudioBlockFromJson_(const Json::Value &val)
{
	if (	(Json::uintValue != val["id"].type() && Json::intValue != val["id"].type()) ||
			(Json::uintValue != val["content_id"].type() && Json::intValue != val["content_id"].type()) ||
			(Json::uintValue != val["offset"].type() && Json::intValue != val["offset"].type()) ||
			(Json::uintValue != val["trailer"].type() && Json::intValue != val["trailer"].type()) )
		throw std::runtime_error("Incorrect block item.");

	unsigned content_id = val["content_id"].asUInt();

	// TODO: cache "content_id -> AudioBlocks" to use 1 AudioFile object for the same file.
	fs::path path_wav( m_path );
	path_wav /= (boost::lexical_cast< std::string > (content_id) + ".wav");

	AudioBlock *ab = new AudioBlock( path_wav.string() );
	ab->m_id = val["id"].asUInt();
	ab->m_content_id = val["content_id"].asUInt();
	ab->m_offset = val["offset"].asUInt();
	ab->m_trailer = val["trailer"].asUInt();

	m_mem_ab.push_back( ab );
	m_map_id_ab[ ab->m_id ] = ab;

	if ( ab->m_id >= m_audioblock_id )
		m_audioblock_id = ab->m_id + 1;

	if ( ab->m_content_id >= m_audioblock_content_id )
		m_audioblock_content_id = ab->m_content_id + 1;
}

const char* TrackAudio::getHash() const
{
	std::stringstream SS;
	SS << m_params.m_rate << m_params.m_bits << m_params.m_channels << m_params.m_frames;
	m_hash = SS.str();

	return m_hash.c_str();
}


std::string TrackAudio::getPath()
{
	return m_path.string();
}


const AudioParams& TrackAudio::getAudioParams() const
{
	return m_params;
}

size_t TrackAudio::seek( long _pos, int _whence )
{
	if ( (long)m_pos == _pos )
		return m_pos;

	if ( SEEK_SET == _whence)
	{
		if ( (size_t)_pos < m_params.m_frames )
			m_pos = _pos;
		else
			if ( m_params.m_frames )
				m_pos = m_params.m_frames - 1;
			else
				m_pos = 0;
	}
	return m_pos;
}

size_t TrackAudio::read( char *_dst, size_t _len )
{
	if ( 0 == _len || NULL == _dst )
		return 0;

	size_t offset_local = 0;
	size_t rd = 0;
	size_t rdtotal = 0;

	// Find block that m_pos belongs to:
	AudioBlock *ab = findBlock__ ( m_pos, &offset_local, NULL );

	// Read _len bytes from blocks sequentially starting from "ab":
	for ( ; NULL != ab && rdtotal < _len; ab = ab->m_next, offset_local = 0 )
	{
		// How many FRAMES (multichannel samples) we can read from this block?
		size_t todo = std::min<size_t>( ab->sizeVirtual() - offset_local, _len - rdtotal);
		if ( 0 == todo )
			continue;	// TODO: maybe break?

		// Read todo frames from this block:
		//rd = readBlock__( ab, _dst + rdtotal * m_params.m_channels, offset_local, todo );
		ab->seek( offset_local, SEEK_SET );
		rd = ab->read( _dst, todo );

		if ( todo != rd )
		{
			LOG_ERROR("Fatal: cannot read todo bytes from block.");
			//LOGVAR5( todo, rd, ab->m_frames, ab->m_type, offset_local )
			return 0; // Cannot proceed.
		}

		rdtotal += rd;
		_dst += m_params.frameSize() * rd;
	}

	m_pos += rdtotal;
	return rdtotal;
}

size_t TrackAudio::readFrom( char *_dst, size_t _len, size_t _pos )
{
	if ( seek( _pos, SEEK_SET ) != _pos )
		return 0;

	return read( _dst, _len );
}

size_t TrackAudio::insertTrack(size_t _position, TrackAudio* _track, size_t _position_track, size_t _len, bool _insert)
{
	LOG_PURE(	"\n"
				<< "m_params.m_frames " << m_params.m_frames << "\n"
				<< "_track->getAudioParams().m_frames " << _track->getAudioParams().m_frames << "\n"
				<< "_position_track " << _position_track << "\n"
				<< "_len " << _len << "\n" )

	if ( _track->getAudioParams().m_frames <= _position_track )
		return 0;

	size_t todo = std::min<size_t> (_track->getAudioParams().m_frames - _position_track, _len);

	if ( ! todo )
		return todo;

	AudioBlock* ab = insertBlock__( _position, todo, _insert );

	if ( NULL == ab )
		return 0;

	ab->seek( 0, SEEK_SET );
	_track->seek(_position_track, SEEK_SET );

	size_t ret = transferFrames < AudioBlock, TrackAudio > ( ab, _track, ab->getAudioParams().m_frames );
	recalcSize__();
	++ m_revision;
	return ret;
}

size_t TrackAudio::insertFile( size_t _position, AudioFile* _file, bool _insert)
{
	//m_file = _file;
	LOGVAR3 (_position, _file, _insert)


	if ( ! m_params.compartible( _file->getAudioParams() ) )
	{
		LOG_ERROR( "Not compartible format." )
		return false;
	}

	AudioBlock* ab = insertBlock__( _position, _file->getAudioParams().m_frames, _insert );

	if ( NULL == ab )
		return false;

	ab->seek( 0, SEEK_SET );
	_file->seek( 0, SEEK_SET );

	size_t ret = transferFrames < AudioBlock, AudioFile > ( ab, _file, ab->getAudioParams().m_frames );
	recalcSize__();
	++ m_revision;
	return ret;
}

bool TrackAudio::insertData( size_t _position, AudioBlock &_ab_src, bool _insert )
{
	if ( ! _ab_src.getAudioParams().m_frames )
		return false;

	if ( ! _ab_src.getAudioParams().compartible( m_params ) )
		return false;

	AudioBlock* ab = insertBlock__( _position, _ab_src.getAudioParams().m_frames, _insert );

	ab->seek( 0, SEEK_SET );
	_ab_src.seek( 0, SEEK_SET );

	size_t written = transferFrames < AudioBlock, AudioBlock > ( ab, &_ab_src, _ab_src.getAudioParams().m_frames );

	if ( written != _ab_src.getAudioParams().m_frames )
		return false;

	recalcSize__();
	++ m_revision;
	return true;
}

bool TrackAudio::erase(size_t _from, size_t _len)
{
	TrackAudioHistoryCommiter tahc( m_history );

	AudioBlock *abp = NULL;
	AudioBlock *abp2 = NULL;
	AudioBlock *prev = NULL;
	size_t position_local = 0;
	size_t position_local2 = 0;

	abp = findBlock__( _from, &position_local, &prev );
	abp2 = findBlock__( _from + _len, &position_local2, NULL );

	if ( ! abp )
		return false;


	m_history.watch( abp );
	if ( abp != abp2 )
		m_history.watch( abp2 );

	// End
	if ( abp2 )
	{
		if ( abp == abp2 )
		{
			AudioBlock *half2 = allocAudioBlock__( abp );
			//m_history.watch(half2);
			half2->m_offset += position_local2;
			half2->m_next = abp->m_next;
			abp->m_next = half2;

			m_history.added( half2 );
		}
		else
		{
			abp->m_next = abp2;
			abp2->m_offset += position_local2;
		}
	}
	else
	{
		abp->m_next = NULL;
	}

	// Begin
	abp->m_trailer += (abp->sizeVirtual() - position_local);

	recalcSize__();
	++ m_revision;
	tahc.commit();
	return true;
}

bool TrackAudio::undo()
{
	if ( m_history.undo() )
	{
		recalcSize__();
		++ m_revision;
		return true;
	}
	return false;
}

bool TrackAudio::redo()
{
	if ( m_history.redo() )
	{
		recalcSize__();
		++ m_revision;
		return true;
	}
	return false;
}


//
// P R I V A T E 
//

AudioBlock* TrackAudio::allocAudioBlock__(AudioBlock* _parent)
{
	AudioBlock *nb = NULL;
	if ( _parent )
		nb = new AudioBlock( *_parent );
	else
	{
		nb = new AudioBlock( m_params );
		nb->m_content_id = m_audioblock_content_id;
		++ m_audioblock_content_id;
	}
	nb->m_id = m_audioblock_id;
	++ m_audioblock_id;

	m_mem_ab.push_back( nb );
	m_map_id_ab[ nb->m_id ] = nb;
	return nb;
}


void TrackAudio::copyFrames__(short *_dst, short *_src, size_t _frames)
{
	memcpy( _dst, _src, sizeof(short) * m_params.m_channels * _frames );
}

AudioBlock* TrackAudio::findBlock__(size_t _position, size_t *_position_local, AudioBlock** _prev)
{
	AudioBlock *abp = m_ab_first;
	AudioBlock *prev = NULL;

	size_t currblock_begin = 0;
	for ( ; NULL != abp; abp = abp->m_next )
	{
		if ( _position >= currblock_begin && _position < (currblock_begin + abp->sizeVirtual()) )
		{
			// Found!
			break;
		}
		currblock_begin += abp->sizeVirtual();
		prev = abp;
	}

	if ( abp )
		*_position_local = _position - currblock_begin;

	// If user ask us to save prev, we do it.
	if ( _prev )
		*_prev = prev;

	return abp;
}


AudioBlock* TrackAudio::insertBlock__(size_t _position, size_t _len, bool _insert )
{
	TrackAudioHistoryCommiter tahc( m_history );
	AudioBlock *abp = NULL;
	AudioBlock *abp2 = NULL;
	AudioBlock *prev = NULL;
	size_t position_local = 0;
	size_t position_local2 = 0;

	abp = findBlock__( _position, &position_local, &prev );
	LOGVAR3( abp, position_local, prev );

//	if ( ! abp )
//		return NULL;

	if ( ! _insert )
		abp2 = findBlock__( _position + _len, &position_local2, NULL );

	AudioBlock *nb = allocAudioBlock__();

	if ( ! nb )
		return nb;

	m_history.watch( nb );
	m_history.watch( abp );
	m_history.watch( abp2 );
	m_history.watch( prev );
	tahc.commit();

	if ( _insert )
	{
		//@//m_params.m_frames += _len;

		// if (_position == currpos) then no division needed.
		// Insert new block between 2 existing blocks.
		if (0 == position_local)
		{
			if ( prev )
				prev->m_next = nb;
			else
				m_ab_first = nb;

			nb->m_next = abp; // abp can be null
			return nb;
		}
		else
		{
			// abp must be separated to 2 blocks at "_position". The "_position" must be inside new block.
			// New picture: "abp -> nb -> half2".
			AudioBlock *half2 = allocAudioBlock__( abp );

			m_history.watch( half2 );

			half2->m_next = abp->m_next;
			abp->m_next = nb;
			nb->m_next = half2;

			abp->m_trailer += (abp->sizeVirtual() - position_local);
			half2->m_offset += position_local;

		}
	} // if _insert
	else
	{
		// OVERWRITE

		// this operation must be performed HERE,
		// because track must be consistent to provide
		// readability from point AFTER nb block.

		// Attach end of nb:

		if ( NULL == abp2 )
		{
			nb->m_next = NULL;
		}
		else if ( abp == abp2 )
		{
			if ( abp->sizeVirtual() - position_local2 )
			{
				AudioBlock *half2 = allocAudioBlock__( abp );

				m_history.watch( half2 );

				half2->m_offset += position_local2;
				half2->m_next = abp->m_next;
				nb->m_next = half2;
			}
			else
				nb->m_next = abp->m_next;
		}
		else
		{
			abp2->m_offset += position_local2;

			if ( abp2->sizeVirtual() )
				nb->m_next = abp2;
			else
				nb->m_next = abp2->m_next; // may be NULL.
		}



		// Attach begin of nb:

		if (0 == position_local)
		{
			if ( prev )
				prev->m_next = nb;
			else
				m_ab_first = nb;
		}
		else
		{
			abp->m_next = nb;
			abp->m_trailer += (abp->sizeVirtual() - position_local);
		}
	}

	return nb;
}

void TrackAudio::recalcSize__ ()
{
	AudioBlock *abp = m_ab_first;

	m_params.m_frames = 0;
	for ( ; NULL != abp; abp = abp->m_next )
	{
		m_params.m_frames += abp->sizeVirtual();
	}

	LOG_PURE("RECALC, frames " << m_params.m_frames);
}


