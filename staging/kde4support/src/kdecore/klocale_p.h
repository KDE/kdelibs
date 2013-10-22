/*  This file is part of the KDE libraries
 *  Copyright 2009 David Faure <faure@kde.org>
 *  Copyright 2010 John Layt <john@layt.net>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef KLOCALE_P_H
#define KLOCALE_P_H

#include "klocale.h"
#include "klocalizedstring.h"

class KCatalog;
class KDayPeriod;

class KLocalePrivate
{
public:
    /**
     * Constructors
     *
     * This class should not be instantited directly, it is intended as a base class for each
     * platform to provide a common KDE fallback implementation.  Instead use the relevant
     * derived system class for Unix, Win, or Mac which will prefer the local platform settings
     * where possible.
     */
    KLocalePrivate(KLocale *q_ptr);

    /**
     * Copy constructor
     */
    KLocalePrivate(const KLocalePrivate &rhs);

    /**
     * Assignment operator
     */
    KLocalePrivate &operator=(const KLocalePrivate &rhs);

    /**
     * Destructor
     */
    virtual ~KLocalePrivate();

    /**************************
     **    Initialization    **
     **************************/

    /**
     * Reads the format configuration from disk.
     */
    virtual void initFormat();

protected:

    /**
     * @internal Returns config object
     */
    KSharedConfigPtr config();

    /**
     * @internal Copies object members
     */
    virtual void copy(const KLocalePrivate &rhs);

    /**
     * @internal Main init function, needs to be called by appropriate child constructor.
     */
    virtual void init(const QString &language, const QString &country,
                      KSharedConfig::Ptr persistantconfig, KConfig *tempConfig);

    /**
     * @internal Init config.
     */
    virtual void initConfig(KConfig *config);

    /**************************
     **   Country settings   **
     **************************/

protected:

    /**
     * @internal Initializes the country if not already explicitly set when calling the constructor
     * Will default to any value set in the config, otherwise will attempt to use the host system
     * country, or finally fall back to the default C.
     */
    virtual void initCountry(const QString &country, const QString &configCountry);

    /**
     * @internal Returns the host system country ISO code
     * If country could not be determined then may return an empty string or "C"
     */
    virtual QString systemCountry() const;

public:

    /**
     * @internal Sets the Locale Country
     * The worker of the same-name KLocale API function.
     */
    virtual bool setCountry(const QString &country, KConfig *config);

    /**
     * @internal Returns the Locale ISO Country Code
     * The worker of the same-name KLocale API function.
     */
    virtual QString country() const;

    /**
     * @internal Returns the name of the default country.
     * The worker of the same-name KLocale API function.
     */
    static QString defaultCountry();

    /**
     * @internal Returns a list of all known country codes.
     * The worker of the same-name KLocale API function.
     */
    virtual QStringList allCountriesList() const;

    /**
     * @internal Convert a known country code to a human readable, localized form.
     * The worker of the same-name KLocale API function.
     */
    virtual QString countryCodeToName(const QString &country) const;

    /**
     * @internal Sets the Country Division Code
     * The worker of the same-name KLocale API function.
     */
    bool setCountryDivisionCode(const QString & countryDivision);

    /**
     * @internal Returns the Country Division Code
     * The worker of the same-name KLocale API function.
     */
    QString countryDivisionCode() const;

    /**************************
     **  Language settings   **
     **************************/

protected:

    /**
     * @internal Converts a language variable in a list of languages
     */
    static void getLanguagesFromVariable(QStringList &list, const char *variable, bool isLanguageList = false);

    /**
     * @internal Returns the list of host system languages
     */
    virtual QStringList systemLanguageList() const;

    /**
     * @internal Initializes the list of valid languages from the user's point of view. This is the
     * list of languages that the user picks in kcontrol. The config object should be valid and
     * contain the global entries.
     *
     * @param language The defualt language to use
     * @param configLanguages The "Language" setting from the current config
     * @param useEnv Whether to use the envvars
     */
    virtual void initLanguageList(const QString &language, const QString &configLanguages, bool useEnv);

    /**
     * @internal function used to determine if we are using the en_US translation
     */
    virtual bool useDefaultLanguage() const;

