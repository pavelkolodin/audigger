/*
 *  pavelkolodin@gmail.com
 */

#ifndef _AudioFile_H_
#define _AudioFile_H_

//#include <boost/thread/thread.hpp>
//#include <boost/thread/locks.hpp>
//#include <boost/thread/condition_variable.hpp>

#include <cstring>
#include <string>
#include <sndfile.h>
#ifdef SUPPORT_MP3
	#include <mpg123.h>
#endif

#include "AudioParams.h"
#include "audio_common.h"

class AudioFile
{

public:
	static const int MODE_CLOSED = 0;
	static const int MODE_READ = 1 << 0;
	static const int MODE_WRITE = 1 << 1;
	static const int MODE_TEMP = 1 << 2;

	enum FileType
	{
		TYPE_UNDEFINED,
		TYPE_LIBSNDFILE,
		TYPE_MP3
	};

	struct Ext
	{
		const char *m_ext;
		FileType m_type;
		int m_libsndfile_flags;
	};

	AudioFile( );
	virtual ~AudioFile();
	bool open(const char *_filename, int _mode = MODE_READ, const AudioParams *_params = NULL);
	void close( );


	size_t read( char* _dst, size_t _size_frames );
	size_t write( const char* _src, size_t _size_frames );
	size_t seek( size_t _frame, int _whence );
	const AudioParams& getAudioParams() const;

private:
	AudioFile(const AudioFile&) { }
	AudioFile& operator=(const AudioFile&) { return *this; }
	bool openLibSndFile__ ();
	bool openMp3__ ();
	static const struct Ext* check__(const char* _filename );

	int m_mode;
	// info about current file type:
	const struct Ext *m_ext;
	AudioParams m_params;
	std::string m_filename;

	SNDFILE *m_snd_file;
	
#ifdef SUPPORT_MP3
	mpg123_handle *m_mp3;
	short *m_reserve;
	size_t m_reserve_from;	// frames
	size_t m_reserve_size;	// frames
#endif
};


#endif

