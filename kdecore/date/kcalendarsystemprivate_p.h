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

#ifndef KCALENDARSYSTEMPRIVATE_H
#define KCALENDARSYSTEMPRIVATE_H

class KCalendarSystem;
class KLocale;

class KCalendarSystemPrivate
{
public:
    explicit KCalendarSystemPrivate( KCalendarSystem *q );

    virtual ~KCalendarSystemPrivate();

    // Virtual methods each calendar system must re-implement
    virtual int monthsInYear( int year ) const;
    virtual int daysInMonth( int year, int month ) const;
    virtual int daysInYear( int year ) const;
    virtual int daysInWeek() const;
    virtual bool isLeapYear( int year ) const;
    virtual bool hasYearZero() const;
    virtual int maxDaysInWeek() const;
    virtual int maxMonthsInYear() const;
    virtual int earliestValidYear() const;
    virtual int latestValidYear() const;

    // Utility functions
    bool setAnyDate( QDate &date, int year, int month, int day ) const;
    int addYearNumber( int originalYear, int addYears ) const;
    QDate invalidDate() const;
    int stringToInteger( const QString &sNum, int &iLength ) const;
    QString simpleDateString( const QString &str ) const;
    QDate firstDayOfYear( int year ) const;
    QDate lastDayOfYear( int year ) const;

    // Global variables each calendar system must initialise
    const KCalendarSystem *q;
    const KLocale *locale;
};

#endif // KCALENDARSYSTEMPRIVATE_H