public:

    /**
     * @internal Sets the Locale Language
     * The worker of the same-name KLocale API function.
     */
    virtual bool setLanguage(const QString &language, KConfig *config);

    /**
     * @internal Sets the list of preferred languages for the locale.
     * The worker of the same-name KLocale API function.
     */
    virtual bool setLanguage(const QStringList &languages);

    /**
     * @internal Returns the Locale Language
     * The worker of the same-name KLocale API function.
     */
    virtual QString language() const;

    /**
     * @internal Returns the language codes selected by user
     * The worker of the same-name KLocale API function.
     */
    virtual QStringList languageList() const;

    /**
     * @internal Returns a list of all known language codes.
     * The worker of the same-name KLocale API function.
     */
    virtual QStringList allLanguagesList();

    /**
     * @internal Returns a list of all installed KDE languages.
     * The worker of the same-name KLocale API function.
     */
    virtual QStringList installedLanguages();

    /**
     * @internal Returns the name of the internal language.
     * The worker of the same-name KLocale API function.
     */
    static QString defaultLanguage();

    /**
     * @internal Convert a known language code to a human readable, localized form.
     * The worker of the same-name KLocale API function.
     */
    virtual QString languageCodeToName(const QString &language);

    /**
     * @deprecated
     * @internal Returns if nouns are declined in the locale language.
     * The worker of the same-name KLocale API function.
     */
    virtual bool nounDeclension() const;

    /***************************
     **   Calendar settings   **
     ***************************/

protected:

    /**
     * @internal Converts a CalendarType into a CalendarSystem
     */
    virtual KLocale::CalendarSystem calendarTypeToCalendarSystem(const QString &calendarType) const;

    /**
     * @internal Converts a CalendarSystem into a CalendarType
     */
    virtual QString calendarSystemToCalendarType(KLocale::CalendarSystem) const;

public:

    /**
     * @internal Sets the current calendar system to the calendar specified.
     * The worker of the same-name KLocale API function.
     */
    virtual void setCalendar(const QString &calendarType);

    /**
     * @internal Sets the current calendar system to the calendar specified.
     * The worker of the same-name KLocale API function.
     */
    virtual void setCalendarSystem(KLocale::CalendarSystem);

    /**
     * @internal Returns the name of the calendar system that is currently being used by the system.
     * The worker of the same-name KLocale API function.
     */
    virtual QString calendarType() const;

    /**
     * @internal Returns the type of the calendar system that is currently being used by the system.
     * The worker of the same-name KLocale API function.
     */
    KLocale::CalendarSystem calendarSystem() const;

    /**
     * @internal Returns a pointer to the calendar system object.
     * The worker of the same-name KLocale API function.
     */
    virtual const KCalendarSystem *calendar();

    /**
     * @internal Sets the Week Number System to use
     * The worker of the same-name KLocale API function.
     */
    void setWeekNumberSystem(KLocale::WeekNumberSystem weekNumberSystem);

    /**
     * @internal Returns the Week Number System used
     * The worker of the same-name KLocale API function.
     */
    KLocale::WeekNumberSystem weekNumberSystem();

    /**
     * @internal Changes how KLocale defines the first day in week.
     * The worker of the same-name KLocale API function.
     */
    virtual void setWeekStartDay(int day);

    /**
     * @internal Returns which day is the first day of the week.
     * The worker of the same-name KLocale API function.
     */
    virtual int weekStartDay() const;

    /**
     * @internal Changes how KLocale defines the first working day in week.
     * The worker of the same-name KLocale API function.
     */
    virtual void setWorkingWeekStartDay(int day);

    /**
     * @internal Returns which day is the first working day of the week.
     * The worker of the same-name KLocale API function.
     */
    virtual int workingWeekStartDay() const;

    /**
     * @internal Changes how KLocale defines the last working day in week.
     * The worker of the same-name KLocale API function.
     */
    virtual void setWorkingWeekEndDay(int day);

    /**
     * @internal Returns which day is the last working day of the week.
     * The worker of the same-name KLocale API function.
     */
    virtual int workingWeekEndDay() const;

    /**
     * @internal Changes how KLocale defines the day reserved for religious observance.
     * The worker of the same-name KLocale API function.
     */
    virtual void setWeekDayOfPray(int day);

    /**
     * @internal Returns which day is reserved for religious observance
     * The worker of the same-name KLocale API function.
     */
    virtual int weekDayOfPray() const;

    /***************************
     **  Date/Time settings   **
     ***************************/

