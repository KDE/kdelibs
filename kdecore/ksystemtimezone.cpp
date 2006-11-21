/*
   This file is part of the KDE libraries
   Copyright (c) 2005,2006 David Jarvie <software@astrojar.org.uk>
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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// This file requires HAVE_STRUCT_TM_TM_ZONE to be defined if struct tm member tm_zone is available.
// This file requires HAVE_TM_GMTOFF to be defined if struct tm member tm_gmtoff is available.

#include <config.h>

#include <climits>
#include <cstdlib>

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QRegExp>
#include <QStringList>
#include <QTextStream>

#include <kcodecs.h>
#include <kstringhandler.h>
#include <ktemporaryfile.h>
#include <kdebug.h>
#include <ksystemtimezone.h>
#include <ktzfiletimezone.h>


/* Return the offset to UTC in the current time zone at the specified UTC time.
 * The thread-safe function localtime_r() is used in preference if available.
 */
static int gmtoff(time_t t)
{
#ifdef _POSIX_THREAD_SAFE_FUNCTIONS
    tm tmtime;
    if (!localtime_r(&t, &tmtime))
        return 0;
#ifdef HAVE_TM_GMTOFF
    return tmtime.tm_gmtoff;
#else
    int lwday = tmtime.tm_wday;
    int lt = 3600*tmtime.tm_hour + 60*tmtime.tm_min + tmtime.tm_sec;
    if (!gmtime_r(&t, &tmtime))
        return 0;
    int uwday = tmtime.tm_wday;
    int ut = 3600*tmtime.tm_hour + 60*tmtime.tm_min + tmtime.tm_sec;
#endif
#else
    tm *tmtime = localtime(&t);
    if (!tmtime)
        return 0;
#ifdef HAVE_TM_GMTOFF
    return tmtime->tm_gmtoff;
#else
    int lwday = tmtime->tm_wday;
    int lt = 3600*tmtime->tm_hour + 60*tmtime->tm_min + tmtime->tm_sec;
    tmtime = gmtime(&t);
    int uwday = tmtime->tm_wday;
    int ut = 3600*tmtime->tm_hour + 60*tmtime->tm_min + tmtime->tm_sec;
#endif
#endif
#ifndef HAVE_TM_GMTOFF
    if (lwday != uwday)
    {
      // Adjust for different day
      if (lwday == uwday + 1  ||  lwday == 0 && uwday == 6)
        lt += 24*3600;
      else
        lt -= 24*3600;
    }
    return lt - ut;
#endif
}


/******************************************************************************/

class KSystemTimeZonesPrivate : public KTimeZones
{
public:
    static KSystemTimeZonesPrivate *instance();
    const KTimeZone *local();
    static QString zoneinfoDir()   { instance();  return m_zoneinfoDir; }
    static KTimeZone *readZone(const QString &name);

private:
    KSystemTimeZonesPrivate() {}
    bool findZoneTab( QFile& f );
    void readZoneTab();
    static QString calcChecksum(const QString &zoneName, qlonglong size);
    static float convertCoordinate(const QString &coordinate);

    static KSystemTimeZonesPrivate *m_instance;
    static KSystemTimeZoneSource *m_source;
    static KTzfileTimeZoneSource *m_tzfileSource;
    static QString m_zoneinfoDir;
    typedef QMap<QString, QString> MD5Map;    // zone name, checksum
    static MD5Map  m_md5Sums;
};

KSystemTimeZonesPrivate         *KSystemTimeZonesPrivate::m_instance = 0;
KSystemTimeZoneSource           *KSystemTimeZonesPrivate::m_source = 0;
KTzfileTimeZoneSource           *KSystemTimeZonesPrivate::m_tzfileSource = 0;
QString                          KSystemTimeZonesPrivate::m_zoneinfoDir;
KSystemTimeZonesPrivate::MD5Map  KSystemTimeZonesPrivate::m_md5Sums;


const KTimeZone *KSystemTimeZones::local()
{
    return KSystemTimeZonesPrivate::instance()->local();
}

QString KSystemTimeZones::zoneinfoDir()
{
    return KSystemTimeZonesPrivate::zoneinfoDir();
}

KTimeZones *KSystemTimeZones::timeZones()
{
    return KSystemTimeZonesPrivate::instance();
}

