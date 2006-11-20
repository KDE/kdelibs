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

#include <QSet>
#include <QSharedData>

#include <kdebug.h>
#include <ktimezones.h>



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
      if (lwday == uwday + 1  ||  lwday == 0 && uwday == 6)
        lt += 24*3600;
      else
        lt -= 24*3600;
    }
    return lt - ut;
#endif
}


/******************************************************************************/

class KTimeZonesPrivate
{
public:
    KTimeZonesPrivate() : zones(new KTimeZones::ZoneMap())  {}
    ~KTimeZonesPrivate()  { clear();  delete zones; }
    void clear();
    static KTimeZone *utc();

    KTimeZones::ZoneMap *zones;
    QSet<KTimeZone*> nonconstZones;   // member zones owned by KTimeZones
};

void KTimeZonesPrivate::clear()
{
  // Delete all zones actually owned by this collection.
  for (KTimeZones::ZoneMap::ConstIterator it = zones->begin(), end = zones->end();  it != end;  ++it) {
    if (nonconstZones.contains(const_cast<KTimeZone*>(it.value())))
      delete it.value();
  }
  zones->clear();
  nonconstZones.clear();
}

KTimeZone *KTimeZonesPrivate::utc()
{
    static KTimeZone utcZone;
    return &utcZone;
}


KTimeZones::KTimeZones()
  : d(new KTimeZonesPrivate)
{
}

KTimeZones::~KTimeZones()
{
    delete d;
}

const KTimeZones::ZoneMap KTimeZones::zones() const
{
    return *d->zones;
}

bool KTimeZones::add(KTimeZone *zone)
{
    if (!zone)
        return false;
    if (d->zones->find(zone->name()) != d->zones->end())
        return false;    // name already exists
    d->zones->insert(zone->name(), zone);
    d->nonconstZones.insert(zone);
    return true;
}

bool KTimeZones::addConst(const KTimeZone *zone)
{
    if (!zone)
        return false;
    if (d->zones->find(zone->name()) != d->zones->end())
        return false;    // name already exists
    d->zones->insert(zone->name(), zone);
    return true;
}

const KTimeZone *KTimeZones::detach(const KTimeZone *zone)
{
    if (zone)
    {
        for (ZoneMap::Iterator it = d->zones->begin(), end = d->zones->end();  it != end;  ++it)
        {
            if (it.value() == zone)
            {
                d->zones->erase(it);
                d->nonconstZones.remove(const_cast<KTimeZone*>(zone));
                return (zone == utc()) ? 0 : zone;
            }
        }
    }
    return 0;
}

const KTimeZone *KTimeZones::detach(const QString &name)
{
    if (!name.isEmpty())
    {
        ZoneMap::Iterator it = d->zones->find(name);
        if (it != d->zones->end())
        {
            const KTimeZone *zone = it.value();
            d->zones->erase(it);
            d->nonconstZones.remove(const_cast<KTimeZone*>(zone));
            return (zone == utc()) ? 0 : zone;
        }
    }
    return 0;
}

void KTimeZones::clear()
{
  d->clear();
}

const KTimeZone *KTimeZones::zone(const QString &name) const
{
#ifdef Q_WS_WIN
    // return always the utc for now
    return KTimeZonesPrivate::utc();
#else
    if (!name.isEmpty())
    {
        ZoneMap::ConstIterator it = d->zones->find(name);
        if (it != d->zones->end())
            return it.value();
        if (name == KTimeZonesPrivate::utc()->name())
            return KTimeZonesPrivate::utc();
    }
    return 0;    // error
#endif
}

const KTimeZone *KTimeZones::utc()
{
    return KTimeZonesPrivate::utc();
}


/******************************************************************************/

class KTimeZonePhasePrivate : public QSharedData
{
    public:
        QByteArray       abbreviations;  // time zone abbreviations (zero-delimited)
        QString          comment;        // optional comment
        int              utcOffset;      // seconds to add to UTC
        bool             dst;            // true if daylight savings time

