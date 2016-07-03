/**
 *  pavelkolodin
 */

 
#include <iostream>
#include <cmath>
#include <ctime>

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

/* All of the ALSA library API is defined
 * in this header */
#include <alsa/asoundlib.h>

void printState(snd_pcm_state_t _state)
{
	switch( _state )
	{
		default:
			std::cout << "Unknown state = " << _state << "\n";
			break;
		case SND_PCM_STATE_OPEN:
			std::cout << "SND_PCM_STATE_OPEN\n";
			break;
		case SND_PCM_STATE_SETUP:
			std::cout << "SND_PCM_STATE_SETUP\n";
			break;
		case SND_PCM_STATE_PREPARED:
			std::cout << "SND_PCM_STATE_PREPARED\n";
			break;
		case SND_PCM_STATE_RUNNING:
			std::cout << "SND_PCM_STATE_RUNNING\n";
			break;
		case SND_PCM_STATE_XRUN:
			std::cout << "SND_PCM_STATE_XRUN\n";
			break;
		case SND_PCM_STATE_DRAINING:
			std::cout << "SND_PCM_STATE_DRAINING\n";
			break;
		case SND_PCM_STATE_PAUSED:
			std::cout << "SND_PCM_STATE_PAUSED\n";
			break;
		case SND_PCM_STATE_SUSPENDED:
			std::cout << "SND_PCM_STATE_SUSPENDED\n";
			break;
		case SND_PCM_STATE_DISCONNECTED:
			std::cout << "SND_PCM_STATE_DISCONNECTED\n";
			break;
	}
}

void hanning(short *p, size_t _size, int _channels)
{
	if ( ! _size )
		return;
	if ( _channels != 1 && _channels != 2 )
		return;
	//double alpha = 0.1;
	//double beta = 0.8;
	//alpha = constant of window
	//beta = constant of window. Generally, beta = (1 - alpha)
	for ( size_t i = 0; i < _size; ++i)
	{
		//w[i] = alpha + beta*cos((M_PI*2*i)/nf);
		p[i*_channels] *= ((1.0 - cos( (M_PI*2*((float)i)) / (float)(_size - 1) ))*0.5);
		if ( _channels == 2 )
			p[i*_channels + 1] *= ((1.0 - cos( (M_PI*2*((float)i)) / (float)(_size - 1) ))*0.5);
	}
}

class ALSATest
{
public:
	ALSATest();
	~ALSATest();
	
	bool init(unsigned _rate, unsigned _bits, unsigned _channels);
	//bool playstart();
	void play();

	
private:
	ALSATest(const ALSATest& _other) { }
	ALSATest& operator=(const ALSATest& _other) { return *this; }
	
	int wait_for_poll();

	snd_pcm_uframes_t m_buffer_size;
	snd_pcm_uframes_t m_period_size;
	
	snd_pcm_t *m_alsa_pcm_handle;
	short *m_buff;

	struct pollfd *m_poll_ufds;
	int m_poll_count;
	unsigned m_periods;
	unsigned m_offset;
	unsigned m_channels;
};


ALSATest::ALSATest()
: m_alsa_pcm_handle(NULL)
{
	m_periods = 256;
	m_offset = 0;
	m_channels = 0;
}

ALSATest::~ALSATest()
{
}

static unsigned int buffer_time = 500000;               /* ring buffer length in us */
static unsigned int period_time = 100000;               /* period time in us */
static int period_event = 0;                            /* produce poll event after each period */


