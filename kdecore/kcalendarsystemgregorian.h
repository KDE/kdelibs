/*
   Copyright (c) 2002 Carlos Moro <cfmoro@correo.uniovi.es>
   Copyright (c) 2002 Hans Petter Bieker <bieker@kde.org>

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

#ifndef KCALENDARSYSTEMGREGORIAN_H
#define KCALENDARSYSTEMGREGORIAN_H

#include "kcalendarsystem.h"

class KCalendarSystemGregorianPrivate;

/**
 * @internal
 * This is the Gregorian calendar implementation.
 *
 * The Gregorian calender is the most used calendar today. The first year in
 * the calendar is set to the birth of Christ.
 *
 * @see KLocale,KCalendarSystem,KCalendarSystemFactory
 *
 * @author Carlos Moro <cfmoro@correo.uniovi.es>
 * @license GNU-GPL v.2
 * @since 3.2
 */
class KDECORE_EXPORT KCalendarSystemGregorian: public KCalendarSystem
{
public:
  KCalendarSystemGregorian (const KLocale * locale = 0);
  virtual ~KCalendarSystemGregorian ();

  virtual int year (const QDate & date) const;
  virtual int month (const QDate & date) const;
  virtual int day (const QDate & date) const;
  virtual int dayOfWeek (const QDate & date) const;
  virtual int dayOfYear (const QDate & date) const;

  virtual bool setYMD(QDate & date, int y, int m, int d) const;

  virtual QDate addYears(const QDate & date, int nyears) const;
  virtual QDate addMonths(const QDate & date, int nmonths) const;
  virtual QDate addDays(const QDate & date, int ndays) const;

  virtual int monthsInYear (const QDate & date) const;

  virtual int daysInYear (const QDate & date) const;
  virtual int daysInMonth (const QDate & date) const;
  virtual int weeksInYear(int year) const;
  virtual int weekNumber(const QDate& date, int * yearNum = 0) const;

  virtual int yearStringToInteger(const QString & sNum, int & iLength) const;

  virtual QString monthName (int month, int year, bool shortName = false) const;
  virtual QString monthName (const QDate & date, bool shortName = false ) const;
  virtual QString monthNamePossessive(int month, int year, bool shortName = false) const;
  virtual QString monthNamePossessive(const QDate & date, bool shortName = false ) const;
  virtual QString weekDayName (int weekDay, bool shortName = false) const;
  virtual QString weekDayName (const QDate & date, bool shortName = false) const;

  virtual int minValidYear () const;
  virtual int maxValidYear () const;
  virtual int weekDayOfPray () const;

  virtual QString calendarName() const;

  virtual bool isLunar() const;
  virtual bool isLunisolar() const;
  virtual bool isSolar() const;

private:
  KCalendarSystemGregorianPrivate * d;
};

#endif
