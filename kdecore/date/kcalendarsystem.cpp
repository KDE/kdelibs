/*
   Copyright (c) 2002 Carlos Moro <cfmoro@correo.uniovi.es>
   Copyright (c) 2002 Hans Petter Bieker <bieker@kde.org>
   Copyright (c) 2007 John Layt <john@layt.net>

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

// Gregorian calendar system implementation factory for creation of kde calendar
// systems.
// Also default gregorian and factory classes

#include "kcalendarsystem.h"

#include "kglobal.h"

#include <QtCore/QDateTime>


#include "kcalendarsystemgregorian.h"
#include "kcalendarsystemhijri.h"
#include "kcalendarsystemhebrew.h"
#include "kcalendarsystemjalali.h"

KCalendarSystem *KCalendarSystem::create( const QString &calType,
                                          const KLocale * locale )
{
  if ( calType == "hebrew" )
    return new KCalendarSystemHebrew(locale);
  if ( calType == "hijri" )
    return new KCalendarSystemHijri(locale);
  if ( calType == "gregorian" )
    return new KCalendarSystemGregorian(locale);
  if ( calType == "jalali" )
    return new KCalendarSystemJalali(locale);

  //kDebug(5400) << "Calendar " << calType << " not found, defaulting to gregorian" << endl;

  // ### HPB: Should it really be a default here?
  return new KCalendarSystemGregorian(locale);
}

QStringList KCalendarSystem::calendarSystems()
{
   QStringList lst;
   lst.append("hebrew");
   lst.append("hijri");
   lst.append("gregorian");
   lst.append("jalali");

   return lst;
}

QString KCalendarSystem::calendarLabel( const QString &calendarType )
{
    if ( calendarType == "gregorian" )
      return ki18nc("@item Calendar system", "Gregorian").toString(KGlobal::locale());

    if ( calendarType == "hebrew" )
      return ki18nc("@item Calendar system", "Hebrew").toString(KGlobal::locale());

    if ( calendarType == "hijri" )
      return ki18nc("@item Calendar system", "Hijri").toString(KGlobal::locale());

    if ( calendarType == "jalali" )
      return ki18nc("@item Calendar system", "Jalali").toString(KGlobal::locale());

    return ki18nc("@item Calendar system", "Invalid Calendar Type").toString(KGlobal::locale());
}


class KCalendarSystemPrivate
{
public:
  const KLocale * locale;
};

KCalendarSystem::KCalendarSystem(const KLocale * locale)
  : d(new KCalendarSystemPrivate)
{
  d->locale = locale;
}

KCalendarSystem::~KCalendarSystem()
{
  delete d;
}

const KLocale * KCalendarSystem::locale() const
{
  if ( d->locale )
    return d->locale;

  return KGlobal::locale();
}

QString KCalendarSystem::dayString( const QDate &pDate, StringFormat format ) const
{
    QString sResult;

    sResult.setNum( day( pDate ) );
    if ( format == LongFormat && sResult.length() == 1 ) {
        sResult.prepend( QLatin1Char('0') );
    }

    return sResult;
}

QString KCalendarSystem::monthString( const QDate &pDate, StringFormat format ) const
{
    QString sResult;

    sResult.setNum( month( pDate ) );
    if ( format == LongFormat && sResult.length() == 1 ) {
        sResult.prepend( QLatin1Char('0') );
    }

  return sResult;
}

QString KCalendarSystem::yearString( const QDate &pDate, StringFormat format ) const
{
    QString sResult;

    sResult.setNum(year(pDate));
    if ( format == ShortFormat && sResult.length() == 4 ) {
        sResult = sResult.right(2);
    }

    return sResult;
}

static int stringToInteger(const QString & sNum, int & iLength)
{
  int iPos = 0;

  int result = 0;
  for (; sNum.length() > iPos && sNum.at(iPos).isDigit(); iPos++)
    {
      result *= 10;
      result += sNum.at(iPos).digitValue();
    }

  iLength = iPos;
  return result;
}


int KCalendarSystem::dayStringToInteger(const QString & sNum, int & iLength) const
{
  return stringToInteger(sNum, iLength);
}

int KCalendarSystem::monthStringToInteger(const QString & sNum, int & iLength) const
{
  return stringToInteger(sNum, iLength);
}

int KCalendarSystem::yearStringToInteger(const QString & sNum, int & iLength) const
{
  return stringToInteger(sNum, iLength);
}

QDate KCalendarSystem::epoch() const
{
    return QDate::fromJulianDay(1);
}

QDate KCalendarSystem::earliestValidDate() const
{
    return epoch();
}

QDate KCalendarSystem::latestValidDate() const
{
    // Default to Gregorian 9999-12-31
    return QDate::fromJulianDay( 5373484 );
}

bool KCalendarSystem::isValid(const QDate &date) const
{
    return isValid( year( date ), month( date ), day( date ) );
}

bool KCalendarSystem::setDate(QDate &date, int year, int month, int day) const
{
    if ( isValid( year, month, day ) ) {
        int jd;
        dateToJulianDay( year, month, day, jd );
        date = QDate::fromJulianDay(jd);
        return true;
    }
    return false;
}

// Deprecated
bool KCalendarSystem::setYMD(QDate & date, int y, int m, int d) const
{
    return setDate( date, y, m, d );
}

int KCalendarSystem::year(const QDate &date) const
{
    int year, month, day;
    julianDayToDate( date.toJulianDay(), year, month, day );
    return year;
}

int KCalendarSystem::month(const QDate &date) const
{
    int year, month, day;
    julianDayToDate( date.toJulianDay(), year, month, day );
    return month;
}

int KCalendarSystem::day(const QDate &date) const
{
    int year, month, day;
    julianDayToDate( date.toJulianDay(), year, month, day );
    return day;
}

int KCalendarSystem::weekNumber(const QDate& date, int * yearNum) const
{
  // Copied straight from Hebrew/Jalali/Hirji version
  QDate firstDayWeek1, lastDayOfYear;
  int y = year(date);
  int week;
  int weekDay1, dayOfWeek1InYear;

  // let's guess 1st day of 1st week
  setYMD(firstDayWeek1, y, 1, 1);
  weekDay1 = dayOfWeek(firstDayWeek1);

  // iso 8601: week 1  is the first containing thursday and week starts on
  // monday
  if (weekDay1 > 4 /*Thursday*/)
    firstDayWeek1 = addDays(firstDayWeek1 , 7 - weekDay1 + 1); // next monday

  dayOfWeek1InYear = dayOfYear(firstDayWeek1);

  if ( dayOfYear(date) < dayOfWeek1InYear ) // our date in prev year's week
  {
    if ( yearNum )
      *yearNum = y - 1;
    return weeksInYear(y - 1);
  }

  // let's check if its last week belongs to next year
  setYMD(lastDayOfYear, y + 1, 1, 1);
  lastDayOfYear = addDays(lastDayOfYear, -1);
  if ( (dayOfYear(date) >= daysInYear(date) - dayOfWeek(lastDayOfYear) + 1)
       // our date is in last week
       && dayOfWeek(lastDayOfYear) < 4) // 1st week in next year has thursday
    {
      if ( yearNum )
        *yearNum = y + 1;
      week = 1;
    }
  else
  {
   if( weekDay1 < 5 ) // To calculate properly the number of weeks
                     //  from day a to x let's make a day 1 of week
      firstDayWeek1 = addDays( firstDayWeek1, -( weekDay1 - 1));

   week = firstDayWeek1.daysTo(date) / 7 + 1;
  }

  return week;
}

