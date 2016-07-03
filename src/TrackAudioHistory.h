/*
 * pavelkolodin
 * 2013-10-25
 */
#pragma once

#include <boost/foreach.hpp>
#include "AudioBlock.h"
#include <json/json.h>

struct AudioBlockState
{
	AudioBlockState()
	: m_prev(0)
	, m_next(0)
	, m_offset(0)
	, m_trailer(0)
	, m_exists(false)
	{

	}

	void learn( const AudioBlock& _block )
	{
		m_prev = _block.m_prev;
		m_next = _block.m_next;
		m_offset = _block.m_offset;
		m_trailer = _block.m_trailer;
		m_exists = true;
	}

	void teach(AudioBlock& _block)
	{
		if ( ! m_exists )
			return;

		_block.m_prev = m_prev;
		_block.m_next = m_next;
		_block.m_offset = m_offset;
		_block.m_trailer = m_trailer;
	}

	bool equals( const AudioBlock& _block )
	{
		return ( _block.m_prev == m_prev &&
				_block.m_next == m_next &&
				_block.m_offset == m_offset &&
				_block.m_trailer == m_trailer &&
				m_exists );
	}

	AudioBlock *m_prev;
	AudioBlock *m_next;
	size_t m_offset;
	size_t m_trailer;
	bool m_exists;
};

struct AudioBlockChange
{
	AudioBlock *m_block;
	AudioBlockState m_old;
	AudioBlockState m_new;
};

struct TrackAudioHistoryRecord
{
	TrackAudioHistoryRecord()
	{
		m_changes.reserve(2);
	}

	void clear()
	{
		m_changes.clear();
	}

	void undo()
	{
		BOOST_FOREACH( AudioBlockChange& change, m_changes )
		{
			change.m_old.teach( *change.m_block );
		}
	}

	void redo()
	{
		BOOST_FOREACH( AudioBlockChange& change, m_changes )
		{
			change.m_new.teach( *change.m_block );
		}
	}

	std::vector<AudioBlockChange> m_changes;
};

class TrackAudioHistory
{
public:
	TrackAudioHistory();
	~TrackAudioHistory();

	void clear();
	void save(Json::Value &_dst);

	// TrackAudio needed to get pointers to real AudioBlocks by their IDs loaded from file.
	void load(const Json::Value &_src, const MapIdAudioBlock &);

	void watch(AudioBlock *);
	void added(AudioBlock *);
	void commit();
	void rollback();

	bool undo();
	bool redo();

	// Replay whole history from start to end.
	// This function is helpful for loading TrackAudio:
	// loaded blocks don't have information about connections between blocks.
	void replay();

private:
	TrackAudioHistoryRecord *m_newrecord;

	std::vector<TrackAudioHistoryRecord*> m_records;
	size_t m_index_next;
};


class TrackAudioHistoryCommiter
{
public:
	TrackAudioHistoryCommiter( TrackAudioHistory& _tah )
	: m_commit( false )
	, m_tah( _tah )
	{

	}

	~TrackAudioHistoryCommiter()
	{
		if ( m_commit )
			m_tah.commit();
		else
			m_tah.rollback();
	}

	void commit()
	{
		m_commit = true;
	}

private:
	bool m_commit;
	TrackAudioHistory& m_tah;
};
