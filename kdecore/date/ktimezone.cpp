/*
   This file is part of the KDE libraries
   Copyright (c) 2005-2008 David Jarvie <djarvie@kde.org>
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

#include "ktimezone.h"

#include <config.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include <climits>
#include <cstdlib>

#include <QtCore/QSet>
#include <QtCore/QSharedData>
#include <QtCore/QCoreApplication>

#include <kdebug.h>

int gmtoff(time_t t);   // defined in ksystemtimezone.cpp


/******************************************************************************/

class KTimeZonesPrivate
{
public:
    KTimeZonesPrivate() {}

    KTimeZones::ZoneMap zones;
};


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
    return d->zones;
}

bool KTimeZones::add(const KTimeZone &zone)
{
    if (!zone.isValid())
        return false;
    if (d->zones.find(zone.name()) != d->zones.end())
        return false;    // name already exists
    d->zones.insert(zone.name(), zone);
    return true;
}

KTimeZone KTimeZones::remove(const KTimeZone &zone)
{
    if (zone.isValid())
    {
        for (ZoneMap::Iterator it = d->zones.begin(), end = d->zones.end();  it != end;  ++it)
        {
            if (it.value() == zone)
            {
                d->zones.erase(it);
                return zone;
            }
        }
    }
    return KTimeZone();
}

KTimeZone KTimeZones::remove(const QString &name)
{
    if (!name.isEmpty())
    {
        ZoneMap::Iterator it = d->zones.find(name);
        if (it != d->zones.end())
        {
            KTimeZone zone = it.value();
            d->zones.erase(it);
            return zone;
        }
    }
    return KTimeZone();
}

void KTimeZones::clear()
{
  d->zones.clear();
}

KTimeZone KTimeZones::zone(const QString &name) const
{
    if (!name.isEmpty())
    {
        ZoneMap::ConstIterator it = d->zones.constFind(name);
        if (it != d->zones.constEnd())
            return it.value();
        if (name == KTimeZone::utc().name())
            return KTimeZone::utc();
    }
    return KTimeZone();    // error
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
        bool transitionIndexes(const QDateTime &start, const QDateTime &end, int &ixstart, int &ixend) const;
        bool isSecondOccurrence(const QDateTime &utcLocalTime, int transitionIndex) const;
};


/******************************************************************************/

class KTimeZonePrivate : public QSharedData
{
public:
    KTimeZonePrivate() : source(0), data(0), refCount(1) {}
    KTimeZonePrivate(KTimeZoneSource *src, const QString& nam,
                     const QString &country, float lat, float lon, const QString &cmnt);
    KTimeZonePrivate(const KTimeZonePrivate &);
    ~KTimeZonePrivate()  { delete data; }
    KTimeZonePrivate &operator=(const KTimeZonePrivate &);
    static KTimeZoneSource *utcSource();
    static void cleanup();

    KTimeZoneSource *source;
    QString name;
    QString countryCode;
    QString comment;
    float   latitude;
    float   longitude;
    mutable KTimeZoneData *data;
    int refCount;

private:
    static KTimeZoneSource *mUtcSource;
};

KTimeZoneSource *KTimeZonePrivate::mUtcSource = 0;


KTimeZonePrivate::KTimeZonePrivate(KTimeZoneSource *src, const QString& nam,
                 const QString &country, float lat, float lon, const QString &cmnt)
  : source(src),
    name(nam),
    countryCode(country.toUpper()),
    comment(cmnt),
    latitude(lat),
    longitude(lon),
    data(0),
    refCount(1)
{
    // Detect duff values.
    if ( latitude > 90 || latitude < -90 )
        latitude = KTimeZone::UNKNOWN;
    if ( longitude > 180 || longitude < -180 )
        longitude = KTimeZone::UNKNOWN;
}

KTimeZonePrivate::KTimeZonePrivate(const KTimeZonePrivate &rhs)
    : QSharedData(rhs),
      source(rhs.source),
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
    {
        mUtcSource = new KTimeZoneSource;
        qAddPostRoutine(KTimeZonePrivate::cleanup);
    }
    return mUtcSource;
}

void KTimeZonePrivate::cleanup()
{
    delete mUtcSource;
}


/******************************************************************************/

KTimeZoneBackend::KTimeZoneBackend()
  : d(new KTimeZonePrivate)
{}

KTimeZoneBackend::KTimeZoneBackend(const QString &name)
  : d(new KTimeZonePrivate(KTimeZonePrivate::utcSource(), name, QString(), KTimeZone::UNKNOWN, KTimeZone::UNKNOWN, QString()))
{}

