/* This file is part of the KDE libraries
    Copyright (C) 1997 Stephan Kulow <coolo@kde.org>
    Copyright (C) 1999-2003 Hans Petter Bieker <bieker@kde.org>
    Copyright (c) 2002 Lukas Tinkl <lukas@kde.org>

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
#ifndef KLOCALE_H
#define KLOCALE_H

#include <kdecore_export.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

#include <QtCore/QString>
#include <QtCore/QList>

class QStringList;
class QTextCodec;
class QDate;
class QTime;
class QDateTime;

class KDateTime;
class KCalendarSystem;
class KCurrencyCode;

class KLocalePrivate;

/**
 * \file klocale.h
 */

/**
  *
  * KLocale provides support for country specific stuff like
  * the national language.
  *
  * KLocale supports translating, as well as specifying the format
  * for numbers, currency, time, and date.
  *
  * Use KGlobal::locale() to get pointer to the global KLocale object,
  * containing the applications current locale settings.
  *
  * For example, to format the date May 17, 1995 in the current locale, use:
  *
  * \code
  *   QString date = KGlobal::locale()->formatDate(QDate(1995,5,17));
  * \endcode
  *
  * @author Stephan Kulow <coolo@kde.org>, Preston Brown <pbrown@kde.org>,
  * Hans Petter Bieker <bieker@kde.org>, Lukas Tinkl <lukas.tinkl@suse.cz>
  * @short class for supporting locale settings and national language
  */
class KDECORE_EXPORT KLocale
{
public:
    /**
     * Constructs a KLocale with the given catalog name
     *
     * The constructor looks for an entry Language in the group Locale in the
     * configuration file.
     *
     * If no configuration file is specified, it will also look for languages
     * using the environment variables (KDE_LANG, LC_MESSAGES, LC_ALL, LANG),
     * as well as the global configuration file. If KLocale is not able to use
     * any of the specified languages, the default language (en_US) will be
     * used.
     *
     * If you specify a configuration file, it has to be valid until
     * the KLocale object is destroyed.
     *
     * @param catalog the name of the main language file
     * @param config  a configuration file with a Locale group detailing
     *                locale-related preferences (such as language and
     *                formatting options)
     */
    explicit KLocale(const QString& catalog, KSharedConfig::Ptr config = KSharedConfig::Ptr());

    /**
     * Constructs a KLocale with the given catalog name
     *
     * Allows you to override the language and, optionally, the
     * country of this locale.
     *
     * @param catalog  the name of the main language file
     * @param language the language for the locale
     * @param country  the country for the locale
     * @param config   a configuration file with a Locale group detailing
     *                 locale-related preferences (such as date and time
     *                 formatting options)
     */
    KLocale(const QString& catalog, const QString &language, const QString &country = QString(),
            KConfig *config = 0);

    /**
     * Copy constructor
     */
    KLocale(const KLocale & rhs);

    /**
     * Assignment operator
     */
    KLocale& operator= (const KLocale & rhs);

    /**
     * Destructor
     */
    virtual ~KLocale();

    /**
     * Raw translation from message catalogs.
     *
     * Never use this directly to get message translations. See the i18n and ki18n
     * family of calls related to KLocalizedString.
     *
     * @param msg the message. Must not be null. Must be UTF-8 encoded.
     * @param lang language in which the translation was found. If no translation
     *             was found, KLocale::defaultLanguage() is reported. If null,
     *             the language is not reported.
     * @param trans raw translation, or original if not found. If no translation
     *              was found, original message is reported. If null, the
     *              translation is not reported.
     *
     * @see KLocalizedString
     */
    void translateRaw(const char* msg, QString *lang, QString *trans) const;

    /**
     * Raw translation from message catalogs, with given context.
     * Context + message are used as the lookup key in catalogs.
     *
     * Never use this directly to get message translations. See i18n* and ki18n*
     * calls related to KLocalizedString.
     *
     * @param ctxt the context. Must not be null. Must be UTF-8 encoded.
     * @param msg the message. Must not be null. Must be UTF-8 encoded.
     * @param lang language in which the translation was found. If no translation
     *             was found, KLocale::defaultLanguage() is reported. If null,
     *             the language is not reported.
     * @param trans raw translation, or original if not found. If no translation
     *              was found, original message is reported. If null, the
     *              translation is not reported.
     *
     * @see KLocalizedString
     */
    void translateRaw(const char *ctxt, const char *msg, QString *lang, QString *trans) const;

    /**
     * Raw translation from message catalogs, with given singular/plural form.
     * Singular form is used as the lookup key in catalogs.
     *
     * Never use this directly to get message translations. See i18n* and ki18n*
     * calls related to KLocalizedString.
     *
     * @param singular the singular form. Must not be null. Must be UTF-8 encoded.
     * @param plural the plural form. Must not be null. Must be UTF-8 encoded.
     * @param n number on which the forms are decided.
     * @param lang language in which the translation was found. If no translation
     *             was found, KLocale::defaultLanguage() is reported. If null,
     *             the language is not reported.
     * @param trans raw translation, or original if not found. If no translation
     *              was found, original message is reported (either plural or
     *              singular, as determined by @p n ). If null, the
     *              translation is not reported.
     *
     * @see KLocalizedString
     */
    void translateRaw(const char *singular, const char *plural,  unsigned long n, QString *lang,
                      QString *trans) const;

    /**
     * Raw translation from message catalogs, with given context and
     * singular/plural form.
     * Context + singular form is used as the lookup key in catalogs.
     *
     * Never use this directly to get message translations. See i18n* and ki18n*
     * calls related to KLocalizedString.
     *
     * @param ctxt the context. Must not be null. Must be UTF-8 encoded.
     * @param singular the singular form. Must not be null. Must be UTF-8 encoded.
     * @param plural the plural form. Must not be null. Must be UTF-8 encoded.
     * @param n number on which the forms are decided.
     * @param lang language in which the translation was found. If no translation
     *             was found, KLocale::defaultLanguage() is reported. If null,
     *             the language is not reported.
     * @param trans raw translation, or original if not found. If no translation
     *              was found, original message is reported (either plural or
     *              singular, as determined by @p n ). If null, the
     *              translation is not reported.
     *
     * @see KLocalizedString
     */
    void translateRaw(const char *ctxt, const char *singular, const char *plural, unsigned long n,
                      QString *lang, QString *trans) const;

    /**
     * Changes the current encoding.
     *
     * @param mibEnum The mib of the preferred codec
     *
     * @return True on success.
     */
    bool setEncoding(int mibEnum);

