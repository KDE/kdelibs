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
#include <QStringList>

#include <QDebug>

// TODO debug crash when calling "./kdirwatchtest ./kdirwatchtest"

int main (int argc, char **argv)
{
  // TODO port to QCommandLineArguments once it exists
  //options.add("+[directory ...]", qi18n("Directory(ies) to watch"));

  QCoreApplication a(argc, argv);

  myTest testObject;

  KDirWatch *dirwatch1 = KDirWatch::self();
  KDirWatch *dirwatch2 = new KDirWatch;

  testObject.connect(dirwatch1, SIGNAL(dirty(QString)), SLOT(dirty(QString)) );
  testObject.connect(dirwatch1, SIGNAL(created(QString)), SLOT(created(QString)) );
  testObject.connect(dirwatch1, SIGNAL(deleted(QString)), SLOT(deleted(QString)) );

  // TODO port to QCommandLineArguments once it exists
  const QStringList args = a.arguments();
  for (int i = 1 ; i < args.count() ; ++i ) {
      const QString arg = args.at(i);
      if (!arg.startsWith("-")) {
          qDebug() << "Watching: " << arg;
          dirwatch2->addDir(arg);
      }
  }

  QString home = QString(getenv ("HOME")) + '/';
  QString desk = home + "Desktop/";
  qDebug() << "Watching: " << home;
  dirwatch1->addDir(home);
  qDebug() << "Watching file: " << home << "foo ";
  dirwatch1->addFile(home+"foo");
  qDebug() << "Watching: " << desk;
  dirwatch1->addDir(desk);
  QString test = home + "test/";
  qDebug() << "Watching: (but skipped) " << test;
  dirwatch1->addDir(test);

  dirwatch1->startScan();
  dirwatch2->startScan();

  if(!dirwatch1->stopDirScan(home))
      qDebug() << "stopDirscan: " << home << " error!";
  if(!dirwatch1->restartDirScan(home))
      qDebug() << "restartDirScan: " << home << "error!";
  if (!dirwatch1->stopDirScan(test))
     qDebug() << "stopDirScan: error";

  KDirWatch::statistics();

  delete dirwatch2;

  KDirWatch::statistics();

  return a.exec();
}
