/*
 * Created on: Mar 17, 2014
 * : pavelkolodin@gmail.com
 */

#ifndef WIDGETAUDIOPARAMS_H_
#define WIDGETAUDIOPARAMS_H_

#include <qwidget.h>
#include <qlabel.h>
#include <qdialog.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qpushbutton.h>

#include "AudioParams.h"

class WidgetAudioParams : public QWidget
{
	Q_OBJECT
public:
	WidgetAudioParams(QWidget *_parent, AudioParams& _audio_params_to_change);

	// transfer params to AudioParams struct.
	void writeParams();

protected:
	//QLabel *m_labels;



	QComboBox *m_rate;
	QComboBox *m_bits;
	QComboBox *m_channels;
	QSpinBox *m_frames;

	QCheckBox *m_bigendian;
	QCheckBox *m_signed;
	QCheckBox *m_float;

	AudioParams &m_audioparams;
};


class DialogAudioParams : public QDialog
{
	Q_OBJECT
public:
	DialogAudioParams(QWidget *_parent, AudioParams& _ap);

protected:
	WidgetAudioParams *m_wap;

public slots:
	void slotAccept();

};


#endif /* WIDGETAUDIOPARAMS_H_ */