    /**
     * Various positions for where to place the positive or negative
     * sign when they are related to a monetary value.
     */
    enum SignPosition {
        /**
         * Put parantheses around the quantity, e.g. "$ (217)"
         */
        ParensAround = 0,
        /**
         * Prefix the quantity with the sign, e.g. "$ -217"
         */
        BeforeQuantityMoney = 1,
        /**
         * Suffix the quanitity with the sign, e.g. "$ 217-"
         */
        AfterQuantityMoney = 2,
        /**
         * Prefix the currency symbol with the sign, e.g. "-$ 217"
         */
        BeforeMoney = 3,
        /**
         * Suffix the currency symbol with the sign, e.g. "$- 217"
         */
        AfterMoney = 4
    };

    /**
     * @since 4.3
     *
     * The set of digit characters used to display and enter numbers.
     */
    enum DigitSet {
        ArabicDigits,             /**< 0123456789 (European and some Asian
                                       languages and western Arabic dialects) */
        ArabicIndicDigits,        /**< ٠١٢٣٤٥٦٧٨٩ (eastern Arabic dialects) */
        EasternArabicIndicDigits, /**< ۰۱۲۳۴۵۶۷۸۹ (Persian and Urdu) */
        DevenagariDigits          /**< ०१२३४५६७८९ (Hindi) */
    };

    /**
     * @since 4.3
     *
     * Convert a digit set identifier to a human readable, localized name.
     *
     * @param digitSet the digit set identifier
     * @param withDigits whether to add the digits themselves to the name
     *
     * @return the human readable and localized name of the digit set
     *
     * @see DigitSet
     */
    QString digitSetToName(DigitSet digitSet, bool withDigits = false) const;

    /**
     * @since 4.3
     *
     * Provides list of all known digit set identifiers.
     *
     * @return list of all digit set identifiers
     * @see DigitSet
     * @see digitSetToName
     */
    QList<DigitSet> allDigitSetsList() const;

    /**
     * Returns what a decimal point should look like ("." or "," etc.)
     * according to the current locale or user settings.
     *
     * @return The decimal symbol used by locale.
     */
    QString decimalSymbol() const;

    /**
     * Returns what the thousands separator should look
     * like ("," or "." etc.)
     * according to the current locale or user settings.
     *
     * @return The thousands separator used by locale.
     */
    QString thousandsSeparator() const;

    /**
     * @since 4.3
     *
     * Returns the identifier of the digit set used to display numbers.
     *
     * @return the digit set identifier
     * @see DigitSet
     * @see digitSetToName
     */
    DigitSet digitSet() const;

    /**
     * @since 4.4
     *
     * Returns the ISO 4217 Currency Code for the current locale
     *
     * @return The default ISO Currency Code used by locale.
    */
    QString currencyCode() const;

    /**
     * @since 4.4
     *
     * Returns the Currency Code object for the current locale
     *
     * @return The default Currency Code object used by locale.
    */
    KCurrencyCode *currency() const;

    /**
     * Returns what the symbol denoting currency in the current locale
     * as as defined by user settings should look like.
     *
     * @return The default currency symbol used by locale.
     */
    QString currencySymbol() const;

    /**
     * Returns what a decimal point should look like ("." or "," etc.)
     * for monetary values, according to the current locale or user
     * settings.
     *
     * @return The monetary decimal symbol used by locale.
     */
    QString monetaryDecimalSymbol() const;

    /**
     * Returns what a thousands separator for monetary values should
     * look like ("," or " " etc.) according to the current locale or
     * user settings.
     *
     * @return The monetary thousands separator used by locale.
     */
    QString monetaryThousandsSeparator() const;

    /**
     * Returns what a positive sign should look like ("+", " ", etc.)
     * according to the current locale or user settings.
     *
     * @return The positive sign used by locale.
     */
    QString positiveSign() const;

    /**
     * Returns what a negative sign should look like ("-", etc.)
     * according to the current locale or user settings.
     *
     * @return The negative sign used by locale.
     */
    QString negativeSign() const;

    /**
     * @deprecated use decimalPlaces() or monetaryDecimalPlaces()
     *
     * The number of fractional digits to include in monetary values (usually 2).
     *
     * @return Default number of fractional digits used by locale.
     */
    int fracDigits() const;

    /**
     * @since 4.4
     *
     * The number of decimal places to include in numeric values (usually 2).
     *
     * @return Default number of numeric decimal places used by locale.
     */
    int decimalPlaces() const;

    /**
     * @since 4.4
     *
     * The number of decimal places to include in monetary values (usually 2).
     *
     * @return Default number of monetary decimal places used by locale.
     */
    int monetaryDecimalPlaces() const;

    /**
     * If and only if the currency symbol precedes a positive value,
     * this will be true.
     *
     * @return Where to print the currency symbol for positive numbers.
     */
    bool positivePrefixCurrencySymbol() const;

    /**
     * If and only if the currency symbol precedes a negative value,
     * this will be true.
     *
     * @return True if the currency symbol precedes negative numbers.
     */
    bool negativePrefixCurrencySymbol() const;

    /**
     * Returns the position of a positive sign in relation to a
     * monetary value.
     *
     * @return Where/how to print the positive sign.
     * @see SignPosition
     */
    SignPosition positiveMonetarySignPosition() const;

    /**
     * Denotes where to place a negative sign in relation to a
     * monetary value.
     *
     * @return Where/how to print the negative sign.
     * @see SignPosition
     */
    SignPosition negativeMonetarySignPosition() const;

    /**
     * @since 4.3
     *
     * Retuns the digit set used to display monetary values.
     *
     * @return the digit set identifier
     * @see DigitSet
     * @see digitSetToName
     */
    DigitSet monetaryDigitSet() const;

    /**
     * Given a double, converts that to a numeric string containing
     * the localized monetary equivalent.
     *
     * e.g. given 123456, return "$ 123,456.00".
     *
     * If precision isn't specified or is < 0, then the default monetaryDecimalPlaces() is used.
     *
     * @param num The number we want to format
     * @param currency The currency symbol you want.
     * @param precision Number of decimal places displayed
     *
     * @return The number of money as a localized string
     * @see monetaryDecimalPlaces()
     */
    QString formatMoney(double num, const QString &currency = QString(), int precision = -1) const;