        explicit KTimeZonePhasePrivate(int offset = 0, bool ds = false)
        : QSharedData(),
          utcOffset(offset),
          dst(ds)
        {}
        KTimeZonePhasePrivate(const KTimeZonePhasePrivate& rhs)
        : QSharedData(rhs),
          abbreviations(rhs.abbreviations),
          comment(rhs.comment),
          utcOffset(rhs.utcOffset),
          dst(rhs.dst)
        {}
        bool operator==(const KTimeZonePhasePrivate &rhs) const
        {
            return abbreviations == rhs.abbreviations
               &&  comment       == rhs.comment
               &&  utcOffset     == rhs.utcOffset
               &&  dst           == rhs.dst;
        }
};


KTimeZone::Phase::Phase()
  : d(new KTimeZonePhasePrivate)
{
}

KTimeZone::Phase::Phase(int utcOffset, const QByteArray &abbrevs,
                        bool dst, const QString &cmt)
  : d(new KTimeZonePhasePrivate(utcOffset, dst))
{
    d->abbreviations = abbrevs;
    d->comment       = cmt;
}

KTimeZone::Phase::Phase(int utcOffset, const QList<QByteArray> &abbrevs,
                        bool dst, const QString &cmt)
  : d(new KTimeZonePhasePrivate(utcOffset, dst))
{
    for (int i = 0, end = abbrevs.count();  i < end;  ++i)
    {
        if (i > 0)
            d->abbreviations += '\0';
        d->abbreviations += abbrevs[i];
    }
    d->comment = cmt;
}

KTimeZone::Phase::Phase(const KTimeZone::Phase &rhs)
  : d(rhs.d)
{
}

KTimeZone::Phase::~Phase()
{
}

KTimeZone::Phase &KTimeZone::Phase::operator=(const KTimeZone::Phase &rhs)
{
    d = rhs.d;
    return *this;
}

bool KTimeZone::Phase::operator==(const KTimeZone::Phase &rhs) const
{
    return d == rhs.d  ||  *d == *rhs.d;
}

int KTimeZone::Phase::utcOffset() const
{
    return d->utcOffset;
}

QList<QByteArray> KTimeZone::Phase::abbreviations() const
{
    return d->abbreviations.split('\0');
}

bool KTimeZone::Phase::isDst() const
{
    return d->dst;
}

QString KTimeZone::Phase::comment() const
{
    return d->comment;
}


/******************************************************************************/

class KTimeZoneTransitionPrivate
{
public:
    QDateTime time;
    KTimeZone::Phase phase;
};


KTimeZone::Transition::Transition()
    : d(new KTimeZoneTransitionPrivate)
{
}

KTimeZone::Transition::Transition(const QDateTime &t, const KTimeZone::Phase &p) 
    : d(new KTimeZoneTransitionPrivate)
{
    d->time  = t;
    d->phase = p;
}

KTimeZone::Transition::Transition(const KTimeZone::Transition &t)
    : d(new KTimeZoneTransitionPrivate)
{
    d->time  = t.d->time;
    d->phase = t.d->phase;
}

KTimeZone::Transition::~Transition()
{
    delete d;
}

KTimeZone::Transition &KTimeZone::Transition::operator=(const KTimeZone::Transition &t)
{
    d->time  = t.d->time;
    d->phase = t.d->phase;
    return *this;
}

bool KTimeZone::Transition::operator<(const KTimeZone::Transition &rhs) const
{
    return d->time < rhs.d->time;
}

QDateTime        KTimeZone::Transition::time() const   { return d->time; }
KTimeZone::Phase KTimeZone::Transition::phase() const  { return d->phase; }


/******************************************************************************/

