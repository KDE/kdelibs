/*
   Copyright (c) 2003 Hans Petter Bieker <bieker@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// Derived hebrew kde calendar class

#include <qdatetime.h>
#include <qstring.h>

#include <klocale.h>
#include <kdebug.h>

#include "kcalendarsystemhebrew.h"

// ### HPB Fix this class

#if 0
KCalendarSystemHebrew::KCalendarSystemHebrew(const KLocale * locale)
  : KCalendarSystem(locale)
{
}

KCalendarSystemHebrew::~KCalendarSystemHebrew()
{
}

static SDATE * toHebrew(const QDate & date)
{
  SDATE *sd;
  sd = hdate(date.year(), date.month(), date.day());
  return sd;
}

int KCalendarSystemHebrew::year(const QDate& date) const
{
  SDATE *sd = toHebrew(date);
  return sd->year;
}

int KCalendarSystemHebrew::monthsInYear( const QDate & date ) const
{
  Q_UNUSED( date )

  kdDebug(5400) << "Arabic monthsInYear" << endl;
  return 12;
}

int KCalendarSystemHebrew::weeksInYear(int year) const
{
  QDate temp;
  setYMD(temp, year, 12, hndays(12, year));

  // If the last day of the year is in the first week, we have to check the
  // week before
  if ( weekNumber(temp) == 1 )
    temp = addDays(temp, -7);

  return weekNumber(temp);
}

int KCalendarSystemHebrew::weekNumber(const QDate& date, int * yearNum) const
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
  if (weekDay1 > 4 /*Thursday*/)
    firstDayWeek1 = addDays(firstDayWeek1 , 7 - weekDay1 + 1); // next monday

  dayOfWeek1InYear = dayOfYear(firstDayWeek1);

  if ( dayOfYear(date) < dayOfWeek1InYear ) // our date in prev year's week
  {
    if ( yearNum )
      *yearNum = y - 1;
    return weeksInYear(y - 1);
  }

  // let' check if its last week belongs to next year
  setYMD(lastDayOfYear, y, 12, hndays(12, y));
  if ( (dayOfYear(date) >= daysInYear(date) - dayOfWeek(lastDayOfYear) + 1)
       // our date is in last week
       && dayOfWeek(lastDayOfYear) < 4) // 1st week in next year has thursday
    {
      if ( yearNum )
        *yearNum = y + 1;
      week = 1;
    }
  else
    week = firstDayWeek1.daysTo(date) / 7 + 1;

  return week;
}

QString KCalendarSystemHebrew::monthName(const QDate& date,
                                        bool shortName) const
{
  return monthName(month(date), shortName);
}

QString KCalendarSystemHebrew::monthNamePossessive(const QDate& date,
                                                  bool shortName) const
{
  return monthNamePossessive(month(date), shortName);
}

QString KCalendarSystemHebrew::monthName(int month, bool shortName) const
{
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

QString KCalendarSystemHebrew::monthNamePossessive(int month,
                                                  bool shortName) const
{
  kdDebug(5400) << "Arabic getMonthName " << hmname[month - 1] << endl;

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

bool KCalendarSystemHebrew::setYMD(QDate & date, int y, int m, int d) const
{
  // range checks
  if ( y < minValidYear() || y > maxValidYear() )
    return false;

  if ( m < 1 || m > 12 )
    return false;

  if ( d < 1 || d > hndays(m, y) )
    return false;

  SDATE * gd = gdate( y, m, d );

  return date.setYMD(gd->year, gd->mon, gd->day);
}

QString KCalendarSystemHebrew::weekDayName(int day, bool shortName) const
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

QString KCalendarSystemHebrew::weekDayName(const QDate& date,
                                          bool shortName) const
{
  return weekDayName(dayOfWeek(date), shortName);
}

int KCalendarSystemHebrew::dayOfWeek(const QDate& date) const
{
  SDATE *sd = toHebrew(date);
  if ( sd->dw == 0 )
    return 7;
  else
    return (sd->dw);
}

int KCalendarSystemHebrew::dayOfYear(const QDate & date) const
{
  QDate first;
  setYMD(first, year(date), 1, 1);

  return first.daysTo(date) + 1;
}

int KCalendarSystemHebrew::daysInMonth(const QDate& date) const
{
  SDATE *sd = toHebrew(date);
  return hndays(sd->mon, sd->year);
}

int KCalendarSystemHebrew::hndays(int mon, int year) const
{
  SDATE fd, ld;
  int nd = 666;
  fd = *gdate(year, mon, 1);
  ld = *gdate(year, mon + 1, 1);
  ld = *caldate(julianday(ld.year, ld.mon, ld.day, 0.0) - 1.0);
  if (fd.mon == ld.mon)
    nd = ld.day - fd.day + 1;
  else
    nd = ndays(fd.mon, fd.year) - fd.day + ld.day + 1;

  return nd;
}

// Min valid year that may be converted to QDate
int KCalendarSystemHebrew::minValidYear() const
{
  QDate date(1753, 1, 1);

  return year(date);
}

// Max valid year that may be converted to QDate
int KCalendarSystemHebrew::maxValidYear() const
{
  QDate date(8000, 1, 1);

  SDATE *sd = toHebrew(date);

  return sd->year;
}

int KCalendarSystemHebrew::day(const QDate& date) const
{
  SDATE *sd = toHebrew(date);

  return sd->day;
}

int KCalendarSystemHebrew::month(const QDate& date) const
{
  SDATE *sd = toHebrew(date);

  return sd->mon;
}

int KCalendarSystemHebrew::daysInYear(const QDate & date) const
{
  QDate first, last;
  setYMD(first, year(date), 1, 1);
  setYMD(last, year(date) + 1, 1, 1);

  return first.daysTo(last);
}

int KCalendarSystemHebrew::weekDayOfPray() const
{
  return 5; // friday
}

QDate KCalendarSystemHebrew::addDays( const QDate & date, int ndays ) const
{
  return date.addDays( ndays );
}

QDate KCalendarSystemHebrew::addMonths( const QDate & date, int nmonths ) const
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

QDate KCalendarSystemHebrew::addYears( const QDate & date, int nyears ) const
{
  QDate result = date;
  int y = year(date) + nyears;

  setYMD( result, y, month(date), day(date) );

  return result;
}
#endif

QString KCalendarSystemHebrew::calendarName() const
{
  return QString::fromLatin1("hebrew");
}

bool KCalendarSystemHebrew::isLunar() const
{
  return false;
}

bool KCalendarSystemHebrew::isLunisolar() const
{
  return true;
}

bool KCalendarSystemHebrew::isSolar() const
{
  return false;
}
