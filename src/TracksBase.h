/*
 *  pavelkolodin@gmail.com
 */

#ifndef _TracksBase_H_
#define _TracksBase_H_

#include <map>
#include <vector>
#include <set>
#include <fir/logger/logger.h>
#include "TrackAbstract.h"

namespace TracksBase
{

//static const char *STR_KEY_ = "";

/*
	{
		"tracks" :
		[
			{
				"name" : "zuzuzu",
				"type" : "marks",
				"filename" : "1.json",
				"linked_name" : "bazar.wav",
				"linked_hash" : "8c8d8c8d8c8d8c8d8c8d8c8d8c8d8c11"
			},
			{
			}
		]
	}
*/

class TracksBaseRecord
{
public:
	typedef std::set<std::string> SetOfStrings;
	TrackAbstract::Type m_type;

	// Name of track.
	std::string m_name;

	// file name in DB's directory where the track is saved
	std::string m_filename;

	// The track can be linked to a hash or to a linked name.
	SetOfStrings m_linked_filename;

	// A hash value that represent some properties of the linked media track,
	// which may be used to choose between several media tracks with the same name.
	// The hash may be calculated in different ways for different types of media tracks.
	// For example, for audio track it may be a hash of string containing
	// audio parameters of the track: "bitrate+channels+frames"
	SetOfStrings m_linked_hash;

	TracksBaseRecord()
	: m_type ( TrackAbstract::TYPE_UNDEFINED )
	{

	}

	bool hasLinkedHash(const std::string &_hash) const
	{
		if ( m_linked_hash.find( _hash ) == m_linked_hash.end() )
			return false;

		return true;
	}

	void print()
	{
		LOG_PURE( "type            " << (int)m_type );
		LOG_PURE( "name            " << m_name );
		LOG_PURE( "linked_filename " << m_linked_filename.size() );
		LOG_PURE( "linked_hash     " << m_linked_hash.size() );
		LOG_PURE( "filename        " << m_filename );
	}
};


class FilenameAllocator
{
public:
	void clear();
	bool add( const std::string & );
	std::string allocate();

private:
	std::set<std::string> m_names;
};


class TracksBase
{		
	public:

		typedef boost::shared_ptr< TracksBaseRecord > TracksBaseRecordPtr;
		typedef std::vector< TracksBaseRecordPtr > TypeVectorTracks;
		typedef std::map< std::string, std::vector< TracksBaseRecordPtr > > MapStrTrackInfo;
		typedef std::vector<unsigned> VectorOfUnsigned;

		enum Mode
		{
			CLOSED,
			FILESYSTEM,
			HTTP
		};

		TracksBase ( );
		virtual ~TracksBase ( );

		// Open new or existing base in filesystem.
		// throws
		void openBaseFS( const char *_path, const char *_name );

		// throws
		void save();

		// return > 0: id of new record
		// return == 0: failed
		unsigned addTrack( const TrackAbstract& _track);
		//bool addTrackLinked( const Track& _track, const Track& _track, size_t &_result_id );
		bool updateTrack( unsigned _id, const TrackAbstract& _track );
		bool deleteTrack( unsigned _id );
		bool addLinkedHash( unsigned _id, const char *_hash );
		bool addLinkedFilename( unsigned _id, const char *_filename );

		size_t findLinkedByHash( const TrackAbstract& _track, VectorOfUnsigned &_result_ids ) const;
		const TracksBaseRecord* getRecord( unsigned _id );
		bool getFilename( size_t _id, std::string &_return );

		// Load marks to TrackMarks
		//bool loadTrackMarks(const char *_linked_name, size_t _index, TrackMarks &_result);

	private:
        TracksBase ( const TracksBase& _other ) { }
        TracksBase& operator= ( const TracksBase& _other ) { return *this; }

		bool bad_id(unsigned _id)
		{
			if ( (0 == _id) || (_id >= m_tracks.size()) )
				return true;
			return false;
		}

        void getFilenameByAudioId__(const char *_audio_id, std::string &_return);
        //void loadPtreeToTrackBaseRecord__(const boost::property_tree::ptree &_tree, TracksBaseRecord &_dst);

        // \return false - cannot express TBR to ptree
        //bool loadTrackBaseRecordToPtree__(const TracksBaseRecord &_tbr, boost::property_tree::ptree &_tree);
        void printTracks__();

        // throws exceptions
        void saveBaseFS__();
        void getFilenameBaseFS__(std::string &_dst);

        void buildRecordByTrack__( const TrackAbstract& _t, TracksBaseRecord &);
        unsigned addTrack__(TracksBaseRecordPtr &_ptr);

        Mode m_mode;
        std::string m_dir_base;

        // All tracks.
        TypeVectorTracks m_tracks;

        // Index for searching TrackMarks associated with some audio track.
        //MapStrTrackInfo m_index_linked_name;

        // Audio tracks.
        //TypeVectorTracks m_tracks_audio;

        FilenameAllocator m_filename_alloc;
};

}

#endif