bool ALSATest::init(unsigned _rate, unsigned _bits, unsigned _channels)
{
	std::cout	<< "init(), rate " << _rate << " "
				<< "bits " << _bits << " "
				<< "chan " << _channels << " ";

	if (	(_rate != 8000 && _rate != 11025 && _rate != 16000 && _rate != 22050 && _rate != 32000 && _rate != 44100 && _rate != 48000 )
			|| _bits != 16 || (_channels != 1 && _channels != 2) )
	{
		std::cout << "(Bad settings) \n";
		std::cout.flush();	
		return false;
	}
	
	m_channels = _channels;

	int err = 0, dir = 0;
	unsigned int rrate = _rate;
	const char *device_name = "default"; 
	snd_pcm_hw_params_t *hw_params = NULL;
	snd_pcm_sw_params_t *sw_params = NULL;
	m_buffer_size = 0; // get
	m_period_size = 0; // get
	//snd_async_handler_t *ahandler;

	// Open the device.	
	err = snd_pcm_open(&m_alsa_pcm_handle, device_name, SND_PCM_STREAM_PLAYBACK, 0);
	if ( err < 0 )
	{
		std::cout << "snd_pcm_open() failed\n";
		return false;
	}
	
	//snd_pcm_hw_params_malloc (&hw_params);
	snd_pcm_hw_params_alloca(&hw_params);
	snd_pcm_sw_params_alloca(&sw_params);
	
	err = snd_pcm_hw_params_any(m_alsa_pcm_handle, hw_params);
	if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }
	err = snd_pcm_hw_params_set_access (m_alsa_pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }
	

	//err = snd_pcm_hw_params_set_rate_resample(m_alsa_pcm_handle, params, resample);
	//if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }
	
	err = snd_pcm_hw_params_set_format(m_alsa_pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE);
	if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }
	
	err = snd_pcm_hw_params_set_channels(m_alsa_pcm_handle, hw_params, _channels);
	if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }
	
	err = snd_pcm_hw_params_set_rate_near(m_alsa_pcm_handle, hw_params, &rrate, &dir);
	if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }
	
	err = snd_pcm_hw_params_set_buffer_time_near(m_alsa_pcm_handle, hw_params, &buffer_time, &dir);
	if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }
	
	err = snd_pcm_hw_params_get_buffer_size(hw_params, &m_buffer_size);
	if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }
	
	err = snd_pcm_hw_params_set_period_time_near(m_alsa_pcm_handle, hw_params, &period_time, &dir);
	if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }
	
	err = snd_pcm_hw_params_get_period_size(hw_params, &m_period_size, &dir);
	if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }
	
	err = snd_pcm_hw_params (m_alsa_pcm_handle, hw_params);
	if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }
	
	// SW
	err = snd_pcm_sw_params_current(m_alsa_pcm_handle, sw_params);
	if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }
	
	err = snd_pcm_sw_params_set_start_threshold(m_alsa_pcm_handle, sw_params, (m_buffer_size / m_period_size) * m_period_size);
	if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }
	
	err = snd_pcm_sw_params_set_avail_min(m_alsa_pcm_handle, sw_params, period_event ? m_buffer_size : m_period_size);	
	if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }
	
	if (period_event)
	{
		err = snd_pcm_sw_params_set_period_event(m_alsa_pcm_handle, sw_params, 1);
		if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }
	}
	
	err = snd_pcm_sw_params(m_alsa_pcm_handle, sw_params);
	if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }
		
	err = snd_pcm_prepare(m_alsa_pcm_handle);
	if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }
	

	m_poll_count = snd_pcm_poll_descriptors_count (m_alsa_pcm_handle);
	if (m_poll_count <= 0) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }

	m_poll_ufds = new struct pollfd[m_poll_count];
	if (m_poll_ufds == NULL) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }

	err = snd_pcm_poll_descriptors(m_alsa_pcm_handle, m_poll_ufds, m_poll_count);
	if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }








	std::cout << "m_buffer_size: " << m_buffer_size << "\n";
	std::cout << "m_period_size: " << m_period_size << "\n";
	
	m_buff = new short[ m_period_size * _channels * m_periods ];
	
	double freq = 0.01;
	//double add = 0.0000001;
	for ( unsigned per = 0; per < m_periods; ++per )
	{
		freq = 0.0005*(1 + rand() % 512);
		//add += 0.0000001;
		for ( unsigned i = 0; i < m_period_size * _channels; i+=2 )
		{
			//freq += add;
			double val = sin( ((double)(i/2)) * freq );

					// + (1/3)*cos(((float)i) * freq*3 ) + (1/5)*cos(((float)i) * freq*5);
			m_buff[per*m_period_size*_channels + i] = 17000 * val; //(sin( ((float)i) * freq ) + (1/3)*cos(((float)i) * freq*2 )); //100*(i&0x05);
			m_buff[per*m_period_size*_channels + i+1] = 17000 * val; //sin( ((float)i) * freq ); //100*(i&0x05);
		}
		hanning(m_buff + per*m_period_size*_channels, m_period_size, 2);
	}

