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


#include "ktimezone_win.h"
#include <config.h>

#include <kdebug.h>

#include <QStringList>
#include <QLibrary>

#include <windows.h>

#include <memory>
#include <string>
#include <cassert>

struct ZoneKey {
    QString zoneOlson;
    QString zoneWin;
};

static const ZoneKey ZoneTbl[] =
{
    {QLatin1String("Australia/Darwin"), QLatin1String("AUS Central Standard Time")},
    {QLatin1String("Australia/Sydney"), QLatin1String("AUS Eastern Standard Time")},
    {QLatin1String("Asia/Kabul"), QLatin1String("Afghanistan Standard Time")},
    {QLatin1String("America/Anchorage"), QLatin1String("Alaskan Standard Time")},
    {QLatin1String("Asia/Riyadh"), QLatin1String("Arab Standard Time")},
    {QLatin1String("Asia/Dubai"), QLatin1String("Arabian Standard Time")},
    {QLatin1String("Asia/Baghdad"), QLatin1String("Arabic Standard Time")},
    {QLatin1String("America/Buenos_Aires"), QLatin1String("Argentina Standard Time")},
    {QLatin1String("Asia/Yerevan"), QLatin1String("Armenian Standard Time")},
    {QLatin1String("America/Halifax"), QLatin1String("Atlantic Standard Time")},
    {QLatin1String("Asia/Baku"), QLatin1String("Azerbaijan Standard Time")},
    {QLatin1String("Atlantic/Azores"), QLatin1String("Azores Standard Time")},
    {QLatin1String("America/Regina"), QLatin1String("Canada Central Standard Time")},
    {QLatin1String("Atlantic/Cape_Verde"), QLatin1String("Cape Verde Standard Time")},
    {QLatin1String("Asia/Yerevan"), QLatin1String("Caucasus Standard Time")},
    {QLatin1String("Australia/Adelaide"), QLatin1String("Cen. Australia Standard Time")},
    {QLatin1String("America/Guatemala"), QLatin1String("Central America Standard Time")},
    {QLatin1String("Asia/Dhaka"), QLatin1String("Central Asia Standard Time")},
    {QLatin1String("America/Manaus"), QLatin1String("Central Brazilian Standard Time")},
    {QLatin1String("Europe/Budapest"), QLatin1String("Central Europe Standard Time")},
    {QLatin1String("Europe/Warsaw"), QLatin1String("Central European Standard Time")},
    {QLatin1String("Pacific/Guadalcanal"), QLatin1String("Central Pacific Standard Time")},
    {QLatin1String("America/Chicago"), QLatin1String("Central Standard Time")},
    {QLatin1String("America/Mexico_City"), QLatin1String("Central Standard Time (Mexico)")},
    {QLatin1String("Asia/Shanghai"), QLatin1String("China Standard Time")},
    {QLatin1String("Etc/GMT+12"), QLatin1String("Dateline Standard Time")},
    {QLatin1String("Africa/Nairobi"), QLatin1String("E. Africa Standard Time")},
    {QLatin1String("Australia/Brisbane"), QLatin1String("E. Australia Standard Time")},
    {QLatin1String("Europe/Minsk"), QLatin1String("E. Europe Standard Time")},
    {QLatin1String("America/Sao_Paulo"), QLatin1String("E. South America Standard Time")},
    {QLatin1String("America/New_York"), QLatin1String("Eastern Standard Time")},
    {QLatin1String("Africa/Cairo"), QLatin1String("Egypt Standard Time")},
    {QLatin1String("Asia/Yekaterinburg"), QLatin1String("Ekaterinburg Standard Time")},
    {QLatin1String("Europe/Kiev"), QLatin1String("FLE Standard Time")},
    {QLatin1String("Pacific/Fiji"), QLatin1String("Fiji Standard Time")},
    {QLatin1String("Europe/London"), QLatin1String("GMT Standard Time")},
    {QLatin1String("Europe/Istanbul"), QLatin1String("GTB Standard Time")},
    {QLatin1String("Etc/GMT-3"), QLatin1String("Georgian Standard Time")},
    {QLatin1String("America/Godthab"), QLatin1String("Greenland Standard Time")},
    {QLatin1String("Atlantic/Reykjavik"), QLatin1String("Greenwich Standard Time")},
    {QLatin1String("Pacific/Honolulu"), QLatin1String("Hawaiian Standard Time")},
    {QLatin1String("Asia/Calcutta"), QLatin1String("India Standard Time")},
    {QLatin1String("Asia/Tehran"), QLatin1String("Iran Standard Time")},
    {QLatin1String("Asia/Jerusalem"), QLatin1String("Israel Standard Time")},
    {QLatin1String("Asia/Amman"), QLatin1String("Jordan Standard Time")},
    {QLatin1String("Asia/Seoul"), QLatin1String("Korea Standard Time")},
    {QLatin1String("Indian/Mauritius"), QLatin1String("Mauritius Standard Time")},
    {QLatin1String("America/Mexico_City"), QLatin1String("Mexico Standard Time")},
    {QLatin1String("America/Chihuahua"), QLatin1String("Mexico Standard Time 2")},
    {QLatin1String("Atlantic/South_Georgia"), QLatin1String("Mid-Atlantic Standard Time")},
    {QLatin1String("Asia/Beirut"), QLatin1String("Middle East Standard Time")},
    {QLatin1String("America/Montevideo"), QLatin1String("Montevideo Standard Time")},
    {QLatin1String("Africa/Casablanca"), QLatin1String("Morocco Standard Time")},
    {QLatin1String("America/Denver"), QLatin1String("Mountain Standard Time")},
    {QLatin1String("America/Chihuahua"), QLatin1String("Mountain Standard Time (Mexico)")},
    {QLatin1String("Asia/Rangoon"), QLatin1String("Myanmar Standard Time")},
    {QLatin1String("Asia/Novosibirsk"), QLatin1String("N. Central Asia Standard Time")},
    {QLatin1String("Africa/Windhoek"), QLatin1String("Namibia Standard Time")},
    {QLatin1String("Asia/Katmandu"), QLatin1String("Nepal Standard Time")},
    {QLatin1String("Pacific/Auckland"), QLatin1String("New Zealand Standard Time")},
    {QLatin1String("America/St_Johns"), QLatin1String("Newfoundland Standard Time")},
    {QLatin1String("Asia/Irkutsk"), QLatin1String("North Asia East Standard Time")},
    {QLatin1String("Asia/Krasnoyarsk"), QLatin1String("North Asia Standard Time")},
    {QLatin1String("America/Santiago"), QLatin1String("Pacific SA Standard Time")},
    {QLatin1String("America/Los_Angeles"), QLatin1String("Pacific Standard Time")},
    {QLatin1String("America/Tijuana"), QLatin1String("Pacific Standard Time (Mexico)")},
    {QLatin1String("Asia/Karachi"), QLatin1String("Pakistan Standard Time")},
    {QLatin1String("Europe/Paris"), QLatin1String("Romance Standard Time")},
    {QLatin1String("Europe/Moscow"), QLatin1String("Russian Standard Time")},
    {QLatin1String("Etc/GMT+3"), QLatin1String("SA Eastern Standard Time")},
    {QLatin1String("America/Bogota"), QLatin1String("SA Pacific Standard Time")},
    {QLatin1String("America/La_Paz"), QLatin1String("SA Western Standard Time")},
    {QLatin1String("Asia/Bangkok"), QLatin1String("SE Asia Standard Time")},
    {QLatin1String("Pacific/Apia"), QLatin1String("Samoa Standard Time")},
    {QLatin1String("Asia/Singapore"), QLatin1String("Singapore Standard Time")},
    {QLatin1String("Africa/Johannesburg"), QLatin1String("South Africa Standard Time")},
    {QLatin1String("Asia/Colombo"), QLatin1String("Sri Lanka Standard Time")},
    {QLatin1String("Asia/Taipei"), QLatin1String("Taipei Standard Time")},
    {QLatin1String("Australia/Hobart"), QLatin1String("Tasmania Standard Time")},
    {QLatin1String("Asia/Tokyo"), QLatin1String("Tokyo Standard Time")},
    {QLatin1String("Pacific/Tongatapu"), QLatin1String("Tonga Standard Time")},
    {QLatin1String("Etc/GMT+5"), QLatin1String("US Eastern Standard Time")},
    {QLatin1String("America/Phoenix"), QLatin1String("US Mountain Standard Time")},
    {QLatin1String("America/Caracas"), QLatin1String("Venezuela Standard Time")},
    {QLatin1String("Asia/Vladivostok"), QLatin1String("Vladivostok Standard Time")},
    {QLatin1String("Australia/Perth"), QLatin1String("W. Australia Standard Time")},
    {QLatin1String("Africa/Lagos"), QLatin1String("W. Central Africa Standard Time")},
    {QLatin1String("Europe/Berlin"), QLatin1String("W. Europe Standard Time")},
    {QLatin1String("Asia/Tashkent"), QLatin1String("West Asia Standard Time")},
    {QLatin1String("Pacific/Port_Moresby"), QLatin1String("West Pacific Standard Time")},
    {QLatin1String("Asia/Yakutsk"), QLatin1String("Yakutsk Standard Time")}
};

