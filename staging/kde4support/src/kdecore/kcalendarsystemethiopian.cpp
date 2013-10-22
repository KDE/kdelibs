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

#include "klocale.h"
#include "klocalizedstring.h"

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

    virtual void loadDefaultEraList();
    virtual QString monthName(int month, int year, KLocale::DateTimeComponentFormat format, bool possessive) const;
    virtual QString weekDayName(int weekDay, KLocale::DateTimeComponentFormat format) const;
};

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

    QStringList languages = locale()->languageList();

    if (format == KLocale::NarrowName) {
        switch (month) {
        case 1:
            return ki18nc("Ethiopian month 1 - KLocale::NarrowName",  "M").toString(languages);
        case 2:
            return ki18nc("Ethiopian month 2 - KLocale::NarrowName",  "T").toString(languages);
        case 3:
            return ki18nc("Ethiopian month 3 - KLocale::NarrowName",  "H").toString(languages);
        case 4:
            return ki18nc("Ethiopian month 4 - KLocale::NarrowName",  "T").toString(languages);
        case 5:
            return ki18nc("Ethiopian month 5 - KLocale::NarrowName",  "T").toString(languages);
        case 6:
            return ki18nc("Ethiopian month 6 - KLocale::NarrowName",  "Y").toString(languages);
        case 7:
            return ki18nc("Ethiopian month 7 - KLocale::NarrowName",  "M").toString(languages);
        case 8:
            return ki18nc("Ethiopian month 8 - KLocale::NarrowName",  "M").toString(languages);
        case 9:
            return ki18nc("Ethiopian month 9 - KLocale::NarrowName",  "G").toString(languages);
        case 10:
            return ki18nc("Ethiopian month 10 - KLocale::NarrowName", "S").toString(languages);
        case 11:
            return ki18nc("Ethiopian month 11 - KLocale::NarrowName", "H").toString(languages);
        case 12:
            return ki18nc("Ethiopian month 12 - KLocale::NarrowName", "N").toString(languages);
        case 13:
            return ki18nc("Ethiopian month 13 - KLocale::NarrowName", "P").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName && possessive) {
        switch (month) {
        case 1:
            return ki18nc("Ethiopian month 1 - KLocale::ShortName Possessive",  "of Mes").toString(languages);
        case 2:
            return ki18nc("Ethiopian month 2 - KLocale::ShortName Possessive",  "of Teq").toString(languages);
        case 3:
            return ki18nc("Ethiopian month 3 - KLocale::ShortName Possessive",  "of Hed").toString(languages);
        case 4:
            return ki18nc("Ethiopian month 4 - KLocale::ShortName Possessive",  "of Tah").toString(languages);
        case 5:
            return ki18nc("Ethiopian month 5 - KLocale::ShortName Possessive",  "of Ter").toString(languages);
        case 6:
            return ki18nc("Ethiopian month 6 - KLocale::ShortName Possessive",  "of Yak").toString(languages);
        case 7:
            return ki18nc("Ethiopian month 7 - KLocale::ShortName Possessive",  "of Mag").toString(languages);
        case 8:
            return ki18nc("Ethiopian month 8 - KLocale::ShortName Possessive",  "of Miy").toString(languages);
        case 9:
            return ki18nc("Ethiopian month 9 - KLocale::ShortName Possessive",  "of Gen").toString(languages);
        case 10:
            return ki18nc("Ethiopian month 10 - KLocale::ShortName Possessive", "of Sen").toString(languages);
        case 11:
            return ki18nc("Ethiopian month 11 - KLocale::ShortName Possessive", "of Ham").toString(languages);
        case 12:
            return ki18nc("Ethiopian month 12 - KLocale::ShortName Possessive", "of Neh").toString(languages);
        case 13:
            return ki18nc("Ethiopian month 13 - KLocale::ShortName Possessive", "of Pag").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName && !possessive) {
        switch (month) {
        case 1:
            return ki18nc("Ethiopian month 1 - KLocale::ShortName",  "Mes").toString(languages);
        case 2:
            return ki18nc("Ethiopian month 2 - KLocale::ShortName",  "Teq").toString(languages);
        case 3:
            return ki18nc("Ethiopian month 3 - KLocale::ShortName",  "Hed").toString(languages);
        case 4:
            return ki18nc("Ethiopian month 4 - KLocale::ShortName",  "Tah").toString(languages);
        case 5:
            return ki18nc("Ethiopian month 5 - KLocale::ShortName",  "Ter").toString(languages);
        case 6:
            return ki18nc("Ethiopian month 6 - KLocale::ShortName",  "Yak").toString(languages);
        case 7:
            return ki18nc("Ethiopian month 7 - KLocale::ShortName",  "Mag").toString(languages);
        case 8:
            return ki18nc("Ethiopian month 8 - KLocale::ShortName",  "Miy").toString(languages);
        case 9:
            return ki18nc("Ethiopian month 9 - KLocale::ShortName",  "Gen").toString(languages);
        case 10:
            return ki18nc("Ethiopian month 10 - KLocale::ShortName", "Sen").toString(languages);
        case 11:
            return ki18nc("Ethiopian month 11 - KLocale::ShortName", "Ham").toString(languages);
        case 12:
            return ki18nc("Ethiopian month 12 - KLocale::ShortName", "Neh").toString(languages);
        case 13:
            return ki18nc("Ethiopian month 13 - KLocale::ShortName", "Pag").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::LongName && possessive) {
        switch (month) {
        case 1:
            return ki18nc("Ethiopian month 1 - KLocale::LongName Possessive",  "of Meskerem").toString(languages);
        case 2:
            return ki18nc("Ethiopian month 2 - KLocale::LongName Possessive",  "of Tequemt").toString(languages);
        case 3:
            return ki18nc("Ethiopian month 3 - KLocale::LongName Possessive",  "of Hedar").toString(languages);
        case 4:
            return ki18nc("Ethiopian month 4 - KLocale::LongName Possessive",  "of Tahsas").toString(languages);
        case 5:
            return ki18nc("Ethiopian month 5 - KLocale::LongName Possessive",  "of Ter").toString(languages);
        case 6:
            return ki18nc("Ethiopian month 6 - KLocale::LongName Possessive",  "of Yakatit").toString(languages);
        case 7:
            return ki18nc("Ethiopian month 7 - KLocale::LongName Possessive",  "of Magabit").toString(languages);
        case 8:
            return ki18nc("Ethiopian month 8 - KLocale::LongName Possessive",  "of Miyazya").toString(languages);
        case 9:
            return ki18nc("Ethiopian month 9 - KLocale::LongName Possessive",  "of Genbot").toString(languages);
        case 10:
            return ki18nc("Ethiopian month 10 - KLocale::LongName Possessive", "of Sene").toString(languages);
        case 11:
            return ki18nc("Ethiopian month 11 - KLocale::LongName Possessive", "of Hamle").toString(languages);
        case 12:
            return ki18nc("Ethiopian month 12 - KLocale::LongName Possessive", "of Nehase").toString(languages);
        case 13:
            return ki18nc("Ethiopian month 13 - KLocale::LongName Possessive", "of Pagumen").toString(languages);
        default:
            return QString();
        }
    }

    // Default to LongName
    switch (month) {
    case 1:
        return ki18nc("Ethiopian month 1 - KLocale::LongName",  "Meskerem").toString(languages);
    case 2:
        return ki18nc("Ethiopian month 2 - KLocale::LongName",  "Tequemt").toString(languages);
    case 3:
        return ki18nc("Ethiopian month 3 - KLocale::LongName",  "Hedar").toString(languages);
    case 4:
        return ki18nc("Ethiopian month 4 - KLocale::LongName",  "Tahsas").toString(languages);
    case 5:
        return ki18nc("Ethiopian month 5 - KLocale::LongName",  "Ter").toString(languages);
    case 6:
        return ki18nc("Ethiopian month 6 - KLocale::LongName",  "Yakatit").toString(languages);
    case 7:
        return ki18nc("Ethiopian month 7 - KLocale::LongName",  "Magabit").toString(languages);
    case 8:
        return ki18nc("Ethiopian month 8 - KLocale::LongName",  "Miyazya").toString(languages);
    case 9:
        return ki18nc("Ethiopian month 9 - KLocale::LongName",  "Genbot").toString(languages);
    case 10:
        return ki18nc("Ethiopian month 10 - KLocale::LongName", "Sene").toString(languages);
    case 11:
        return ki18nc("Ethiopian month 11 - KLocale::LongName", "Hamle").toString(languages);
    case 12:
        return ki18nc("Ethiopian month 12 - KLocale::LongName", "Nehase").toString(languages);
    case 13:
        return ki18nc("Ethiopian month 13 - KLocale::LongName", "Pagumen").toString(languages);
    default:
        return QString();
    }
}

// Names taken from http://www.ethiopianembassy.at/dates_cycles.htm, alternative transliterations exist
QString KCalendarSystemEthiopianPrivate::weekDayName(int weekDay, KLocale::DateTimeComponentFormat format) const
{
    QStringList languages = locale()->languageList();

    if (format == KLocale::NarrowName) {
        switch (weekDay) {
        case 1:
            return ki18nc("Ethiopian weekday 1 - KLocale::NarrowName ", "S").toString(languages);
        case 2:
            return ki18nc("Ethiopian weekday 2 - KLocale::NarrowName ", "M").toString(languages);
        case 3:
            return ki18nc("Ethiopian weekday 3 - KLocale::NarrowName ", "R").toString(languages);
        case 4:
            return ki18nc("Ethiopian weekday 4 - KLocale::NarrowName ", "H").toString(languages);
        case 5:
            return ki18nc("Ethiopian weekday 5 - KLocale::NarrowName ", "A").toString(languages);
        case 6:
            return ki18nc("Ethiopian weekday 6 - KLocale::NarrowName ", "Q").toString(languages);
        case 7:
            return ki18nc("Ethiopian weekday 7 - KLocale::NarrowName ", "E").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName  || format == KLocale:: ShortNumber) {
        switch (weekDay) {
        case 1:
            return ki18nc("Ethiopian weekday 1 - KLocale::ShortName", "Seg").toString(languages);
        case 2:
            return ki18nc("Ethiopian weekday 2 - KLocale::ShortName", "Mak").toString(languages);
        case 3:
            return ki18nc("Ethiopian weekday 3 - KLocale::ShortName", "Rob").toString(languages);
        case 4:
            return ki18nc("Ethiopian weekday 4 - KLocale::ShortName", "Ham").toString(languages);
        case 5:
            return ki18nc("Ethiopian weekday 5 - KLocale::ShortName", "Arb").toString(languages);
        case 6:
            return ki18nc("Ethiopian weekday 6 - KLocale::ShortName", "Qed").toString(languages);
        case 7:
            return ki18nc("Ethiopian weekday 7 - KLocale::ShortName", "Ehu").toString(languages);
        default: return QString();
        }
    }

    switch (weekDay) {
    case 1:
        return ki18nc("Ethiopian weekday 1 - KLocale::LongName", "Segno").toString(languages);
    case 2:
        return ki18nc("Ethiopian weekday 2 - KLocale::LongName", "Maksegno").toString(languages);
    case 3:
        return ki18nc("Ethiopian weekday 3 - KLocale::LongName", "Rob").toString(languages);
    case 4:
        return ki18nc("Ethiopian weekday 4 - KLocale::LongName", "Hamus").toString(languages);
    case 5:
        return ki18nc("Ethiopian weekday 5 - KLocale::LongName", "Arb").toString(languages);
    case 6:
        return ki18nc("Ethiopian weekday 6 - KLocale::LongName", "Qedame").toString(languages);
    case 7:
        return ki18nc("Ethiopian weekday 7 - KLocale::LongName", "Ehud").toString(languages);
    default:
        return QString();
    }
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

KLocale::CalendarSystem KCalendarSystemEthiopian::calendarSystem() const
{
    return KLocale::EthiopianCalendar;
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

bool KCalendarSystemEthiopian::julianDayToDate(qint64 jd, int &year, int &month, int &day) const
{
    return KCalendarSystemCoptic::julianDayToDate(jd, year, month, day);
}

bool KCalendarSystemEthiopian::dateToJulianDay(int year, int month, int day, qint64 &jd) const
{
    return KCalendarSystemCoptic::dateToJulianDay(year, month, day, jd);
}
