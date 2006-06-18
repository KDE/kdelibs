/****************************************************************************
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
#include <stdlib.h>
#include <QDesktopWidget>
#include <kcmdlineargs.h>
int main( int argc, char ** argv )
{
    KCmdLineArgs::init( argc, argv, "helpviewer", 0, 0, 0, 0);
    KApplication a;

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
