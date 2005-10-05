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


#include "kcalendarsystemjalali.h"

#include <qdatetime.h>
#include <qstring.h>
#include <math.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <stdio.h>


static const int  gMonthDay[2][13]={
        {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
};

static const    int     jMonthDay[2][13] = {
        {0, 31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29},
        {0, 31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 30},
};

typedef struct {
        int day;
        int mon;
        int year;
        } SDATE;
// converting funcs from

static int Ceil(float number)
{
    int ret;
    if(number>0)
	number += 0.5;
    ret =(int) number;
    return ret;
}

static long jalali_jdn(int year, int month, int day)
{
    const long PERSIAN_EPOCH = 1948321; /* The JDN of 1 Farvardin 1*/
    int epbase;
    long epyear;
    long mdays;
    long jdn;
    epbase = year - 474;
    epyear = 474 + (epbase % 2820);
    if (month <= 7)
        mdays = (month - 1) * 31;
    else
        mdays = (month - 1) * 30 + 6;
    jdn = day + mdays ;
    jdn += (((epyear * 682) - 110) / 2816) ;
    jdn	+= (epyear - 1) * 365;
    jdn += (epbase / 2820) * 1029983 ;
    jdn += (PERSIAN_EPOCH - 1);
    return jdn;
}


static SDATE jdn_jalali(long jdn)
{
    static SDATE ret;
    int day, month, year;
    int iYear, iMonth, iDay;
    int depoch;
    int cycle;
    int cyear;
    int ycycle;
    int aux1, aux2;
    int yday;
    day = 1;
    month = 1;
    year = 475;
    depoch = jdn - jalali_jdn(year,month, day);
    cycle = (int) (depoch / 1029983);
    cyear = depoch % 1029983;
    if( cyear == 1029982)
        ycycle = 2820;
    else{
        aux1 = cyear / 366;
        aux2 = cyear % 366;
        ycycle = (((2134 * aux1) + (2816 * aux2) + 2815) / 1028522) + aux1 + 1;
    }
    iYear = ycycle + (2820 * cycle) + 474;
    if (iYear <= 0)
        iYear = iYear - 1;
    year = iYear;
    yday = (jdn - jalali_jdn(year, month, day)) + 1;
    if(yday <= 186 )
        iMonth = Ceil((yday-1) / 31);
    else
        iMonth = Ceil((yday - 7) / 30);
    iMonth++;
    month = iMonth;
    iDay = (jdn - jalali_jdn(year, month, day)) + 1;
    ret.day = iDay;
    ret.mon = iMonth;
    ret.year = iYear;
    return ret;
}



static long civil_jdn(int year, int month, int day)
{
    long jdn = ((1461 * (year + 4800 + ((month - 14) / 12))) / 4)
	+ ((367 * (month - 2 - 12 * (((month - 14) / 12)))) / 12)
	- ((3 * (((year + 4900 + ((month - 14) / 12)) / 100))) / 4)
	+ day - 32075;
    return jdn;
}

static SDATE jdn_civil(long jdn)
{
    long l, n, i, j;
    static SDATE ret;
    int iday, imonth, iyear;
    l = jdn + 68569;
    n = ((4 * l) / 146097);
    l = l - ((146097 * n + 3) / 4);
    i = ((4000 * (l + 1)) / 1461001);
    l = l - ((1461 * i) / 4) + 31;
    j = ((80 * l) / 2447);
    iday = l - ((2447 * j) / 80);
    l = (j / 11);
    imonth = j + 2 - 12 * l;
    iyear = 100 * (n - 49) + i + l;
    ret.day = iday;
    ret.mon = imonth;
    ret.year = iyear;
    return (ret);
}

static SDATE *jalaliToGregorian(int y,int m,int d)
{
static SDATE sd;
long jday = jalali_jdn(y,m,d);
sd= jdn_civil(jday);
return (&sd);
}
static SDATE *gregorianToJalali(int y,int m, int d)
{
    static SDATE sd;
    long   jdn = civil_jdn(y,m,d);//QDate::gregorianToJulian(y, m, d);
    sd = jdn_jalali(jdn);
    return(&sd);
}
static void gregorianToJalali(const QDate & date, int * pYear, int * pMonth,
                               int * pDay)
{
  SDATE *sd;
  sd = gregorianToJalali(date.year(), date.month(), date.day());
  if (pYear)
    *pYear = sd->year;
  if (pMonth)
    *pMonth = sd->mon;
  if (pDay)
    *pDay = sd->day;

}

// End of converting functions

static int isJalaliLeap(int year)
{
 int     tmp;
 tmp = year % 33;
 if (tmp == 1 || tmp == 5||tmp==9||tmp==13||tmp==17||tmp==22||tmp==26||tmp==30)
     return 1;
else
     return 0;
}
static int hndays(int m,int y)
{
  return jMonthDay[isJalaliLeap(y)][m];
}


KCalendarSystemJalali::KCalendarSystemJalali(const KLocale * locale)
  : KCalendarSystem(locale)
{
}

KCalendarSystemJalali::~KCalendarSystemJalali()
{
}

int KCalendarSystemJalali::year(const QDate& date) const

{
  kdDebug(5400) << "Jalali year..." <<  endl;
int y;
  gregorianToJalali(date, &y, 0, 0);
  return y;
}

int KCalendarSystemJalali::month (const QDate& date) const

{
  kdDebug(5400) << "Jalali month..." <<  endl;
int m;
  gregorianToJalali(date, 0 , &m, 0);
  return m;
}

int KCalendarSystemJalali::day(const QDate& date) const

{
  kdDebug(5400) << "Jalali day..." <<  endl;
int d;
  gregorianToJalali(date, 0, 0, &d);
  return d;
}

int KCalendarSystemJalali::dayOfWeek(const QDate& date) const
{
//same same I think?!
  return date.dayOfWeek();

}

//NOT TESTED YET
int KCalendarSystemJalali::dayOfYear(const QDate & date) const
{
  QDate first;
  setYMD(first, year(date), 1, 1);

  return first.daysTo(date) + 1;
}

//MAY BE BUGGY
bool KCalendarSystemJalali::setYMD(QDate & date, int y, int m, int d) const
{
  // range checks
  if ( y < minValidYear() || y > maxValidYear() )
    return false;

  if ( m < 1 || m > 12 )
    return false;

  if ( d < 1 || d > hndays(m, y) )
    return false;

  SDATE  *gd =jalaliToGregorian( y, m, d);

  return date.setYMD(gd->year, gd->mon, gd->day);
}

QDate KCalendarSystemJalali::addYears( const QDate & date, int nyears ) const
{
  QDate result = date;
  int y = year(date) + nyears;
  setYMD( result, y, month(date), day(date) );

  return result;
}

QDate KCalendarSystemJalali::addMonths( const QDate & date, int nmonths ) const
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

QDate KCalendarSystemJalali::addDays( const QDate & date, int ndays ) const
{
  return date.addDays( ndays );
}

int KCalendarSystemJalali::monthsInYear( const QDate & date ) const
{
  Q_UNUSED( date )

  return 12;
}

int KCalendarSystemJalali::daysInYear(const QDate & date) const
{
Q_UNUSED(date);
int result;
//SDATE *sd = gregorianToJalali(year(date),month(date),day(date));
//if (isJalaliLeap(sd->year))
	result=366;
//else
//	result=365;
return result;
}

int KCalendarSystemJalali::daysInMonth(const QDate & date) const
{
SDATE *sd = gregorianToJalali(date.year(),date.month(),date.day());
return hndays(sd->mon,sd->year);
}

int KCalendarSystemJalali::weeksInYear(int year) const

{
  Q_UNUSED(year);
// couldn't understand it!
return 52;
}

int KCalendarSystemJalali::weekNumber(const QDate& date, int * yearNum) const
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

QString KCalendarSystemJalali::monthName(int month, int year, bool shortName)
  const
{
  Q_UNUSED(year);

  if (shortName)
    switch ( month )
      {
      case 1:
        return locale()->translate("Far");
      case 2:
        return locale()->translate("Ord");
      case 3:
        return locale()->translate("Kho");
      case 4:
        return locale()->translate("Tir");
      case 5:
        return locale()->translate("Mor");
      case 6:
        return locale()->translate("Sha");
      case 7:
        return locale()->translate("Meh");
      case 8:
        return locale()->translate("Aba");
      case 9:
        return locale()->translate("Aza");
      case 10:
        return locale()->translate("Dei");
      case 11:
        return locale()->translate("Bah");
      case 12:
        return locale()->translate("Esf");
    }
  else
    switch ( month )
      {
      case 1:
        return locale()->translate("Farvardin");
      case 2:
        return locale()->translate("Ordibehesht");
      case 3:
        return locale()->translate("Khordad");
      case 4:
        return locale()->translate("Tir");
      case 5:
        return locale()->translate("Mordad");
      case 6:
        return locale()->translate("Shahrivar");
      case 7:
        return locale()->translate("Mehr");
      case 8:
        return locale()->translate("Aban");
      case 9:
        return locale()->translate("Azar");
      case 10:
      	return locale()->translate("Dei");
      case 11:
        return locale()->translate("Bahman");
      case 12:
        return locale()->translate("Esfand");
      }

  return QString::null;
}

QString KCalendarSystemJalali::monthName(const QDate& date, bool shortName)
  const
{
  int mon;
  gregorianToJalali(date,0,&mon,0);
  //SDATE *sd = gregorianToJalali(date.year(),date.month(),date.day());
  return (monthName(mon, 0, shortName));
}

QString KCalendarSystemJalali::monthNamePossessive(const QDate& date,
                                                     bool shortName  ) const
{
  return monthName(date,shortName);
}

QString KCalendarSystemJalali::monthNamePossessive(int month, int year,
                                                     bool shortName ) const
{
  return monthName(month,year,shortName);
}


QString KCalendarSystemJalali::weekDayName(int day, bool shortName) const
{
  if ( shortName )
    switch (day)
      {
      case 1:
        return locale()->translate("2sh");
      case 2:
        return locale()->translate("3sh");
      case 3:
        return locale()->translate("4sh");
      case 4:
        return locale()->translate("5sh");
      case 5:
        return locale()->translate("Jom");
      case 6:
        return locale()->translate("shn");
      case 7:
        return locale()->translate("1sh");
      }
  else
    switch ( day )
      {
      case 1:
        return locale()->translate("Do shanbe");
      case 2:
        return locale()->translate("Se shanbe");
      case 3:
        return locale()->translate("Chahar shanbe");
      case 4:
        return locale()->translate("Panj shanbe");
      case 5:
        return locale()->translate("Jumee");
      case 6:
        return locale()->translate("Shanbe");
      case 7:
        return locale()->translate("Yek-shanbe");
      }

  return QString::null;
}

QString KCalendarSystemJalali::weekDayName(const QDate &date,bool shortName)
  const
{
 return weekDayName(dayOfWeek(date), shortName);
}

// Min valid year that may be converted to QDate
int KCalendarSystemJalali::minValidYear() const
{
  QDate date(1753, 1, 1);

  return year(date);
}

// Max valid year that may be converted to QDate
int KCalendarSystemJalali::maxValidYear() const
{
/*
  QDate date(8000, 1, 1);

  SDATE *sd = toJalali(date);

  return sd->year;
  */
  return 10000;
}
int KCalendarSystemJalali::weekDayOfPray() const
{
  return 5; // friday
}
QString KCalendarSystemJalali::calendarName() const
{
  return QLatin1String("jalali");
}

bool KCalendarSystemJalali::isLunar() const
{
  return false;
}

bool KCalendarSystemJalali::isLunisolar() const
{
  return false;
}

bool KCalendarSystemJalali::isSolar() const
{
  return true;
}