protected:

    /**
     * @internal initialises the Day Periods
     */
    virtual void initDayPeriods(const KConfigGroup &cg);

public:

    /**
     * @internal Sets the current date format.
     * The worker of the same-name KLocale API function.
     */
    virtual void setDateFormat(const QString &format);

    /**
     * @internal Returns the currently selected date format.
     * The worker of the same-name KLocale API function.
     */
    virtual QString dateFormat() const;

    /**
     * @internal Sets the current short date format.
     * The worker of the same-name KLocale API function.
     */
    virtual void setDateFormatShort(const QString &format);

    /**
     * @internal Returns the currently selected short date format.
     * The worker of the same-name KLocale API function.
     */
    virtual QString dateFormatShort() const;

    /**
     * @internal Changes the current time format.
     * The worker of the same-name KLocale API function.
     */
    virtual void setTimeFormat(const QString &format);

    /**
     * @internal Returns the currently selected time format.
     * The worker of the same-name KLocale API function.
     */
    virtual QString timeFormat() const;

    /**
     * @internal Set digit characters used to display dates and time.
     * The worker of the same-name KLocale API function.
     */
    virtual void setDateTimeDigitSet(KLocale::DigitSet digitSet);

    /**
     * @internal Returns the identifier of the digit set used to display dates and time.
     * The worker of the same-name KLocale API function.
     */
    virtual KLocale::DigitSet dateTimeDigitSet() const;

    /**
     * @internal Sets of the possessive form of month name should be used in dates.
     * The worker of the same-name KLocale API function.
     */
    virtual void setDateMonthNamePossessive(bool possessive);

    /**
     * @internal Returns if possessive form of month name should be used
     * The worker of the same-name KLocale API function.
     */
    virtual bool dateMonthNamePossessive() const;

    /**
     * @internal Returns if the user wants 12h clock
     * The worker of the same-name KLocale API function.
     */
    virtual bool use12Clock() const;

    /**
     * @internal
     * The worker of the same-name KLocale API function.
     */
    virtual void setDayPeriods(const QList<KDayPeriod> &dayPeriods);

    /**
     * @internal
     * The worker of the same-name KLocale API function.
     */
    virtual QList<KDayPeriod> dayPeriods() const;

    /**
     * @internal
     * The worker of the same-name KLocale API function.
     */
    virtual KDayPeriod dayPeriodForTime(const QTime &time) const;

    /**
     * @internal Returns a string formatted to the current locale's conventions
     * The worker of the same-name KLocale API function.
     */
    virtual QString formatDate(const QDate &date, KLocale::DateFormat format = KLocale::LongDate);

    /**
     * @internal Converts a localized date string to a QDate.
     * The worker of the same-name KLocale API function.
     */
    virtual QDate readDate(const QString &str, bool *ok = 0);

    /**
     * @internal Converts a localized date string to a QDate, using the specified format.
     * The worker of the same-name KLocale API function.
     */
    virtual QDate readDate(const QString &intstr, const QString &fmt, bool *ok = 0);

    /**
     * @internal Converts a localized date string to a QDate.
     * The worker of the same-name KLocale API function.
     */
    virtual QDate readDate(const QString &str, KLocale::ReadDateFlags flags, bool *ok = 0);

    /**
     * @deprecated replaced by formatLocaleTime()
     * @internal Returns a string formatted to the current locale's conventions regarding times.
     * The worker of the same-name KLocale API function.
     */
    virtual QString formatTime(const QTime &pTime, bool includeSecs = false, bool isDuration = false) const;

    /**
     * @internal Returns a string formatted to the current locale's conventions regarding times.
     * The worker of the same-name KLocale API function.
     */
    virtual QString formatLocaleTime(const QTime &pTime, KLocale::TimeFormatOptions options = KLocale::TimeDefault) const;

    /**
     * @internal Converts a localized time string to a QTime.
     * The worker of the same-name KLocale API function.
     */
    virtual QTime readTime(const QString &str, bool *ok = 0) const;

    /**
     * @deprecated replaced by readLocaleTime()
     * @internal Converts a localized time string to a QTime.
     * The worker of the same-name KLocale API function.
     */
    virtual QTime readTime(const QString &str, KLocale::ReadTimeFlags flags, bool *ok = 0) const;

    /**
     * @internal Converts a localized time string to a QTime.
     * The worker of the same-name KLocale API function.
     */
    virtual QTime readLocaleTime(const QString &str, bool *ok = 0,
                                 KLocale::TimeFormatOptions options = KLocale::TimeDefault,
                                 KLocale::TimeProcessingOptions processing = KLocale::ProcessNonStrict) const;

    /**
     * @internal Formats a date/time according to specified format.
     * The worker of the same-name KLocale API function.
     */
    static QString formatDateTime(const KLocale *locale, const QDateTime &dateTime, KLocale::DateFormat,
                                  bool includeSeconds, int daysToNow, int secsToNow);

    /**
     * @internal Return the date and time as a string
     * The worker of the same-name KLocale API function.
     */
    virtual QString formatDateTime(const QDateTime &dateTime, KLocale::DateFormat format = KLocale::ShortDate,
                                   bool includeSecs = false) const;

    /**
     * @internal Return the date and time as a string
     * The worker of the same-name KLocale API function.
     */
    virtual QString formatDateTime(const KDateTime &dateTime, KLocale::DateFormat format = KLocale::ShortDate,
                                   KLocale::DateTimeFormatOptions options = 0);

    /**
     * @internal Returns converted duration as a string
     * The worker of the same-name KLocale API function.
     */
    virtual QString formatDuration(unsigned long mSec) const;

    /**
     * @internal Returns converted duration as a string.
     * The worker of the same-name KLocale API function.
     */
    virtual QString prettyFormatDuration(unsigned long mSec) const;

    /***************************
     **  Digit Set settings   **
     ***************************/

