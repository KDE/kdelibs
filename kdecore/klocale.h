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

#include <qstring.h>
#include <qstringlist.h>

class QTextCodec;
class QStrList;
class QDate;
class QTime;
class QDateTime;

class KConfig;
class KLocalePrivate;
class KCatalogue;

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
 *  If the string is too ambigious to be translated well to a non-english
 *  language, use this form of i18n to separate lookup string and english
 *  text
 *  @see translate
 **/
QString i18n(const char *index, const char *text);

/**
 *  If you want to handle plural forms, use this form of i18n.
 *  The plural has to contain a %n where n fits into.
 *  @see translate
 **/
QString i18n(const char *singular, const char *plural, unsigned long n);

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
class KLocale
{
public:
  /**
   * Create a KLocale with the given catalogue name.
   * The constructor looks for an entry Locale/Language in the
   * configuration file.
   * If nothing is set there, it looks for the environment variable
   * $LANG. The format for LANG is de, if de (german) is your
   * prefered language. If none of them can be find, the default (C)
   * will be used.
   *
   * @param catalogue The name of the main language file
   * @param useEnv True if we should use environment variables.
   */
  KLocale( const QString& catalogue, bool useEnv = true );

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
   *
   * @param index The lookup text and default text, if not found.
   */
  QString translate( const char *index ) const;

  /**
   * Translate the string into the corresponding string in the
   * national language, if available.
   *
   * The real contents of the string is in the argument fallback,
   * but the meaning of it is coded into the argument index.
   * In some cases you'll need this function, when english is
   * too ambigious to express it.
   *
   * Most of the times the translators will tell you if it can't
   * be translated as it, but think of cases as "New", where the
   * translations differs depending on what is New.
   * Or simple cases as "Open", that can be used to express something
   * is open or it can be used to express that you want something to
   * open... There are tons of such examples.
   *
   * If translate("Open") is not enough to translate it well, use
   * translate("To Open", "Open") or translate("Is Open", "Open").
   * The english user will see "Open" in both cases, but the translated
   * version may vary. Of course you can also use i18n()
   *
   * @param index The lookup text
   * @param fallback the default text, if not found
   * @return translation
   */
  QString translate( const char *index, const char *fallback) const;

  QString translate( const char *singular, const char *plural,
		     unsigned long n) const;

  /**
   * @deprecated Changes the current charset used for fonts.
   *
   * @param charset The new charset
   *
   * @return True on success.
   */
  bool setCharset(const QString & charset);
  /**
   * Changes the current encoding.
   *
   * @param mibEnum The mib of the prefered codec
   *
   * @return True on success.
   */
  bool setEncoding(int mibEnum);

  /**
   * Changes the current language. The current language will be left
   * unchanged if failed. It will force a reload of the country specific 
   * configuration as well.
   *
   * @param language The ISO ???? language code.
   *
   * @return True on success.
   */
  bool setLanguage(const QString & language);

  /**
   * Changes the current country. The current country will be left 
   * unchanged if failed. It will force a reload of the country specific
   * configuration.
   *
   * @param country The ISO 3166 country code.
   * @param defaultsOnly True if we should not let the global configuration
   * overload the country profile.
   *
   * @return True on success.
   */
  bool setCountry(const QString & country);
  
  /**
   * @deprecated This will be changed before KDE 3 is released. Do not use.
   * Reloads the configuration with or without the global configuration.
   *
   * @param defaultsOnly True if we should not read the global configuration.
   */
  void setDefaultsOnly(bool defaultsOnly);

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
   *
   * @return The decimal symbol used by locale.
   */
  QString decimalSymbol() const;

  /**
   * Retrieve what the thousands separator should look
   * like ("," or "." etc.)
   * according to the current locale or user settings.
   *
   * @return The thousands separator used by locale.
   */
  QString thousandsSeparator() const;

  /**
   * Retrieve what the symbol denoting currency in the current locale
   * as as defined by user settings should look like.
   *
   * @return The default currecy symbol used by locale.
   */
  QString currencySymbol() const;
  
  /**
   * Retrieve what a decimal point should look like ("." or "," etc.)
   * for monetary values, according to the current locale or user
   * settings.
   *
   * @return The monetary decimal symbol used by locale.
   */
  QString monetaryDecimalSymbol() const;

