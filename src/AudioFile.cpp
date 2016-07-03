/*
 *  pavelkolodin@gmail.com
 */

#include "AudioFile.h"

#include <cstring>
#include <boost/filesystem.hpp>
#include <fir/str/str.h>
#include <fir/logger/logger.h>

#define SIZE_RESERVE sizeof(short)*1024*1024


struct AudioFile::Ext g_audiofile_ext[] = {
		{".wav",  AudioFile::TYPE_LIBSNDFILE, SF_FORMAT_WAV},
		{".wave", AudioFile::TYPE_LIBSNDFILE, SF_FORMAT_WAV},
		{".flac", AudioFile::TYPE_LIBSNDFILE, SF_FORMAT_FLAC},
		//{".ogg", AudioFile::TYPE_LIBSNDFILE, SF_FORMAT_OGG},
		{".voc",  AudioFile::TYPE_LIBSNDFILE, SF_FORMAT_VOC},
		{".nist", AudioFile::TYPE_LIBSNDFILE, SF_FORMAT_NIST},
		{".nst",  AudioFile::TYPE_LIBSNDFILE, SF_FORMAT_NIST},
		{".htk",  AudioFile::TYPE_LIBSNDFILE, SF_FORMAT_HTK},
		{".mat",  AudioFile::TYPE_LIBSNDFILE, SF_FORMAT_MAT4},
		{".mp3",  AudioFile::TYPE_MP3, 0 },
		{NULL,    AudioFile::TYPE_UNDEFINED, 0}
};



AudioFile::AudioFile()
	: m_mode ( MODE_CLOSED )
	, m_ext( 0 )
	, m_snd_file(0)
{
#ifdef SUPPORT_MP3
	m_mp3 = 0;
	m_reserve = 0;
	m_reserve_from = 0;
	m_reserve_size = 0;
	m_reserve = new short[ SIZE_RESERVE ];
#endif
}

AudioFile::~AudioFile()
{
#ifdef SUPPORT_MP3
	if ( m_reserve )
		delete[] m_reserve;
#endif

	close( );
}



bool AudioFile::open(const char *_filename, int _mode, const AudioParams *_params)
{
	// params by pointer, because when we open file for reading we don't know the parameters.

	if ( ! _filename || ! *_filename )
		return false;

	close();

	m_filename = _filename;
	m_mode = _mode;

	if ( MODE_WRITE & _mode )
	{
		if ( ! _params )
			return false;

		m_params = *_params;
	}


	//FileExtHelper file_ext_helper;
	m_ext = check__( _filename );
	if ( NULL == m_ext )
		return false;

	bool res_open = false;

	if ( m_ext->m_type == TYPE_LIBSNDFILE )
		res_open = openLibSndFile__ ();

	else if ( m_ext->m_type == TYPE_MP3 )
		res_open = openMp3__ ();

	if ( ! res_open )
		close();

	return res_open;
}

void AudioFile::close( )
{
	if ( ! m_ext )
		return;

	if ( m_snd_file )
	{
		sf_close( m_snd_file );
		m_snd_file = NULL;
	}

#ifdef SUPPORT_MP3
	m_reserve_from = 0;
	m_reserve_size = 0;
	if ( m_mp3 )
	{
		mpg123_close( m_mp3 );
		m_mp3 = NULL;
	}
#endif

	if ( (m_mode & MODE_TEMP) && (m_mode & MODE_WRITE ) )
	{
		// remove the file
		try
		{
			boost::filesystem::remove( m_filename );
		} catch ( ... )
		{
		}
	}

	m_filename.clear();
	m_ext = 0;
	m_mode = MODE_CLOSED;
}

