/*
    Copyright 2010 John Layt <john@layt.net>

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

#ifndef KCALENDARERA_H
#define KCALENDARERA_H

#include <QtCore/QString>
#include <QtCore/QDate>

class KCalendarSystemPrivate;

/**
  * @internal
  * Class to hold details of a Calendar Era.  Internal for now, but may later be
  * made public if user configuration of Era formatting via System Settings is
  * required, in which case a d-ptr will be needed.  Details based on POSIX LC_TIME
  * format.
  *
  * @b license GNU-LGPL v.2 or later
  *
  * @see KCalendarSystem
  *
  * @author John Layt <john@layt.net>
*/
class KCalendarEra
{
public:
    explicit KCalendarEra();
    virtual ~KCalendarEra();

    bool isValid() const;

    int sequence() const;
    QDate startDate() const;
    QDate endDate() const;
    QString name() const;
    QString shortName() const;
    QString format() const;
    int direction() const;
    int offset() const;

    bool isInEra( const QDate &date ) const;
    int yearInEra( int year ) const;
    int year( int yearInEra ) const;

private:
    friend class KCalendarSystemPrivate;

    int m_sequence;
    QDate m_startDate;
    int m_startYear;
    QDate m_endDate;
    QString m_name;
    QString m_shortName;
    QString m_format;
    int m_direction;
    int m_offset;
};

#endif // KCALENDARERA_H
