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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// Derived hijri kde calendar class

#include <qdatetime.h>
#include <qstring.h>

#include <klocale.h>
#include <kdebug.h>

#include <stdlib.h>
#include <math.h>

#include "kcalendarsystemhijri.h"

#define GREGORIAN_CROSSOVER 2299161

/* radians per degree (pi/180) */
static const double RadPerDeg = 0.01745329251994329577;

/* Synodic Period (mean time between 2 successive
 * new moon: 29d, 12 hr, 44min, 3sec
 */
static const double SynPeriod = 29.53058868;

/* Solar days in year/SynPeriod */
static const double SynMonth = 365.25/29.53058868;

/* Julian day on Jan 1, 1900 */
static const double jd1900 = 2415020.75933;

/* Reference point: March 26, 2001 == 1422 Hijri == 1252
 * Synodial month from 1900
 */
static const long SynRef = 1252;

static const long GregRef = 1422;

/* Local time specific to Saudi Arabia */
static const double SA_TimeZone = 3.0;

static const double EveningPeriod = 6.0;

static const long LeapYear[] = {
  2, 5, 7, 10, 13, 16, 18, 21, 24, 26, 29
};

typedef struct {
  int day;
  int mon;
  int year;
} SDATE;

/**
 * @internal
 * This function returns the Julian date/time of the Nth new moon since
 * January 1900.  The synodic month is passed as parameter.
 *
 * Adapted from "Astronomical  Formulae for Calculators" by
 * Jean Meeus, Third Edition, Willmann-Bell, 1985.
 */
static double newMoon(long n)
{
  double jd, t, t2, t3, k, ma, sa, tf, xtra;
  k = n;
  t = k/1236.85;	// Time in Julian centuries from 1900 January 0.5
  t2 = t * t;
  t3 = t2 * t;

  // Mean time of phase
  jd =  jd1900
    + SynPeriod * k
    - 0.0001178 * t2
    - 0.000000155 * t3
    + 0.00033 * sin(RadPerDeg * (166.56 + 132.87 * t - 0.009173 * t2));

  // Sun's mean anomaly in radian
  sa =  RadPerDeg * (359.2242
    + 29.10535608 * k
    - 0.0000333 * t2
    - 0.00000347 * t3);

  // Moon's mean anomaly
    ma =  RadPerDeg * (306.0253
    + 385.81691806 * k
    + 0.0107306 * t2
    + 0.00001236 * t3);

  // Moon's argument of latitude
    tf = RadPerDeg * 2.0 * (21.2964
      + 390.67050646 * k
      - 0.0016528 * t2
      - 0.00000239 * t3);

  // should reduce to interval between 0 to 1.0 before calculating further
  // Corrections for New Moon
  xtra = (0.1734 - 0.000393 * t) * sin(sa)
    + 0.0021 * sin(sa * 2)
    - 0.4068 * sin(ma)
    + 0.0161 * sin(2 * ma)
    - 0.0004 * sin(3 * ma)
    + 0.0104 * sin(tf)
    - 0.0051 * sin(sa + ma)
    - 0.0074 * sin(sa - ma)
    + 0.0004 * sin(tf + sa)
    - 0.0004 * sin(tf - sa)
    - 0.0006 * sin(tf + ma)
    + 0.0010 * sin(tf - ma)
    + 0.0005 * sin(sa + 2 * ma);

  // convert from Ephemeris Time (ET) to (approximate) Universal Time (UT)
  jd += xtra - (0.41 + 1.2053 * t + 0.4992 * t2)/1440;

  return (jd);
}

/**
 * @internal
 */
static double getJulianDay(long day, long month, long year)
{
  double jy, jm;

  if( year == 0 ) {
    return -1.0;
  }

  if( year == 1582 && month == 10 && day > 4 && day < 15 ) {
    return -1.0;
  }

  if( month > 2 )
  {
    jy = year;
    jm = month + 1;
  }
  else
  {
    jy = year - 1;
    jm = month + 13;
  }

  long intgr = (long)((long)(365.25 * jy) + (long)(30.6001 * jm)
    + day + 1720995 );

  //check for switch to Gregorian calendar
  double gregcal = 15 + 31 * ( 10 + 12 * 1582 );

  if( day + 31 * (month + 12 * year) >= gregcal )
  {
    double ja;
    ja = (long)(0.01 * jy);
    intgr += (long)(2 - ja + (long)(0.25 * ja));
  }

  return (double) intgr;
}

/*
 * compute general hijri date structure from gregorian date
 */
static SDATE * gregorianToHijri(long day, long month, long year)
{
  static SDATE h;

  double prevday;
  // CFM unused double dayfraction;
  long syndiff;
  long newsyn;
  double newjd;
  double julday;
  long synmonth;

  // Get Julian Day from Gregorian
  julday = getJulianDay(day, month, year);

  /* obtain approx. of how many Synodic months since the beginning
   * of the year 1900
   */
  synmonth = (long)(0.5 + (julday - jd1900)/SynPeriod);

  newsyn = synmonth;
  prevday = (long)julday - 0.5;

  do {
    newjd = newMoon(newsyn);

    // Decrement syndonic months
    newsyn--;
  } while (newjd > prevday);
  newsyn++;

  // difference from reference point
  syndiff = newsyn - SynRef;

  // Round up the day
  day = (long)(((long)julday) - newjd + 0.5);
  month =  (syndiff % 12) + 1;

  // currently not supported
  //dayOfYear = (sal_Int32)(month * SynPeriod + day);
  year = GregRef + (long)(syndiff / 12);

  // If month negative, consider it previous year
  if (syndiff != 0 && month <= 0) {
    month += 12;
    (year)--;
  }

  // If Before Hijri subtract 1
  if (year <= 0) (year)--;

  h.day = day;
  h.mon = month;
  h.year = year;

  return(&h);
}

