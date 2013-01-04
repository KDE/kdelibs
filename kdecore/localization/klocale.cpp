/* This file is part of the KDE libraries
   Copyright (c) 1997,2001 Stephan Kulow <coolo@kde.org>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1999-2002 Hans Petter Bieker <bieker@kde.org>
   Copyright (c) 2002 Lukas Tinkl <lukas@kde.org>
   Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>
   Copyright (C) 2009, 2010 John Layt <john@layt.net>

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

#include "klocale.h"
#if defined Q_OS_WIN
    #include "klocale_win_p.h"
#elif defined Q_OS_MAC
    #include "klocale_mac_p.h"
#else
    #include "klocale_unix_p.h"
#endif

#include <QMutex>
#include <QThread>
#include <QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QTextCodec>

#include "kconfig.h"
#include "kdatetime.h"
#include "kcalendarsystem.h"
#include "kdayperiod_p.h"
#include "kcurrencycode.h"

KLocale::KLocale(KSharedConfig::Ptr config)
#if defined Q_OS_WIN
        : d(new KLocaleWindowsPrivate(this, config))
#elif defined Q_OS_MAC
        : d(new KLocaleMacPrivate(this, config))
#else
        : d(new KLocaleUnixPrivate(this, config))
#endif
{
}

KLocale::KLocale(const QString &language, const QString &country, KConfig *config)
#if defined Q_OS_WIN
        : d(new KLocaleWindowsPrivate(this, language, country, config))
#elif defined Q_OS_MAC
        : d(new KLocaleMacPrivate(this, language, country, config))
#else
        : d(new KLocaleUnixPrivate(this, language, country, config))
#endif
{
}

KLocale::~KLocale()
{
    delete d;
}

bool KLocale::setCountry(const QString &country, KConfig *config)
{
    return d->setCountry(country, config);
}

bool KLocale::setCountryDivisionCode(const QString &countryDivisionCode)
{
    return d->setCountryDivisionCode(countryDivisionCode);
}

bool KLocale::setLanguage(const QString &language, KConfig *config)
{
    return d->setLanguage(language, config);
}

bool KLocale::setLanguage(const QStringList &languages)
{
    return d->setLanguage(languages);
}

void KLocale::setCurrencyCode(const QString &newCurrencyCode)
{
    d->setCurrencyCode(newCurrencyCode);
}

void KLocale::splitLocale(const QString &locale, QString &language, QString &country, QString &modifier,
                          QString &charset)
{
    KLocalePrivate::splitLocale(locale, language, country, modifier, charset);
}

QString KLocale::language() const
{
    return d->language();
}

QString KLocale::country() const
{
    return d->country();
}

QString KLocale::countryDivisionCode() const
{
    return d->countryDivisionCode();
}

KCurrencyCode *KLocale::currency() const
{
    return d->currency();
}

QString KLocale::currencyCode() const
{
    return d->currencyCode();
}

QList<KLocale::DigitSet> KLocale::allDigitSetsList() const
{
    return d->allDigitSetsList();
}

QString KLocale::digitSetToName(KLocale::DigitSet digitSet, bool withDigits) const
{
    return d->digitSetToName(digitSet, withDigits);
}

QString KLocale::convertDigits(const QString &str, DigitSet digitSet, bool ignoreContext) const
{
    return d->convertDigits(str, digitSet, ignoreContext);
}

bool KLocale::nounDeclension() const
{
    return d->nounDeclension();
}

bool KLocale::dateMonthNamePossessive() const
{
    return d->dateMonthNamePossessive();
}

int KLocale::weekStartDay() const
{
    return d->weekStartDay();
}

int KLocale::workingWeekStartDay() const
{
    return d->workingWeekStartDay();
}

int KLocale::workingWeekEndDay() const
{
    return d->workingWeekEndDay();
}

int KLocale::weekDayOfPray() const
{
    return d->weekDayOfPray();
}

int KLocale::decimalPlaces() const
{
    return d->decimalPlaces();
}

QString KLocale::decimalSymbol() const
{
    return d->decimalSymbol();
}

QString KLocale::thousandsSeparator() const
{
    return d->thousandsSeparator();
}

QString KLocale::currencySymbol() const
{
    return d->currencySymbol();
}

QString KLocale::monetaryDecimalSymbol() const
{
    return d->monetaryDecimalSymbol();
}

QString KLocale::monetaryThousandsSeparator() const
{
    return d->monetaryThousandsSeparator();
}

QString KLocale::positiveSign() const
{
    return d->positiveSign();
}

QString KLocale::negativeSign() const
{
    return d->negativeSign();
}

int KLocale::fracDigits() const
{
    return monetaryDecimalPlaces();
}

int KLocale::monetaryDecimalPlaces() const
{
    return d->monetaryDecimalPlaces();
}

bool KLocale::positivePrefixCurrencySymbol() const
{
    return d->positivePrefixCurrencySymbol();
}

bool KLocale::negativePrefixCurrencySymbol() const
{
    return d->negativePrefixCurrencySymbol();
}

KLocale::SignPosition KLocale::positiveMonetarySignPosition() const
{
    return d->positiveMonetarySignPosition();
}

KLocale::SignPosition KLocale::negativeMonetarySignPosition() const
{
    return d->negativeMonetarySignPosition();
}

QString KLocale::formatMoney(double num, const QString &symbol, int precision) const
{
    return d->formatMoney(num, symbol, precision);
}

QString KLocale::formatNumber(double num, int precision) const
{
    return d->formatNumber(num, precision);
}

QString KLocale::formatLong(long num) const
{
    return d->formatLong(num);
}

QString KLocale::formatNumber(const QString &numStr, bool round, int precision) const
{
    return d->formatNumber(numStr, round, precision);
}

QString KLocale::formatByteSize(double size, int precision, KLocale::BinaryUnitDialect dialect,
                                KLocale::BinarySizeUnits specificUnit) const
{
    return d->formatByteSize(size, precision, dialect, specificUnit);
}

QString KLocale::formatByteSize(double size) const
{
    return d->formatByteSize(size);
}

KLocale::BinaryUnitDialect KLocale::binaryUnitDialect() const
{
    return d->binaryUnitDialect();
}

void KLocale::setBinaryUnitDialect(KLocale::BinaryUnitDialect newDialect)
{
    d->setBinaryUnitDialect(newDialect);
}

QString KLocale::formatDuration(unsigned long mSec) const
{
    return d->formatDuration(mSec);
}

QString KLocale::prettyFormatDuration(unsigned long mSec) const
{
    return d->prettyFormatDuration(mSec);
}

QString KLocale::formatDate(const QDate &date, KLocale::DateFormat format) const
{
    return d->formatDate(date, format);
}

class KGlobalLocaleStatic
{
public:
    KGlobalLocaleStatic()
        : locale(),
          mutex(QMutex::Recursive),
          inited(false)
    {
    }

    // This cannot be called from the constructor, because the LanguageChange event
    // calls tr() which calls KLocale::global(), so KLocale::global() must exist already.
    void init()
    {
        QMutexLocker lock(&mutex);
        // If there's no QApp, postpone initialization
        QCoreApplication* coreApp = QCoreApplication::instance();
        if (inited || !coreApp)
            return;
        inited = true;
        QTextCodec::setCodecForLocale(locale.codecForEncoding());
        if (coreApp->thread() != QThread::currentThread()) {
            qFatal("KLocale::global() must be called from the main thread before using i18n() in threads. KApplication takes care of this. If not using KApplication, call KLocale::global() during initialization.");
        } else {
            QCoreApplication::installTranslator(new KDETranslator(coreApp));
        }
    }
    KLocale locale;
    QMutex mutex;
    bool inited;
};

Q_GLOBAL_STATIC(KGlobalLocaleStatic, s_globalLocale)

KLocale * KLocale::global()
{
    KGlobalLocaleStatic* glob = s_globalLocale();
    glob->init();
    return &glob->locale;
}

double KLocale::readNumber(const QString &_str, bool * ok) const
{
    return d->readNumber(_str, ok);
}

double KLocale::readMoney(const QString &_str, bool *ok) const
{
    return d->readMoney(_str, ok);
}

QDate KLocale::readDate(const QString &intstr, bool *ok) const
{
    return d->readDate(intstr, ok);
}

QDate KLocale::readDate(const QString &intstr, ReadDateFlags flags, bool *ok) const
{
    return d->readDate(intstr, flags, ok);
}

QDate KLocale::readDate(const QString &intstr, const QString &fmt, bool *ok) const
{
    return d->readDate(intstr, fmt, ok);
}

QTime KLocale::readTime(const QString &intstr, bool *ok) const
{
    return d->readTime(intstr, ok);
}

QTime KLocale::readTime(const QString &intstr, KLocale::ReadTimeFlags flags, bool *ok) const
{
    return d->readTime(intstr, flags, ok);
}

QTime KLocale::readLocaleTime(const QString &intstr, bool *ok, TimeFormatOptions options,
                              TimeProcessingOptions processing) const
{
    return d->readLocaleTime(intstr, ok, options, processing);
}

QString KLocale::formatTime(const QTime &time, bool includeSecs, bool isDuration) const
{
    return d->formatTime(time, includeSecs, isDuration);
}

QString KLocale::formatLocaleTime(const QTime &time, TimeFormatOptions options) const
{
    return d->formatLocaleTime(time, options);
}

bool KLocale::use12Clock() const
{
    return d->use12Clock();
}

QString KLocale::dayPeriodText(const QTime &time, DateTimeComponentFormat format) const
{
    return d->dayPeriodForTime(time).periodName(format);
}

QStringList KLocale::languageList() const
{
    return d->languageList();
}

QStringList KLocale::currencyCodeList() const
{
    return d->currencyCodeList();
}

/* Just copy in for now to keep diff clean, remove later
QString KLocalePrivate::formatDateTime(const KLocale *locale, const QDateTime &dateTime, KLocale::DateFormat format,
                                       bool includeSeconds, int daysTo, int secsTo)
{
}
*/

