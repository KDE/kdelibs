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

static int jflg = 0;
int dysiz(int y);

class hdate
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
static class hdate * gdate(int y, int m, int d)
{
  static class hdate h;
  int s;

  y -= 3744;
  s = dysiz(y);
  d += s;
  s = dysiz(y + 1) - s;    /* length of year */
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
static class hdate * hdate(int y, int m, int d)
{
  static class hdate h;
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
  s = dysiz(y);
  m = dysiz(y + 1);
  while(d >= m) {  /* computed year was underestimated */
    s = m;
    y++;
    m = dysiz(y + 1);
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
#define HOUR 1080
#define DAY  (24*HOUR)
#define WEEK (7*DAY)
#define M(h,p) ((h)*HOUR+p)
#define MONTH (DAY+M(12,793))

/* other macros */
#define LEAP_YR_HEB(x) ((1L + (long)(x)* 7L) % 19L < 7L ? 1 : 0)

/**
 * @internal
 * no. of days in y years
 */
static int dysiz(int y)
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
  return ((dysiz(year) % 10) == 5);
}

/**
 * @internal
 * true if short Kislev
 */
static int short_kislev(int year)
{
  return ((dysiz(year) % 10) == 3);
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
class hdate * toHebrew(const QDate & date)
{
  class hdate *sd;
  sd = hdate(date.year(), date.month(), date.day());
  return sd;
}

int KCalendarSystemHebrew::year(const QDate& date) const
{
  class hdate *sd = toHebrew(date);
  return sd->hd_year;
}

// Ok
int KCalendarSystemHebrew::monthsInYear( const QDate & date ) const
{
  if ((((7 * year(date)) + 1)  %  19) < 7)
    return 13;
  else
    return 12;
}

// ### Fixme
int KCalendarSystemHebrew::weeksInYear(int year) const
{
  return 51;
}

// ### Fixme
int KCalendarSystemHebrew::weekNumber(const QDate& date, int * yearNum) const
{
  return date.weekNumber(yearNum);
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
QString KCalendarSystemHebrew::monthName(int month, bool shortName) const
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
  return "of " + monthNamePossessive(month, shortName);
}

bool KCalendarSystemHebrew::setYMD(QDate & date, int y, int m, int d) const
{
  // range checks
  // ### Fixme

  class hdate * gd = gdate( y, m, d );

  return date.setYMD(gd->hd_year, gd->hd_mon, gd->hd_day);
}


QString KCalendarSystemHebrew::weekDayName(int day, bool shortName) const
{
  if(shortName)
  {
     if (day != 7)
        return QString("%1").arg(day);
     else
        return QString("Sab");
  }
  else
  {
     if (day != 7)
        return QString("day  % 1").arg(day);
     else
        return QString("Sabbath");
  }
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
  class hdate *sd = toHebrew(date);
  if ( sd->hd_dw == 0 )
    return 7;
  else
    return (sd->hd_dw);
}

// Don't know
int KCalendarSystemHebrew::dayOfYear(const QDate & date) const
{
  QDate first;
  setYMD(first, year(date), 1, 1);

  return first.daysTo(date) + 1;
}

// ### Fixme
int KCalendarSystemHebrew::daysInMonth(const QDate& date) const
{
  if ( true != false ) 
    return 29;
  else
    return 30;
}

// ### CFM check. Fix Adar1 and Adar
int KCalendarSystemHebrew::hndays(int mon, int year) const
{
  if( mon == 8 /*IYYAR*/ || month == 10 /*TAMUZ*/ ||
    month == 12 /*ELUL*/ || month == 4 /*TEVET*/ || 
    month == 14 /*ADAR 2*/||
    ( month == 13 /*ADAR 1*/ && !LEAP_YR_HEB (year)) ||
    (month ==  2 /*CHESHVAN*/ && !long_cheshvan(year)) ||
    (month == 3 /*KISLEV*/ && short_kislev(year)))
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
  class hdate *sd = toHebrew(date);

  return sd->hd_day;
}

// Ok
int KCalendarSystemHebrew::month(const QDate& date) const
{
  class hdate *sd = toHebrew(date);

  return sd->hd_mon;
}

// ### Not sure
int KCalendarSystemHebrew::daysInYear(const QDate & date) const
{
  QDate first, last;
  setYMD(first, year(date), 1, 1);
  setYMD(last, year(date) + 1, 1, 1);

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

// ### Fixme
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
  m  %= 12;
  ++m;

  setYMD( result, y, m, day(date) );

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
