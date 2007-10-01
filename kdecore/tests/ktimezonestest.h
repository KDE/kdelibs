/* This file is part of the KDE libraries
   Copyright (c) 2005-2007 David Jarvie <software@astrojar.org.uk>

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

#ifndef KTIMEZONESTEST_H
#define KTIMEZONESTEST_H

#include <QtCore/QObject>

class KTimeZonesTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void ktimezones();
    void refcount();
    void utc();
    void local();
    void zone();
    void zoneinfoDir();
    void zonetabChange();
    void currentOffset();
    void offsetAtUtc();
    void offsetAtZoneTime();
    void abbreviation();
    void timet();
    void toUtc();
    void toZoneTime();
    void convert();
    void tzfile();
    void tzfileToZoneTime();
    void tzfileOffsetAtZoneTime();
    void tzfileUtcOffsets();
    void tzfileAbbreviation();
    void tzfileTransitions();
private:
    void removeDir(const QString &subdir);
    void writeZoneTab(bool testcase);
    QString mDataDir;
};

#endif
