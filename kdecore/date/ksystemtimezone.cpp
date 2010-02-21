/*
   This file is part of the KDE libraries
   Copyright (c) 2005-2009 David Jarvie <djarvie@kde.org>
   Copyright (c) 2005 S.R.Haque <srhaque@iee.org>.

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

// This file requires HAVE_STRUCT_TM_TM_ZONE to be defined if struct tm member tm_zone is available.
// This file requires HAVE_TM_GMTOFF to be defined if struct tm member tm_gmtoff is available.

#include "ksystemtimezone.moc"

#include <config.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include <climits>
#include <cstdlib>

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusReply>

#include <kglobal.h>
#include <klocale.h>
#include <kcodecs.h>
#include <kstringhandler.h>
#include <ktemporaryfile.h>
#include <ktoolinvocation.h>
#include <kdebug.h>
#include <kconfiggroup.h>
#include "ktzfiletimezone.h"
#ifdef Q_OS_WIN
#include "ktimezone_win.h"
#endif

#define KTIMEZONED_DBUS_IFACE "org.kde.KTimeZoned"


/* Return the offset to UTC in the current time zone at the specified UTC time.
 * The thread-safe function localtime_r() is used in preference if available.
 */
int gmtoff(time_t t)
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
      if (lwday == uwday + 1  ||  (lwday == 0 && uwday == 6))
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
    static KTzfileTimeZoneSource *tzfileSource();
    static void setLocalZone();
    static void cleanup();
    static void readConfig(bool init);
#ifdef Q_OS_WIN
    static void updateTimezoneInformation()
    {
      instance()->updateTimezoneInformation(true);
    }
#else
    static void updateZonetab()  { instance()->readZoneTab(true); }
#endif

    static KTimeZone m_localZone;
    static QString m_localZoneName;
    static QString m_zoneinfoDir;
    static QString m_zonetab;
    static KSystemTimeZoneSource *m_source;

private:
    KSystemTimeZonesPrivate() {}
#ifdef Q_OS_WIN
    void updateTimezoneInformation(bool update);
#else
    void readZoneTab(bool update);
    static float convertCoordinate(const QString &coordinate);
#endif

    static KSystemTimeZones *m_parent;
    static KSystemTimeZonesPrivate *m_instance;
    static KTzfileTimeZoneSource *m_tzfileSource;
};

KTimeZone                KSystemTimeZonesPrivate::m_localZone;
QString                  KSystemTimeZonesPrivate::m_localZoneName;
QString                  KSystemTimeZonesPrivate::m_zoneinfoDir;
QString                  KSystemTimeZonesPrivate::m_zonetab;
KSystemTimeZoneSource   *KSystemTimeZonesPrivate::m_source = 0;
KTzfileTimeZoneSource   *KSystemTimeZonesPrivate::m_tzfileSource = 0;
KSystemTimeZones        *KSystemTimeZonesPrivate::m_parent = 0;
KSystemTimeZonesPrivate *KSystemTimeZonesPrivate::m_instance = 0;

KTzfileTimeZoneSource *KSystemTimeZonesPrivate::tzfileSource()
{
    if (!m_tzfileSource)
    {
        instance();
        m_tzfileSource = new KTzfileTimeZoneSource(m_zoneinfoDir);
    }
    return m_tzfileSource;
}


#ifndef NDEBUG
K_GLOBAL_STATIC(KTimeZone, simulatedLocalZone)
#endif


KSystemTimeZones::KSystemTimeZones()
  : d(0)
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.connect(QString(), QString(), KTIMEZONED_DBUS_IFACE, "configChanged", this, SLOT(configChanged()));
    dbus.connect(QString(), QString(), KTIMEZONED_DBUS_IFACE, "zonetabChanged", this, SLOT(zonetabChanged(QString)));
    // No need to connect to definitionChanged() - see comments in zoneDefinitionChanged()
    //dbus.connect(QString(), QString(), KTIMEZONED_DBUS_IFACE, "definitionChanged", this, SLOT(zoneDefinitionChanged(QString)));
}

KSystemTimeZones::~KSystemTimeZones()
{
}

KTimeZone KSystemTimeZones::local()
{
#ifndef NDEBUG
    if (simulatedLocalZone->isValid())
	return *simulatedLocalZone;
#endif
    KSystemTimeZonesPrivate::instance();
    return KSystemTimeZonesPrivate::m_localZone;
}

KTimeZone KSystemTimeZones::realLocalZone()
{
    KSystemTimeZonesPrivate::instance();
    return KSystemTimeZonesPrivate::m_localZone;
}

