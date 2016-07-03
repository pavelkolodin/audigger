/*
 *  pavelkolodin@gmail.com
 */

#ifndef _WidgetToolBank_H_
#define _WidgetToolBank_H_

#include <QtWidgets>
#include "ToolBar.h"
#include "Dragging.h"

namespace ToolBar
{

typedef std::vector < QWidget *> VectorWidgets;

class ToolBankAccount
{
public:
	ToolBankAccount( const ToolInfo &_bi )
	: m_type ( _bi.m_type )
	, m_pixmap ( _bi.m_icon_filename )
	, m_label ( _bi.m_label )
	, m_size( 	0,
				_bi.m_icon_height + TBA_PADDING * 2 )
	, m_height_text( 0 ) // lazy calculation ( in paintEvent )
	{

	}

	void resize( int w, int h ) { m_size.setWidth( w ); m_size.setHeight( h ); }
	void move( int x, int y ) { m_position.setX( x ); m_position.setY( y ); }
	QPoint& pos( ) { return m_position; }
	int width() { return m_size.width(); }
	int height() { return m_size.height(); }

	ToolType m_type;
	QPixmap m_pixmap;
	const char *m_label;
	QSize m_size;
	QPoint m_position;
	int m_height_text;
};

class WidgetToolBankContent : public QWidget
{
	Q_OBJECT
public:
	//WidgetToolBankContent();
	WidgetToolBankContent(QWidget *_parent = NULL);

	void dragTool( int _tool_index, int _coord );
	void dragToolFinish( int _tool_index);

protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void resizeEvent(QResizeEvent *_event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);

private:
	int whosePlace( int _x, int _y );


	typedef std::vector< ToolBankAccount > VectorAccounts;
	VectorAccounts m_accounts;

	Dragging< WidgetToolBankContent, int, int > m_drg_tool;
};

class WidgetToolBank : public QWidget
{
	Q_OBJECT
public:
	WidgetToolBank(QWidget *_parent);

protected:
	virtual void resizeEvent(QResizeEvent *_event);

private:
	WidgetToolBankContent *m_content;
	QScrollArea *m_scrollarea;

};

}


#endif

