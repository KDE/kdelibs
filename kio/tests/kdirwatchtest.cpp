 /*
  This file is or will be part of KDE desktop environment

  Copyright 1998 Sven Radej <sven@lisa.exp.univie.ac.at>

  It is licensed under GPL version 2.

  If it is part of KDE libraries than this file is licensed under
  LGPL version 2.
 */

#include "kdirwatchtest.h"

int main (int argc, char **argv)
{
  KApplication a(argc, argv, "DirWatchTest");
  
  myTest testObject;

  printf( "You must create directory test in your home\n");
  printf( "Directory test will be watched, but skipped\n");
  printf( "When test is changed, you will be notified on console\n");
  printf( "Open kfms on home and test and move/copy files between them and root\n");
  printf( "Note that there will allways be output for test\n");

  KDirWatch *dirwatch = new KDirWatch();

  QString home = getenv ("HOME");
  QString desk = getenv ("HOME");
  desk.append("/Desktop/");
  home.append("/");
  printf( "Watching: \n%s\n%s\n", home.ascii(), desk.ascii());
  dirwatch->addDir(home.data());
  home.append("test/");
  dirwatch->addDir(home.data());
  dirwatch->addDir(desk.data());
  printf( "Watching: (but skipped) \n%s\n", home.data());

  testObject.connect(dirwatch, SIGNAL( dirty( const QString &)), 
	SLOT( dirty( const QString &)) );
  
  dirwatch->startScan();
  if (!dirwatch->stopDirScan(home.data()))
    printf( "stopDirScan: error\n");

  return a.exec();
}
#include "kdirwatchtest.moc"
