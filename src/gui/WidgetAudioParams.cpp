/*
 *  Created on: Mar 17, 2014
 *  pavelkolodin@gmail.com
 */

#include <boost/lexical_cast.hpp>
#include "qfontmetrics.h"
#include "qapplication.h"
#include "qdesktopwidget.h"
#include "WidgetAudioParams.h"

template < typename T1, typename T2, typename T3 > void fillCombo(T1 *_combo, const T2 *_array, T3 _value)
{
	for ( int i = 0; _array[i]; ++i)
	{
		_combo->addItem( QString::fromStdString( boost::lexical_cast<std::string>(_array[i])) );
		if ( _array[i] == _value )
			_combo->setCurrentIndex( i );
	}
}

void placeLabels(QWidget *_parent, int x, int _height, const char **_labels)
{
	int y = 0;
	for ( const char **t = _labels; *t; ++t, y += _height)
	{
		QLabel *l = new QLabel(*t, _parent);

		QFontMetrics qfm(l->font());
		QRect r = qfm.boundingRect(*t);

		l->move( x - r.width(), y + 3 ); // 3 - MAGIC
		l->show();
	}
}

WidgetAudioParams::WidgetAudioParams(QWidget *_parent, AudioParams& _ap)
: QWidget( _parent )
, m_audioparams(_ap)
{

	static const char *labels[] = {"rate", "bits", "channels","bigendian","signed", "float", "frames", 0 };
	placeLabels(this, 90, 30, labels);


	m_rate = new QComboBox(this);
	m_bits = new QComboBox(this);
	m_channels = new QComboBox(this);

	fillCombo(m_rate, AudioParams::m_rates_all, _ap.m_rate );
	fillCombo(m_bits, AudioParams::m_bits_all, _ap.m_bits );
	fillCombo(m_channels, AudioParams::m_channels_all, _ap.m_channels );


	m_frames = new QSpinBox(this);
	m_frames->setRange(0, ULONG_MAX);
	m_frames->setValue ( _ap.m_frames );
	m_frames->setDisabled(true);

	m_bigendian = new QCheckBox(this);
	m_bigendian->setChecked( _ap.m_bigendian );
	m_bigendian->setDisabled(true);

	m_signed = new QCheckBox(this);
	m_signed->setChecked(_ap.m_signed );
	m_signed->setDisabled(true);

	m_float = new QCheckBox(this);
	m_float->setChecked(_ap.m_float);
	m_float->setDisabled(true);


	// Resize
	m_rate		->resize(100, 20);
	m_bits		->resize(100, 20);
	m_channels	->resize(100, 20);
	m_frames	->resize(100, 20);


	// Move
	m_rate		->move(100, 30*0);
	m_bits		->move(100, 30*1);
	m_channels	->move(100, 30*2);
	m_bigendian ->move(100, 30*3);
	m_signed 	->move(100, 30*4);
	m_float 	->move(100, 30*5);
	m_frames	->move(100, 30*6);


	setFixedSize(200, 30*6 + 20); // TODO: why not resize()?
}



void WidgetAudioParams::writeParams()
{
	m_audioparams.m_rate = AudioParams::m_rates_all[ m_rate->currentIndex() ];
	m_audioparams.m_bits = AudioParams::m_bits_all[ m_bits->currentIndex() ];
	m_audioparams.m_channels = AudioParams::m_channels_all[ m_channels->currentIndex() ];
	m_audioparams.m_bigendian = m_bigendian->isChecked();
	m_audioparams.m_signed = m_signed->isChecked();
	m_audioparams.m_float = m_float->isChecked();
}



DialogAudioParams::DialogAudioParams(QWidget *_parent, AudioParams& _ap)
: QDialog(_parent)
, m_wap( new WidgetAudioParams(this, _ap) )
{
	//move()

	setFixedSize(m_wap->width(), m_wap->height() + 50);
	m_wap->move(0,0);
	m_wap->show();

	QPushButton *b = new QPushButton(this);
	b->setText("OK");
	b->resize(50, 20);
	b->move((width()/2)-(50/2), m_wap->height() + 20);
	b->show();

	move( QApplication::desktop()->screen()->rect().center() - QRect(0, 0, width(), height()).center() );

	connect(b, SIGNAL(clicked()), this, SLOT(slotAccept()));
}

void DialogAudioParams::slotAccept()
{
	m_wap->writeParams();
	accept();
}



