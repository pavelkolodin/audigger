/*
 * pavelkolodin@gmail.com
 */

#include "WindowAbout.h"

WindowAbout::WindowAbout( QWidget *_parent )
//	: QDialog(_parent)
{
	setModal( true );

	QPushButton *m_b_OK = new QPushButton(this);

	m_b_OK->setText( "OK" );

	int wb = m_b_OK->width();
	int hb = m_b_OK->height();

	QLabel *m_label = new QLabel(this);

	m_label->move( 20, 20 );
	m_label->setFixedWidth( 300 );
	m_label->setWordWrap( true );
	m_label->setText(
						"Audigger website: <a href=\"mailto:pavelkolodin@gmail.com\">http://audigger.ru/</a><br/>"\
					 	"Author: Pavel Kolodin <a href=\"mailto:pavelkolodin@gmail.com\">pavelkolodin@gmail.com</a><br/>"\
						"Free Software. BSD License.<br/>"\
						"http://en.wikipedia.org/wiki/BSD_licenses\n"
					 	"<br/>"\
					 	"Build: " __DATE__ ", " __TIME__ );

	m_label->setTextInteractionFlags( 	Qt::TextSelectableByMouse |
										Qt::TextSelectableByKeyboard |
										Qt::LinksAccessibleByMouse |
										Qt::LinksAccessibleByKeyboard );
	m_label->setTextFormat( Qt::RichText );

	int wt = m_label->sizeHint().width();
	int ht = m_label->sizeHint().height();
	m_label->resize( wt, ht );

	m_b_OK->move ( 20 + (wt/2 - m_b_OK->width()/2), ht + 20 + 20);

	resize(wt + 20 + 20, ht + 20 + 20 + m_b_OK->height());

	setMinimumSize( width(), height() );
	setMaximumSize( width(), height() );

	connect(m_b_OK, SIGNAL(clicked()), this, SLOT(slotOK()));
}

WindowAbout::~WindowAbout()
{

}

void WindowAbout::slotOK()
{
	accept();
}




