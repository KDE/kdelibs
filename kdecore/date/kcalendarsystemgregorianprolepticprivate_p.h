/*
    Copyright 2009, 2010 John Layt <john@layt.net>

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

#ifndef KCALENDARSYSTEMGREGORIANPROLEPTICPRIVATE_H
#define KCALENDARSYSTEMGREGORIANPROLEPTICPRIVATE_H

// Derived gregorian kde calendar class

#include "kcalendarsystemprivate_p.h"

class KCalendarSystemGregorianProlepticPrivate : public KCalendarSystemPrivate
{
public:
    explicit KCalendarSystemGregorianProlepticPrivate( KCalendarSystemGregorianProleptic *q );

    virtual ~KCalendarSystemGregorianProlepticPrivate();

    // Virtual methods each calendar system must re-implement
    virtual void initDefaultEraList();
    virtual int monthsInYear( int year ) const;
    virtual int daysInMonth( int year, int month ) const;
    virtual int daysInYear( int year ) const;
    virtual int daysInWeek() const;
    virtual bool isLeapYear( int year ) const;
    virtual bool hasLeapMonths() const;
    virtual bool hasYearZero() const;
    virtual int maxDaysInWeek() const;
    virtual int maxMonthsInYear() const;
    virtual int earliestValidYear() const;
    virtual int latestValidYear() const;

    bool m_useCommonEra;
};

#endif // KCALENDARSYSTEMGREGORIANPROLEPTICPRIVATE_H
