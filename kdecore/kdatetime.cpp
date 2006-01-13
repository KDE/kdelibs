/*
    This file is part of the KDE libraries
    Copyright (c) 2005 David Jarvie <software@astrojar.org.uk>

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

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <QDateTime>
#include <QRegExp>
#include <QStringList>
#include <kglobal.h>
#include <klocale.h>
#include <kcalendarsystemgregorian.h>
#include <kdebug.h>
#include <kdatetime.h>


static const QLatin1String shortDay[] = {
    QLatin1String("Mon"), QLatin1String("Tue"), QLatin1String("Wed"),
    QLatin1String("Thu"), QLatin1String("Fri"), QLatin1String("Sat"),
    QLatin1String("Sun")
};
static const QLatin1String longDay[] = {
    QLatin1String("Monday"), QLatin1String("Tuesday"), QLatin1String("Wednesday"),
    QLatin1String("Thursday"), QLatin1String("Friday"), QLatin1String("Saturday"),
    QLatin1String("Sunday")
};
static const QLatin1String shortMonth[] = {
    QLatin1String("Jan"), QLatin1String("Feb"), QLatin1String("Mar"), QLatin1String("Apr"),
    QLatin1String("May"), QLatin1String("Jun"), QLatin1String("Jul"), QLatin1String("Aug"),
    QLatin1String("Sep"), QLatin1String("Oct"), QLatin1String("Nov"), QLatin1String("Dec")
};
static const QLatin1String longMonth[] = {
    QLatin1String("January"), QLatin1String("February"), QLatin1String("March"),
    QLatin1String("April"), QLatin1String("May"), QLatin1String("June"),
    QLatin1String("July"), QLatin1String("August"), QLatin1String("September"),
    QLatin1String("October"), QLatin1String("November"), QLatin1String("December")
};


// Time specification or invalid status type, stored in KDateTimePrivate::spec
enum Status {
    stUTC           = KDateTime::UTC,
    stOffsetFromUTC = KDateTime::OffsetFromUTC,
    stTimeZone      = KDateTime::TimeZone,
    stClockTime     = KDateTime::ClockTime,
    stTooEarly      = -2,     // invalid (valid date before QDate range)
    stTooLate       = -3,     // invalid (valid date after QDate range)
    stInvalid       = -1      // really invalid
};


static QDateTime fromStr(const QString& string, const QString& format, int& utcOffset,
                         QString& zoneName, QByteArray& zoneAbbrev, bool& dateOnly, Status&);
static int matchDay(const QString &string, int &offset, KCalendarSystem*);
static int matchMonth(const QString &string, int &offset, KCalendarSystem*);
static bool getUTCOffset(const QString &string, int &offset, bool colon, int &result);
static int getAmPm(const QString &string, int &offset, KLocale*);
static bool getNumber(const QString &string, int &offset, int mindigits, int maxdigits, int minval, int maxval, int &result);
static int findString(const QString &string, const QLatin1String *array, int count, int &offset);
static QDate checkDate(int year, int month, int day, Status&);


/*----------------------------------------------------------------------------*/

class KDateTimePrivate
{
  public:
    KDateTimePrivate(Status s = stInvalid, const KTimezone *z = 0)   // default = invalid spec value
        : tz(z),
          spec(s),
          utcOffset(0),
          utcCached(false),
          mDateOnly(false)
    {}

    KDateTimePrivate(const QDateTime &d, Status s, const KTimezone *z, bool donly = false)   // default = invalid spec value
        : mDt(d),
          tz(z),
          spec(s),
          utcOffset(0),
          utcCached(false),
          mDateOnly(donly)
    {}

    KDateTimePrivate(const QDateTime &d, bool donly = false)   // default = invalid spec value
        : mDt(d),
          tz(0),
          utcOffset(0),
          utcCached(false),
          mDateOnly(donly)
    {}
    KDateTimePrivate(const KDateTimePrivate &rhs)
        : mDt(rhs.mDt),
          utc(rhs.utc),
          tz(rhs.tz),
          spec(rhs.spec),
          utcOffset(rhs.utcOffset),
          utcCached(rhs.utcCached),
          mDateOnly(rhs.mDateOnly)
    {}
    ~KDateTimePrivate()  {}
    KDateTimePrivate &operator=(const KDateTimePrivate&);
    const QDateTime& dt() const                                    { return mDt; }
    const QDate   date() const                                     { return mDt.date(); }
    bool      dateOnly() const                                     { return mDateOnly; }
    void      setDt(const QDateTime &dt)                           { mDt = dt; utcCached = false; }
    void      setDt(const QDateTime &dt, const QDateTime &utcDt)   { mDt = dt; utc = utcDt; utcCached = true; }
    void      setDate(const QDate &d)                              { mDt.setDate(d); utcCached = false; }
    void      setTime(const QTime &t)                              { mDt.setTime(t); mDateOnly = false; utcCached = false; }
    void      setDtTimeSpec(Qt::TimeSpec s)                        { mDt.setTimeSpec(s); utcCached = false; }
    void      setDateOnly(bool d);
    QDateTime toUTC(const KTimezone *local = 0) const;

    static KDateTime    fromStringDefault;   // default time spec used by fromString()
    static QTime        sod;     // start of day (00:00:00)

  private:
    QDateTime           mDt;
  public:
    mutable QDateTime   utc;     // cached UTC equivalent of 'dt'
    mutable const KTimezone *tz; // if spec == TimeZone, the instance's time zone.
                                 // if spec == ClockTime, the local time zone used to calculate the cached UTC time.
                                 // N.B. the KTimezone instance is not owned by KDateTime or KDateTimePrivate!
    Status              spec;        // time spec or invalid status type
    int                 utcOffset;
    mutable bool        utcCached;   // true if 'utc' is valid
  private:
    bool                mDateOnly;
};

KDateTime KDateTimePrivate::fromStringDefault(QDate(2000,1,1), KDateTime::ClockTime);
QTime     KDateTimePrivate::sod(0,0,0);

KDateTimePrivate &KDateTimePrivate::operator=(const KDateTimePrivate &other)
{
    mDt        = other.mDt;
    utc        = other.utc;
    tz         = other.tz;
    spec       = other.spec;
    utcOffset  = other.utcOffset;
    utcCached  = other.utcCached;
    mDateOnly = other.mDateOnly;
    return *this;
}

void KDateTimePrivate::setDateOnly(bool dateOnly)
{
    if (dateOnly != mDateOnly)
    {
        mDateOnly = dateOnly;
        if (dateOnly  &&  mDt.time() != sod)
        {
            mDt.setTime(sod);
            utcCached = false;
        }
    }
}

/*
 * Returns the date/time converted to UTC.
 * Depending on which KTimezone class is involved, conversion to UTC may require
 * significant calculation, so the calculated UTC value is cached.
 */
QDateTime KDateTimePrivate::toUTC(const KTimezone *local) const
{
    if (utcCached)
    {
        // Return cached UTC value
        if (spec == stClockTime)
        {
            // ClockTime uses the dynamic current local system time zone.
            // Check for a time zone change before using the cached UTC value.
            if (!local)
                local = KSystemTimezones::local();
            if (tz == local)
#ifdef KDATETIME_TEST
                kdDebug() << "toUTC(): cached -> " << utc << endl,
#endif
                return utc;
        }
        else
#ifdef KDATETIME_TEST
            kdDebug() << "toUTC(): cached -> " << utc << endl,
#endif
            return utc;
    }

    // No cached UTC value, so calculate it
    if (!mDt.isValid())
    {
        utc = mDt;
        utcCached = true;
#ifdef KDATETIME_TEST
        kdDebug() << "toUTC(): cached -> invalid" << endl;
#endif
        return mDt;
    }
    switch (spec)
    {
        case stUTC:
            return mDt;
        case stOffsetFromUTC:
            utc = QDateTime(mDt.date(), mDt.time(), Qt::UTC).addSecs(-utcOffset);
            utcCached = true;
#ifdef KDATETIME_TEST
            kdDebug() << "toUTC(): calculated -> " << utc << endl,
#endif
            return utc;
        case stClockTime:
            if (!local)
                local = KSystemTimezones::local();
            tz = local;
            // fall through to TimeZone
        case stTimeZone:
            utc = tz->toUTC(mDt);
            utcCached = true;
#ifdef KDATETIME_TEST
            kdDebug() << "toUTC(): calculated -> " << utc << endl,
#endif
            return utc;
        default:
            return QDateTime();
    }
}


/*----------------------------------------------------------------------------*/

KDateTime::KDateTime()
  : d(new KDateTimePrivate)
{
}

KDateTime::KDateTime(const QDate &date, const KTimezone *tz)
: d(new KDateTimePrivate(QDateTime(date, KDateTimePrivate::sod, Qt::LocalTime), (tz ? stTimeZone : stInvalid), tz, true))
{
}

KDateTime::KDateTime(const QDate &date, const QTime &time, const KTimezone *tz)
  : d(new KDateTimePrivate(QDateTime(date, time, Qt::LocalTime), (tz ? stTimeZone : stInvalid), tz))
{
}

KDateTime::KDateTime(const QDateTime &dt, const KTimezone *tz)
  : d(new KDateTimePrivate(dt, stTimeZone, tz))
{
    if (!tz)
        d->spec = stInvalid;   // set to invalid value
    else if (dt.timeSpec() == Qt::UTC)
    {
        d->setDt(tz->toZoneTime(dt), dt);
        d->utc = dt;
        d->utcCached = true;
    }
}

