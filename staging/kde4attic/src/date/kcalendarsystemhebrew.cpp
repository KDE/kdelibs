/*
    Copyright (c) 2003 Hans Petter Bieker <bieker@kde.org>
    Copyright 2007, 2009, 2010 John Layt <john@layt.net>
        Calendar conversion routines based on Hdate v6, by Amos
        Shapir 1978 (rev. 1985, 1992)

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

// Derived hebrew kde calendar class

#include "kcalendarsystemhebrew_p.h"
#include "kcalendarsystemprivate_p.h"

#include "klocale.h"
#include "klocalizedstring.h"

#include <QtCore/QDate>
#include <QtCore/QCharRef>

static int hebrewDaysElapsed(int y);

class h_date
{
public:
    int hd_day;
    int hd_mon;
    int hd_year;
    int hd_dw;
    int hd_flg;
};

/*
 * compute general date structure from hebrew date
 */
static class h_date * hebrewToGregorian(int y, int m, int d)
{
    static class h_date h;
    int s;

    y -= 3744;
    s = hebrewDaysElapsed(y);
    d += s;
    s = hebrewDaysElapsed(y + 1) - s;      /* length of year */

    if (s > 365 && m > 6) {
        --m;
        d += 30;
    }
    d += (59 * (m - 1) + 1) / 2;      /* regular months */
    /* special cases */
    if (s % 10 > 4 && m > 2) {    /* long Heshvan */
        d++;
    }
    if (s % 10 < 4 && m > 3) {    /* short Kislev */
        d--;
    }
    // ### HPB: Broken in leap years
    //if (s > 365 && m > 6)  /* leap year */
    //  d += 30;
    d -= 6002;

    y = (d + 36525) * 4 / 146097 - 1;
    d -= y / 4 * 146097 + (y % 4) * 36524;
    y *= 100;

    /* compute year */
    s = (d + 366) * 4 / 1461 - 1;
    d -= s / 4 * 1461 + (s % 4) * 365;
    y += s;
    /* compute month */
    m = (d + 245) * 12 / 367 - 7;
    d -= m * 367 / 12 - 30;
    if (++m >= 12) {
        m -= 12;
        y++;
    }
    h.hd_day = d;
    h.hd_mon = m;
    h.hd_year = y;
    return(&h);
}

/*
 * compute date structure from no. of days since 1 Tishrei 3744
 */
static class h_date * gregorianToHebrew(int y, int m, int d)
{
    static class h_date h;
    int s;

    if ((m -= 2) <= 0) {
        m += 12;
        y--;
    }
    /* no. of days, Julian calendar */
    d += 365 * y + y / 4 + 367 * m / 12 + 5968;
    /* Gregorian calendar */
    d -= y / 100 - y / 400 - 2;
    h.hd_dw = (d + 1) % 7;

    /* compute the year */
    y += 16;
    s = hebrewDaysElapsed(y);
    m = hebrewDaysElapsed(y + 1);
    while (d >= m) {   /* computed year was underestimated */
        s = m;
        y++;
        m = hebrewDaysElapsed(y + 1);
    }
    d -= s;
    s = m - s;  /* size of current year */
    y += 3744;

    h.hd_flg = s % 10 - 4;

    /* compute day and month */
    if (d >= s - 236) {    /* last 8 months are regular */
        d -= s - 236;
        m = d * 2 / 59;
        d -= (m * 59 + 1) / 2;
        m += 4;
        if (s > 365 && m <= 5) {    /* Adar of Meuberet */
            m += 8;
        }
    } else {
        /* first 4 months have 117-119 days */
        s = 114 + s % 10;
        m = d * 4 / s;
        d -= (m * s + 3) / 4;
    }

    h.hd_day = d;
    h.hd_mon = m;
    h.hd_year = y;
    return(&h);
}

/* constants, in 1/18th of minute */
static const int HOUR = 1080;
static const int DAY = 24 * HOUR;
static const int WEEK = 7 * DAY;
#define M(h,p) ((h)*HOUR+p)
#define MONTH (DAY+M(12,793))

/**
 * @internal
 * no. of days in y years
 */
