/*
 *  pavelkolodin@gmail.com
 */

#ifndef _WidgetTrackSonogram_H_
#define _WidgetTrackSonogram_H_

#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/condition_variable.hpp>

#include <QtWidgets>

#include "TrackAudio.h"
#include "WidgetTrackAbstract.h"
#include "BlockTable.h"
#include "BlockTask.h"
#include "WidgetTrackSonogramUtils.h"
#include "IProcessorClient.h"


class WidgetTrackSonogram : public WidgetTrackAbstract, public Processor::IProcessorClient
{
	Q_OBJECT

public:
	WidgetTrackSonogram(QWidget *_parent, WidgetTrackAbstract *_wtg, Controller *_ctrl, TrackAudio::Ptr _track);
	virtual ~WidgetTrackSonogram();

	void setCanvasSize(unsigned _x, unsigned _y);
	unsigned getCanvasWidth() { return m_canvas_width; }
	unsigned getCanvasHeight() { return m_canvas_height; }

	//
	// WidgetTrackAbstract
	//
	virtual TrackType getType() { return TYPE_SONOGRAM; }
	virtual TrackAudio::Ptr getTrackAudio() { return m_track; }

	// Redraw widget content.
	virtual void refresh();
	virtual TrackPos getLenPixels();

	virtual void setOffset(TrackPos _off);
	virtual void refreshRange(TrackPos _from, TrackPos _len);
	virtual void setFFTWin( unsigned _win );
	virtual void setStepFrames( unsigned _step );

	//
	//IProcessorClient
	//
	virtual bool taskStillValid( unsigned _task_id );
	virtual void taskDone(const Processor::TaskAbstract &_task);
	virtual void taskPercent(const Processor::TaskAbstract &_task, unsigned _percent);


protected:
	virtual void resizeEvent(QResizeEvent *);
	virtual void paintEvent(QPaintEvent *);
	void paintBlocks_(QPaintEvent *);
	//void paintProgressBar_(QPaintEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	//virtual void mouseReleaseEvent(QMouseEvent *);

private slots:
	//void slotRepaint();

	// We need slotDrawDone + signalDrawDone, because we want to change QPixmap.
	// QPixmap cannot be safely changed outside MAIN THREAD!!! X11 client crashes.
	void slotDrawDone(unsigned _drawproc_id);
	void slotScrollBarHoriz(int _value);
	void slotExportTrack();
	void slotExportTrackInterval();
	void slotShowCursor();
signals:
	void signalDrawDone(unsigned _drawproc_id);
	void signalProgressValue(int _value);
	//void signalVisualsChanged();
private:
	// Deny copying // TODO
	WidgetTrackSonogram& operator=(const WidgetTrackSonogram&) { return *this; }

protected:
	void recalcParams__();
	void drawScreen__();

	// Confirm usage of newly allocated block. Don't allocate it again :)
	// This scheme (allocate + commit) is used because we give allocated block
	// to drawing algorithm when we ask it to draw spectrum on it. The drawing
	// algorithm has a rights to reject our request, so we can use this block for another request.
	//void commitBlockAllocation__(size_t _index_dst);

	void actualizeScrollBarHoriz__();


	// m_audioparams_cache.m_frames is INVALID !
	// Used to keep channels, rate, etc...

	TrackAudio::Ptr m_track;
	unsigned m_fft_win;
	boost::shared_ptr<QScrollBar> m_scrollbar;
	//QWidget *m_widget_progress;
	QProgressBar *m_widget_progress;

	BlockTable::BlockTable<BlockFFT> m_block_table;
	BlockTask::BlockTask<FFTDrawingTaskData> m_block_task;

	// mutex for accessing to blocks mechanism (draw, display...)
	boost::mutex m_mutex_blocks;
	boost::mutex m_mutex_signal_draw_done;

	unsigned m_mouse_click_offset;
	unsigned m_mouse_click_coord_time;
	unsigned m_mouse_click_coord_freq;

};
#endif