class KTimeZoneDataPrivate
{
    public:
        QList<KTimeZone::Phase>       phases;
        QList<KTimeZone::Transition>  transitions;
        QList<KTimeZone::LeapSeconds> leapChanges;
        QList<int>                    utcOffsets;
        QList<QByteArray>             abbreviations;
        int preUtcOffset;    // UTC offset to use before the first phase

        KTimeZoneDataPrivate() : preUtcOffset(0) {}
        // Find the last transition before a specified UTC or local date/time.
        int transitionIndex(const QDateTime &dt) const;
        bool isSecondOccurrence(const QDateTime &utcLocalTime, int transitionIndex) const;
};


/******************************************************************************/

class KTimeZonePrivate
{
public:
    KTimeZonePrivate(KTimeZoneSource *src, const QString& nam,
                     const QString &country, float lat, float lon, const QString &cmnt);
    KTimeZonePrivate(const KTimeZonePrivate &);
    ~KTimeZonePrivate()  { delete data; }
    KTimeZonePrivate &operator=(const KTimeZonePrivate &);
    static KTimeZoneSource *utcSource();

    KTimeZoneSource *source;
    QString name;
    QString countryCode;
    QString comment;
    float   latitude;
    float   longitude;
    KTimeZoneData *data;

private:
    static KTimeZoneSource *mUtcSource;
};

KTimeZoneSource *KTimeZonePrivate::mUtcSource;


KTimeZonePrivate::KTimeZonePrivate(KTimeZoneSource *src, const QString& nam,
                 const QString &country, float lat, float lon, const QString &cmnt)
  : source(src),
    name(nam),
    countryCode(country),
    comment(cmnt),
    latitude(lat),
    longitude(lon),
    data(0)
{
    // Detect duff values.
    if ( latitude > 90 || latitude < -90 )
        latitude = KTimeZone::UNKNOWN;
    if ( longitude > 180 || longitude < -180 )
        longitude = KTimeZone::UNKNOWN;
}

KTimeZonePrivate::KTimeZonePrivate(const KTimeZonePrivate &rhs)
    : source(rhs.source),
      name(rhs.name),
      countryCode(rhs.countryCode),
      comment(rhs.comment),
      latitude(rhs.latitude),
      longitude(rhs.longitude)
{
    if (rhs.data)
        data = rhs.data->clone();
    else
        data = 0;
}

KTimeZonePrivate &KTimeZonePrivate::operator=(const KTimeZonePrivate &rhs)
{
    source      = rhs.source;
    name        = rhs.name;
    countryCode = rhs.countryCode;
    comment     = rhs.comment;
    latitude    = rhs.latitude;
    longitude   = rhs.longitude;
    delete data;
    if (rhs.data)
        data = rhs.data->clone();
    else
        data = 0;
    return *this;
}

KTimeZoneSource *KTimeZonePrivate::utcSource()
{
    if (!mUtcSource)
        mUtcSource = new KTimeZoneSource;
    return mUtcSource;
}


#if SIZEOF_TIME_T == 8
const time_t KTimeZone::InvalidTime_t = 0x800000000000000LL;
#else
const time_t KTimeZone::InvalidTime_t = 0x80000000;
#endif
const int    KTimeZone::InvalidOffset = 0x80000000;
const float  KTimeZone::UNKNOWN = 1000.0;


KTimeZone::KTimeZone(const QString &name)
  : d(new KTimeZonePrivate(KTimeZonePrivate::utcSource(), name, QString(), UNKNOWN, UNKNOWN, QString()))
{
}

KTimeZone::KTimeZone(
    KTimeZoneSource *source, const QString &name,
    const QString &countryCode, float latitude, float longitude,
    const QString &comment)
  : d(new KTimeZonePrivate(source, name, countryCode, latitude, longitude, comment))
{
}

KTimeZone::KTimeZone(const KTimeZone &rhs)
  : d(new KTimeZonePrivate(*rhs.d))
{
}

KTimeZone::~KTimeZone()
{
    delete d;
}

