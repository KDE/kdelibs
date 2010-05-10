 /*
  This file is or will be part of KDE desktop environment

  Copyright 1998 Sven Radej <sven@lisa.exp.univie.ac.at>

  It is licensed under GPL version 2.

  If it is part of KDE libraries than this file is licensed under
  LGPL version 2.
 */

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>

#include <kdebug.h>
#include <kcmdlineargs.h>

#include "kdirwatchtest.h"

int main (int argc, char **argv)
{
  KCmdLineOptions options;
  options.add("+[directory ...]", ki18n("Directory(ies) to watch"));

  KCmdLineArgs::init(argc, argv, "kdirwatchtest", 0, ki18n("KDirWatchTest"),
		     "1.0", ki18n("Test for KDirWatch"));
  KCmdLineArgs::addCmdLineOptions( options );
  KCmdLineArgs::addStdCmdLineOptions();

  QCoreApplication a(argc, argv);
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  myTest testObject;

  KDirWatch *dirwatch1 = KDirWatch::self();
  KDirWatch *dirwatch2 = new KDirWatch;

  testObject.connect(dirwatch1, SIGNAL( dirty( const QString &)), SLOT( dirty( const QString &)) );
  testObject.connect(dirwatch1, SIGNAL( created( const QString &)), SLOT( created( const QString &)) );
  testObject.connect(dirwatch1, SIGNAL( deleted( const QString &)), SLOT( deleted( const QString &)) );

  if (args->count() >0) {
    for(int i = 0; i < args->count(); i++) {
      kDebug() << "Watching: " << args->arg(i);
      dirwatch2->addDir( args->arg(i));
    }
  }

  QString home = QString(getenv ("HOME")) + '/';
  QString desk = home + "Desktop/";
  kDebug() << "Watching: " << home;
  dirwatch1->addDir(home);
  kDebug() << "Watching file: " << home << "foo ";
  dirwatch1->addFile(home+"foo");
  kDebug() << "Watching: " << desk;
  dirwatch1->addDir(desk);
  QString test = home + "test/";
  kDebug() << "Watching: (but skipped) " << test;
  dirwatch1->addDir(test);

  dirwatch1->startScan();
  dirwatch2->startScan();

  if(!dirwatch1->stopDirScan(home))
      kDebug() << "stopDirscan: " << home << " error!";
  if(!dirwatch1->restartDirScan(home))
      kDebug() << "restartDirScan: " << home << "error!";
  if (!dirwatch1->stopDirScan(test))
     kDebug() << "stopDirScan: error";

  KDirWatch::statistics();

  delete dirwatch2;

  KDirWatch::statistics();

  return a.exec();
}
#include "kdirwatchtest.moc"
