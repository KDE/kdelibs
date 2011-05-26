/* This file is part of the KDE libraries
   Copyright 1998 Sven Radej <sven@lisa.exp.univie.ac.at>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kdirwatchtest.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>

#include <kdebug.h>
#include <kcmdlineargs.h>

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