static QString getWinZoneName(const QString &name)
{
    for ( int i = 0; i < sizeof(ZoneTbl) / sizeof(ZoneTbl[0]); ++i ) {
        if (ZoneTbl[i].zoneOlson == name) {
            return ZoneTbl[i].zoneWin;
        }
    }
    
    return name;
}

typedef BOOL (WINAPI *PtrTzSpecificLocalTimeToSystemTime )(LPTIME_ZONE_INFORMATION lpTimeZoneInformation,
                                                           LPSYSTEMTIME lpLocalTime,
                                                           LPSYSTEMTIME lpUniversalTime
);
static PtrTzSpecificLocalTimeToSystemTime pTzSpecificLocalTimeToSystemTime = 0;

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

// TCHAR can be either uchar, or wchar_t:
#ifdef UNICODE

static inline QString tchar_to_qstring( const TCHAR * str ) {
    return QString::fromUtf16( reinterpret_cast<const ushort*>( str ) );
}

static inline const TCHAR * qstring_to_tchar( const QString& str ) {
    return reinterpret_cast<const TCHAR*>( str.utf16() );
}

static inline std::basic_string<TCHAR> qstring_to_tcharstring( const QString& str ) {
    return std::basic_string<TCHAR>( qstring_to_tchar(str) );
}

