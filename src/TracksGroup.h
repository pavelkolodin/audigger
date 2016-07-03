// pavelkolodin

#ifndef _TracksGroup_h_
#define _TracksGroup_h_

#include <vector>
#include <set>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include "TrackAudio.h"
#include "TrackMarks.h"
#include "TrackInfo.h"
#include "TracksBase.h"
#include "ITracksGroupClient.h"

class Controller;

// The TrackGroup is a set of tracks.
// TracksGroup can create/delete tracks and save itself in a "project file".
class TracksGroup
{

public:
	typedef boost::shared_ptr<TracksGroup> Ptr;
	typedef boost::shared_ptr<TrackAbstract> TrackPtr;
	typedef std::vector < TrackAudio::Ptr > VectorTracksAudio;
	typedef std::vector < TrackMarks::Ptr > VectorTracksMarks;
	typedef std::vector < TrackTimeLine::Ptr > VectorTracksTimeLine;

	TracksGroup();

	void save(const std::string &_dirname);
	void save();
	void load(const std::string &_dirname);

	//bool saveBase();

	void setCallback( ITracksGroupClient * );
	void setAudioParams( const AudioParams &_params );
	const AudioParams &getAudioParams( ) const;

	TrackAudio::Ptr createTrackAudio();
	TrackAudio::Ptr createTrackAudio(const AudioParams &);
	TrackAudio::Ptr createTrackAudio(const char *_filename);
	TrackMarks::Ptr createTrackMarks();
	TrackTimeLine::Ptr createTrackTimeLine();

	bool deleteTrack( TrackAbstract* _t );

	TrackAudio::Ptr getTrackAudioSelectedFirst( ) const;
	size_t getTracksAudio( VectorTracksAudio &_result ) const;
	size_t getTracksMarks( VectorTracksMarks &_result ) const;

	// Actions affecting all thracks in the group
	void setSelected(bool _value);
	void setCursorPos(size_t _pos);
	void setSelectionTime(size_t _start, size_t _len);
	void setSelectionFreq(size_t _start_bin, size_t _bins);


	//void getTracks( VectorTracks &_output ) const;
private:
	// We use pointers, because TrackInfo keeps tracks position, selection parameters
	// that cannot be different in different places.

	//void touchBase_();
	void saveMetadata_();

	//Controller *m_ctrl;
	VectorTracksAudio m_tracks_audio;
	VectorTracksMarks m_tracks_marks;
	VectorTracksTimeLine m_tracks_timeline;

	AudioParams m_params;
	ITracksGroupClient *m_callback;

	boost::filesystem::path m_path;
};

#endif


