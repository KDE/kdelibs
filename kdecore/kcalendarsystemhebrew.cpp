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

#include <klocale.h>
#include <kdebug.h>

#include "kcalendarsystemhebrew.h"

static int jflg = 0; // ### HPB Remove this!?!?

static int hebrewDaysInYear(int y);

class h_date
{
public:
  int hd_day;
  int hd_mon;
  int hd_year;
  int hd_dw;
  int hd_flg;
};

/*
 * compute general date structure from hebrew date
 */
static class h_date * hebrewToGregorian(int y, int m, int d)
{
  static class h_date h;
  int s;

  y -= 3744;
  s = hebrewDaysInYear(y);
  d += s;
  s = hebrewDaysInYear(y + 1) - s;    /* length of year */
  d += (59 * (m - 1) + 1) / 2;  /* regular months */
  /* special cases */
  if (s % 10 > 4 && m > 2)  /* long Heshvan */
    d++;
  if (s % 10 < 4 && m > 3)  /* short Kislev */
    d--;
  if (s > 365 && m > 6)  /* leap year */
    d += 30;
  d -= 6002;
  if (!jflg) {  /* compute century */
    y = (d + 36525) * 4 / 146097 - 1;
    d -= y / 4 * 146097 + (y % 4) * 36524;
    y *= 100;
  } else {
    d += 2;
    y = 0;
  }
  /* compute year */
  s = (d + 366)*4/1461-1;
  d -= s/4*1461 + (s % 4)*365;
  y += s;
  /* compute month */
  m = (d + 245)*12/367-7;
  d -= m*367/12-30;
  if (++m >= 12) {
    m -= 12;
    y++;
  }
  h.hd_day = d;
  h.hd_mon = m;
  h.hd_year = y;
  return(&h);
}

/*
 * compute date structure from no. of days since 1 Tishrei 3744
 */
static class h_date * gregorianToHebrew(int y, int m, int d)
{
  static class h_date h;
  int s;

  if ((m -= 2) <= 0) {
    m += 12;
    y--;
  }
  /* no. of days, Julian calendar */
  d += 365*y + y/4 + 367*m/12 + 5968;
  /* Gregorian calendar */
  if (!jflg)
    d -= y/100-y/400-2;
  h.hd_dw = (d + 1) % 7;

  /* compute the year */
  y += 16;
  s = hebrewDaysInYear(y);
  m = hebrewDaysInYear(y + 1);
  while(d >= m) {  /* computed year was underestimated */
    s = m;
    y++;
    m = hebrewDaysInYear(y + 1);
  }
  d -= s;
  s = m-s;  /* size of current year */
  y += 3744;

  h.hd_flg = s % 10-4;

  /* compute day and month */
  if (d >= s-236) {  /* last 8 months are regular */
    d -= s-236;
    m = d*2/59;
    d -= (m*59 + 1)/2;
    m += 4;
    if (s > 365 && m <= 5)  /* Adar of Meuberet */
      m += 8;
  } else {
    /* first 4 months have 117-119 days */
    s = 114 + s % 10;
    m = d * 4 / s;
    d -= (m * s + 3) / 4;
  }

  h.hd_day = d;
  h.hd_mon = m;
  h.hd_year = y;
  return(&h);
}

/* constants, in 1/18th of minute */
static const int HOUR = 1080;
static const int DAY = 24*HOUR;
static const int WEEK = 7*DAY;
#define M(h,p) ((h)*HOUR+p)
#define MONTH (DAY+M(12,793))
  
/**
 * @internal
 * no. of days in y years
 */
static int hebrewDaysInYear(int y)
{
  int m, nm, dw, s, l;

  l = y * 7 + 1;  // no. of leap months
  m = y*12+l/19;  // total no. of months
  l %= 19;
  nm = m*MONTH+M(1+6,779); // molad new year 3744 (16BC) + 6 hours
  s = m*28+nm/DAY-2;

  nm %= WEEK;
  dw = nm/DAY;
  nm %= DAY;

  // special cases of Molad Zaken
  if (l < 12 && dw == 3 && nm >= M(9 + 6,204) ||
   l < 7 && dw == 2 && nm>=M(15+6,589))
    s++,dw++;
  /* ADU */
  if (dw == 1 || dw == 4 || dw == 6)
    s++;
  return s;
}

