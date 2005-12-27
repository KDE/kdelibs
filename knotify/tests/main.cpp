/*
 * Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>
 */
#include <kapplication.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include "knotifytestwindow.h"

static const KCmdLineOptions options[] =
{
    KCmdLineLastOption
};

int main( int argc, char ** argv )
{
  KAboutData aboutData( "knotifytest", I18N_NOOP("KNotifyTest"),
			"0.1", I18N_NOOP( "A test program for KDE Notifications" ),
			KAboutData::License_GPL,
			"(c) 2006, Olivier Goffart");
  aboutData.addAuthor("Olivier Goffart",0, "ogoffart @ kde.org");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication a;

  KNotifyTestWindow* knotifytestwindow = new KNotifyTestWindow;
  a.setMainWidget( knotifytestwindow );
  knotifytestwindow->show();

  return a.exec();
}
