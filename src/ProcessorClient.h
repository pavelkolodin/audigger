/*
 *  pavelkolodin@gmail.com
 */

#ifndef _ProcessorClient_H_
#define _ProcessorClient_H_

class ProcessorClient
{		
	public:
		ProcessorClient ( );
		virtual ~ProcessorClient ( );
	private:
        ProcessorClient ( const ProcessorClient& _other ) { }
        ProcessorClient& operator= ( const ProcessorClient& _other ) { }
};

#endif