    /**
     * Given a double, converts that to a numeric string containing
     * the localized numeric equivalent.
     *
     * e.g. given 123456.78F, return "123,456.78" (for some European country).
     *
     * If precision isn't specified or is < 0, then the default decimalPlaces() is used.
     *
     * This function is a wrapper that is provided for convenience.
     *
     * @param num The number to convert
     * @param precision Number of decimal places used.
     *
     * @return The number as a localized string
     * @see formatNumber(const QString, bool, int)
     * @see decimalPlaces()
     */
    QString formatNumber(double num, int precision = -1) const;

    /**
     * Given a string representing a number, converts that to a numeric
     * string containing the localized numeric equivalent.
     *
     * e.g. given 123456.78F, return "123,456.78" (for some European country).
     *
     * If precision isn't specified or is < 0, then the default decimalPlaces() is used.
     *
     * @param numStr The number to format, as a string.
     * @param round Round fractional digits. (default true)
     * @param precision Number of fractional digits used for rounding. Unused if round=false.
     *
     * @return The number as a localized string
     */
    QString formatNumber(const QString &numStr, bool round = true, int precision = -1) const;

    /**
     * Given an integer, converts that to a numeric string containing
     * the localized numeric equivalent.
     *
     * e.g. given 123456L, return "123,456" (for some European country).
     *
     * @param num The number to convert
     *
     * @return The number as a localized string
     */
    QString formatLong(long num) const;

    /**
     * These binary units are used in KDE by the formatByteSize()
     * functions.
     *
     * NOTE: There are several different units standards:
     * 1) SI  (i.e. metric), powers-of-10.
     * 2) IEC, powers-of-2, with specific units KiB, MiB, etc.
     * 3) JEDEC, powers-of-2, used for solid state memory sizing which
     *    is why you see flash cards labels as e.g. 4GB.  These (ab)use
     *    the metric units.  Although JEDEC only defines KB, MB, GB, if
     *    JEDEC is selected all units will be powers-of-2 with metric
     *    prefixes for clarity in the event of sizes larger than 1024 GB.
     *
     * Although 3 different dialects are possible this enum only uses
     * metric names since adding all 3 different names of essentially the same
     * unit would be pointless.  Use BinaryUnitDialect to control the exact
     * units returned.
     *
     * @since 4.4
     * @see binaryUnitDialect
     */
    enum BinarySizeUnits {
        /// Auto-choose a unit such that the result is in the range [0, 1000 or 1024)
        DefaultBinaryUnits = -1,

        // The first real unit must be 0 for the current implementation!
        UnitByte,      ///<  B         1 byte
        UnitKiloByte,  ///<  KiB/KB/kB 1024/1000 bytes.
        UnitMegaByte,  ///<  MiB/MB/MB 2^20/10^06 bytes.
        UnitGigaByte,  ///<  GiB/GB/GB 2^30/10^09 bytes.
        UnitTeraByte,  ///<  TiB/TB/TB 2^40/10^12 bytes.
        UnitPetaByte,  ///<  PiB/PB/PB 2^50/10^15 bytes.
        UnitExaByte,   ///<  EiB/EB/EB 2^60/10^18 bytes.
        UnitZettaByte, ///<  ZiB/ZB/ZB 2^70/10^21 bytes.
        UnitYottaByte, ///<  YiB/YB/YB 2^80/10^24 bytes.
        UnitLastUnit = UnitYottaByte
    };

    /**
     * This enum chooses what dialect is used for binary units.
     *
     * Note: Although JEDEC abuses the metric prefixes and can therefore be
     * confusing, it has been used to describe *memory* sizes for quite some time
     * and programs should therefore use either Default, JEDEC, or IEC 60027-2
     * for memory sizes.
     *
     * On the other hand network transmission rates are typically in metric so
     * Default, Metric, or IEC (which is unambiguous) should be chosen.
     *
     * Normally choosing DefaultBinaryUnits is the best option as that uses
     * the user's selection for units.
     *
     * @since 4.4
     * @see binaryUnitDialect
     * @see setBinaryUnitDialect
     */
    enum BinaryUnitDialect {
        DefaultBinaryDialect = -1, ///< Used if no specific preference
        IECBinaryDialect,          ///< KDE Default, KiB, MiB, etc. 2^(10*n)
        JEDECBinaryDialect,        ///< KDE 3.5 default, KB, MB, etc. 2^(10*n)
        MetricBinaryDialect,       ///< SI Units, kB, MB, etc. 10^(3*n)
        LastBinaryDialect = MetricBinaryDialect
    };

    /**
     * Converts @p size from bytes to the string representation using the
     * user's default binary unit dialect.  The default unit dialect is
     * IEC 60027-2.
     *
     * Example:
     * formatByteSize(1024) returns "1.0 KiB" by default.
     *
     * @param  size  size in bytes
     * @return converted size as a string - e.g. 123.4 KiB , 12.0 MiB
     * @see BinaryUnitDialect
     * @todo KDE 5: Remove in favor of overload added in KDE 4.4.
     */
    QString formatByteSize(double size) const;

    /**
     * @since 4.4
     *
     * Converts @p size from bytes to the appropriate string representation
     * using the binary unit dialect @p dialect and the specific units @p specificUnit.
     *
     * Example:
     * formatByteSize(1000, unit, KLocale::BinaryUnitKilo) returns:
     *   for KLocale::MetricBinaryUnits, "1.0 kB",
     *   for KLocale::IECBinaryUnits,    "0.9 KiB",
     *   for KLocale::JEDECBinaryUnits,  "0.9 KB".
     *
     * @param size size in bytes
     * @param precision number of places after the decimal point to use.  KDE uses
     *        1 by default so when in doubt use 1.
     * @param dialect binary unit standard to use.  Use DefaultBinaryUnits to
     *        use the localized user selection unless you need to use a specific
     *        unit type (such as displaying a flash memory size in JEDEC).
     * @param specificUnit specific unit size to use in result.  Use
     *        DefaultBinarySize to automatically select a unit that will return
     *        a sanely-sized number.
     * @return converted size as a translated string including the units.
     *         E.g. "1.23 KiB", "2 GB" (JEDEC), "4.2 kB" (Metric).
     */
    QString formatByteSize(double size, int precision,
                           BinaryUnitDialect dialect = KLocale::DefaultBinaryDialect,
                           BinarySizeUnits specificUnit = KLocale::DefaultBinaryUnits) const;

    /**
     * Returns the user's default binary unit dialect.
     *
     * @since 4.4
     * @return User's default binary unit dialect
     * @see BinaryUnitDialect
     */
    BinaryUnitDialect binaryUnitDialect() const;

