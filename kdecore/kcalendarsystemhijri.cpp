/*
   Copyright (c) 2002-2003 Carlos Moro <cfmoro@correo.uniovi.es>
   Copyright (c) 2002-2003 Hans Petter Bieker <bieker@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// Derived hijri kde calendar class

#include "kcalendarsystemhijri.h"

#include "kdebug.h"
#include "klocale.h"

#include <qdatetime.h>
#include <qstring.h>

/*
  The following C++ code is translated from the Lisp code
  in ``Calendrical Calculations'' by Nachum Dershowitz and
  Edward M. Reingold, Software---Practice & Experience,
  vol. 20, no. 9 (September, 1990), pp. 899--928.

  This code is in the public domain, but any use of it
  should publically acknowledge its source.

  Classes GregorianDate, IslamicDate
 */

static int lastDayOfGregorianMonth(int month, int year) {
// Compute the last date of the month for the Gregorian calendar.

  switch (month) {
  case 2:
    if ((((year % 4) == 0) && ((year % 100) != 0))
        || ((year % 400) == 0))
      return 29;
    else
      return 28;
  case 4:
  case 6:
  case 9:
  case 11: return 30;
  default: return 31;
  }
}

class GregorianDate {
private:
  int year;   // 1...
  int month;  // 1 == January, ..., 12 == December
  int day;    // 1..lastDayOfGregorianMonth(month, year)

public:
  GregorianDate(int m, int d, int y) { month = m; day = d; year = y; }

  GregorianDate(int d) { // Computes the Gregorian date from the absolute date.

    // Search forward year by year from approximate year
    year = d/366;
    while (d >= GregorianDate(1,1,year+1))
      year++;
    // Search forward month by month from January
    month = 1;
    while (d > GregorianDate(month, lastDayOfGregorianMonth(month,year), year))
      month++;
    day = d - GregorianDate(month,1,year) + 1;
  }

  operator int() { // Computes the absolute date from the Gregorian date.
    int N = day;           // days this month
    for (int m = month - 1;  m > 0; m--) // days in prior months this year
      N = N + lastDayOfGregorianMonth(m, year);
    return
      (N                    // days this year
       + 365 * (year - 1)   // days in previous years ignoring leap days
       + (year - 1)/4       // Julian leap days before this year...
       - (year - 1)/100     // ...minus prior century years...
       + (year - 1)/400);   // ...plus prior years divisible by 400
  }

  int getMonth() { return month; }
  int getDay() { return day; }
  int getYear() { return year; }

};

static int IslamicLeapYear(int year) {
// True if year is an Islamic leap year

  if ((((11 * year) + 14) % 30) < 11)
    return 1;
  else
    return 0;
}

static const int IslamicEpoch = 227014; // Absolute date of start of
                                        // Islamic calendar

static int lastDayOfIslamicMonth(int month, int year) {
// Last day in month during year on the Islamic calendar.

  if (((month % 2) == 1) || ((month == 12) && IslamicLeapYear(year)))
    return 30;
  else
    return 29;
}

class IslamicDate {
private:
  int year;   // 1...
  int month;  // 1..13 (12 in a common year)
  int day;    // 1..lastDayOfIslamicMonth(month,year)

public:
  IslamicDate(int m, int d, int y) { month = m; day = d; year = y; }

  IslamicDate(int d) { // Computes the Islamic date from the absolute date.
    if (d <= IslamicEpoch) { // Date is pre-Islamic
      month = 0;
      day = 0;
      year = 0;
    }
    else {
      // Search forward year by year from approximate year
      year = (d - IslamicEpoch) / 355;
      while (d >= IslamicDate(1,1,year+1))
        year++;
      // Search forward month by month from Muharram
      month = 1;
      while (d > IslamicDate(month, lastDayOfIslamicMonth(month,year), year))
        month++;
      day = d - IslamicDate(month,1,year) + 1;
    }
  }

  operator int() { // Computes the absolute date from the Islamic date.
    return (day                      // days so far this month
            + 29 * (month - 1)       // days so far...
            + month/2                //            ...this year
            + 354 * (year - 1)       // non-leap days in prior years
            + (3 + (11 * year)) / 30 // leap days in prior years
            + IslamicEpoch);                // days before start of calendar
  }

  int getMonth() { return month; }
  int getDay() { return day; }
  int getYear() { return year; }

};

static void gregorianToHijri(const QDate & date, int * pYear, int * pMonth,
   int * pDay)
{
  GregorianDate gregorian(date.month(),date.day(),date.year());
  int absolute = gregorian;

  IslamicDate islamic(absolute);

  if (pYear)
    *pYear = islamic.getYear();
  if (pMonth)
    *pMonth = islamic.getMonth();
  if (pDay)
    *pDay = islamic.getDay();
}

KCalendarSystemHijri::KCalendarSystemHijri(const KLocale * locale)
  : KCalendarSystem(locale)
{
}

KCalendarSystemHijri::~KCalendarSystemHijri()
{
}

