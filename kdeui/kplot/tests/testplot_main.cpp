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

static const char description[] = I18N_NOOP("KPlotWidget test program");
static const char notice[] = I18N_NOOP("Performs various tests of KPlotWidget");

static KCmdLineOptions options[] =
{
	KCmdLineLastOption
};

int main( int argc, char *argv[] )
{
	KAboutData aboutData( "testplot", I18N_NOOP("Test KPlotWidget"),
				"0.1", description, KAboutData::License_GPL,
				I18N_NOOP("(c) 2006, Jason Harris"), notice,
				"http://edu.kde.org/");
	aboutData.addAuthor("Jason Harris", 0,
				"kstars@30doradus.org", "http://edu.kde.org/");

	KCmdLineArgs::init( argc, argv, &aboutData );

	KApplication a;
	TestPlot *tp = new TestPlot(0);
	tp->show();
	QObject::connect(kapp, SIGNAL(lastWindowClosed()), kapp, SLOT(quit()));
	return a.exec();
}