KDateTime::KDateTime(const QDate &date, TimeSpec spec, int utcOffset)
  : d(new KDateTimePrivate(QDateTime(date, KDateTimePrivate::sod, Qt::LocalTime), true))
{
    switch (spec)
    {
        case LocalZone:
            d->tz = KSystemTimezones::local();
            d->spec = stTimeZone;
            break;
        case ClockTime:
            d->spec = stClockTime;
            break;
        case UTC:
            d->setDtTimeSpec(Qt::UTC);
            d->spec = stUTC;
            break;
        case OffsetFromUTC:
            d->utcOffset = utcOffset;
            d->spec = stOffsetFromUTC;
            break;
        case TimeZone:
        default:
            d->spec = stInvalid;   // set to invalid value
            break;
    }
}

KDateTime::KDateTime(const QDate &date, const QTime &time, TimeSpec spec, int utcOffset)
  : d(new KDateTimePrivate(QDateTime(date, time, Qt::LocalTime)))
{
    switch (spec)
    {
        case LocalZone:
            d->tz = KSystemTimezones::local();
            d->spec = stTimeZone;
            break;
        case ClockTime:
            d->spec = stClockTime;
            break;
        case UTC:
            d->setDtTimeSpec(Qt::UTC);
            d->spec = stUTC;
            break;
        case OffsetFromUTC:
            d->utcOffset = utcOffset;
            d->spec = stOffsetFromUTC;
            break;
        case TimeZone:
        default:
            d->spec = stInvalid;   // set to invalid value
            break;
    }
}

KDateTime::KDateTime(const QDateTime &dt, TimeSpec spec, int utcOffset)
  : d(new KDateTimePrivate())
{
    switch (spec)
    {
        case LocalZone:
            d->tz = KSystemTimezones::local();
            d->spec = stTimeZone;
            if (dt.timeSpec() == Qt::UTC)
                d->setDt(d->tz->toZoneTime(dt), dt);
            else
                d->setDt(dt);
            break;
        case ClockTime:
            d->spec = stClockTime;
            if (dt.timeSpec() == Qt::UTC)
            {
                d->tz = KSystemTimezones::local();
                d->setDt(d->tz->toZoneTime(dt), dt);
            }
            else
                d->setDt(dt);
            break;
        case UTC:
            d->spec = stUTC;
            if (dt.timeSpec() == Qt::LocalTime)
                d->setDt(KSystemTimezones::local()->toUTC(dt));
            else
                d->setDt(dt);
            break;
        case OffsetFromUTC:
            d->spec = stOffsetFromUTC;
            d->utcOffset = utcOffset;
            if (dt.timeSpec() == Qt::UTC)
            {
                QDateTime local = dt.addSecs(utcOffset);
                local.setTimeSpec(Qt::LocalTime);
                d->setDt(local, dt);
            }
            else
                d->setDt(dt);
            break;
        case TimeZone:
            d->spec = stInvalid;   // set to invalid value
            break;
    }
}

KDateTime::KDateTime(const QDateTime &dt)
  : d(new KDateTimePrivate(dt))
{
    if (dt.timeSpec() == Qt::LocalTime)
    {
        d->spec = stTimeZone;
        d->tz = KSystemTimezones::local();
    }
    else
        d->spec = stUTC;
}

KDateTime::KDateTime(const KDateTime &other)
  : d(new KDateTimePrivate(*other.d))
{
}

KDateTime::~KDateTime()
{
  delete d;
}

KDateTime &KDateTime::operator=(const KDateTime &other)
{
    d->operator=(*other.d);
    return *this;
}

bool      KDateTime::isNull() const       { return d->dt().isNull(); }
bool      KDateTime::isValid() const      { return d->spec >= 0 && d->dt().isValid(); }
bool      KDateTime::isTooEarly() const   { return d->spec == stTooEarly; }
bool      KDateTime::isTooLate() const    { return d->spec == stTooLate; }
bool      KDateTime::isDateOnly() const   { return d->dateOnly(); }
QDate     KDateTime::date() const         { return d->date(); }
QTime     KDateTime::time() const         { return d->dt().time(); }
QDateTime KDateTime::dateTime() const     { return d->dt(); }

KDateTime::TimeSpec KDateTime::timeSpec() const
{
    if (d->spec == stTimeZone)
    {
        if (d->tz == KTimezones::utc())
            return UTC;
        if (d->tz == KSystemTimezones::local())
            return LocalZone;
    }
    else if (d->spec < 0)
        return LocalZone;    // actually invalid, but we have to return something
    return static_cast<KDateTime::TimeSpec>(d->spec);
}

bool KDateTime::isLocalZone() const
{
    return (d->spec == stTimeZone  &&  d->tz == KSystemTimezones::local());
}

bool KDateTime::isClockTime() const
{
    return (d->spec == stClockTime);
}

bool KDateTime::isUTC() const
{
    if (d->spec == stUTC)
        return true;
    if (d->spec == stTimeZone  &&  d->tz == KTimezones::utc())
        return true;
    return false;
}

bool KDateTime::isOffsetFromUTC() const
{
    return (d->spec == stOffsetFromUTC);
}

int KDateTime::UTCOffset() const
{
    switch (d->spec)
    {
        case stTimeZone:
            return d->tz->offsetAtZoneTime(d->dt());
        case stOffsetFromUTC:
            return d->utcOffset;
        case stUTC:
        case stClockTime:
        default:
            return 0;
    }
}

const KTimezone *KDateTime::timeZone() const
{
    if (d->spec == stTimeZone)
        return d->tz;
    if (d->spec == stUTC)
        return KTimezones::utc();
    return 0;
}

KDateTime KDateTime::toUTC() const
{
    if (d->spec == stUTC)
        return *this;
    if (d->dateOnly())
    {
        if (!isValid())
            return KDateTime();
        return KDateTime(d->date(), UTC);
    }
    QDateTime udt = d->toUTC();
    if (!udt.isValid())
        return KDateTime();
    return KDateTime(udt, UTC);
}

KDateTime KDateTime::toLocalZone() const
{
    const KTimezone *local = KSystemTimezones::local();
    if (d->spec == stTimeZone  &&  d->tz == local)
        return *this;    // preserve UTC cache, if any
    if (d->dateOnly())
    {
        if (!isValid())
            return KDateTime();
        return KDateTime(d->date(), local);
    }
    switch (d->spec)
    {
        case stTimeZone:
            /* Convert the time using zone->toZoneTime(utc) rather than d->tz->convert(),
             * because convert() works by converting to UTC first, and this way we can use
             * the cached UTC time rather than having convert() recalculate it every time.
             */
        case stOffsetFromUTC:
        case stUTC:
            return KDateTime(d->toUTC(), local);
        case stClockTime:
            return KDateTime(d->dt(), local);
        default:
            return KDateTime();
    }
}

KDateTime KDateTime::toClockTime() const
{
    if (d->spec == stClockTime)
        return *this;
    if (d->dateOnly())
    {
        if (!isValid())
            return KDateTime();
        return KDateTime(d->date(), ClockTime);
    }
    KDateTime result = toLocalZone();
    result.d->spec = stClockTime;   // cached value (if any) is unaffected, and d->tz is set to local zone
    return result;
}

KDateTime KDateTime::toZone(const KTimezone *zone) const
{
    if (!zone)
        return KDateTime();
    if (d->spec == stTimeZone  &&  d->tz == zone)
        return *this;    // preserve UTC cache, if any
    if (d->dateOnly())
    {
        if (!isValid())
            return KDateTime();
        return KDateTime(d->date(), zone);
    }
    switch (d->spec)
    {
        case stTimeZone:
            /* Convert the time using zone->toZoneTime(utc) rather than d->tz->convert(),
             * because convert() works by converting to UTC first, and this way we can use
             * the cached UTC time rather than having convert() recalculate it every time.
             */
        case stOffsetFromUTC:
        case stUTC:
        case stClockTime:
            return KDateTime(d->toUTC(), zone);
        default:
            return KDateTime();
    }
}

KDateTime KDateTime::toTimeSpec(const KDateTime &other) const
{
    if (compareTimeSpec(other))
        return *this;
    if (!isValid())
        return KDateTime();
    if (d->dateOnly())
    {
        if (other.d->spec == stTimeZone)
            return KDateTime(d->date(), other.d->tz);
        else
            return KDateTime(d->date(), static_cast<KDateTime::TimeSpec>(other.d->spec), other.d->utcOffset);
    }
    if (other.d->spec == stTimeZone)
        return KDateTime(d->toUTC(), other.d->tz);
    else
        return KDateTime(d->toUTC(), static_cast<KDateTime::TimeSpec>(other.d->spec), other.d->utcOffset);
}

uint KDateTime::toTime_t() const
{
    return d->toUTC().toTime_t();
}

void KDateTime::setDateOnly(bool dateOnly)
{
    d->setDateOnly(dateOnly);
}

void KDateTime::setDate(const QDate &date)
{
    d->setDate(date);
}

void KDateTime::setTime(const QTime &time)
{
    d->setTime(time);
}

void KDateTime::setDateTime(const QDateTime &dt)
{
    d->utcCached = false;
    d->setDateOnly(false);
    if (dt.timeSpec() == Qt::LocalTime)
    {
        if (d->spec == stUTC)
            d->setDt(KSystemTimezones::local()->toUTC(dt));
        else
            d->setDt(dt);
    }
    else
    {
        if (d->spec == stUTC)
            d->setDt(dt);
        else
        {
            const KTimezone *tz = KSystemTimezones::local();
            d->setDt(tz->toZoneTime(dt), dt);
            if (d->spec == stClockTime)
                d->tz = tz;    // note time zone for cached UTC value
        }
    }
}