KTimeZone &KTimeZone::operator=(const KTimeZone &rhs)
{
    d->operator=(*rhs.d);
    return *this;
}

QString KTimeZone::countryCode() const
{
    return d->countryCode;
}

float KTimeZone::latitude() const
{
    return d->latitude;
}

float KTimeZone::longitude() const
{
    return d->longitude;
}

QString KTimeZone::comment() const
{
    return d->comment;
}

QString KTimeZone::name() const
{
    return d->name;
}

QList<QByteArray> KTimeZone::abbreviations() const
{
    if (!data(true))
        return QList<QByteArray>();
    return d->data->abbreviations();
}

QByteArray KTimeZone::abbreviation(const QDateTime &utcDateTime) const
{
    if (utcDateTime.timeSpec() != Qt::UTC  ||  !data(true))
        return QByteArray();
    return d->data->abbreviation(utcDateTime);
}

QList<int> KTimeZone::utcOffsets() const
{
    if (!data(true))
        return QList<int>();
    return d->data->utcOffsets();
}

QList<KTimeZone::Phase> KTimeZone::phases() const
{
    if (!data(true))
        return QList<KTimeZone::Phase>();
    return d->data->phases();
}

bool KTimeZone::hasTransitions() const
{
    return false;
}

QList<KTimeZone::Transition> KTimeZone::transitions() const
{
    if (!data(true))
        return QList<KTimeZone::Transition>();
    return d->data->transitions();
}

const KTimeZone::Transition *KTimeZone::transition(const QDateTime &dt, const Transition **secondTransition,
                                                   bool *validTime ) const
{
    if (!data(true))
        return 0;
    return d->data->transition(dt, secondTransition, validTime);
}

int KTimeZone::transitionIndex(const QDateTime &dt, int *secondIndex, bool *validTime) const
{
    if (!data(true))
        return -1;
    return d->data->transitionIndex(dt, secondIndex, validTime);
}

QList<KTimeZone::LeapSeconds> KTimeZone::leapSecondChanges() const
{
    if (!data(true))
        return QList<KTimeZone::LeapSeconds>();
    return d->data->leapSecondChanges();
}

const KTimeZoneData *KTimeZone::data(bool create) const
{
    if (create && !d->data)
        d->data = d->source->parse(this);
    return d->data;
}

void KTimeZone::setData(KTimeZoneData *data)
{
    if (d->data)
        delete d->data;
    d->data = data;
}

bool KTimeZone::parse() const
{
    delete d->data;
    d->data = d->source->parse(this);
    return d->data;
}

QDateTime KTimeZone::toUtc(const QDateTime &zoneDateTime) const
{
    if (!zoneDateTime.isValid()  ||  zoneDateTime.timeSpec() != Qt::LocalTime)
        return QDateTime();
    int secs = offsetAtZoneTime(zoneDateTime);
    if (secs == InvalidOffset)
        return QDateTime();
    QDateTime dt = zoneDateTime;
    dt.setTimeSpec(Qt::UTC);
    return dt.addSecs(-secs);
}

QDateTime KTimeZone::toZoneTime(const QDateTime &utcDateTime, bool *secondOccurrence) const
{
    if (secondOccurrence)
        *secondOccurrence = false;
    if (!utcDateTime.isValid()  ||  utcDateTime.timeSpec() != Qt::UTC)    // check for invalid time
        return QDateTime();

    // Convert UTC to local time
    if (hasTransitions())
    {
        if (!data(true))
        {
            // No data - default to UTC
            QDateTime dt = utcDateTime;
            dt.setTimeSpec(Qt::LocalTime);
            return dt;
        }

        int index = d->data->transitionIndex(utcDateTime);
        int secs = (index >= 0) ? d->data->transitions()[index].phase().utcOffset() : d->data->previousUtcOffset();
        QDateTime dt = utcDateTime.addSecs(secs);
        if (secondOccurrence)
        {
            // Check whether the local time occurs twice around a daylight savings time
            // shift, and if so, whether it's the first or second occurrence.
            *secondOccurrence = d->data->d->isSecondOccurrence(dt, index);
        }
        dt.setTimeSpec(Qt::LocalTime);
        return dt;
    }
    else
    {
        int secs = offsetAtUtc(utcDateTime);
        QDateTime dt = utcDateTime.addSecs(secs);
        dt.setTimeSpec(Qt::LocalTime);
        if (secondOccurrence)
        {
            // Check whether the local time occurs twice around a daylight savings time
            // shift, and if so, whether it's the first or second occurrence.
            *secondOccurrence = (secs != offsetAtZoneTime(dt));
        }
        return dt;
    }
}