static int hebrewDaysElapsed(int y)
{
    int m, nm, dw, s, l;

    l = y * 7 + 1;  // no. of leap months
    m = y * 12 + l / 19;  // total no. of months
    l %= 19;
    nm = m * MONTH + M(1 + 6, 779);   // molad new year 3744 (16BC) + 6 hours
    s = m * 28 + nm / DAY - 2;

    nm %= WEEK;
    dw = nm / DAY;
    nm %= DAY;

    // special cases of Molad Zaken
    if ((l < 12 && dw == 3 && nm >= M(9 + 6, 204)) ||
            (l < 7 && dw == 2 && nm >= M(15 + 6, 589))) {
        s++, dw++;
    }

    /* ADU */
    if (dw == 1 || dw == 4 || dw == 6) {
        s++;
    }
    return s;
}

/**
 * @internal
 * true if long Cheshvan
 */
static int long_cheshvan(int year)
{
    QDate first, last;
    class h_date *gd;

    gd = hebrewToGregorian(year, 1, 1);
    first.setDate(gd->hd_year, gd->hd_mon + 1, gd->hd_day + 1);

    gd = hebrewToGregorian(year + 1, 1, 1);
    last.setDate(gd->hd_year, gd->hd_mon + 1, gd->hd_day + 1);

    return (first.daysTo(last) % 10 == 5);
}

/**
 * @internal
 * true if short Kislev
 */
static int short_kislev(int year)
{
    QDate first, last;
    class h_date * gd;

    gd = hebrewToGregorian(year, 1, 1);
    first.setDate(gd->hd_year, gd->hd_mon + 1, gd->hd_day + 1);

    gd = hebrewToGregorian(year + 1, 1, 1);
    last.setDate(gd->hd_year, gd->hd_mon + 1, gd->hd_day + 1);

    return (first.daysTo(last) % 10 == 3);
}

// Ok
static class h_date *toHebrew(const QDate &date)
{
    class h_date *sd;

    sd = gregorianToHebrew(date.year(), date.month(), date.day());
    ++sd->hd_mon;
    ++sd->hd_day;

    return sd;
}

class KCalendarSystemHebrewPrivate : public KCalendarSystemPrivate
{
public:
    explicit KCalendarSystemHebrewPrivate(KCalendarSystemHebrew *q);

    virtual ~KCalendarSystemHebrewPrivate();

    // Virtual methods each calendar system must re-implement
    virtual void loadDefaultEraList();
    virtual int monthsInYear(int year) const;
    virtual int daysInMonth(int year, int month) const;
    virtual int daysInYear(int year) const;
    virtual bool isLeapYear(int year) const;
    virtual bool hasLeapMonths() const;
    virtual bool hasYearZero() const;
    virtual int maxMonthsInYear() const;
    virtual int earliestValidYear() const;
    virtual int latestValidYear() const;
    virtual QString monthName(int month, int year, KLocale::DateTimeComponentFormat format, bool possessive) const;
    virtual QString weekDayName(int weekDay, KLocale::DateTimeComponentFormat format) const;

    virtual int integerFromString(const QString &string, int maxLength, int &readLength) const;
    virtual QString stringFromInteger(int number, int padWidth = 0, QChar padChar = QLatin1Char('0')) const;
    virtual QString stringFromInteger(int number, int padWidth, QChar padChar, KLocale::DigitSet digitSet) const;

    virtual int monthNumberToMonthIndex(int year, int month) const;
};

// Shared d pointer base class definitions

KCalendarSystemHebrewPrivate::KCalendarSystemHebrewPrivate(KCalendarSystemHebrew *q)
                            : KCalendarSystemPrivate(q)
{
}

KCalendarSystemHebrewPrivate::~KCalendarSystemHebrewPrivate()
{
}

void KCalendarSystemHebrewPrivate::loadDefaultEraList()
{
    QString name, shortName, format;
    // Jewish Era, Anno Mundi, "Year of the World".
    name = i18nc("Calendar Era: Hebrew Era, years > 0, LongFormat", "Anno Mundi");
    shortName = i18nc("Calendar Era: Hebrew Era, years > 0, ShortFormat", "AM");
    format = i18nc("(kdedt-format) Hebrew, AM, full era year format used for %EY, e.g. 2000 AM", "%Ey %EC");
    addEra('+', 1, q->epoch(), 1, q->latestValidDate(), name, shortName, format);
}

int KCalendarSystemHebrewPrivate::monthsInYear(int year) const
{
    if (isLeapYear(year)) {
        return 13;
    } else {
        return 12;
    }
}

int KCalendarSystemHebrewPrivate::daysInMonth(int year, int month) const
{
    int mi = monthNumberToMonthIndex(year, month);

    if (mi == 2 && long_cheshvan(year)) {
        return 30;
    }

    if (mi == 3 && short_kislev(year)) {
        return 29;
    }

    if (mi % 2 == 0) {    // Even number months have 29 days
        return 29;
    } else {  // Odd number months have 30 days
        return 30;
    }
}

