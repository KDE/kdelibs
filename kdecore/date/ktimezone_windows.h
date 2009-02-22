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

#include <ktimezone.h>
#include <windows.h>

namespace {
    class HKeyCloser {
        const HKEY hkey;
        Q_DISABLE_COPY( HKeyCloser )
    public:
        explicit HKeyCloser( HKEY hk ) : hkey( hk ) {}
        ~HKeyCloser() { RegCloseKey(  hkey ); }
    };

    struct TZI {
        LONG Bias;
        LONG StandardBias;
        LONG DaylightBias;
        SYSTEMTIME StandardDate;
        SYSTEMTIME DaylightDate;
    };
}



class KSystemTimeZoneDataWindows : public KTimeZoneData
{
public:
  KSystemTimeZoneDataWindows()
  :KTimeZoneData()
  {

  }
  TIME_ZONE_INFORMATION _tzi;
  QString displayName;

  const TIME_ZONE_INFORMATION & tzi( int year = 0 ) const { Q_UNUSED( year ); return _tzi; }
};

class KSystemTimeZoneSourceWindows : public KSystemTimeZoneSource
{
public:
  KSystemTimeZoneSourceWindows() {}
  KTimeZoneData* parse(const KTimeZone &zone) const;
};

class KSystemTimeZoneBackendWindows : public KTimeZoneBackend
{
public:
  KSystemTimeZoneBackendWindows(KTimeZoneSource *source, const QString &name)
  : KTimeZoneBackend(source, name) {}

  ~KSystemTimeZoneBackendWindows() {}

  KSystemTimeZoneBackendWindows *clone() const;

  QByteArray type() const;

  int offsetAtZoneTime(const KTimeZone *caller, const QDateTime &zoneDateTime, int *secondOffset) const;
  int offsetAtUtc(const KTimeZone *caller, const QDateTime &utcDateTime) const;
  int offset(const KTimeZone *caller, time_t t) const;
  bool isDstAtUtc(const KTimeZone *caller, const QDateTime &utcDateTime) const;
  bool isDst(const KTimeZone *caller, time_t t) const;
};

class KSystemTimeZoneWindows : public KTimeZone
{
public:
  KSystemTimeZoneWindows(KTimeZoneSource *source, const QString &name)
    : KTimeZone(new KSystemTimeZoneBackendWindows(source, name)) {}

  ~KSystemTimeZoneWindows() {}
};

#endif // _KTIMEZONE_WINDOWS_H

