/*
   This file is part of the KDE libraries
   Copyright (c) 2005 S.R.Haque <srhaque@iee.org>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "ktimezones.h"
#include "kdebug.h"
#include "kprocess.h"
#include "kstringhandler.h"
#include "ktempfile.h"

#include <qdatetime.h>
#include <qfile.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qtextstream.h>

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <ctime>

const float KTimezone::UNKNOWN = 1000.0;

KTimezone::KTimezone(const QString& name, const QString& countryCode, float latitude, float longitude, const QString& comment) :
    m_name(name),
    m_countryCode(countryCode),
    m_latitude(latitude),
    m_longitude(longitude),
    m_comment(comment),
    d(0)
{
    // Detect duff values.
    if (m_latitude * m_latitude > 90 * 90)
        m_latitude = UNKNOWN;
    if (m_longitude * m_longitude > 180 * 180)
        m_longitude = UNKNOWN;
}

QString KTimezone::comment() const
{
    return m_comment;
}

QString KTimezone::name() const
{
    return m_name;
}

QString KTimezone::countryCode() const
{
    return m_countryCode;
}

float KTimezone::latitude() const
{
    return m_latitude;
}

float KTimezone::longitude() const
{
    return m_longitude;
}

int KTimezone::offset(Qt::TimeSpec basisSpec) const
{
    char *originalZone = ::getenv("TZ");
    QDateTime localTime = QDateTime::currentDateTime(basisSpec);

    // Set the given timezone and find out what time it is there and GMT.
    ::putenv(strdup(QString("TZ=:").append(m_name).latin1()));
    tzset();
    QDateTime remoteTime = QDateTime::currentDateTime(Qt::LocalTime);
    int offset = remoteTime.secsTo(localTime);

    // Now restore things
    if (!originalZone)
    {
        ::unsetenv("TZ");
    }
    else
    {
        ::putenv(strdup(QString("TZ=").append(originalZone).latin1()));
    }
    tzset();
    return offset;
}

KTimezones::KTimezones() :
    m_zoneinfoDir(),
    m_zones(0),
    d(0)
{
}

KTimezones::~KTimezones()
{
    // FIXME when needed:
    // delete d;

    // autodelete behavior
    if ( m_zones )
        for( ZoneMap::ConstIterator it = m_zones->begin(); it != m_zones->end(); ++it )
            delete it.data();
    delete m_zones;
}

const KTimezones::ZoneMap KTimezones::allZones()
{
    // Have we already done all the hard work? If not, create the cache.
    if (m_zones)
        return *m_zones;
    m_zones = new ZoneMap();

    // Go read the database.
    //
    // On Windows, HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Time Zones
    // is the place to look. The TZI binary value is the TIME_ZONE_INFORMATION structure.
    //
    // For Unix its all easy except knowing where to look. Try the LSB location first.
    QFile f;
    m_zoneinfoDir = "/usr/share/zoneinfo";
    f.setName(m_zoneinfoDir + "/zone.tab");
    if (!f.open(IO_ReadOnly))
    {
        kdDebug() << "Can't open " << f.name() << endl;
        m_zoneinfoDir = "/usr/lib/zoneinfo";
        f.setName(m_zoneinfoDir + "/zone.tab");
        if (!f.open(IO_ReadOnly))
        {
            kdDebug() << "Can't open " << f.name() << endl;
            m_zoneinfoDir = ::getenv("TZDIR");
            f.setName(m_zoneinfoDir + "/zone.tab");
            if (m_zoneinfoDir.isEmpty() || !f.open(IO_ReadOnly))
            {
                kdDebug() << "Can't open " << f.name() << endl;

                // Solaris support. Synthesise something that looks like a zone.tab.
                //
                // grep -h ^Zone /usr/share/lib/zoneinfo/src/* | awk '{print "??\t+9999+99999\t" $2}'
                //
                // where the country code is set to "??" and the lattitude/longitude
                // values are dummies.
                m_zoneinfoDir = "/usr/share/lib/zoneinfo";
                KTempFile temp;
                KShellProcess *reader = new KShellProcess();
                *reader << "grep" << "-h" << "^Zone" << m_zoneinfoDir << "/src/*" << temp.name() << "|" <<
                    "awk" << "'{print \"??\\t+9999+99999\\t\" $2}'";
                // Note the use of blocking here...it is a trivial amount of data!
                temp.close();
                reader->start(KProcess::Block);
                f.setName(temp.name());
                if (!temp.status() || !f.open(IO_ReadOnly))
                {
                    kdDebug() << "Can't open " << f.name() << endl;
                    return *m_zones;
                }
            }
        }
    }

    // Parse the zone.tab.
    QTextStream str(&f);
    QRegExp lineSeparator("[ \t]");
    QRegExp ordinateSeparator("[+-]");
    while (!str.atEnd())
    {
        QString line = str.readLine();
        if (line.isEmpty() || '#' == line[0])
            continue;
        QStringList tokens = KStringHandler::perlSplit(lineSeparator, line, 4);
        if (tokens.count() < 3)
        {
            kdError() << "invalid record: " << line << endl;
            continue;
        }

        // Got three tokens. Now check for two ordinates plus first one is "".
        QStringList ordinates = KStringHandler::perlSplit(ordinateSeparator, tokens[1], 2);
        if (ordinates.count() < 2)
        {
            kdError() << "invalid coordinates: " << tokens[1] << endl;
            continue;
        }

        float latitude = convertOrdinate(ordinates[1]);
        float longitude = convertOrdinate(ordinates[2]);

        // Add entry to list.
        KTimezone *timezone = new KTimezone(tokens[2], tokens[0], latitude, longitude, tokens[3]);
        m_zones->insert(tokens[2], timezone);
    }
    return *m_zones;
}

/**
 * Convert sHHMM or sHHMMSS to a floating point number of degrees.
 */