KTimeZone *KSystemTimeZones::readZone(const QString &name)
{
    return KSystemTimeZonesPrivate::readZone(name);
}

KTimeZone *KSystemTimeZonesPrivate::readZone(const QString &name)
{
    if (!m_tzfileSource)
        m_tzfileSource = new KTzfileTimeZoneSource(zoneinfoDir());
    return new KTzfileTimeZone(m_tzfileSource, name);
}

/*
 * Initialisation can be very calculation intensive, so ensure that only one
 * instance is ever constructed by making the constructor private.
 */
KSystemTimeZonesPrivate *KSystemTimeZonesPrivate::instance()
{
    if (!m_instance)
    {
        m_instance = new KSystemTimeZonesPrivate;
        // Go read the database.
        //
        // On Windows, HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Time Zones
        // is the place to look. The TZI binary value is the TIME_ZONE_INFORMATION structure.
        //
        // For Unix, read zone.tab.
        m_instance->readZoneTab();
    }
    return m_instance;
}

bool KSystemTimeZonesPrivate::findZoneTab( QFile& f )
{
#if defined(SOLARIS) || defined(USE_SOLARIS)
    const char *ZONE_TAB_FILE = "/tab/zone_sun.tab";
    const char *ZONE_INFO_DIR = "/usr/share/lib/zoneinfo";
#else
    const char *ZONE_TAB_FILE = "/zone.tab";
    const char *ZONE_INFO_DIR = "/usr/share/zoneinfo";
#endif

    // Find and open zone.tab - it's all easy except knowing where to look. Try the LSB location first.
    QDir dir;
    QString zoneinfoDir = ZONE_INFO_DIR;
    f.setFileName(zoneinfoDir + ZONE_TAB_FILE);
    // make a note if the dir exists; whether it contains zone.tab or not
    if ( dir.exists( zoneinfoDir ) )
        m_zoneinfoDir = zoneinfoDir;
    if (f.open(QIODevice::ReadOnly))
        return true;
    kDebug() << "Can't open " << f.fileName() << endl;

    zoneinfoDir = "/usr/lib/zoneinfo";
    f.setFileName(zoneinfoDir + ZONE_TAB_FILE);
    if ( dir.exists( zoneinfoDir ) )
        m_zoneinfoDir = zoneinfoDir;
    if (f.open(QIODevice::ReadOnly))
        return true;
    kDebug() << "Can't open " << f.fileName() << endl;

    zoneinfoDir = ::getenv("TZDIR");
    if ( !zoneinfoDir.isEmpty() )
    {
        if ( dir.exists( zoneinfoDir ) )
            m_zoneinfoDir = zoneinfoDir;
        f.setFileName(zoneinfoDir + ZONE_TAB_FILE);
        if (f.open(QIODevice::ReadOnly))
            return true;
        kDebug() << "Can't open " << f.fileName() << endl;
    }

    zoneinfoDir = "/usr/share/lib/zoneinfo";
    if ( dir.exists( zoneinfoDir + "/src" ) )
    {
        m_zoneinfoDir = zoneinfoDir;
        // Solaris support. Synthesise something that looks like a zone.tab.
        //
        // grep -h ^Zone /usr/share/lib/zoneinfo/src/* | awk '{print "??\t+9999+99999\t" $2}'
        //
        // where the country code is set to "??" and the latitude/longitude
        // values are dummies.
        //
        QDir d(m_zoneinfoDir + "/src");
        d.setFilter( QDir::Files | QDir::Hidden | QDir::NoSymLinks );
        QStringList fileList = d.entryList();

        KTemporaryFile f;
        f.setAutoRemove(false);
        if (!f.open())
        {
            kError() << "Could not open/create temp file for writing" << endl;
            return false;
        }

        QFile zoneFile;
        QList<QByteArray> tokens;
        QByteArray line;
        line.reserve(1024);
        QTextStream tmpStream(&f);
        qint64 r;
        for (int i = 0, end = fileList.count();  i < end;  ++i)
        {
            zoneFile.setFileName(d.filePath(fileList[i].toLatin1()));
            if (!zoneFile.open(QIODevice::ReadOnly))
            {
                kDebug() << "Could not open file '" << zoneFile.fileName().toLatin1() \
                         << "' for reading." << endl;
                continue;
            }
            while (!zoneFile.atEnd())
            {
                if ((r = zoneFile.readLine(line.data(), 1023)) > 0
                &&  line.startsWith("Zone"))
                {
                    line.replace('\t', ' ');    // change tabs to spaces
                    tokens = line.split(' ');
                    for (int j = 0, jend = tokens.count();  j < jend;  ++j)
                        if (tokens[j].endsWith(' '))
                            tokens[j].chop(1);
                    tmpStream << "??\t+9999+99999\t" << tokens[1] << "\n";
                }
            }
            zoneFile.close();
        }
        f.close();
        if (!f.open())
        {
            kError() << "Could not reopen temp file for reading." << endl;
            return false;
        }
    }
    return false;
}

