
#include "TrackMarks.h"
#include <fir/logger/logger.h>

#include <fstream>
#include <cstdlib>
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/optional.hpp>
#include <boost/foreach.hpp>
#include <fstream>
#include "json/json.h"
#include "common.h"

#define MARK_NEXT_ID_INIT 1

namespace pt = boost::property_tree;

TrackMarks::TrackMarks()
	: m_multiselect(0)
{
	setName( "marks" );
}

TrackMarks::~TrackMarks ( )
{

}

const char* TrackMarks::getHash() const
{
	// TrackMarks cannot have hash.
	return "none";
}

void TrackMarks::save( const std::string &_filename )
{
	if ( ! _filename.size() )
		throw std::runtime_error("empty filename");

	LOG_PURE( "filename: " << _filename );

	Json::Value root;

	root["params"]["framerate"] = (Json::UInt) 44100;
	root["params"]["name"] = getName();
	Json::Value &marks = root["marks"];

	BOOST_FOREACH( MapMarks::value_type &val, m_marks )
	{
		Json::Value mark;
		mark["pos"] = (Json::UInt) val.second.m_pos;
		mark["len"] = (Json::UInt) val.second.m_len;
		mark["text"] = val.second.m_text;

		marks.append( mark );
	}

	writeJsonToFile(_filename, root);
}

void TrackMarks::load( const std::string &_filename )
{
	if ( ! _filename.size() )
		throw std::runtime_error("TrackMarks::load(), empty filename");

	clear();

	LOG_PURE( "filename: " << _filename );

	std::fstream f1( _filename.c_str(), std::ios_base::in | std::ios_base::binary );

	Json::Reader reader;
	Json::Value root;

	if ( ! reader.parse( f1, root ) )
		throw std::runtime_error("TrackMarks::load(), cannot read json document");

	const Json::Value &marks = root["marks"];
	for ( Json::Value::UInt i = 0; i < marks.size(); ++i )
	{
		unsigned long pos = marks[i]["pos"].asDouble();
		unsigned long len = marks[i]["len"].asDouble();
		std::string text = marks[i]["text"].asCString();

		addMark( pos, len, text.c_str(), false );
	}
	++ m_revision;
}

bool TrackMarks::addMark(Mark::TimeValue _pos, Mark::TimeValue _len, const char *_text, bool _selected)
{
	return addMark ( Mark ( _pos, _len, _text, _selected ) );
}

bool TrackMarks::addMark(const Mark& _m)
{
	if ( ! canAddMark( _m.m_pos, _m.m_len ))
		// mark already exists
		return false;

	m_marks[ _m.m_pos ] = _m;
	++ m_revision;

	return true;
}


bool TrackMarks::canAddMark(Mark::TimeValue _pos, Mark::TimeValue _len)
{
	if ( ! _len )
		++ _len;

	MapMarks::const_iterator itf = m_marks.lower_bound( _pos );
	if ( itf != m_marks.end() )
	{
		if ( itf->second.m_pos < _pos + _len )
			return false;

		// if there is marks before itf, check it:
		if ( itf != m_marks.begin() )
		{
			--itf;
			if ( itf->second.m_pos + itf->second.m_len > _pos )
				return false;
		}
	}
	else
	{
		// if there is marks before itf, check it:
		if ( itf != m_marks.begin() )
		{
			--itf;
			if ( itf->second.m_pos + itf->second.m_len > _pos )
				return false;
		}
	}

	// OK, we can add (_pos, _len) mark.
	return true;
}


bool TrackMarks::delMark(Mark::TimeValue _pos)
{
	MapMarks::iterator itf = m_marks.find( _pos );
	if ( itf == m_marks.end() )
		return false;

	m_marks.erase( itf );
	++ m_revision;
	return true;
}

bool TrackMarks::delAllSelectedMarks()
{
	bool deleted = false;
	bool changed = false;

	MapMarks::iterator it, ite;
	do
	{
		deleted = false;

		it = m_marks.begin();
		ite = m_marks.end();
		for ( ; it != ite; ++it )
		{
			if ( it->second.m_selected )
			{
				m_marks.erase( it );
				deleted = true;
				changed = true;
				break;
			}
		}
	} while( deleted );

	if ( changed )
		++ m_revision;

	return changed;
}

const Mark *TrackMarks::getMark(Mark::TimeValue _pos)
{
	if ( m_marks.end() == m_marks.find( _pos ) )
		return NULL;

	return &m_marks[ _pos ];
}

bool TrackMarks::setMarkText(Mark::TimeValue _pos, const char* _name)
{
	if ( m_marks.end() == m_marks.find( _pos ) )
		return false;

	if ( ! _name )
		_name = "";

	m_marks[ _pos ].m_text = _name;
	++ m_revision;
	return true;
}

//bool TrackMarks::setMultiSelect(bool _multiselect)
//{
//	m_multiselect = _multiselect;
//	if ( ! _multiselect )
//	{
//		// reset all selections, except last.
//		VectorMarks::iterator it, ite;
//		it = m_marks.begin();
//		ite = m_marks.end();
//
//		for ( ; it != ite; ++it )
//			(*it)->second.m_selected = false;
//
//		//setMarkSelected(m_id_lastselect, true);
//	}
//	return true;
//}

bool TrackMarks::setMarkSelected(Mark::TimeValue _pos, bool _selected)
{
	if ( m_marks.end() == m_marks.find( _pos ) )
		return false;

	m_marks[ _pos ].m_selected = _selected;
	return true;
}

void TrackMarks::clearSelection()
{
	BOOST_FOREACH( MapMarks::value_type &val, m_marks )
	{
		val.second.m_selected = false;
	}
}

size_t TrackMarks::loadMarks(Mark::TimeValue _start, Mark::TimeValue _interval, Mark* _mem, size_t _size)
{
	size_t ret = 0;
	//size_t i = 0;
	Mark::TimeValue pos = _start;

	// Get 1 mark before first interval
	MapMarks::const_iterator itf = m_marks.lower_bound( pos );
	if ( itf != m_marks.begin() )
	{
		--itf;
		_mem[ ret ] = itf->second;
		++ ret;
	}

	// --- Get maximum 1 Mark from each interval ---
	// TODO: optimize: step over several intervals at once,
	// if lower_bound returns a mark after current interval.
	for (size_t i = 0; i < _size; pos += _interval, ++i )
	{
		itf = m_marks.lower_bound( pos );
		if ( m_marks.end() == itf )
			break;

		if ( itf->second.m_pos < (pos + _interval) )
		{
			_mem[ ret ] = itf->second;
			++ ret;
		}

//		if ( ret >= _size )
//			break;
//
//		// Put to next interval
//		if ( itf->second.m_pos == (pos + _interval) )
//		{
//			_mem[ ret ] = itf->second;
//			++ ret;
//		}
	}

	return ret;
}

size_t TrackMarks::loadAll(VectorMarks& _vector) const
{
	MapMarks::const_iterator it, ite;
	it = m_marks.begin();
	ite = m_marks.end();

	for ( ; it != ite; ++it )
	{
		_vector.push_back( it->second );

		// Need: ?
		// _vector.back().m_pos = it->first;
	}

	return _vector.size();
}

// Erase everything:
void TrackMarks::clear()
{
	m_marks.clear();
	++ m_revision;
}

void TrackMarks::debug_dump()
{

}