/**
 * @internal
 * true if long Cheshvan
 */
static int long_cheshvan(int year)
{
  return ((hebrewDaysInYear(year) % 10) == 5);
}

/**
 * @internal
 * true if short Kislev
 */
static int short_kislev(int year)
{
  return ((hebrewDaysInYear(year) % 10) == 3);
}

static bool is_leap_year(int year)
{
  return ((((7 * year) + 1) % 19) < 7);
}

// Ok
KCalendarSystemHebrew::KCalendarSystemHebrew(const KLocale * locale)
  : KCalendarSystem(locale)
{
}

// Ok
KCalendarSystemHebrew::~KCalendarSystemHebrew()
{
}

// Ok
static class h_date * toHebrew(const QDate & date)
{
  class h_date *sd;
  sd = gregorianToHebrew(date.year(), date.month(), date.day());
  ++sd->hd_mon;
  ++sd->hd_day;
  return sd;
}

// Ok
int KCalendarSystemHebrew::year(const QDate& date) const
{
  class h_date *sd = toHebrew(date);
  return sd->hd_year;
}

// Ok
int KCalendarSystemHebrew::monthsInYear( const QDate & date ) const
{
  if ( is_leap_year( year(date) ) )
    return 13;
  else
    return 12;
}

// ### HPB Recursive call without termination
int KCalendarSystemHebrew::weeksInYear(int year) const
{
  QDate temp;
  setYMD(temp, year + 1, 1, 1);

  // If the first day of the next year is in the first week, we have to
  // check the week before
  int yearNum;
  int nWeekNumber = weekNumber(temp, &yearNum);
  if ( yearNum != year )
    temp = addDays(temp, -7);

  return nWeekNumber;
}

// Ok
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
    week = firstDayWeek1.daysTo(date) / 7 + 1;

  return week;
}

// Ok
QString KCalendarSystemHebrew::monthName(const QDate& date,
                                        bool shortName) const
{
  return monthName(month(date), shortName);
}

// Ok
QString KCalendarSystemHebrew::monthNamePossessive(const QDate& date,
                                                  bool shortName) const
{
  return monthNamePossessive(month(date), shortName);
}

// ### Fixme
QString KCalendarSystemHebrew::monthName(int month, bool /*shortName*/) const
{
  switch(month)
  {
  case 1:
    return locale()->translate("Tishrey");
  case 2:
    return locale()->translate("Heshvan");
  case 3:
    return locale()->translate("Kislev");
  case 4:
    return locale()->translate("Tevet");
  case 5:
    return locale()->translate("Shvat");
  case 6:
    return locale()->translate("Adar");
  case 7:
    return locale()->translate("Nisan");
  case 8:
    return locale()->translate("Iyar");
  case 9:
    return locale()->translate("Sivan");
  case 10:
    return locale()->translate("Tamuz");
  case 11:
    return locale()->translate("Av");
  case 12:
    return locale()->translate("Elul");
  case 13:
    return locale()->translate("Adar I");
  case 14:
    return locale()->translate("Adar II");
  default:
    break;
  }

  return QString::null;
}

// ### Fixme
QString KCalendarSystemHebrew::monthNamePossessive(int month,
                                                  bool shortName) const
{
  return "of " + monthName(month, shortName);
}

bool KCalendarSystemHebrew::setYMD(QDate & date, int y, int m, int d) const
{
  // range checks
  // ### Fixme

  class h_date * gd = hebrewToGregorian( y, m, d );

  return date.setYMD(gd->hd_year, gd->hd_mon + 1, gd->hd_day + 1);
}

QString KCalendarSystemHebrew::weekDayName(int day, bool shortName) const
{
  if ( day == 6 )
  {
    if (shortName)
      return locale()->translate("Hebrew Calendar WeekDay 6 Short", "Sab");
    else
      return locale()->translate("Hebrew Calendar WeekDay 6 Long", "Sabbath");
  }

  return locale()->weekDayName(day, shortName);
}

