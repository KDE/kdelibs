/* This file is part of the KDE libraries
    Copyright (C) 1997 Stephan Kulow (coolo@kde.org)

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef _KLOCALE_H
#define _KLOCALE_H

#include <qintdict.h>
#include <qstring.h>
#include <qstringlist.h>

class QTextCodec;
class QStrList;
class QDate;
class QTime;
class QDateTime;
class KConfig;
class KLocalePrivate;

#ifndef I18N_NOOP
#define I18N_NOOP(x) (x)
#endif

/**
 *  i18n is the function that does everything you need to translate
 *  a string. You just wrap around every user visible string a i18n
 *  call to get a QString with the string in the user's prefered
 *  language.
 **/
QString i18n(const char *text);

/**
  *
  * KLocale provides support for country specific stuff like
  * the national language.
  *
  * KLocale supports translating, as well as specifying the format
  * for numbers, currency, time, and date.
  *
  * @author Stephan Kulow <coolo@kde.org>, Preston Brown <pbrown@kde.org>, Hans Petter Bieker <bieker@kde.org>
  * @short class for supporting locale settings and national language
  */
class KLocale {

      friend class KGlobal;
public:
    /**
      * Create a KLocale with the given catalogue name.
      * If no catalogue is given, the application name is used.
      * The constructor looks for an entry Locale/Language in the
      * configuration file.
      * If nothing is set there, it looks for the environment variable
      * $LANG. The format for LANG is de:fr:.., if de
      * (german) is your prefered language and fr (french) is your
      * second prefered language. You can add as many languages as
      * you want. If none of them can be find, the default (C) will
      * be used.
      * @param catalogue The name of the language file
      */
    KLocale( const QString& catalogue = QString::null );

    /**
      * Destructor.
      */
    ~KLocale();

    /**
      * Translate the string into the corresponding string in
      * the national language, if available. If not, returns
      * the string itself.
      * There is a KDE wide message file that contains the most
      * often used phrases, so we can avoid duplicating the
      * translation of these phrases. If a phrase is not found
      * in the catalogue given to the constructor, it will search
      * in the system catalog. This makes it possible to override
      * some phrases for your needs.
      * @param index The lookup text and default text, if not found.
      */
    QString translate( const char *index ) const;

    /**
     * Allows programs such as kcmlocale to choose which translation to use.
     */
    void setLanguage(const QString &_lang);

    /**
     * Allows programs such as kcmlocale to choose how to format numbers etc.
     */
    void setCountry(const QString &country);
    void setCountry(const QString &_number, const QString &_money, const QString &_time);

    /**
     * Various positions for where to place the positive or negative
     * sign when they are related to a monetary value.
     */
    enum SignPosition { ParensAround = 0, BeforeQuantityMoney = 1,
			AfterQuantityMoney = 2,
			BeforeMoney = 3, AfterMoney = 4 };

    /**
     * Retrieve what a decimal point should look like ("." or "," etc.)
     * according to the current locale or user settings.
     */
    QString decimalSymbol() const;

    /**
     * Retrieve what the thousands separator should look
     * like ("," or "." etc.)
     * according to the current locale or user settings.
     */
    QString thousandsSeparator() const;

    /**
     * Retrieve what the symbol denoting currency in the current locale
     * as as defined by user settings should look like.
     */
    QString currencySymbol() const;

    /**
     * Retrieve what a decimal point should look like ("." or "," etc.)
     * for monetary values, according to the current locale or user
     * settings.
     */
    QString monetaryDecimalSymbol() const;

    /**
     * Retrieve what a thousands separator for monetary values should
     * look like ("," or " " etc.) according to the current locale or
     * user settings.
     */
    QString monetaryThousandsSeparator() const;

    /**
     * Retrieve what a positive sign should look like ("+", " ", etc.)
     * according to the current locale or user settings.
     */
    QString positiveSign() const;

    /**
     * Retrieve what a negative sign should look like ("-", etc.)
     * according to the current locale or user settings.
     */
    QString negativeSign() const;

    /**
     * The number of fractional digits to include in numeric/monetary
     * values (usually 2).
     */
    int fracDigits() const;

    /**
     * If and only if the currency symbol precedes a positive value,
     * this will be true.
     */
    bool positivePrefixCurrencySymbol() const;

    /**
     * If and only if the currency symbol precedes a negative value,
     * this will be true.
     */
    bool negativePrefixCurrencySymbol() const;

    /**
     * Retrieve the position of a positive sign in relation to a
     * monetary value.
     *
     * @see SignPosition
     */
    SignPosition positiveMonetarySignPosition() const;

    /**
     * Denotes where to place a negative sign in relation to a
     * monetary value.
     *
     * @see SignPosition
     */
    SignPosition negativeMonetarySignPosition() const;

    /**
     * Given an double, convert that to a numeric string containing
     * the localized monetary equivalent.
     *
     * e.g. given 123456, return "$123,456".
     *
     * @return The number of money as a localized string
     */
    QString formatMoney(double num, const QString &currency = QString::null, int digits = -1) const;

    /**
     * This function differs from the above only in that it can take
     * a QString as the argument for convenience.
     *
     * @return The number of money as a localized string
     */
    QString formatMoney(const QString &numStr) const;

    /**
     * Given an double, convert that to a numeric string containing
     * the localized numeric equivalent.
     *
     * e.g. given 123456.78, return "123,456.78" (for some European country).
     * If precision isn't specified, fracDigits is used.
     *
     * @return The number as a localized string
     */
    QString formatNumber(double num, int precision = -1) const;

    /**
     * This function differs from the above only in that it can take
     * a QString as the argument for convenience.
     *
     * @return The number as a formated string
     */
    QString formatNumber(const QString &numStr) const;

