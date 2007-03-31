/*
   This file is part of the KDE libraries
   Copyright (c) 2000 Waldo Bastian <bastian@kde.org>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KRFCDATE_H
#define KRFCDATE_H

#include <kde3support_export.h>
#include <time.h>

class QString;
class QByteArray;

/**
 * The K3RFCDate class contains functions related to the parsing of dates.
 *
 * @deprecated this class is made obsolete by KDateTime
 *
 * @author Waldo Bastian <bastian@kde.org>
 */
class KDE3SUPPORT_EXPORT K3RFCDate
{
public:
   /**
    * This function tries to parse a string containing a date/time in any
    * of the formats specified by RFC822, RFC850, RFC1036, RFC1123 and RFC2822.
    *
    * If the date/time could not be parsed, 0 is returned.  If the
    * parsed date is epoch, then epoch+1 is returned so that a valid
    * date will not be confused with an improper date string.
    *
    * The date/time returned is converted to UTC.
    * @param date the date to parse
    * @return the date, or 0 if not possible
    */
   static time_t parseDate(const QString &date);

   /**
    * This function tries to parse a string containing a date/time in 
    * any of the formats specified by http://www.w3.org/TR/NOTE-datetime
    *
    * This is a subset of the formats specified in ISO8601.
    *
    * If the date/time could not be parsed, 0 is returned.  If the
    * parsed date is epoch, then epoch+1 is returned so that a valid
    * date will not be confused with an improper date string.
    *
    * The date/time returned is converted to UTC.
    *
    * @param date the date to parse
    * @return the date, or 0 if not possible
    */
   static time_t parseDateISO8601(const QString &date);

  /**
   * Returns the local timezone offset to UTC in minutes
   * @return the local timezone offset in minutes
   */
   static int localUTCOffset();


  /**
   * Returns a string representation of the given date and time formated
   * in conformance to RFC2822.
   *
   * @param utcTime    a date and time in UTC
   * @param utcOffset  the offset to UTC in minutes
   * @return the string representation of the date
   */

   static QByteArray rfc2822DateString(time_t utcTime, int utcOffset=localUTCOffset());

};

#endif