void KSystemTimeZones::setLocalZone(const KTimeZone& tz)
{
    Q_UNUSED(tz);
#ifndef NDEBUG
    *simulatedLocalZone = tz;
#endif
}

bool KSystemTimeZones::isSimulated()
{
#ifndef NDEBUG
    return simulatedLocalZone->isValid();
#else
    return false;
#endif
}

QString KSystemTimeZones::zoneinfoDir()
{
    KSystemTimeZonesPrivate::instance();
    return KSystemTimeZonesPrivate::m_zoneinfoDir;
}

KTimeZones *KSystemTimeZones::timeZones()
{
    return KSystemTimeZonesPrivate::instance();
}

KTimeZone KSystemTimeZones::readZone(const QString &name)
{
    return KTzfileTimeZone(KSystemTimeZonesPrivate::tzfileSource(), name);
}

const KTimeZones::ZoneMap KSystemTimeZones::zones()
{
    return KSystemTimeZonesPrivate::instance()->zones();
}

KTimeZone KSystemTimeZones::zone(const QString& name)
{
    return KSystemTimeZonesPrivate::instance()->zone(name);
}

void KSystemTimeZones::configChanged()
{
    kDebug(161) << "KSystemTimeZones::configChanged()";
    KSystemTimeZonesPrivate::readConfig(false);
}

void KSystemTimeZones::zonetabChanged(const QString &zonetab)
{
    Q_UNUSED(zonetab)
#ifndef Q_OS_WIN
    kDebug(161) << "KSystemTimeZones::zonetabChanged()";
    // Re-read zone.tab and update our collection, removing any deleted
    // zones and adding any new zones.
    KSystemTimeZonesPrivate::updateZonetab();
#endif
}

void KSystemTimeZones::zoneDefinitionChanged(const QString &zone)
{
    // No need to do anything when the definition (as opposed to the
    // identity) of the local zone changes, since the updated details
    // will always be accessed by the system library calls to fetch
    // local zone information.
    Q_UNUSED(zone)
}

// Perform initialization, create the unique KSystemTimeZones instance,
// whose only function is to receive D-Bus signals from KTimeZoned,
// and create the unique KSystemTimeZonesPrivate instance.
KSystemTimeZonesPrivate *KSystemTimeZonesPrivate::instance()
{
    if (!m_instance)
    {
        m_instance = new KSystemTimeZonesPrivate;

        // A KSystemTimeZones instance is required only to catch D-Bus signals.
        m_parent = new KSystemTimeZones;
        // Ensure that the KDED time zones module has initialized. The call loads the module on demand.
        if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kded"))
            KToolInvocation::klauncher();   // this calls startKdeinit, and blocks until it returns
        QDBusInterface *ktimezoned = new QDBusInterface("org.kde.kded", "/modules/ktimezoned", KTIMEZONED_DBUS_IFACE);
        QDBusReply<void> reply = ktimezoned->call("initialize", false);
        if (!reply.isValid())
            kError(161) << "KSystemTimeZones: ktimezoned initialize() D-Bus call failed: " << reply.error().message() << endl;
kDebug(161)<<"instance(): ... initialised";
        delete ktimezoned;

        // Read the time zone config written by ktimezoned
        readConfig(true);

        // Go read the database.
#ifdef Q_OS_WIN
        // On Windows, HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Time Zones
        // is the place to look. The TZI binary value is the TIME_ZONE_INFORMATION structure.
        m_instance->updateTimezoneInformation(false);
#else
        // For Unix, read zone.tab.
        if (!m_zonetab.isEmpty())
            m_instance->readZoneTab(false);
#endif
        setLocalZone();
        if (!m_localZone.isValid())
            m_localZone = KTimeZone::utc();   // ensure a time zone is always returned

        qAddPostRoutine(KSystemTimeZonesPrivate::cleanup);
    }
    return m_instance;
}

void KSystemTimeZonesPrivate::readConfig(bool init)
{
    KConfig config(QLatin1String("ktimezonedrc"));
    if (!init)
        config.reparseConfiguration();
    KConfigGroup group(&config, "TimeZones");
    m_zoneinfoDir   = group.readEntry("ZoneinfoDir");
    m_zonetab       = group.readEntry("Zonetab");
    m_localZoneName = group.readEntry("LocalZone");
    if (m_zoneinfoDir.length() > 1 && m_zoneinfoDir.endsWith('/'))
        m_zoneinfoDir.truncate(m_zoneinfoDir.length() - 1);  // strip trailing '/'
    if (!init)
        setLocalZone();
    kDebug(161) << "readConfig(): local zone=" << m_localZoneName;
}