QString KLocale::formatDateTime(const QDateTime &dateTime, KLocale::DateFormat format, bool includeSeconds) const
{
    return d->formatDateTime(dateTime, format, includeSeconds);
}

QString KLocale::formatDateTime(const KDateTime &dateTime, KLocale::DateFormat format, DateTimeFormatOptions options) const
{
    return d->formatDateTime(dateTime, format, options);
}

void KLocale::setDateFormat(const QString &format)
{
    d->setDateFormat(format);
}

void KLocale::setDateFormatShort(const QString &format)
{
    d->setDateFormatShort(format);
}

void KLocale::setDateMonthNamePossessive(bool possessive)
{
    d->setDateMonthNamePossessive(possessive);
}

void KLocale::setTimeFormat(const QString &format)
{
    d->setTimeFormat(format);
}

void KLocale::setWeekStartDay(int day)
{
    d->setWeekStartDay(day);
}

void KLocale::setWorkingWeekStartDay(int day)
{
    d->setWorkingWeekStartDay(day);
}

void KLocale::setWorkingWeekEndDay(int day)
{
    d->setWorkingWeekEndDay(day);
}

void KLocale::setWeekDayOfPray(int day)
{
    d->setWeekDayOfPray(day);
}

QString KLocale::dateFormat() const
{
    return d->dateFormat();
}