size_t AudioFile::read(char* _dst, size_t _size)
{
	if ( NULL == m_ext )
		return 0;

	if ( TYPE_LIBSNDFILE == m_ext->m_type )
	{
		if ( 16 == m_params.m_bits )
		{
			sf_count_t rd = sf_readf_short (m_snd_file, (short*)_dst, _size);
			return rd;
		}

		if ( 32 == m_params.m_bits )
			return sf_readf_float (m_snd_file, (float*)_dst, _size);

		return 0;
	}
	else
	if ( TYPE_MP3 == m_ext->m_type )
	{
#ifdef SUPPORT_MP3
		// If we have decoded these bytes:
		// SIZE_RESERVE

		size_t frames_read = 0;
		size_t local_todo = 0;
		size_t local_from = _from_frame;

		while ( frames_read < todo )
		{
			//todo_local = SIZE_RESERVE;
			local_todo = std::min<size_t> ( todo - frames_read, SIZE_RESERVE/m_params.m_channels );
			//local_todo = SIZE_RESERVE/m_params.m_channels;


			//if ( todo_local < SIZE_RESERVE )
			//	todo

			//LOGVAR2( todo, todo_local )

			// Decode some frames.
			if ( local_from < m_reserve_from || ((local_from + local_todo) > (m_reserve_from + m_reserve_size)))
			{
				// Re-fill reserve.
				//size_t reserve_frames = SIZE_RESERVE/m_params.m_channels;
				//int err;
				//size_t done = 0;
				//off_t sk = 0;


				sk = mpg123_seek (m_mp3, local_from, SEEK_SET);
				if ( sk >= 0 )
				{
					err = mpg123_read( m_mp3,
										(unsigned char*) m_reserve,
										sizeof(short) * m_params.m_channels * reserve_frames/*local_todo*/,
										&done ); // done ( bytes )

					if ( MPG123_OK == err )
					{
						LOG_PURE( "(TYPE_MP3 decoded " << done << ")");
						m_reserve_from = local_from;
						m_reserve_size = done/(sizeof(short) * m_params.m_channels);
					}
					else
					{
						LOG_ERROR ( "(mpg123) " << mpg123_strerror(m_mp3) << ", err " << err );
					}
				}


			}

			//LOGCOUT("ZZZ:\n")
			//LOGVAR5( todo, todo_local, _from_frame, m_reserve_from, m_reserve_size )

			if ( local_from >= m_reserve_from && (local_from + local_todo) <= (m_reserve_from + m_reserve_size))
			{
				// We do have reserve for this range.
				memcpy ( _dst, &m_reserve[ local_from * m_params.m_channels - m_reserve_from* m_params.m_channels], sizeof(short) * local_todo * m_params.m_channels );
				_dst += local_todo * m_params.m_channels;
				frames_read += local_todo;
				local_from += local_todo;

				//LOGCOUT("\n+" << todo_local)

				//if ( frames_read < todo )
				//	LOGCOUT("CN")

				continue;
			}
			else
			{
				LOG_ERROR( "Error reading.\n" )
				LOGVAR4( m_reserve_from, m_reserve_size, _from_frame, todo )
				break;
			}
		} // while reading todo
		return frames_read;
	#endif

	} // MP3

	return 0;
}

size_t AudioFile::write( const char* _src, size_t _size )
{
	long s = 0;
	for( size_t i = 0; i < _size; ++i )
	{
		s += (long)_src[i];
	}

	// Tests
	if ( !_src || !_size )
	{
		LOG_ERROR( "!_src || !_size" )
		return 0;
	}

	if ( ! m_ext )
	{
		LOG_ERROR( "! m_ext" )
		return 0;
	}



	if ( ! (AudioFile::MODE_WRITE & m_mode) )
	{
		LOG_ERROR( "! (AudioFile::MODE_WRITE & m_mode)" )
		return 0;
	}

	if ( AudioFile::TYPE_LIBSNDFILE != m_ext->m_type )
	{
		LOG_ERROR( "MediaInfo::TYPE_LIBSNDFILE != m_params.m_type" )
		return 0;
	}

	if ( ! m_snd_file )
	{
		LOG_ERROR( "! m_snd_file" )
		return 0;
	}


	size_t ret = 0;

	//if ( 8 == m_params.m_bits )
		//ret = sf_writef_ (m_snd_file, (short*)_src, _size);

	if ( 16 == m_params.m_bits )
	{
		ret = sf_writef_short(m_snd_file, (const short*)_src, _size);
	}

	if ( 32 == m_params.m_bits )
	{
		ret = sf_writef_float(m_snd_file, (const float*)_src, _size);
	}

	return ret;
}

size_t AudioFile::seek( size_t _pos, int _whence )
{
	if ( NULL == m_ext )
		return 0;

	if ( TYPE_LIBSNDFILE == m_ext->m_type )
	{
		return sf_seek( m_snd_file, _pos, _whence );
	}

	// TODO: mp3 seek. Hard thing.
	return 0;
}

const AudioParams& AudioFile::getAudioParams() const
{
	return m_params;
}

//
// P R I V A T E
//