  /**
   * Retrieve what a thousands separator for monetary values should
   * look like ("," or " " etc.) according to the current locale or
   * user settings.
   *
   * @return The monetary thousands separator used by locale.
   */
  QString monetaryThousandsSeparator() const;

  /**
   * Retrieve what a positive sign should look like ("+", " ", etc.)
   * according to the current locale or user settings.
   *
   * @return The positive sign used by locale.
   */
  QString positiveSign() const;

  /**
   * Retrieve what a negative sign should look like ("-", etc.)
   * according to the current locale or user settings.
   *
   * @return The negative sign used by locale.
   */
  QString negativeSign() const;

  /**
   * The number of fractional digits to include in numeric/monetary
   * values (usually 2).
   *
   * @return Default number of fractional digits used by locale.
   */
  int fracDigits() const;

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
   * @return Where to print the currecy symbol for negative numbers.
   */
  bool negativePrefixCurrencySymbol() const;

  /**
   * Retrieve the position of a positive sign in relation to a
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
   * Given a double, convert that to a numeric string containing
   * the localized monetary equivalent.
   *
   * e.g. given 123456, return "$ 123,456.00".
   *
   * @param num The number we want to format
   * @param currency The currency symbol you want.
   * @param digits Number of fractional digits.
   *
   * @return The number of money as a localized string
   */
  QString formatMoney(double num,
		      const QString & currency = QString::null,
		      int digits = -1) const;

  /**
   * This function differs from the above only in that it can take
   * a QString as the argument for convenience.
   *
   * @param numStr The string you want to reformat.
   *
   * @return The number of money as a localized string
   */
  QString formatMoney(const QString &numStr) const;
  
  /**
   * Given a double, convert that to a numeric string containing
   * the localized numeric equivalent.
   *
   * e.g. given 123456.78, return "123,456.78" (for some European country).
   * If precision isn't specified, 2 is used.
   *
   * @param num The number to convert
   * @param precision Number of fractinal digits used.
   *
   * @return The number as a localized string
   */
  QString formatNumber(double num, int precision = -1) const;

  /**
   * This function differs from the above only in that it can take
   * a QString as the argument for convenience.
   *
   * @param numStr The string you want to convert.
   *
   * @return The number as a formated string
   */
  QString formatNumber(const QString &numStr) const;

  /**
   * Return a string formatted to the current locale's conventions
   * regarding dates.
   *
   * @param pDate The date to be formated.
   * @param shotFormat True for non text dates.
   *
   * @return The date as a string
   */
  QString formatDate(const QDate &pDate, bool shortFormat = false) const;

  /**
   * Return a string formatted to the current locale's conventions
   * regarding times.
   *
   * @param pTime The time to be formated.
   * @param includeSecs if true, seconds are included in the output,
   *        otherwise only hours and minutes are formatted.
   *
   * @return The time as a string
   */
  QString formatTime(const QTime &pTime, bool includeSecs = false) const;

  /**
   * Use this to determine if the user wants a 12 hour clock.
   *
   * @return If the user wants 12h clock
   */
  bool use12Clock() const;

  /**
   * Use this to determine if the user wants the week to start on monday.
   *
   * @return true if the week starts on monday
   */
  bool weekStartsMonday() const;

  /**
   * Return a string containing the name of the month name.
   *
   * @param i the month number of the year starting at 1/January.
   * @param shortName we will return the short versoin of the string.
   *
   * @return The name of the month
   */
  QString monthName(int i, bool shortName = false) const;

  /**
   * Return a string containing the name of the week day.
   *
   * @param i the day number of the week starting at 1/Monday.
   * @param shortName we will return the short versoin of the string.
   *
   * @return The name of the week
   */
  QString weekDayName(int i, bool shortName = false) const;

  /**
   * Return a string formated to the current locale's conventions
   * regarding both date and time. Defaults to short date format.
   *
   * @param pDateTime The date and time to be formated.
   *
   * @return The date and time as a string
   */
  QString formatDateTime(const QDateTime &pDateTime) const;

  /**
   * Return a string formated to the current locale's conventions
   * regarding both date and time.
   *
   * @param pDateTime The date and time to be formated.
   * @param shortFormat using the short date format.
   * @param includeSeconds using the short date format.
   *
   * @return The date and time as a string
   */
  QString formatDateTime(const QDateTime &pDateTime,
			 bool shortFormat,
			 bool includeSecs = false) const;

