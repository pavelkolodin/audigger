

#include <TrackMarks.h>
#include <fir/logger/logger.h>
#include <iostream>

#define MEMSIZE 64

int main(void)
{

	size_t arr[] =  {50, 100, 140, 200, 450, 700, 0};
	size_t arr2[] = {50, 100, 140, 200, 700, 800, 0};
	size_t arr_cnt;
	TrackMarks trm;
	const Mark *m;
	bool res;
	size_t num;
	std::vector<Mark> mem;
	//mem.reserve( MEMSIZE );
	mem.resize( MEMSIZE );

	arr_cnt = 0;
	for ( int i = 0; arr[i]; ++i )
	{
		++ arr_cnt;
		m = trm.addMark( arr[i], 1, "super", true );
		EXPECT_TRUE( m );
		EXPECT_TRUE( (int)m->m_id == i );
		EXPECT_TRUE( m->m_pos == arr[i] );
		EXPECT_TRUE( m->m_len == 1 );
	}

	// Test
	num = trm.loadMarks(0, 1000, &mem.front(), MEMSIZE );
	EXPECT_TRUE( arr_cnt == num );

	for ( int i = 0; arr[i]; ++i )
	{
		EXPECT_TRUE( mem[i].m_pos == arr[i] );
	}

	// move 450 -> 800
	res = trm.moveMark( 4, 800 );

	// load from 0
	num = trm.loadMarks(0, 1000, &mem.front(), MEMSIZE );
	EXPECT_TRUE( arr_cnt == num );
	for ( int i = 0; arr2[i]; ++i )
	{
		EXPECT_TRUE( mem[i].m_pos == arr2[i] );
	}

	// load from 45
	//num = trm.loadMarks(0, 1000, &mem.front(), MEMSIZE );

	return 0;
}
