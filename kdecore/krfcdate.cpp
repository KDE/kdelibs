/*
 *
 *  This file is part of the KDE libraries
 *  Copyright (c) 2000-2002 Waldo Bastian <bastian@kde.org>
 *                2002 Rik Hemsley <rik@kde.org>
 *
 * $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include <config.h>

#include <sys/param.h>
#include <ctype.h>
#include <stdlib.h>

#include <qstringlist.h>

#include <krfcdate.h>

static unsigned int ymdhms_to_seconds(int year, int mon, int day, int hour, int minute, int second)
{
    if (sizeof(time_t) == 4)
    {
       if ((time_t)-1 < 0)
       {
          if (year >= 2038)
          {
             year = 2038;
             mon = 0;
             day = 1;
             hour = 0;
             minute = 0;
             second = 0;
          }
       }
       else
       {
          if (year >= 2115)
          {
             year = 2115;
             mon = 0;
             day = 1;
             hour = 0;
             minute = 0;
             second = 0;
          }
       }
    }

    unsigned int ret = (day - 32075)       /* days */
            + 1461L * (year + 4800L + (mon - 14) / 12) / 4
            + 367 * (mon - 2 - (mon - 14) / 12 * 12) / 12
            - 3 * ((year + 4900L + (mon - 14) / 12) / 100) / 4
            - 2440588;
    ret = 24*ret + hour;     /* hours   */
    ret = 60*ret + minute;   /* minutes */
    ret = 60*ret + second;   /* seconds */

    return ret;
}

static const char haystack[37]="janfebmaraprmayjunjulaugsepoctnovdec";

// we follow the recommendation of rfc2822 to consider all
// obsolete time zones not listed here equivalent to "-0000"
static const struct {
    const char *tzName;
    int tzOffset;
} known_zones[] = {
    { "UT", 0 },
    { "GMT", 0 },
    { "EST", -300 },
    { "EDT", -240 },
    { "CST", -360 },
    { "CDT", -300 },
    { "MST", -420 },
    { "MDT", -360 },
    { "PST", -480 },
    { "PDT", -420 },
    { 0, 0 }
};