int KCalendarSystemHebrewPrivate::daysInYear(int year) const
{
    int days;

    // Get Regular year length
    if (isLeapYear(year)) {      // Has 13 months
        days = 384;
    } else {  // Has 12 months
        days = 354;
    }

    // Check if is Deficient or Abundant year
    if (short_kislev(year)) {     // Deficient
        days = days - 1;
    } else if (long_cheshvan(year)) {     // Abundant
        days = days + 1;
    }

    return days;
}

bool KCalendarSystemHebrewPrivate::isLeapYear(int year) const
{
    return ((((7 * year) + 1) % 19) < 7);
}

bool KCalendarSystemHebrewPrivate::hasLeapMonths() const
{
    return true;
}

bool KCalendarSystemHebrewPrivate::hasYearZero() const
{
    return false;
}

int KCalendarSystemHebrewPrivate::maxMonthsInYear() const
{
    return 13;
}

int KCalendarSystemHebrewPrivate::earliestValidYear() const
{
    return 5344;
}

int KCalendarSystemHebrewPrivate::latestValidYear() const
{
    return 8119;
}

int KCalendarSystemHebrewPrivate::integerFromString(const QString &inputString, int maxLength, int &readLength) const
{
    if (locale()->language() == QLatin1String("he")) {

        // Hebrew numbers are composed of combinations of normal letters which have a numeric value.
        // This is a non-positional system, the numeric values are simply added together, however
        // convention is for a RTL highest to lowest value ordering. There is also a degree of
        // ambiguity due to the lack of a letter for 0, hence 5 and 5000 are written the same.
        // Hebrew numbers are only used in dates.
        // See http://www.i18nguy.com/unicode/hebrew-numbers.html for more explaination

        /*
        Ref table for numbers to Hebrew chars

        Value     1       2       3        4        5       6         7        8      9

        x 1    Alef א  Bet  ב  Gimel ג  Dalet ד  He   ה  Vav  ו    Zayen ז  Het  ח  Tet  ט
               0x05D0  0x05D1  0x05D2   0x05D3   0x05D4  0x05D5    0x05D6   0x05D7  0x05D8

        x 10   Yod  י  Kaf  כ  Lamed ל  Mem  מ   Nun  נ  Samekh ס  Ayin ע   Pe   פ  Tzadi צ
               0x05D9  0x05DB  0x05DC   0x05DE   0x05E0  0x05E1    0x05E2   0x05E4  0x05E6

        x 100  Qof  ק  Resh ר  Shin ש   Tav  ת
               0x05E7  0x05E8  0x05E9   0x05EA

        Note special cases 15 = 9 + 6 = 96 טו and 16 = 9 + 7 = 97 טז
        */

        int decadeValues[14] = {10, 20, 20, 30, 40, 40, 50, 50, 60, 70, 80, 80, 90, 90};

        QChar thisChar, nextChar;
        QString string = inputString;

        int stringLength = string.length();
        readLength = 0;
        int position = 0;
        int result = 0;
        int value = 0;

        for (; position < stringLength ; ++position) {

            thisChar = string[position];

            if (position + 1 < stringLength) {
                nextChar = string[position + 1];
                // Ignore any geresh or gershayim chars, we don't bother checking they are in the right place
                if (nextChar == QLatin1Char('\'') ||  nextChar == QChar(0x05F3) ||      // geresh
                        nextChar == QLatin1Char('\"') ||  nextChar == QChar(0x05F4)) {     // gershayim
                    string.remove(position + 1, 1);
                    stringLength = string.length();
                    if (position + 1 < stringLength) {
                        nextChar = string[position + 1];
                    } else {
                        nextChar = QChar();
                    }
                    readLength = readLength + 1;
                }
            } else {
                nextChar = QChar();
            }

            if (thisChar >= QChar(0x05D0) && thisChar <= QChar(0x05D7)) {

                // If this char Alef to Het, 1 to 8, א to ח

                // If next char is any valid digit char (Alef to Tav, 1 to 400, א to ת)
                // then this char is a thousands digit
                // else this char is a ones digit

                if (nextChar >= QChar(0x05D0) && nextChar <= QChar(0x05EA)) {
                    value = (thisChar.unicode() - 0x05D0 + 1) * 1000;
                } else {
                    value = thisChar.unicode() - 0x05D0 + 1;
                }

            } else if (thisChar == QChar(0x05D8)) {

                // If this char is Tet, 9, ט

                // If next char is any valid digit char (Alef to Tav, 1 to 400, א to ת)
                // and next char not 6 (Special case for 96 = 15)
                // and next char not 7 (Special case for 97 = 16)
                // then is a thousands digit else is 9

                if (nextChar >= QChar(0x05D0) && nextChar <= QChar(0x05EA) &&
                        nextChar != QChar(0x05D5) && nextChar != QChar(0x05D6)) {
                    value = 9000;
                } else {
                    value = 9;
                }

            } else if (thisChar >= QChar(0x05D9) && thisChar <= QChar(0x05E6)) {

                // If this char Yod to Tsadi, 10 to 90, י to צ

                // If next char is a tens or hundreds char then is an error
                // Else is a tens digit

                if (nextChar >= QChar(0x05D9)) {
                    return -1;
                } else {
                    value = decadeValues[thisChar.unicode() - 0x05D9];
                }

            } else if (thisChar >= QChar(0x05E7) && thisChar <= QChar(0x05EA)) {

                // If this char Qof to Tav, 100 to 400, ק to ת, then is hundreds digit

                value = (thisChar.unicode() - 0x05E7 + 1) * 100;

            } else {

                // If this char any non-digit char including whitespace or punctuation, we're done
                break;

            }

            result = result + value;

            value = 0;
        }

        readLength += position;

        return result;

    } else {
        return KCalendarSystemPrivate::integerFromString(inputString, maxLength, readLength);
    }
}

