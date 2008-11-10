/*
   This file is part of the KDE libraries
   Copyright (c) 2005-2008 David Jarvie <djarvie@kde.org>

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

#include "ktzfiletimezone.h"

#include <config.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif

#include <QtCore/QFile>
#include <QtCore/QDataStream>
#include <QtCore/QVector>

#include <kdebug.h>


// Use this replacement for QDateTime::setTime_t(uint) since our time
// values are signed.
static QDateTime fromTime_t(qint32 seconds)
{
    static QDate epochDate(1970,1,1);
    static QTime epochTime(0,0,0);
    int secs = (seconds >= 0) ? seconds % 86400 : -(-seconds % 86400);
    return QDateTime(epochDate.addDays(seconds / 86400), epochTime.addSecs(secs), Qt::UTC);
}

/******************************************************************************/

KTzfileTimeZoneBackend::KTzfileTimeZoneBackend(KTzfileTimeZoneSource *source, const QString &name,
        const QString &countryCode, float latitude, float longitude, const QString &comment)
  : KTimeZoneBackend(source, name, countryCode, latitude, longitude, comment)
{}

KTzfileTimeZoneBackend::~KTzfileTimeZoneBackend()
{}

KTimeZoneBackend *KTzfileTimeZoneBackend::clone() const
{
    return new KTzfileTimeZoneBackend(*this);
}

QByteArray KTzfileTimeZoneBackend::type() const
{
    return "KTzfileTimeZone";
}

bool KTzfileTimeZoneBackend::hasTransitions(const KTimeZone *caller) const
{
    Q_UNUSED(caller)
    return true;
}


/******************************************************************************/

KTzfileTimeZone::KTzfileTimeZone(KTzfileTimeZoneSource *source, const QString &name,
        const QString &countryCode, float latitude, float longitude,
        const QString &comment)
  : KTimeZone(new KTzfileTimeZoneBackend(source, name, countryCode, latitude, longitude, comment))
{}

KTzfileTimeZone::~KTzfileTimeZone()
{}


/******************************************************************************/

class KTzfileTimeZoneDataPrivate
{
public:
};


KTzfileTimeZoneData::KTzfileTimeZoneData()
//  : d(new KTzfileTimeZoneDataPrivate)
{ }

KTzfileTimeZoneData::KTzfileTimeZoneData(const KTzfileTimeZoneData &rhs)
  : KTimeZoneData(rhs)
//    d(new KTzfileTimeZoneDataPrivate)
{
}

KTzfileTimeZoneData::~KTzfileTimeZoneData()
{
//    delete d;
}

KTzfileTimeZoneData &KTzfileTimeZoneData::operator=(const KTzfileTimeZoneData &rhs)
{
    KTimeZoneData::operator=(rhs);
    return *this;
}

KTimeZoneData *KTzfileTimeZoneData::clone() const
{
    return new KTzfileTimeZoneData(*this);
}

bool KTzfileTimeZoneData::hasTransitions() const
{
    return true;
}


/******************************************************************************/

class KTzfileTimeZoneSourcePrivate
{
public:
    KTzfileTimeZoneSourcePrivate(const QString &loc)
      : location(loc) {}
    ~KTzfileTimeZoneSourcePrivate() {}

    QString location;
};


KTzfileTimeZoneSource::KTzfileTimeZoneSource(const QString &location)
  : d(new KTzfileTimeZoneSourcePrivate(location))
{
    if (location.length() > 1  &&  location.endsWith('/'))
        d->location.chop(1);
}

KTzfileTimeZoneSource::~KTzfileTimeZoneSource()
{
    delete d;
}

QString KTzfileTimeZoneSource::location() const
{
    return d->location;
}

