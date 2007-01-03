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
        return ki18nc("January", "Jan").toString(locale());
      case 2:
        return ki18nc("February", "Feb").toString(locale());
      case 3:
        return ki18nc("March", "Mar").toString(locale());
      case 4:
        return ki18nc("April", "Apr").toString(locale());
      case 5:
        return ki18nc("May short", "May").toString(locale());
      case 6:
        return ki18nc("June", "Jun").toString(locale());
      case 7:
        return ki18nc("July", "Jul").toString(locale());
      case 8:
        return ki18nc("August", "Aug").toString(locale());
      case 9:
        return ki18nc("September", "Sep").toString(locale());
      case 10:
        return ki18nc("October", "Oct").toString(locale());
      case 11:
        return ki18nc("November", "Nov").toString(locale());
      case 12:
        return ki18nc("December", "Dec").toString(locale());
      }
  else
    switch ( month )
      {
      case 1:
        return ki18n("January").toString(locale());
      case 2:
        return ki18n("February").toString(locale());
      case 3:
        return ki18n("March").toString(locale());
      case 4:
        return ki18n("April").toString(locale());
      case 5:
        return ki18nc("May long", "May").toString(locale());
      case 6:
        return ki18n("June").toString(locale());
      case 7:
        return ki18n("July").toString(locale());
      case 8:
        return ki18n("August").toString(locale());
      case 9:
        return ki18n("September").toString(locale());
      case 10:
        return ki18n("October").toString(locale());
      case 11:
        return ki18n("November").toString(locale());
      case 12:
        return ki18n("December").toString(locale());
      }

  return QString();
}

QString KCalendarSystemGregorian::monthNamePossessive(int month, int year,
                                                      bool shortName) const
{
  Q_UNUSED(year);

  if ( shortName )
    switch ( month )
      {
      case 1:
        return ki18nc("of January", "of Jan").toString(locale());
      case 2:
        return ki18nc("of February", "of Feb").toString(locale());
      case 3:
        return ki18nc("of March", "of Mar").toString(locale());
      case 4:
        return ki18nc("of April", "of Apr").toString(locale());
      case 5:
        return ki18nc("of May short", "of May").toString(locale());
      case 6:
        return ki18nc("of June", "of Jun").toString(locale());
      case 7:
        return ki18nc("of July", "of Jul").toString(locale());
      case 8:
        return ki18nc("of August", "of Aug").toString(locale());
      case 9:
        return ki18nc("of September", "of Sep").toString(locale());
      case 10:
        return ki18nc("of October", "of Oct").toString(locale());
      case 11:
       return ki18nc("of November", "of Nov").toString(locale());
      case 12:
        return ki18nc("of December", "of Dec").toString(locale());
      }
  else
    switch ( month )
      {
      case 1:
        return ki18n("of January").toString(locale());
      case 2:
        return ki18n("of February").toString(locale());
      case 3:
        return ki18n("of March").toString(locale());
      case 4:
        return ki18n("of April").toString(locale());
      case 5:
        return ki18nc("of May long", "of May").toString(locale());
      case 6:
        return ki18n("of June").toString(locale());
      case 7:
        return ki18n("of July").toString(locale());
      case 8:
        return ki18n("of August").toString(locale());
      case 9:
        return ki18n("of September").toString(locale());
      case 10:
        return ki18n("of October").toString(locale());
      case 11:
        return ki18n("of November").toString(locale());
      case 12:
        return ki18n("of December").toString(locale());
      }

  return QString();
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