QString KCalendarSystemHebrewPrivate::stringFromInteger(int number, int padWidth, QChar padChar) const
{
    return KCalendarSystemPrivate::stringFromInteger(number, padWidth, padChar);
}

QString KCalendarSystemHebrewPrivate::stringFromInteger(int number, int padWidth, QChar padChar, KLocale::DigitSet digitSet) const
{
    if (locale()->language() == QLatin1String("he")) {

        // Hebrew numbers are composed of combinations of normal letters which have a numeric value.
        // This is a non-positional system, the numeric values are simply added together, however
        // convention is for a RTL highest to lowest value ordering. There is also a degree of
        // ambiguity due to the lack of a letter for 0, hence 5 and 5000 are written the same.
        // Hebrew numbers are only used in dates.
        // See http://www.i18nguy.com/unicode/hebrew-numbers.html for more explaination

        /*
        Ref table for numbers to Hebrew chars

        Value     1       2       3        4        5       6         7        8      9

        x 1    Alef א  Bet  ב  Gimel ג  Dalet ד  He   ה  Vav  ו    Zayen ז  Het  ח  Tet  ט
               0x05D0  0x05D1  0x05D2   0x05D3   0x05D4  0x05D5    0x05D6   0x05D7  0x05D8

        x 10   Yod  י  Kaf  כ  Lamed ל  Mem  מ   Nun  נ  Samekh ס  Ayin ע   Pe   פ  Tzadi צ
               0x05D9  0x05DB  0x05DC   0x05DE   0x05E0  0x05E1    0x05E2   0x05E4  0x05E6

        x 100  Qof  ק  Resh ר  Shin ש   Tav  ת
               0x05E7  0x05E8  0x05E9   0x05EA

        Note special cases 15 = 9 + 6 = 96 טו and 16 = 9 + 7 = 97 טז
        */

        const QChar decade[] = {
            //  Tet = ט,    Yod = י,    Kaf = כ,    Lamed = ל,  Mem = מ
            //  Nun = נ,    Samekh = ס, Ayin = ע,   Pe = פ,     Tsadi = צ
            0x05D8,     0x05D9,     0x05DB,     0x05DC,     0x05DE,
            0x05E0,     0x05E1,     0x05E2,     0x05E4,     0x05E6
        };

        QString result;

        // We have no rules for coping with numbers outside this range
        if (number < 1 || number > 9999) {
            return KCalendarSystemPrivate::stringFromInteger(number, padWidth, padChar, digitSet);
        }

        // Translate the thousands digit, just uses letter for number 1..9 ( א to ט, Alef to Tet )
        // Years 5001-5999 do not have the thousands by convention
        if (number >= 1000) {
            if (number <= 5000 || number >= 6000) {
                result += QChar(0x05D0 - 1 + number / 1000);    // Alef א to Tet ט
            }
            number %= 1000;
        }

        // Translate the hundreds digit
        // Use traditional method where we only have letters assigned values for 100, 200, 300 and 400
        // so may need to repeat 400 twice to make up the required number
        if (number >= 100) {
            while (number >= 500) {
                result += QChar(0x05EA);    // Tav = ת
                number -= 400;
            }
            result += QChar(0x05E7 - 1 + number / 100);   // Qof = ק to xxx
            number %= 100;
        }

        // Translate the tens digit
        // The numbers 15 and 16 translate to letters that spell out the name of God which is
        // forbidden, so require special treatment where 15 = 9 + 6 and 1 = 9 + 7.
        if (number >= 10) {
            if (number == 15 || number == 16)
                number -= 9;
            result += decade[number / 10];
            number %= 10;
        }

        // Translate the ones digit, uses letter for number 1..9 ( א to ט, Alef to Tet )
        if (number > 0) {
            result += QChar(0x05D0 - 1 + number);    // Alef = א to xxx
        }

        // When used in a string with mixed names and numbers the numbers need special chars to
        // distinguish them from words composed of the same letters.
        // Single digit numbers are followed by a geresh symbol ? (Unicode = 0x05F3), but we use
        // single quote for convenience.
        // Multiple digit numbers have a gershayim symbol ? (Unicode = 0x05F4) as second-to-last
        // char, but we use double quote for convenience.
        if (result.length() == 1) {
            result += QLatin1Char('\'');
        } else {
            result.insert(result.length() - 1, QLatin1Char('\"'));
        }

        return result;

    } else {
        return KCalendarSystemPrivate::stringFromInteger(number, padWidth, padChar, digitSet);
    }
}