bool AudioFile::openLibSndFile__()
{
	SF_INFO snd_info;
	memset(&snd_info, 0, sizeof(SF_INFO));

	if ( MODE_READ == m_mode )
	{
		m_params.clear();
		//snd_info.format = 0;
		m_snd_file = sf_open( m_filename.c_str(), SFM_READ, &snd_info );
		if ( NULL == m_snd_file )
			return false;

		//sf_seek(m_snd_file, 0, SEEK_SET);
//		// Cannot operate on more than two channels.
//		if ( snd_info.channels > 2 )
//		{
//			LOG_ERROR("Processor::_openWav(), snd_info.channels > 2");
//			return false;
//		}

		m_params.m_rate = snd_info.samplerate;
		m_params.m_signed = true;
		m_params.m_bigendian = (SF_ENDIAN_BIG == (snd_info.format & SF_FORMAT_ENDMASK) );

		if (	(SF_FORMAT_PCM_S8 == (snd_info.format & SF_FORMAT_SUBMASK) ) &&
				(SF_ENDIAN_LITTLE == (snd_info.format & SF_FORMAT_ENDMASK) )
				)
		{
			m_params.m_bits = 8;
		}

		else if ( SF_FORMAT_PCM_16 == (snd_info.format & SF_FORMAT_SUBMASK) ) //|| snd_info.format & SF_FORMAT_VORBIS )
		{
			m_params.m_bits = 16;
		}

		else if ( SF_FORMAT_PCM_32 == (snd_info.format & SF_FORMAT_SUBMASK) )
		{
			m_params.m_bits = 32;
		}

		else if ( SF_FORMAT_FLOAT == (snd_info.format & SF_FORMAT_SUBMASK) )
		{
			m_params.m_bits = 32;
			m_params.m_float = true;
		}

		else if ( SF_FORMAT_DOUBLE == (snd_info.format & SF_FORMAT_SUBMASK) )
		{
			m_params.m_bits = 64;
			m_params.m_float = true;
		}

		m_params.m_channels = snd_info.channels;
		m_params.m_frames = (size_t)snd_info.frames;
	}
	else
	if ( MODE_WRITE & m_mode )
	{
		if ( (m_params.m_channels < 1) || (m_params.m_channels > 2) )
		{
			LOG_ERROR("m_params.m_channels < 1 || m_params.m_channels > 2");
			return false;
		}

		if ( ! m_params.goodRate() )
		{
			LOG_ERROR("! goodRate( m_params.m_rate )");
			return false;
		}

		snd_info.samplerate = m_params.m_rate;
		snd_info.channels = m_params.m_channels;
		snd_info.format = m_ext->m_libsndfile_flags; // | SF_ENDIAN_LITTLE;

		if ( 8 == m_params.m_bits )
			snd_info.format |= SF_FORMAT_PCM_S8;
		if ( 16 == m_params.m_bits )
			snd_info.format |= SF_FORMAT_PCM_16;
		if ( 24 == m_params.m_bits )
			snd_info.format |= SF_FORMAT_PCM_24;
		if ( 32 == m_params.m_bits )
			snd_info.format |= SF_FORMAT_PCM_32;

		if ( MODE_WRITE == m_mode )
			m_snd_file = sf_open( m_filename.c_str(), SFM_WRITE, &snd_info );

		if ( (MODE_WRITE & m_mode) && (MODE_READ & m_mode) )
			m_snd_file = sf_open( m_filename.c_str(), SFM_RDWR, &snd_info );

		if ( ! m_snd_file )
		{
			LOG_ERROR("! m_snd_file");
			return false;
		}
	}

	return true;
}

bool AudioFile::openMp3__()
{
#ifndef SUPPORT_MP3
	return false;
#else
	if ( ::AudioFile::MODE_READ != m_mode )
		return false;

	int  err  = MPG123_OK;
	int  channels = 0, encoding = 0;
	long rate = 0;
	err = mpg123_init();

	if(err != MPG123_OK || (m_mp3 = mpg123_new(NULL, &err)) == NULL)
	{
		LOG_ERROR("Processor::openMp3__(), cannot init.");
		return false;
	}

	if(   	mpg123_open(m_mp3, _filename) != MPG123_OK ||
			mpg123_getformat(m_mp3, &rate, &channels, &encoding) != MPG123_OK )
	{
		LOG_ERROR("ERROR: Processor::openMp3__(), cannot open.");
		return false;
	}

	if ( encoding & MPG123_ENC_16 )
		m_params.m_bits = 16;
	else
	{
		LOG_ERROR("ERROR: Processor::openMp3__(), 16 bits only.");
		return false;
	}

	/* Ensure that this output format will not change (it could, when we allow it). */
	mpg123_format_none(m_mp3);
	mpg123_format(m_mp3, rate, channels, encoding);

	m_params.m_rate = rate;
	m_params.m_channels = channels;
	m_params.m_frames = mpg123_length(m_mp3);

	return true;
#endif
}

const struct AudioFile::Ext* AudioFile::check__(const char* _filename ) //, const Ext *_result)
{
	if ( NULL == _filename ) // || NULL == _result )
		return 0;

	static const int SIZE_EXT = 6;
	char ext_copy[ SIZE_EXT ];
	memset(ext_copy, 0, SIZE_EXT);

	const char* ext = strrchr( _filename, '.' );
	if ( strlen(ext) > SIZE_EXT-1 )
		return NULL;

	strcpy(ext_copy, ext);
	// WaVe -> wave
	::fir::str::latinToLower( ext_copy );

	// This structure contains relations between file type
	// and libsnd flags to open these files for WRITING

	for ( int i = 0; NULL != g_audiofile_ext[i].m_ext; ++i)
	{
		if (! strcmp(ext_copy, g_audiofile_ext[i].m_ext) )
		{
			return &g_audiofile_ext[i];
		}
	}

	return NULL;
}