int KCalendarSystem::dayOfYear(const QDate &date) const
{
    //Take the jd of the given date, and subtract the jd of the first day of that year
    int firstDayOfYear;
    dateToJulianDay( year(date), 1, 1, firstDayOfYear );
    return ( date.toJulianDay() - firstDayOfYear + 1 );
}

int KCalendarSystem::dayOfWeek(const QDate &date) const
{
    // Makes assumption that Julian Day 0 was day 1 of week
    // This is true for Julian/Gregorian calendar with jd 0 being Monday
    // We add 1 for ISO compliant numbering for 7 day week
    // Assumes we've never skipped weekdays
    return ( ( date.toJulianDay() % daysInWeek(date) ) + 1 );
}

QDate KCalendarSystem::addYears(const QDate &date, int nyears) const
{
    int originalYear, originalMonth, originalDay;
    int newYear, newMonth, newDay;
    QDate firstOfNewMonth, newDate;

    julianDayToDate( date.toJulianDay(), originalYear, originalMonth, originalDay );

    newYear = originalYear + nyears;
    newMonth = originalMonth;

    //Adjust day number if new month has fewer days than old month
    if ( setDate( firstOfNewMonth, newYear, newMonth, 1 ) ) {
        int daysInNewMonth = daysInMonth( firstOfNewMonth );
        newDay = ( daysInNewMonth < originalDay ) ? daysInNewMonth : originalDay;

        if ( setDate( newDate, newYear, newMonth, newDay ) ) {
            return newDate;
        }
    }

    return QDate::fromJulianDay(0); //Is QDate's way of saying is invalid
}

QDate KCalendarSystem::addMonths(const QDate &date, int nmonths) const
{
    int originalYear, originalMonth, originalDay;
    int newYear, newMonth, newDay;
    int monthsInOriginalYear, daysInNewMonth;
    QDate firstOfNewMonth, newDate;

    julianDayToDate( date.toJulianDay(), originalYear, originalMonth, originalDay );

    monthsInOriginalYear = monthsInYear(date);

    newYear = originalYear + ((originalMonth + nmonths) / monthsInOriginalYear);
    newMonth = (originalMonth + nmonths) % monthsInOriginalYear;

    if ( newMonth == 0 ) {
      newYear = newYear - 1;
      newMonth = monthsInOriginalYear;
    }
    if ( newMonth < 0 ) {
      newYear = newYear - 1;
      newMonth = newMonth + monthsInOriginalYear;
    }

    //Adjust day number if new month has fewer days than old month
    if ( setDate( firstOfNewMonth, newYear, newMonth, 1 ) ) {
        daysInNewMonth = daysInMonth( firstOfNewMonth );
        newDay = ( daysInNewMonth < originalDay ) ? daysInNewMonth : originalDay;

        if ( setDate( newDate, newYear, newMonth, newDay ) ) {
            return newDate;
        }
    }

    return QDate::fromJulianDay(0); //Is QDate's way of saying is invalid
}

