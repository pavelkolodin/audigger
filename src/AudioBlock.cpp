
#include "AudioBlock.h"
#include <stdexcept>
#include <boost/filesystem.hpp>

//AudioBlock::AudioBlock ()
//: m_type( TYPE_SILENCE )
//, m_mem( 0 )
//, m_size_bytes_mem_limit( 0 )
//, m_file(NULL)
//, m_pos(0)
//, m_tmp(false)
//, m_master(false)
//
//, m_prev(NULL)
//, m_next(NULL)
//, m_offset(0)
//, m_trailer(0)
//, m_content_id(0)
//{
//	// first block
//}

AudioBlock::AudioBlock ( const AudioBlock& b, bool deepcopy )
: m_params (b.m_params)
, m_type (b.m_type)
, m_mem (b.m_mem)
, m_size_bytes_mem_limit(0)
, m_file (b.m_file)
, m_pos (0)
, m_tmp (false)
, m_master (false)

, m_id (0)
, m_content_id (b.m_content_id)
, m_prev (NULL)
, m_next (NULL)
, m_offset (b.m_offset)
, m_trailer (b.m_trailer)

{

}

AudioBlock::AudioBlock ( const std::string &_filename )
: m_type (TYPE_FILE)
, m_mem (0)
, m_size_bytes_mem_limit (0)
, m_file (new AudioFile())
, m_pos (0)
, m_tmp (false)
, m_master (true)

, m_id (0)
, m_content_id(0)
, m_prev(NULL)
, m_next(NULL)
, m_offset(0)
, m_trailer(0)
{
	if ( ! m_file->open(_filename.c_str(), AudioFile::MODE_READ ) )
	{
		delete m_file;
		throw std::logic_error( "AudioBlock: cannot open file." );
	}
	m_params = m_file->getAudioParams();
}

AudioBlock::AudioBlock ( const AudioParams &_params, bool _tmp, size_t _size_bytes_mem_limit )
: m_type (TYPE_MEM)
, m_mem (0) // changed later
, m_size_bytes_mem_limit (_size_bytes_mem_limit)
, m_file(NULL)
, m_pos(0)
, m_tmp(_tmp)
, m_master(true)

, m_id (0)
, m_content_id(0)
, m_prev(NULL)
, m_next(NULL)
, m_offset(0)
, m_trailer(0)
{
	m_params = _params;
	m_params.m_frames = 0;
}

AudioBlock::~AudioBlock( )
{
	if ( m_master && m_file )
		delete m_file;
}

//void AudioBlock::clear()
//{
//	m_params.clear();
//	if ( m_file )
//	{
//		delete m_file;
//		m_file = NULL;
//	}
//
//	m_type = TYPE_MEM;
//	m_size_bytes_mem_limit = 0;
//	m_pos = 0;
//	m_prev = NULL;
//	m_next = NULL;
//	m_offset = 0;
//	m_trailer = 0;
//}

//AudioBlock& AudioBlock::operator= ( const AudioBlock& _other)
//{
//	m_prev = _other.m_prev;
//	m_next = _other.m_next;
//	m_offset = _other.m_offset;
//	m_trailer = _other.m_trailer;
//	return *this;
//}

//void AudioBlock::cloneFace(AudioBlock *dst) const
//{
//	dst->m_params = m_params;
//	dst->m_type = m_type;
//	dst->m_mem = m_mem;
//	dst->m_file = m_file;
//
//	dst->m_offset = m_offset;
//	dst->m_trailer = m_trailer;
//	dst->m_content_id = m_content_id;
//	dst->m_params = m_params;
//}

const AudioParams& AudioBlock::getAudioParams() const
{
	return m_params;
}

size_t AudioBlock::write( const char *_data_src, size_t _frames )
{
	size_t bytes = _frames * m_params.frameSize(); //(m_params.m_bits / 8) * m_params.m_channels;
	size_t bytes_used = m_params.m_frames * m_params.frameSize(); //(m_params.m_bits / 8) * m_params.m_channels;

	if ( TYPE_MEM == m_type )
	{
		if (bytes_used + bytes > m_size_bytes_mem_limit )
		{
			// Move data to a file
			// create file, write data to file.
			boost::filesystem::path tmpname = boost::filesystem::unique_path( "%%%%-%%%%-%%%%-%%%%" );

			std::string filename = tmpname.string() + ".wav";

			LOG_PURE( filename );


			m_file = new AudioFile( );
			int mode = (m_tmp) ? AudioFile::MODE_TEMP : 0;
			mode |= (AudioFile::MODE_WRITE | AudioFile::MODE_READ);

			if ( ! m_file->open( filename.c_str(), mode, &m_params ) )
			{
				delete m_file;
				m_file = NULL;
				throw std::logic_error( "Cannot write data to audio file." );
			}

			m_file->seek( 0, SEEK_SET );
			m_file->write( m_mem, m_params.m_frames );

			m_mem_storage.clear();
			m_mem = NULL;

			m_type = TYPE_FILE;
		}
	}

	// Write data:
	if ( TYPE_MEM == m_type )
	{
		m_mem_storage.resize( bytes_used + bytes );
		m_mem = &m_mem_storage[0];
		//char *p = &m_mem[ bytes_used ];
		memcpy( (m_mem + bytes_used), _data_src, bytes );
	}
	else
	if ( TYPE_FILE == m_type )
	{
		//m_file->seek( m_params.m_frames, SEEK_SET );
		m_file->write( _data_src, _frames );
	}

	m_pos += _frames;
	m_params.m_frames += _frames;

	// TODO: stupid: always return correct value.
	return _frames;
}

size_t AudioBlock::read( char *_dst, size_t _len )
{
	size_t ret = 0;
	size_t todo = std::min < size_t > ( _len, sizeVirtual() - m_pos );
	if ( TYPE_FILE == m_type )
	{
		size_t read_frames = 0;
		char *p = _dst;
		while ( read_frames < todo )
		{
			size_t rd = m_file->read(p, todo - read_frames );
			if ( 0 == rd )
				break;

			read_frames += rd;
			p += m_file->getAudioParams().frameSize();
		}

		ret = read_frames;
	}
	else
	if ( TYPE_MEM == m_type )
	{
		memcpy ( _dst, m_mem + m_params.frameSize() * m_pos + m_params.frameSize() * m_offset, m_params.frameSize() * todo );
		ret = todo;
	}

	m_pos += ret;
	return ret;
}

size_t AudioBlock::seek( size_t _pos_frames, int _whence )
{
	// TODO: (1) other WHENCEs, (2) stretch the block by seeking forward to space.

	if ( SEEK_SET == _whence )
	{
		m_pos = std::min < size_t > ( (size_t)_pos_frames, sizeVirtual() );

		if ( TYPE_FILE == m_type )
		{
			m_file->seek( m_pos + m_offset, SEEK_SET );
		}
	}

	return m_pos;
}

size_t AudioBlock::sizeVirtual() const
{
	if ( ( m_offset + m_trailer ) >= m_params.m_frames )
		return 0;

	return (m_params.m_frames - ( m_offset + m_trailer ));
}