KTimeZoneBackend::KTimeZoneBackend(KTimeZoneSource *source, const QString &name,
        const QString &countryCode, float latitude, float longitude, const QString &comment)
  : d(new KTimeZonePrivate(source, name, countryCode, latitude, longitude, comment))
{}

KTimeZoneBackend::KTimeZoneBackend(const KTimeZoneBackend &other)
  : d(other.d)
{
    ++d->refCount;
}

KTimeZoneBackend::~KTimeZoneBackend()
{
    if (d && --d->refCount == 0)
        delete d;
    d = 0;
}

KTimeZoneBackend &KTimeZoneBackend::operator=(const KTimeZoneBackend &other)
{
    if (d != other.d)
    {
        if (--d->refCount == 0)
            delete d;
        d = other.d;
        ++d->refCount;
    }
    return *this;
}

QByteArray KTimeZoneBackend::type() const
{
    return "KTimeZone";
}

KTimeZoneBackend *KTimeZoneBackend::clone() const
{
    return new KTimeZoneBackend(*this);
}

int KTimeZoneBackend::offsetAtZoneTime(const KTimeZone* caller, const QDateTime &zoneDateTime, int *secondOffset) const
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
        const KTimeZone::Transition *tr = caller->transition(zoneDateTime, &tr2, &validTime);
        if (!tr)
        {
            if (!validTime)
                *secondOffset = KTimeZone::InvalidOffset;
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
        const KTimeZone::Transition *tr = caller->transition(zoneDateTime, 0, &validTime);
        if (!tr)
        {
            if (!validTime)
                return KTimeZone::InvalidOffset;
            return d->data ? d->data->previousUtcOffset() : 0;
        }
        return tr->phase().utcOffset();
    }
}

int KTimeZoneBackend::offsetAtUtc(const KTimeZone* caller, const QDateTime &utcDateTime) const
{
    if (!utcDateTime.isValid()  ||  utcDateTime.timeSpec() != Qt::UTC)    // check for invalid time
        return 0;
    const KTimeZone::Transition *tr = caller->transition(utcDateTime);
    if (!tr)
        return d->data ? d->data->previousUtcOffset() : 0;
    return tr->phase().utcOffset();
}

int KTimeZoneBackend::offset(const KTimeZone* caller, time_t t) const
{
    return offsetAtUtc(caller, KTimeZone::fromTime_t(t));
}

bool KTimeZoneBackend::isDstAtUtc(const KTimeZone* caller, const QDateTime &utcDateTime) const
{
    if (!utcDateTime.isValid()  ||  utcDateTime.timeSpec() != Qt::UTC)    // check for invalid time
        return false;
    const KTimeZone::Transition *tr = caller->transition(utcDateTime);
    if (!tr)
        return false;
    return tr->phase().isDst();
}

bool KTimeZoneBackend::isDst(const KTimeZone* caller, time_t t) const
{
    return isDstAtUtc(caller, KTimeZone::fromTime_t(t));
}

bool KTimeZoneBackend::hasTransitions(const KTimeZone* caller) const
{
    Q_UNUSED(caller);
    return false;
}


/******************************************************************************/

#if SIZEOF_TIME_T == 8
const time_t KTimeZone::InvalidTime_t = 0x800000000000000LL;
#else
const time_t KTimeZone::InvalidTime_t = 0x80000000;
#endif
const int    KTimeZone::InvalidOffset = 0x80000000;
const float  KTimeZone::UNKNOWN = 1000.0;


KTimeZone::KTimeZone()
  : d(new KTimeZoneBackend())
{}

KTimeZone::KTimeZone(const QString &name)
  : d(new KTimeZoneBackend(name))
{}

KTimeZone::KTimeZone(const KTimeZone &tz)
  : d(tz.d->clone())
{}

KTimeZone::~KTimeZone()
{
    delete d;
}

KTimeZone::KTimeZone(KTimeZoneBackend *impl)
  : d(impl)
{
    // 'impl' should be a newly constructed object, with refCount = 1
    Q_ASSERT(d->d->refCount == 1);
}

KTimeZone &KTimeZone::operator=(const KTimeZone &tz)
{
    if (d != tz.d)
    {
        delete d;
        d = tz.d->clone();
    }
    return *this;
}

bool KTimeZone::operator==(const KTimeZone &rhs) const
{
    return d->d == rhs.d->d;
}

QByteArray KTimeZone::type() const
{
    return d->type();
}

bool KTimeZone::isValid() const
{
    return !d->d->name.isEmpty();
}

