/* This file is part of the KDE libraries

    Copyright (c) 2001  Martin R. Jones <mjones@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include <qdialog.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kcrash.h>

#include "kscreensaver.h"
#include "kscreensaver_vroot.h"

extern "C"
{
    extern const char *kss_applicationName;
    extern const char *kss_description;
    extern const char *kss_version;
    KScreenSaver *kss_create( WId d );
    QDialog *kss_setup();
};

static const KCmdLineOptions options[] =
{
  { "setup", I18N_NOOP("Setup screen saver."), 0 },
  { "window-id wid", I18N_NOOP("Run in the specified XWindow."), 0 },
  { "root", I18N_NOOP("Run in the root XWindow."), 0 },
  { "demo", I18N_NOOP("Start screen saver in demo mode."), "default"},
  { 0,0,0 }
};

static void crashHandler( int sig )
{
#ifdef SIGABRT
    signal (SIGABRT, SIG_DFL);
#endif
    abort();
}

//----------------------------------------------------------------------------

class DemoWindow : public QWidget
{
public:
    DemoWindow() : QWidget()
    {
	setFixedSize(600, 420);
    }

protected:
    virtual void keyPressEvent(QKeyEvent *e)
    {
        if (e->ascii() == 'q')
        {
            kapp->quit();
        }
    }

    virtual void closeEvent( QCloseEvent * )
    {
        kapp->quit();
    }
};


//----------------------------------------------------------------------------
#ifdef Q_WS_QWS
typedef WId Window;
#endif

int main(int argc, char *argv[])
{
    KLocale::setMainCatalogue("libkscreensaver");
    KCmdLineArgs::init(argc, argv, kss_applicationName, kss_description, kss_version);

    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    KCrash::setCrashHandler( crashHandler );
    KGlobal::locale()->insertCatalogue("klock");
    KGlobal::locale()->insertCatalogue("kscreensaver");

    DemoWindow *demoWidget = 0;
    Window saveWin = 0;
    KScreenSaver *target;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	
    if (args->isSet("setup"))
    {
       QDialog *dlg = kss_setup();
       dlg->exec();
       delete dlg;
       exit(0);
    }

    if (args->isSet("window-id"))
    {
        saveWin = atol(args->getOption("window-id"));
    }

#ifdef Q_WS_X11 //FIXME
    if (args->isSet("root"))
    {
        saveWin = RootWindow(qt_xdisplay(), qt_xscreen());
    }
#endif

    if (args->isSet("demo"))
    {
        saveWin = 0;
    }

    if (saveWin == 0)
    {
        demoWidget = new DemoWindow();
        demoWidget->setBackgroundMode(QWidget::NoBackground);
        saveWin = demoWidget->winId();
        app.setMainWidget(demoWidget);
        app.processEvents();
    }

    target = kss_create( saveWin );

    if ( demoWidget )
    {
        demoWidget->setFixedSize( 600, 420 );
        demoWidget->show();
    }

    app.exec();

    delete target;
    if ( demoWidget )
        delete demoWidget;

    return 0;
}

