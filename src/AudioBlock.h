
#ifndef _AUDIOBLOCK_H_
#define _AUDIOBLOCK_H_

#include <vector>
#include <map>
#include "AudioParams.h"
#include "AudioFile.h"

class AudioBlock
{
public:

	enum Type
	{
		TYPE_SILENCE,
		TYPE_MEM,
		TYPE_FILE
	};
	//AudioBlock( );
	AudioBlock ( const AudioBlock&, bool deepcopy = false );
	AudioBlock ( const std::string &_filename );
	AudioBlock ( const AudioParams &_params, bool _tmp = false, size_t _memlimit_bytes = 1024*1024 );
	virtual ~AudioBlock( );

	//void clear();

	//AudioBlock& operator= ( const AudioBlock& _other);
	//void cloneFace(AudioBlock *_dst) const;

	const AudioParams& getAudioParams() const;
	size_t write( const char *_data_src, size_t _frames );
	size_t read( char *_data_dst, size_t _frames );
	size_t seek( size_t _pos_frames, int _whence );
	size_t sizeVirtual() const;
	void clearLimits()
	{
		m_offset = 0;
		m_trailer = 0;
	}

private:
	AudioParams m_params;
	std::vector < char > m_mem_storage;
	Type m_type;
	char *m_mem;
	size_t m_size_bytes_mem_limit;
	AudioFile *m_file;
	size_t m_pos; // current position, modified by seek()
	bool m_tmp;
	bool m_master;

public:
	// ###
	// ### TODO: public???
	// ###

	unsigned m_id;
	unsigned m_content_id;
	AudioBlock *m_prev;
	AudioBlock *m_next;
	size_t m_offset;
	size_t m_trailer;

};

typedef std::map<unsigned, AudioBlock *> MapIdAudioBlock;

#endif