QString KTimeZone::countryCode() const
{
    return d->d->countryCode;
}

float KTimeZone::latitude() const
{
    return d->d->latitude;
}

float KTimeZone::longitude() const
{
    return d->d->longitude;
}

QString KTimeZone::comment() const
{
    return d->d->comment;
}

QString KTimeZone::name() const
{
    return d->d->name;
}

QList<QByteArray> KTimeZone::abbreviations() const
{
    if (!data(true))
        return QList<QByteArray>();
    return d->d->data->abbreviations();
}

QByteArray KTimeZone::abbreviation(const QDateTime &utcDateTime) const
{
    if (utcDateTime.timeSpec() != Qt::UTC  ||  !data(true))
        return QByteArray();
    return d->d->data->abbreviation(utcDateTime);
}

QList<int> KTimeZone::utcOffsets() const
{
    if (!data(true))
        return QList<int>();
    return d->d->data->utcOffsets();
}

QList<KTimeZone::Phase> KTimeZone::phases() const
{
    if (!data(true))
        return QList<KTimeZone::Phase>();
    return d->d->data->phases();
}

bool KTimeZone::hasTransitions() const
{
    return d->hasTransitions(this);
}

QList<KTimeZone::Transition> KTimeZone::transitions(const QDateTime &start, const QDateTime &end) const
{
    if (!data(true))
        return QList<KTimeZone::Transition>();
    return d->d->data->transitions(start, end);
}

const KTimeZone::Transition *KTimeZone::transition(const QDateTime &dt, const Transition **secondTransition,
                                                   bool *validTime ) const
{
    if (!data(true))
        return 0;
    return d->d->data->transition(dt, secondTransition, validTime);
}

int KTimeZone::transitionIndex(const QDateTime &dt, int *secondIndex, bool *validTime) const
{
    if (!data(true))
        return -1;
    return d->d->data->transitionIndex(dt, secondIndex, validTime);
}

QList<QDateTime> KTimeZone::transitionTimes(const Phase &phase, const QDateTime &start, const QDateTime &end) const
{
    if (!data(true))
        return QList<QDateTime>();
    return d->d->data->transitionTimes(phase, start, end);
}

QList<KTimeZone::LeapSeconds> KTimeZone::leapSecondChanges() const
{
    if (!data(true))
        return QList<KTimeZone::LeapSeconds>();
    return d->d->data->leapSecondChanges();
}

KTimeZoneSource *KTimeZone::source() const
{
    return d->d->source;
}

const KTimeZoneData *KTimeZone::data(bool create) const
{
    if (!isValid())
        return 0;
    if (create && !d->d->data && d->d->source->useZoneParse())
        d->d->data = d->d->source->parse(*this);
    return d->d->data;
}

void KTimeZone::setData(KTimeZoneData *data, KTimeZoneSource *source)
{
    if (!isValid())
        return;
    if (d->d->data)
        delete d->d->data;
    d->d->data = data;
    if (source)
        d->d->source = source;
}

bool KTimeZone::updateBase(const KTimeZone &other)
{
    if (d->d->name.isEmpty() || d->d->name != other.d->d->name)
        return false;
    d->d->countryCode = other.d->d->countryCode;
    d->d->comment     = other.d->d->comment;
    d->d->latitude    = other.d->d->latitude;
    d->d->longitude   = other.d->d->longitude;
    return true;
}

