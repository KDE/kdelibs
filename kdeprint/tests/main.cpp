/****************************************************************************
** $Id: main.cpp 117166 2001-10-10 17:40:42Z mueller $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "helpwindow.h"
#include <kapplication.h>
#include <qwindowsstyle.h>
#include <qstylesheet.h>
#include <stdlib.h>


int main( int argc, char ** argv )
{
    KApplication a(argc, argv, "helpviewer");

    QString home;
    if (argc > 1)
        home = argv[1];
    else
        home = QString(getenv("QTDIR")) + "/doc/html/index.html";

    
    HelpWindow *help = new HelpWindow(home, ".", 0, "help viewer");

    if ( QApplication::desktop()->width() > 400
	 && QApplication::desktop()->height() > 500 )
	help->show();
    else
	help->showMaximized();

    QObject::connect( &a, SIGNAL(lastWindowClosed()),
                      &a, SLOT(quit()) );

    return a.exec();
}