void KDateTime::setTimeSpec(TimeSpec spec, int utcOffset)
{
    if (spec == static_cast<KDateTime::TimeSpec>(d->spec))
    {
        if (spec == OffsetFromUTC  &&  utcOffset != d->utcOffset)
        {
            d->utcOffset = utcOffset;
            d->utcCached = false;
        }
        return;
    }

    switch (spec)
    {
        case UTC:
            d->spec = stUTC;
            d->tz = 0;
            d->setDtTimeSpec(Qt::UTC);    // this clears cached UTC value
            return;
        case OffsetFromUTC:
            d->spec = stOffsetFromUTC;;
            d->utcOffset = utcOffset;
            d->tz = 0;
            d->setDtTimeSpec(Qt::LocalTime);    // this clears cached UTC value
            return;
        case LocalZone:
        {
            const KTimezone *local = KSystemTimezones::local();
            if (d->spec == stTimeZone  &&  d->tz == local)
                return;
            d->spec = stTimeZone;
            d->tz = local;
            break;
        }
        case ClockTime:
            d->spec = stClockTime;
            break;
        case TimeZone:
            return;   // invalid operation
    }
    d->utcOffset = 0;
    d->setDtTimeSpec(Qt::LocalTime);    // this clears cached UTC value
}

void KDateTime::setTimeSpec(const KTimezone *tz)
{
    if (tz  &&  (d->spec != stTimeZone || tz != d->tz))
    {
        d->spec = stTimeZone;
        d->tz = tz;
        d->utcOffset = 0;
        d->setDtTimeSpec(Qt::LocalTime);    // this clears cached UTC value
    }
}

void KDateTime::setTimeSpec(const KDateTime &other)
{
    if (other.d->spec != d->spec
    ||  d->spec == stOffsetFromUTC && other.d->utcOffset != d->utcOffset
    ||  d->spec == stTimeZone && other.d->tz != d->tz)
    {
        d->tz        = other.d->tz;
        d->spec      = other.d->spec;
        d->utcOffset = other.d->utcOffset;
        d->setDtTimeSpec(other.d->dt().timeSpec());    // this clears cached UTC value
    }
}

bool KDateTime::compareTimeSpec(const KDateTime &other) const
{
    if (d->spec == other.d->spec)
    {
        switch (d->spec)
        {
            case stTimeZone:
                if (d->tz == other.d->tz)
                    return true;
                break;
            case stOffsetFromUTC:
                if (d->utcOffset == other.d->utcOffset)
                    return true;
                break;
            case stUTC:
            case stClockTime:
            default:   // invalid times
                return true;
        }
    }
    return false;
}

KDateTime KDateTime::addSecs(int secs) const
{
    if (!isValid())
        return KDateTime();
    if (d->dateOnly())
    {
        KDateTime result(*this);
        result.d->setDate(d->date().addDays(secs / 86400));
        return result;
    }
    if (d->spec == stClockTime)
    {
        QDateTime qdt = d->dt();
        qdt.setTimeSpec(Qt::UTC);    // set time as UTC to avoid daylight savings adjustments in addSecs()
        qdt = qdt.addSecs(secs);
        qdt.setTimeSpec(Qt::LocalTime);
        return KDateTime(qdt, ClockTime);
    }
    QDateTime ut = d->toUTC().addSecs(secs);
    switch (d->spec)
    {
        case stUTC:
            return KDateTime(ut, UTC);
        case stOffsetFromUTC:
        {
            ut = ut.addSecs(d->utcOffset);
            ut.setTimeSpec(Qt::LocalTime);
            return KDateTime(ut, OffsetFromUTC, d->utcOffset);
        }
        case stTimeZone:
            return KDateTime(ut, d->tz);
        default:
            return KDateTime();
    }
}

KDateTime KDateTime::addDays(int days) const
{
    KDateTime result(*this);
    result.d->setDate(d->date().addDays(days));
    return result;
}

KDateTime KDateTime::addMonths(int months) const
{
    KDateTime result(*this);
    result.d->setDate(d->date().addMonths(months));
    return result;
}

KDateTime KDateTime::addYears(int years) const
{
    KDateTime result(*this);
    result.d->setDate(d->date().addYears(years));
    return result;
}

int KDateTime::secsTo(const KDateTime &t2) const
{
    if (!isValid() || !t2.isValid())
        return 0;
    if (d->dateOnly())
    {
        QDate dat = t2.d->dateOnly() ? t2.d->date() : t2.toTimeSpec(*this).d->date();
        return d->date().daysTo(dat) * 86400;
    }
    if (t2.d->dateOnly())
        return toTimeSpec(t2).d->date().daysTo(t2.d->date()) * 86400;

    if (d->spec == stClockTime  &&  t2.d->spec == stClockTime)
    {
        // Set both times as UTC to avoid daylight savings adjustments in secsTo()
        QDateTime dt1 = d->dt();
        dt1.setTimeSpec(Qt::UTC);
        QDateTime dt2 = t2.d->dt();
        dt2.setTimeSpec(Qt::UTC);
        return dt1.secsTo(dt2);
    }
    return d->toUTC().secsTo(t2.d->toUTC());
}

int KDateTime::daysTo(const KDateTime &t2) const
{
    if (!isValid() || !t2.isValid())
        return 0;
    if (d->dateOnly())
    {
        QDate dat = t2.d->dateOnly() ? t2.d->date() : t2.toTimeSpec(*this).d->date();
        return d->date().daysTo(dat);
    }
    if (t2.d->dateOnly())
        return toTimeSpec(t2).d->date().daysTo(t2.d->date());

    QDate dat;
    switch (d->spec)
    {
        case stUTC:
            dat = t2.d->toUTC().date();
            break;
        case stOffsetFromUTC:
            dat = t2.d->toUTC().addSecs(d->utcOffset).date();
            break;
        case stTimeZone:
            dat = d->tz->toZoneTime(t2.d->toUTC()).date();
            break;
        case stClockTime:
        {
            const KTimezone *local = KSystemTimezones::local();
            dat = local->toZoneTime(t2.d->toUTC(local)).date();
            break;
        }
        default:    // invalid
            return 0;
    }
    return d->date().daysTo(dat);
}

KDateTime KDateTime::currentDateTime()
{
    return KDateTime(QDateTime::currentDateTime(), KSystemTimezones::local());
}

KDateTime::Comparison KDateTime::compare(const KDateTime &other) const
{
    QDateTime start1, start2;
    bool conv = !compareTimeSpec(other);
    if (conv)
    {
        // Different time specs, so convert to UTC before comparing
        start1 = d->toUTC();
        start2 = other.d->toUTC();
    }
    else
    {
        // Same time specs, so no need to convert to UTC
        start1 = d->dt();
        start2 = other.d->dt();
    }
    if (d->dateOnly() || other.d->dateOnly())
    {
        // At least one of the instances is date-only, so need to compare
        // time periods rather than just times.
        QDateTime end1, end2;
        if (conv)
        {
            if (d->dateOnly())
            {
                KDateTime kdt(*this);
                kdt.setTime(QTime(23,59,59,999));
                end1 = kdt.d->toUTC();
            }
            else
                end1 = d->toUTC();
            if (other.d->dateOnly())
            {
                KDateTime kdt(other);
                kdt.setTime(QTime(23,59,59,999));
                end2 = kdt.d->toUTC();
            }
            else
                end2 = other.d->toUTC();
        }
        else
        {
            if (d->dateOnly())
                end1 = QDateTime(d->date(), QTime(23,59,59,999), Qt::LocalTime);
            else
                end1 = d->dt();
            if (other.d->dateOnly())
                end2 = QDateTime(other.d->date(), QTime(23,59,59,999), Qt::LocalTime);
            else
                end2 = other.d->dt();
        }
        if (start1 == start2)
            return (end1 == end2) ? Equal : (end1 < end2) ? ContainedBy : Contains;
        if (start1 < start2)
            return (end1 < start2) ? Before : (end1 < end2) ? BeforeOverlap : Contains;
        else
            return (start1 > end2) ? After : (end1 > end2) ? AfterOverlap : ContainedBy;
    }
    return (start1 == start2) ? Equal : (start1 < start2) ? Before : After;
}

bool KDateTime::operator==(const KDateTime &other) const
{
    bool dates = (d->dateOnly() || other.d->dateOnly());
    if (dates)
    {
        if (d->dateOnly() && other.d->dateOnly())
            return d->date() == other.d->date();
    }
    if (compareTimeSpec(other))
    {
        // Both instances are in the same time zone, so compare directly
        if (dates)
            return d->date() == other.d->date();
        return d->dt() == other.d->dt();
    }

    if (dates)
    {
        // One instance is date-only, the other is date/time
        const KDateTime *dtime;
        const KDateTime *donly;
        if (d->dateOnly())
        {
            dtime = &other;
            donly = this;
        }
        else
        {
            dtime = this;
            donly = &other;
        }
        QDateTime dt  = dtime->d->toUTC();    // get date/time value, in UTC
        QDateTime day = donly->d->toUTC();    // get start of date-only day, in UTC
        return (dt >= day  &&  dt < day.addSecs(86400));
    }

    return d->toUTC() == other.d->toUTC();
}

bool KDateTime::operator<(const KDateTime &other) const
{
    bool dates = (d->dateOnly() || other.d->dateOnly());
    if (dates)
    {
        if (d->dateOnly() && other.d->dateOnly())
            return d->date() < other.d->date();
    }
    if (compareTimeSpec(other))
    {
        // Both instances are in the same time zone, so compare directly
        if (dates)
            return d->date() < other.d->date();
        return d->dt() < other.d->dt();
    }

    QDateTime thisDt = d->toUTC();
    if (d->dateOnly())
        thisDt = thisDt.addSecs(86400 - 1);    // this instance is date-only - get end of its day
    return thisDt < other.d->toUTC();
}

