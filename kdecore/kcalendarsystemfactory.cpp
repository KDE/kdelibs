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

// Gregorian calendar system implementation factory for creation of kde
// calendar systems.
// Also default gregorian and factory classes

#include "kcalendarsystemfactory.h"

#include "kcalendarsystemgregorian.h"
#include "kcalendarsystemhijri.h"
#include "kcalendarsystemhebrew.h"
#include "kcalendarsystemjalali.h"
#include "kdebug.h"

#include <qstringlist.h>

KCalendarSystemFactory::KCalendarSystemFactory()
{
  kdDebug(5400) << "Created factory calendar" << endl;
}

KCalendarSystemFactory::~KCalendarSystemFactory()
{
}

KCalendarSystem *KCalendarSystemFactory::create( const QString &calType,
                                                 const KLocale * locale )
{
  if ( calType == "hebrew" )
    return new KCalendarSystemHebrew(locale);
  if ( calType == "hijri" )
    return new KCalendarSystemHijri(locale);
  if ( calType == "gregorian" )
    return new KCalendarSystemGregorian(locale);
  if ( calType == "jalali" )
    return new KCalendarSystemJalali(locale);

  kdDebug(5400) << "Calendar " << calType << " not found, defaulting to gregorian" << endl;

  // ### HPB: Should it really be a default here?
  return new KCalendarSystemGregorian(locale);
}

QStringList KCalendarSystemFactory::calendarSystems()
{
   QStringList lst;
   lst.append("hebrew");
   lst.append("hijri");
   lst.append("gregorian");
   lst.append("jalali");

   return lst;
}
