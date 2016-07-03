/*
 *  pavelkolodin@gmail.com
 */

#ifndef _WidgetTracksContainer_H_
#define _WidgetTracksContainer_H_

#include <QtWidgets>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "WidgetHeaders.h"
#include "WidgetTrackAbstract.h"
#include "ITracksGroupClient.h"

#include "WidgetsCommon.h"
#include "EventDistributor.h"


class WidgetTracksGroup : public WidgetTrackAbstract, public ITracksGroupClient, public EventDistributor
{
	Q_OBJECT
	public:
		Q_PROPERTY(int animparam WRITE setAnimParam);

		static const int HEIGHT_HEADER = 25;
		static const int WIDTH_SCROLLBAR = 20;

		WidgetTracksGroup (QWidget *_parent, Controller *_ctrl, TracksGroup::Ptr _group);
		virtual ~WidgetTracksGroup ( );

		void addTrack( TrackAudio::Ptr _t, WidgetTrackAbstract::TrackType _type );
		void addTrack( TrackMarks::Ptr _t );
		void addTrack( TrackTimeLine::Ptr _t );
		size_t numTracks();
		WidgetTrackAbstract* getTrack(size_t _index);
		virtual void scroll( int );

		void setAnimParam(int _param);

		void startAnimation();
		void stopAnimation();

		void undo();
		void redo();

		void load();


		// WidgetTrackAbstract:

		virtual TrackType getType() { return TYPE_GROUP; }
		virtual TracksGroup::Ptr getGroup() { return m_group; }

		virtual void setCursorPos( TrackPos );
		virtual void refresh();
		virtual void setOffset( TrackPos );
		virtual TrackPos getLenPixels();

		virtual TrackPos moveCursor( TrackPosSigned _pixels );
		virtual void setMouseMode( WidgetTrackAbstract::MouseMode );

		// Used by sonogram only.
		//virtual void setFFTWin( unsigned _win );
		// Used by sonogram, marks.
		virtual void setStepFrames( unsigned _step );

		//virtual void trackChanged( WidgetTrackAbstract *_whose );
		//virtual void selectTrack( WidgetTrackAbstract *_track );
		virtual void setSelectedChild( WidgetTrackAbstract* _track, bool _selected );


		// ITrackGroupClient
		virtual void trackCreated( TrackAudio::Ptr );
		virtual void trackCreated( TrackMarks::Ptr );

	protected:
		//virtual void paintEvent(QPaintEvent *);
		virtual void resizeEvent(QResizeEvent *);
	private:
		void setOffset__(WidgetTrackAbstract::TrackPos);
		void recalcWidgetsPosition__();
		void fixScrollBar__();
		void addTrack__( WidgetTrackAbstract*, const QString &_name = QString("untitled") );
		WidgetTrackAbstract* findFirstSelected__();
        WidgetTracksGroup& operator= ( const WidgetTracksGroup& _other ) { return *this; }

        TracksGroup::Ptr m_group;
        QElapsedTimer m_animation_elapsed_timer;
        std::vector <WidgetTrackAbstract*> m_widgets;
        boost::shared_ptr<QPropertyAnimation> m_animation;

        WidgetHeaders *m_headers;
        QScrollBar *m_scrollbar;
        // widgetsholder - need to separate widgettracks from qscrollbar.
        QWidget *m_widgetsholder;
        WidgetTrackAbstract* m_widgettrack_current;
        WidgetTrackAbstract::TrackPos m_animation_start_pos;
        // index of header for which context menu is displayed.
        int m_index_header_menu;

private slots:
        void slotScrollVerticalChanged(int);

        void slotHeaderClose(unsigned);
        void slotHeaderSelected( unsigned _index );
        void slotHeaderMove( unsigned _from, unsigned _to );
        void slotHeaderResize( unsigned _index, unsigned _size );
    	void slotHeaderRename(unsigned _index, const QString &_text);
    	void slotHeaderMenu(unsigned _index, int x, int y);

    	// Open new FFT WidgetTrack for track index "m_index_header_menu"
    	void slotNewFFT();
    	// Open new WaveForm WidgetTrack for track index "m_index_header_menu"
    	void slotNewWaveForm();

    	void slotSaveAsWav();
};


#endif