/**
 * @internal
 * This algorithm is taken from "Numerical Recipes in C", 2nd ed, pp 14-15.
 * This algorithm only valid for non-negative gregorian year
 */
static void getGregorianDay(long lJulianDay, long *pnDay,
   long *pnMonth, long *pnYear)
{
  /* working variables */
  long lFactorA, lFactorB, lFactorC, lFactorD, lFactorE;
  long lAdjust;

  /* test whether to adjust for the Gregorian calendar crossover */
  if (lJulianDay >= GREGORIAN_CROSSOVER) {
    /* calculate a small adjustment */
    lAdjust = (long) (((float) (lJulianDay - 1867216) - 0.25) / 36524.25);

    lFactorA = lJulianDay + 1 + lAdjust - ((long) (0.25 * lAdjust));

  } else {
    /* no adjustment needed */
    lFactorA = lJulianDay;
  }

  lFactorB = lFactorA + 1524;
  lFactorC = (long) (6680.0 + ((float) (lFactorB - 2439870) - 122.1) / 365.25);
  lFactorD = (long) (365 * lFactorC + (0.25 * lFactorC));
  lFactorE = (long) ((lFactorB - lFactorD) / 30.6001);

  /* now, pull out the day number */
  *pnDay = lFactorB - lFactorD - (long) (30.6001 * lFactorE);

  /* ...and the month, adjusting it if necessary */
  *pnMonth = lFactorE - 1;
  if (*pnMonth > 12)
    (*pnMonth) -= 12;

  /* ...and similarly for the year */
  *pnYear = lFactorC - 4715;
  if (*pnMonth > 2)
    (*pnYear)--;

  // Negative year adjustments
  if (*pnYear <= 0)
    (*pnYear)--;
}

/*
 * compute general gregorian date structure from hijri date
 */
static SDATE *hijriToGregorian(long *day, long *month, long *year)
{
  static SDATE h;

  long nmonth;
  // CFM unused double dayfraction;
  double jday;
  // CFM unused long dayint;

  if ( *year < 0 ) (*year)++;

  // Number of month from reference point
  nmonth = *month + *year * 12 - (GregRef * 12 + 1);

  // Add Synodic Reference point
  nmonth += SynRef;

  // Get Julian days add time too
  jday = newMoon(nmonth) + *day;

  // Round-up
  jday = (double)((long)(jday + 0.5));

  // Use algorithm from "Numerical Recipes in C"
  getGregorianDay((long)jday, day, month, year);

  // Julian -> Gregorian only works for non-negative year
  if ( *year <= 0 )
  {
    *day = -1;
    *month = -1;
    *year = -1;
  }

  h.day = (int)*day;
  h.mon = (int)*month;
  h.year = (int)*year;

  return(&h);
}

static SDATE * toHijri(const QDate & date)
{
  SDATE *sd;
  sd = gregorianToHijri(date.day(), date.month(), date.year());
  return sd;
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
  SDATE *sd = toHijri(date);
  return sd->year;
}

int KCalendarSystemHijri::monthsInYear( const QDate & date ) const
{
  Q_UNUSED( date )

  return 12;
}

int KCalendarSystemHijri::weeksInYear(int year) const
{

  QDate temp;
  setYMD(temp, year, 12, hndays(12, year));

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

QString KCalendarSystemHijri::monthName(const QDate& date,
                                        bool shortName) const
{
  return monthName(month(date), shortName);
}

QString KCalendarSystemHijri::monthNamePossessive(const QDate& date,
                                                  bool shortName) const
{
  return monthNamePossessive(month(date), shortName);
}

QString KCalendarSystemHijri::monthName(int month, bool shortName) const
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

QString KCalendarSystemHijri::monthNamePossessive(int month,
                                                  bool shortName) const
{
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

  if ( d < 1 || d > hndays(m, y) )
    return false;

  SDATE * gd = hijriToGregorian( (long *)&d, (long *)&m, (long *)&y );

  return date.setYMD(gd->year, gd->mon, gd->day);
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

// From Calendrical
static int islamicLeapYear(int year)
{
// True if year is an Islamic leap year

  if ((((11 * year) + 14) % 30) < 11)
    return 1;
  else
    return 0;
}

int KCalendarSystemHijri::daysInMonth(const QDate& date) const
{
  SDATE *sd = toHijri(date);

  return hndays(sd->mon, sd->year);
}

int KCalendarSystemHijri::hndays(int month, int year) const
{
  if (((month % 2) == 1) || ((month == 12)
    && islamicLeapYear(year)))
    return 30;
  else
    return 29;
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

  SDATE *sd = toHijri(date);

  return sd->year;
}

int KCalendarSystemHijri::day(const QDate& date) const
{
  SDATE *sd = toHijri(date);
  return sd->day;
}

int KCalendarSystemHijri::month(const QDate& date) const
{
  SDATE *sd = toHijri(date);
  return sd->mon;
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
  return QString::fromLatin1("hijri");
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
