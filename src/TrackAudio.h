/*
 *  pavelkolodin@gmail.com
 */

#ifndef _AudioTrack_H_
#define _AudioTrack_H_

#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <json/json.h>
#include "audio_common.h"
#include "AudioFile.h"
#include "AudioBlock.h"
#include "TrackAbstract.h"
#include "TrackAudioHistory.h"

class TrackAudio : public TrackAbstract
{
	public:
		typedef boost::shared_ptr<TrackAudio> Ptr;

		TrackAudio ( const AudioParams & );
		TrackAudio ( const char *_filename );
		virtual ~TrackAudio ( );
		void clear();

		//
		// SAVE: save track as a folder: all blocks + history.
		//
		// Problem: open TrackAudio from folder A and try to save it to the same folder A.
		// TrackAudio will be trying to rewrite block-wav-files that are opened for reading data of this track.
		// Solution: write only new blocks, because TrackAudio NEVER NEVER NEVER changing contents of the blocks,
		// but creating new ones.
		void save( const std::string &_filename );
		void load( const std::string &_filename );

		TrackAbstract::Type getType() const { return TYPE_AUDIO; }
		virtual const char* getHash() const;
		std::string getPath();

		const AudioParams& getAudioParams() const;

		// \return: new position (frames)
		size_t seek( long _pos, int _whence );

		// \return: read frames
		size_t read( char *_dst, size_t _len );

		size_t readFrom( char *_dst, size_t _len, size_t _pos );
		// Insert part of another track.
		// Insert _len frames to _position from _track from _position_track.
		size_t insertTrack( size_t _position, TrackAudio* _track, size_t _position_track, size_t _len, bool _insert);

		// Insert entire AudioFile
		size_t insertFile( size_t _to_position, AudioFile* _file, bool _insert);

		bool insertData( size_t _position, AudioBlock &_ab, bool _insert );

		// Erase interval.
		bool erase(size_t _from, size_t _len);

		/// \return true - action cancelled successfully
		/// \return false - no action to cancel. 
		bool undo();

		/// \return true - action done successfully
		/// \return false - no action to do.
		bool redo();

	private:
        TrackAudio& operator= ( const TrackAudio& _other ) { return *this; }

		AudioBlock* allocAudioBlock__(AudioBlock *_parent = NULL);
		void copyFrames__(short *_dst, short *_src, size_t _frames);

		// \return NULL - not found
		// \return !NULL - AudioBlock that contains track position "_position".
		// _position_local = what position in returned block the "_position" corresponds?
		// _prev - block previous to found. You allowed to change only "m_next" member of prev bock
		// because it may be "m_ab_first".
		AudioBlock* findBlock__(size_t _position, size_t *_position_local, AudioBlock** _prev);
		AudioBlock* insertBlock__(size_t _position, size_t _len, bool _insert);
		void recalcSize__ ();

		// save / load functions
		void saveBlocksData__( const boost::filesystem::path &_path, bool _rewrite );
		void saveBlocksMetadata__( const boost::filesystem::path &_path );

		void loadAudioBlockFromJson_(const Json::Value &val);

    	AudioParams m_params;
    	size_t m_pos;
    	MapIdAudioBlock m_map_id_ab;
		std::vector<AudioBlock*> m_mem_ab;
		AudioBlock *m_ab_first;

		TrackAudioHistory m_history;

		mutable std::string m_hash;
		unsigned m_audioblock_id;
		unsigned m_audioblock_content_id;
		boost::filesystem::path m_path;
};

#endif

