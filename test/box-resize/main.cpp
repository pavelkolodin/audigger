

#include "WinWin.h"

int main ( int argc, char **argv )
{
	QApplication app(argc, argv);

	WinWin winwin;
	winwin.show();

    return app.exec();
}
