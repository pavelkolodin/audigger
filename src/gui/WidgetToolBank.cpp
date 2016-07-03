/*
 *  pavelkolodin@gmail.com
 */


#include "WidgetToolBank.h"
#include <stdint.h>
#include "defines.h"
#include <fir/logger/logger.h>

namespace ToolBar
{

extern struct ToolInfo global_toolsinfo[];

WidgetToolBankContent::WidgetToolBankContent(QWidget *_parent)
: QWidget (_parent)
, m_drg_tool ( const_cast<WidgetToolBankContent*>(this), DRAGGING_START_DISTANCE, -9999, 9999 )
{
	const ToolInfo* b = &global_toolsinfo[0];
	unsigned coord = 0;
	for ( ; b->m_type < ToolBar::CONTROL_TYPE_LIMIT; ++b )
	{
		//std::cout << (int)b->type << ", " << b->iconfilename << ", " << b->width << ", " << b->height << "\n";
		//const ToolInfo *bi = &global_toolsinfo[ (int)b->type ]; //findButton( _type );
		//ToolBankAccount *acc = new ToolBankAccount( *b );

		ToolBankAccount tba( *b );
		tba.m_position = QPoint( 0, coord );
		tba.m_size.setHeight( b->m_icon_height + TBA_PADDING * 2 );
		coord += tba.m_size.height();
		coord += TBA_SPACING;

		m_accounts.push_back( tba );
	}
	resize( 555, coord );
}


void WidgetToolBankContent::dragTool( int _tool_index, int _coord )
{
	if ( _tool_index < 0 )
		return;

	m_drg_tool.stop( &WidgetToolBankContent::dragToolFinish );

    QMimeData *mimeData = new QMimeData;
    QByteArray b_array_toolcode;
	uint32_t code = (uint32_t) m_accounts[ _tool_index ].m_type;
	b_array_toolcode.append( (const char*)&code, sizeof(uint32_t) );
	mimeData->setData( "application/x-marker-tool", b_array_toolcode );

    QDrag *drag = new QDrag(this);
    drag->setPixmap( QPixmap(global_toolsinfo[ _tool_index ].m_icon_filename) );
    drag->setMimeData(mimeData);

    //Qt::DropAction dropAction =
    drag->exec(Qt::CopyAction | Qt::MoveAction);
}

void WidgetToolBankContent::dragToolFinish( int _tool_index )
{

}

void WidgetToolBankContent::paintEvent(QPaintEvent *_event)
{
	QPainter pnt( this );

	pnt.fillRect( 0, 0, width(), height(), QColor( 0xff, 0xff, 0xff, 0xff ));

	int vec_size = m_accounts.size();

//	pnt.setPen( QColor(0xcc, 0xcc, 0xcc, 0xff) );
//	for ( int i = 0; i < vec_size; ++i )
//	{
//		pnt.fillRect( m_accounts[i].pos().x(), m_accounts[i].pos().y(), m_accounts[i].width(), m_accounts[i].height(), QColor( 0xdd, 0xdd, 0xdd, 0xff ) );
//		pnt.drawRect( m_accounts[i].pos().x(), m_accounts[i].pos().y(), m_accounts[i].width(), m_accounts[i].height() );
//	}

	pnt.setPen( QColor(0, 0, 0, 0xff) );
	for ( int i = 0; i < vec_size; ++i )
	{
		pnt.drawPixmap( TBA_PADDING + m_accounts[i].pos().x(),
						TBA_PADDING + m_accounts[i].pos().y(),
						m_accounts[i].m_pixmap );

		if ( 0 == m_accounts[i].m_height_text )
		{
			QFontMetrics fm( pnt.font() );
			QRect r = fm.boundingRect( 0, 0,
										m_accounts[i].width(),
										1,
										Qt::AlignLeft | Qt::TextWordWrap, m_accounts[i].m_label );

			m_accounts[i].m_height_text = r.height();
		}

		int y_text_add = (m_accounts[i].height() - m_accounts[i].m_height_text) - TBA_PADDING*2;
		if ( y_text_add < 0 )
			y_text_add = 0;

		int y_text = m_accounts[i].pos().y() + m_accounts[i].m_height_text + y_text_add/2;

		pnt.drawText(	TBA_PADDING + m_accounts[i].pos().x() + m_accounts[i].m_pixmap.width() + WIDTH_TBA_TEXTMARGIN,
						TBA_PADDING + y_text,
						m_accounts[i].m_label );
	}

}

void WidgetToolBankContent::resizeEvent(QResizeEvent *_event)
{
	//deployWidgets(m_accounts, _event->size().width(), _event->size().height(), 2 );

	int len = m_accounts.size();
	for ( int i = 0; i < len; ++i )
	{
		m_accounts[i].m_size.setWidth( width() );
	}
}


void WidgetToolBankContent::mousePressEvent(QMouseEvent *_event)
{
	if ( Qt::LeftButton == _event->button() )
	{
		int index = whosePlace( _event->x(), _event->y() );
		LOGVAR3( index, _event->x(), _event->y() );
		if ( -1 == index )
			return;

		m_drg_tool.start( &WidgetToolBankContent::dragTool, index, _event->y(), 0 );
	}
}

void WidgetToolBankContent::mouseMoveEvent(QMouseEvent *_event)
{
    if (!(_event->buttons() & Qt::LeftButton))
        return;

    m_drg_tool.update( _event->y() );

    //if ((_event->pos() - m_drag_start).manhattanLength() < QApplication::startDragDistance())
        //return;
}

void WidgetToolBankContent::mouseReleaseEvent(QMouseEvent *_event)
{
	m_drg_tool.stop( &WidgetToolBankContent::dragToolFinish );
}

//
// PRIVATE
//

int WidgetToolBankContent::whosePlace( int _x, int _y )
{
	int len = m_accounts.size();

	for ( int i = 0; i < len; ++i )
	{
		if ( _y >= m_accounts[i].m_position.y() && _y < m_accounts[i].m_position.y() + m_accounts[i].m_size.height() )
			return i;
	}
	return -1;
}




WidgetToolBank::WidgetToolBank(QWidget *_parent)
: QWidget (_parent)
{
	m_content = new WidgetToolBankContent( this );
	m_scrollarea = new QScrollArea( this );
	m_scrollarea->setWidget( m_content );
	m_scrollarea->move( 0, 0 );
	m_scrollarea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
	//m_scrollarea->setWidgetResizable( true );
}


void WidgetToolBank::resizeEvent(QResizeEvent *_event)
{
	m_content->resize( width(), m_content->height() );
	m_scrollarea->resize( width(), height() );
}

}

