/*
 *  pavelkolodin@gmail.com
 */

#ifndef _IProcessorClient_H_
#define _IProcessorClient_H_

#include <stdint.h>
#include <stddef.h> // size_t
#include "complex.h"

namespace Processor
{

struct TaskAbstract;

class IProcessorClient
{
	public:
		IProcessorClient ( ) { }
		virtual ~IProcessorClient ( ) { }

		virtual bool taskStillValid( unsigned _task_id ) = 0;

		// Return 'task is done' event.
		virtual void taskDone( const TaskAbstract& ) = 0;

		// Return % of task that is done.
		virtual void taskPercent( const TaskAbstract &, unsigned _percent ) { }
};

enum ProcessorTaskType
{
	PTT_UNDEFINED = 0,
	PTT_FFT,
	PTT_WAVEFORM,
	PTT_DATA,
	PTT_EXPORT
};

struct RequestAbstract
{
public:
	RequestAbstract(IProcessorClient *_callback, size_t _id, ProcessorTaskType _type)
	: m_callback( _callback)
	, m_id( _id )
	, m_type ( _type )
	{

	}

	IProcessorClient *m_callback;
	size_t m_id;
	// user-defined value
	ProcessorTaskType m_type;
};

struct TaskAbstract
{
	TaskAbstract() : m_callback(0), m_id(0), m_type(PTT_UNDEFINED) { } //, m_busy(false) { }
	virtual ~TaskAbstract() { }
	virtual void init( const RequestAbstract& _r )
	{
		m_callback	= _r.m_callback;
		m_id		= _r.m_id;
		m_type		= _r.m_type;
	}
	virtual bool initialized() { return (NULL != m_callback); }
	virtual void start() = 0;

	IProcessorClient *m_callback;
	size_t m_id;
	ProcessorTaskType m_type;
};

}

#endif

