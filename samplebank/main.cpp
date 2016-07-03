


#include <iostream>
#include <fir/logger/logger.h>

#include "SampleBank.h"

int main(int argc, char **argv)
{

	if ( argc != 3 )
	{
		LOG_PURE("Need 2 arguments: <path to audigger project> <output dir>");
		return 1;
	}


	SampleBank s;
	try
	{
		s.process(argv[1], argv[2]);
	} catch ( const std::runtime_error &e )
	{
		LOG_PURE( e.what() );
	}

	return 0;
}