    /**
     * Sets @p newDialect to be the default dialect for this locale (and only
     * this locale).  Newly created KLocale objects will continue to default
     * to the user's choice.
     *
     * @param newDialect the new dialect to set as default for this locale object.
     * @since 4.4
     */
    void setBinaryUnitDialect(BinaryUnitDialect newDialect);

    /**
     * Given a number of milliseconds, converts that to a string containing
     * the localized equivalent
     *
     * e.g. given formatDuration(60000), returns "1.0 minutes"
     *
     * @param mSec Time duration in milliseconds
     * @return converted duration as a string - e.g. "5.5 seconds" "23.0 minutes"
     */
    QString formatDuration(unsigned long mSec) const;

    /**
     * Given a number of milliseconds, converts that to a pretty string containing
     * the localized equivalent.
     *
     * e.g. given prettyFormatDuration(60001) returns "1 minute"
     *      given prettyFormatDuration(62005) returns "1 minute and 2 seconds"
     *      given prettyFormatDuration(90060000) returns "1 day and 1 hour"
     *
     * @param mSec Time duration in milliseconds
     * @return converted duration as a string.
     *         Units not interesting to the user, for example seconds or minutes when the first
     *         unit is day, are not returned because they are irrelevant. The same applies for
     *         seconds when the first unit is hour.
     * @since 4.2
     */
    QString prettyFormatDuration(unsigned long mSec) const;

    /**
     * @deprecated
     *
     * Use this to determine whether nouns are declined in
     * locale's language. This property should remain
     * read-only (no setter function)
     *
     * @return If nouns are declined
     */
    bool nounDeclension() const;

    /**
     * @since 4.4
     *
     * Standard used for Date Time Format String
     */
    enum DateTimeFormatStandard {
         KdeFormat                   /**< KDE Standard */
         //PosixFormat,              /**< POSIX Standard */  KDE 4.5
         //GnuFormat                 /**< GNU Standard */    KDE 4.5
         //UnicodeFormat             /**< UNICODE Standard (Java/OSX/Windows?) */  KDE 4.5
    };

    /**
     * Format for date string.
     */
    enum DateFormat {
        ShortDate,        /**< Locale Short date format, e.g. 08-04-2007 */
        LongDate,         /**< Locale Long date format, e.g. Sunday 08 April 2007 */
        FancyShortDate,   /**< Same as ShortDate for dates a week or more ago. For more
                               recent dates, it is represented as Today, Yesterday, or
                               the weekday name. */
        FancyLongDate,    /**< Same as LongDate for dates a week or more ago. For more
                               recent dates, it is represented as Today, Yesterday, or
                               the weekday name. */
        IsoDate,          /**< ISO-8601 Date format YYYY-MM-DD, e.g. 2009-12-31 */
        IsoWeekDate,      /**< ISO-8601 Week Date format YYYY-Www-D, e.g. 2009-W01-1 */
        IsoOrdinalDate    /**< ISO-8601 Ordinal Date format YYYY-DDD, e.g. 2009-001 */
    };

    /**
     * Returns a string formatted to the current locale's conventions
     * regarding dates.
     *
     * @param date the date to be formatted
     * @param format category of date format to use
     *
     * @return the date as a string
     */
    QString formatDate(const QDate &date, DateFormat format = LongDate) const;

    /**
     * Returns a string formatted to the current locale's conventions
     * regarding both date and time.
     *
     * @param dateTime the date and time to be formatted
     * @param format category of date format to use
     * @param includeSecs if @c true, the string will include the seconds part
     *                    of the time; otherwise, the seconds will be omitted
     *
     * @return the date and time as a string
     */
    QString formatDateTime(const QDateTime &dateTime, DateFormat format = ShortDate,
                           bool includeSecs = false) const;

    /**
     * Options for formatting date-time values.
     */
    enum DateTimeFormatOption {
        TimeZone = 0x01,    /**< Include a time zone string */
        Seconds  = 0x02     /**< Include the seconds value */
    };
    Q_DECLARE_FLAGS(DateTimeFormatOptions, DateTimeFormatOption)

    /**
     * Returns a string formatted to the current locale's conventions
     * regarding both date and time.
     *
     * @param dateTime the date and time to be formatted
     * @param format category of date format to use
     * @param options additional output options
     *
     * @return The date and time as a string
     */
    QString formatDateTime(const KDateTime &dateTime, DateFormat format = ShortDate,
                           DateTimeFormatOptions options = 0) const;

    /**
     * Use this to determine whether in dates a possessive form of month
     * name is preferred ("of January" rather than "January")
     *
     * @return If possessive form should be used
    */
    bool dateMonthNamePossessive() const;

    /**
     * @deprecated replaced by formatLocaleTime()
     *
     * Returns a string formatted to the current locale's conventions
     * regarding times.
     *
     * @param pTime The time to be formatted.
     * @param includeSecs if true, seconds are included in the output,
     *        otherwise only hours and minutes are formatted.
     * @param isDuration if true, the given time is a duration, not a clock time.
     * This means "am/pm" shouldn't be displayed.
     *
     * @return The time as a string
     */
    QString formatTime(const QTime &pTime, bool includeSecs = false, bool isDuration = false) const;

    /**
     * @since 4.4
     *
     * Format flags for readLocaleTime() and formatLocaleTime()
     */
    enum TimeFormatOption {
        TimeDefault        = 0x0,   ///< Default formatting using seconds and the format
                                    ///< as specified by the locale.
        TimeWithoutSeconds = 0x1,   ///< Exclude the seconds part of the time from display
        TimeWithoutAmPm    = 0x2,   ///< Read/format time string without am/pm suffix but
                                    ///< keep the 12/24h format as specified by locale time
                                    ///< format, eg. "07.33.05" instead of "07.33.05 pm" for
                                    ///< time format "%I.%M.%S %p".
        TimeDuration       = 0x6    ///< Read/format time string as duration. This will strip
                                    ///< the am/pm suffix and read/format times with an hour
                                    ///< value of 0-23 hours, eg. "19.33.05" instead of
                                    ///< "07.33.05 pm" for time format "%I.%M.%S %p".
                                    ///< This automatically implies @c TimeWithoutAmPm.
    };
    Q_DECLARE_FLAGS(TimeFormatOptions, TimeFormatOption)

    /**
     * @since 4.4
     *
     * Returns a string formatted to the current locale's conventions
     * regarding times.
     *
     * @param pTime the time to be formatted
     * @param options format option to use when formatting the time
     * @return The time as a string
     */
    QString formatLocaleTime(const QTime &pTime,
                             TimeFormatOptions options = KLocale::TimeDefault) const;

