/*
    Copyright 2009, 2010 John Layt <john@layt.net>

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

#include "kcalendarsystemethiopian_p.h"
#include "kcalendarsystemcopticprivate_p.h"

#include "kdebug.h"
#include "klocale.h"

#include <QtCore/QDate>
#include <QtCore/QCharRef>

//Reuse the Coptic private implementation
class KCalendarSystemEthiopianPrivate : public KCalendarSystemCopticPrivate
{
public:
    explicit KCalendarSystemEthiopianPrivate(KCalendarSystemEthiopian *q) : KCalendarSystemCopticPrivate(q) {
    }

    virtual ~KCalendarSystemEthiopianPrivate() {
    }

    virtual KLocale::CalendarSystem calendarSystem() const;
    virtual void loadDefaultEraList();
    virtual QString monthName(int month, int year, KLocale::DateTimeComponentFormat format, bool possessive) const;
    virtual QString weekDayName(int weekDay, KLocale::DateTimeComponentFormat format) const;
};

KLocale::CalendarSystem KCalendarSystemEthiopianPrivate::calendarSystem() const
{
    return KLocale::EthiopianCalendar;
}

void KCalendarSystemEthiopianPrivate::loadDefaultEraList()
{
    QString name, shortName, format;
    // Incarnation Era, Amätä Mehrät, "Year of Mercy".
    name = i18nc("Calendar Era: Ethiopian Incarnation Era, years > 0, LongFormat", "Amata Mehrat");
    shortName = i18nc("Calendar Era: Ethiopian Incarnation Era, years > 0, ShortFormat", "AM");
    format = i18nc("(kdedt-format) Ethiopian, AM, full era year format used for %EY, e.g. 2000 AM", "%Ey %EC");
    addEra('+', 1, q->epoch(), 1, q->latestValidDate(), name, shortName, format);
}

// Names taken from http://www.ethiopianembassy.at/dates_cycles.htm, alternative transliterations exist
QString KCalendarSystemEthiopianPrivate::monthName(int month, int year, KLocale::DateTimeComponentFormat format, bool possessive) const
{
    Q_UNUSED(year);

    if (format == KLocale::NarrowName) {
        switch (month) {
        case 1:
            return ki18nc("Ethiopian month 1 - KLocale::NarrowName",  "M").toString(locale());
        case 2:
            return ki18nc("Ethiopian month 2 - KLocale::NarrowName",  "T").toString(locale());
        case 3:
            return ki18nc("Ethiopian month 3 - KLocale::NarrowName",  "H").toString(locale());
        case 4:
            return ki18nc("Ethiopian month 4 - KLocale::NarrowName",  "T").toString(locale());
        case 5:
            return ki18nc("Ethiopian month 5 - KLocale::NarrowName",  "T").toString(locale());
        case 6:
            return ki18nc("Ethiopian month 6 - KLocale::NarrowName",  "Y").toString(locale());
        case 7:
            return ki18nc("Ethiopian month 7 - KLocale::NarrowName",  "M").toString(locale());
        case 8:
            return ki18nc("Ethiopian month 8 - KLocale::NarrowName",  "M").toString(locale());
        case 9:
            return ki18nc("Ethiopian month 9 - KLocale::NarrowName",  "G").toString(locale());
        case 10:
            return ki18nc("Ethiopian month 10 - KLocale::NarrowName", "S").toString(locale());
        case 11:
            return ki18nc("Ethiopian month 11 - KLocale::NarrowName", "H").toString(locale());
        case 12:
            return ki18nc("Ethiopian month 12 - KLocale::NarrowName", "N").toString(locale());
        case 13:
            return ki18nc("Ethiopian month 13 - KLocale::NarrowName", "P").toString(locale());
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName && possessive) {
        switch (month) {
        case 1:
            return ki18nc("Ethiopian month 1 - KLocale::ShortName Possessive",  "of Mes").toString(locale());
        case 2:
            return ki18nc("Ethiopian month 2 - KLocale::ShortName Possessive",  "of Teq").toString(locale());
        case 3:
            return ki18nc("Ethiopian month 3 - KLocale::ShortName Possessive",  "of Hed").toString(locale());
        case 4:
            return ki18nc("Ethiopian month 4 - KLocale::ShortName Possessive",  "of Tah").toString(locale());
        case 5:
            return ki18nc("Ethiopian month 5 - KLocale::ShortName Possessive",  "of Ter").toString(locale());
        case 6:
            return ki18nc("Ethiopian month 6 - KLocale::ShortName Possessive",  "of Yak").toString(locale());
        case 7:
            return ki18nc("Ethiopian month 7 - KLocale::ShortName Possessive",  "of Mag").toString(locale());
        case 8:
            return ki18nc("Ethiopian month 8 - KLocale::ShortName Possessive",  "of Miy").toString(locale());
        case 9:
            return ki18nc("Ethiopian month 9 - KLocale::ShortName Possessive",  "of Gen").toString(locale());
        case 10:
            return ki18nc("Ethiopian month 10 - KLocale::ShortName Possessive", "of Sen").toString(locale());
        case 11:
            return ki18nc("Ethiopian month 11 - KLocale::ShortName Possessive", "of Ham").toString(locale());
        case 12:
            return ki18nc("Ethiopian month 12 - KLocale::ShortName Possessive", "of Neh").toString(locale());
        case 13:
            return ki18nc("Ethiopian month 13 - KLocale::ShortName Possessive", "of Pag").toString(locale());
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName && !possessive) {
        switch (month) {
        case 1:
            return ki18nc("Ethiopian month 1 - KLocale::ShortName",  "Mes").toString(locale());
        case 2:
            return ki18nc("Ethiopian month 2 - KLocale::ShortName",  "Teq").toString(locale());
        case 3:
            return ki18nc("Ethiopian month 3 - KLocale::ShortName",  "Hed").toString(locale());
        case 4:
            return ki18nc("Ethiopian month 4 - KLocale::ShortName",  "Tah").toString(locale());
        case 5:
            return ki18nc("Ethiopian month 5 - KLocale::ShortName",  "Ter").toString(locale());
        case 6:
            return ki18nc("Ethiopian month 6 - KLocale::ShortName",  "Yak").toString(locale());
        case 7:
            return ki18nc("Ethiopian month 7 - KLocale::ShortName",  "Mag").toString(locale());
        case 8:
            return ki18nc("Ethiopian month 8 - KLocale::ShortName",  "Miy").toString(locale());
        case 9:
            return ki18nc("Ethiopian month 9 - KLocale::ShortName",  "Gen").toString(locale());
        case 10:
            return ki18nc("Ethiopian month 10 - KLocale::ShortName", "Sen").toString(locale());
        case 11:
            return ki18nc("Ethiopian month 11 - KLocale::ShortName", "Ham").toString(locale());
        case 12:
            return ki18nc("Ethiopian month 12 - KLocale::ShortName", "Neh").toString(locale());
        case 13:
            return ki18nc("Ethiopian month 13 - KLocale::ShortName", "Pag").toString(locale());
        default:
            return QString();
        }
    }

    if (format == KLocale::LongName && possessive) {
        switch (month) {
        case 1:
            return ki18nc("Ethiopian month 1 - KLocale::LongName Possessive",  "of Meskerem").toString(locale());
        case 2:
            return ki18nc("Ethiopian month 2 - KLocale::LongName Possessive",  "of Tequemt").toString(locale());
        case 3:
            return ki18nc("Ethiopian month 3 - KLocale::LongName Possessive",  "of Hedar").toString(locale());
        case 4:
            return ki18nc("Ethiopian month 4 - KLocale::LongName Possessive",  "of Tahsas").toString(locale());
        case 5:
            return ki18nc("Ethiopian month 5 - KLocale::LongName Possessive",  "of Ter").toString(locale());
        case 6:
            return ki18nc("Ethiopian month 6 - KLocale::LongName Possessive",  "of Yakatit").toString(locale());
        case 7:
            return ki18nc("Ethiopian month 7 - KLocale::LongName Possessive",  "of Magabit").toString(locale());
        case 8:
            return ki18nc("Ethiopian month 8 - KLocale::LongName Possessive",  "of Miyazya").toString(locale());
        case 9:
            return ki18nc("Ethiopian month 9 - KLocale::LongName Possessive",  "of Genbot").toString(locale());
        case 10:
            return ki18nc("Ethiopian month 10 - KLocale::LongName Possessive", "of Sene").toString(locale());
        case 11:
            return ki18nc("Ethiopian month 11 - KLocale::LongName Possessive", "of Hamle").toString(locale());
        case 12:
            return ki18nc("Ethiopian month 12 - KLocale::LongName Possessive", "of Nehase").toString(locale());
        case 13:
            return ki18nc("Ethiopian month 13 - KLocale::LongName Possessive", "of Pagumen").toString(locale());
        default:
            return QString();
        }
    }

    // Default to LongName
    switch (month) {
    case 1:
        return ki18nc("Ethiopian month 1 - KLocale::LongName",  "Meskerem").toString(locale());
    case 2:
        return ki18nc("Ethiopian month 2 - KLocale::LongName",  "Tequemt").toString(locale());
    case 3:
        return ki18nc("Ethiopian month 3 - KLocale::LongName",  "Hedar").toString(locale());
    case 4:
        return ki18nc("Ethiopian month 4 - KLocale::LongName",  "Tahsas").toString(locale());
    case 5:
        return ki18nc("Ethiopian month 5 - KLocale::LongName",  "Ter").toString(locale());
    case 6:
        return ki18nc("Ethiopian month 6 - KLocale::LongName",  "Yakatit").toString(locale());
    case 7:
        return ki18nc("Ethiopian month 7 - KLocale::LongName",  "Magabit").toString(locale());
    case 8:
        return ki18nc("Ethiopian month 8 - KLocale::LongName",  "Miyazya").toString(locale());
    case 9:
        return ki18nc("Ethiopian month 9 - KLocale::LongName",  "Genbot").toString(locale());
    case 10:
        return ki18nc("Ethiopian month 10 - KLocale::LongName", "Sene").toString(locale());
    case 11:
        return ki18nc("Ethiopian month 11 - KLocale::LongName", "Hamle").toString(locale());
    case 12:
        return ki18nc("Ethiopian month 12 - KLocale::LongName", "Nehase").toString(locale());
    case 13:
        return ki18nc("Ethiopian month 13 - KLocale::LongName", "Pagumen").toString(locale());
    default:
        return QString();
    }
}

// Names taken from http://www.ethiopianembassy.at/dates_cycles.htm, alternative transliterations exist
QString KCalendarSystemEthiopianPrivate::weekDayName(int weekDay, KLocale::DateTimeComponentFormat format) const
{
    if (format == KLocale::NarrowName) {
        switch (weekDay) {
        case 1:
            return ki18nc("Ethiopian weekday 1 - KLocale::NarrowName ", "S").toString(locale());
        case 2:
            return ki18nc("Ethiopian weekday 2 - KLocale::NarrowName ", "M").toString(locale());
        case 3:
            return ki18nc("Ethiopian weekday 3 - KLocale::NarrowName ", "R").toString(locale());
        case 4:
            return ki18nc("Ethiopian weekday 4 - KLocale::NarrowName ", "H").toString(locale());
        case 5:
            return ki18nc("Ethiopian weekday 5 - KLocale::NarrowName ", "A").toString(locale());
        case 6:
            return ki18nc("Ethiopian weekday 6 - KLocale::NarrowName ", "Q").toString(locale());
        case 7:
            return ki18nc("Ethiopian weekday 7 - KLocale::NarrowName ", "E").toString(locale());
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName  || format == KLocale:: ShortNumber) {
        switch (weekDay) {
        case 1:
            return ki18nc("Ethiopian weekday 1 - KLocale::ShortName", "Seg").toString(locale());
        case 2:
            return ki18nc("Ethiopian weekday 2 - KLocale::ShortName", "Mak").toString(locale());
        case 3:
            return ki18nc("Ethiopian weekday 3 - KLocale::ShortName", "Rob").toString(locale());
        case 4:
            return ki18nc("Ethiopian weekday 4 - KLocale::ShortName", "Ham").toString(locale());
        case 5:
            return ki18nc("Ethiopian weekday 5 - KLocale::ShortName", "Arb").toString(locale());
        case 6:
            return ki18nc("Ethiopian weekday 6 - KLocale::ShortName", "Qed").toString(locale());
        case 7:
            return ki18nc("Ethiopian weekday 7 - KLocale::ShortName", "Ehu").toString(locale());
        default: return QString();
        }
    }

    switch (weekDay) {
    case 1:
        return ki18nc("Ethiopian weekday 1 - KLocale::LongName", "Segno").toString(locale());
    case 2:
        return ki18nc("Ethiopian weekday 2 - KLocale::LongName", "Maksegno").toString(locale());
    case 3:
        return ki18nc("Ethiopian weekday 3 - KLocale::LongName", "Rob").toString(locale());
    case 4:
        return ki18nc("Ethiopian weekday 4 - KLocale::LongName", "Hamus").toString(locale());
    case 5:
        return ki18nc("Ethiopian weekday 5 - KLocale::LongName", "Arb").toString(locale());
    case 6:
        return ki18nc("Ethiopian weekday 6 - KLocale::LongName", "Qedame").toString(locale());
    case 7:
        return ki18nc("Ethiopian weekday 7 - KLocale::LongName", "Ehud").toString(locale());
    default:
        return QString();
    }
}


KCalendarSystemEthiopian::KCalendarSystemEthiopian(const KLocale *locale)
                        : KCalendarSystemCoptic(*new KCalendarSystemEthiopianPrivate(this), KSharedConfig::Ptr(), locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemEthiopian::KCalendarSystemEthiopian(const KSharedConfig::Ptr config, const KLocale *locale)
                        : KCalendarSystemCoptic(*new KCalendarSystemEthiopianPrivate(this), config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemEthiopian::KCalendarSystemEthiopian(KCalendarSystemEthiopianPrivate &dd,
                                                   const KSharedConfig::Ptr config, const KLocale *locale)
                        : KCalendarSystemCoptic(dd, config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemEthiopian::~KCalendarSystemEthiopian()
{
}

QString KCalendarSystemEthiopian::calendarType() const
{
    return QLatin1String("ethiopian");
}

QDate KCalendarSystemEthiopian::epoch() const
{
    //0001-01-01, no Year 0.
    //0008-08-29 AD Julian
    return QDate::fromJulianDay(1724221);
}

QDate KCalendarSystemEthiopian::earliestValidDate() const
{
    //0001-01-01, no Year 0.
    //0008-08-29 AD Julian
    return QDate::fromJulianDay(1724221);
}

QDate KCalendarSystemEthiopian::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widgets support > 9999
    //9999-12-30
    //100008-08-29 AD Julian
    return QDate::fromJulianDay(5376721);
}

bool KCalendarSystemEthiopian::isValid(int year, int month, int day) const
{
    return KCalendarSystemCoptic::isValid(year, month, day);
}

bool KCalendarSystemEthiopian::isValid(const QDate &date) const
{
    return KCalendarSystemCoptic::isValid(date);
}

bool KCalendarSystemEthiopian::isLeapYear(int year) const
{
    return KCalendarSystemCoptic::isLeapYear(year);
}

bool KCalendarSystemEthiopian::isLeapYear(const QDate &date) const
{
    return KCalendarSystemCoptic::isLeapYear(date);
}

QString KCalendarSystemEthiopian::monthName(int month, int year, MonthNameFormat format) const
{
    return KCalendarSystemCoptic::monthName(month, year, format);
}

QString KCalendarSystemEthiopian::monthName(const QDate &date, MonthNameFormat format) const
{
    return KCalendarSystemCoptic::monthName(date, format);
}

QString KCalendarSystemEthiopian::weekDayName(int weekDay, WeekDayNameFormat format) const
{
    return KCalendarSystemCoptic::weekDayName(weekDay, format);
}

QString KCalendarSystemEthiopian::weekDayName(const QDate &date, WeekDayNameFormat format) const
{
    return KCalendarSystemCoptic::weekDayName(date, format);
}

int KCalendarSystemEthiopian::weekDayOfPray() const
{
    return 7;
}

bool KCalendarSystemEthiopian::isLunar() const
{
    return KCalendarSystemCoptic::isLunar();
}

bool KCalendarSystemEthiopian::isLunisolar() const
{
    return KCalendarSystemCoptic::isLunisolar();
}

bool KCalendarSystemEthiopian::isSolar() const
{
    return KCalendarSystemCoptic::isSolar();
}

bool KCalendarSystemEthiopian::isProleptic() const
{
    return false;
}

bool KCalendarSystemEthiopian::julianDayToDate(int jd, int &year, int &month, int &day) const
{
    return KCalendarSystemCoptic::julianDayToDate(jd, year, month, day);
}

bool KCalendarSystemEthiopian::dateToJulianDay(int year, int month, int day, int &jd) const
{
    return KCalendarSystemCoptic::dateToJulianDay(year, month, day, jd);
}
