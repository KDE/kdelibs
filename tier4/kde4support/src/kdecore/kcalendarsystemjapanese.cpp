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

#include "kcalendarsystemjapanese_p.h"
#include "kcalendarsystemgregorianprivate_p.h"

#include "klocale.h"
#include "klocalizedstring.h"

#include <QtCore/QDate>

//Reuse the Gregorian private implementation
class KCalendarSystemJapanesePrivate : public KCalendarSystemGregorianPrivate
{
public:
    explicit KCalendarSystemJapanesePrivate(KCalendarSystemJapanese *q);
    virtual ~KCalendarSystemJapanesePrivate();

    virtual void loadDefaultEraList();
    virtual int earliestValidYear() const;
};

//Override only a few of the Gregorian private methods

KCalendarSystemJapanesePrivate::KCalendarSystemJapanesePrivate(KCalendarSystemJapanese *q)
                              : KCalendarSystemGregorianPrivate(q)
{
}

KCalendarSystemJapanesePrivate::~KCalendarSystemJapanesePrivate()
{
}

void KCalendarSystemJapanesePrivate::loadDefaultEraList()
{
    QString name, shortName, format;

    // Nengō, Only do most recent for now, use AD for the rest.
    // Feel free to add more, but have mercy on the translators :-)

    name = i18nc("Calendar Era: Gregorian Christian Era, years > 0, LongFormat", "Anno Domini");
    shortName = i18nc("Calendar Era: Gregorian Christian Era, years > 0, ShortFormat", "AD");
    format = i18nc("(kdedt-format) Gregorian, AD, full era year format used for %EY, e.g. 2000 AD", "%Ey %EC");
    addEra('+', 1, q->epoch(), 1, QDate(1868, 9, 7), name, shortName, format);

    name = i18nc("Calendar Era: Japanese Nengō, Meiji Era, LongFormat", "Meiji");
    shortName = name;
    format = i18nc("(kdedt-format) Japanese, Meiji, full era year format used for %EY, year = 1, e.g. Meiji 1", "%EC Gannen");
    addEra('+', 1, QDate(1868, 9, 8), 1868, QDate(1868, 12, 31), name, shortName, format);
    format = i18nc("(kdedt-format) Japanese, Meiji, full era year format used for %EY, year > 1, e.g. Meiji 22", "%EC %Ey");
    addEra('+', 2, QDate(1869, 1, 1), 1869, QDate(1912, 7, 29), name, shortName, format);

    name = i18nc("Calendar Era: Japanese Nengō, Taishō Era, LongFormat", "Taishō");
    shortName = name;
    format = i18nc("(kdedt-format) Japanese, Taishō, full era year format used for %EY, year = 1, e.g. Taishō 1", "%EC Gannen");
    addEra('+', 1, QDate(1912, 7, 30), 1912, QDate(1912, 12, 31), name, shortName, format);
    format = i18nc("(kdedt-format) Japanese, Taishō, full era year format used for %EY, year > 1, e.g. Taishō 22", "%EC %Ey");
    addEra('+', 2, QDate(1913, 1, 1), 1913, QDate(1926, 12, 24), name, shortName, format);

    name = i18nc("Calendar Era: Japanese Nengō, Shōwa Era, LongFormat", "Shōwa");
    shortName = name;
    format = i18nc("(kdedt-format) Japanese, Shōwa, full era year format used for %EY, year = 1, e.g. Shōwa 1", "%EC Gannen");
    addEra('+', 1, QDate(1926, 12, 25), 1926, QDate(1926, 12, 31), name, shortName, format);
    format = i18nc("(kdedt-format) Japanese, Shōwa, full era year format used for %EY, year > 1, e.g. Shōwa 22", "%EC %Ey");
    addEra('+', 2, QDate(1927, 1, 1), 1927, QDate(1989, 1, 7), name, shortName, format);

    name = i18nc("Calendar Era: Japanese Nengō, Heisei Era, LongFormat", "Heisei");
    shortName = name;
    format = i18nc("(kdedt-format) Japanese, Heisei, full era year format used for %EY, year = 1, e.g. Heisei 1", "%EC Gannen");
    addEra('+', 1, QDate(1989, 1, 8), 1989, QDate(1989, 12, 31), name, shortName, format);
    format = i18nc("(kdedt-format) Japanese, Heisei, full era year format used for %EY, year > 1, e.g. Heisei 22", "%EC %Ey");
    addEra('+', 2, QDate(1990, 1, 1), 1990, q->latestValidDate(), name, shortName, format);
}