/*
 * Find the location of the zoneinfo files and store in m_zoneinfoDir.
 * Parse zone.tab and for each time zone, create a KSystemTimeZone instance.
 */
void KSystemTimeZonesPrivate::readZoneTab()
{
    QFile f;
    if ( !findZoneTab( f ) )
        return;
    // Parse the zone.tab or the fake temp file.
    QTextStream str(&f);
    QRegExp lineSeparator("[ \t]");
    QRegExp ordinateSeparator("[+-]");
    if (!m_source)
        m_source = new KSystemTimeZoneSource;
    while (!str.atEnd())
    {
        QString line = str.readLine();
        if (line.isEmpty() || '#' == line[0])
            continue;
        QStringList tokens = KStringHandler::perlSplit(lineSeparator, line, 4);
        int n = tokens.count();
        if (n < 3)
        {
            kError() << "invalid record: " << line << endl;
            continue;
        }

        // Got three tokens. Now check for two ordinates plus first one is "".
        QStringList ordinates = KStringHandler::perlSplit(ordinateSeparator, tokens[1], 2);
        if (ordinates.count() < 2)
        {
            kError() << "invalid coordinates: " << tokens[1] << endl;
            continue;
        }

        float latitude = convertCoordinate(ordinates[0]);
        float longitude = convertCoordinate(ordinates[1]);

        // Add entry to list.
        if (tokens[0] == "??")
            tokens[0] = "";
        // Solaris sets the empty Comments field to '-', making it not empty.
        // Clean it up.
        if (n > 3  &&  tokens[3] == "-")
            tokens[3] = "";
        KTimeZone *tzone = new KSystemTimeZone(m_source, tokens[2], tokens[0], latitude, longitude, (n > 3 ? tokens[3] : QString()));
        add(tzone);
    }
    f.close();
}