int KCalendarSystemHijri::year(const QDate& date) const
{
  int y;
  gregorianToHijri(date, &y, 0, 0);
  return y;
}

int KCalendarSystemHijri::month(const QDate& date) const
{
  int m;
  gregorianToHijri(date, 0, &m, 0);
  return m;
}

int KCalendarSystemHijri::day(const QDate& date) const
{
  int d;
  gregorianToHijri(date, 0, 0, &d);
  return d;
}

int KCalendarSystemHijri::monthsInYear( const QDate & date ) const
{
  Q_UNUSED( date )

  return 12;
}

int KCalendarSystemHijri::weeksInYear(int year) const
{
  QDate temp;
  setYMD(temp, year, 12, lastDayOfIslamicMonth(12, year));

  // If the last day of the year is in the first week, we have to check the
  // week before
  if ( weekNumber(temp) == 1 )
    temp = addDays(temp, -7);

  return weekNumber(temp);
}

int KCalendarSystemHijri::weekNumber(const QDate& date, int * yearNum) const
{
  QDate firstDayWeek1, lastDayOfYear;
  int y = year(date);
  int week;
  int weekDay1, dayOfWeek1InYear;

  // let's guess 1st day of 1st week
  setYMD(firstDayWeek1, y, 1, 1);
  weekDay1 = dayOfWeek(firstDayWeek1);

  // iso 8601: week 1  is the first containing thursday and week starts on
  // monday
  if (weekDay1 > 4 )
    firstDayWeek1 = addDays(firstDayWeek1 , 7 - weekDay1 + 1); // next monday

  dayOfWeek1InYear = dayOfYear(firstDayWeek1);

  if ( dayOfYear(date) < dayOfWeek1InYear ) // our date in prev year's week
  {
    if ( yearNum )
      *yearNum = y - 1;
    return weeksInYear(y - 1);
  }

  // let' check if its last week belongs to next year
  setYMD(lastDayOfYear, y, 12, lastDayOfIslamicMonth(12, y));
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
    if ( weekDay1 < 5 )
      firstDayWeek1 = addDays(firstDayWeek1, - (weekDay1 - 1));

    week = firstDayWeek1.daysTo(date) / 7 + 1;
  }

  return week;
}

QString KCalendarSystemHijri::monthName(const QDate& date,
                                        bool shortName) const
{
  return monthName(month(date), year(date), shortName);
}

QString KCalendarSystemHijri::monthNamePossessive(const QDate& date,
                                                  bool shortName) const
{
  return monthNamePossessive(month(date), year(date), shortName);
}

QString KCalendarSystemHijri::monthName(int month, int year, bool shortName)
  const {

  Q_UNUSED(year);

  if (shortName)
    switch ( month )
      {
      case 1:
        return locale()->translate("Muharram");
      case 2:
        return locale()->translate("Safar");
      case 3:
        return locale()->translate("R. Awal");
      case 4:
        return locale()->translate("R. Thaani");
      case 5:
        return locale()->translate("J. Awal");
      case 6:
        return locale()->translate("J. Thaani");
      case 7:
        return locale()->translate("Rajab");
      case 8:
        return locale()->translate("Sha`ban");
      case 9:
        return locale()->translate("Ramadan");
      case 10:
        return locale()->translate("Shawwal");
      case 11:
        return locale()->translate("Qi`dah");
      case 12:
        return locale()->translate("Hijjah");
    }
  else
    switch ( month )
      {
      case 1:
        return locale()->translate("Muharram");
      case 2:
        return locale()->translate("Safar");
      case 3:
        return locale()->translate("Rabi` al-Awal");
      case 4:
        return locale()->translate("Rabi` al-Thaani");
      case 5:
        return locale()->translate("Jumaada al-Awal");
      case 6:
        return locale()->translate("Jumaada al-Thaani");
      case 7:
        return locale()->translate("Rajab");
      case 8:
        return locale()->translate("Sha`ban");
      case 9:
        return locale()->translate("Ramadan");
      case 10:
        return locale()->translate("Shawwal");
      case 11:
        return locale()->translate("Thu al-Qi`dah");
      case 12:
        return locale()->translate("Thu al-Hijjah");
      }

  return QString::null;
}