int KCalendarSystemJapanesePrivate::earliestValidYear() const
{
    return 1;
}


KCalendarSystemJapanese::KCalendarSystemJapanese(const KSharedConfig::Ptr config, const KLocale *locale)
                       : KCalendarSystemGregorian(*new KCalendarSystemJapanesePrivate(this), config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemJapanese::KCalendarSystemJapanese(KCalendarSystemJapanesePrivate &dd,
                                                 const KSharedConfig::Ptr config, const KLocale *locale)
                       : KCalendarSystemGregorian(dd, config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemJapanese::~KCalendarSystemJapanese()
{
}

QString KCalendarSystemJapanese::calendarType() const
{
    return QLatin1String("japanese");
}

KLocale::CalendarSystem KCalendarSystemJapanese::calendarSystem() const
{
    return KLocale::JapaneseCalendar;
}

QDate KCalendarSystemJapanese::epoch() const
{
    // 0001-01-01 Gregorian for now
    return QDate::fromJulianDay(1721426);
}

QDate KCalendarSystemJapanese::earliestValidDate() const
{
    // 0001-01-01 Gregorian for now
    return QDate::fromJulianDay(1721426);
}

QDate KCalendarSystemJapanese::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widgets support > 9999
    // 9999-12-31 Gregorian
    return QDate::fromJulianDay(5373484);
}

QString KCalendarSystemJapanese::monthName(int month, int year, MonthNameFormat format) const
{
    return KCalendarSystemGregorian::monthName(month, year, format);
}

QString KCalendarSystemJapanese::monthName(const QDate &date, MonthNameFormat format) const
{
    return KCalendarSystemGregorian::monthName(date, format);
}

QString KCalendarSystemJapanese::weekDayName(int weekDay, WeekDayNameFormat format) const
{
    return KCalendarSystemGregorian::weekDayName(weekDay, format);
}

QString KCalendarSystemJapanese::weekDayName(const QDate &date, WeekDayNameFormat format) const
{
    return KCalendarSystemGregorian::weekDayName(date, format);
}

int KCalendarSystemJapanese::yearStringToInteger(const QString &sNum, int &iLength) const
{
    QString gannen = i18nc("Japanese year 1 of era", "Gannen");
    if (sNum.startsWith(gannen, Qt::CaseInsensitive)) {
        iLength = gannen.length();
        return 1;
    } else {
        return KCalendarSystemGregorian::yearStringToInteger(sNum, iLength);
    }
}

bool KCalendarSystemJapanese::isLunar() const
{
    return KCalendarSystemGregorian::isLunar();
}

bool KCalendarSystemJapanese::isLunisolar() const
{
    return KCalendarSystemGregorian::isLunisolar();
}

bool KCalendarSystemJapanese::isSolar() const
{
    return KCalendarSystemGregorian::isSolar();
}

bool KCalendarSystemJapanese::isProleptic() const
{
    return false;
}

bool KCalendarSystemJapanese::julianDayToDate(qint64 jd, int &year, int &month, int &day) const
{
    return KCalendarSystemGregorian::julianDayToDate(jd, year, month, day);
}

bool KCalendarSystemJapanese::dateToJulianDay(int year, int month, int day, qint64 &jd) const
{
    return KCalendarSystemGregorian::dateToJulianDay(year, month, day, jd);
}
