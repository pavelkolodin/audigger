
#include <stdexcept>
#include <boost/foreach.hpp>
#include "TrackAudioHistory.h"
#include "common.h"

TrackAudioHistory::TrackAudioHistory()
: m_newrecord( new TrackAudioHistoryRecord() )
, m_index_next(0)
{

}

TrackAudioHistory::~TrackAudioHistory()
{
	clear();

	delete m_newrecord;
}

void TrackAudioHistory::clear()
{
	m_newrecord->clear();
	BOOST_FOREACH( TrackAudioHistoryRecord* rec, m_records )
	{
		delete rec;
	}
	m_records.clear();
}

void saveAudioBlockState(Json::Value &_json_state, const AudioBlockState& _state )
{
	_json_state["exists"] = _state.m_exists;
	if ( _state.m_exists )
	{
		if ( _state.m_prev )
			_json_state["prev"] = (Json::UInt)_state.m_prev->m_id;
		if ( _state.m_next )
			_json_state["next"] = (Json::UInt)_state.m_next->m_id;

		_json_state["offset"] = (Json::UInt)_state.m_offset;
		_json_state["trailer"] = (Json::UInt)_state.m_trailer;
	}
}

bool func1(const Json::Value &_json, AudioBlock **_dst, const MapIdAudioBlock &_map )
{
	if (Json::uintValue == _json.type() || Json::intValue == _json.type())
	{
		LOG_PURE("id " << _json.asUInt() << "... " );
		MapIdAudioBlock::const_iterator itf = _map.find( _json.asUInt() );
		if ( _map.end() == itf )
			throw std::runtime_error("Error loading AudioBlockState: can't find AudioBlock in map.");

		*_dst = itf->second;
		LOG_PURE("id " << itf->first << ", ptr " << (void*) itf->second );
		return true;
	}
	*_dst = 0;
	return false;
}

void loadAudioBlockState(const Json::Value &_json_state, AudioBlockState& _state, const MapIdAudioBlock &_map )
{
	_state.m_exists = _json_state["exists"].asBool();
	if ( _state.m_exists )
	{
		func1(_json_state["prev"], &_state.m_prev, _map );
		func1(_json_state["next"], &_state.m_next, _map );
		_state.m_offset = _json_state["offset"].asUInt();
		_state.m_trailer = _json_state["trailer"].asUInt();
	}
}

void TrackAudioHistory::save(Json::Value &_dst)
{
	BOOST_FOREACH( TrackAudioHistoryRecord* rec, m_records )
	{
		Json::Value json_record;
		Json::Value &json_changes = json_record["changes"];

		BOOST_FOREACH( const AudioBlockChange& _change, rec->m_changes )
		{
			Json::Value json_chng;
			json_chng["block"] = (Json::UInt) _change.m_block->m_id;
			saveAudioBlockState(json_chng["old"], _change.m_old );
			saveAudioBlockState(json_chng["new"], _change.m_new );
			json_changes.append( json_chng );
		}
		_dst["records"].append( json_record );
	}

	_dst["index_next"] = (Json::UInt) m_index_next;
}

void TrackAudioHistory::load(const Json::Value &_src, const MapIdAudioBlock &_map)
{
	clear();

	const Json::Value &json_records = _src["records"];
	for ( Json::Value::UInt i = 0; i < json_records.size(); ++i )
	{
		LOG_PURE("rec");
		TrackAudioHistoryRecord *rec = new TrackAudioHistoryRecord();

		const Json::Value &json_changes = json_records[i]["changes"];
		for ( Json::Value::UInt j = 0; j < json_changes.size(); ++j )
		{
			LOG_PURE("change");
			rec->m_changes.push_back( AudioBlockChange() );
			AudioBlockChange &change = rec->m_changes.back();

			func1(json_changes[j]["block"], &change.m_block, _map );
			loadAudioBlockState(json_changes[j]["old"], change.m_old, _map );
			loadAudioBlockState(json_changes[j]["new"], change.m_new, _map );
		}
		m_records.push_back( rec );
	}

	m_index_next = _src["index_next"].asUInt();
	if ( m_index_next > (m_records.size() + 1) )
		throw std::runtime_error("Incorrect index_next value for TrackAudioHistory loaded.");
}

void TrackAudioHistory::watch(AudioBlock *_block)
{
	if ( ! _block )
		return;
	m_newrecord->m_changes.push_back( AudioBlockChange() );
	AudioBlockChange &c = m_newrecord->m_changes.back();
	c.m_block = _block;
	c.m_old.learn( *_block );
}

void TrackAudioHistory::added(AudioBlock *_block)
{
	if ( ! _block )
		return;
	m_newrecord->m_changes.push_back( AudioBlockChange() );
	AudioBlockChange &c = m_newrecord->m_changes.back();
	c.m_block = _block;
	// m_old: NOT EXISTS
	c.m_new.learn( *_block );
}

void TrackAudioHistory::commit()
{
	BOOST_FOREACH( AudioBlockChange& change, m_newrecord->m_changes )
	{
		change.m_new.learn( *change.m_block );
	}

	// resize or shrink.
	m_records.resize( m_index_next + 1 );
	m_records[m_index_next] = m_newrecord;
	++ m_index_next;

	m_newrecord = new TrackAudioHistoryRecord();
}

void TrackAudioHistory::rollback()
{
	m_newrecord->m_changes.clear();
}

bool TrackAudioHistory::undo()
{
	if ( ! m_index_next )
		return false;

	-- m_index_next;
	m_records[m_index_next]->undo();
	return true;
}


bool TrackAudioHistory::redo()
{
	if ( m_index_next >= m_records.size() )
		return false;

	m_records[m_index_next]->redo();
	m_index_next ++;
	return true;
}

void TrackAudioHistory::replay()
{
	//for ( size_t i = 0; i < m_records.size(); ++i )
	for ( size_t i = 0; i < m_index_next; ++i )
	{
		m_records[i]->redo();
	}
}
