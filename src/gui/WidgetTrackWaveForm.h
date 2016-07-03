/*
 *  pavelkolodin@gmail.com
 */

#ifndef _WidgetTrackWaveForm_H_
#define _WidgetTrackWaveForm_H_

#include <QtWidgets>
#include <vector>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

#include "WidgetTrackAbstract.h"
#include "IProcessorClient.h"
#include "Processor.h"
#include "BlockTable.h"
#include "BlockTask.h"


class BlockWaveForm
{

public:
	struct Initializer
	{
		Initializer(size_t _len) : m_len(_len) { }
		void operator()(BlockWaveForm &_w)
		{
			_w.m_waveform.resize( m_len * 2 );
		}

		size_t m_len;
	};





	void clear() { m_result_size = 0; }
	void init(const Processor::TaskWaveForm &_task)
	{
		_task.copyResult( m_waveform );
		m_result_size = _task.resultSize();
		m_min = _task.m_min;
		m_max = _task.m_max;
	}

    std::vector< Processor::TaskWaveForm::MinMaxType > m_waveform;
    Processor::TaskWaveForm::MinMaxType m_min;
    Processor::TaskWaveForm::MinMaxType m_max;
    size_t m_result_size;
};


struct TaskWaveForm
{
	struct Initializer
	{
		void operator()(TaskWaveForm &) { }
	};

	void clear() { }
	size_t m_block_index;
};


class WidgetTrackWaveForm : public WidgetTrackAbstract, public Processor::IProcessorClient
{		
	Q_OBJECT
	public:
		WidgetTrackWaveForm ( QWidget *_parent, WidgetTrackAbstract *_wtg, Controller *_ctrl, TrackAudio::Ptr _track );
		virtual ~WidgetTrackWaveForm ( );

		virtual TrackType getType() { return TYPE_WAVE; }
		//virtual TrackAbstract* getTrackAbstract() { return m_track.get(); }
		virtual TrackAudio::Ptr getTrackAudio() { return m_track; }

		virtual void refresh();
		virtual TrackPos getLenPixels();
		virtual void setOffset( TrackPos _off );
		virtual void setMouseMode(WidgetTrackAbstract::MouseMode _mode);

		//
		//IProcessorClient
		//
		virtual bool taskStillValid( unsigned _task_id ) { return true; }
		virtual void taskDone(const Processor::TaskAbstract &_task);


	public slots:
		void slotTaskDone();
	signals:
		void signalTaskDone();
	protected:
		virtual void paintEvent(QPaintEvent *);
		virtual void resizeEvent(QResizeEvent *);

	private:
        WidgetTrackWaveForm& operator= ( const WidgetTrackWaveForm& _other ) { return *this; }
        void drawScreen__();

        TrackAudio::Ptr m_track;
        boost::mutex m_mutex_blocks;

    	BlockTable::BlockTable<BlockWaveForm> m_block_table;
    	BlockTask::BlockTask<TaskWaveForm> m_block_task;
};

#endif