protected:

    /**
     * @internal Converts a number string in any digit set into Arabic digits
     */
    static QString toArabicDigits(const QString &str);

    /**
     * @internal Returns the digits for a digit set as a string, e.g. "0123456789"
     */
    static QString digitSetString(KLocale::DigitSet digitSet);

public:

    /**
     * @internal Provides list of all known digit set identifiers.
     * The worker of the same-name KLocale API function.
     */
    virtual QList<KLocale::DigitSet> allDigitSetsList() const;

    /**
     * @internal Convert a digit set identifier to a human readable, localized name.
     * The worker of the same-name KLocale API function.
     */
    virtual QString digitSetToName(KLocale::DigitSet digitSet, bool withDigits = false) const;

    /**
     * @internal Convert all digits in the string to the given digit set.
     * The worker of the same-name KLocale API function.
     */
    virtual QString convertDigits(const QString &str, KLocale::DigitSet digitSet, bool ignoreContext = false) const;

    /***************************
     **    Number settings    **
     ***************************/

public:

    /**
     * @internal Sets the number of decimal places used when formating numbers.
     * The worker of the same-name KLocale API function.
     */
    virtual void setDecimalPlaces(int digits);

    /**
     * @internal Returns the number of numeric decimal places used by locale.
     * The worker of the same-name KLocale API function.
     */
    virtual int decimalPlaces() const;

    /**
     * @internal Sets the symbol used to identify the decimal pointer.
     * The worker of the same-name KLocale API function.
     */
    virtual void setDecimalSymbol(const QString &symbol);

    /**
     * @internal Returns the decimal symbol used by locale.
     * The worker of the same-name KLocale API function.
     */
    virtual QString decimalSymbol() const;

    /**
     * @internal Sets the separator used to group digits when formating numbers.
     * The worker of the same-name KLocale API function.
     * KDE5 Rename to setNumericDigitGroupSeparator()
     */
    virtual void setThousandsSeparator(const QString &separator);

    /**
     * @internal Returns the digit group separator used by locale.
     * The worker of the same-name KLocale API function.
     * KDE5 Rename to numericDigitGroupSeparator()
     */
    virtual QString thousandsSeparator() const;

    /**
     * @internal Sets the digit grouping to apply to numbers
     * For now internal only api designed for processing efficiency, if needed publicly then may
     * need to review if this is the best way.
     */
    virtual void setNumericDigitGrouping(QList<int> groupList);

    /**
     * @internal Returns the digit grouping to apply to numbers
     * For now internal only api designed for processing efficiency, if needed publicly then may
     * need to review if this is the best way.
     */
    virtual QList<int> numericDigitGrouping() const;

    /**
     * @internal Sets the sign used to identify a positive number.
     * The worker of the same-name KLocale API function.
     */
    virtual void setPositiveSign(const QString &sign);

    /**
     * @internal Returns the positive sign used by locale.
     * The worker of the same-name KLocale API function.
     */
    virtual QString positiveSign() const;

    /**
     * @internal Sets the sign used to identify a negative number.
     * The worker of the same-name KLocale API function.
     */
    virtual void setNegativeSign(const QString &sign);

    /**
     * @internal Returns the negative sign used by locale.
     * The worker of the same-name KLocale API function.
     */
    virtual QString negativeSign() const;

    /**
     * @internal Sets the set of digit characters used to display numbers.
     * The worker of the same-name KLocale API function.
     */
    virtual void setDigitSet(KLocale::DigitSet digitSet);

    /**
     * @internal Returns the identifier of the digit set used to display numbers.
     * The worker of the same-name KLocale API function.
     */
    virtual KLocale::DigitSet digitSet() const;

    /**
     * @internal Returns a number as a localized string
     * The worker of the same-name KLocale API function.
     */
    virtual QString formatNumber(double num, int precision = -1) const;

    /**
     * @internal Returns a number as a localized string
     * The worker of the same-name KLocale API function.
     */
    virtual QString formatNumber(const QString &numStr, bool round = true, int precision = -1) const;

    /**
     * @internal Returns a number as a localized string
     * The worker of the same-name KLocale API function.
     */
    virtual QString formatLong(long num) const;

    /**
     * @internal Converts a localized numeric string to a double.
     * The worker of the same-name KLocale API function.
     */
    virtual double readNumber(const QString &numStr, bool *ok = 0) const;

    /**************************
     **  Currency settings   **
     **************************/