    /**
     * @since 4.3
     *
     * Returns the identifier of the digit set used to display dates and time.
     *
     * @return the digit set identifier
     * @see DigitSet
     * @see digitSetToName
     */
    DigitSet dateTimeDigitSet() const;

    /**
     * Use this to determine if the user wants a 12 hour clock.
     *
     * @return If the user wants 12h clock
     */
    bool use12Clock() const;

    /**
     * Use this to determine which day is the first day of the week.
     *
     * @return an integer (Monday=1..Sunday=7)
     */
    int weekStartDay() const;

    /**
     * Use this to determine which day is the first working day of the week.
     *
     * @since 4.2
     * @return an integer (Monday=1..Sunday=7)
     */
    int workingWeekStartDay() const;

    /**
     * Use this to determine which day is the last working day of the week.
     *
     * @since 4.2
     * @return an integer (Monday=1..Sunday=7)
     */
    int workingWeekEndDay() const;

    /**
     * Use this to determine which day is reserved for religious observance
     *
     * @since 4.2
     * @return day number (None = 0, Monday = 1, ..., Sunday = 7)
     */
    int weekDayOfPray() const;

    /**
     * Returns a pointer to the calendar system object.
     *
     * @return the current calendar system instance
     */
    const KCalendarSystem * calendar() const;

    /**
     * Returns the name of the calendar system that is currently being
     * used by the system.
     *
     * @return the name of the calendar system
     */
    QString calendarType() const;

    /**
     * Changes the current calendar system to the calendar specified.
     * Currently "gregorian" and "hijri" are supported. If the calendar
     * system specified is not found, gregorian will be used.
     *
     * @param calendarType the name of the calendar type
     */
    void setCalendar(const QString & calendarType);

    /**
     * Converts a localized monetary string to a double.
     *
     * @param numStr the string we want to convert.
     * @param ok the boolean that is set to false if it's not a number.
     *           If @p ok is 0, it will be ignored
     *
     * @return The string converted to a double
     */
    double readMoney(const QString &numStr, bool * ok = 0) const;

    /**
     * Converts a localized numeric string to a double.
     *
     * @param numStr the string we want to convert.
     * @param ok the boolean that is set to false if it's not a number.
     *           If @p ok is 0, it will be ignored
     *
     * @return The string converted to a double
     */
    double readNumber(const QString &numStr, bool * ok = 0) const;

    /**
     * Converts a localized date string to a QDate.  This method will try all
     * ReadDateFlag formats in preferred order to read a valid date.
     *
     * The bool pointed by ok will be invalid if the date entered was not valid.
     *
     * @param str the string we want to convert.
     * @param ok the boolean that is set to false if it's not a valid date.
     *           If @p ok is 0, it will be ignored
     *
     * @return The string converted to a QDate
     * @see KCalendarSystem::readDate()
     */
    QDate readDate(const QString &str, bool* ok = 0) const;

    /**
     * Converts a localized date string to a QDate, using the specified format.
     * You will usually not want to use this method.
     * @see KCalendarSystem::readDate()
     */
    QDate readDate(const QString &intstr, const QString &fmt, bool* ok = 0) const;

    /**
     * Flags for readDate()
     */
    enum ReadDateFlags {
        NormalFormat          =    1, /**< Only accept a date string in
                                           the locale LongDate format */
        ShortFormat           =    2, /**< Only accept a date string in
                                           the locale ShortDate format */
        IsoFormat             =    4, /**< Only accept a date string in
                                           ISO date format (YYYY-MM-DD) */
        IsoWeekFormat         =    8, /**< Only accept a date string in
                                           ISO Week date format (YYYY-Www-D) */
        IsoOrdinalFormat      =   16  /**< Only accept a date string in
                                           ISO Week date format (YYYY-DDD) */
    };

    /**
     * Converts a localized date string to a QDate.
     * This method is stricter than readDate(str,&ok): it will only accept
     * a date in a specific format, depending on @p flags.
     *
     * @param str the string we want to convert.
     * @param flags what format the the date string will be in
     * @param ok the boolean that is set to false if it's not a valid date.
     *           If @p ok is 0, it will be ignored
     *
     * @return The string converted to a QDate
     * @see KCalendarSystem::readDate()
     */
    QDate readDate(const QString &str, ReadDateFlags flags, bool *ok = 0) const;

    /**
     * Converts a localized time string to a QTime.
     * This method will try to parse it with seconds, then without seconds.
     * The bool pointed to by @p ok will be set to false if the time entered was
     * not valid.
     *
     * @param str the string we want to convert.
     * @param ok the boolean that is set to false if it's not a valid time.
     *           If @p ok is 0, it will be ignored
     *
     * @return The string converted to a QTime
     */
    QTime readTime(const QString &str, bool* ok = 0) const;

    /**
     * Flags for the old version of readTime()
     *
     * @deprecated replaced by TimeFormatOptions
     */
    enum ReadTimeFlags {
        WithSeconds = 0,    ///< Only accept a time string with seconds. Default (no flag set)
        WithoutSeconds = 1  ///< Only accept a time string without seconds.
    }; // (maybe use this enum as a bitfield, if adding independent features?)

    /**
     * @deprecated replaced readLocaleTime()
     *
     * Converts a localized time string to a QTime.
     * This method is stricter than readTime(str,&ok): it will either accept
     * a time with seconds or a time without seconds.
     * Use this method when the format is known by the application.
     *
     * @param str the string we want to convert.
     * @param flags whether the time string is expected to contain seconds or not.
     * @param ok the boolean that is set to false if it's not a valid time.
     *           If @p ok is 0, it will be ignored
     *
     * @return The string converted to a QTime
     */
    QTime readTime(const QString &str, ReadTimeFlags flags, bool *ok = 0) const;

    /**
     * Additional processing options for readLocaleTime().
     *
     * @remarks This is currently used as an enum but declared as a flag
     *          to be extensible
     */
    enum TimeProcessingOption {
        ProcessStrict    = 0x1,    ///< Process time in a strict manner, ie.
                                   ///< a read time string has to exactly match
                                   ///< the defined time format.
        ProcessNonStrict = 0x2     ///< Process time in a lax manner, ie.
                                   ///< allow spaces in the time-format to be
                                   ///< left out when entering a time string.
    };
    Q_DECLARE_FLAGS(TimeProcessingOptions, TimeProcessingOption)