void KSystemTimeZonesPrivate::setLocalZone()
{
    QString filename;
    if (m_localZoneName.startsWith('/'))
    {
        // The time zone is specified by a file outside the zoneinfo directory
        filename = m_localZoneName;
    }
    else
    {
        // The zone name is either a known zone, or it's a relative file name
        // in zoneinfo directory which isn't in zone.tab.
        m_localZone = m_instance->zone(m_localZoneName);
        if (m_localZone.isValid())
            return;
        // It's a relative file name
        filename = m_zoneinfoDir + '/' + m_localZoneName;
    }

    // Parse the specified time zone data file
    QString zonename = filename;
    if (zonename.startsWith(m_zoneinfoDir + '/'))
        zonename = zonename.mid(m_zoneinfoDir.length() + 1);
    m_localZone = KTzfileTimeZone(KSystemTimeZonesPrivate::tzfileSource(), zonename);
    if (m_localZone.isValid() && m_instance)
    {
        // Add the new time zone to the list
        KTimeZone oldzone = m_instance->zone(zonename);
        if (!oldzone.isValid() || oldzone.type() != "KTzfileTimeZone")
        {
            m_instance->remove(oldzone);
            m_instance->add(m_localZone);
        }
    }
}

void KSystemTimeZonesPrivate::cleanup()
{
    delete m_parent;
    delete m_instance;
    delete m_source;
    delete m_tzfileSource;
}

#ifdef Q_OS_WIN

void KSystemTimeZonesPrivate::updateTimezoneInformation(bool update)
{
    if (!m_source)
        m_source = new KSystemTimeZoneSourceWindows;
    QStringList newZones;
    Q_FOREACH( const QString & tz, KSystemTimeZoneWindows::listTimeZones() ) {
       // const std::wstring wstr = tz.toStdWString();
       // const KTimeZone info = make_time_zone( wstr.c_str() );
      KSystemTimeZoneWindows stz(m_source, tz);
      if (update)
        {
            // Update the existing collection with the new zone definition
            newZones += stz.name();
            KTimeZone oldTz = zone(stz.name());
            if (oldTz.isValid())
                oldTz.updateBase(stz);   // the zone previously existed, so update its definition
            else
                add(stz);   // the zone didn't previously exist, so add it
        }
        else
            add(stz);
    }
    if (update)
    {
        // Remove any zones from the collection which no longer exist
        const ZoneMap oldZones = zones();
        for (ZoneMap::const_iterator it = oldZones.begin();  it != oldZones.end();  ++it)
        {
            if (newZones.indexOf(it.key()) < 0)
                remove(it.value());
        }
    }
}

#else
/*
 * Find the location of the zoneinfo files and store in mZoneinfoDir.
 * Parse zone.tab and for each time zone, create a KSystemTimeZone instance.
 */
void KSystemTimeZonesPrivate::readZoneTab(bool update)
{
    kDebug(161) << "readZoneTab(" << m_zonetab<< ")";
    QStringList newZones;
    QFile f;
    f.setFileName(m_zonetab);
    if (!f.open(QIODevice::ReadOnly))
        return;
    QTextStream str(&f);
    QRegExp lineSeparator("[ \t]");
    QRegExp ordinateSeparator("[+-]");
    if (!m_source)
        m_source = new KSystemTimeZoneSource;
    while (!str.atEnd())
    {
        QString line = str.readLine();
        if (line.isEmpty() || line[0] == '#')
            continue;
        QStringList tokens = KStringHandler::perlSplit(lineSeparator, line, 4);
        int n = tokens.count();
        if (n < 3)
        {
            kError(161) << "readZoneTab(): invalid record: " << line << endl;
            continue;
        }

        // Got three tokens. Now check for two ordinates plus first one is "".
        int i = tokens[1].indexOf(QRegExp("[+-]"), 1);
        if (i < 0)
        {
            kError(161) << "readZoneTab() " << tokens[2] << ": invalid coordinates: " << tokens[1] << endl;
            continue;
        }

        float latitude = convertCoordinate(tokens[1].left(i));
        float longitude = convertCoordinate(tokens[1].mid(i));

        // Add entry to list.
        if (tokens[0] == "??")
            tokens[0] = "";
        // Solaris sets the empty Comments field to '-', making it not empty.
        // Clean it up.
        if (n > 3  &&  tokens[3] == "-")
            tokens[3] = "";
        KSystemTimeZone tz(m_source, tokens[2], tokens[0], latitude, longitude, (n > 3 ? tokens[3] : QString()));
        if (update)
        {
            // Update the existing collection with the new zone definition
            newZones += tz.name();
            KTimeZone oldTz = zone(tz.name());
            if (oldTz.isValid())
                oldTz.updateBase(tz);   // the zone previously existed, so update its definition
            else
                add(tz);   // the zone didn't previously exist, so add it
        }
        else
            add(tz);
    }
    f.close();

    if (update)
    {
        // Remove any zones from the collection which no longer exist
        const ZoneMap oldZones = zones();
        for (ZoneMap::ConstIterator it = oldZones.constBegin();  it != oldZones.constEnd();  ++it)
        {
            if (newZones.indexOf(it.key()) < 0)
                remove(it.value());
        }
    }
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

    if (coordinate.length() > 6)
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
#endif


/******************************************************************************/


KSystemTimeZoneBackend::KSystemTimeZoneBackend(KSystemTimeZoneSource *source, const QString &name,
        const QString &countryCode, float latitude, float longitude, const QString &comment)
  : KTimeZoneBackend(source, name, countryCode, latitude, longitude, comment)
{}

KSystemTimeZoneBackend::~KSystemTimeZoneBackend()
{}

KTimeZoneBackend *KSystemTimeZoneBackend::clone() const
{
    return new KSystemTimeZoneBackend(*this);
}

QByteArray KSystemTimeZoneBackend::type() const
{
    return "KSystemTimeZone";
}

int KSystemTimeZoneBackend::offsetAtZoneTime(const KTimeZone *caller, const QDateTime &zoneDateTime, int *secondOffset) const
{
    if (!caller->isValid()  ||  !zoneDateTime.isValid()  ||  zoneDateTime.timeSpec() != Qt::LocalTime)
        return 0;
    // Make this time zone the current local time zone
    const QByteArray originalZone = qgetenv("TZ");   // save the original local time zone
    QByteArray tz = caller->name().toUtf8();
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
        if (originalZone.isEmpty())
            ::unsetenv("TZ");
        else
            ::setenv("TZ", originalZone, 1);
        ::tzset();
    }
    return offset1;
}

