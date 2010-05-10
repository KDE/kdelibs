 /*
  This file is or will be part of KDE desktop environment

  Copyright 1998 Sven Radej <sven@lisa.exp.univie.ac.at>

  It is licensed under GPL version 2.

  If it is part of KDE libraries than this file is licensed under
  LGPL version 2.
 */

#ifndef _KDIRWATCHTEST_H_
#define _KDIRWATCHTEST_H_

#include <stdlib.h>
#include <stdio.h>
#include <QtCore/QObject>

#include "kdirwatch.h"

class myTest : public QObject 
{
   Q_OBJECT
public:
   myTest() { }
public Q_SLOTS:
   void dirty(const QString &a) { printf("Dirty: %s\n", a.toLocal8Bit().constData()); }
   void created(const QString& f) { printf("Created: %s\n", f.toLocal8Bit().constData()); }
   void deleted(const QString& f) { printf("Deleted: %s\n", f.toLocal8Bit().constData()); }
};

#endif
