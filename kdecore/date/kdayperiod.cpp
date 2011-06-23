/*
    Copyright (c) 2010 John Layt <john@layt.net>

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

#include "kdayperiod_p.h"

#include <QtCore/QSharedData>
#include <QtCore/QString>
#include <QtCore/QTime>

class KDayPeriodPrivate : public QSharedData
{
public:
    KDayPeriodPrivate(const QString &periodCode,
                      const QString &longName,
                      const QString &shortName,
                      const QString &narrowName,
                      const QTime &periodStart,
                      const QTime &periodEnd,
                      int offsetFromStart,
                      int offsetIfZero);
    KDayPeriodPrivate(const KDayPeriodPrivate &other);
    ~KDayPeriodPrivate();

    QString m_periodCode, m_longName, m_shortName, m_narrowName;
    QTime m_periodStart, m_periodEnd;
    int m_offsetFromStart, m_offsetIfZero;
};

KDayPeriodPrivate::KDayPeriodPrivate(const QString &periodCode,
                                     const QString &longName,
                                     const QString &shortName,
                                     const QString &narrowName,
                                     const QTime &periodStart,
                                     const QTime &periodEnd,
                                     int offsetFromStart,
                                     int offsetIfZero)
                 : QSharedData(),
                   m_periodCode(periodCode),
                   m_longName(longName),
                   m_shortName(shortName),
                   m_narrowName(narrowName),
                   m_periodStart(periodStart),
                   m_periodEnd(periodEnd),
                   m_offsetFromStart(offsetFromStart),
                   m_offsetIfZero(offsetIfZero)
{
}

KDayPeriodPrivate::KDayPeriodPrivate(const KDayPeriodPrivate &other)
                 : QSharedData(other),
                   m_periodCode(other.m_periodCode),
                   m_longName(other.m_longName),
                   m_shortName(other.m_shortName),
                   m_narrowName(other.m_narrowName),
                   m_periodStart(other.m_periodStart),
                   m_periodEnd(other.m_periodEnd),
                   m_offsetFromStart(other.m_offsetFromStart),
                   m_offsetIfZero(other.m_offsetIfZero)
{
}

KDayPeriodPrivate::~KDayPeriodPrivate()
{
}

KDayPeriod::KDayPeriod(const QString &periodCode,
                       const QString &longName,
                       const QString &shortName,
                       const QString &narrowName,
                       const QTime &periodStart,
                       const QTime &periodEnd,
                       int offsetFromStart,
                       int offsetIfZero)
          : d(new KDayPeriodPrivate(periodCode,
                                    longName,
                                    shortName,
                                    narrowName,
                                    periodStart,
                                    periodEnd,
                                    offsetFromStart,
                                    offsetIfZero))
{
}

KDayPeriod::KDayPeriod()
          : d(new KDayPeriodPrivate(QString(), QString(), QString(), QString(), QTime(), QTime(), -1, -1))
{
}

KDayPeriod::KDayPeriod(const KDayPeriod &rhs)
          : d(rhs.d)
{
}

KDayPeriod::~KDayPeriod()
{
}

KDayPeriod& KDayPeriod::operator=(const KDayPeriod &rhs)
{
    if (&rhs != this) {
        d = rhs.d;
    }
    return *this;
}

QString KDayPeriod::periodCode() const
{
    return d->m_periodCode;
}

QTime KDayPeriod::periodStart() const
{
    return d->m_periodStart;
}

QTime KDayPeriod::periodEnd() const
{
    return d->m_periodEnd;
}

QString KDayPeriod::periodName(KLocale::DateTimeComponentFormat format) const
{
    if (format == KLocale::LongName) {
        return d->m_longName;
    } else if (format == KLocale::NarrowName) {
        return d->m_narrowName;
    } else {
        return d->m_shortName;
    }
}

int KDayPeriod::hourInPeriod(const QTime &time) const
{
    int hourInPeriod = -1;
    if (time.isValid() && isValid(time)) {
        hourInPeriod = time.hour() - periodStart().hour() + d->m_offsetFromStart;
        while (d->m_offsetIfZero > 0 && hourInPeriod <= 0) {
            hourInPeriod = hourInPeriod + d->m_offsetIfZero;
        }
    }
    return hourInPeriod;
}

QTime KDayPeriod::time(int hip, int minute, int second, int millisecond) const
{
    QTime time;
    if (isValid()) {
        if (hip == d->m_offsetIfZero) {
            hip = 0;
        }
        int hour;
        if (periodStart() <= periodEnd() ||
                (hip >= hourInPeriod(periodStart()) &&
                 hip <= hourInPeriod(QTime(23, 59, 59, 999)))) {
            hour = hip + periodStart().hour() - d->m_offsetFromStart;
        } else {
            hour = hip;
        }
        time =  QTime(hour, minute, second, millisecond);
        if (time.isValid() && isValid(time)) {
            return time;
        } else {
            return QTime();
        }
    }
    return time;
}

bool KDayPeriod::isValid() const
{
    return !d->m_periodCode.isEmpty() &&
           d->m_periodStart.isValid() &&
           d->m_periodEnd.isValid();
}

bool KDayPeriod::isValid(const QTime &time) const
{
    if (isValid()) {
        if (periodStart() <= periodEnd()) {
            return time >= periodStart() && time <= periodEnd();
        } else {
            return ((time >= periodStart() && time <= QTime(23, 59, 59, 999)) ||
                    (time >= QTime(0, 0, 0) && time <= periodEnd()));
        }
    } else {
        return false;
    }
}

