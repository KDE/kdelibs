/*
 * Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>
 */
#include <QApplication>
#include <klocalizedstring.h>

#include "knotifytestwindow.h"

int main( int argc, char ** argv )
{
  QApplication a(argc, argv);
  QApplication::setApplicationName("knotifytest");
  QApplication::setApplicationDisplayName(i18n("KNotifyTest"));

  KNotifyTestWindow* knotifytestwindow = new KNotifyTestWindow;
  knotifytestwindow->show();

  return a.exec();
}
