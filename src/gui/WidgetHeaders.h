/*
 *  pavelkolodin@gmail.com
 */

#ifndef _WidgetHeaders_H_
#define _WidgetHeaders_H_

#include <QtWidgets>
#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include "Dragging.h"


class WidgetLineEdit : public QLineEdit
{
	Q_OBJECT
public:
	WidgetLineEdit(QWidget *_parent);
	virtual void focusOutEvent ( QFocusEvent *_event );
	virtual void keyPressEvent ( QKeyEvent *_event );
};

class WidgetCloseButton : public QWidget
{
	Q_OBJECT
public:
	WidgetCloseButton( QWidget *_parent );
	virtual ~WidgetCloseButton();
	virtual void enterEvent(QEvent *);
	virtual void leaveEvent(QEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);

	virtual void paintEvent(QPaintEvent *);

signals:
	void clicked(unsigned);
public:
	unsigned m_id;

private:
	boost::shared_ptr<QPixmap> m_pixmap_close;
	boost::shared_ptr<QPixmap> m_pixmap_circle_red;
	bool m_under_mouse;
};


class WidgetSpace : public QWidget
{
	Q_OBJECT
public:
	WidgetSpace( QWidget *_parent, int _width )
	: QWidget ( _parent )
	{
		resize( _width, 1 );
	}
};



//
// HEADER OPTIONS
//

static const int HEADER_OPT_AUTORESIZE = 1;
static const int HEADER_OPT_CLOSEABLE = (1 << 2);


//
// TrackHeaderInfo
//

class TrackHeaderInfo
{
public:
	//TrackHeaderInfo( QWidget *_parent, const QString &_text, bool _autoresize, bool _closeable);
	TrackHeaderInfo( QWidget *_parent, const QString &_text, int _options);

	virtual ~TrackHeaderInfo();
	void textSize();
	void rename( const QString &_text );
	void resize( unsigned _width, unsigned _height );
	void setIndex(unsigned);
	// return: true - animated, false - nothing to animate
	bool animate();
	// go to last animation frame
	void finishAnimation();
	void addWidget(QWidget *);

private:
	void updateWidgetsPositions();
	QWidget *m_parent;
	std::vector< QWidget* > m_widgets;
public:
	boost::shared_ptr<WidgetCloseButton> m_close_button;

private:
	//unsigned m_index;

public:
	QString m_text;
	int m_text_x;
	int m_text_y;
	int m_text_width;
	int m_text_height;


	unsigned m_width;
	unsigned m_height;
	int m_pos_goal;
	int m_pos;


	bool m_selected;
	int m_options;
};



// Class WidgetHeaders
// draw headers for tracks.
class WidgetHeaders : public QWidget
{
	Q_OBJECT
public:


	WidgetHeaders(QWidget *_parent);

	void setHasButtonAdd(bool _f);
	void setTabLook(bool _f)			{ m_tab_look = _f; }
	void setFitScreen(bool _f)			{ m_fit_screen = _f; }
	void setSelectable(bool _f)		{ m_selectable = _f; }
	void setAnimated(bool _f)			{ m_animated = _f; }

	void changeBorderPos( int, int );
	void changeHeaderPos( int, int );
	void finishBorderPos( int ) { }
	void finishHeaderPos( int );

	bool isSelected(unsigned _index);

protected:
	virtual void resizeEvent(QResizeEvent *);
	virtual void mousePressEvent ( QMouseEvent * _event );
	virtual void mouseReleaseEvent( QMouseEvent *_event );
	virtual void mouseMoveEvent ( QMouseEvent *_event );
	virtual void mouseDoubleClickEvent ( QMouseEvent *_event );
	virtual void paintEvent(QPaintEvent *_event);
	virtual void leaveEvent( QEvent *_event );


	void paintHeader(QPainter &_pnt, int _index);
	void paintTab(QPainter &_pnt, int _index);
	//void paintCloseButton(QPainter &_pnt, int _index, int _height_offset);
	void fitHeaders(unsigned _starting_from);
	//void changeSpace(unsigned )

	int whosePlace(int _coord);
	int whoseRightBorder(int _coord);
	int whoseCloseButton(int x, int y);
	void updateAllPositions( );
	void finishAnimation( );
	void enumerate();

public slots:
	void animate();

	// \return index of added header
	unsigned addHeader(const QString &_name, int _width, int _options);

	void delHeader(unsigned _index);
	void rename(unsigned _index, const QString& _name);

	// _index == -1 : remove all selections.
	void setSelected(unsigned _index, bool _flag);
	void setSelectedAll( bool _flag );

	// WidgetHeader takes QWidget's ownership.
	void addWidgetToHeader(unsigned _index, QWidget *);

private slots:
	void slotRenameEditFinished();
	void slotCloseButtonClicked(unsigned _index_tab);
	void slotButtonAdd();

signals:
	void signalAdd();
	void signalClose(unsigned _index);
	void signalSelected(unsigned _index);
	void signalMove(unsigned _from, unsigned _to);
	void signalResize(unsigned _index, unsigned _size);
	void signalRename(unsigned _index, const QString &_text);
	void signalMenu(unsigned _index, int x, int y);

private:
	Dragging< WidgetHeaders, int, int > m_drg_header, m_drg_border;
	int m_index_pressed;

	std::vector< TrackHeaderInfo* > m_headers;
	boost::recursive_mutex m_mutex;

	WidgetLineEdit *m_rename_edit;
	int m_index_editing;
	QPushButton *m_buttonadd;

	bool m_has_button_add;
	bool m_tab_look;
	bool m_fit_screen;
	bool m_selectable;
	bool m_animated;
};


#endif