QString KDateTime::toString(const QString &format) const
{
    if (!isValid())
        return QString();
    enum { TZNone, UTCOffsetShort, UTCOffset, UTCOffsetColon, TZAbbrev, TZName };
    KLocale *locale = KGlobal::locale();
    KCalendarSystem* calendar = new KCalendarSystemGregorian(locale);
    QString result;
    QString s;
    int num, numLength, zone;
    bool escape = false;
    ushort flag = 0;
    for (int i = 0, end = format.length();  i < end;  ++i)
    {
        zone = TZNone;
        num = -1;
        numLength = 0;    // no leading zeroes
        ushort ch = format[i].unicode();
        if (!escape)
        {
            if (ch == '%')
                escape = true;
            else
                result += format[i];
            continue;
        }
        if (!flag)
        {
            switch (ch)
            {
                case '%':
                    result += QLatin1Char('%');
                    break;
                case ':':
                    flag = ch;
                    break;
                case 'Y':     // year
                    num = d->date().year();
                    break;
                case 'y':     // year, 2 digits
                    num = d->date().year() % 100;
                    numLength = 2;
                    break;
                case 'm':     // month, 01 - 12
                    numLength = 2;
                    num = d->date().month();
                    break;
                case 'B':     // month name, translated
                    result += calendar->monthName(d->date().month(), 2000, false);
                    break;
                case 'b':     // month name, translated, short
                    result += calendar->monthName(d->date().month(), 2000, true);
                    break;
                case 'd':     // day of month, 01 - 31
                    numLength = 2;
                    // fall through to 'e'
                case 'e':     // day of month, 1 - 31
                    num = d->date().day();
                    break;
                case 'A':     // week day name, translated
                    result += calendar->weekDayName(d->date().dayOfWeek(), false);
                    break;
                case 'a':     // week day name, translated, short
                    result += calendar->weekDayName(d->date().dayOfWeek(), true);
                    break;
                case 'H':     // hour, 00 - 23
                    numLength = 2;
                    // fall through to 'k'
                case 'k':     // hour, 0 - 23
                    num = d->dt().time().hour();
                    break;
                case 'I':     // hour, 01 - 12
                    numLength = 2;
                    // fall through to 'l'
                case 'l':     // hour, 1 - 12
                    num = (d->dt().time().hour() + 11) % 12 + 1;
                    break;
                case 'M':     // minutes, 00 - 59
                    num = d->dt().time().minute();
                    numLength = 2;
                    break;
                case 'S':     // seconds, 00 - 59
                    num = d->dt().time().second();
                    numLength = 2;
                    break;
                case 'P':     // am/pm
                {
                    bool am = (d->dt().time().hour() < 12);
                    QString ap = locale->translate(am ? "am" : "pm");
                    if (ap.isEmpty())
                        result += am ? QLatin1String("am") : QLatin1String("pm");
                    else
                        result += ap;
                    break;
                }
                case 'p':     // AM/PM
                {
                    bool am = (d->dt().time().hour() < 12);
                    QString ap = locale->translate(am ? "am" : "pm").toUpper();
                    if (ap.isEmpty())
                        result += am ? QLatin1String("AM") : QLatin1String("PM");
                    else
                        result += ap;
                    break;
                }
                case 'z':     // UTC offset in hours and minutes
                    zone = UTCOffset;
                    break;
                case 'Z':     // time zone abbreviation
                    zone = TZAbbrev;
                    break;
                default:
                    result += QLatin1Char('%');
                    result += format[i];
                    break;
            }
        }
        else if (flag == ':')
        {
            // It's a "%:" sequence
            switch (ch)
            {
                case 'A':     // week day name in English
                    result += longDay[d->date().dayOfWeek() - 1];
                    break;
                case 'a':     // week day name in English, short
                    result += shortDay[d->date().dayOfWeek() - 1];
                    break;
                case 'B':     // month name in English
                    result += longMonth[d->date().month() - 1];
                    break;
                case 'b':     // month name in English, short
                    result += shortMonth[d->date().month() - 1];
                    break;
                case 'm':     // month, 1 - 12
                    num = d->date().month();
                    break;
                case 'P':     // am/pm
                    result += (d->dt().time().hour() < 12) ? QLatin1String("am") : QLatin1String("pm");
                    break;
                case 'p':     // AM/PM
                    result += (d->dt().time().hour() < 12) ? QLatin1String("AM") : QLatin1String("PM");
                    break;
                case 'S':     // seconds with ':' prefix, only if non-zero
                {
                    int sec = d->dt().time().second();
                    if (sec || d->dt().time().msec())
                    {
                        result += QLatin1Char(':');
                        num = sec;
                        numLength = 2;
                    }
                    break;
                }
                case 's':     // milliseconds
                    result += s.sprintf("%03d", d->dt().time().msec());
                    break;
                case 'u':     // UTC offset in hours
                    zone = UTCOffsetShort;
                    break;
                case 'z':     // UTC offset in hours and minutes, with colon
                    zone = UTCOffsetColon;
                    break;
                case 'Z':     // time zone name
                    zone = TZName;
                    break;
                default:
                    result += QLatin1String("%:");
                    result += format[i];
                    break;
            }
            flag = 0;
        }
        if (!flag)
            escape = false;

        // Append any required number or time zone information
        if (num >= 0)
        {
            if (!numLength)
                result += QString::number(num);
            else if (numLength == 2)
                result += s.sprintf("%02d", num);
        }
        else if (zone != TZNone)
        {
            const KTimezone *tz = 0;
            int offset;
            switch (d->spec)
            {
                case stUTC:
                case stTimeZone:
                    tz = (d->spec == stTimeZone) ? d->tz : KTimezones::utc();
                    // fall through to OffsetFromUTC
                case stOffsetFromUTC:
                    offset = (d->spec == stTimeZone) ? d->tz->offsetAtZoneTime(d->dt())
                           : (d->spec == stOffsetFromUTC) ? d->utcOffset : 0;
                    offset /= 60;
                    switch (zone)
                    {
                        case UTCOffsetShort:  // UTC offset in hours
                        case UTCOffset:       // UTC offset in hours and minutes
                        case UTCOffsetColon:  // UTC offset in hours and minutes, with colon
                        {
                            if (offset >= 0)
                                result += QLatin1Char('+');
                            else
                            {
                                result += QLatin1Char('-');
                                offset = -offset;
                            }
                            QString s;
                            result += s.sprintf(((zone == UTCOffsetColon) ? "%02d:" : "%02d"), offset/60);
                            if (ch != 'u'  ||  offset % 60)
                                result += s.sprintf("%02d", offset % 60);
                            break;
                        }
                        case TZAbbrev:     // time zone abbreviation
                            if (d->spec != stOffsetFromUTC)
                                result += tz->abbreviation(d->toUTC());
                            break;
                        case TZName:       // time zone name
                            if (d->spec != stOffsetFromUTC)
                                result += tz->name();
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
    }
    return result;
}

QString KDateTime::toString(TimeFormat format) const
{
    QString result;
    if (!isValid())
        return result;

    QString s;
    char tzsign = '+';
    int offset = 0;
    const char *tzcolon = "";
    const KTimezone *tz = (d->spec == stTimeZone) ? d->tz : 0;
    switch (format)
    {
        case RFCDateDay:
            result += shortDay[d->date().dayOfWeek() - 1];
            result += QLatin1String(", ");
            // fall through to RFCDate
        case RFCDate:
        {
            char seconds[8] = { 0 };
            if (d->dt().time().second())
                sprintf(seconds, ":%02d", d->dt().time().second());
            result += s.sprintf("%02d %s %04d %02d:%02d%s ",
                                d->date().day(), shortMonth[d->date().month() - 1].latin1(),
                                d->date().year(), d->dt().time().hour(), d->dt().time().minute(), seconds);
            if (d->spec == stClockTime)
                tz = KSystemTimezones::local();
            break;
        }
        case ISODate:
        {
            // QDateTime::toString(Qt::ISODate) doesn't output fractions of a second
            QString s;
            result += s.sprintf("%04d-%02d-%02d",
                                d->date().year(), d->date().month(), d->date().day());
            if (!d->dateOnly()  ||  d->spec != stClockTime)
            {
                result += s.sprintf("T%02d:%02d:%02d",
                                    d->dt().time().hour(), d->dt().time().minute(), d->dt().time().second());
                if (d->dt().time().msec())
                {
                    // Comma is preferred by ISO8601 as the decimal point symbol,
                    // so use it unless '.' is the symbol used in this locale.
                    result += (KGlobal::locale()->decimalSymbol() == QLatin1String(".")) ? QLatin1Char('.') : QLatin1Char(',');
                    result += s.sprintf("%03d", d->dt().time().msec());
                }
            }
            if (d->spec == stUTC)
                return result + QLatin1Char('Z');
            if (d->spec == stClockTime)
                return result;
            tzcolon = ":";
            break;
        }
            // fall through to QtTextDate
        case QtTextDate:
        case LocalDate:
        {
            Qt::DateFormat qtfmt = (format == QtTextDate) ? Qt::TextDate : Qt::LocalDate;
            if (d->dateOnly())
                result = d->date().toString(qtfmt);
            else
                result = d->dt().toString(qtfmt);
            if (result.isEmpty()  ||  d->spec == stClockTime)
                return result;
            result += QLatin1Char(' ');
            break;
        }
        default:
            return result;
    }

    // Return the string with UTC offset ±hhmm appended
    if (d->spec == stOffsetFromUTC  ||  tz)
    {
        offset = tz ? tz->offsetAtZoneTime(d->dt()) : d->utcOffset;
        if (offset < 0)
        {
            offset = -offset;
            tzsign = '-';
        }
    }
    offset /= 60;
    return result + s.sprintf("%c%02d%s%02d", tzsign, offset/60, tzcolon, offset%60);
}

KDateTime KDateTime::fromString(const QString &string, TimeFormat format, bool *negZero)
{
    if (negZero)
        *negZero = false;
    QString str = string.trimmed();
    if (str.isEmpty())
        return KDateTime();

    switch (format)
    {
        case RFCDateDay: // format is Wdy, DD Mon YYYY hh:mm:ss ±hhmm
        case RFCDate:    // format is [Wdy,] DD Mon YYYY hh:mm[:ss] ±hhmm
        {
            int nyear  = 6;   // indexes within string to values
            int nmonth = 4;
            int nday   = 2;
            int nwday  = 1;
            int nhour  = 7;
            int nmin   = 8;
            int nsec   = 9;
            // Also accept obsolete form "Weekday, DD-Mon-YY HH:MM:SS ±hhmm"
            QRegExp rx("^(?:([A-Z][a-z]+),\\s*)?(\\d{1,2})(\\s+|-)([^-\\s]+)(\\s+|-)(\\d{2,4})\\s+(\\d\\d):(\\d\\d)(?::(\\d\\d))?\\s+(\\S+)$");
            QStringList parts;
            if (!str.indexOf(rx))
            {
                // Check that if date has '-' separators, both separators are '-'.
                parts = rx.capturedTexts();
                bool h1 = (parts[3] == QLatin1String("-"));
                bool h2 = (parts[5] == QLatin1String("-"));
                if (h1 != h2)
                    break;
            }
            else
            {
                // Check for the obsolete form "Wdy Mon DD HH:MM:SS YYYY"
                rx = QRegExp("^([A-Z][a-z]+)\\s+(\\S+)\\s+(\\d\\d)\\s+(\\d\\d):(\\d\\d):(\\d\\d)\\s+(\\d\\d\\d\\d)$");
                if (str.indexOf(rx))
                    break;
                nyear  = 7;
                nmonth = 2;
                nday   = 3;
                nwday  = 1;
                nhour  = 4;
                nmin   = 5;
                nsec   = 6;
                parts = rx.capturedTexts();
            }
            bool ok[4];
            int day    = parts[nday].toInt(&ok[0]);
            int year   = parts[nyear].toInt(&ok[1]);
            int hour   = parts[nhour].toInt(&ok[2]);
            int minute = parts[nmin].toInt(&ok[3]);
            if (!ok[0] || !ok[1] || !ok[2] || !ok[3])
                break;
            int second = 0;
            if (!parts[nsec].isEmpty())
            {
                second = parts[nsec].toInt(&ok[0]);
                if (!ok[0])
                    break;
            }
            bool leapSecond = (second == 60);
            if (leapSecond)
                second = 59;   // apparently a leap second - validate below, once time zone is known
            int month = 0;
            for ( ;  month < 12  &&  parts[nmonth] != shortMonth[month];  ++month) ;
            int dayOfWeek = -1;
            if (!parts[nwday].isEmpty())
            {
                // Look up the weekday name
                while (++dayOfWeek < 7  &&  shortDay[dayOfWeek] != parts[nwday]) ;
                if (dayOfWeek >= 7)
                    for (dayOfWeek = 0;  dayOfWeek < 7  &&  longDay[dayOfWeek] != parts[nwday];  ++dayOfWeek) ;
            }
            if (month >= 12 || dayOfWeek >= 7
            ||  (dayOfWeek < 0  &&  format == RFCDateDay))
                break;
            int i = parts[nyear].size();
            if (i < 4)
            {
                // It's an obsolete year specification with less than 4 digits
                year += (i == 2  &&  year < 50) ? 2000 : 1900;
            }

            // Parse the UTC offset part
            int offset = 0;           // set default to '-0000'
            bool negOffset = false;
            if (parts.count() > 10)
            {
                rx = QRegExp("^([+-])(\\d\\d)(\\d\\d)$");
                if (!parts[10].indexOf(rx))
                {
                    // It's a UTC offset ±hhmm
                    parts = rx.capturedTexts();
                    offset = parts[2].toInt(&ok[0]) * 3600;
                    int offsetMin = parts[3].toInt(&ok[1]);
                    if (!ok[0] || !ok[1] || offsetMin > 59)
                        break;
                    offset += offsetMin * 60;
                    negOffset = (parts[1] == QLatin1String("-"));
                    if (negOffset)
                        offset = -offset;
                }
                else
                {
                    // Check for an obsolete time zone name
                    QByteArray zone = parts[10].toLatin1();
                    if (zone.length() == 1  &&  isalpha(zone[0])  &&  toupper(zone[0]) != 'J')
                        negOffset = true;    // military zone: RFC 2822 treats as '-0000'
                    else if (zone != "UT" && zone != "GMT")    // treated as '+0000'
                    {
                        offset = (zone == "EDT")                  ? -4*3600
                               : (zone == "EST" || zone == "CDT") ? -5*3600
                               : (zone == "CST" || zone == "MDT") ? -6*3600
                               : (zone == "MST" || zone == "PDT") ? -7*3600
                               : (zone == "PST")                  ? -8*3600
                               : 0;
                        if (!offset)
                        {
                            // Check for any other alphabetic time zone
                            bool nonalpha = false;
                            for (int i = 0, end = zone.size();  i < end && !nonalpha;  ++i)
                                nonalpha = !isalpha(zone[i]);
                            if (nonalpha)
                                break;
                            // TODO: Attempt to recognise the time zone abbreviation?
                            negOffset = true;    // unknown time zone: RFC 2822 treats as '-0000'
                        }
                    }
                }
            }
            Status invalid = static_cast<Status>(0);
            QDate qdate = checkDate(year, month+1, day, invalid);   // convert date, and check for out-of-range
            if (!qdate.isValid())
                break;
            KDateTime result(qdate, QTime(hour, minute, second), OffsetFromUTC, offset);
            if (!result.isValid()
            ||  dayOfWeek >= 0  &&  result.date().dayOfWeek() != dayOfWeek+1)
                break;    // invalid date/time, or weekday doesn't correspond with date
            if (!offset)
            {
                if (negOffset && negZero)
                    *negZero = true;   // UTC offset given as "-0000"
                result.setTimeSpec(UTC);
            }
            if (leapSecond)
            {
                // Validate a leap second time. Leap seconds are inserted after 23:59:59 UTC.
                // Convert the time to UTC and check that it is 00:00:00.
                if ((hour*3600 + minute*60 + 60 - offset + 86400*5) % 86400)   // (max abs(offset) is 100 hours)
                    break;    // the time isn't the last second of the day
            }
            if (invalid)
            {
                KDateTime dt;            // date out of range - return invalid KDateTime ...
                dt.d->spec = invalid;    // ... with reason for error
                return dt;
            }
            return result;
        }
        case ISODate:
        {
            /*
             * Extended format: YYYY-MM-DD[Thh[:mm[:ss.s]][TZ]]
             * Basic format:    YYYYMMDD[Thh[mm[ss.s]][TZ]]
             * In either format, the month and day can be replaced by a 3-digit day of the year.
             *
             * QDateTime::fromString(Qt::ISODate) is a rather limited implementation
             * of parsing ISO 8601 format date/time strings, so it isn't used here.
             * This implementation isn't complete either, but it's better.
             *
             * ISO 8601 allows truncation, but for a combined date & time, the date part cannot
             * be truncated from the right, and the time part cannot be truncated from the left.
             * In other words, only the outer parts of the string can be omitted.
             * The standard does not actually define how to interpret omitted parts - it is up
             * to those interchanging the data to agree on a scheme.
             */
            bool dateOnly = false;
            // Check first for the extended format of ISO 8601
            QRegExp rx("^(\\d\\d\\d\\d)-(\\d\\d\\d|\\d\\d-\\d\\d)[T ](\\d\\d)(?::(\\d\\d)(?::(\\d\\d)(?:(?:\\.|,)(\\d+))?)?)?(Z|([+-])(\\d\\d)(?::(\\d\\d))?)?$");
            if (str.indexOf(rx))
            {
                // It's not the extended format - check for the basic format
                rx = QRegExp("^(\\d\\d\\d\\d)(\\d{3,4})[T ](\\d\\d)(?:(\\d\\d)(?:(\\d\\d)(?:(?:\\.|,)(\\d+))?)?)?(Z|([+-])(\\d\\d)(\\d\\d)?)?$");
                if (str.indexOf(rx))
                {
                    // Check for date-only formats
                    dateOnly = true;
                    rx = QRegExp("^(\\d\\d\\d\\d)-(\\d\\d\\d|\\d\\d-\\d\\d)$");
                    if (str.indexOf(rx))
                    {
                        // It's not the extended format - check for the basic format
                        rx = QRegExp("^(\\d\\d\\d\\d)(\\d{3,4})$");
                        if (str.indexOf(rx))
                            break;
                    }
                }
            }
            QStringList parts = rx.capturedTexts();
            bool ok, ok1;
            QDate d;
            int hour   = 0;
            int minute = 0;
            int second = 0;
            int msecs  = 0;
            bool leapSecond = false;
            int year = parts[1].toInt(&ok);
            if (!ok)
                break;
            if (!dateOnly)
            {
                hour = parts[3].toInt(&ok);
                if (!ok)
                    break;
                if (!parts[4].isEmpty())
                {
                    minute = parts[4].toInt(&ok);
                    if (!ok)
                        break;
                }
                if (!parts[5].isEmpty())
                {
                    second = parts[5].toInt(&ok);
                    if (!ok)
                        break;
                }
                leapSecond = (second == 60);
                if (leapSecond)
                    second = 59;   // apparently a leap second - validate below, once time zone is known
                if (!parts[6].isEmpty())
                {
                    QString ms = parts[6] + QString::fromLatin1("00");
                    ms.truncate(3);
                    msecs = ms.toInt(&ok);
                    if (!ok)
                        break;
                }
            }
            int month, day;
            Status invalid = static_cast<Status>(0);
            if (parts[2].length() == 3)
            {
                // A day of the year is specified
                day = parts[2].toInt(&ok);
                if (!ok || day < 1 || day > 366)
                    break;
                d = checkDate(year, 1, 1, invalid).addDays(day - 1);   // convert date, and check for out-of-range
                if (!d.isValid()  ||  (!invalid && d.year() != year))
                    break;
                day   = d.day();
                month = d.month();
            }
            else
            {
                // A month and day are specified
                month = parts[2].left(2).toInt(&ok);
                day   = parts[2].right(2).toInt(&ok1);
                if (!ok || !ok1)
                    break;
                d = checkDate(year, month, day, invalid);   // convert date, and check for out-of-range
                if (!d.isValid())
                    break;
            }
            if (dateOnly)
            {
                if (invalid)
                {
                    KDateTime dt;            // date out of range - return invalid KDateTime ...
                    dt.d->spec = invalid;    // ... with reason for error
                    return dt;
                }
                return KDateTime(d, ClockTime);
            }
            if (hour == 24  && !minute && !second && !msecs)
            {
                // A time of 24:00:00 is allowed by ISO 8601, and means midnight at the end of the day
                d = d.addDays(1);
                hour = 0;
            }

            QTime t(hour, minute, second, msecs);
            if (!t.isValid())
                break;
            if (parts[7].isEmpty())
            {
                // No UTC offset is specified. Don't try to validate leap seconds.
                if (invalid)
                {
                    KDateTime dt;            // date out of range - return invalid KDateTime ...
                    dt.d->spec = invalid;    // ... with reason for error
                    return dt;
                }
                KDateTime result(d, t, ClockTime);
                result.setTimeSpec(KDateTimePrivate::fromStringDefault);
                return result;
            }
            int offset = 0;
            TimeSpec spec = (parts[7] == QLatin1String("Z")) ? UTC : OffsetFromUTC;
            if (spec == OffsetFromUTC)
            {
                offset = parts[9].toInt(&ok) * 3600;
                if (!ok)
                    break;
                if (!parts[10].isEmpty())
                {
                    offset += parts[10].toInt(&ok) * 60;
                    if (!ok)
                        break;
                }
                if (parts[8] == QLatin1String("-"))
                {
                    offset = -offset;
                    if (!offset && negZero)
                        *negZero = true;
                }
            }
            if (leapSecond)
            {
                // Validate a leap second time. Leap seconds are inserted after 23:59:59 UTC.
                // Convert the time to UTC and check that it is 00:00:00.
                if ((hour*3600 + minute*60 + 60 - offset + 86400*5) % 86400)   // (max abs(offset) is 100 hours)
                    break;    // the time isn't the last second of the day
            }
            if (invalid)
            {
                KDateTime dt;            // date out of range - return invalid KDateTime ...
                dt.d->spec = invalid;    // ... with reason for error
                return dt;
            }
            return KDateTime(d, t, spec, offset);
        }
        case QtTextDate:    // format is Wdy Mth DD [hh:mm:ss] YYYY [±hhmm]
        {
            int offset = 0;
            QRegExp rx("^(\\S+\\s+\\S+\\s+\\d\\d\\s+(\\d\\d:\\d\\d:\\d\\d\\s+)?\\d\\d\\d\\d)\\s*(.*)$");
            if (str.indexOf(rx) < 0)
                break;
            QStringList parts = rx.capturedTexts();
            QDate     qd;
            QDateTime qdt;
            bool dateOnly = parts[2].isEmpty();
            if (dateOnly)
            {
                qd = QDate::fromString(parts[1], Qt::TextDate);
                if (!qd.isValid())
                    break;
            }
            else
            {
                qdt = QDateTime::fromString(parts[1], Qt::TextDate);
                if (!qdt.isValid())
                    break;
            }
            if (parts[3].isEmpty())
            {
                // No time zone offset specified, so return a local clock time
                if (dateOnly)
                {
                    KDateTime result(qd, ClockTime);
                    result.setTimeSpec(KDateTimePrivate::fromStringDefault);
                    return result;
                }
                else
                {
                    qdt.setTimeSpec(Qt::LocalTime);
                    KDateTime result(qdt, ClockTime);
                    result.setTimeSpec(KDateTimePrivate::fromStringDefault);
                    return result;
                }
            }
            rx = QRegExp("([+-])([\\d][\\d])(?::?([\\d][\\d]))?$");
            if (parts[3].indexOf(rx) < 0)
                break;

            // Extract the UTC offset at the end of the string
            bool ok;
            parts = rx.capturedTexts();
            offset = parts[2].toInt(&ok) * 3600;
            if (!ok)
                break;
            if (parts.count() > 3)
            {
                offset += parts[3].toInt(&ok) * 60;
                if (!ok)
                    break;
            }
            if (parts[1] == QLatin1String("-"))
            {
                offset = -offset;
                if (!offset && negZero)
                    *negZero = true;
            }
            if (dateOnly)
                return KDateTime(qd, (offset ? OffsetFromUTC : UTC), offset);
            qdt.setTimeSpec(offset ? Qt::LocalTime : Qt::UTC);
            return KDateTime(qdt, (offset ? OffsetFromUTC : UTC), offset);
        }
        case LocalDate:
        default:
            break;
    }
    return KDateTime();
}

KDateTime KDateTime::fromString(const QString &string, const QString &format,
                                const KTimezones *zones, bool offsetIfAmbiguous)
{
    int     utcOffset;    // UTC offset in seconds
    bool    dateOnly = false;
    Status invalid = static_cast<Status>(0);
    QString zoneName;
    QByteArray zoneAbbrev;
    QDateTime qdt = fromStr(string, format, utcOffset, zoneName, zoneAbbrev, dateOnly, invalid);
    if (!qdt.isValid())
        return KDateTime();
    if (zones)
    {
        // Try to find a time zone match
        bool zname = false;
        const KTimezone *zone = 0;
        if (!zoneName.isEmpty())
        {
            // A time zone name has been found.
            // Use the time zone with that name.
            zone = zones->zone(zoneName);
            zname = true;
        }
        else if (!invalid)
        {
            if (!zoneAbbrev.isEmpty())
            {
                // A time zone abbreviation has been found.
                // Use the time zone which contains it, if any, provided that the
                // abbreviation applies at the specified date/time.
                bool useUtcOffset = false;
                const KTimezones::ZoneMap z = zones->zones();
                for (KTimezones::ZoneMap::ConstIterator it = z.begin();  it != z.end();  ++it)
                {
                    if (it.data()->abbreviations().indexOf(zoneAbbrev) >= 0)
                    {
                        int offset2;
                        int offset = it.data()->offsetAtZoneTime(qdt, &offset2);
                        QDateTime ut = qdt;
                        ut.setTimeSpec(Qt::UTC);
                        ut.addSecs(-offset);
                        if (it.data()->abbreviation(ut) != zoneAbbrev)
                        {
                            if (offset == offset2)
                                continue;     // abbreviation doesn't apply at specified time
                            ut.addSecs(offset - offset2);
                            if (it.data()->abbreviation(ut) != zoneAbbrev)
                                continue;     // abbreviation doesn't apply at specified time
                            offset = offset2;
                        }
                        // Found a time zone which uses this abbreviation at the specified date/time
                        if (zone)
                        {
                            // Abbreviation is used by more than one time zone
                            if (!offsetIfAmbiguous  ||  offset != utcOffset)
                                return KDateTime();
                            useUtcOffset = true;
                        }
                        else
                        {
                            zone = it.data();
                            utcOffset = offset;
                        }
                    }
                }
                if (useUtcOffset)
                {
                    zone = 0;
                    if (!utcOffset)
                        qdt.setTimeSpec(Qt::UTC);
                }
                else
                    zname = true;
            }
            else if (utcOffset  ||  qdt.timeSpec() == Qt::UTC)
            {
                // A UTC offset has been found.
                // Use the time zone which contains it, if any.
                // For a date-only value, use the start of the day.
                QDateTime dtUTC = qdt;
                dtUTC.setTimeSpec(Qt::UTC);
                dtUTC.addSecs(-utcOffset);
                const KTimezones::ZoneMap z = zones->zones();
                for (KTimezones::ZoneMap::ConstIterator it = z.begin();  it != z.end();  ++it)
                {
                    QList<int> offsets = it.data()->UTCOffsets();
                    if ((offsets.isEmpty() || offsets.indexOf(utcOffset) >= 0)
                    &&  it.data()->offsetAtUTC(dtUTC) == utcOffset)
                    {
                        // Found a time zone which uses this offset at the specified time
                        if (zone  ||  !utcOffset)
                        {
                            // UTC offset is used by more than one time zone
                            if (!offsetIfAmbiguous)
                                return KDateTime();
                            if (invalid)
                            {
                                KDateTime dt;            // date out of range - return invalid KDateTime ...
                                dt.d->spec = invalid;    // ... with reason for error
                                return dt;
                            }
                            if (dateOnly)
                                return KDateTime(qdt.date(), OffsetFromUTC, utcOffset);
                            qdt.setTimeSpec(Qt::LocalTime);
                            return KDateTime(qdt, OffsetFromUTC, utcOffset);
                        }
                        zone = it.data();
                    }
                }
            }
        }
        if (!zone && zname)
            return KDateTime();    // an unknown zone name or abbreviation was found
        if (zone && !invalid)
        {
            if (dateOnly)
                return KDateTime(qdt.date(), zone);
            return KDateTime(qdt, zone);
        }
    }

    // No time zone match was found
    if (invalid)
    {
        KDateTime dt;            // date out of range - return invalid KDateTime ...
        dt.d->spec = invalid;    // ... with reason for error
        return dt;
    }
    KDateTime result;
    if (utcOffset)
    {
        qdt.setTimeSpec(Qt::LocalTime);
        result = KDateTime(qdt, OffsetFromUTC, utcOffset);
    }
    else if (qdt.timeSpec() == Qt::UTC)
        result = KDateTime(qdt, UTC);
    else
    {
        result = KDateTime(qdt, ClockTime);
        result.setTimeSpec(KDateTimePrivate::fromStringDefault);
    }
    if (dateOnly)
        result.setDateOnly(true);
    return result;
}

void KDateTime::setFromStringDefault(TimeSpec spec, int utcOffset)
{
    KDateTimePrivate::fromStringDefault.setTimeSpec(spec, utcOffset);
}

void KDateTime::setFromStringDefault(const KTimezone *tz)
{
    KDateTimePrivate::fromStringDefault.setTimeSpec(tz);
}

QDataStream & operator<<(QDataStream &s, const KDateTime &dt)
{
    quint8 flags = dt.timeSpec();
    if (dt.isDateOnly())
        flags |= 0x80;
    s << dt.dateTime() << flags;
    switch (dt.timeSpec())
    {
        case KDateTime::OffsetFromUTC:
            s << dt.UTCOffset();
            break;
        case KDateTime::TimeZone:
#ifdef __GNUC__
#warning TODO: output time zone data
#endif
//            s << *dt.timeZone();
            break;
        default:
            break;
    }
    return s;
}

QDataStream & operator>>(QDataStream &s, KDateTime &kdt)
{
    quint8 flags;
    QDateTime dt;
    int utcOffset = 0;
    s >> dt >> flags;
    KDateTime::TimeSpec spec = (KDateTime::TimeSpec)(flags & 0x7f);
    switch (spec)
    {
        case KDateTime::TimeZone:
        {
            KTimezone *tz = new KTimezone;
#ifdef __GNUC__
#warning TODO: read time zone data
#endif
//            s >> *tz;
            kdt = KDateTime(dt, tz);
            break;
        }
        case KDateTime::OffsetFromUTC:
            s >> utcOffset;
            // fall through to UTC
        case KDateTime::UTC:
        case KDateTime::ClockTime:
            dt.setTimeSpec(spec == KDateTime::UTC ? Qt::UTC : Qt::LocalTime);
            kdt = KDateTime(dt, spec, utcOffset);
            break;
        default:     // invalid time spec
            kdt = KDateTime();
            return s;
    }
    if (flags & 0x80)
        kdt.setDateOnly(true);
    return s;
}


/*
 * Extracts a QDateTime from a string, given a format string.
 * The date/time is set to Qt::UTC if a zero UTC offset is found,
 * otherwise it is Qt::LocalTime. If Qt::LocalTime is returned and
 * utcOffset == 0, that indicates that no UTC offset was found.
 */
QDateTime fromStr(const QString& string, const QString& format, int& utcOffset,
                  QString& zoneName, QByteArray& zoneAbbrev, bool& dateOnly, Status &status)
{
    QString str = string.simplified();
    int year      = -1;
    int month     = -1;
    int day       = -1;
    int dayOfWeek = -1;
    int hour      = -1;
    int minute    = -1;
    int second    = -1;
    int millisec  = -1;
    int ampm      = -1;
    int tzoffset  = -99999;
    zoneName = QString();
    zoneAbbrev = QByteArray();

    enum { TZNone, UTCOffset, UTCOffsetColon, TZAbbrev, TZName };
    KLocale *locale = KGlobal::locale();
    KCalendarSystem* calendar = new KCalendarSystemGregorian(locale);
    int zone;
    int s = 0;
    int send = str.length();
    bool escape = false;
    ushort flag = 0;
    for (int f = 0, fend = format.length();  f < fend && s < send;  ++f)
    {
        zone = TZNone;
        ushort ch = format[f].unicode();
        if (!escape)
        {
            if (ch == '%')
                escape = true;
            else if (format[f].isSpace())
            {
                if (str[s].isSpace())
                    ++s;
            }
            else if (format[f] == str[s])
                ++s;
            else
                return QDateTime();
            continue;
        }
        if (!flag)
        {
            switch (ch)
            {
                case '%':
                    if (str[s++] != QLatin1Char('%'))
                        return QDateTime();
                    break;
                case ':':
                    flag = ch;
                    break;
                case 'Y':     // full year
                    if (!getNumber(str, s, 4, 4, 0, -1, year))
                        return QDateTime();
                    break;
                case 'y':     // year, 2 digits
                    if (!getNumber(str, s, 2, 2, 0, 99, year))
                        return QDateTime();
                    year += (year <= 50) ? 2000 : 1999;
                    break;
                case 'm':     // month, 1 - 12
                    if (!getNumber(str, s, 1, 2, 1, 12, month))
                        return QDateTime();
                    break;
                case 'B':
                case 'b':     // month name, translated or English
                {
                    int m = matchMonth(str, s, calendar);
                    if (m <= 0  ||  month != -1 && month != m)
                        return QDateTime();
                    month = m;
                    break;
                }
                case 'd':
                case 'e':     // day of month, 1 - 31
                    if (!getNumber(str, s, 1, 2, 1, 31, day))
                        return QDateTime();
                    break;
                case 'A':
                case 'a':     // week day name, translated or English
                {
                    int dow = matchDay(str, s, calendar);
                    if (dow <= 0  ||  dayOfWeek != -1 && dayOfWeek != dow)
                        return QDateTime();
                    dayOfWeek = dow;
                    break;
                }
                case 'H':
                case 'k':     // hour, 0 - 23
                    if (!getNumber(str, s, 1, 2, 0, 23, hour))
                        return QDateTime();
                    break;
                case 'I':
                case 'l':     // hour, 1 - 12
                    if (!getNumber(str, s, 1, 2, 1, 12, hour))
                        return QDateTime();
                    break;
                case 'M':     // minutes, 0 - 59
                    if (!getNumber(str, s, 1, 2, 0, 59, minute))
                        return QDateTime();
                    break;
                case 'S':     // seconds, 0 - 59
                    if (!getNumber(str, s, 1, 2, 0, 59, second))
                        return QDateTime();
                    break;
                case 'P':
                case 'p':     // am/pm
                {
                    int ap = getAmPm(str, s, locale);
                    if (!ap  ||  ampm != -1 && ampm != ap)
                        return QDateTime();
                    ampm = ap;
                    break;
                }
                case 'z':     // UTC offset in hours and optionally minutes
                    zone = UTCOffset;
                    break;
                case 'Z':     // time zone abbreviation
                    zone = TZAbbrev;
                    break;
                case 't':     // whitespace
                    if (str[s++] != QLatin1Char(' '))
                        return QDateTime();
                    break;
                default:
                    if (s + 2 > send
                    ||  str[s++] != QLatin1Char('%')
                    ||  str[s++] != format[f])
                        return QDateTime();
                    break;
            }
        }
        else if (flag == ':')
        {
            // It's a "%:" sequence
            switch (ch)
            {
                case 'A':
                case 'a':     // week day name in English
                {
                    int dow = matchDay(str, s, 0);
                    if (dow <= 0  ||  dayOfWeek != -1 && dayOfWeek != dow)
                        return QDateTime();
                    dayOfWeek = dow;
                    break;
                }
                case 'B':
                case 'b':     // month name in English
                {
                    int m = matchMonth(str, s, 0);
                    if (month != -1 && month != m)
                        return QDateTime();
                    month = m;
                    break;
                }
                case 'm':     // month, 1 - 12
                    if (!getNumber(str, s, 1, 2, 1, 12, month))
                        return QDateTime();
                    break;
                case 'P':
                case 'p':     // am/pm in English
                {
                    int ap = getAmPm(str, s, 0);
                    if (!ap  ||  ampm != -1 && ampm != ap)
                        return QDateTime();
                    ampm = ap;
                    break;
                }
                case 'S':     // seconds with ':' prefix, defaults to zero
                    if (str[s] != QLatin1Char(':'))
                    {
                        second = 0;
                        break;
                    }
                    ++s;
                    if (!getNumber(str, s, 1, 2, 0, 59, second))
                        return QDateTime();
                    break;
                case 's':     // milliseconds, with decimal point prefix
                {
                    if (str[s] != QLatin1Char('.'))
                    {
                        QString dpt = locale->decimalSymbol();
                        if (!str.mid(s).startsWith(dpt))
                            return QDateTime();
                        s += dpt.length() - 1;
                    }
                    ++s;
                    if (s >= send)
                        return QDateTime();
                    QString val = str.mid(s);
                    int i = 0;
                    for (int end = val.length();  i < end && val[i].isDigit();  ++i) ;
                    if (!i)
                        return QDateTime();
                    val.truncate(i);
                    val += QLatin1String("00");
                    val.truncate(3);
                    int ms = val.toInt();
                    if (millisec != -1 && millisec != ms)
                        return QDateTime();
                    millisec = ms;
                    s += i;
                    break;
                }
                case 'u':     // UTC offset in hours and optionally minutes
                    zone = UTCOffset;
                    break;
                case 'z':     // UTC offset in hours and minutes, with colon
                    zone = UTCOffsetColon;
                    break;
                case 'Z':     // time zone name
                    zone = TZName;
                    break;
                default:
                    if (s + 3 > send
                    ||  str[s++] != QLatin1Char('%')
                    ||  str[s++] != QLatin1Char(':')
                    ||  str[s++] != format[f])
                        return QDateTime();
                    break;
            }
            flag = 0;
        }
        if (!flag)
            escape = false;

        if (zone != TZNone)
        {
            // Read time zone or UTC offset
            switch (zone)
            {
                case UTCOffset:
                case UTCOffsetColon:
                    if (!zoneAbbrev.isEmpty() || !zoneName.isEmpty())
                        return QDateTime();
                    if (!getUTCOffset(str, s, (zone == UTCOffsetColon), tzoffset))
                        return QDateTime();
                    break;
                case TZAbbrev:     // time zone abbreviation
                {
                    if (tzoffset != -99999 || !zoneName.isEmpty())
                        return QDateTime();
                    int start = s;
                    while (s < send && str[s].isLetterOrNumber())
                        ++s;
                    if (s == start)
                        return QDateTime();
                    QString z = str.mid(start, s - start);
                    if (!zoneAbbrev.isEmpty()  &&  z != zoneAbbrev)
                        return QDateTime();
                    zoneAbbrev = z.toLatin1();
                    break;
                }
                case TZName:       // time zone name
                {
                    if (tzoffset != -99999 || !zoneAbbrev.isEmpty())
                        return QDateTime();
                    QString z;
                    if (f + 1 >= fend)
                    {
                        z = str.mid(s);
                        s = send;
                    }
                    else
                    {
                        // Get the terminating character for the zone name
                        QChar endchar = format[f + 1];
                        if (endchar == QLatin1Char('%')  &&  f + 2 < fend)
                        {
                            QChar endchar2 = format[f + 2];
                            if (endchar2 == QLatin1Char('n') || endchar2 == QLatin1Char('t'))
                                endchar = QLatin1Char(' ');
                        }
                        // Extract from the input string up to the terminating character
                        int start = s;
                        for ( ;  s < send && str[s] != endchar;  ++s) ;
                        if (s == start)
                            return QDateTime();
                        z = str.mid(start, s - start);
                    }
                    if (!zoneName.isEmpty()  &&  z != zoneName)
                        return QDateTime();
                    zoneName = z;
                    break;
                }
                default:
                    break;
            }
        }
    }

    if (year == -1)
        year = QDate::currentDate().year();
    if (month == -1)
        month = 1;
    QDate d = checkDate(year, month, (day > 0 ? day : 1), status);   // convert date, and check for out-of-range
    if (!d.isValid())
        return QDateTime();
    if (dayOfWeek != -1  &&  !status)
    {
        if (day == -1)
        {
            day = 1 + dayOfWeek - QDate(year, month, 1).dayOfWeek();
            if (day <= 0)
                day += 7;
        }
        else
        {
            if (QDate(year, month, day).dayOfWeek() != dayOfWeek)
                return QDateTime();
        }
    }
    if (day == -1)
        day = 1;
    dateOnly = (hour == -1 && minute == -1 && second == -1 && millisec == -1);
    if (hour == -1)
        hour = 0;
    if (minute == -1)
        minute = 0;
    if (second == -1)
        second = 0;
    if (millisec == -1)
        millisec = 0;
    if (ampm != -1)
    {
        if (!hour || hour > 12)
            return QDateTime();
        if (ampm == 1  &&  hour == 12)
            hour = 0;
        else if (ampm == 2  &&  hour < 12)
            hour += 12;
    }

    QDateTime dt(d, QTime(hour, minute, second, millisec), (tzoffset == 0 ? Qt::UTC : Qt::LocalTime));

    utcOffset = (tzoffset == -99999) ? 0 : tzoffset*60;

    return dt;
}


/*
 * Find which day name matches the specified part of a string.
 * 'offset' is incremented by the length of the match.
 * Reply = day number (1 - 7), or <= 0 if no match.
 */
int matchDay(const QString &string, int &offset, KCalendarSystem *calendar)
{
    int dayOfWeek;
    QString part = string.mid(offset);
    if (part.isEmpty())
        return -1;
    if (calendar)
    {
        // Check for localised day name first
        for (dayOfWeek = 1;  dayOfWeek <= 7;  ++dayOfWeek)
        {
            QString name = calendar->weekDayName(dayOfWeek, false);
            if (part.startsWith(name, Qt::CaseInsensitive))
            {
                offset += name.length();
                return dayOfWeek;
            }
        }
        for (dayOfWeek = 1;  dayOfWeek <= 7;  ++dayOfWeek)
        {
            QString name = calendar->weekDayName(dayOfWeek, true);
            if (part.startsWith(name, Qt::CaseInsensitive))
            {
                offset += name.length();
                return dayOfWeek;
            }
        }
    }

    // Check for English day name
    dayOfWeek = findString(part, longDay, 7, offset);
    if (dayOfWeek < 0)
        dayOfWeek = findString(part, shortDay, 7, offset);
    return dayOfWeek + 1;
}

/*
 * Find which month name matches the specified part of a string.
 * 'offset' is incremented by the length of the match.
 * Reply = month number (1 - 12), or <= 0 if no match.
 */
int matchMonth(const QString &string, int &offset, KCalendarSystem *calendar)
{
    int month;
    QString part = string.mid(offset);
    if (part.isEmpty())
        return -1;
    if (calendar)
    {
        // Check for localised month name first
        for (month = 1;  month <= 12;  ++month)
        {
            QString name = calendar->monthName(month, 2000, false);
            if (part.startsWith(name, Qt::CaseInsensitive))
            {
                offset += name.length();
                return month;
            }
        }
        for (month = 1;  month <= 12;  ++month)
        {
            QString name = calendar->monthName(month, 2000, true);
            if (part.startsWith(name, Qt::CaseInsensitive))
            {
                offset += name.length();
                return month;
            }
        }
    }
    // Check for English month name
    month = findString(part, longMonth, 12, offset);
    if (month < 0)
        month = findString(part, shortMonth, 12, offset);
    return month + 1;
}

/*
 * Read a UTC offset from the input string.
 */
bool getUTCOffset(const QString &string, int &offset, bool colon, int &result)
{
    int sign;
    int len = string.length();
    if (offset >= len)
        return false;
    switch (string[offset++].unicode())
    {
        case '+':
            sign = 1;
            break;
        case '-':
            sign = -1;
            break;
        default:
            return false;
    }
    int tzhour = -1;
    int tzmin  = -1;
    if (!getNumber(string, offset, 2, 2, 0, 99, tzhour))
        return false;
    if (colon)
    {
        if (offset >= len  ||  string[offset++] != QLatin1Char(':'))
            return false;
    }
    if (offset >= len  ||  !string[offset].isDigit())
        tzmin = 0;
    else
    {
        if (!getNumber(string, offset, 2, 2, 0, 59, tzmin))
            return false;
    }
    tzmin += tzhour * 60;
    if (result != -99999  &&  result != tzmin)
        return false;
    result = tzmin;
    return true;
}

/*
 * Read an am/pm indicator from the input string.
 * 'offset' is incremented by the length of the match.
 * Reply = 1 (am), 2 (pm), or 0 if no match.
 */
int getAmPm(const QString &string, int &offset, KLocale *locale)
{
    QString part = string.mid(offset);
    int ap = 0;
    int n = 2;
    if (locale)
    {
        // Check localised form first
        QString aps = locale->translate("am");
        if (part.startsWith(aps, Qt::CaseInsensitive))
        {
            ap = 1;
            n = aps.length();
        }
        else
        {
            aps = locale->translate("pm");
            if (part.startsWith(aps, Qt::CaseInsensitive))
            {
                ap = 2;
                n = aps.length();
            }
        }
    }
    if (!ap)
    {
        if (part.startsWith(QLatin1String("am"), Qt::CaseInsensitive))
            ap = 1;
        else if (part.startsWith(QLatin1String("pm"), Qt::CaseInsensitive))
            ap = 2;
    }
    if (ap)
        offset += n;
    return ap;
}

/* Convert part of 'string' to a number.
 * If converted number differs from any current value in 'result', the function fails.
 * Reply = true if successful.
 */
bool getNumber(const QString& string, int& offset, int mindigits, int maxdigits, int minval, int maxval, int& result)
{
    int end = string.size();
    if (offset + maxdigits > end)
        maxdigits = end - offset;
    int ndigits;
    for (ndigits = 0;  ndigits < maxdigits && string[offset + ndigits].isDigit();  ++ndigits) ;
    if (ndigits < mindigits)
        return false;
    bool ok;
    int n = string.mid(offset, ndigits).toInt(&ok);
    if (!ok  ||  result != -1 && n != result  ||  n < minval  ||  (n > maxval && maxval >= 0))
        return false;
    result = n;
    offset += ndigits;
    return true;
}

int findString(const QString &string, const QLatin1String *array, int count, int &offset)
{
    for (int i = 0;  i < count;  ++i)
    {
        if (string.startsWith(array[i], Qt::CaseInsensitive))
        {
            offset += QString::fromLatin1(array[i].latin1()).length();
            return i;
        }
    }
    return -1;
}

/*
 * Return the QDate for a given year, month and day.
 * If in error, check whether the reason is that the year is out of range.
 * If so, return a valid (but wrong) date but with 'status' set to the
 * appropriate error code. If no error, 'status' is set to zero.
 */
QDate checkDate(int year, int month, int day, Status &status)
{
    QDate qdate(year, month, day);
    // QDate documentation says that year must be < 8000, although QDate constructor accepts >= 8000
    if (qdate.isValid()  &&  year < 8000)
    {
        status = static_cast<Status>(0);
        return qdate;
    }

    // Invalid date - check whether it's simply out of range
    status = stInvalid;
    if (year < 1753  ||  year >= 8000)
    {
        bool leap = (year % 4 == 0) && (year % 100 || year % 400 == 0);
        qdate.setYMD((leap ? 2000 : 2001), month, day);
        if (qdate.isValid())
            status = (year < 1753) ? stTooEarly : stTooLate;
    }
    return qdate;
}