QDate KCalendarSystem::addDays(const QDate &date, int ndays) const
{
    if ( date.toJulianDay() + ndays > 0 ) {  // QDate only holds a uint, don't overflow!
        QDate temp = date.addDays( ndays );  // QDate adds straight to jd
        if ( isValid( temp ) ) {
            return temp;
        }
    }
    return QDate::fromJulianDay(0); //Is QDate's way of saying is invalid
}

bool KCalendarSystem::isLeapYear(const QDate &date) const
{
    return isLeapYear( year( date ) );
}

int KCalendarSystem::monthsInYear(const QDate &date) const
{
    // last day of this year = first day of next year minus 1 day
    QDate firstDayOfNextYear;
    setDate( firstDayOfNextYear, year( date ) + 1, 1, 1 );
    QDate lastDayOfThisYear = addDays( firstDayOfNextYear, -1 );
    return month( lastDayOfThisYear );
}

int KCalendarSystem::weeksInYear(const QDate &date) const
{
    return weeksInYear( year( date ) );
}

int KCalendarSystem::weeksInYear(int year) const
{
    // ISO compliant week numbering, not traditional number

    // last day of this year = first day of next year minus 1 day
    QDate firstDayOfNextYear;
    setDate( firstDayOfNextYear, year + 1, 1, 1 );
    QDate lastDayOfThisYear = addDays( firstDayOfNextYear, -1 );

    int lastWeekInThisYear = weekNumber( lastDayOfThisYear );

    // If the last day of the year is in the first week of next year use the week before
    if ( lastWeekInThisYear == 1 ) {
      lastDayOfThisYear = lastDayOfThisYear.addDays(-7);
      lastWeekInThisYear = weekNumber( lastDayOfThisYear );
    }

    return lastWeekInThisYear;
}

int KCalendarSystem::daysInYear (const QDate & date) const
{
    QDate firstDayOfThisYear, firstDayOfNextYear;
    setDate( firstDayOfThisYear, year( date ), 1, 1 );
    setDate( firstDayOfNextYear, year( date ) + 1, 1, 1 );
    return ( firstDayOfNextYear.toJulianDay() - firstDayOfNextYear.toJulianDay() );
}

int KCalendarSystem::daysInMonth(const QDate &date) const
{
    //get the jd of the first day of the next month and subtract the jd of the first date of current month
    QDate firstDayOfThisMonth, firstDayOfNextMonth;

    int thisYear = year(date);
    int thisMonth = month(date);

    setDate( firstDayOfThisMonth, thisYear, thisMonth, 1 );

    //check if next month falls in next year
    if ( thisMonth < monthsInYear( date ) ) {
        setDate( firstDayOfNextMonth, thisYear, thisMonth, 1 );
    } else {
        setDate( firstDayOfNextMonth, thisYear + 1, 1, 1 );
    }

    return ( firstDayOfNextMonth.toJulianDay() - firstDayOfThisMonth.toJulianDay() );
}

int KCalendarSystem::daysInWeek(const QDate &date) const
{
    Q_UNUSED(date);
    return 7;
}

int KCalendarSystem::weekStartDay() const
{
    return locale()->weekStartDay();
}

QString KCalendarSystem::monthName( const QDate& date, MonthNameFormat format ) const
{
    return monthName( month(date), year(date), format );
}

QString KCalendarSystem::weekDayName( const QDate& date, WeekDayNameFormat format ) const
{
    return weekDayName( dayOfWeek(date), format );
}

QString KCalendarSystem::formatDate(const QDate &date, KLocale::DateFormat format) const
{
    return locale()->formatDate( date, format );
}

QDate KCalendarSystem::readDate(const QString &str, bool* ok) const
{
    return locale()->readDate( str, ok );
}

QDate KCalendarSystem::readDate( const QString &intstr, const QString &fmt, bool* ok) const
{
    return locale()->readDate( intstr, fmt, ok );
}

QDate KCalendarSystem::readDate(const QString &str, KLocale::ReadDateFlags flags, bool *ok) const
{
    return locale()->readDate( str, flags, ok );
}

bool KCalendarSystem::julianDayToDate(int jd, int &year, int &month, int &day) const
{
    QDate date = QDate::fromJulianDay( jd );
    if ( date.isValid() ) {
        year = date.year();
        month = date.month();
        day = date.day();
    }
    return date.isValid();
}

bool KCalendarSystem::dateToJulianDay(int year, int month, int day, int &jd) const
{
    QDate date;
    if ( date.setDate( year, month, day ) ) {
        jd = date.toJulianDay();
        return true;
    }
    return false;
}
