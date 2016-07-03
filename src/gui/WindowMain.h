/*
 *  pavelkolodin@gmail.com
 */

#ifndef _Templater_H_
#define _Templater_H_

#include <QtWidgets>
#include <vector>
#include <sstream>

#include "WidgetToolBar.h"
#include "WidgetToolBank.h"
#include "IProcessorClient.h"
#include "WidgetTrackAbstract.h"
#include "common.h"
#include "fft.h"
#include "ProjectManager.h"

class Controller;
class AudioTrack;
class WidgetTrackSonogram;

class WidgetMainMenu: public QObject
{
	Q_OBJECT
	public:
		WidgetMainMenu(QWidget *_parent);
		void projectsExists(bool);
	private:
		QMenu *m_menu;

		QAction *m_act_project_new;
		QAction *m_act_project_new_tools_bank;
		QAction *m_act_project_open;
		QAction *m_act_project_save;
		QAction *m_act_project_save_as;
		QAction *m_act_project_close;

		QAction *m_act_new_trackaudio;
		QAction *m_act_new_trackmarks;
		QAction *m_act_new_tracktimeline;
		QAction *m_act_open_audio;
		QAction *m_act_open_marks;
		//QAction *m_act_save_marks_tracks_as;
		QAction *m_act_export;


	public slots:
		void slotExec()
		{
			m_menu->exec ( QCursor::pos() );
		}
	private slots:
		void slot_act_project_new()
		{
			signalProjectNew();
		}
		void slot_act_project_new_tools_bank()
		{
			signalProjectNewToolsBank();
		}
		void slot_act_project_open()
		{
			signalProjectOpen();
		}
		void slot_act_project_save()
		{
			signalProjectSave();
		}
		void slot_act_project_save_as()
		{
			signalProjectSaveAs();
		}
		void slot_act_project_close()
		{
			signalProjectClose();
		}
		void slot_act_new_trackaudio()
		{
			signalTrackAudioNew();
		}
		void slot_act_new_trackmarks()
		{
			signalTrackMarksNew();
		}
		void slot_act_new_tracktimeline()
		{
			signalTrackTimeLineNew();
		}

		void slot_act_open_audio()
		{
			signalTrackAudioOpen();
		}
		void slot_act_open_marks()
		{
			signalTrackMarksOpen();
		}
		void slot_act_save_marks_tracks_as()
		{
			signalTrackMarksSaveAllAs();
		}
		void slot_act_export()
		{

		}


	signals:
		void signalProjectNew();
		void signalProjectNewToolsBank();
		void signalProjectOpen();
		void signalProjectSave();
		void signalProjectSaveAs();
		void signalProjectClose();

		void signalTrackAudioNew();
		void signalTrackMarksNew();
		void signalTrackTimeLineNew();
		void signalTrackAudioOpen();
		void signalTrackMarksOpen();

		void signalTrackMarksSaveAllAs();
};





class WindowMain : public QWidget
{
	Q_OBJECT
	public:
		WindowMain ( Controller * );
		virtual ~WindowMain ( );

		/// Open new "document" and load data in it from _filename.
		/// For testing purposes.
		ProjectManager *getProjectManager() { return &m_pm; }

	protected:
		void paintEvent(QPaintEvent *);
		void wheelEvent(QWheelEvent *);
		void resizeEvent(QResizeEvent *);
		void keyPressEvent ( QKeyEvent * _event );
		void keyReleaseEvent ( QKeyEvent * _event );

	private:
        WindowMain ( const WindowMain& _other ); // { }
        WindowMain& operator= ( const WindowMain& _other ); // { return *this; }

        void actualizeGui();
        void buildStatusBar__();
        void buildMainMenu__();
		void updateLayout( );
		void updateSize( );
		void sndStartStopSwitch__();

	private slots:
		void slotTimerAnimation();

	private slots:
		void slotToolBarAction(ToolBar::ToolType, unsigned);
		void slotProjectCurrentChanged(unsigned);
		void slotProjectsMove(unsigned, unsigned);
		void slotProjectCloseRequested(unsigned);

	public slots:
		void slotProjectNew();

	private slots:
		void slotProjectNewToolsBank();
		void slotProjectOpen();
		void slotProjectSave();
		void slotProjectSaveAs();
		void slotProjectClose();

		void slotTrackAudioNew();
		void slotTrackAudioOpen();
		void slotTrackAudioExport();

		void slotTrackMarksNew();
		void slotTrackMarksOpen();
		void slotTrackMarksSaveAllAs();

		void slotTrackTimeLineNew();

		void slotButtonUndo();
		void slotButtonRedo();
		void slotCopy();
		void slotPaste();
		void slotDelete();
		void slotCut();

		void slotApply();
		void slotInfo();

		// Used by all WidgetTrack objects to notify main window about changes
		// in cursor position or selection. In response to this slot, the WindowMain
		// must ask details for given WidgetTrack.
		void slotVisualsChanged(WidgetTracksGroup *_subject);
		void slotSndStart();
		void slotSndPause();
		void slotStepFrames(int);

private:
		Controller *m_ctrl;
		QTimer m_timer_animation;
		WidgetHeaders *m_tabs;
		QPushButton *m_buttonmenu;
		ToolBar::WidgetToolBar *m_toolbar;
		WidgetMainMenu *m_main_menu;

		QStatusBar *m_statusbar;
		QLabel *m_label_freq;
		QLabel *m_label_position;
		QLabel *m_label_range;
		QLabel *m_label_range_len;

		long long m_play_starttime;
		float m_play_from_line;
		bool m_insert;

		int m_panelsheight, m_statusheight;

		std::stringstream m_tmp_SS;

		bool m_scroll_pressed;
		int m_scroll_energy;
		unsigned m_scroll_cnt;
		ProjectManager m_pm;
};

#endif