QDateTime KTimeZone::convert(const KTimeZone *newZone, const QDateTime &zoneDateTime) const
{
    if (newZone == this)
    {
        if (zoneDateTime.timeSpec() != Qt::LocalTime)
            return QDateTime();
        return zoneDateTime;
    }
    return newZone->toZoneTime(toUtc(zoneDateTime));
}

int KTimeZone::offsetAtZoneTime(const QDateTime &zoneDateTime, int *secondOffset) const
{
    if (!zoneDateTime.isValid()  ||  zoneDateTime.timeSpec() != Qt::LocalTime)    // check for invalid time
    {
        if (secondOffset)
            *secondOffset = 0;
        return 0;
    }
    bool validTime;
    if (secondOffset)
    {
        const KTimeZone::Transition *tr2;
        const KTimeZone::Transition *tr = transition(zoneDateTime, &tr2, &validTime);
        if (!tr)
        {
            if (!validTime)
                *secondOffset = InvalidOffset;
            else
                *secondOffset = d->data ? d->data->previousUtcOffset() : 0;
            return *secondOffset;
        }
        int offset = tr->phase().utcOffset();
        *secondOffset = tr2 ? tr2->phase().utcOffset() : offset;
        return offset;
    }
    else
    {
        const KTimeZone::Transition *tr = transition(zoneDateTime, 0, &validTime);
        if (!tr)
        {
            if (!validTime)
                return InvalidOffset;
            return d->data ? d->data->previousUtcOffset() : 0;
        }
        return tr->phase().utcOffset();
    }
}

int KTimeZone::offsetAtUtc(const QDateTime &utcDateTime) const
{
    if (!utcDateTime.isValid()  ||  utcDateTime.timeSpec() != Qt::UTC)    // check for invalid time
        return 0;
    const KTimeZone::Transition *tr = transition(utcDateTime);
    if (!tr)
        return d->data ? d->data->previousUtcOffset() : 0;
    return tr->phase().utcOffset();
}

int KTimeZone::offset(time_t t) const
{
    return offsetAtUtc(fromTime_t(t));
}

int KTimeZone::currentOffset(Qt::TimeSpec basis) const
{
    // Get current offset of this time zone to UTC
    time_t now = time(0);
    int secs = offset(now);

    switch (basis)
    {
        case Qt::LocalTime:
            // Return the current offset of this time zone to the local system time
            return secs - gmtoff(now);
        case Qt::UTC:
            // Return the current offset of this time zone to UTC
            return secs;

        default:
            break;
    }
    return 0;
}

bool KTimeZone::isDstAtUtc(const QDateTime &utcDateTime) const
{
    if (!utcDateTime.isValid()  ||  utcDateTime.timeSpec() != Qt::UTC)    // check for invalid time
        return false;
    const KTimeZone::Transition *tr = transition(utcDateTime);
    if (!tr)
        return false;
    return tr->phase().isDst();
}

bool KTimeZone::isDst(time_t t) const
{
    return isDstAtUtc(fromTime_t(t));
}