QString KLocale::dateFormatShort() const
{
    return d->dateFormatShort();
}

QString KLocale::timeFormat() const
{
    return d->timeFormat();
}

void KLocale::setDecimalPlaces(int digits)
{
    d->setDecimalPlaces(digits);
}

void KLocale::setDecimalSymbol(const QString &symbol)
{
    d->setDecimalSymbol(symbol);
}

void KLocale::setThousandsSeparator(const QString &separator)
{
    d->setThousandsSeparator(separator);
}

void KLocale::setPositiveSign(const QString &sign)
{
    d->setPositiveSign(sign);
}

void KLocale::setNegativeSign(const QString &sign)
{
    d->setNegativeSign(sign);
}

void KLocale::setPositiveMonetarySignPosition(KLocale::SignPosition signpos)
{
    d->setPositiveMonetarySignPosition(signpos);
}

void KLocale::setNegativeMonetarySignPosition(KLocale::SignPosition signpos)
{
    d->setNegativeMonetarySignPosition(signpos);
}

void KLocale::setPositivePrefixCurrencySymbol(bool prefix)
{
    d->setPositivePrefixCurrencySymbol(prefix);
}

void KLocale::setNegativePrefixCurrencySymbol(bool prefix)
{
    d->setNegativePrefixCurrencySymbol(prefix);
}