QString KCalendarSystemHijri::monthNamePossessive(int month, int year,
                                                  bool shortName) const
{
  Q_UNUSED(year);

  if (shortName)
    switch ( month )
      {
      case 1:
        return locale()->translate("of Muharram");
      case 2:
        return locale()->translate("of Safar");
      case 3:
        return locale()->translate("of R. Awal");
      case 4:
        return locale()->translate("of R. Thaani");
      case 5:
        return locale()->translate("of J. Awal");
      case 6:
        return locale()->translate("of J. Thaani");
      case 7:
        return locale()->translate("of Rajab");
      case 8:
        return locale()->translate("of Sha`ban");
      case 9:
        return locale()->translate("of Ramadan");
      case 10:
        return locale()->translate("of Shawwal");
      case 11:
        return locale()->translate("of Qi`dah");
      case 12:
        return locale()->translate("of Hijjah");
    }
  else
    switch ( month )
      {
      case 1:
        return locale()->translate("of Muharram");
      case 2:
        return locale()->translate("of Safar");
      case 3:
        return locale()->translate("of Rabi` al-Awal");
      case 4:
        return locale()->translate("of Rabi` al-Thaani");
      case 5:
        return locale()->translate("of Jumaada al-Awal");
      case 6:
        return locale()->translate("of Jumaada al-Thaani");
      case 7:
        return locale()->translate("of Rajab");
      case 8:
        return locale()->translate("of Sha`ban");
      case 9:
        return locale()->translate("of Ramadan");
      case 10:
        return locale()->translate("of Shawwal");
      case 11:
        return locale()->translate("of Thu al-Qi`dah");
      case 12:
        return locale()->translate("of Thu al-Hijjah");
      }

  return QString::null;
}

bool KCalendarSystemHijri::setYMD(QDate & date, int y, int m, int d) const
{
  // range checks
  if ( y < minValidYear() || y > maxValidYear() )
    return false;

  if ( m < 1 || m > 12 )
    return false;

  if ( d < 1 || d > lastDayOfIslamicMonth(m, y) )
    return false;

  IslamicDate islamic (m, d, y);
  int absolute = islamic;
  GregorianDate gregorian(absolute);

  return date.setYMD(gregorian.getYear(), gregorian.getMonth(),
    gregorian.getDay());
}

QString KCalendarSystemHijri::weekDayName(int day, bool shortName) const
{
  if ( shortName )
    switch (day)
      {
      case 1:
        return locale()->translate("Ith");
      case 2:
        return locale()->translate("Thl");
      case 3:
        return locale()->translate("Arb");
      case 4:
        return locale()->translate("Kha");
      case 5:
        return locale()->translate("Jum");
      case 6:
        return locale()->translate("Sab");
      case 7:
        return locale()->translate("Ahd");
      }
  else
    switch ( day )
      {
      case 1:
        return locale()->translate("Yaum al-Ithnain");
      case 2:
        return locale()->translate("Yau al-Thulatha");
      case 3:
        return locale()->translate("Yaum al-Arbi'a");
      case 4:
        return locale()->translate("Yaum al-Khamees");
      case 5:
        return locale()->translate("Yaum al-Jumma");
      case 6:
        return locale()->translate("Yaum al-Sabt");
      case 7:
        return locale()->translate("Yaum al-Ahad");
      }

  return QString::null;
}

QString KCalendarSystemHijri::weekDayName(const QDate& date,
                                          bool shortName) const
{
  return weekDayName(dayOfWeek(date), shortName);
}

int KCalendarSystemHijri::dayOfWeek(const QDate& date) const
{
  return date.dayOfWeek(); // same as gregorian
}

int KCalendarSystemHijri::dayOfYear(const QDate & date) const
{
  QDate first;
  setYMD(first, year(date), 1, 1);

  return first.daysTo(date) + 1;

  return 100;
}

int KCalendarSystemHijri::daysInMonth(const QDate& date) const
{
  int y, m;
  gregorianToHijri(date, &y, &m, 0);

  return lastDayOfIslamicMonth(m, y);
}

// Min valid year that may be converted to QDate
int KCalendarSystemHijri::minValidYear() const
{
  QDate date(1753, 1, 1);

  return year(date);
}

// Max valid year that may be converted to QDate
int KCalendarSystemHijri::maxValidYear() const
{
  QDate date(8000, 1, 1);

  return year(date);
}

int KCalendarSystemHijri::daysInYear(const QDate & date) const
{
  QDate first, last;
  setYMD(first, year(date), 1, 1);
  setYMD(last, year(date) + 1, 1, 1);

  return first.daysTo(last);
}

int KCalendarSystemHijri::weekDayOfPray() const
{
  return 5; // friday
}

QDate KCalendarSystemHijri::addDays( const QDate & date, int ndays ) const
{
  return date.addDays( ndays );
}

QDate KCalendarSystemHijri::addMonths( const QDate & date, int nmonths ) const
{
  QDate result = date;
  int m = month(date);
  int y = year(date);

  if ( nmonths < 0 )
  {
    m += 12;
    y -= 1;
  }

  --m; // this only works if we start counting at zero
  m += nmonths;
  y += m / 12;
  m %= 12;
  ++m;

  setYMD( result, y, m, day(date) );

  return result;
}

QDate KCalendarSystemHijri::addYears( const QDate & date, int nyears ) const
{
  QDate result = date;
  int y = year(date) + nyears;

  setYMD( result, y, month(date), day(date) );

  return result;
}

QString KCalendarSystemHijri::calendarName() const
{
  return QLatin1String("hijri");
}

bool KCalendarSystemHijri::isLunar() const
{
  return true;
}

bool KCalendarSystemHijri::isLunisolar() const
{
  return false;
}

bool KCalendarSystemHijri::isSolar() const
{
  return false;
}