bool KTimeZone::parse() const
{
    if (!isValid())
        return false;
    if (d->d->source->useZoneParse())
    {
        delete d->d->data;
        d->d->data = d->d->source->parse(*this);
    }
    return d->d->data;
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

        int index = d->d->data->transitionIndex(utcDateTime);
        int secs = (index >= 0) ? d->d->data->transitions()[index].phase().utcOffset() : d->d->data->previousUtcOffset();
        QDateTime dt = utcDateTime.addSecs(secs);
        if (secondOccurrence)
        {
            // Check whether the local time occurs twice around a daylight savings time
            // shift, and if so, whether it's the first or second occurrence.
            *secondOccurrence = d->d->data->d->isSecondOccurrence(dt, index);
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

QDateTime KTimeZone::convert(const KTimeZone &newZone, const QDateTime &zoneDateTime) const
{
    if (newZone == *this)
    {
        if (zoneDateTime.timeSpec() != Qt::LocalTime)
            return QDateTime();
        return zoneDateTime;
    }
    return newZone.toZoneTime(toUtc(zoneDateTime));
}

int KTimeZone::offsetAtZoneTime(const QDateTime &zoneDateTime, int *secondOffset) const
{
    return d->offsetAtZoneTime(this, zoneDateTime, secondOffset);
}

int KTimeZone::offsetAtUtc(const QDateTime &utcDateTime) const
{
    return d->offsetAtUtc(this, utcDateTime);
}

int KTimeZone::offset(time_t t) const
{
    return d->offset(this, t);
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
    return d->isDstAtUtc(this, utcDateTime);
}

bool KTimeZone::isDst(time_t t) const
{
    return d->isDst(this, t);
}

KTimeZone KTimeZone::utc()
{
    static KTimeZone utcZone(QLatin1String("UTC"));
    return utcZone;
}

QDateTime KTimeZone::fromTime_t(time_t t)
{
    static const int secondsADay = 86400;
    static const QDate epochDate(1970,1,1);
    static const QTime epochTime(0,0,0);
    int days = t / secondsADay;
    int secs;
    if (t >= 0)
        secs = t % secondsADay;
    else
    {
        secs = secondsADay - (-t % secondsADay);
        --days;
    }
    return QDateTime(epochDate.addDays(days), epochTime.addSecs(secs), Qt::UTC);
}

time_t KTimeZone::toTime_t(const QDateTime &utcDateTime)
{
    static const QDate epochDate(1970,1,1);
    static const QTime epochTime(0,0,0);
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

class KTimeZoneSourcePrivate
{
public:
    bool mUseZoneParse;
};


KTimeZoneSource::KTimeZoneSource()
  : d(new KTimeZoneSourcePrivate)
{
    d->mUseZoneParse = true;
}

KTimeZoneSource::KTimeZoneSource(bool useZoneParse)
  : d(new KTimeZoneSourcePrivate)
{
    d->mUseZoneParse = useZoneParse;
}

KTimeZoneSource::~KTimeZoneSource()
{
    delete d;
}

KTimeZoneData *KTimeZoneSource::parse(const KTimeZone &) const
{
    Q_ASSERT(d->mUseZoneParse);  // method should never be called if it isn't usable
    return new KTimeZoneData;
}

bool KTimeZoneSource::useZoneParse() const
{
    return d->mUseZoneParse;
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

// Find the indexes to the transitions at or after start, and before or at end.
// start and end must be UTC.
// Reply = false if none.
bool KTimeZoneDataPrivate::transitionIndexes(const QDateTime &start, const QDateTime &end, int &ixstart, int &ixend) const
{
    ixstart = 0;
    if (start.isValid() && start.timeSpec() == Qt::UTC)
    {
        ixstart = transitionIndex(start);
        if (ixstart < 0)
            ixstart = 0;
        else if (transitions[ixstart].time() < start)
        {
            if (++ixstart >= transitions.count())
                return false;   // there are no transitions at/after 'start'
        }
    }
    ixend = -1;
    if (end.isValid() && end.timeSpec() == Qt::UTC)
    {
        ixend = transitionIndex(end);
        if (ixend < 0)
            return false;   // there are no transitions at/before 'end'
    }
    return true;
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

KTimeZoneData *KTimeZoneData::clone() const
{
    return new KTimeZoneData(*this);
}

QList<QByteArray> KTimeZoneData::abbreviations() const
{
    if (d->abbreviations.isEmpty())
    {
        for (int i = 0, end = d->phases.count();  i < end;  ++i)
        {
            const QList<QByteArray> abbrevs = d->phases[i].abbreviations();
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
    const QList<QByteArray> abbrevs = tr->phase().abbreviations();
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

QList<KTimeZone::Transition> KTimeZoneData::transitions(const QDateTime &start, const QDateTime &end) const
{
    int ixstart, ixend;
    if (!d->transitionIndexes(start, end, ixstart, ixend))
        return QList<KTimeZone::Transition>();   // there are no transitions within the time period
    if (ixend >= 0)
        return d->transitions.mid(ixstart, ixend - ixstart + 1);
    if (ixstart > 0)
        return d->transitions.mid(ixstart);
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

QList<QDateTime> KTimeZoneData::transitionTimes(const KTimeZone::Phase &phase, const QDateTime &start, const QDateTime &end) const
{
    QList<QDateTime> times;
    int ixstart, ixend;
    if (d->transitionIndexes(start, end, ixstart, ixend))
    {
        if (ixend < 0)
            ixend = d->transitions.count() - 1;
        while (ixstart <= ixend)
        {
            if (d->transitions[ixstart].phase() == phase)
                times += d->transitions[ixstart].time();
        }
    }
    return times;
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
