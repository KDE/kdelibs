/***************************************************************************
                          testplot_main.cpp  -  description
                             -------------------
    begin                : Thu Oct 26 2006
    copyright            : (C) 2006 by Jason Harris
    email                : kstars@30doradus.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "testplot_widget.h"
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>

int main( int argc, char *argv[] )
{
	KAboutData aboutData( "testplot", 0, ki18n("Test KPlotWidget"), "0.1",
				ki18n("KPlotWidget test program"),
				KAboutData::License_GPL,
				ki18n("(c) 2006, Jason Harris"),
				ki18n("Performs various tests of KPlotWidget"),
				"http://edu.kde.org/");
	aboutData.addAuthor(ki18n("Jason Harris"), ki18n("no task"),
				"kstars@30doradus.org", "http://edu.kde.org/");

	KCmdLineArgs::init( argc, argv, &aboutData );

	KApplication a;
	TestPlot *tp = new TestPlot(0);
	tp->show();
	QObject::connect(kapp, SIGNAL(lastWindowClosed()), kapp, SLOT(quit()));
	return a.exec();
}