const KTimeZone *KSystemTimeZonesPrivate::local()
{
    const KTimeZone *local = 0;

    instance();    // initialize data

    // SOLUTION 1: DEFINITIVE.
    // First try the simplest solution of checking for well-formed TZ setting.
    char *envZone = ::getenv("TZ");
    if (envZone)
    {
        if (envZone[0] == '\0')
            return utc();
        if (envZone[0] == ':')
            ++envZone;
        local = zone(envZone);
    }
    if (local)
        return local;

    if (!m_zoneinfoDir.isEmpty())
    {
        // SOLUTION 2: DEFINITIVE.
        // Try to follow any /etc/localtime symlink to a zoneinfo file.
        QFile f;
        f.setFileName("/etc/localtime");
        QFileInfo fi(f);
        if (fi.isSymLink())
        {
            // Get the path of the file which the symlink points to
            QString zoneInfoFileName = fi.canonicalFilePath();
            if (zoneInfoFileName.startsWith(m_zoneinfoDir))
            {
                QFileInfo fiz(zoneInfoFileName);
                if (fiz.exists() && fiz.isReadable())
                {
                    // We've got the zoneinfo file path.
                    // The time zone name is the part of the path after the zoneinfo directory.
                    QString name = zoneInfoFileName.mid(m_zoneinfoDir.length() + 1);
                    // kDebug() << "local=" << name << endl;
                    local = zone(name);
                }
            }
        }
        else if (f.open(QIODevice::ReadOnly))
        {
            // SOLUTION 3: DEFINITIVE.
            // Try to match /etc/localtime against the list of zoneinfo files.

            // Compute the MD5 sum of /etc/localtime.
            KMD5 context("");
            context.reset();
            context.update(f);
            qlonglong referenceSize = f.size();
            QString referenceMd5Sum = context.hexDigest();
            f.close();

            // Look for a checksum match with the cached checksum values
            bool changed = false;
            MD5Map oldChecksums = m_md5Sums;   // save a copy of the existing checksums
            for (MD5Map::ConstIterator it5 = m_md5Sums.begin(), end5 = m_md5Sums.end();  it5 != end5;  ++it5)
            {
                if (it5.value() == referenceMd5Sum)
                {
                    // The cached checksum matches. Ensure that the file hasn't changed.
                    QString zoneName = it5.key();
                    QString candidateMd5Sum = calcChecksum(zoneName, referenceSize);
                    if (candidateMd5Sum.isNull())
                    {
                        m_md5Sums.remove(zoneName);    // no match - wrong file size
                        changed = true;
                    }
                    else if (candidateMd5Sum != referenceMd5Sum)
                        changed = true;
                    else
                        local = zone(zoneName);    // file still matches

                    // File(s) have changed, so clear the cache
                    if (changed)
                    {
                        oldChecksums.clear();
                        m_md5Sums.clear();
                        m_md5Sums[zoneName] = candidateMd5Sum;    // reinsert the newly calculated checksum
                   }
                    break;
                }
            }
            if (local)
                return local;

            // The checksum didn't match any in the cache.
            // Continue building missing entries in the cache on the assumption that
            // we haven't previously looked at the zoneinfo file which matches.
            const ZoneMap zmap = zones();
            for (ZoneMap::ConstIterator zit = zmap.begin(), zend = zmap.end();  zit != zend;  ++zit)
            {
                const KTimeZone *zone = zit.value();
                QString zonename = zone->name();
                if (!m_md5Sums.contains(zonename))
                {
                    QString candidateMd5Sum = calcChecksum(zonename, referenceSize);
                    if (candidateMd5Sum == referenceMd5Sum)
                    {
                        // kDebug() << "local=" << zone->name() << endl;
                        local = zone;
                        break;
                    }
                }
            }
            if (local)
                return local;

            // Didn't find the file, so presumably a previously cached checksum must
            // have changed. Delete all the old checksums.
            MD5Map::ConstIterator mit;
            MD5Map::ConstIterator mend = oldChecksums.end();
            for (mit = oldChecksums.begin();  mit != mend;  ++mit)
                m_md5Sums.remove(mit.key());

            // And recalculate the old checksums
            for (mit = oldChecksums.begin(); mit != mend; ++mit)
            {
                QString zonename = mit.key();
                QString candidateMd5Sum = calcChecksum(zonename, referenceSize);
                if (candidateMd5Sum == referenceMd5Sum)
                {
                    // kDebug() << "local=" << zonename << endl;
                    local = zone(zonename);
                    break;
                }
            }
        }
        if (local)
            return local;
    }

    // SOLUTION 4: DEFINITIVE.
    // BSD support.
    QString fileZone;
    QFile f;
    f.setFileName("/etc/timezone");
    if (!f.open(QIODevice::ReadOnly))
    {
        kDebug() << "Can't open " << f.fileName() << endl;

        // SOLUTION 5: DEFINITIVE.
        // Solaris support using /etc/default/init.
        f.setFileName("/etc/default/init");
        if (!f.open(QIODevice::ReadOnly))
        {
            kDebug() << "Can't open " << f.fileName() << endl;
        }
        else
        {
            QTextStream ts(&f);
            ts.setCodec("ISO-8859-1");

            // Read the last line starting "TZ=".
            while (!ts.atEnd())
            {
                fileZone = ts.readLine();
                if (fileZone.startsWith("TZ="))
                {
                    fileZone = fileZone.mid(3);

                    // kDebug() << "local=" << fileZone << endl;
                    local = zone(fileZone);
                }
            }
            f.close();
        }
    }
    else
    {
        QTextStream ts(&f);
        ts.setCodec("ISO-8859-1");

        // Read the first line.
        if (!ts.atEnd())
        {
            fileZone = ts.readLine();

            // kDebug() << "local=" << fileZone << endl;
            local = zone(fileZone);
        }
        f.close();
    }
    if (local)
        return local;

    // SOLUTION 6: HEURISTIC.
    // None of the deterministic stuff above has worked: try a heuristic. We
    // try to find a pair of matching time zone abbreviations...that way, we'll
    // likely return a value in the user's own country.
    if (!m_zoneinfoDir.isEmpty())
    {
        tzset();
        QByteArray tzname0(tzname[0]);   // store copies, because zone->parse() will change them
        QByteArray tzname1(tzname[1]);
        int bestOffset = INT_MAX;
        KSystemTimeZoneSource::startParseBlock();
        const ZoneMap zmap = zones();
        for (ZoneMap::ConstIterator it = zmap.begin(), end = zmap.end();  it != end;  ++it)
        {
            const KSystemTimeZone *zone = static_cast<const KSystemTimeZone*>(it.value());
            int candidateOffset = qAbs(zone->currentOffset(Qt::LocalTime));
            if (candidateOffset < bestOffset
            &&  zone->parse())
            {
                QList<QByteArray> abbrs = zone->abbreviations();
                if (abbrs.contains(tzname0)  &&  abbrs.contains(tzname1))
                {
                    // kDebug() << "local=" << zone->name() << endl;
                    local = zone;
                    bestOffset = candidateOffset;
                    if (!bestOffset)
                        break;
                }
            }
        }
        KSystemTimeZoneSource::endParseBlock();
    }
    if (local)
        return local;

    // SOLUTION 7: FAILSAFE.
    return KTimeZones::utc();
}