#else

static inline QString tchar_to_qstring( const TCHAR * str ) {
    return QString::fromLocal8Bit( str );
}

static inline const TCHAR * qstring_to_tchar( const QString& str ) {
    return str.toLocal8Bit().constData();
}

static inline std::basic_string<TCHAR> qstring_to_tcharstring( const QString& str ) {
    return std::basic_string<TCHAR>( qstring_to_tchar(str) );
}

#endif

static const TCHAR timeZonesKey[] = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones");
static inline QDateTime systemtime_to_qdatetime( const SYSTEMTIME & st ) {
    return QDateTime( QDate( st.wYear, st.wMonth, st.wDay ),
                      QTime( st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ) );
}

static SYSTEMTIME qdatetime_to_systemtime( const QDateTime & dt ) {
    const QDate d = dt.date();
    const QTime t = dt.time();
    const SYSTEMTIME st = {
        d.year(),
        d.month(),
        d.dayOfWeek() % 7, // 1..7 (Mon..Sun)->0..6(Sun..Sat)
        d.day(),
        t.hour(),
        t.minute(),
        t.second(),
        t.msec(),
    };
    return st;
}

static bool TzSpecificLocalTimeToSystemTime_Portable( TIME_ZONE_INFORMATION* tz,
                                                      SYSTEMTIME *i_stLocal,
                                                      SYSTEMTIME *o_stUniversal )
{

    // the method below was introduced in XP. If it's there, use it, otherwise
    // fall back to doing things manually
    if (!pTzSpecificLocalTimeToSystemTime) {
        QLibrary kernelLib(QLatin1String("kernel32"));
        pTzSpecificLocalTimeToSystemTime  = (PtrTzSpecificLocalTimeToSystemTime)kernelLib.resolve("TzSpecificLocalTimeToSystemTime");
    }

    if ( pTzSpecificLocalTimeToSystemTime )
        return pTzSpecificLocalTimeToSystemTime( tz, i_stLocal , o_stUniversal ) != 0;
    
    // the algorithm is:
    // - switch to the desired timezone temporarily
    // - convert system time to (local) file time in that timezone
    // - convert local file time to utc file time
    // - convert utc file time to system time
    // - reset timezone
    FILETIME ft, ft_utc;
    int result = 1;
    TIME_ZONE_INFORMATION currentTimeZone;
    result = GetTimeZoneInformation(&currentTimeZone);
    if ( result == TIME_ZONE_ID_INVALID ) {
        kWarning(161) << "Getting time zone information failed";
        return false;
    }
    result = SetTimeZoneInformation(tz);
    if ( result == 0 ) {
        kWarning(161) << "Setting temporary time zone failed";
        return false;
    }
    result = SystemTimeToFileTime(i_stLocal, &ft);
    if ( result == 0 ) {
        kWarning(161) << "SysteTimeToFileTime failed";
        return false;
    }
    result = LocalFileTimeToFileTime(&ft, &ft_utc);
    if ( result == 0 ) {
        kWarning(161) << "LocalFileTimeToFileTime failed";
        return false;
    }
    result = FileTimeToSystemTime(&ft_utc,o_stUniversal);
    if ( result == 0 ) {
        kWarning(161) << "FileTimeToSystemTime failed";
        return false;
    }
    result = SetTimeZoneInformation(&currentTimeZone);
    if ( result == 0 ) {
        kWarning(161) << "Re-setting time zone information failed";
        return false;
    }
    return true;
}




