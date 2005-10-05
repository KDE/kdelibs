/*
   Copyright (c) 2002 Carlos Moro <cfmoro@correo.uniovi.es>
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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// Derived gregorian kde calendar class
// Just a schema.

#include "kcalendarsystemgregorian.h"

#include "kdebug.h"
#include "klocale.h"

#include <qdatetime.h>
#include <qstring.h>

KCalendarSystemGregorian::KCalendarSystemGregorian(const KLocale * locale)
  : KCalendarSystem(locale)
{
}

KCalendarSystemGregorian::~KCalendarSystemGregorian()
{
}

int KCalendarSystemGregorian::year(const QDate& date) const
{
  return date.year();
}

int KCalendarSystemGregorian::monthsInYear( const QDate & date ) const
{
  Q_UNUSED( date )

  return 12;
}

int KCalendarSystemGregorian::weeksInYear(int year) const
{
  QDate temp;
  temp.setYMD(year, 12, 31);

  // If the last day of the year is in the first week, we have to check the
  // week before
  if ( temp.weekNumber() == 1 )
    temp = temp.addDays(-7);

  return temp.weekNumber();
}

int KCalendarSystemGregorian::weekNumber(const QDate& date,
                                         int * yearNum) const
{
  return date.weekNumber(yearNum);
}

QString KCalendarSystemGregorian::monthName(const QDate& date,
                                            bool shortName) const
{
  return monthName(month(date), year(date), shortName);
}

QString KCalendarSystemGregorian::monthNamePossessive(const QDate& date, bool shortName) const
{
  return monthNamePossessive(month(date), year(date), shortName);
}

QString KCalendarSystemGregorian::monthName(int month, int year, bool shortName) const
{
  Q_UNUSED(year);

  if ( shortName )
    switch ( month )
      {
      case 1:
        return locale()->translate("January", "Jan");
      case 2:
        return locale()->translate("February", "Feb");
      case 3:
        return locale()->translate("March", "Mar");
      case 4:
        return locale()->translate("April", "Apr");
      case 5:
        return locale()->translate("May short", "May");
      case 6:
        return locale()->translate("June", "Jun");
      case 7:
        return locale()->translate("July", "Jul");
      case 8:
        return locale()->translate("August", "Aug");
      case 9:
        return locale()->translate("September", "Sep");
      case 10:
        return locale()->translate("October", "Oct");
      case 11:
        return locale()->translate("November", "Nov");
      case 12:
        return locale()->translate("December", "Dec");
      }
  else
    switch ( month )
      {
      case 1:
        return locale()->translate("January");
      case 2:
        return locale()->translate("February");
      case 3:
        return locale()->translate("March");
      case 4:
        return locale()->translate("April");
      case 5:
        return locale()->translate("May long", "May");
      case 6:
        return locale()->translate("June");
      case 7:
        return locale()->translate("July");
      case 8:
        return locale()->translate("August");
      case 9:
        return locale()->translate("September");
      case 10:
        return locale()->translate("October");
      case 11:
        return locale()->translate("November");
      case 12:
        return locale()->translate("December");
      }

  return QString::null;
}

QString KCalendarSystemGregorian::monthNamePossessive(int month, int year,
                                                      bool shortName) const
{
  Q_UNUSED(year);

  if ( shortName )
    switch ( month )
      {
      case 1:
        return locale()->translate("of January", "of Jan");
      case 2:
        return locale()->translate("of February", "of Feb");
      case 3:
        return locale()->translate("of March", "of Mar");
      case 4:
        return locale()->translate("of April", "of Apr");
      case 5:
        return locale()->translate("of May short", "of May");
      case 6:
        return locale()->translate("of June", "of Jun");
      case 7:
        return locale()->translate("of July", "of Jul");
      case 8:
        return locale()->translate("of August", "of Aug");
      case 9:
        return locale()->translate("of September", "of Sep");
      case 10:
        return locale()->translate("of October", "of Oct");
      case 11:
       return locale()->translate("of November", "of Nov");
      case 12:
        return locale()->translate("of December", "of Dec");
      }
  else
    switch ( month )
      {
      case 1:
        return locale()->translate("of January");
      case 2:
        return locale()->translate("of February");
      case 3:
        return locale()->translate("of March");
      case 4:
        return locale()->translate("of April");
      case 5:
        return locale()->translate("of May long", "of May");
      case 6:
        return locale()->translate("of June");
      case 7:
        return locale()->translate("of July");
      case 8:
        return locale()->translate("of August");
      case 9:
        return locale()->translate("of September");
      case 10:
        return locale()->translate("of October");
      case 11:
        return locale()->translate("of November");
      case 12:
        return locale()->translate("of December");
      }

  return QString::null;
}

bool KCalendarSystemGregorian::setYMD(QDate & date, int y, int m, int d) const
{
  // We don't want Qt to add 1900 to them
  if ( y >= 0 && y <= 99 )
    return false;

  // QDate supports gregorian internally
  return date.setYMD(y, m, d);
}

QDate KCalendarSystemGregorian::addYears(const QDate & date, int nyears) const
{
  return date.addYears(nyears);
}

QDate KCalendarSystemGregorian::addMonths(const QDate & date, int nmonths) const
{
  return date.addMonths(nmonths);
}

QDate KCalendarSystemGregorian::addDays(const QDate & date, int ndays) const
{
  return date.addDays(ndays);
}

QString KCalendarSystemGregorian::weekDayName(int col, bool shortName) const
{
  // ### Should this really be different to each calendar system? Or are we
  //     only going to support weeks with 7 days?

  return KCalendarSystem::weekDayName(col, shortName);
}

QString KCalendarSystemGregorian::weekDayName(const QDate& date, bool shortName) const
{
  return weekDayName(dayOfWeek(date), shortName);
}


int KCalendarSystemGregorian::dayOfWeek(const QDate& date) const
{
  return date.dayOfWeek();
}

int KCalendarSystemGregorian::dayOfYear(const QDate & date) const
{
  return date.dayOfYear();
}

int KCalendarSystemGregorian::daysInMonth(const QDate& date) const
{
  return date.daysInMonth();
}

int KCalendarSystemGregorian::minValidYear() const
{
  return 1753; // QDate limit
}

int KCalendarSystemGregorian::maxValidYear() const
{
  return 8000; // QDate limit
}

int KCalendarSystemGregorian::day(const QDate& date) const
{
  return date.day();
}

int KCalendarSystemGregorian::month(const QDate& date) const
{
  return date.month();
}

int KCalendarSystemGregorian::daysInYear(const QDate& date) const
{
  return date.daysInYear();
}

int KCalendarSystemGregorian::weekDayOfPray() const
{
  return 7; // sunday
}

QString KCalendarSystemGregorian::calendarName() const
{
  return QLatin1String("gregorian");
}

bool KCalendarSystemGregorian::isLunar() const
{
  return false;
}

bool KCalendarSystemGregorian::isLunisolar() const
{
  return false;
}

bool KCalendarSystemGregorian::isSolar() const
{
  return true;
}

int KCalendarSystemGregorian::yearStringToInteger(const QString & sNum, int & iLength) const
{
  int iYear;
  iYear = KCalendarSystem::yearStringToInteger(sNum, iLength);
  
  // Qt treats a year in the range 0-100 as 1900-1999.
  // It is nicer for the user if we treat 0-68 as 2000-2068
  if (iYear < 69)
    iYear += 2000;
  else if (iYear < 100)
    iYear += 1900;

  return iYear;
}