KTimeZoneData* KTzfileTimeZoneSource::parse(const KTimeZone &zone) const
{
    quint32 abbrCharCount;     // the number of characters of time zone abbreviation strings
    quint32 ttisgmtcnt;
    quint8  is;
    quint8  T_, Z_, i_, f_;    // tzfile identifier prefix

    QString path = zone.name();
    if (!path.startsWith('/'))
    {
        if (d->location == QLatin1String("/"))
            path.prepend(d->location);
        else
            path = d->location + '/' + path;
    }
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
    {
        kError() << "Cannot open " << f.fileName() << endl;
        return 0;
    }
    QDataStream str(&f);

    // Read the file type identifier
    str >> T_ >> Z_ >> i_ >> f_;
    if (T_ != 'T' || Z_ != 'Z' || i_ != 'i' || f_ != 'f')
    {
        kError() << "Not a TZFILE: " << f.fileName() << endl;
        return 0;
    }
    // Discard 16 bytes reserved for future use
    unsigned i;
    for (i = 0; i < 4; ++i)
        str >> ttisgmtcnt;

    KTzfileTimeZoneData* data = new KTzfileTimeZoneData;

    // Read the sizes of arrays held in the file
    quint32 nTransitionTimes;
    quint32 nLocalTimeTypes;
    quint32 nLeapSecondAdjusts;
    quint32 nIsStandard;
    quint32 nIsUtc;
    str >> nIsUtc
        >> nIsStandard
        >> nLeapSecondAdjusts
        >> nTransitionTimes
        >> nLocalTimeTypes
        >> abbrCharCount;
    // kDebug() << "header: " << nIsUtc << ", " << nIsStandard << ", " << nLeapSecondAdjusts << ", " <<
    //    nTransitionTimes << ", " << nLocalTimeTypes << ", " << abbrCharCount << endl;

    // Read the transition times, at which the rules for computing local time change
    struct TransitionTime
    {
        qint32 time;            // time (as returned by time(2)) at which the rules for computing local time change
        quint8 localTimeIndex;  // index into the LocalTimeType array
    };
//kDebug()<<"Reading zone "<<zone.name();
    TransitionTime *transitionTimes = new TransitionTime[nTransitionTimes];
    for (i = 0;  i < nTransitionTimes;  ++i)
    {
        str >> transitionTimes[i].time;
    }
    for (i = 0;  i < nTransitionTimes;  ++i)
    {
        str >> transitionTimes[i].localTimeIndex;
//kDebug() << "Transition time "<<i<<": "<<transitionTimes[i].time<<"   lt index="<<(int)transitionTimes[i].localTimeIndex;
    }

    // Read the local time types
    struct LocalTimeType
    {
        qint32 gmtoff;     // number of seconds to be added to UTC
        bool   isdst;      // whether tm_isdst should be set by localtime(3)
        quint8 abbrIndex;  // index into the list of time zone abbreviations
        bool   isutc;      // transition times are in UTC. If UTC, isstd is ignored.
        bool   isstd;      // if true, transition times are in standard time;
                           // if false, transition times are in wall clock time,
                           // i.e. standard time or daylight savings time
                           // whichever is current before the transition
    };
    LocalTimeType *localTimeTypes = new LocalTimeType[nLocalTimeTypes];
    LocalTimeType *ltt = localTimeTypes;
    for (i = 0;  i < nLocalTimeTypes;  ++ltt, ++i)
    {
        str >> ltt->gmtoff;
        str >> is;
        ltt->isdst = (is != 0);
        str >> ltt->abbrIndex;
        // kDebug() << "local type: " << ltt->gmtoff << ", " << is << ", " << ltt->abbrIndex;
        ltt->isstd = false;   // default if no data
        ltt->isutc = false;   // default if no data
    }

    // Read the timezone abbreviations. They are stored as null terminated strings in
    // a character array.
    // Make sure we don't fall foul of maliciously coded time zone abbreviations.
    if (abbrCharCount > 64)
    {
        kError() << "excessive length for timezone abbreviations: " << abbrCharCount << endl;
        delete data;
        delete[] transitionTimes;
        delete[] localTimeTypes;
        return 0;
    }
    QByteArray array(abbrCharCount, 0);
    str.readRawData(array.data(), array.size());
    char *abbrs = array.data();
    if (abbrs[abbrCharCount - 1] != 0)
    {
        // These abbreviations are corrupt!
        kError() << "timezone abbreviations not null terminated: " << abbrs[abbrCharCount - 1] << endl;
        delete data;
        delete[] transitionTimes;
        delete[] localTimeTypes;
        return 0;
    }
    quint8 n = 0;
    QList<QByteArray> abbreviations;
    for (i = 0;  i < abbrCharCount;  ++n, i += strlen(abbrs + i) + 1)
    {
        abbreviations += QByteArray(abbrs + i);
        // Convert the LocalTimeTypes pointer to a sequential index
        ltt = localTimeTypes;
        for (unsigned j = 0;  j < nLocalTimeTypes;  ++ltt, ++j)
        {
            if (ltt->abbrIndex == i)
                ltt->abbrIndex = n;
        }
    }


    // Read the leap second adjustments
    qint32  t;
    quint32 s;
    QList<KTimeZone::LeapSeconds> leapChanges;
    for (i = 0;  i < nLeapSecondAdjusts;  ++i)
    {
        str >> t >> s;
        // kDebug() << "leap entry: " << t << ", " << s;
        // Don't use QDateTime::setTime_t() because it takes an unsigned argument
        leapChanges += KTimeZone::LeapSeconds(fromTime_t(t), static_cast<int>(s));
    }
    data->setLeapSecondChanges(leapChanges);

    // Read the standard/wall time indicators.
    // These are true if the transition times associated with local time types
    // are specified as standard time, false if wall clock time.
    for (i = 0;  i < nIsStandard;  ++i)
    {
        str >> is;
        localTimeTypes[i].isstd = (is != 0);
        // kDebug() << "standard: " << is;
    }

    // Read the UTC/local time indicators.
    // These are true if the transition times associated with local time types
    // are specified as UTC, false if local time.
    for (i = 0;  i < nIsUtc;  ++i)
    {
        str >> is;
        localTimeTypes[i].isutc = (is != 0);
        // kDebug() << "UTC: " << is;
    }


    // Find the starting offset from UTC to use before the first transition time.
    // This is first non-daylight savings local time type, or if there is none,
    // the first local time type.
    int firstoffset = (nLocalTimeTypes > 0) ? localTimeTypes[0].gmtoff : 0;
    ltt = localTimeTypes;
    for (i = 0;  i < nLocalTimeTypes;  ++ltt, ++i)
    {
        if (!ltt->isdst)
        {
            firstoffset = ltt->gmtoff;
            break;
        }
    }

    // Compile the time type data into a list of KTimeZone::Phase instances.
    // Also check for local time types which are identical (this does happen)
    // and use the same Phase instance for each.
    QByteArray abbrev;
    QList<KTimeZone::Phase> phases;
    QList<QByteArray> phaseAbbrevs;
    QVector<int> lttLookup(nLocalTimeTypes);
    ltt = localTimeTypes;
    for (i = 0;  i < nLocalTimeTypes;  ++ltt, ++i)
    {
        if (ltt->abbrIndex >= abbreviations.count())
        {
            kError() << "KTzfileTimeZoneSource::parse(): abbreviation index out of range" << endl;
            abbrev = "???";
        }
        else
            abbrev = abbreviations[ltt->abbrIndex];
        // Check for an identical Phase
        int phindex = 0;
        for (int j = 0, jend = phases.count();  j < jend;  ++j, ++phindex)
        {
            if (ltt->gmtoff == phases[j].utcOffset()
            &&  (bool)ltt->isdst == phases[j].isDst()
            &&  abbrev == phaseAbbrevs[j])
                break;
        }
        lttLookup[i] = phindex;
        if (phindex == phases.count())
        {
            phases += KTimeZone::Phase(ltt->gmtoff, abbrev, ltt->isdst);
            phaseAbbrevs += abbrev;
        }
    }
    data->setPhases(phases, firstoffset);

    // Compile the transition list
    QList<KTimeZone::Transition> transitions;
    int stdoffset = firstoffset;
    int offset    = stdoffset;
    TransitionTime *tt = transitionTimes;
    for (i = 0;  i < nTransitionTimes;  ++tt, ++i)
    {
        if (tt->localTimeIndex >= nLocalTimeTypes)
        {
            kError() << "KTzfileTimeZoneSource::parse(): transition ignored: local time type out of range: " <<(int)tt->localTimeIndex<<" > "<<nLocalTimeTypes << endl;
            continue;
        }

        // Convert local transition times to UTC
        ltt = &localTimeTypes[tt->localTimeIndex];
        if (!ltt->isutc)
        {
            /* The transition time is in local time, so convert it to UTC.
             * If the transition is in "local wall clock time", use the UTC offset
             * set up by the previous transition; otherwise, the transition is in
             * standard time, so use the UTC offset set up by the last non-daylight
             * savings time transition.
             */
            tt->time -= ltt->isstd ? stdoffset : offset;
            offset = ltt->gmtoff;     // keep note of latest offset
            if (!ltt->isdst)
                stdoffset = offset;   // keep note of latest standard time offset
        }

        KTimeZone::Phase phase = phases[lttLookup[tt->localTimeIndex]];
//kDebug(161) << "Transition time "<<i<<": "<<fromTime_t(tt->time)<<", offset="<<phase.utcOffset()/60;
        transitions += KTimeZone::Transition(fromTime_t(tt->time), phase);
    }
    data->setTransitions(transitions);
//for(int xxx=1;xxx<data->transitions().count();xxx++)
//kDebug(161) << "Transition time "<<xxx<<": "<<data->transitions()[xxx].time()<<", offset="<<data->transitions()[xxx].phase().utcOffset()/60;
    delete[] localTimeTypes;
    delete[] transitionTimes;

    return data;
}