time_t
KRFCDate::parseDate(const QString &_date)
{
     // This parse a date in the form:
     //     Wednesday, 09-Nov-99 23:12:40 GMT
     // or
     //     Sat, 01-Jan-2000 08:00:00 GMT
     // or
     //     Sat, 01 Jan 2000 08:00:00 GMT
     // or
     //     01 Jan 99 22:00 +0100    (exceptions in rfc822/rfc2822)
     //
     // We ignore the weekday
     //
     time_t result = 0;
     int offset = 0;
     char *newPosStr;
     const char *dateString = _date.latin1();
     int day = 0;
     char monthStr[4];
     int month = -1;
     int year = 0;
     int hour = 0;
     int minute = 0;
     int second = 0;

     // Strip leading space
     while(*dateString && isspace(*dateString))
     	dateString++;

     // Strip weekday
     while(*dateString && !isdigit(*dateString) && !isspace(*dateString))
     	dateString++;

     // Strip trailing space
     while(*dateString && isspace(*dateString))
     	dateString++;

     if (!*dateString)
     	return result;  // Invalid date

     if (isalpha(*dateString))
     {
        // ' Nov 5 1994 18:15:30 GMT'
        // Strip leading space
        while(*dateString && isspace(*dateString))
           dateString++;

        for(int i=0; i < 3;i++)
        {
           if (!*dateString || (*dateString == '-') || isspace(*dateString))
              return result;  // Invalid date
           monthStr[i] = tolower(*dateString++);
        }
        monthStr[3] = '\0';

        newPosStr = (char*)strstr(haystack, monthStr);

        if (!newPosStr)
           return result;  // Invalid date

        month = (newPosStr-haystack)/3; // Jan=00, Feb=01, Mar=02, ..

        if ((month < 0) || (month > 11))
           return result;  // Invalid date

        while (*dateString && isalpha(*dateString))
           dateString++; // Skip rest of month-name
     }

     // ' 09-Nov-99 23:12:40 GMT'
     // ' 5 1994 18:15:30 GMT'
     day = strtol(dateString, &newPosStr, 10);
     dateString = newPosStr;

     if ((day < 1) || (day > 31))
         return result; // Invalid date;

     if (!*dateString)
        return result;  // Invalid date

     while(*dateString && (isspace(*dateString) || (*dateString == '-')))
     	dateString++;

     if (month == -1)
     {
        for(int i=0; i < 3;i++)
        {
           if (!*dateString || (*dateString == '-') || isspace(*dateString))
              return result;  // Invalid date
           monthStr[i] = tolower(*dateString++);
        }
        monthStr[3] = '\0';
        
        newPosStr = (char*)strstr(haystack, monthStr);

        if (!newPosStr)
           return result;  // Invalid date

        month = (newPosStr-haystack)/3; // Jan=00, Feb=01, Mar=02, ..

        if ((month < 0) || (month > 11))
           return result;  // Invalid date
           
        while (*dateString && isalpha(*dateString))
           dateString++; // Skip rest of month-name
           
     }

     // '-99 23:12:40 GMT'
     while(*dateString && (isspace(*dateString) || (*dateString == '-')))
     	dateString++;

     if (!*dateString || !isdigit(*dateString))
     	return result;  // Invalid date

     // '99 23:12:40 GMT'
     year = strtol(dateString, &newPosStr, 10);
     dateString = newPosStr;

     // Y2K: Solve 2 digit years
     if ((year >= 0) && (year < 50))
         year += 2000;

     if ((year >= 50) && (year < 100))
         year += 1900;  // Y2K

     if ((year < 1900) || (year > 2500))
     	return result; // Invalid date

     // Don't fail if the time is missing.
     if (*dateString)
     {
        // ' 23:12:40 GMT'
        if (!isspace(*dateString++))
           return result;  // Invalid date

        hour = strtol(dateString, &newPosStr, 10);
        dateString = newPosStr;

        if ((hour < 0) || (hour > 23))
           return result; // Invalid date

        if (!*dateString)
           return result;  // Invalid date

        // ':12:40 GMT'
        if (*dateString++ != ':')
           return result;  // Invalid date

        minute = strtol(dateString, &newPosStr, 10);
        dateString = newPosStr;

        if ((minute < 0) || (minute > 59))
           return result; // Invalid date

        if (!*dateString)
           return result;  // Invalid date

        // ':40 GMT'
        if (*dateString != ':' && !isspace(*dateString))
           return result;  // Invalid date

        // seconds are optional in rfc822 + rfc2822
        if (*dateString ==':') {
           dateString++;

           second = strtol(dateString, &newPosStr, 10);
           dateString = newPosStr;

           if ((second < 0) || (second > 59))
              return result; // Invalid date
        } else {
           dateString++;
        }

        while(*dateString && isspace(*dateString))
           dateString++;
     }

     // don't fail if the time zone is missing, some
     // broken mail-/news-clients omit the time zone
     if (*dateString) {
        if ((strncasecmp(dateString, "gmt", 3) == 0) ||
            (strncasecmp(dateString, "utc", 3) == 0))
        {
           dateString += 3;
           while(*dateString && isspace(*dateString))
              dateString++;
        }

        if ((*dateString == '+') || (*dateString == '-')) {
           offset = strtol(dateString, &newPosStr, 10);
           if (abs(offset) < 30)
           {
              dateString = newPosStr;
              
              offset = offset * 100;
              
              if (*dateString && *(dateString+1))
              {
                 dateString++;
                 int minutes = strtol(dateString, &newPosStr, 10);
                 if (offset > 0)
                    offset += minutes;
                 else
                    offset -= minutes;
              }
           }

           if ((offset < -9959) || (offset > 9959))
              return result; // Invalid date

           int sgn = (offset < 0)? -1:1;
           offset = abs(offset);
           offset = ((offset / 100)*60 + (offset % 100))*sgn;
        } else {
           for (int i=0; known_zones[i].tzName != 0; i++) {
              if (0 == strncasecmp(dateString, known_zones[i].tzName, strlen(known_zones[i].tzName))) {
                 offset = known_zones[i].tzOffset;
                 break;
              }
           }
        }
     }

     result = ymdhms_to_seconds(year, month+1, day, hour, minute, second);

     // avoid negative time values
     if ((offset > 0) && (offset > result))
        offset = 0;

     result -= offset*60;

     // If epoch 0 return epoch +1 which is Thu, 01-Jan-70 00:00:01 GMT
     // This is so that parse error and valid epoch 0 return values won't
     // be the same for sensitive applications...
     if (result < 1) result = 1;

     return result;
}