static bool get_binary_value( HKEY key, const TCHAR * value, void * data, DWORD numData, DWORD * outNumData=0 ) {
    DWORD size = numData;
    DWORD type = REG_BINARY;
    if ( RegQueryValueEx( key, value, 0, &type, (LPBYTE)data, &size ) != ERROR_SUCCESS )
        return false;
    assert( type == REG_BINARY );
    if (  type != REG_BINARY )
        return false;
    if ( outNumData )
        *outNumData = size;
    return true;
}

static bool get_string_value( HKEY key, const TCHAR * value, TCHAR * dest, DWORD destSizeInBytes ) {
    DWORD size = destSizeInBytes;
    DWORD type = REG_SZ;
    dest[0] = '\0';
    if ( RegQueryValueEx( key, value, 0, &type, (LPBYTE)dest, &size ) != ERROR_SUCCESS )
        return false;
    //dest[ qMin( size, destSizeInBytes - sizeof( WCHAR ) ) / sizeof( WCHAR ) ] = 0;
    assert( type == REG_SZ );
    if ( type != REG_SZ )
        return false;
    return true;
}

//
//
// Backend interface impl:
//
//

static bool check_prereq( const KTimeZone * caller, const QDateTime & dt, Qt::TimeSpec spec ) {
    return caller && caller->isValid() && dt.isValid() && dt.timeSpec() == spec ;
}

static inline bool check_local( const KTimeZone * caller, const QDateTime & dt ) {
    return check_prereq( caller, dt, Qt::LocalTime );
}

static inline bool check_utc( const KTimeZone * caller, const QDateTime & dt ) {
    return check_prereq( caller, dt, Qt::UTC );
}

static bool has_transition( const TIME_ZONE_INFORMATION & tz ) {
    return tz.StandardDate.wMonth != 0 && tz.DaylightDate.wMonth != 0 ;
}

static int win_dayofweek_to_qt_dayofweek( int wdow ) {
    // Sun(0)..Sat(6) -> Mon(1)...Sun(7)
    return wdow ? wdow : 7 ;
}

static int qt_dayofweek_to_win_dayofweek( int qdow ) {
    // Mon(1)...Sun(7) -> Sub(0)...Sat(6)
    return qdow % 7;
}