//	double freq = 0.1;
//	double f = 0;
//	for ( int i = 0; i < m_periods* m_period_size * _channels; i+=2, f+=0.1 )
//	{
//		double val = sin( f );
//		m_buff[i] = 17000 * val;
//		m_buff[i+1] = 17000 * val;
//	}



	return true;
}

//bool ALSATest::playstart()
//{
//	int err = 0;
//
//	std::cout << "[S]"; std::cout.flush();
//
//	err = snd_pcm_writei(m_alsa_pcm_handle, m_buff, m_period_size);
//	if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n";  }
//
//	if (snd_pcm_state(m_alsa_pcm_handle) == SND_PCM_STATE_PREPARED)
//	{
//		std::cout << "[s]"; std::cout.flush();
//		err = snd_pcm_start(m_alsa_pcm_handle);
//		if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; return false; }
//	}
//	return true;
//}

int ALSATest::wait_for_poll()
{
	unsigned short revents;

	while (1)
	{
		poll(m_poll_ufds, m_poll_count, -1);
		snd_pcm_poll_descriptors_revents(m_alsa_pcm_handle, m_poll_ufds, m_poll_count, &revents);
		if (revents & POLLERR)
			return -EIO;
		if (revents & POLLOUT)
			return 0;
	}
}


void ALSATest::play()
{
	int err = 0;
	snd_pcm_uframes_t avail = 0;
	snd_pcm_state_t state;
	std::cout << "[p]"; std::cout.flush();


	if (snd_pcm_state(m_alsa_pcm_handle) == SND_PCM_STATE_PREPARED)
	{
		std::cout << "[s]"; std::cout.flush();
		err = snd_pcm_start(m_alsa_pcm_handle);
		if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; }
	}


	while ( 1 )
	{
		err = wait_for_poll();

		for ( 	avail = snd_pcm_avail_update(m_alsa_pcm_handle);
				avail >= m_period_size;
				avail = snd_pcm_avail_update(m_alsa_pcm_handle))
		{
			std::cout << "." << m_offset << "\n"; std::cout.flush();
			err = snd_pcm_writei(m_alsa_pcm_handle, m_buff + m_offset*m_channels, m_period_size);

			m_offset += m_period_size;
			if ( m_offset >= m_period_size * m_periods )
				m_offset = 0;

			if ( err < 0 )
			{
				state = snd_pcm_state(m_alsa_pcm_handle);
				std::cout << "ERROR: " << __LINE__ << ", \""<<  snd_strerror(err) <<"\", state " << state << "\n";
				printState( state );
				
				if (err == -EPIPE)
				{
					std::cout << "EPIPE\n";
					// under-run
					err = snd_pcm_prepare(m_alsa_pcm_handle);
					if ( err < 0 ) { std::cout << "ERROR: " << __LINE__ << "\n"; }

				}
				else
				if (err == -ESTRPIPE)
				{
					std::cout << "ESTRPIPE\n";
					/* wait until the suspend flag is released */
					while ( (err = snd_pcm_resume(m_alsa_pcm_handle)) == -EAGAIN )
					{
						std::cout << "sleep(1)\n";
						sleep( 1 );
					}
				}
			}

			if ( err > 0 && err != (int)m_period_size )
			{
				std::cout << "ERROR: " << __LINE__ << ", err("<<err<<") != period_size ("<<m_period_size<<"), " << "\n";
			}
		}
	} // while(1)
}

int main(void)
{

	srand( std::time(NULL) );
	ALSATest a;
	
	if ( a.init(44100, 16, 2) ) // CD stereo
	{
		std::cout << "Init OK\n";
	}
	
	while ( 1 )
	{
		a.play();
	}
	
	std::cout << "Exit.\n";

	return 0;
}