QDateTime KTimeZone::fromTime_t(time_t t)
{
    static QDate epochDate(1970,1,1);
    static QTime epochTime(0,0,0);
    int days = t / 86400;
    int secs;
    if (t >= 0)
        secs = t % 86400;
    else
    {
        secs = 86400 - (-t % 86400);
        --days;
    }
    return QDateTime(epochDate.addDays(days), epochTime.addSecs(secs), Qt::UTC);
}

time_t KTimeZone::toTime_t(const QDateTime &utcDateTime)
{
    static QDate epochDate(1970,1,1);
    static QTime epochTime(0,0,0);
    if (utcDateTime.timeSpec() != Qt::UTC)
        return InvalidTime_t;
    qint64 days = epochDate.daysTo(utcDateTime.date());
    qint64 secs = epochTime.secsTo(utcDateTime.time());
    qint64 t64 = days * 86400 + secs;
    time_t t = static_cast<time_t>(t64);
    if (static_cast<qint64>(t) != t64)
        return InvalidTime_t;
    return t;
}


/******************************************************************************/

KTimeZoneData *KTimeZoneSource::parse(const KTimeZone *) const
{
    return new KTimeZoneData;
}


/******************************************************************************/

class KTimeZoneLeapSecondsPrivate
{
    public:
        QDateTime  dt;         // UTC time when this change occurred
        QString    comment;    // optional comment
        int        seconds;    // number of leap seconds
};


KTimeZone::LeapSeconds::LeapSeconds()
  : d(new KTimeZoneLeapSecondsPrivate)
{
}

KTimeZone::LeapSeconds::LeapSeconds(const QDateTime &utc, int leap, const QString &cmt)
  : d(new KTimeZoneLeapSecondsPrivate)
{
    if (utc.timeSpec() == Qt::UTC)   // invalid if start time is not UTC
    {
        d->dt      = utc;
        d->comment = cmt;
        d->seconds = leap;
    }
}

KTimeZone::LeapSeconds::LeapSeconds(const KTimeZone::LeapSeconds &c)
  : d(new KTimeZoneLeapSecondsPrivate)
{
    d->dt      = c.d->dt;
    d->comment = c.d->comment;
    d->seconds = c.d->seconds;
}

KTimeZone::LeapSeconds::~LeapSeconds()
{
    delete d;
}

KTimeZone::LeapSeconds &KTimeZone::LeapSeconds::operator=(const KTimeZone::LeapSeconds &c)
{
    d->dt      = c.d->dt;
    d->comment = c.d->comment;
    d->seconds = c.d->seconds;
    return *this;
}

bool KTimeZone::LeapSeconds::operator<(const KTimeZone::LeapSeconds& c) const
{
    return d->dt < c.d->dt;
}

QDateTime KTimeZone::LeapSeconds::dateTime() const
{
    return d->dt;
}

bool KTimeZone::LeapSeconds::isValid() const
{
    return d->dt.isValid();
}

int KTimeZone::LeapSeconds::leapSeconds() const
{
    return d->seconds;
}

QString KTimeZone::LeapSeconds::comment() const
{
    return d->comment;
}


/******************************************************************************/


int KTimeZoneDataPrivate::transitionIndex(const QDateTime &dt) const
{
    // Do a binary search to find the last transition before this date/time
    int start = -1;
    int end = transitions.count();
    if (dt.timeSpec() == Qt::UTC)
    {
        while (end - start > 1)
        {
            int i = (start + end) / 2;
            if (dt < transitions[i].time())
                end = i;
            else
                start = i;
        }
    }
    else
    {
        QDateTime dtutc = dt;
        dtutc.setTimeSpec(Qt::UTC);
        while (end - start > 1)
        {
            int i = (start + end) / 2;
            if (dtutc.addSecs(-transitions[i].phase().utcOffset()) < transitions[i].time())
                end = i;
            else
                start = i;
        }
    }
    return end ? start : -1;
}