    /**
     * Return a string formatted to the current locale's conventions
     * regarding dates.
     *
     * @return The date as a string
     */
    QString formatDate(const QDate &pDate, bool shortfmt = false) const;

    /**
     * Return a string formatted to the current locale's conventions
     * regarding times.
     *
     * @param includeSecs if true, seconds are included in the output,
     *        otherwise only hours and minutes are formatted.
     *
     * @return The time as a string
     */
    QString formatTime(const QTime &pTime, bool includeSecs = false) const;

    /**
     * Use this to determine if the user wants a 12 clock.
     *
     * @return If the user wants 12h lock
     */
    bool use12Clock();

    /**
     * Return a string containing the name of the month name.
     *
     * @param i the month number of the year starting at 1/January.
     *
     * @return The name of the month
     */
    QString MonthName(int i) const;

    /**
     * Return a string containing the name of the week day.
     *
     * @param i the day number of the week starting at 1/Monday.
     *
     * @return The name of the week
     */
    QString WeekDayName(int i) const;

    /**
     * Return a string formated to the current locale's conventions
     * regarding both date and time.
     *
     * @return The date and time as a string
     */
    QString formatDateTime(const QDateTime &pDateTime) const;

    /**
     * Converts a localized monetary string to a double.
     * @param numStr the string we want to convert.
     * @param ok the boolean that is set to false if it's not a number.
     *
     * @return The string converted to a double
     */
    double readMoney(const QString &numStr, bool * ok=0) const;

    /**
     * Converts a localized numeric string to a double.
     * @param numStr the string we want to convert.
     * @param ok the boolean that is set to false if it's not a number.
     *
     * @return The string converted to a double
     */
    double readNumber(const QString &numStr, bool * ok=0) const;

    /**
     * Converts a localized date string to a QDate. Note:
     * This only works on short dates for the time beeing.
     *
     * @param str the string we want to convert.
     *
     * @return The string converted to a QDate
     */
    QDate readDate(const QString &str) const;

    /**
      * Creates an alias for the string text. It will be translated
      * and stored under the integer constant index.
      * This can help you to avoid repeated translation.
      * Be aware, that this is only needed in cases, where you want
      * to translate it in loops or something like that.
      * In every other case, the @ref translate methods is fast
      * enough.
      */
    void aliasLocale( const char *text, long int index);

    /**
      * Returns an alias, that you have set before or 0, if not
      * set. This method uses @ref QIntDict.
      */
    QString getAlias( long key ) const;

    /**
      * Returns the language used by this object. The domain AND the
      * library translation must be available in this language.
      * 'C' is default, if no other available.
      */
    QString language() const;

    /**
      * Returns the locale used for money by object.
      */
    QString money() const;

    /**
      * Returns the locale used for numbers by object.
      */
    QString number() const;

    /**
      * Returns the locale used for time by object.
      */
    QString time() const;

    /**
      * Returns the languages selected by user.
      *
      * @return String containing locale codes separated by colons
      */
    QString languages() const { return langs; }

    /**
      * Returns the languages selected by user.
      *
      * @return List of language codes
      */
    QStringList languageList() const;

    /**
      * Returns the charset name by selected locale.
      * This will be the charset defined in the config file.
      * NOTE: This is no longer the same as encoding.
      * "iso-8859-1" is default
      *
      * @return Name of the prefered charset for fonts
      */
    QString charset() const { return chset; }

    /**
     * adds anther catalogue to search for translation lookup.
     * This function is useful for extern libraries and/or code,
     * that provides it's own messages.
     *
     * If the catalogue does not exist for the chosen language,
     * it will be ignored and C will be used.
     **/
    void insertCatalogue(const QString& catalogue);

    /**
     * returns the parts of the parameter str understood as language setting
     * the format is language_country.charset
     */
    static void splitLocale(const QString& str,QString& language, QString& country,
                     QString &charset);

    /**
     * Init the l10n part of the instance with the config object.
     */
    void initFormat(KConfig *config);

    /**
     * Init the l18n part of the instance with the given config object. It should
     * be valid and contain the global entries.
     **/
    void initLanguage(KConfig *config, const QString& catalogue);

    /**
     * @return True if the KLocale instance is initialized already. You can't
     * translate before it is.
     * The constructor will initialize the instance, but under some
     * circumstances - when the circumstances do not fit for initialization
     * - it will just delay the initialization til you call initLanguage
     */
    bool inited() const { return _inited; }

    /**
     * Use this to as main catalogue for *all* KLocales, if not the appname
     * will be used.
     */
    static void setMainCatalogue(const char *catalogue);

protected:
    QString chset;

    // Numbers and money
    QString _decimalSymbol;
    QString _thousandsSeparator;
    QString _currencySymbol;
    QString _monetaryDecimalSymbol;
    QString _monetaryThousandsSeparator;
    QString _positiveSign;
    QString _negativeSign;
    int _fracDigits;
    bool _positivePrefixCurrencySymbol;
    bool _negativePrefixCurrencySymbol;
    SignPosition _positiveMonetarySignPosition;
    SignPosition _negativeMonetarySignPosition;

    // Date and time
    QString _timefmt;
    QString _datefmt;
    QString _datefmtshort;

private:
    QStrList *catalogues;
    QIntDict<QString> aliases;
    bool _inited;
    QString lang;
    QTextCodec *_codec;
    QString langs;
    QString _number;
    QString _money;
    QString _time;

    void setEncodingLang(const QString &_lang);

    // Disallow assignment and copy-construction
    KLocale( const KLocale& );
    KLocale& operator= ( const KLocale& );

    static void initInstance();

    KLocalePrivate *d;
};

QString i18n(const char *text);

#endif