time_t
KRFCDate::parseDateISO8601( const QString& input)
{
  // These dates look like this:
  // YYYY-MM-DDTHH:MM:SS
  // But they may also have 0, 1 or 2 suffixes.
  // Suffix 1: .secfrac (fraction of second)
  // Suffix 2: Either 'Z' or +zone or -zone, where zone is HHMM

  unsigned int year     = 0;
  unsigned int month    = 0;
  unsigned int mday     = 0;
  unsigned int hour     = 0;
  unsigned int min      = 0;
  unsigned int sec      = 0;

  int offset = 0;

  // First find the 'T' separator.
  int tPos = input.find('T');

  if (-1 == tPos)
    return 0;

  // Now parse the date part.

  QString dateString = input.left(tPos).stripWhiteSpace();

  QString timeString = input.mid(tPos + 1).stripWhiteSpace();

  QStringList l = QStringList::split('-', dateString);

  year   = l[0].toUInt();
  month  = l[1].toUInt();
  mday   = l[2].toUInt();

  // Z suffix means UTC.
  if ('Z' == timeString.at(timeString.length() - 1)) {
    timeString.remove(timeString.length() - 1, 1);
  }

  // +zone or -zone suffix (offset from UTC).

  int plusPos = timeString.findRev('+');

  if (-1 != plusPos) {
    QString offsetString = timeString.mid(plusPos + 1);

    offset = offsetString.left(2).toUInt() * 60 + offsetString.right(2).toUInt();

    timeString = timeString.left(plusPos);
  } else {
    int minusPos = timeString.findRev('-');

    if (-1 != minusPos) {
      QString offsetString = timeString.mid(minusPos + 1);

      offset = - (offsetString.left(2).toUInt() * 60 + offsetString.right(2).toUInt());

      timeString = timeString.left(minusPos);
    }
  }

  // secfrac suffix.
  int dotPos = timeString.findRev('.');

  if (-1 != dotPos) {
    timeString = timeString.left(dotPos);
  }

  // Now parse the time part.

  l = QStringList::split(':', timeString);

  hour   = l[0].toUInt();
  min    = l[1].toUInt();
  sec    = l[2].toUInt();

  time_t result = ymdhms_to_seconds(year, month, mday, hour, min, sec);

  // avoid negative time values
  if ((offset > 0) && (offset > result))
     offset = 0;

  result -= offset*60;

  // If epoch 0 return epoch +1 which is Thu, 01-Jan-70 00:00:01 GMT
  // This is so that parse error and valid epoch 0 return values won't
  // be the same for sensitive applications...
  if (result < 1) result = 1;

  return result;
}


int KRFCDate::localUTCOffset()
{
  time_t timeNow = time((time_t*) 0);

  tm *tM = gmtime(&timeNow);
  unsigned int timeUTC = ymdhms_to_seconds(tM->tm_year+1900, tM->tm_mon+1, tM->tm_mday,
                                           tM->tm_hour, tM->tm_min, tM->tm_sec);

  tM = localtime(&timeNow);
  unsigned int timeLocal = ymdhms_to_seconds(tM->tm_year+1900, tM->tm_mon+1, tM->tm_mday,
                                             tM->tm_hour, tM->tm_min, tM->tm_sec);

  return ((int)(timeLocal-timeUTC))/60;
}


static const char * const day_names[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static const char * const month_names[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};


QCString KRFCDate::rfc2822DateString(time_t utcTime, int utcOffset)
{
    utcTime += utcOffset * 60;
    tm *tM = gmtime(&utcTime);
    char sgn = (utcOffset < 0) ? '-' : '+';
    int z = (utcOffset < 0) ? -utcOffset : utcOffset;
    QCString dateStr;

    dateStr.sprintf("%s, %02d %s %04d %02d:%02d:%02d %c%02d%02d",
                    day_names[tM->tm_wday], tM->tm_mday,
                    month_names[tM->tm_mon], tM->tm_year+1900,
                    tM->tm_hour, tM->tm_min, tM->tm_sec,
                    sgn, z/60%24, z%60);

    return dateStr;
}


QCString KRFCDate::rfc2822DateString(time_t utcTime)
{
    return rfc2822DateString(utcTime, localUTCOffset());
}