// Calculate the MD5 checksum for the given zone file, provided that its size matches.
// The calculated checksum is cached.
QString KSystemTimeZonesPrivate::calcChecksum(const QString &zoneName, qlonglong size)
{
    QString path = m_zoneinfoDir + '/' + zoneName;
    QFileInfo fi(path);
    if (static_cast<qlonglong>(fi.size()) == size)
    {
        // Only do the heavy lifting for file sizes which match.
        QFile f;
        f.setFileName(path);
        if (f.open(QIODevice::ReadOnly))
        {
            KMD5 context("");
            context.reset();
            context.update(f);
            QString candidateMd5Sum = context.hexDigest();
            f.close();
            m_md5Sums[zoneName] = candidateMd5Sum;    // cache the new checksum
            return candidateMd5Sum;
        }
    }
    return QString();
}

/**
 * Convert sHHMM or sHHMMSS to a floating point number of degrees.
 */
float KSystemTimeZonesPrivate::convertCoordinate(const QString &coordinate)
{
    int value = coordinate.toInt();
    int degrees = 0;
    int minutes = 0;
    int seconds = 0;

    if (coordinate.length() > 11)
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


/******************************************************************************/

KSystemTimeZone::KSystemTimeZone(KSystemTimeZoneSource *source, const QString &name,
        const QString &countryCode, float latitude, float longitude, const QString &comment)
  : KTimeZone(source, name, countryCode, latitude, longitude, comment)
//    , d(0)
{
}

KSystemTimeZone::~KSystemTimeZone()
{
//    delete d;
}

int KSystemTimeZone::offsetAtUtc(const QDateTime &utcDateTime) const
{
    return offset(toTime_t(utcDateTime));
}

int KSystemTimeZone::offset(time_t t) const
{
    if (t == InvalidTime_t)
        return 0;

    // Make this time zone the current local time zone
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    QByteArray tz = name().toUtf8();
    tz.prepend(":");
    bool change = (tz != originalZone);
    if (change)
    {
        ::setenv("TZ", tz, 1);
        ::tzset();
    }

    int secs = gmtoff(t);

    if (change)
    {
        // Restore the original local time zone
        if (!originalZone)
            ::unsetenv("TZ");
        else
            ::setenv("TZ", originalZone, 1);
        ::tzset();
    }
    return secs;
}

int KSystemTimeZone::offsetAtZoneTime(const QDateTime &zoneDateTime, int *secondOffset) const
{
    if (!zoneDateTime.isValid()  ||  zoneDateTime.timeSpec() != Qt::LocalTime)
        return 0;
    // Make this time zone the current local time zone
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    QByteArray tz = name().toUtf8();
    tz.prepend(":");
    bool change = (tz != originalZone);
    if (change)
    {
        ::setenv("TZ", tz, 1);
        ::tzset();
    }

    // Convert zone time to UTC, and then get the offset to UTC
    tm tmtime;
    tmtime.tm_sec    = zoneDateTime.time().second();
    tmtime.tm_min    = zoneDateTime.time().minute();
    tmtime.tm_hour   = zoneDateTime.time().hour();
    tmtime.tm_mday   = zoneDateTime.date().day();
    tmtime.tm_mon    = zoneDateTime.date().month() - 1;
    tmtime.tm_year   = zoneDateTime.date().year() - 1900;
    tmtime.tm_isdst  = -1;
    time_t t = mktime(&tmtime);
    int offset1 = (t == (time_t)-1) ? 0 : gmtoff(t);
    if (secondOffset)
    {
        int offset2 = offset1;
        if (t != (time_t)-1)
        {
            // Check if there is a backward DST change near to this time, by
            // checking if the UTC offset is different 1 hour later or earlier.
            // ASSUMPTION: DST SHIFTS ARE NEVER GREATER THAN 1 HOUR.
            int maxShift = 3600;
            offset2 = gmtoff(t + maxShift);
            if (offset2 < offset1)
            {
                // There is a backward DST shift during the following hour
                if (offset1 - offset2 < maxShift)
                    offset2 = gmtoff(t + (offset1 - offset2));
            }
            else if ((offset2 = gmtoff(t - maxShift)) > offset1)
            {
                // There is a backward DST shift during the previous hour
                if (offset2 - offset1 < maxShift)
                    offset2 = gmtoff(t - (offset2 - offset1));
                // Put UTC offsets into the correct order
                int o = offset1;
                offset1 = offset2;
                offset2 = o;
            }
            else offset2 = offset1;
        }
        *secondOffset = offset2;
    }

    if (change)
    {
        // Restore the original local time zone
        if (!originalZone)
            ::unsetenv("TZ");
        else
            ::setenv("TZ", originalZone, 1);
        ::tzset();
    }
    return offset1;
}

bool KSystemTimeZone::isDstAtUtc(const QDateTime &utcDateTime) const
{
    return isDst(toTime_t(utcDateTime));
}

bool KSystemTimeZone::isDst(time_t t) const
{
    if (t != (time_t)-1)
    {
#ifdef _POSIX_THREAD_SAFE_FUNCTIONS
        tm tmtime;
        if (localtime_r(&t, &tmtime))
            return tmtime.tm_isdst > 0;
#else
        tm *tmtime = localtime(&t);
        if (tmtime)
            return tmtime->tm_isdst > 0;
#endif
    }
    return false;
}


/******************************************************************************/

class KSystemTimeZoneDataPrivate
{
public:
    QByteArray TZ;
    QList<QByteArray> abbreviations;
};


// N.B. KSystemTimeZoneSourcePrivate is also used by KSystemTimeZoneData
class KSystemTimeZoneSourcePrivate
{
public:
    static void setTZ(const QByteArray &zoneName);
    static void restoreTZ();
    static char      *savedTZ;       // temporary value of TZ environment variable saved by setTZ()
    static QByteArray originalTZ;    // saved value of TZ environment variable during multiple parse() calls
    static bool       TZIsSaved;     // TZ has been saved in savedTZ
    static bool       multiParse;    // true if performing multiple parse() calls
};

char      *KSystemTimeZoneSourcePrivate::savedTZ;
QByteArray KSystemTimeZoneSourcePrivate::originalTZ;
bool       KSystemTimeZoneSourcePrivate::TZIsSaved = false;
bool       KSystemTimeZoneSourcePrivate::multiParse = false;


KSystemTimeZoneSource::KSystemTimeZoneSource()
//  : d(new KSystemTimeZoneSourcePrivate)
{
}

KSystemTimeZoneSource::~KSystemTimeZoneSource()
{
//    delete d;
}

KTimeZoneData* KSystemTimeZoneSource::parse(const KTimeZone *zone) const
{
    QByteArray tz = zone->name().toUtf8();
    KSystemTimeZoneSourcePrivate::setTZ(tz);   // make this time zone the current local time zone

    tzset();    // initialize the tzname array
    KSystemTimeZoneData* data = new KSystemTimeZoneData;
    data->d->TZ = tz;
    data->d->abbreviations.append(tzname[0]);
    data->d->abbreviations.append(tzname[1]);

    // There is no easy means to access the sequence of daylight savings time
    // changes, or leap seconds adjustments, so leave that data empty.

    KSystemTimeZoneSourcePrivate::restoreTZ();   // restore the original local time zone if necessary
    return data;
}

void KSystemTimeZoneSource::startParseBlock()
{
    KSystemTimeZoneSourcePrivate::originalTZ = ::getenv("TZ");   // save the original local time zone
    KSystemTimeZoneSourcePrivate::multiParse = true;
}

void KSystemTimeZoneSource::endParseBlock()
{
    if (KSystemTimeZoneSourcePrivate::multiParse)
    {
        // Restore the original local time zone
        if (KSystemTimeZoneSourcePrivate::originalTZ.isEmpty())
            ::unsetenv("TZ");
        else
            ::setenv("TZ", KSystemTimeZoneSourcePrivate::originalTZ, 1);
        ::tzset();
        KSystemTimeZoneSourcePrivate::multiParse = false;
    }
}

// Set the TZ environment variable to the specified time zone,
// saving its current setting first if necessary.
void KSystemTimeZoneSourcePrivate::setTZ(const QByteArray &zoneName)
{
    QByteArray tz = zoneName;
    tz.prepend(":");
    bool setTZ = multiParse;
    if (!setTZ)
    {
        savedTZ = ::getenv("TZ");   // save the original local time zone
        TZIsSaved = true;
        setTZ = (tz != savedTZ);
    }
    if (setTZ)
    {
        ::setenv("TZ", tz, 1);
        ::tzset();
    }
}

// Restore the TZ environment variable if it was saved by setTz()
void KSystemTimeZoneSourcePrivate::restoreTZ()
{
    if (TZIsSaved)
    {
        if (!savedTZ)
            ::unsetenv("TZ");
        else
            ::setenv("TZ", savedTZ, 1);
        ::tzset();
        TZIsSaved = false;
    }
}


/******************************************************************************/

KSystemTimeZoneData::KSystemTimeZoneData()
  : d(new KSystemTimeZoneDataPrivate)
{ }

KSystemTimeZoneData::KSystemTimeZoneData(const KSystemTimeZoneData &rhs)
  : KTimeZoneData(),
    d(new KSystemTimeZoneDataPrivate)
{
    operator=(rhs);
}

KSystemTimeZoneData::~KSystemTimeZoneData()
{
    delete d;
}

KSystemTimeZoneData &KSystemTimeZoneData::operator=(const KSystemTimeZoneData &rhs)
{
    d->TZ = rhs.d->TZ;
    d->abbreviations = rhs.d->abbreviations;
    return *this;
}

KTimeZoneData *KSystemTimeZoneData::clone()
{
    return new KSystemTimeZoneData(*this);
}

QList<QByteArray> KSystemTimeZoneData::abbreviations() const
{
    return d->abbreviations;
}

QByteArray KSystemTimeZoneData::abbreviation(const QDateTime &utcDateTime) const
{
    QByteArray abbr;
    if (utcDateTime.timeSpec() != Qt::UTC)
        return abbr;
    time_t t = utcDateTime.toTime_t();
    if (t != KTimeZone::InvalidTime_t)
    {
        KSystemTimeZoneSourcePrivate::setTZ(d->TZ);   // make this time zone the current local time zone

        /* Use tm.tm_zone if available because it returns the abbreviation
         * in use at the time specified. Otherwise, use tzname[] which
         * returns the appropriate current abbreviation instead.
         */
#ifdef _POSIX_THREAD_SAFE_FUNCTIONS
        tm tmtime;
        if (localtime_r(&t, &tmtime))
#ifdef HAVE_STRUCT_TM_TM_ZONE
            abbr = tmtime.tm_zone;
#else
            abbr = tzname[(tmtime.tm_isdst > 0) ? 1 : 0];
#endif
#else
        tm *tmtime = localtime(&t);
        if (tmtime)
#ifdef HAVE_STRUCT_TM_TM_ZONE
            abbr = tmtime->tm_zone;
#else
            abbr = tzname[(tmtime->tm_isdst > 0) ? 1 : 0];
#endif
#endif
        KSystemTimeZoneSourcePrivate::restoreTZ();   // restore the original local time zone if necessary
    }
    return abbr;
}

QList<int> KSystemTimeZoneData::utcOffsets() const
{
    return QList<int>();
}