void KLocale::setFracDigits(int digits)
{
    setMonetaryDecimalPlaces(digits);
}

void KLocale::setMonetaryDecimalPlaces(int digits)
{
    d->setMonetaryDecimalPlaces(digits);
}

void KLocale::setMonetaryThousandsSeparator(const QString &separator)
{
    d->setMonetaryThousandsSeparator(separator);
}

void KLocale::setMonetaryDecimalSymbol(const QString &symbol)
{
    d->setMonetaryDecimalSymbol(symbol);
}

void KLocale::setCurrencySymbol(const QString & symbol)
{
    d->setCurrencySymbol(symbol);
}

int KLocale::pageSize() const
{
    return d->pageSize();
}

void KLocale::setPageSize(int size)
{
    d->setPageSize(size);
}

KLocale::MeasureSystem KLocale::measureSystem() const
{
    return d->measureSystem();
}

void KLocale::setMeasureSystem(KLocale::MeasureSystem value)
{
    d->setMeasureSystem(value);
}

QString KLocale::defaultLanguage()
{
    return KLocalePrivate::defaultLanguage();
}

QString KLocale::defaultCountry()
{
    return KLocalePrivate::defaultCountry();
}

QString KLocale::defaultCurrencyCode()
{
    return KLocalePrivate::defaultCurrencyCode();
}

const QByteArray KLocale::encoding() const
{
    return d->encoding();
}

int KLocale::encodingMib() const
{
    return d->encodingMib();
}

int KLocale::fileEncodingMib() const
{
    return d->fileEncodingMib();
}

QTextCodec *KLocale::codecForEncoding() const
{
    return d->codecForEncoding();
}

bool KLocale::setEncoding(int mibEnum)
{
    return d->setEncoding(mibEnum);
}

QStringList KLocale::allLanguagesList() const
{
    return d->allLanguagesList();
}

QStringList KLocale::installedLanguages() const
{
    return d->installedLanguages();
}

QString KLocale::languageCodeToName(const QString &language) const
{
    return d->languageCodeToName(language);
}

QStringList KLocale::allCountriesList() const
{
    return d->allCountriesList();
}

QString KLocale::countryCodeToName(const QString &country) const
{
    return d->countryCodeToName(country);
}

void KLocale::setCalendar(const QString &calendarType)
{
    d->setCalendar(calendarType);
}

void KLocale::setCalendarSystem(KLocale::CalendarSystem calendarSystem)
{
    d->setCalendarSystem(calendarSystem);
}

QString KLocale::calendarType() const
{
    return d->calendarType();
}

KLocale::CalendarSystem KLocale::calendarSystem() const
{
    return d->calendarSystem();
}

const KCalendarSystem * KLocale::calendar() const
{
    return d->calendar();
}

void KLocale::setWeekNumberSystem(KLocale::WeekNumberSystem weekNumberSystem)
{
    d->setWeekNumberSystem(weekNumberSystem);
}

KLocale::WeekNumberSystem KLocale::weekNumberSystem()
{
    return d->weekNumberSystem();
}

KLocale::WeekNumberSystem KLocale::weekNumberSystem() const
{
    return d->weekNumberSystem();
}

KLocale::KLocale(const KLocale &rhs)
        : d(new KLocalePrivate(*rhs.d))
{
    d->q = this;
}

KLocale & KLocale::operator=(const KLocale & rhs)
{
    // the assignment operator works here
    *d = *rhs.d;
    d->q = this;
    return *this;
}

void KLocale::setDigitSet(KLocale::DigitSet digitSet)
{
    d->setDigitSet(digitSet);
}

KLocale::DigitSet KLocale::digitSet() const
{
    return d->digitSet();
}

void KLocale::setMonetaryDigitSet(KLocale::DigitSet digitSet)
{
    d->setMonetaryDigitSet(digitSet);
}

KLocale::DigitSet KLocale::monetaryDigitSet() const
{
    return d->monetaryDigitSet();
}

void KLocale::setDateTimeDigitSet(KLocale::DigitSet digitSet)
{
    d->setDateTimeDigitSet(digitSet);
}

KLocale::DigitSet KLocale::dateTimeDigitSet() const
{
    return d->dateTimeDigitSet();
}

void KLocale::reparseConfiguration()
{
    d->initFormat();
}
