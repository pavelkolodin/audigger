/*
 *  pavelkolodin
 */

#include <iostream>
#include <vector>

#include "TrackAudio.h"
#include <fir/debug.h>

#define DATASIZE 1024*16

int main()
{
//
//	std::vector<short> m_data;
//	m_data.resize( DATASIZE, 0 );
//
//	std::vector<short> data1, data2, data3, data_read;
//	data1.resize(16, 1);
//	data2.resize(16, 2);
//	data3.resize(16, 3);
//	data_read.resize( 256, 0);
//
//	TrackAudio tr;
//	tr.init( 16, 44100, 1 );
//	ASSERT_PRINT ( 0 == tr.getAudioParams().m_frames )
//
//
//	tr.insertData( 0, &data1[0], 1, true );
//	ASSERT_PRINT ( 1 == tr.getAudioParams().m_frames )
//
//	tr.insertData( 0, &data2[0], 8, true );
//	ASSERT_PRINT ( 9 == tr.getAudioParams().m_frames )
//
//	tr.insertData( 0, &data3[0], 4, true );
//	ASSERT_PRINT ( 13 == tr.getAudioParams().m_frames )
//
//	tr.insertData( 5, &data1[0], 4, true );
//	ASSERT_PRINT ( 17 == tr.getAudioParams().m_frames )
//
//
//	size_t rd = tr.read( &data_read[0], 0, tr.getAudioParams().m_frames );
//	std::cout << "\n";
//	for ( size_t i = 0; i < rd; ++i )
//		std::cout << data_read[i] << " ";
//
//	std::cout << "\n";
//
//	LOGVAR1 ( tr.getAudioParams().m_frames )


	return 0;
}
