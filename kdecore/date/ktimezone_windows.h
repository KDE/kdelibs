/*
   This file is part of the KDE libraries
   Copyright (c) 2008 Marc Mutz <mutz@kde.org>, Till Adam <adam@kde.org>

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

#ifndef _KTIMEZONE_WINDOWS_H
#define _KTIMEZONE_WINDOWS_H

#include <kdecore_export.h>

#include <ktimezone.h>
#include <ksystemtimezone.h>
#include <windows.h>

class KDECORE_EXPORT KSystemTimeZoneSourceWindows : public KSystemTimeZoneSource
{
public:
  KSystemTimeZoneSourceWindows() {}
  KTimeZoneData* parse(const KTimeZone &zone) const;
};


class KDECORE_EXPORT KSystemTimeZoneWindows : public KTimeZone
{
public:
  KSystemTimeZoneWindows(KTimeZoneSource *source, const QString &name);
    
  ~KSystemTimeZoneWindows() {}
  static QStringList listTimeZones();
};

#endif // _KTIMEZONE_WINDOWS_H