    /**
     * @since 4.4
     *
     * Converts a localized time string to a QTime.
     * This method is stricter than readTime(str, &ok) in that it will either
     * accept a time with seconds or a time without seconds.
     *
     * @param str the string we want to convert
     * @param ok the boolean that is set to false if it's not a valid time.
     *           If @p ok is 0, it will be ignored.
     * @param options format option to apply when formatting the time
     * @param processing if set to @c ProcessStrict, checking will be strict
     *               and the read time string has to have the exact time format
     *               specified. If set to @c ProcessNonStrict processing the time
     *               is lax and spaces in the time string can be left out.
     *
     * @return The string converted to a QTime
     */
    QTime readLocaleTime(const QString &str, bool *ok = 0,
                         TimeFormatOptions options = KLocale::TimeDefault,
                         TimeProcessingOptions processing = ProcessNonStrict) const;

    /**
     * Returns the language code used by this object. The domain AND the
     * library translation must be available in this language.
     * defaultLanguage() is returned by default, if no other available.
     *
     * Use languageCodeToName(language) to get human readable, localized
     * language name.
     *
     * @return the currently used language code
     *
     * @see languageCodeToName
     */
    QString language() const;

    /**
     * Returns the country code of the country where the user lives.
     * defaultCountry() is returned by default, if no other available.
     *
     * Use countryCodeToName(country) to get human readable, localized
     * country names.
     *
     * @return the country code for the user
     *
     * @see countryCodeToName
     */
    QString country() const;

    /**
     * Returns the language codes selected by user, ordered by decreasing
     * priority.
     *
     * Use languageCodeToName(language) to get human readable, localized
     * language name.
     *
     * @return list of language codes
     *
     * @see languageCodeToName
     */
    QStringList languageList() const;

    /**
     * @since 4.4
     *
     * Returns the ISO Currency Codes used in the locale, ordered by decreasing
     * priority.
     *
     * Use KCurrency::currencyCodeToName(currencyCode) to get human readable,
     * localized language name.
     *
     * @return list of ISO Currency Codes
     *
     * @see currencyCodeToName
     */
    QStringList currencyCodeList() const;

    /**
     * Returns the user's preferred encoding.
     *
     * @return The name of the preferred encoding
     *
     * @see codecForEncoding
     * @see encodingMib
     */
    const QByteArray encoding() const;

    /**
     * Returns the user's preferred encoding.
     *
     * @return The Mib of the preferred encoding
     *
     * @see encoding
     * @see codecForEncoding
     */
    int encodingMib() const;

    /**
     * Returns the user's preferred encoding. Should never be NULL.
     *
     * @return The codec for the preferred encoding
     *
     * @see encoding
     * @see encodingMib
     */
    QTextCodec * codecForEncoding() const;

    /**
     * Returns the file encoding.
     *
     * @return The Mib of the file encoding
     *
     * @see QFile::encodeName
     * @see QFile::decodeName
     */
    int fileEncodingMib() const;

    /**
     * Changes the current date format.
     *
     * The format of the date is a string which contains variables that will
     * be replaced:
     * @li %Y with the whole year (e.g. "2004" for "2004")
     * @li %y with the lower 2 digits of the year (e.g. "04" for "2004")
     * @li %n with the month (January="1", December="12")
     * @li %m with the month with two digits (January="01", December="12")
     * @li %e with the day of the month (e.g. "1" on the first of march)
     * @li %d with the day of the month with two digits (e.g. "01" on the first of march)
     * @li %b with the short form of the month (e.g. "Jan" for January)
     * @li %B with the long form of the month (e.g. "January")
     * @li %a with the short form of the weekday (e.g. "Wed" for Wednesday)
     * @li %A with the long form of the weekday (e.g. "Wednesday" for Wednesday)
     *
     * Everything else in the format string will be taken as is.
     * For example, March 20th 1989 with the format "%y:%m:%d" results
     * in "89:03:20".
     *
     * @param format The new date format
     */
    void setDateFormat(const QString & format);

    /**
     * Changes the current short date format.
     *
     * The format of the date is a string which contains variables that will
     * be replaced:
     * @li %Y with the whole year (e.g. "1984" for "1984")
     * @li %y with the lower 2 digits of the year (e.g. "84" for "1984")
     * @li %n with the month (January="1", December="12")
     * @li %m with the month with two digits (January="01", December="12")
     * @li %e with the day of the month (e.g. "1" on the first of march)
     * @li %d with the day of the month with two digits(e.g. "01" on the first of march)
     * @li %b with the short form of the month (e.g. "Jan" for January)
     * @li %B with the long form of the month (e.g. "January")
     * @li %a with the short form of the weekday (e.g. "Wed" for Wednesday)
     * @li %A with the long form of the weekday (e.g. "Wednesday" for Wednesday)
     *
     * Everything else in the format string will be taken as is.
     * For example, March 20th 1989 with the format "%y:%m:%d" results
     * in "89:03:20".
     *
     * @param format The new short date format
     */
    void setDateFormatShort(const QString & format);

    /**
     * Changes the form of month name used in dates.
     *
     * @param possessive True if possessive forms should be used
     */
    void setDateMonthNamePossessive(bool possessive);

    /**
     * Changes the current time format.
     *
     * The format of the time is string a which contains variables that will
     * be replaced:
     * @li %H with the hour in 24h format and 2 digits (e.g. 5pm is "17", 5am is "05")
     * @li %k with the hour in 24h format and one digits (e.g. 5pm is "17", 5am is "5")
     * @li %I with the hour in 12h format and 2 digits (e.g. 5pm is "05", 5am is "05")
     * @li %l with the hour in 12h format and one digits (e.g. 5pm is "5", 5am is "5")
     * @li %M with the minute with 2 digits (e.g. the minute of 07:02:09 is "02")
     * @li %S with the seconds with 2 digits  (e.g. the minute of 07:02:09 is "09")
     * @li %p with pm or am (e.g. 17.00 is "pm", 05.00 is "am")
     *
     * Everything else in the format string will be taken as is.
     * For example, 5.23pm with the format "%H:%M" results
     * in "17:23".
     *
     * @param format The new time format
     */
    void setTimeFormat(const QString & format);

    /**
     * @since 4.3
     *
     * Set digit characters used to display dates and time.
     *
     * @param digitSet the digit set identifier
     * @see DigitSet
     */
    void setDateTimeDigitSet(DigitSet digitSet);

    /**
     * Changes how KLocale defines the first day in week.
     *
     * @param day first day of the week (Monday=1..Sunday=7) as integer
     */
    void setWeekStartDay(int day);

    /**
     * Changes how KLocale defines the first working day in week.
     *
     * @since 4.2
     * @param day first working day of the week (Monday=1..Sunday=7) as integer
     */
    void setWorkingWeekStartDay(int day);

