/*
 *  pavelkolodin@gmail.com
 */

#ifndef _TrackMarks_H_
#define _TrackMarks_H_

#include <list>
#include <map>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <fir/logger/logger.h>
#include "TrackAbstract.h"

class Mark
{
public:
	typedef unsigned long TimeValue;

	TimeValue m_pos;
	TimeValue m_len;
	// UTF-8 Text.
	std::string m_text;
	bool m_selected;


	Mark()
	: m_pos( 0 )
	, m_len( 0 )
	, m_selected( 0 )
	{
	}

	Mark(TimeValue _pos, TimeValue _len, const char *_text, bool _selected)
	: m_pos( _pos )
	, m_len( _len )
	, m_selected( _selected )
	{
		if ( _text )
			m_text = _text;

		if ( ! m_len )
			++ m_len;
	}

	void print ()
	{
		LOG_PURE ( "MARK" );
		LOG_PURE ( "pos      " << m_pos );
		LOG_PURE ( "len      " << m_len );
		LOG_PURE ( "text     " << m_text.c_str() );
		LOG_PURE ( "selected " << m_selected );
		LOG_PURE ( "" );
	}
};

// Class is responsible for establishing relations between marks, find ranges of marks.
class TrackMarks : public TrackAbstract
{
public:
	typedef boost::shared_ptr<TrackMarks> Ptr;
	typedef std::vector<Mark> VectorMarks;
	TrackMarks();
	virtual ~TrackMarks ( );

	TrackAbstract::Type getType() const { return TYPE_MARKS; }
	const char* getHash() const;

	void save( const std::string &_filename );
	void load( const std::string &_filename );

	// died
	//bool setMultiSelect(bool _multiselect);

	/// Add mark to track "_track"
	/// \return a pointer to new Mark.
	bool addMark(Mark::TimeValue _pos, Mark::TimeValue _len, const char *_text, bool _selected);
	bool addMark(const Mark&);
	bool canAddMark(Mark::TimeValue _pos, Mark::TimeValue _len);
	bool delMark(Mark::TimeValue _pos);
	bool delAllSelectedMarks();

	// set mark len: remove mark, add mark with new parameters

	// Set new UTF-8 name.
	bool setMarkText(Mark::TimeValue _pos, const char* _newname);
	bool setMarkSelected(Mark::TimeValue _pos, bool _selected);
	void clearSelection();

	const Mark *getMark(Mark::TimeValue _pos);

	// Return maximum "_size" marks found in range [start, start + interval*size).
	// For each interval return maximum 1 Mark.
	// \return number of Marks found and written in "mem"
	size_t loadMarks(Mark::TimeValue start, Mark::TimeValue interval, Mark* mem, size_t size);

	// How many marks the track has.
	size_t size() const { return m_marks.size(); }

	// Load all the marks into vector.
	size_t loadAll(VectorMarks& _vector) const;


protected:
	void clear();
	//Mark* findEqMore(Mark::TimeValue _pos);
	void debug_dump();

	typedef std::map< Mark::TimeValue, Mark > MapMarks;
	MapMarks m_marks;
	bool m_multiselect;
};


#endif