int KCalendarSystemHebrewPrivate::monthNumberToMonthIndex(int year, int month) const
{
    if (isLeapYear(year)) {
        if (month == 6) {
            return 13;        // Adar I
        } else if (month == 7) {
            return 14;        // Adar II
        } else if (month > 7) {
            return month - 1; // Because of Adar II
        }
    }

    return month;
}

QString KCalendarSystemHebrewPrivate::monthName(int month, int year, KLocale::DateTimeComponentFormat format, bool possessive) const
{
    // We must map month number to month index
    int monthIndex = monthNumberToMonthIndex(year, month);

    QStringList languages = locale()->languageList();

    if (format == KLocale::NarrowName) {
        switch (monthIndex) {
        case 1:
            return ki18nc("Hebrew month 1 - KLocale::NarrowName",  "T").toString(languages);
        case 2:
            return ki18nc("Hebrew month 2 - KLocale::NarrowName",  "H").toString(languages);
        case 3:
            return ki18nc("Hebrew month 3 - KLocale::NarrowName",  "K").toString(languages);
        case 4:
            return ki18nc("Hebrew month 4 - KLocale::NarrowName",  "T").toString(languages);
        case 5:
            return ki18nc("Hebrew month 5 - KLocale::NarrowName",  "S").toString(languages);
        case 6:
            return ki18nc("Hebrew month 6 - KLocale::NarrowName",  "A").toString(languages);
        case 7:
            return ki18nc("Hebrew month 7 - KLocale::NarrowName",  "N").toString(languages);
        case 8:
            return ki18nc("Hebrew month 8 - KLocale::NarrowName",  "I").toString(languages);
        case 9:
            return ki18nc("Hebrew month 9 - KLocale::NarrowName",  "S").toString(languages);
        case 10:
            return ki18nc("Hebrew month 10 - KLocale::NarrowName", "T").toString(languages);
        case 11:
            return ki18nc("Hebrew month 11 - KLocale::NarrowName", "A").toString(languages);
        case 12:
            return ki18nc("Hebrew month 12 - KLocale::NarrowName", "E").toString(languages);
        case 13:
            return ki18nc("Hebrew month 13 - KLocale::NarrowName", "A").toString(languages);
        case 14:
            return ki18nc("Hebrew month 14 - KLocale::NarrowName", "A").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName && possessive) {
        switch (monthIndex) {
        case 1:
            return ki18nc("Hebrew month 1 - KLocale::ShortName Possessive",  "of Tis").toString(languages);
        case 2:
            return ki18nc("Hebrew month 2 - KLocale::ShortName Possessive",  "of Hes").toString(languages);
        case 3:
            return ki18nc("Hebrew month 3 - KLocale::ShortName Possessive",  "of Kis").toString(languages);
        case 4:
            return ki18nc("Hebrew month 4 - KLocale::ShortName Possessive",  "of Tev").toString(languages);
        case 5:
            return ki18nc("Hebrew month 5 - KLocale::ShortName Possessive",  "of Shv").toString(languages);
        case 6:
            return ki18nc("Hebrew month 6 - KLocale::ShortName Possessive",  "of Ada").toString(languages);
        case 7:
            return ki18nc("Hebrew month 7 - KLocale::ShortName Possessive",  "of Nis").toString(languages);
        case 8:
            return ki18nc("Hebrew month 8 - KLocale::ShortName Possessive",  "of Iya").toString(languages);
        case 9:
            return ki18nc("Hebrew month 9 - KLocale::ShortName Possessive",  "of Siv").toString(languages);
        case 10:
            return ki18nc("Hebrew month 10 - KLocale::ShortName Possessive", "of Tam").toString(languages);
        case 11:
            return ki18nc("Hebrew month 11 - KLocale::ShortName Possessive", "of Av").toString(languages);
        case 12:
            return ki18nc("Hebrew month 12 - KLocale::ShortName Possessive", "of Elu").toString(languages);
        case 13:
            return ki18nc("Hebrew month 13 - KLocale::ShortName Possessive", "of Ad1").toString(languages);
        case 14:
            return ki18nc("Hebrew month 14 - KLocale::ShortName Possessive", "of Ad2").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName && !possessive) {
        switch (monthIndex) {
        case 1:
            return ki18nc("Hebrew month 1 - KLocale::ShortName",  "Tis").toString(languages);
        case 2:
            return ki18nc("Hebrew month 2 - KLocale::ShortName",  "Hes").toString(languages);
        case 3:
            return ki18nc("Hebrew month 3 - KLocale::ShortName",  "Kis").toString(languages);
        case 4:
            return ki18nc("Hebrew month 4 - KLocale::ShortName",  "Tev").toString(languages);
        case 5:
            return ki18nc("Hebrew month 5 - KLocale::ShortName",  "Shv").toString(languages);
        case 6:
            return ki18nc("Hebrew month 6 - KLocale::ShortName",  "Ada").toString(languages);
        case 7:
            return ki18nc("Hebrew month 7 - KLocale::ShortName",  "Nis").toString(languages);
        case 8:
            return ki18nc("Hebrew month 8 - KLocale::ShortName",  "Iya").toString(languages);
        case 9:
            return ki18nc("Hebrew month 9 - KLocale::ShortName",  "Siv").toString(languages);
        case 10:
            return ki18nc("Hebrew month 10 - KLocale::ShortName", "Tam").toString(languages);
        case 11:
            return ki18nc("Hebrew month 11 - KLocale::ShortName", "Av").toString(languages);
        case 12:
            return ki18nc("Hebrew month 12 - KLocale::ShortName", "Elu").toString(languages);
        case 13:
            return ki18nc("Hebrew month 13 - KLocale::ShortName", "Ad1").toString(languages);
        case 14:
            return ki18nc("Hebrew month 14 - KLocale::ShortName", "Ad2").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::LongName && possessive) {
        switch (monthIndex) {
        case 1:
            return ki18nc("Hebrew month 1 - KLocale::LongName Possessive",  "of Tishrey").toString(languages);
        case 2:
            return ki18nc("Hebrew month 2 - KLocale::LongName Possessive",  "of Heshvan").toString(languages);
        case 3:
            return ki18nc("Hebrew month 3 - KLocale::LongName Possessive",  "of Kislev").toString(languages);
        case 4:
            return ki18nc("Hebrew month 4 - KLocale::LongName Possessive",  "of Tevet").toString(languages);
        case 5:
            return ki18nc("Hebrew month 5 - KLocale::LongName Possessive",  "of Shvat").toString(languages);
        case 6:
            return ki18nc("Hebrew month 6 - KLocale::LongName Possessive",  "of Adar").toString(languages);
        case 7:
            return ki18nc("Hebrew month 7 - KLocale::LongName Possessive",  "of Nisan").toString(languages);
        case 8:
            return ki18nc("Hebrew month 8 - KLocale::LongName Possessive",  "of Iyar").toString(languages);
        case 9:
            return ki18nc("Hebrew month 9 - KLocale::LongName Possessive",  "of Sivan").toString(languages);
        case 10:
            return ki18nc("Hebrew month 10 - KLocale::LongName Possessive", "of Tamuz").toString(languages);
        case 11:
            return ki18nc("Hebrew month 11 - KLocale::LongName Possessive", "of Av").toString(languages);
        case 12:
            return ki18nc("Hebrew month 12 - KLocale::LongName Possessive", "of Elul").toString(languages);
        case 13:
            return ki18nc("Hebrew month 13 - KLocale::LongName Possessive", "of Adar I").toString(languages);
        case 14:
            return ki18nc("Hebrew month 14 - KLocale::LongName Possessive", "of Adar II").toString(languages);
        default:
            return QString();
        }
    }

    // Default to LongName
    switch (monthIndex) {
    case 1:
        return ki18nc("Hebrew month 1 - KLocale::LongName",  "Tishrey").toString(languages);
    case 2:
        return ki18nc("Hebrew month 2 - KLocale::LongName",  "Heshvan").toString(languages);
    case 3:
        return ki18nc("Hebrew month 3 - KLocale::LongName",  "Kislev").toString(languages);
    case 4:
        return ki18nc("Hebrew month 4 - KLocale::LongName",  "Tevet").toString(languages);
    case 5:
        return ki18nc("Hebrew month 5 - KLocale::LongName",  "Shvat").toString(languages);
    case 6:
        return ki18nc("Hebrew month 6 - KLocale::LongName",  "Adar").toString(languages);
    case 7:
        return ki18nc("Hebrew month 7 - KLocale::LongName",  "Nisan").toString(languages);
    case 8:
        return ki18nc("Hebrew month 8 - KLocale::LongName",  "Iyar").toString(languages);
    case 9:
        return ki18nc("Hebrew month 9 - KLocale::LongName",  "Sivan").toString(languages);
    case 10:
        return ki18nc("Hebrew month 10 - KLocale::LongName", "Tamuz").toString(languages);
    case 11:
        return ki18nc("Hebrew month 11 - KLocale::LongName", "Av").toString(languages);
    case 12:
        return ki18nc("Hebrew month 12 - KLocale::LongName", "Elul").toString(languages);
    case 13:
        return ki18nc("Hebrew month 13 - KLocale::LongName", "Adar I").toString(languages);
    case 14:
        return ki18nc("Hebrew month 14 - KLocale::LongName", "Adar II").toString(languages);
    default:
        return QString();
    }
}

// Use Western day names for now as that's what the old version did,
// but wouldn't it be better to use the right Hebrew names like Shabbat?
// Could make it switchable by adding new enums to WeekDayFormat, e.g. ShortNameWestern?
QString KCalendarSystemHebrewPrivate::weekDayName(int weekDay, KLocale::DateTimeComponentFormat format) const
{
    QStringList languages = locale()->languageList();

    if (format == KLocale::NarrowName) {
        switch (weekDay) {
        case 1:
            return ki18nc("Gregorian weekday 1 - KLocale::NarrowName ", "M").toString(languages);
        case 2:
            return ki18nc("Gregorian weekday 2 - KLocale::NarrowName ", "T").toString(languages);
        case 3:
            return ki18nc("Gregorian weekday 3 - KLocale::NarrowName ", "W").toString(languages);
        case 4:
            return ki18nc("Gregorian weekday 4 - KLocale::NarrowName ", "T").toString(languages);
        case 5:
            return ki18nc("Gregorian weekday 5 - KLocale::NarrowName ", "F").toString(languages);
        case 6:
            return ki18nc("Gregorian weekday 6 - KLocale::NarrowName ", "S").toString(languages);
        case 7:
            return ki18nc("Gregorian weekday 7 - KLocale::NarrowName ", "S").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName  || format == KLocale:: ShortNumber) {
        switch (weekDay) {
        case 1:
            return ki18nc("Gregorian weekday 1 - KLocale::ShortName", "Mon").toString(languages);
        case 2:
            return ki18nc("Gregorian weekday 2 - KLocale::ShortName", "Tue").toString(languages);
        case 3:
            return ki18nc("Gregorian weekday 3 - KLocale::ShortName", "Wed").toString(languages);
        case 4:
            return ki18nc("Gregorian weekday 4 - KLocale::ShortName", "Thu").toString(languages);
        case 5:
            return ki18nc("Gregorian weekday 5 - KLocale::ShortName", "Fri").toString(languages);
        case 6:
            return ki18nc("Gregorian weekday 6 - KLocale::ShortName", "Sat").toString(languages);
        case 7:
            return ki18nc("Gregorian weekday 7 - KLocale::ShortName", "Sun").toString(languages);
        default: return QString();
        }
    }

    switch (weekDay) {
    case 1:
        return ki18nc("Gregorian weekday 1 - KLocale::LongName", "Monday").toString(languages);
    case 2:
        return ki18nc("Gregorian weekday 2 - KLocale::LongName", "Tuesday").toString(languages);
    case 3:
        return ki18nc("Gregorian weekday 3 - KLocale::LongName", "Wednesday").toString(languages);
    case 4:
        return ki18nc("Gregorian weekday 4 - KLocale::LongName", "Thursday").toString(languages);
    case 5:
        return ki18nc("Gregorian weekday 5 - KLocale::LongName", "Friday").toString(languages);
    case 6:
        return ki18nc("Gregorian weekday 6 - KLocale::LongName", "Saturday").toString(languages);
    case 7:
        return ki18nc("Gregorian weekday 7 - KLocale::LongName", "Sunday").toString(languages);
    default:
        return QString();
    }
}


KCalendarSystemHebrew::KCalendarSystemHebrew(const KSharedConfig::Ptr config, const KLocale *locale)
                     : KCalendarSystem(*new KCalendarSystemHebrewPrivate(this), config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemHebrew::KCalendarSystemHebrew(KCalendarSystemHebrewPrivate &dd,
                                             const KSharedConfig::Ptr config, const KLocale *locale)
                     : KCalendarSystem(dd, config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemHebrew::~KCalendarSystemHebrew()
{
}

QString KCalendarSystemHebrew::calendarType() const
{
    return QLatin1String("hebrew");
}

KLocale::CalendarSystem KCalendarSystemHebrew::calendarSystem() const
{
    return KLocale::HebrewCalendar;
}

QDate KCalendarSystemHebrew::epoch() const
{
    // Hebrew 0001-01-01 (Gregorian -3760-09-07, Julian -3761-10-07)
    return QDate::fromJulianDay(347998);
}

QDate KCalendarSystemHebrew::earliestValidDate() const
{
    // Current formulas using direct Gregorian <-> Hebrew conversion using Qt
    // will return invalid results prior to the Gregorian switchover in 1582
    // Next valid Hebrew year starts 5344-01-01 (Gregorian 1583-09-17)
    return QDate::fromJulianDay(2299498);
}

QDate KCalendarSystemHebrew::latestValidDate() const
{
    // Testing shows current formulas only work up to 8119-13-29 (Gregorian 4359-10-07)
    return QDate::fromJulianDay(3313431);
}

QString KCalendarSystemHebrew::monthName(int month, int year, MonthNameFormat format) const
{
    return KCalendarSystem::monthName(month, year, format);
}

QString KCalendarSystemHebrew::monthName(const QDate &date, MonthNameFormat format) const
{
    return KCalendarSystem::monthName(date, format);
}

QString KCalendarSystemHebrew::weekDayName(int weekDay, WeekDayNameFormat format) const
{
    return KCalendarSystem::weekDayName(weekDay, format);
}

QString KCalendarSystemHebrew::weekDayName(const QDate &date, WeekDayNameFormat format) const
{
    return KCalendarSystem::weekDayName(date, format);
}

int KCalendarSystemHebrew::yearStringToInteger(const QString &string, int &readLength) const
{
    int result = KCalendarSystem::yearStringToInteger(string, readLength);

    // Hebrew has no letter for 0, so 5 and 5000 are written the same
    // Assume if less than 10 then we are in an exact multiple of 1000
    if (result < 10) {
        result = result * 1000;
    }

    // Not good just assuming, make configurable
    if (result < 1000) {
        result += 5000; // assume we're in the 6th millenium (y6k bug)
    }

    return result;
}

bool KCalendarSystemHebrew::isLunar() const
{
    return false;
}

bool KCalendarSystemHebrew::isLunisolar() const
{
    return true;
}

bool KCalendarSystemHebrew::isSolar() const
{
    return false;
}

bool KCalendarSystemHebrew::isProleptic() const
{
    return false;
}

bool KCalendarSystemHebrew::julianDayToDate(qint64 jd, int &year, int &month, int &day) const
{
    class h_date * sd = toHebrew(QDate::fromJulianDay(jd));

    year = sd->hd_year;

    month = sd->hd_mon;
    if (isLeapYear(sd->hd_year)) {
        if (month == 13 /*AdarI*/) {
            month = 6;
        } else if (month == 14 /*AdarII*/) {
            month = 7;
        } else if (month > 6 && month < 13) {
            ++month;
        }
    }

    day = sd->hd_day;

    return true;
}

bool KCalendarSystemHebrew::dateToJulianDay(int year, int month, int day, qint64 &jd) const
{
    class h_date * gd = hebrewToGregorian(year, month, day);

    QDate tempDate(gd->hd_year, gd->hd_mon + 1, gd->hd_day + 1);

    jd = tempDate.toJulianDay();

    return true;
}
