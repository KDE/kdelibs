 /*
  This file is or will be part of KDE desktop environment

  Copyright 1998 Sven Radej <sven@lisa.exp.univie.ac.at>

  It is licensed under GPL version 2.

  If it is part of KDE libraries than this file is licensed under
  LGPL version 2.
 */

#include "kdirwatchtest.h"
#include <kdebug.h>

int main (int argc, char **argv)
{
  KApplication a(argc, argv, "DirWatchTest");

  myTest testObject;

  KDirWatch *dirwatch = KDirWatch::self();

  QString home = QString(getenv ("HOME")) + "/";
  QString desk = home + "Desktop/";
  kdDebug() << "Watching: " << home << endl;
  kdDebug() << "Watching: " << desk << endl;
  dirwatch->addDir(home);
  dirwatch->addDir(desk);
  QString test = home + "test/";
  dirwatch->addDir(test);
  kdDebug() << "Watching: (but skipped) " << test << endl;

  testObject.connect(dirwatch, SIGNAL( dirty( const QString &)), SLOT( dirty( const QString &)) );
  testObject.connect(dirwatch, SIGNAL( fileDirty( const QString &)), SLOT( fileDirty( const QString &)) );
  testObject.connect(dirwatch, SIGNAL( deleted( const QString &)), SLOT( deleted( const QString &)) );

  dirwatch->startScan();
  if(!dirwatch->stopDirScan(home))
      kdDebug() << "stopDirscan: " << home << " error!" << endl;
  if(!dirwatch->restartDirScan(home))
      kdDebug() << "restartDirScan: " << home << "error!" << endl;
  if (!dirwatch->stopDirScan(test))
     kdDebug() << "stopDirScan: error" << endl;

  return a.exec();
}
#include "kdirwatchtest.moc"
