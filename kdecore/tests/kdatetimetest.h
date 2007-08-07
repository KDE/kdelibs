/* This file is part of the KDE libraries
   Copyright (c) 2005 David Jarvie <software@astrojar.org.uk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDATETIMETEST_H
#define KDATETIMETEST_H

#include <QtCore/QObject>

class KDateTimeTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void specConstructors();
    void specSet();
    void constructors();
    void toUtc();
    void toOffsetFromUtc();
    void toLocalZone();
    void toClockTime();
    void toZone();
    void toTimeSpec();
    void set();
    void compare();
    void addSubtract();
    void addMSecs();
    void addSubtractDate();
    void dstShifts();
    void strings_iso8601();
    void strings_rfc2822();
    void strings_qttextdate();
    void strings_format();
    void cache();
    void misc();
private:
    void removeDir(const QString &subdir);
    QString mDataDir;
};

#endif
