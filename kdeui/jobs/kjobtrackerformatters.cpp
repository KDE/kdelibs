/*  This file is part of the KDE project
    Copyright (c) 1997,2001 Stephan Kulow <coolo@kde.org>
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>
    Copyright (c) 1999-2002 Hans Petter Bieker <bieker@kde.org>
    Copyright (c) 2002 Lukas Tinkl <lukas@kde.org>
    Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>
    Copyright (C) 2009, 2010 John Layt <john@layt.net>

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

#include "kjobtrackerformatters_p.h"

#include <QObject>

QString KJobTrackerFormatters::byteSize(double size)
{
    QList<QString> units;
    units << QObject::tr("%1 B")
          << QObject::tr("%1 KiB")
          << QObject::tr("%1 MiB")
          << QObject::tr("%1 GiB")
          << QObject::tr("%1 TiB")
          << QObject::tr("%1 PiB")
          << QObject::tr("%1 EiB")
          << QObject::tr("%1 ZiB")
          << QObject::tr("%1 YiB");

    int unit = 0; // Selects what unit to use
    const double multiplier = 1024.0;

    while (qAbs(size) >= multiplier && unit < units.size()) {
        size /= multiplier;
        unit++;
    }

    if (unit == 0) {
        // Bytes, no rounding
        return units[unit].arg(QString::number(size, 'f', 0));
    } else {
        return units[unit].arg(QString::number(size, 'f', 1));
    }
}

QString KJobTrackerFormatters::daysDuration(int n)
{
    return QObject::tr("%n day(s)", "@item:intext", n);
}

QString KJobTrackerFormatters::hoursDuration(int n)
{
    return QObject::tr("%n hour(s)", "@item:intext", n);
}

QString KJobTrackerFormatters::minutesDuration(int n)
{
    return QObject::tr("%n minute(s)", "@item:intext", n);
}

QString KJobTrackerFormatters::secondsDuration(int n)
{
    return QObject::tr("%n second(s)", "@item:intext", n);
}

QString KJobTrackerFormatters::duration(unsigned long mSec)
{
    unsigned long ms = mSec;
    int days = ms / (24 * 3600000);
    ms = ms % (24 * 3600000);
    int hours = ms / 3600000;
    ms = ms % 3600000;
    int minutes = ms / 60000;
    ms = ms % 60000;
    int seconds = qRound(ms / 1000.0);

    // Handle correctly problematic case #1 (look at KLocaleTest::prettyFormatDuration()
    // at klocaletest.cpp)
    if (seconds == 60) {
        return duration(mSec - ms + 60000);
    }

    if (days && hours) {
        return QObject::tr("%1 and %2", "@item:intext days and hours.").arg(daysDuration(days)).arg(hoursDuration(hours));
    } else if (days) {
        return daysDuration(days);
    } else if (hours && minutes) {
        return QObject::tr("%1 and %2", "@item:intext hours and minutes.").arg(hoursDuration(hours)).arg(minutesDuration(minutes));
    } else if (hours) {
        return hoursDuration(hours);
    } else if (minutes && seconds) {
        return QObject::tr("%1 and %2", "@item:intext minutes and seconds.").arg(minutesDuration(minutes)).arg(secondsDuration(seconds));
    } else if (minutes) {
        return minutesDuration(minutes);
    } else {
        return secondsDuration(seconds);
    }
}