static QDate find_nth_weekday_in_month_of_year( int nth, int dayOfWeek, int month, int year ) {
    assert( nth >= 1 );
    assert( nth <= 5 );

    const QDate first( year, month, 1 );
    const int actualDayOfWeek = first.dayOfWeek();
    QDate candidate = first.addDays( ( nth - 1 ) * 7 + dayOfWeek - actualDayOfWeek );
    assert( candidate.dayOfWeek() == dayOfWeek );
    if ( nth == 5 )
        if ( candidate.month() != month )
            candidate = candidate.addDays( -7 );
    assert( candidate.month() == month );
    return candidate;
}

static QDateTime transition( const SYSTEMTIME & st, int year ) {
    assert( st.wYear == 0 );
    assert( st.wMonth != 0 );
    return QDateTime( find_nth_weekday_in_month_of_year( st.wDay, win_dayofweek_to_qt_dayofweek( st.wDayOfWeek ), st.wMonth, year ),
                      QTime( st.wHour, st.wMinute, st.wSecond, st.wMilliseconds ) );
}

struct Transitions {
    QDateTime stdStart, dstStart;
};

Transitions transitions( const TIME_ZONE_INFORMATION & tz, int year ) {
    const Transitions t = {
        transition( tz.StandardDate, year ), transition( tz.DaylightDate, year )
    };
    return t;
}


static const int MAX_KEY_LENGTH = 255;

static QStringList list_key( HKEY key ) {

    DWORD numSubKeys = 0;
    QStringList result;

    if ( RegQueryInfoKey( key, 0, 0, 0, &numSubKeys, 0, 0, 0, 0, 0, 0, 0 ) == ERROR_SUCCESS )
        for ( DWORD i = 0 ; i < numSubKeys ; ++i ) {
            TCHAR name[MAX_KEY_LENGTH+1];
            DWORD nameLen = MAX_KEY_LENGTH;
            if ( RegEnumKeyEx( key, i, name, &nameLen, 0, 0, 0, 0 ) == ERROR_SUCCESS )
                result.push_back( tchar_to_qstring( name ) );
        }

    return result;
}

static QStringList list_standard_names()
{
    QStringList standardNames;
    
    HKEY timeZones;
    QStringList keys;
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, timeZonesKey, 0, KEY_READ, &timeZones ) == ERROR_SUCCESS )
        keys = list_key(timeZones);

    std::basic_string<TCHAR> path( timeZonesKey );
    path += TEXT( "\\" );

    const HKeyCloser closer( timeZones );
    Q_FOREACH( const QString & keyname, keys ) {
    
        std::basic_string<TCHAR> keypath(path);
        keypath += qstring_to_tcharstring(keyname);
    HKEY key;
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, keypath.c_str(), 0, KEY_READ, &key ) != ERROR_SUCCESS ) {
        return standardNames; // FIXME what's the right error handling here?
    }

    const HKeyCloser closer( key );

    TIME_ZONE_INFORMATION tz;
    get_string_value( key, L"Std", tz.StandardName, sizeof( tz.StandardName ) );

    standardNames << tchar_to_qstring(tz.StandardName);
    }
    
    for ( int i = 0; i < sizeof(ZoneTbl) / sizeof(ZoneTbl[0]); ++i ) {
        standardNames << ZoneTbl[i].zoneOlson;
    }

    return standardNames;
}

static std::basic_string<TCHAR> pathFromZoneName(const KTimeZone& zone)
{
    std::basic_string<TCHAR> path( timeZonesKey );
    path += TEXT( "\\" );
    
    QString name = zone.name();

    name = getWinZoneName(name);

    HKEY timeZones;
    QStringList keys;
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, timeZonesKey, 0, KEY_READ, &timeZones ) == ERROR_SUCCESS )
        keys = list_key(timeZones);

    const HKeyCloser closer( timeZones );
    Q_FOREACH( const QString & keyname, keys ) {
    
        std::basic_string<TCHAR> keypath(path);
        keypath += qstring_to_tcharstring(keyname);
    HKEY key;
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, keypath.c_str(), 0, KEY_READ, &key ) != ERROR_SUCCESS ) {
        return 0; // FIXME what's the right error handling here?
    }

    const HKeyCloser closer( key );

    TIME_ZONE_INFORMATION tz;
    get_string_value( key, L"Std", tz.StandardName, sizeof( tz.StandardName ) );

    if ( tchar_to_qstring(tz.StandardName) == name ) {
        return keypath;
    }
    }
    Q_ASSERT(false);

    return path;
}

