/*
 * pavelkolodin@gmail.com
 */

#ifndef BlockTask_H_
#define BlockTask_H_

#include <boost/foreach.hpp>

namespace BlockTask
{

template <typename T> struct Task
{
	Task()
	: m_busy(0)
	{
	}

	bool m_busy;
	T m_data;
};


template <typename T>
class BlockTask
{
public:
	typedef std::vector< Task<T> >	TasksVectorType;

	BlockTask(size_t _tasksize);
	void init(typename T::Initializer &_ti);
	void clear();
	bool newTask(size_t *_task_id);
	void freeTask(size_t _task_id);
	T *getTaskData(size_t _task_id);
	bool validDrawProcId__(size_t _task_id)
	{
		return ( (_task_id >= m_task_id_offset) && (_task_id < m_task_id_offset + m_tasks.size()) );
	}

private:
	bool allocateTask__(size_t *_task_id);
	TasksVectorType m_tasks;
	size_t m_task_id_offset;
};

template <typename T> BlockTask<T>::BlockTask(size_t _tasksize)
: m_task_id_offset(0)
{
	m_tasks.resize( _tasksize );
}

template <typename T> void BlockTask<T>::init(typename T::Initializer &_ti)
{
	BOOST_FOREACH(Task<T> &task, m_tasks)
	{
		_ti( task.m_data );
	}
	clear();
}

template <typename T>void BlockTask<T>::clear()
{
	BOOST_FOREACH( Task<T> &task, m_tasks )
	{
		task.m_busy = false;
	}
	m_task_id_offset += m_tasks.size();
}

template <typename T> bool BlockTask<T>::newTask(size_t *_task_id)
{
	for( size_t i = 0; i < m_tasks.size(); ++i )
	{
		Task<T> &task = m_tasks[i];
		if ( ! task.m_busy )
		{
			task.m_busy = true;
			task.m_data.clear();
			*_task_id = m_task_id_offset + i;
			return true;
		}
	}
	return false;
}

template <typename T> void BlockTask<T>::freeTask(size_t _task_id)
{
	if ( validDrawProcId__( _task_id ) )
		m_tasks[ _task_id - m_task_id_offset ].m_busy = false;
}

template <typename T> T* BlockTask<T>::getTaskData(size_t _task_id)
{
	if ( ! validDrawProcId__( _task_id ) )
		return NULL;

	if ( ! m_tasks[ _task_id - m_task_id_offset ].m_busy )
		return NULL;

	return &m_tasks[ _task_id - m_task_id_offset ].m_data;
}

}




#endif /* BlockTask_H_ */