  /**
   * Converts a localized monetary string to a double.
   *
   * @param numStr the string we want to convert.
   * @param ok the boolean that is set to false if it's not a number.
   *
   * @return The string converted to a double
   */
  double readMoney(const QString &numStr, bool * ok = 0) const;

  /**
   * Converts a localized numeric string to a double.
   *
   * @param numStr the string we want to convert.
   * @param ok the boolean that is set to false if it's not a number.
   *
   * @return The string converted to a double
   */
  double readNumber(const QString &numStr, bool * ok = 0) const;

  /**
   * Converts a localized date string to a QDate.
   * isValid() on the returning object will be false
   * if the date entered was not valid.
   *
   * Note: This only works on short dates for the time beeing.
   *
   * @param str the string we want to convert.
   *
   * @return The string converted to a QDate
   */
  QDate readDate(const QString &str) const;

  /**
   * Converts a localized date string to a QDate, using the specified format.
   * You will usually not want to use this method.
   */
  QDate readDate( const QString &intstr, const QString &fmt) const;

  /**
   * Converts a localized time string to a QTime.
   * isValid() on the returning object will be false
   * if the time entered was not valid.
   *
   * @param str the string we want to convert.
   *
   * @return The string converted to a QTime
   */
  QTime readTime(const QString &str) const;

  /**
   * Returns the language used by this object. The domain AND the
   * library translation must be available in this language.
   * "C" is default, if no other available.
   *
   * @return The currently used language.
   */
  QString language() const;

  /**
   * Returns the country code of the country where the user lives.
   * "C" is default, if no other available
   *
   * @return The country code for the user.
   */
  QString country() const;

  /**
   * Returns the languages selected by user.
   *
   * @return String containing language codes separated by colons
   */
  QString languages() const;

  /**
   * Returns the languages selected by user.
   *
   * @return List of language codes
   */
  QStringList languageList() const;

  /**
   * @deprecated Returns the charset name by selected locale.
   * This will be the charset defined in the config file.
   * NOTE: This is no longer the same as encoding.
   * "iso-8859-1" is default
   *
   * @return Name of the prefered charset for fonts
   *
   * @sa encoding
   */
  QString charset() const;

  /**
   * Retrives the user's prefered encoding
   *
   * @return The name of the prefered encoding
   *
   * @sa codecForEncoding
   * @sa encodingMib
   */
  const char * encoding() const;

  /**
   * Retrieves the user's prefered encoding.
   *
   * @return The Mib of the prefered encoding
   *
   * @sa encoding
   * @sa codecForEncoding
   */
  int encodingMib() const;
  /**
   * Retrieves the user's prefered encoding. Should never be NULL.
   *
   * @return The codec for the prefered encoding
   *
   * @sa encoding
   * @sa encodingMib
   */
  QTextCodec * codecForEncoding() const;

  /**
   * Changes the current date format.
   *
   * @param format The new date format
   */
  void setDateFormat(const QString & format);
  /**
   * Changes the current short date format.
   *
   * @param format The new short date format
   */
  void setDateFormatShort(const QString & format);
  /**
   * Chagnes the current time format.
   *
   * @param format The new time format
   */
  void setTimeFormat(const QString & format);
  void setWeekStartsMonday(bool start);
  /**
   * Retrieves the currently selected date format.
   *
   * @return Current date format.
   */
  QString dateFormat() const;
  /**
   * Retrieves the currently selected short date format.
   *
   * @return Current short date format.
   */
  QString dateFormatShort() const;
  /**
   * Retrieves the currently selected time format.
   *
   * @return Current time format.
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
   * @param seprator The new thousands separator.
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
   * Changes the number of digits used when formating numbers.
   *
   * @param digits The default number of digist to use.
   */
  void setFracDigits(int digits);
  /**
   * Changes the separator used to group digits when formating monetary values.
   *
   * @param seprator The new thousands separator.
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
   * Changes the current currency symbol.
   *
   * @param symbol The new currency symbol
   */
  void setCurrencySymbol(const QString & symbol);

  /**
   * adds another catalogue to search for translation lookup.
   * This function is useful for extern libraries and/or code,
   * that provides its own messages.
   *
   * If the catalogue does not exist for the chosen language,
   * it will be ignored and C will be used.
   *
   * @param catalogue The catalogue to add.
   */
  void insertCatalogue(const QString& catalogue);