protected:

    /**
     * @internal Initialises the Currency
     */
    virtual void initCurrency();

public:

    /**
     * @internal Sets the Locale Currency Code
     * The worker of the same-name KLocale API function.
     */
    virtual void setCurrencyCode(const QString &newCurrencyCode);

    /**
     * @internal Returns the Locale ISO Currency Code
     * The worker of the same-name KLocale API function.
     */
    virtual QString currencyCode() const;

    /**
     * @internal Returns the Locale Currency object
     * The worker of the same-name KLocale API function.
     */
    virtual KCurrencyCode *currency();

    /**
     * @internal Returns the ISO Code of the default currency.
     * The worker of the same-name KLocale API function.
     */
    static QString defaultCurrencyCode();

    /**
     * @internal Returns the ISO Currency Codes used in the locale
     * The worker of the same-name KLocale API function.
     */
    virtual QStringList currencyCodeList() const;

    /***************************
     **    Money settings     **
     ***************************/

public:

    /**
     * @internal Sets the current currency symbol.
     * The worker of the same-name KLocale API function.
     */
    virtual void setCurrencySymbol(const QString &symbol);

    /**
     * @internal Returns the default currency symbol used by locale.
     * The worker of the same-name KLocale API function.
     */
    virtual QString currencySymbol() const;

    /**
     * @internal Sets the symbol used to identify the decimal pointer for monetary values.
     * The worker of the same-name KLocale API function.
     */
    virtual void setMonetaryDecimalSymbol(const QString &symbol);

    /**
     * @internal Returns the monetary decimal symbol used by locale.
     * The worker of the same-name KLocale API function.
     */
    virtual QString monetaryDecimalSymbol() const;

    /**
     * @internal Sets the separator used to group digits when formating monetary values.
     * The worker of the same-name KLocale API function.
     * KDE5 Rename to setMonetaryDigitGroupSeparator()
     */
    virtual void setMonetaryThousandsSeparator(const QString &separator);

    /**
     * @internal Returns the monetary thousands separator used by locale.
     * The worker of the same-name KLocale API function.
     * KDE5 Rename to monetaryDigitGroupSeparator()
     */
    virtual QString monetaryThousandsSeparator() const;

    /**
     * @internal Sets the digit grouping to apply to numbers
     * For now internal only api designed for processing efficiency, if needed publicly then may
     * need to review if this is the best way.
     */
    virtual void setMonetaryDigitGrouping(QList<int> groupList);

    /**
     * @internal Returns the digit grouping to apply to numbers
     * For now internal only api designed for processing efficiency, if needed publicly then may
     * need to review if this is the best way.
     */
    virtual QList<int> monetaryDigitGrouping() const;

    /**
     * @internal Sets the number of decimal places used when formating money.
     * The worker of the same-name KLocale API function.
     */
    virtual void setMonetaryDecimalPlaces(int digits);

    /**
     * @internal Returns the number of monetary decimal places used by locale.
     * The worker of the same-name KLocale API function.
     */
    virtual int monetaryDecimalPlaces() const;

    /**
     * @internal Sets the position where the currency symbol should be printed for
     * positive monetary values.
     * The worker of the same-name KLocale API function.
     */
    virtual void setPositivePrefixCurrencySymbol(bool prefix);

    /**
     * @internal Returns where to print the currency symbol for positive numbers.
     * The worker of the same-name KLocale API function.
     */
    virtual bool positivePrefixCurrencySymbol() const;

    /**
     * @internal Sets the position where the currency symbol should be printed for
     * negative monetary values.
     * The worker of the same-name KLocale API function.
     */
    virtual void setNegativePrefixCurrencySymbol(bool prefix);

    /**
     * @internal Returns if the currency symbol precedes negative numbers.
     * The worker of the same-name KLocale API function.
     */
    virtual bool negativePrefixCurrencySymbol() const;

    /**
     * @internal Sets the sign position used for positive monetary values.
     * The worker of the same-name KLocale API function.
     */
    virtual void setPositiveMonetarySignPosition(KLocale::SignPosition signpos);

    /**
     * @internal Returns where/how to print the positive sign.
     * The worker of the same-name KLocale API function.
     */
    virtual KLocale::SignPosition positiveMonetarySignPosition() const;

    /**
     * @internal Sets the sign position used for negative monetary values.
     * The worker of the same-name KLocale API function.
     */
    virtual void setNegativeMonetarySignPosition(KLocale::SignPosition signpos);

    /**
     * @internal Returns where/how to print the negative sign.
     * The worker of the same-name KLocale API function.
     */
    virtual KLocale::SignPosition negativeMonetarySignPosition() const;

    /**
     * @internal Set digit characters used to display monetary values.
     * The worker of the same-name KLocale API function.
     */
    virtual void setMonetaryDigitSet(KLocale::DigitSet digitSet);

    /**
     * @internal Retuns the digit set used to display monetary values.
     * The worker of the same-name KLocale API function.
     */
    virtual KLocale::DigitSet monetaryDigitSet() const;

    /**
     * @internal Returns an amount of money as a localized string
     * The worker of the same-name KLocale API function.
     */
    virtual QString formatMoney(double num, const QString &currency = QString(), int precision = -1) const;

    /**
     * @internal Converts a localized monetary string to a double.
     * The worker of the same-name KLocale API function.
     */
    virtual double readMoney(const QString &numStr, bool *ok = 0) const;

    /***************************
     **    Units settings     **
     ***************************/