/******************************************************************************/

class KSystemTimeZoneSourceWindowsPrivate
{
public:
    KSystemTimeZoneSourceWindowsPrivate() {}
    ~KSystemTimeZoneSourceWindowsPrivate() {}
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

KSystemTimeZoneSourceWindows::KSystemTimeZoneSourceWindows()
:d( new KSystemTimeZoneSourceWindowsPrivate )
{
}

KTimeZoneData* KSystemTimeZoneSourceWindows::parse(const KTimeZone &zone) const
{
    KSystemTimeZoneDataWindows* data = new KSystemTimeZoneDataWindows();

    std::basic_string<TCHAR> path = pathFromZoneName(zone);

    HKEY key;
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_READ, &key ) != ERROR_SUCCESS ) {
        delete data;
        return 0; // FIXME what's the right error handling here?
    }

    const HKeyCloser closer( key );

    TZI tzi = { 0 };

    if ( !get_binary_value( key, TEXT( "TZI" ), &tzi, sizeof( TZI ) ) ) {
        delete data;
        return 0; // ?
    }

    get_string_value( key, L"Std", data->_tzi.StandardName, sizeof( data->_tzi.StandardName ) );
    get_string_value( key, L"Dlt", data->_tzi.DaylightName, sizeof( data->_tzi.DaylightName ) );

    TCHAR display[512];
    get_string_value( key, L"Display", display, sizeof( display ) );
    data->displayName = tchar_to_qstring( display );

#define COPY( name ) data->_tzi.name = tzi.name
    COPY( Bias );
    COPY( StandardBias );
    COPY( StandardDate );
    COPY( DaylightBias );
    COPY( DaylightDate );
#undef COPY

    return data;
}

Transitions transitions( const KTimeZone * caller, int year ) {
    return transitions( static_cast<const KSystemTimeZoneDataWindows*>( caller->data(true) )->tzi( year ), year );
}

static bool is_dst( const TIME_ZONE_INFORMATION & tzi, const QDateTime & utc, int year ) {
    if ( !has_transition( tzi ) )
        return false;
    const Transitions trans = transitions( tzi, year );
    if ( trans.stdStart < trans.dstStart )
        return trans.dstStart <= utc || utc < trans.stdStart ;
    else
        return trans.dstStart <= utc && utc < trans.stdStart ;
}

static bool is_dst( const KTimeZone * caller, const QDateTime & utc ) {
    assert( caller );
    assert( caller->isValid() );
    const int year = utc.date().year();
    const TIME_ZONE_INFORMATION & tzi = static_cast<const KSystemTimeZoneDataWindows*>( caller->data(true) )->tzi( year );
    return is_dst( tzi, utc, year );
}

static int effective_offset( const TIME_ZONE_INFORMATION& tz, bool isDst ) {
    int bias = tz.Bias;
    if ( has_transition( tz ) )
        if ( isDst )
            bias += tz.DaylightBias;
        else
            bias += tz.StandardBias;
    return bias * -60; // min -> secs
}

static int offset_at_utc( const KTimeZone * caller, const QDateTime & utc ) {
    assert( caller );
    assert( caller->isValid() );
    const int year = utc.date().year();
    const TIME_ZONE_INFORMATION & tz = static_cast<const KSystemTimeZoneDataWindows*>( caller->data(true) )->tzi( year );
    return effective_offset( tz, is_dst( tz, utc, year ) );
}

static const int OneHour = 3600; //sec

static int difference( const SYSTEMTIME & st1, const SYSTEMTIME & st2 ) {
    return systemtime_to_qdatetime( st1 ).secsTo( systemtime_to_qdatetime( st2 ) );
}

