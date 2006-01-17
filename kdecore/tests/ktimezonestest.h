/* This file is part of the KDE libraries
    Copyright (c) 2005 David Jarvie <software@astrojar.org.uk>

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

#ifndef KTIMEZONESTEST_H
#define KTIMEZONESSTEST_H

#include <qobject.h>

class KTimezonesTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void ktimezones();
    void utc();
    void local();
    void zone();
    void zoneinfoDir();
    void currentOffset();
    void offsetAtUTC();
    void offsetAtZoneTime();
    void abbreviation();
    void toUTC();
    void toZoneTime();
    void convert();
    void tzfile();
    void tzfileOffsetAtZoneTime();
    void tzfileUTCOffsets();
};

#endif