protected:

    /**
     * @internal
     * @return list of translated binary unit for @p dialect.
     */
    QList<QString> dialectUnitsList(KLocale::BinaryUnitDialect dialect);

    enum DurationType {
        DaysDurationType = 0,
        HoursDurationType,
        MinutesDurationType,
        SecondsDurationType
    };

    /**
     * @internal Formats a duration according to the given type and number
     */
    static QString formatSingleDuration(KLocalePrivate::DurationType durationType, int n);

public:

    /**
     * @internal Returns the user's default binary unit dialect.
     * The worker of the same-name KLocale API function.
     */
   virtual KLocale::BinaryUnitDialect binaryUnitDialect() const;

    /**
     * @internal Sets the default dialect for this locale
     * The worker of the same-name KLocale API function.
     */
    virtual void setBinaryUnitDialect(KLocale::BinaryUnitDialect newDialect);

    /**
     * @internal Returns converted size as a string
     * The worker of the same-name KLocale API function.
     */
    virtual QString formatByteSize(double size);

    /**
     * @internal Returns converted size as a translated string including the units.
     * The worker of the same-name KLocale API function.
     */
    virtual QString formatByteSize(double size, int precision,
                                   KLocale::BinaryUnitDialect dialect = KLocale::DefaultBinaryDialect,
                                   KLocale:: BinarySizeUnits specificUnit = KLocale::DefaultBinaryUnits);

    /**
     * @internal Sets the preferred page size when printing.
     * The worker of the same-name KLocale API function.
     */
    virtual void setPageSize(int paperFormat);

    /**
     * @internal Returns the preferred page size for printing.
     * The worker of the same-name KLocale API function.
     */
    virtual int pageSize() const;

    /**
     * @internal Sets the preferred measuring system.
     * The worker of the same-name KLocale API function.
     */
    virtual void setMeasureSystem(KLocale::MeasureSystem value);

    /**
     * @internal Returns which measuring system we use.
     * The worker of the same-name KLocale API function.
     */
    virtual KLocale::MeasureSystem measureSystem() const;

    /***************************
     **   Encoding settings   **
     ***************************/