int KSystemTimeZoneBackend::offsetAtUtc(const KTimeZone *caller, const QDateTime &utcDateTime) const
{
    return offset(caller, KTimeZone::toTime_t(utcDateTime));
}

int KSystemTimeZoneBackend::offset(const KTimeZone *caller, time_t t) const
{
    if (!caller->isValid()  ||  t == KTimeZone::InvalidTime_t)
        return 0;

    // Make this time zone the current local time zone
    const QByteArray originalZone = qgetenv("TZ");   // save the original local time zone
    QByteArray tz = caller->name().toUtf8();
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
        if (originalZone.isEmpty())
            ::unsetenv("TZ");
        else
            ::setenv("TZ", originalZone, 1);
        ::tzset();
    }
    return secs;
}

bool KSystemTimeZoneBackend::isDstAtUtc(const KTimeZone *caller, const QDateTime &utcDateTime) const
{
    return isDst(caller, KTimeZone::toTime_t(utcDateTime));
}

bool KSystemTimeZoneBackend::isDst(const KTimeZone *caller, time_t t) const
{
    Q_UNUSED(caller)
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

KSystemTimeZone::KSystemTimeZone(KSystemTimeZoneSource *source, const QString &name,
        const QString &countryCode, float latitude, float longitude, const QString &comment)
  : KTimeZone(new KSystemTimeZoneBackend(source, name, countryCode, latitude, longitude, comment))
{
}

KSystemTimeZone::~KSystemTimeZone()
{
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
    static QByteArray savedTZ;       // temporary value of TZ environment variable saved by setTZ()
    static QByteArray originalTZ;    // saved value of TZ environment variable during multiple parse() calls
    static bool       TZIsSaved;     // TZ has been saved in savedTZ
    static bool       multiParse;    // true if performing multiple parse() calls
};

QByteArray KSystemTimeZoneSourcePrivate::savedTZ;
QByteArray KSystemTimeZoneSourcePrivate::originalTZ;
bool       KSystemTimeZoneSourcePrivate::TZIsSaved = false;
bool       KSystemTimeZoneSourcePrivate::multiParse = false;


KSystemTimeZoneSource::KSystemTimeZoneSource()
    : d(0)
//  : d(new KSystemTimeZoneSourcePrivate)
{
}

KSystemTimeZoneSource::~KSystemTimeZoneSource()
{
//    delete d;
}

KTimeZoneData* KSystemTimeZoneSource::parse(const KTimeZone &zone) const
{
    QByteArray tz = zone.name().toUtf8();
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
    KSystemTimeZoneSourcePrivate::originalTZ = qgetenv("TZ");   // save the original local time zone
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
        savedTZ = qgetenv("TZ");   // save the original local time zone
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
        if (savedTZ.isEmpty())
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

KTimeZoneData *KSystemTimeZoneData::clone() const
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