/* Check if it's a local time which occurs both before and after the specified
 * transition (for which it has to span a daylight saving to standard time change).
 * @param utcLocalTime local time set to Qt::UTC
 */
bool KTimeZoneDataPrivate::isSecondOccurrence(const QDateTime &utcLocalTime, int transitionIndex) const
{
    if (transitionIndex < 0)
        return false;
    int offset = transitions[transitionIndex].phase().utcOffset();
    int prevoffset = (transitionIndex > 0) ? transitions[transitionIndex-1].phase().utcOffset() : preUtcOffset;
    int phaseDiff = prevoffset - offset;
    if (phaseDiff <= 0)
        return false;
    // Find how long after the start of the latest phase 'dt' is
    int afterStart = transitions[transitionIndex].time().secsTo(utcLocalTime) - offset;
    return (afterStart < phaseDiff);
}



KTimeZoneData::KTimeZoneData()
  : d(new KTimeZoneDataPrivate)
{ }

KTimeZoneData::KTimeZoneData(const KTimeZoneData &c)
  : d(new KTimeZoneDataPrivate)
{
    d->phases        = c.d->phases;
    d->transitions   = c.d->transitions;
    d->leapChanges   = c.d->leapChanges;
    d->utcOffsets    = c.d->utcOffsets;
    d->abbreviations = c.d->abbreviations;
    d->preUtcOffset  = c.d->preUtcOffset;
}

KTimeZoneData::~KTimeZoneData()
{
    delete d;
}

KTimeZoneData &KTimeZoneData::operator=(const KTimeZoneData &c)
{
    d->phases        = c.d->phases;
    d->transitions   = c.d->transitions;
    d->leapChanges   = c.d->leapChanges;
    d->utcOffsets    = c.d->utcOffsets;
    d->abbreviations = c.d->abbreviations;
    d->preUtcOffset  = c.d->preUtcOffset;
    return *this;
}

KTimeZoneData *KTimeZoneData::clone()
{
    return new KTimeZoneData(*this);
}

QList<QByteArray> KTimeZoneData::abbreviations() const
{
    if (d->abbreviations.isEmpty())
    {
        for (int i = 0, end = d->phases.count();  i < end;  ++i)
        {
            QList<QByteArray> abbrevs = d->phases[i].abbreviations();
            for (int j = 0, jend = abbrevs.count();  j < jend;  ++j)
                if (!d->abbreviations.contains(abbrevs[j]))
                    d->abbreviations.append(abbrevs[j]);
        }
        if (d->abbreviations.isEmpty())
            d->abbreviations += "UTC";
    }
    return d->abbreviations;
}

QByteArray KTimeZoneData::abbreviation(const QDateTime &utcDateTime) const
{
    if (d->phases.isEmpty())
        return "UTC";
    const KTimeZone::Transition *tr = transition(utcDateTime);
    if (!tr)
        return QByteArray();
    QList<QByteArray> abbrevs = tr->phase().abbreviations();
    if (abbrevs.isEmpty())
        return QByteArray();
    return abbrevs[0];
}

QList<int> KTimeZoneData::utcOffsets() const
{
    if (d->utcOffsets.isEmpty())
    {
        for (int i = 0, end = d->phases.count();  i < end;  ++i)
        {
            int offset = d->phases[i].utcOffset();
            if (!d->utcOffsets.contains(offset))
                d->utcOffsets.append(offset);
        }
        if (d->utcOffsets.isEmpty())
            d->utcOffsets += 0;
        else
            qSort(d->utcOffsets);
    }
    return d->utcOffsets;
}

QList<KTimeZone::Phase> KTimeZoneData::phases() const
{
    return d->phases;
}

void KTimeZoneData::setPhases(const QList<KTimeZone::Phase> &phases, int previousUtcOffset)
{
    d->phases = phases;
    d->preUtcOffset = previousUtcOffset;
}