    /**
     * Changes how KLocale defines the last working day in week.
     *
     * @since 4.2
     * @param day last working day of the week (Monday=1..Sunday=7) as integer
     */
    void setWorkingWeekEndDay(int day);

    /**
     * Changes how KLocale defines the day reserved for religious observance.
     *
     * @since 4.2
     * @param day day of the week for religious observance (None=0,Monday=1..Sunday=7) as integer
     */
    void setWeekDayOfPray(int day);

    /**
     * Returns the currently selected date format.
     *
     * @return Current date format.
     * @see setDateFormat()
     */
    QString dateFormat() const;

    /**
     * Returns the currently selected short date format.
     *
     * @return Current short date format.
     * @see setDateFormatShort()
     */
    QString dateFormatShort() const;

    /**
     * Returns the currently selected time format.
     *
     * @return Current time format.
     * @see setTimeFormat()
     */
    QString timeFormat() const;

    /**
     * Changes the symbol used to identify the decimal pointer.
     *
     * @param symbol The new decimal symbol.
     */
    void setDecimalSymbol(const QString & symbol);

    /**
     * Changes the separator used to group digits when formating numbers.
     *
     * @param separator The new thousands separator.
     */
    void setThousandsSeparator(const QString & separator);

    /**
     * Changes the sign used to identify a positive number. Normally this is
     * left blank.
     *
     * @param sign Sign used for positive numbers.
     */
    void setPositiveSign(const QString & sign);

    /**
     * Changes the sign used to identify a negative number.
     *
     * @param sign Sign used for negative numbers.
     */
    void setNegativeSign(const QString & sign);

    /**
     * @since 4.3
     *
     * Changes the set of digit characters used to display numbers.
     *
     * @param digitSet the digit set identifier
     * @see DigitSet
     */
    void setDigitSet(DigitSet digitSet);

    /**
     * Changes the sign position used for positive monetary values.
     *
     * @param signpos The new sign position
     */
    void setPositiveMonetarySignPosition(SignPosition signpos);

    /**
     * Changes the sign position used for negative monetary values.
     *
     * @param signpos The new sign position
     */
    void setNegativeMonetarySignPosition(SignPosition signpos);

    /**
     * Changes the position where the currency symbol should be printed for
     * positive monetary values.
     *
     * @param prefix True if the currency symbol should be prefixed instead of
     * postfixed
     */
    void setPositivePrefixCurrencySymbol(bool prefix);

    /**
     * Changes the position where the currency symbol should be printed for
     * negative monetary values.
     *
     * @param prefix True if the currency symbol should be prefixed instead of
     * postfixed
     */
    void setNegativePrefixCurrencySymbol(bool prefix);

    /**
     * @deprecated use setDecimalPlaces() or setMonetaryDecimalPlaces()
     *
     * Changes the number of digits used when formating numbers.
     *
     * @param digits The default number of digits to use.
     */
    void setFracDigits(int digits);

    /**
     * @since 4.4
     *
     * Changes the number of decimal places used when formating numbers.
     *
     * @param digits The default number of digits to use.
     */
    void setDecimalPlaces(int digits);

    /**
     * @since 4.4
     *
     * Changes the number of decimal places used when formating money.
     *
     * @param digits The default number of digits to use.
     */
    void setMonetaryDecimalPlaces(int digits);

    /**
     * Changes the separator used to group digits when formating monetary values.
     *
     * @param separator The new thousands separator.
     */
    void setMonetaryThousandsSeparator(const QString & separator);

    /**
     * Changes the symbol used to identify the decimal pointer for monetary
     * values.
     *
     * @param symbol The new decimal symbol.
     */
    void setMonetaryDecimalSymbol(const QString & symbol);

    /**
     * @since 4.4
     *
     * Changes the current ISO Currency Code.
     *
     * @param newCurrencyCode The new Currency Code
     */
    void setCurrencyCode(const QString &newCurrencyCode);

    /**
     * Changes the current currency symbol.
     *
     * This symbol should be consistant with the selected Currency Code
     *
     * @param symbol The new currency symbol
     * @see currencyCode, KCurrency::currencySymbols
     */
    void setCurrencySymbol(const QString & symbol);

    /**
     * @since 4.3
     *
     * Set digit characters used to display monetary values.
     *
     * @param digitSet the digit set identifier
     * @see DigitSet
     */
    void setMonetaryDigitSet(DigitSet digitSet);

    /**
     * Returns the preferred page size for printing.
     *
     * @return The preferred page size, cast it to QPrinter::PageSize
     */
    int pageSize() const;

    /**
     * Changes the preferred page size when printing.
     *
     * @param paperFormat the new preferred page size in the format QPrinter::PageSize
     */
    void setPageSize(int paperFormat);

    /**
     * The Metric system will give you information in mm, while the
     * Imperial system will give you information in inches.
     */
    enum MeasureSystem {
        Metric,    ///< Metric system (used e.g. in Europe)
        Imperial   ///< Imperial system (used e.g. in the United States)
    };

    /**
     * Returns which measuring system we use.
     *
     * @return The preferred measuring system
     */
    MeasureSystem measureSystem() const;

    /**
     * Changes the preferred measuring system.
     *
     * @return value The preferred measuring system
     */
    void setMeasureSystem(MeasureSystem value);

    /**
     * Adds another catalog to search for translation lookup.
     * This function is useful for extern libraries and/or code,
     * that provide there own messages.
     *
     * If the catalog does not exist for the chosen language,
     * it will be ignored and en_US will be used.
     *
     * @param catalog The catalog to add.
     */
    void insertCatalog(const QString& catalog);

    /**
     * Removes a catalog for translation lookup.
     * @param catalog The catalog to remove.
     * @see insertCatalog()
     */
    void removeCatalog(const QString &catalog);

    /**
     * Sets the active catalog for translation lookup.
     * @param catalog The catalog to activate.
     */
    void setActiveCatalog(const QString &catalog);

    /**
     * Translates a message as a QTranslator is supposed to.
     * The parameters are similar to i18n(), but the result
     * value has other semantics (it can be QString())
     */
    QString translateQt(const char *context, const char *sourceText, const char *comment) const;

    /**
     * Provides list of all known language codes.
     *
     * Use languageCodeToName(language) to get human readable, localized
     * language names.
     *
     * @return list of all language codes
     *
     * @see languageCodeToName
     */
    QStringList allLanguagesList() const;

