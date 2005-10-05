#ifndef KCALENDARSYSTEMJALALI_H
#define KCALENDARSYSTEMJALALI_H


/*
   Copyright (C) 2002-2003 Arash Bijanzadeh  and FarsiKDE Project <www.farsikde.org>
   Contact: Arash Bijanzadeh <a.bijanzadeh@linuxiran.org>

   This program is part of FarsiKDE

   FarsiKDE is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   FarsiKDE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#include "kcalendarsystem.h"

class KCalendarSystemJalaliPrivate;
/**
 Jalali calendar type implementation
*/
class KDECORE_EXPORT KCalendarSystemJalali : public KCalendarSystem
{
public:
  KCalendarSystemJalali(const KLocale * locale = 0);
  virtual ~KCalendarSystemJalali();



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

  virtual QString monthName (const QDate & date, bool shortName = false ) const;
  virtual QString monthNamePossessive(const QDate & date, bool shortName = false ) const;
  virtual QString weekDayName (int weekDay, bool shortName = false) const;
  virtual QString weekDayName (const QDate & date, bool shortName = false) const;
  virtual QString monthNamePossessive(int month, int year, bool shortName = false) const;
  virtual QString monthName(int month, int year, bool shortName = false) const;

  virtual int minValidYear () const;
  virtual int maxValidYear () const;
  virtual int weekDayOfPray () const;

  virtual QString calendarName() const;

  virtual bool isLunar() const;
  virtual bool isLunisolar() const;
  virtual bool isSolar() const;

};

#endif