float KTimezones::convertOrdinate(const QString& ordinate)
{
    int value = ordinate.toInt();
    int degrees = 0;
    int minutes = 0;
    int seconds = 0;

    if (ordinate.length() > 11)
    {
        degrees = value / 10000;
        value -= degrees * 10000;
        minutes = value / 100;
        value -= minutes * 100;
        seconds = value;
    }
    else
    {
        degrees = value / 100;
        value -= degrees * 100;
        minutes = value;
    }
    value = degrees * 3600 + minutes * 60 + seconds;
    return value / 3600.0;
}

const KTimezone *KTimezones::local()
{
    // First try the simplest solution of checking for well-formed TZ setting.
    char *originalZone = ::getenv("TZ");
    const KTimezone *local = 0;
    if (originalZone)
    {
        if (originalZone[0] == ':')
        {
            originalZone++;
        }
        local = zone(originalZone);
        if (local)
            return local;
    }

    // FIXME. Is there a way to use /etc/localtime? What if it is a copy of the
    // zoneinfo file instead of a link?

    // Try to find a matching timezone abbreviation...that way, we'll
    // try to return a value in the user's own country.
    allZones();
    if (!m_zoneinfoDir.isEmpty())
    {
        tzset();
        QString stdZone = tzname[0];
        QString dstZone = tzname[1];
        for( ZoneMap::Iterator it = m_zones->begin(); it != m_zones->end(); ++it )
        {
            const KTimezone *zone = it.data();
            if (matchAbbreviations(m_zoneinfoDir + '/' + zone->name(), stdZone, dstZone))
            {
                // kdError() << "local=" << zone->name() << endl;
                local = zone;
                break;
            }
        }
    }
    return local;
}

// Parse a zoneinfo binary, and see if the abbreviations it contains match
// the pair of Standard and DaylightSavings abbreviations passed in.
bool KTimezones::matchAbbreviations(const QString& zoneFile, const QString& stdZone, const QString& dstZone)
{
    QFile f(zoneFile);
    if (!f.open(IO_ReadOnly))
    {
        kdError() << "Cannot open " << zoneFile << endl;
        return false;
    }

    // Structures that represent the zoneinfo file.
    Q_UINT8 T, z, i_, f_;
    struct
    {
        Q_UINT32 ttisgmtcnt;
        Q_UINT32 ttisstdcnt;
        Q_UINT32 leapcnt;
        Q_UINT32 timecnt;
        Q_UINT32 typecnt;
        Q_UINT32 charcnt;
    } tzh;
    Q_UINT32 transitionTime;
    Q_UINT8 localTimeIndex;
    struct
    {
        Q_INT32 gmtoff;
        Q_INT8 isdst;
        Q_UINT8 abbrind;
    } tt;
    Q_UINT32 leapTime;
    Q_UINT32 leapSeconds;
    Q_UINT8 isStandard;
    Q_UINT8 isUTC;
    char *abbr;

    bool foundStd = false;
    bool foundDst = dstZone.isEmpty();
    QDataStream str(&f);
    str >> T >> z >> i_ >> f_;
    // kdError() << "signature: " << QChar(T) << QChar(z) << QChar(i_) << QChar(f_) << endl;
    unsigned i;
    for (i = 0; i < 4; i++)
        str >> tzh.ttisgmtcnt;
    str >> tzh.ttisgmtcnt >> tzh.ttisstdcnt >> tzh.leapcnt >> tzh.timecnt >> tzh.typecnt >> tzh.charcnt;
    // kdError() << "header: " << tzh.ttisgmtcnt << ", " << tzh.ttisstdcnt << ", " << tzh.leapcnt << ", " <<
    //    tzh.timecnt << ", " << tzh.typecnt << ", " << tzh.charcnt << endl;
    for (i = 0; i < tzh.timecnt; i++)
        str >> transitionTime;
    for (i = 0; i < tzh.timecnt; i++)
    {
        // NB: these appear to be 1-based, not zero-based!
        str >> localTimeIndex;
    }
    for (i = 0; i < tzh.typecnt; i++)
    {
        str >> tt.gmtoff >> tt.isdst >> tt.abbrind;
        // kdError() << "local type: " << tt.gmtoff << ", " << tt.isdst << ", " << tt.abbrind << endl;
    }
    char abbrs[tzh.charcnt];
    str.readRawBytes(abbrs, tzh.charcnt);
    abbr = abbrs;
    while (abbr < abbrs + tzh.charcnt)
    {
        // kdError() << "abbr: " << abbr << endl;
        if (stdZone == abbr)
        {
            foundStd = true;
        }
        if (dstZone == abbr)
        {
            foundDst = true;
        }
        abbr += strlen(abbr) + 1;
    }
    if (foundStd && foundDst)
    {
        return true;
    }
    for (i = 0; i < tzh.leapcnt; i++)
    {
        str >> leapTime >> leapSeconds;
        // kdError() << "leap entry: " << leapTime << ", " << leapSeconds << endl;
    }
    for (i = 0; i < tzh.ttisstdcnt; i++)
    {
        str >> isStandard;
        // kdError() << "standard: " << isStandard << endl;
    }
    for (i = 0; i < tzh.ttisgmtcnt; i++)
    {
        str >> isUTC;
        // kdError() << "UTC: " << isUTC << endl;
    }
    return false;
}

const KTimezone *KTimezones::zone(const QString &name)
{
    allZones();
    ZoneMap::ConstIterator it = m_zones->find( name );
    if ( it != m_zones->end() )
        return it.data();

    // Error.
    return 0;
}