    /**
     * Convert a known language code to a human readable, localized form.
     * If an unknown language code is supplied, empty string is returned;
     * this will never happen if the code has been obtained by one of the
     * KLocale methods.
     *
     * @param language the language code
     *
     * @return the human readable and localized form if the code is known,
     *         empty otherwise
     *
     * @see language
     * @see languageList
     * @see allLanguagesList
     */
    QString languageCodeToName(const QString &language) const;

    /**
     * Provides list of all known country codes.
     *
     * Use countryCodeToName(country) to get human readable, localized
     * country names.
     *
     * @return a list of all country codes
     *
     * @see countryCodeToName
     */
    QStringList allCountriesList() const;

    /**
     * Convert a known country code to a human readable, localized form.
     *
     * If an unknown country code is supplied, empty string is returned;
     * this will never happen if the code has been obtained by one of the
     * KLocale methods.
     *
     * @param country the country code
     *
     * @return the human readable and localized form of the country name
     *
     * @see country
     * @see allCountriesList
     */
    QString countryCodeToName(const QString &country) const;

    /**
     * Parses locale string into distinct parts.
     * The format of locale is language_COUNTRY@modifier.CHARSET
     *
     * @param locale the locale string to split
     * @param language set to the language part of the locale
     * @param country set to the country part of the locale
     * @param modifier set to the modifer part of the locale
     * @param charset set to the charset part of the locale
     */
    static void splitLocale(const QString &locale, QString &language, QString &country,
                            QString &modifier, QString &charset);

    /**
     * Use this as main catalog for *all* KLocales, if not the appname
     * will be used. This function is best to be the very first instruction
     * in your program's main function as it only has an effect before the
     * first KLocale object is created.
     *
     * @param catalog Catalog to override all other main Catalogs.
     */
    static void setMainCatalog(const char *catalog);

    /**
     * @deprecated
     *
     * Finds localized resource in resourceDir( rtype ) + \<lang> + fname.
     *
     * Since KDE 4.1, this service is provided in a slightly different form,
     * automatically by e.g. KStandardDirs::locate() and other KDE core classes
     * dealing with paths. For manual use, it is replaced by localizedFilePath().
     *
     * @param fname relative path to find
     * @param rtype resource type to use
     *
     * @return path to localized resource
     *
     * @see localizedFilePath
     */
    static QString langLookup(const QString &fname, const char *rtype = "html");

    /**
     * Returns the name of the internal language.
     *
     * @return Name of the default language
     */
    static QString defaultLanguage();

    /**
     * Returns the name of the default country.
     *
     * @return Name of the default country
     */
    static QString defaultCountry();

    /**
     * @since 4.4
     *
     * Returns the ISO Code of the default currency.
     *
     * @return ISO Currency Code of the default currency
     */
    static QString defaultCurrencyCode();

    /**
     * Reports whether evaluation of translation scripts is enabled.
     *
     * @return true if script evaluation is enabled, false otherwise.
     */
    bool useTranscript() const;

    /**
     * Checks whether or not the active catalog is found for the given language.
     *
     * @param language language to check
     */
    bool isApplicationTranslatedInto(const QString & language);

    /**
     * Copies the catalogs of this object to an other KLocale object.
     *
     * @param locale the destination KLocale object
     */
    void copyCatalogsTo(KLocale *locale);

    /**
     * Changes the current country. The current country will be left
     * unchanged if failed. It will force a reload of the country specific
     * configuration.
     *
     * @param country the ISO 3166 country code
     * @param config  a configuration file with a Locale group detailing
     *                locale-related preferences (such as date and time
     *                formatting)
     *
     * @return @c true on success, @c false on failure
     */
    bool setCountry(const QString & country, KConfig *config);

    /**
     * Changes the current language. The current language will be left
     * unchanged if failed. It will force a reload of the country specific
     * configuration as well.
     *
     * @param language the language code
     * @param config  a configuration file with a Locale group detailing
     *                locale-related preferences (such as date and time
     *                formatting)
     *
     * @return true on success
     */
    bool setLanguage(const QString &language, KConfig *config);

    /**
     * Changes the list of preferred languages for the locale. The first valid
     * language in the list will be used, or the default language (en_US)
     * if none of the specified languages were available.
     *
     * @param languages the list of language codes
     *
     * @return true if one of the specified languages were used
     */
    bool setLanguage(const QStringList &languages);

    /**
     * @since 4.1
     *
     * Tries to find a path to the localized file for the given original path.
     * This is intended mainly for non-text resources (images, sounds, etc.),
     * whereas text resources should be handled in more specific ways.
     *
     * The possible localized paths are checked in turn by priority of set
     * languages, in form of dirname/l10n/ll/basename, where dirname and
     * basename are those of the original path, and ll is the language code.
     *
     * KDE core classes which resolve paths internally (e.g. KStandardDirs)
     * will usually perform this lookup behind the scene.
     * In general, you should pipe resource paths through this method only
     * on explicit translators' request, or when a resource is an obvious
     * candidate for localization (e.g. a splash screen or a custom icon
     * with some text drawn on it).
     *
     * @param filePath path to the original file
     *
     * @return path to the localized file if found, original path otherwise
     */
    QString localizedFilePath(const QString &filePath) const;

    /**
     * @since 4.2
     *
     * Removes accelerator marker from a UI text label.
     *
     * Accelerator marker is not always a plain ampersand (&),
     * so it is not enough to just remove it by @c QString::remove().
     * The label may contain escaped markers ("&&") which must be resolved
     * and skipped, as well as CJK-style markers ("Foo (&F)") where
     * the whole parenthesis construct should be removed.
     * Therefore always use this function to remove accelerator marker
     * from UI labels.
     *
     * @param label UI label which may contain an accelerator marker
     * @return label without the accelerator marker
     */
    QString removeAcceleratorMarker(const QString &label) const;

    /**
     * @since 4.3
     *
     * Convert all digits in the string to the given digit set.
     *
     * Conversion is normally not performed if the given digit set
     * is not appropriate in the current locale and language context.
     * Unconditional conversion may be requested by setting
     * @p ignoreContext to @c true.
     *
     * @param str the string to convert
     * @param digitSet the digit set identifier
     * @param ignoreContext unconditional conversion if @c true
     *
     * @return string with converted digits
     *
     * @see DigitSet
     */
    QString convertDigits(const QString &str, DigitSet digitSet,
                          bool ignoreContext = false) const;

private:
    KLocalePrivate * const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KLocale::DateTimeFormatOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(KLocale::TimeFormatOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(KLocale::TimeProcessingOptions)

#endif
