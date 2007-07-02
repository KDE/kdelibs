/*
 * Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>
 */
#include <kapplication.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include "knotifytestwindow.h"

int main( int argc, char ** argv )
{
  KCmdLineOptions options;
  KAboutData aboutData( "knotifytest", 0, ki18n("KNotifyTest"), "0.1",
			ki18n( "A test program for KDE Notifications" ),
			KAboutData::License_GPL,
			ki18n("(c) 2006, Olivier Goffart"));
  aboutData.addAuthor(ki18n("Olivier Goffart"), KLocalizedString(), "ogoffart @ kde.org");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication a;
  KNotifyTestWindow* knotifytestwindow = new KNotifyTestWindow;
  knotifytestwindow->show();

  return a.exec();
}
