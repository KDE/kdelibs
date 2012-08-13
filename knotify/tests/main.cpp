/*
 * Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>
 */
#include <QApplication>
#include <klocalizedstring.h>
#include <kaboutdata.h>

#include "knotifytestwindow.h"

int main( int argc, char ** argv )
{
  KAboutData aboutData( "knotifytest", 0, qi18n("KNotifyTest"), "0.1",
			qi18n( "A test program for KDE Notifications" ),
			KAboutData::License_GPL,
			qi18n("(c) 2006, Olivier Goffart"));
  aboutData.addAuthor(qi18n("Olivier Goffart"), QLocalizedString(), "ogoffart @ kde.org");
  QApplication::setApplicationName(aboutData.appName());

  QApplication a(argc, argv);
  KNotifyTestWindow* knotifytestwindow = new KNotifyTestWindow;
  knotifytestwindow->show();

  return a.exec();
}