bool KTimeZoneData::hasTransitions() const
{
    return false;
}

QList<KTimeZone::Transition> KTimeZoneData::transitions() const
{
    return d->transitions;
}

void KTimeZoneData::setTransitions(const QList<KTimeZone::Transition> &transitions)
{
    d->transitions = transitions;
}

int KTimeZoneData::previousUtcOffset() const
{
    return d->preUtcOffset;
}

const KTimeZone::Transition *KTimeZoneData::transition(const QDateTime &dt, const KTimeZone::Transition **secondTransition,
                                                       bool *validTime) const
{
    int secondIndex;
    int index = transitionIndex(dt, (secondTransition ? &secondIndex : 0), validTime);
    if (secondTransition)
        *secondTransition = (secondIndex >= 0) ? &d->transitions[secondIndex] : 0;
    return (index >= 0) ? &d->transitions[index] : 0;
}

int KTimeZoneData::transitionIndex(const QDateTime &dt, int *secondIndex, bool *validTime) const
{
    if (validTime)
        *validTime = true;

    // Find the last transition before this date/time
    int index = d->transitionIndex(dt);
    if (dt.timeSpec() == Qt::UTC)
    {
        if (secondIndex)
            *secondIndex = index;
        return index;
    }
    else
    {
        /* Check whether the specified local time actually occurs.
         * Find the start of the next phase, and check if it falls in the gap
         * between the two phases.
         */
        QDateTime dtutc = dt;
        dtutc.setTimeSpec(Qt::UTC);
        int count = d->transitions.count();
        int next = (index >= 0) ? index + 1 : 0;
        if (next < count)
        {
            KTimeZone::Phase nextPhase = d->transitions[next].phase();
            int offset = (index >= 0) ? d->transitions[index].phase().utcOffset() : d->preUtcOffset;
            int phaseDiff = nextPhase.utcOffset() - offset;
            if (phaseDiff > 0)
            {
                // Get UTC equivalent as if 'dt' was in the next phase
                if (dtutc.secsTo(d->transitions[next].time()) + nextPhase.utcOffset() < phaseDiff)
                {
                    // The time falls in the gap between the two phases,
                    // so return an invalid value.
                    if (validTime)
                        *validTime = false;
                    if (secondIndex)
                        *secondIndex = -1;
                    return -1;
                }
            }
        }

        if (index < 0)
        {
            // The specified time is before the first phase
            if (secondIndex)
                *secondIndex = -1;
            return -1;
        }

        /* Check if it's a local time which occurs both before and after the 'latest'
         * phase start time (for which it has to span a daylight saving to standard
         * time change).
         */
        bool duplicate = true;
        if (d->isSecondOccurrence(dtutc, index))
        {
            // 'dt' occurs twice
            if (secondIndex)
            {
                *secondIndex = index;
                duplicate = false;
            }
            // Get the transition containing the first occurrence of 'dt'
            if (index <= 0)
                return -1;   // first occurrence of 'dt' is just before the first transition
            --index;
        }

        if (secondIndex  &&  duplicate)
            *secondIndex = index;
        return index;
    }
}

QList<KTimeZone::LeapSeconds> KTimeZoneData::leapSecondChanges() const
{
    return d->leapChanges;
}

void KTimeZoneData::setLeapSecondChanges(const QList<KTimeZone::LeapSeconds> &adjusts)
{
    d->leapChanges = adjusts;
}

KTimeZone::LeapSeconds KTimeZoneData::leapSecondChange(const QDateTime &utc) const
{
    if (utc.timeSpec() != Qt::UTC)
        kError() << "KTimeZoneData::leapSecondChange(): non-UTC time specified" << endl;
    else
    {
        for (int i = d->leapChanges.count();  --i >= 0;  )
        {
            if (d->leapChanges[i].dateTime() < utc)
                return d->leapChanges[i];
        }
    }
    return KTimeZone::LeapSeconds();
}
