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
#include <QApplication>

int main( int argc, char *argv[] )
{
	QApplication a(argc, argv);
	TestPlot *tp = new TestPlot(0);
	tp->show();
	QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));
	return a.exec();
}