static int offset_at_zone_time( const KTimeZone * caller, const SYSTEMTIME & zone, int * secondOffset ) {
    assert( caller );
    assert( caller->isValid() );
    assert(caller->data(true));
    const KSystemTimeZoneDataWindows * const data = static_cast<const KSystemTimeZoneDataWindows*>( caller->data(true) );
    const TIME_ZONE_INFORMATION & tz = data->tzi( zone.wYear );
    SYSTEMTIME utc;
    if ( !TzSpecificLocalTimeToSystemTime_Portable( const_cast<LPTIME_ZONE_INFORMATION>( &tz ), const_cast<LPSYSTEMTIME>( &zone ), &utc ) )
        return 0;
    const bool isDst = is_dst( tz, systemtime_to_qdatetime( utc ), utc.wYear );
    int result = effective_offset( tz, isDst );
//FIXME: SystemTimeToTzSpecificLocalTime does not exsit on wince
#ifndef _WIN32_WCE
    if ( secondOffset ) {
        const SYSTEMTIME utcplus1 = qdatetime_to_systemtime( systemtime_to_qdatetime( utc ).addSecs( OneHour ) );
        const SYSTEMTIME utcminus1 = qdatetime_to_systemtime( systemtime_to_qdatetime( utc ).addSecs( -OneHour ) );
        SYSTEMTIME zoneplus1, zoneminus1;
        if ( !SystemTimeToTzSpecificLocalTime( const_cast<LPTIME_ZONE_INFORMATION>( &tz ), const_cast<LPSYSTEMTIME>( &utcplus1 ), &zoneplus1 ) ||
             !SystemTimeToTzSpecificLocalTime( const_cast<LPTIME_ZONE_INFORMATION>( &tz ), const_cast<LPSYSTEMTIME>( &utcminus1 ), &zoneminus1 ) )
            return result;
        if ( difference( zoneminus1, zone ) != OneHour ||
             difference( zone, zoneplus1 ) != OneHour )
        {
            *secondOffset = effective_offset( tz, !isDst );
            if ( result < *secondOffset )
                qSwap( result, *secondOffset );
        }
    }
#endif
    return result;
}



KSystemTimeZoneBackendWindows * KSystemTimeZoneBackendWindows::clone() const
{
    return new KSystemTimeZoneBackendWindows(*this);
}

QByteArray KSystemTimeZoneBackendWindows::type() const
{
    return "KSystemTimeZoneWindows";
}

int KSystemTimeZoneBackendWindows::offsetAtZoneTime(const KTimeZone *caller, const QDateTime &zoneDateTime, int *secondOffset) const
{
    if (!caller->isValid()  ||  !zoneDateTime.isValid()  ||  zoneDateTime.timeSpec() != Qt::LocalTime)
        return 0;
    if ( !check_local( caller, zoneDateTime ) )
        return 0;

    return offset_at_zone_time( caller, qdatetime_to_systemtime( zoneDateTime ), secondOffset );
}

int KSystemTimeZoneBackendWindows::offsetAtUtc(const KTimeZone *caller, const QDateTime &utcDateTime) const
{
    if (!caller->isValid()  ||  !utcDateTime.isValid())
        return 0;
    if ( !check_utc( caller, utcDateTime ) )
        return 0;
    return offset_at_utc( caller, utcDateTime );
}

int KSystemTimeZoneBackendWindows::offset(const KTimeZone *caller, time_t t) const
{
    if (!caller->isValid()  ||  t == KTimeZone::InvalidTime_t)
        return 0;
    return offsetAtUtc( caller, KTimeZone::fromTime_t( t ) );
}

bool KSystemTimeZoneBackendWindows::isDstAtUtc(const KTimeZone *caller, const QDateTime &utcDateTime) const
{
    return check_utc( caller, utcDateTime ) && is_dst( caller, utcDateTime );
}


bool KSystemTimeZoneBackendWindows::isDst(const KTimeZone *caller, time_t t) const
{
    return isDstAtUtc( caller, KTimeZone::fromTime_t( t ) );
}

KSystemTimeZoneWindows::KSystemTimeZoneWindows(KTimeZoneSource *source, const QString &name)
: KTimeZone(new KSystemTimeZoneBackendWindows(source, name))
{}

QStringList KSystemTimeZoneWindows::listTimeZones() 
{
    return list_standard_names();
}