protected:

    /**
     * @internal Figures out which encoding the user prefers.
     */
    virtual void initEncoding();

    /**
     * @internal Returns the system codeset.
     */
    virtual QByteArray systemCodeset() const;

    /**
     * @internal A QFile filename encoding function (QFile::encodeFn).
     */
    static QByteArray encodeFileNameUTF8(const QString &fileName);

    /**
     * @internal QFile filename decoding function (QFile::decodeFn).
     */
    static QString decodeFileNameUTF8(const QByteArray &localFileName);

public:

    /**
     * @internal Sets the current encoding
     * The worker of the same-name KLocale API function.
     */
    virtual bool setEncoding(int mibEnum);

    /**
     * @internal Returns the user's preferred encoding.
     * The worker of the same-name KLocale API function.
     */
    virtual const QByteArray encoding();

    /**
     * @internal Returns the user's preferred encoding.
     * The worker of the same-name KLocale API function.
     */
    virtual int encodingMib() const;

    /**
     * @internal Returns the file encoding.
     * The worker of the same-name KLocale API function.
     */
    virtual int fileEncodingMib() const;

    /**
     * @internal Returns the user's preferred encoding.
     * The worker of the same-name KLocale API function.
     */
    virtual QTextCodec *codecForEncoding() const;

    /***************************
     **       Utilities       **
     ***************************/

