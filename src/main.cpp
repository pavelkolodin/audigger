
#include <iostream>
#include <QApplication>
#include <ctime>
#include <boost/shared_ptr.hpp>

#include "Controller.h"
#include "WindowMain.h"
//#include "debug.h"

int main (int argc, char **argv)
{
	srand( std::time(NULL) );

	QApplication app(argc, argv);

	boost::shared_ptr< Controller > ctrl ( new Controller );
	boost::shared_ptr< WindowMain > mainwin ( new WindowMain( ctrl.get() ) );
	mainwin->show();

	if ( argc > 1 )
	{
		mainwin->slotProjectNew();

		//mainwin->getProjectManager()->addTrackMarksNew();
		mainwin->getProjectManager()->addTrackTimeLine();
		for ( int i = 1; i < argc; ++i )
		{
			LOG_PURE( "Opening " << argv[i] );
			//mainwin->getProjectManager()->addTrackAudio( argv[i], WidgetTrackAbstract::TYPE_WAVE  );
			mainwin->getProjectManager()->addTrackAudio( argv[i], WidgetTrackAbstract::TYPE_SONOGRAM  );
		}

		mainwin->getProjectManager()->getCurrWTG()->refresh();
	}

    return app.exec();
}
