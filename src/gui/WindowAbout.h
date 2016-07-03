
#pragma once

#include <QtWidgets>

class WindowAbout : public QDialog
{
	Q_OBJECT
public:
	WindowAbout( QWidget *_parent );
	virtual ~WindowAbout();

private:
	WindowAbout();
	WindowAbout ( const WindowAbout& _other ) { }
	WindowAbout& operator= ( const WindowAbout& _other ) { return *this; }

private slots:
	void slotOK();

};
