/*
 *
 *  This file is part of the KDE libraries
 *  Copyright (c) 2000 Waldo Bastian <bastian@kde.org>
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

#include <sys/param.h>

#include <ctype.h>
#include <stdlib.h>

#include <krfcdate.h>

static int ymd_to_days(int year, int mon, int day)
{
    return (day - 32075)
            + 1461L * (year + 4800L + (mon - 14) / 12) / 4
            + 367 * (mon - 2 - (mon - 14) / 12 * 12) / 12
            - 3 * ((year + 4900L + (mon - 14) / 12) / 100) / 4
            - 2440588;
}

static const char haystack[37]="janfebmaraprmayjunjulaugsepoctnovdec";

time_t 
KRFCDate::parseDate(const QString &_date)
{
     // This parse a date in the form:
     //     Wednesday, 09-Nov-99 23:12:40 GMT
     // or
     //     Sat, 01-Jan-2000 08:00:00 GMT
     // or
     //     Sat, 01 Jan 2000 08:00:00 GMT
     //
     // We always assume GMT, the weekday is ignored
     //
     time_t result = 0;
     char *newPosStr;
     const char *dateString = _date.latin1();
     int day;
     char monthStr[4];
     int month;
     int year;
     int hour;
     int minute;
     int second;

     while(*dateString && (*dateString != ' '))
     	dateString++;

     if (!*dateString)
     	return result;  // Invalid expire date

     // ' 09-Nov-99 23:12:40 GMT'
     day = strtol(dateString, &newPosStr, 10);
     dateString = newPosStr;

     if ((day < 1) || (day > 31))
     	return result; // Invalid expire date;
     if (!*dateString)
     	return result;  // Invalid expire date

     if ((*dateString != '-') && (*dateString != ' '))
     	return result;  // Invalid expire date
     dateString++;

     for(int i=0; i < 3;i++)
     {
         if (!*dateString || (*dateString == '-') || (*dateString == ' '))
              return result;  // Invalid expire date
         monthStr[i] = tolower(*dateString++);
     }
     monthStr[3] = '\0';

     newPosStr = (char*)strstr(haystack, monthStr);

     if (!newPosStr)
     	return result;  // Invalid expire date

     month = (newPosStr-haystack)/3; // Jan=00, Feb=01, Mar=02, ..

     if ((month < 0) || (month > 11))
     	return result;  // Invalid expire date

     while(*dateString && (*dateString != '-') && (*dateString != ' '))
     	dateString++;

     if (!*dateString)
     	return result;  // Invalid expire date

     // '-99 23:12:40 GMT'
     if ((*dateString != '-') && (*dateString != ' '))
     	return result;  // Invalid expire date
     dateString++;

     // '99 23:12:40 GMT'
     year = strtol(dateString, &newPosStr, 10);
     dateString = newPosStr;

     // Y2K: Solve 2 digit years
     if ((year >= 0) && (year < 50))
         year += 2000;

     if ((year >= 50) && (year < 100))
         year += 1900;  // Y2K

     if ((year < 1900) || (year > 2500))
     	return result; // Invalid expire date

     if (!*dateString)
     	return result;  // Invalid expire date

     // ' 23:12:40 GMT'
     if (*dateString++ != ' ')
     	return result;  // Invalid expire date

     hour = strtol(dateString, &newPosStr, 10);
     dateString = newPosStr;

     if ((hour < 0) || (hour > 23))
     	return result; // Invalid expire date

     if (!*dateString)
     	return result;  // Invalid expire date

     // ':12:40 GMT'
     if (*dateString++ != ':')
     	return result;  // Invalid expire date

     minute = strtol(dateString, &newPosStr, 10);
     dateString = newPosStr;

     if ((minute < 0) || (minute > 59))
     	return result; // Invalid expire date

     if (!*dateString)
     	return result;  // Invalid expire date

     // ':40 GMT'
     if (*dateString++ != ':')
     	return result;  // Invalid expire date

     second = strtol(dateString, &newPosStr, 10);

     if ((second < 0) || (second > 59))
     	return result; // Invalid expire date

     result = ymd_to_days(year, month+1, day); /* days */
     result = 24*result + hour;  /* hours   */
     result = 60*result + minute;   /* minutes */
     result = 60*result + second;   /* seconds */

////////////////
// Debug stuff
//
//   printf("time = %ld sec since epoch, ctime=%s\n", result, ctime(&result));
//
//   time_t now = time(0);
//   printf("now = %ld (expire is in %s)\n", now, result > now ? "future" : "past");
////////////////
     // If epoch 0 return epoch +1 which is Thu, 01-Jan-70 00:00:01 GMT
     // This is so that parse error and valid epoch 0 return values won't
     // be the same for sensitive applications...
     if( !result ) result++;
     return result;
}
