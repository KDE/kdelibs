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

// Gregorian calendar system implementation factory for creation of kde calendar
// systems.
// Also default gregorian and factory classes

#include <kglobal.h>

#include "kcalendarsystem.h"

class KCalendarSystemPrivate
{
public:
  const KLocale * locale;
};

KCalendarSystem::KCalendarSystem(const KLocale * locale)
  : d(new KCalendarSystemPrivate)
{
  d->locale = locale;
}

KCalendarSystem::~KCalendarSystem()
{
  delete d;
}

const KLocale * KCalendarSystem::locale() const
{
  if ( d->locale )
    return d->locale;

  return KGlobal::locale();
}