public:

    /**
     * @internal Parses locale string into distinct parts.
     * The worker of the same-name KLocale API function.
     */
    static void splitLocale(const QString &locale, QString &language, QString &country,
                            QString &modifier, QString &charset);

private:

    /**
     * @internal COnvert digit group format string to digit group list
     */
    QList<int> digitGroupFormatToList(const QString &digitGroupFormat) const;

    /**
     * @internal Insert digit group separator
     */
    QString formatDigitGroup(const QString &number, const QString &groupSeparator, const QString &decimalSeperator, QList<int> groupList) const;

    /**
     * @internal Remove digit group separator, return ok if valid format
     */
    QString parseDigitGroup(const QString &number, const QString &groupSeparator, const QString &decimalSeperator, QList<int> groupList, bool *ok) const;

public:
    // Parent KLocale, public needed for copy ctor
    KLocale *q;

private:
    // Config file containing locale config
    KSharedConfig::Ptr m_config;

    // Country settings
    QString m_country;
    QString m_countryDivisionCode;

    // Language settings
    QString      m_language;
    KConfig     *m_languages;
    QStringList  m_languageList;
    bool         m_languageSensitiveDigits;  // FIXME: Temporary until full language-sensitivity implemented.
    bool         m_nounDeclension;

    // Calendar settings
    KLocale::CalendarSystem m_calendarSystem;
    KCalendarSystem *m_calendar;
    KLocale::WeekNumberSystem m_weekNumberSystem;
    int              m_weekStartDay;
    int              m_workingWeekStartDay;
    int              m_workingWeekEndDay;
    int              m_weekDayOfPray;

    // Date/Time settings
    QString           m_dateFormat;
    QString           m_dateFormatShort;
    QString           m_timeFormat;
    KLocale::DigitSet m_dateTimeDigitSet;
    bool              m_dateMonthNamePossessive;
    mutable QList<KDayPeriod> m_dayPeriods;

    // Number settings
    int               m_decimalPlaces;
    QString           m_decimalSymbol;
    QString           m_thousandsSeparator;
    QList<int>        m_numericDigitGrouping;
    QString           m_positiveSign;
    QString           m_negativeSign;
    KLocale::DigitSet m_digitSet;

    // Currency settings
    QString        m_currencyCode;
    KCurrencyCode *m_currency;
    QStringList    m_currencyCodeList;

    // Money settings
    QString               m_currencySymbol;
    QString               m_monetaryDecimalSymbol;
    QString               m_monetaryThousandsSeparator;
    QList<int>            m_monetaryDigitGrouping;
    int                   m_monetaryDecimalPlaces;
    KLocale::SignPosition m_positiveMonetarySignPosition;
    KLocale::SignPosition m_negativeMonetarySignPosition;
    bool                  m_positivePrefixCurrencySymbol;
    bool                  m_negativePrefixCurrencySymbol;
    KLocale::DigitSet     m_monetaryDigitSet;

    // Units settings
    KLocale::BinaryUnitDialect m_binaryUnitDialect;
    QList<QString>             m_byteSizeFmt;
    int                        m_pageSize;
    KLocale::MeasureSystem     m_measureSystem;

    // Encoding settings
    QString     m_encoding;
    QTextCodec *m_codecForEncoding;
    bool        m_utf8FileEncoding;
};

#include <QtCore/QTranslator>
#include <QDebug>

class KDETranslator : public QTranslator
{
public:
    KDETranslator(QObject *parent) : QTranslator(parent)
    {
        setObjectName(QLatin1String("kdetranslator"));
    }

    virtual QString translate(const char* context,
                              const char *sourceText,
                              const char* comment
                              , int n
                              ) const
    {
        return KLocalizedString::translateQt(context, sourceText, comment
                                             , n
            );
    }

    virtual bool isEmpty() const
    {
        return false;
    }
};

#endif /* KLOCALE_P_H */
