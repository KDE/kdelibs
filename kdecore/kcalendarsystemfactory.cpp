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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// Gregorian calendar system implementation factory for creation of kde
// calendar systems.
// Also default gregorian and factory classes

#include <kdebug.h>

#include "kcalendarsystemfactory.h"

//#define HEBREW_CALENDAR_SUPPORT

#include "kcalendarsystemgregorian.h"
#include "kcalendarsystemhijri.h"
#ifdef HEBREW_CALENDAR_SUPPORT
#include "kcalendarsystemhebrew.h"
#endif

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
#ifdef HEBREW_CALENDAR_SUPPORT
  if ( calType == "hebrew" )
    return new KCalendarSystemHebrew(locale);
#endif
  if ( calType == "hijri" )
    return new KCalendarSystemHijri(locale);
  if ( calType == "gregorian" )
    return new KCalendarSystemGregorian(locale);

  kdDebug(5400) << "Calendar " << calType << " not found, defaulting to gregorian" << endl;

  // ### HPB: Should it really be a default here?
  return new KCalendarSystemGregorian(locale);
}

QStringList KCalendarSystemFactory::calendarSystems()
{
   QStringList lst;
#ifdef HEBREW_CALENDAR_SUPPORT
   lst.append("hebrew");
#endif
   lst.append("hijri");
   lst.append("gregorian");

   return lst;
}
