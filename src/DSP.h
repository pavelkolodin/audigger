
/**
 *  pavelkolodin, 2012-08-08
 *
 */

#ifndef _marker_DSP_h_
#define _marker_DSP_h_

#include <vector>
#include <cstring>
#include <stdint.h>
#include "complex.h"

// class DSP is used for signal processing operations.
// Most of the operations are performed with FFT methods.
class DSP
{
	public:
		typedef int16_t TypeSample16;
		typedef float TypeSample32;

		enum Mode
		{
			UNDEFINED,
			MODE_BANDPASS,		/// The band will be passed, all other frequencies will be reduced.
			MODE_BANDPASS_ZERO,
			MODE_BANDSUPPRESS,
			MODE_F1,
			MODE_F2,
			MODE_F3,
			MODE_F4
		};

		DSP();
		virtual ~DSP();
		
		/// Set FFT params. Used in signal processing.
		void setFFTParams(unsigned _win, unsigned _step);

		/// Set mode.
		void setMode(Mode _mode);

		/// temporary solution :)
		void setParam1( double _param1 );

		/// Set band.
		void setBand(unsigned _start_fft_bin, unsigned _len);
		
		void setChannels(unsigned _ch);

		void Restart();
		/// The main function that performs all the processing
		/// \return wrote frames.
		size_t Process16(const TypeSample16* _src, TypeSample16* _dst, size_t _frames, size_t _limitout = 0);

		size_t Flush16(TypeSample16* _dst);


	private:
		void _init16FFT();
		size_t _process16FFT_Loop(const TypeSample16* _src, TypeSample16* _dst, size_t _frames, size_t _limitout = 0);
		void _process16FFT_Core();
		void _processBandSuppress();

	Mode m_mode;
	unsigned m_fft_win;
	unsigned m_fft_step;
	unsigned m_band_start;
	unsigned m_band_len;
	unsigned m_channels;
	bool m_initdone_16FFT;

	double m_param1;

	// Bufin - buffer for waiting frames
	//TypeSample16 *m_bufin;
	// How many frames are in m_bufin already.
	//size_t m_bufin_size;
	std::vector <TypeSample16> m_bufin;
	size_t m_strv;
	size_t m_prestep;
	std::vector <TypeSample16> m_bufwork;

	std::vector <TypeSample16> m_buf1;
	std::vector <TypeSample16> m_buf2;
	TypeSample16 *m_face;
	TypeSample16 *m_shadow;
	size_t m_bufstep_size;

	std::vector <complex> m_complex;
	std::vector <complex> m_reverb;


};

#endif