// Ok
QString KCalendarSystemHebrew::weekDayName(const QDate& date,
                                          bool shortName) const
{
  return weekDayName(dayOfWeek(date), shortName);
}

// Ok
int KCalendarSystemHebrew::dayOfWeek(const QDate& date) const
{
  class h_date *sd = toHebrew(date);
  if ( sd->hd_dw == 0 )
    return 7;
  else
    return (sd->hd_dw);
}

// Ok
int KCalendarSystemHebrew::dayOfYear(const QDate & date) const
{
  QDate first;
  setYMD(first, year(date), 1, 1);

  return first.daysTo(date) + 1;
}

int KCalendarSystemHebrew::daysInMonth(const QDate& date) const
{
  class h_date *sd = toHebrew(date);
  return hndays(sd->hd_mon, sd->hd_year);
}

// ### CFM check. Fix Adar1 and Adar
int KCalendarSystemHebrew::hndays(int mon, int year) const
{
  if( mon == 8 /*IYYAR*/ || mon == 10 /*TAMUZ*/ ||
    mon == 12 /*ELUL*/ || mon == 4 /*TEVET*/ || 
    mon == 14 /*ADAR 2*/||
    ( mon == 13 /*ADAR 1*/ && !is_leap_year(year)) /*!leap*/||
    (mon ==  2 /*CHESHVAN*/ && !long_cheshvan(year)) ||
    (mon == 3 /*KISLEV*/ && short_kislev(year)))
    return 29;
  else
    return 30;
}

// Ok
// Min valid year that may be converted to QDate
int KCalendarSystemHebrew::minValidYear() const
{
  QDate date(1753, 1, 1);

  return year(date);
}

// Ok
// Max valid year that may be converted to QDate
int KCalendarSystemHebrew::maxValidYear() const
{
  QDate date(8000, 1, 1);

  return year(date);
}

// Ok
int KCalendarSystemHebrew::day(const QDate& date) const
{
  class h_date *sd = toHebrew(date);

  return sd->hd_day;
}

// Ok
int KCalendarSystemHebrew::month(const QDate& date) const
{
  class h_date *sd = toHebrew(date);

  return sd->hd_mon;
}

// Ok
int KCalendarSystemHebrew::daysInYear(const QDate & date) const
{
  QDate first, last;
  setYMD(first, year(date), 1, 1); // 1 Tishrey
  setYMD(last, year(date) + 1, 1, 1); // 1 Tishrey the year later

  return first.daysTo(last);
}

// Ok
int KCalendarSystemHebrew::weekDayOfPray() const
{
  return 6; // saturday
}

// Ok
QDate KCalendarSystemHebrew::addDays( const QDate & date, int ndays ) const
{
  return date.addDays( ndays );
}

// Ok
QDate KCalendarSystemHebrew::addMonths( const QDate & date, int nmonths ) const
{
  QDate result = date;

  while ( nmonths > 0 )
  {
    result = addDays(result, daysInMonth(result));
    --nmonths;
  }

  while ( nmonths < 0 )
  {
    // get the number of days in the previous month to be consistent with 
    // addMonths where nmonths > 0
    int nDaysInMonth = daysInMonth(addDays(result, -day(result)));
    result = addDays(result, -nDaysInMonth);
    ++nmonths;
  }

  return result;
}

// Ok
QDate KCalendarSystemHebrew::addYears( const QDate & date, int nyears ) const
{
  QDate result = date;
  int y = year(date) + nyears;

  setYMD( result, y, month(date), day(date) );

  return result;
}

// Ok
QString KCalendarSystemHebrew::calendarName() const
{
  return QString::fromLatin1("hebrew");
}

// Ok
bool KCalendarSystemHebrew::isLunar() const
{
  return false;
}

// Ok
bool KCalendarSystemHebrew::isLunisolar() const
{
  return true;
}

// Ok
bool KCalendarSystemHebrew::isSolar() const
{
  return false;
}
