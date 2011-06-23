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