  /**
   * translates a message as a QTranslator is supposed to.
   * The parameters are similiar to i18n(), but the result
   * value has other semantics (it can be QString::null)
   **/
  QString translateQt(const char *context, const char *sourceText) const;

  /**
   * returns the parts of the parameter str understood as language setting
   * the format is language_COUNTRY.charset
   *
   * @param str The string to split.
   * @param language This will be set to the language part of the string.
   * @param country This will be set to the country part of the string.
   * @param charset This will be set to the charset part of the stirng.
   */
  static void splitLocale(const QString & str,
			  QString & language,
			  QString & country,
			  QString & charset);

  /**
   * Use this to as main catalogue for *all* KLocales, if not the appname
   * will be used.
   *
   * @param catalogue Catalogue to override all other main catalogues.
   */
  static void setMainCatalogue(const char *catalogue);

  /**
   * Find localized resource in resourceDir( rtype ) + <lang> + fname.
   *
   * @param fname relative path to find
   * @param rtype resource type to use
   */
  static QString langLookup(const QString &fname, const char *rtype = "html");

  /**
   * Retrieves the name of the internal language
   *
   * @return Name of the default language
   */
  static QString defaultLanguage();

  /**
   * Retrieves the name of the default country
   *
   * @return Name of the default country
   */
  static QString defaultCountry();

  /**
   * @internal Creates a KLocale object for KGlobal and inits the locale
   * pointer.
   *
   * #### HPB: Make protected again?
   */
  static void initInstance();

private:
  // Disallow assignment and copy-construction
  // #### HPB: Implement this for KDE 3 and make it public!?
  KLocale( const KLocale& );
  KLocale& operator= ( const KLocale& );

private:
  /**
   * @internal Init the localization part of the instance with the config
   * object.
   *
   * @param config The configuration object used for init.
   */
  void initFormat(KConfig *config);

  /**
   * @internal Init the language part of the instance with the given config
   * object. It should be valid and contain the global entries.
   *
   * @param config The configuration object used for init
   * @param useEnv True if we should use environment variables
   */
  void initLanguage(KConfig * config, bool useEnv);
  
  /**
   * @internal Figure out which charset the user prefers.
   *
   * @param config The configuration object used for init
   */
  void initCharset(KConfig * config);

  /**
   * @internal FIgure out which encoding the user prefers.
   * 
   * @param config The configuration object used for init
   */
  void initEncoding(KConfig * config);

  /**
   * @internal function used by setLanguage().
   */
  void initCodec();

  /**
   * @internal function used by readTime(const QString &) const.
   */
  QTime readTime(const QString &str, bool seconds) const;

  /**
   * @internal function used by readDate(const QString &) const.
   */
  QDate readDate(const QString &str, bool shortFormat) const;

  /**
   * @internal Changes the file name of the catalogue to the correct
   * one.
   */
  void initCatalogue( KCatalogue * catalogue );
  /**
   * @internal Read the language and format configuration form disk.
   */
  void doBindInit();
  /**
   * @internal Reads the format configuration from disk.
   */
  void doFormatInit();

  /**
   * @internal function used by the two translate versions
   */
  QString translate_priv(const char *index,
			 const char *text,
			 const char ** original = 0) const;

  /**
   * @internal function used to determine if we are using the C translation
   */
  bool useDefaultLanguage() const;

private:
  // #### HPB: remove for KDE 3?
  QString m_charset;

  bool m_weekStartsMonday;

  // Numbers and money
  QString m_decimalSymbol;
  QString m_thousandsSeparator;
  QString m_currencySymbol;
  QString m_monetaryDecimalSymbol;
  QString m_monetaryThousandsSeparator;
  QString m_positiveSign;
  QString m_negativeSign;
  int m_fracDigits;
  bool m_positivePrefixCurrencySymbol;
  bool m_negativePrefixCurrencySymbol;
  SignPosition m_positiveMonetarySignPosition;
  SignPosition m_negativeMonetarySignPosition;

  // Date and time
  QString m_timeFormat;
  QString m_dateFormat;
  QString m_dateFormatShort;

  // #### HPB: Remove in KDE 3
  QStrList *catalogues;
  // #### HPB: Remove in KDE 3
  bool m_inited;

  // #### HPB: Remove in KDE 3
  QCString m_lang;
  QTextCodec * m_codec; // Codec used for translations
  // #### HPB: Remove in KDE 3
  QString m_langs;
  QString m_country;

  KLocalePrivate *d;
};

#endif
