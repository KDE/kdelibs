// -*- c-basic-offset: 2 -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997 Stephan Kulow <coolo@kde.org>
    Copyright (C) 1999-2001 Hans Petter Bieker <bieker@kde.org>
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef _KLOCALE_H
#define _KLOCALE_H

#include <qstring.h>

class QStringList;
class QTextCodec;
class QDate;
class QTime;
class QDateTime;

class KGlobal;
class KConfig;
class KConfigBase;
class KLocalePrivate;
class KCatalogue;

#ifndef I18N_NOOP
#define I18N_NOOP(x) (x)
#endif

/**
 *  i18n is the function that does everything you need to translate
 *  a string. You just wrap around every user visible string a i18n
 *  call to get a QString with the string in the user's preferred
 *  language.
 *
 *  The argument must be an UTF-8 encoded string (If you only use
 *  characters that are in US-ASCII, you're on the safe side. But
 *  for e.g. german umlauts or french accents should be recoded to
 *  UTF-8)
 **/
QString i18n(const char *text);

/**
 *  If the string is too ambiguous to be translated well to a non-english
 *  language, use this form of i18n to separate lookup string and english
 *  text.
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
 * Qt3's uic generates i18n( "msg", "comment" ) calls which conflict
 * with our i18n method. We use uic -tr tr2i18n to redirect
 * to the right i18n() function
**/
inline QString tr2i18n(const char* message, const char* =0) {
  return i18n(message);
}

/**
  *
  * KLocale provides support for country specific stuff like
  * the national language.
  *
  * KLocale supports translating, as well as specifying the format
  * for numbers, currency, time, and date.
  *
  * @author Stephan Kulow <coolo@kde.org>, Preston Brown <pbrown@kde.org>,
  * Hans Petter Bieker <bieker@kde.org>, Lukas Tinkl <lukas.tinkl@suse.cz>
  * @short class for supporting locale settings and national language
  */
class KLocale
{
  friend class KGlobal; // for initInstance()
public:
  /**
   * Constructs a KLocale with the given catalogue name.
   * The constructor looks for an entry Locale/Language in the
   * configuration file.
   * If no config file is specified, it will also look for languages
   * using the environment variables (KDE_LANG, LC_MESSAGES, LC_ALL, LANG),
   * as well as the global configuration fie. If we were not able to use
   * non of the specified languages, the default language (en_US) will be
   * used.
   *
   * If you specify a configuration file, it has to be valid until
   * the KLocale object is destroyed.
   *
   * @param catalogue The name of the main language file
   * @param config The configuration file to use.
   */
  KLocale( const QString& catalogue, KConfig *config = 0 );

  /**
   * Copy constructor.
   */
  KLocale( const KLocale & rhs );

  /**
   * Assignment operator.
   */
  KLocale& operator= ( const KLocale & rhs );

  /**
   * Destructor.
   */
  ~KLocale();

  /**
   * Translates the string into the corresponding string in
   * the national language, if available. If not, returns
   * the string itself.
   * There is a KDE wide message file that contains the most
   * often used phrases, so we can avoid duplicating the
   * translation of these phrases. If a phrase is not found
   * in the catalogue given to the constructor, it will search
   * in the system catalog. This makes it possible to override
   * some phrases for your needs.
   *
   *  The argument must be an UTF-8 encoded string (If you only use
   *  characters that are in US-ASCII you're on the safe side. But
   *  for e.g. german umlauts or french accents should be recoded to
   *  UTF-8)
   *
   * @param index The lookup text and default text, if not found.
   */
  QString translate( const char *index ) const;

  /**
   * Translates the string into the corresponding string in the
   * national language, if available.
   *
   * The real contents of the string is in the argument fallback,
   * but the meaning of it is coded into the argument index.
   * In some cases you'll need this function, when english is
   * too ambiguous to express it.
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

  /**
   * Used to get the correct, translated singular or plural of a
   * word.
   * @param singular the singular form of the word, for example "file".
   * @param plural the plural form of the word. Must contain a "%n" that will
   *               be replaced by the number @n, for example "%n files"
   * @param n the number
   * @return the correct singular or plural for the selected language,
   *         depending on n
   */
  QString translate( const char *singular, const char *plural,
		     unsigned long n) const;

  /**
   * Changes the current encoding.
   *
   * @param mibEnum The mib of the preferred codec
   *
   * @return True on success.
   */
  bool setEncoding(int mibEnum);

  /**
   * Changes the current language. The current language will be left
   * unchanged if failed. It will force a reload of the country specific
   * configuration as well.
   *
   * @param language The language code.
   *
   * @return True on success.
   */
  bool setLanguage(const QString & language);

  /**
   * Changes the list of prefed languages for the locale. The first valid
   * language in the list will be used, or the default (en_US) language
   * will be used if non of the specified languages were available.
   *
   * @param languages The list of language codes.
   *
   * @return True if one of the specified languages were used.
   */
  bool setLanguage(const QStringList & languages);

  /**
   * Changes the current country. The current country will be left
   * unchanged if failed. It will force a reload of the country specific
   * configuration.
   *
   * @param country The ISO 3166 country code.
   *
   * @return True on success.
   */
  bool setCountry(const QString & country);

  /**
   * Various positions for where to place the positive or negative
   * sign when they are related to a monetary value.
   */
  enum SignPosition { ParensAround = 0, BeforeQuantityMoney = 1,
		      AfterQuantityMoney = 2,
		      BeforeMoney = 3, AfterMoney = 4 };

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
   * Given a double, converts that to a numeric string containing
   * the localized monetary equivalent.
   *
   * e.g. given 123456, return "$ 123,456.00".
   *
   * @param num The number we want to format
   * @param currency The currency symbol you want.
   * @param digits Number of fractional digits, or -1 for the default
   *               value
   *
   * @return The number of money as a localized string
   * @see fracDigits()
   */
  QString formatMoney(double num,
		      const QString & currency = QString::null,
		      int digits = -1) const;

  /**
   * Given a double, converts that to a numeric string containing
   * the localized numeric equivalent.
   *
   * e.g. given 123456.78, return "123,456.78" (for some European country).
   * If precision isn't specified, 2 is used.
   *
   * @param num The number to convert
   * @param precision Number of fractional digits used.
   *
   * @return The number as a localized string
   */
  QString formatNumber(double num, int precision = -1) const;

  /**
   * Use this to determine whether nouns are declined in
   * locale's language. This property should remain
   * read-only (no setter function)
   *
   * @return If nouns are declined
   * @since 3.1
   */
   bool nounDeclension() const;

  /**
   * Returns a string formatted to the current locale's conventions
   * regarding dates.
   *
   * @param pDate The date to be formated.
   * @param shortFormat True for non text dates.
   *
   * @return The date as a string
   */
  QString formatDate(const QDate &pDate, bool shortFormat = false) const;

  /**
   * Use this to determine whether in dates a possessive form of month
   * name is preferred ("of January" rather than "January")
   *
   * @return If possessive form should be used
   * @since 3.1
  */
  bool dateMonthNamePossessive() const;

  /**
   * Returns a string formatted to the current locale's conventions
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
   * @deprecated
   *
   * Please use the @ref weekStartDay method instead.
   *
   * Use this to determine if the user wants the week to start on Monday.
   *
   * @return true if the week starts on Monday
   */
  bool weekStartsMonday() const; //### remove for KDE 4.0

  /**
   * Use this to determine which day is the first day of the week.
   *
   * @return an integer (Monday=1..Sunday=7)
   * @since 3.1
   */
  int weekStartDay() const;

  /**
   * Returns a string containing the name of the month name.
   *
   * @param i the month number of the year starting at 1/January.
   * @param shortName we will return the short version of the string.
   *
   * @return The name of the month
   */
  QString monthName(int i, bool shortName = false) const;

  /**
   * Returns a string containing the possessive form of the month name.
   * ("of January", "of February", etc.)
   * It's needed in long format dates in some languages.
   *
   * @param i the month number of the year starting at 1/January.
   * @param shortName we will return the short version of the string.
   *
   * @return The possessive form of the name of the month
   * @since 3.1
  */
  QString monthNamePossessive(int i, bool shortName = false) const;

  /**
   * Returns a string containing the name of the week day.
   *
   * @param i the day number of the week starting at 1/Monday.
   * @param shortName we will return the short version of the string.
   *
   * @return The name of the day
   */
  QString weekDayName(int i, bool shortName = false) const;

  /**
   * Returns a string formated to the current locale's conventions
   * regarding both date and time.
   *
   * @param pDateTime The date and time to be formated.
   * @param shortFormat using the short date format.
   * @param includeSeconds using the short date format.
   *
   * @return The date and time as a string
   */
  QString formatDateTime(const QDateTime &pDateTime,
			 bool shortFormat = true,
			 bool includeSecs = false) const;

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
   * Converts a localized date string to a QDate.
   * The bool pointed by ok will be invalid if the date entered was not valid.
   *
   * @param str the string we want to convert.
   * @param ok the boolean that is set to false if it's not a valid date.
   *           If @p ok is 0, it will be ignored
   *
   * @return The string converted to a QDate
   */
  QDate readDate(const QString &str, bool* ok = 0) const;

  /**
   * Converts a localized date string to a QDate, using the specified format.
   * You will usually not want to use this method.
   */
  QDate readDate( const QString &intstr, const QString &fmt, bool* ok = 0) const;

  /**
   * Converts a localized time string to a QTime.
   * The bool pointed by ok will be false if the time entered was not valid.
   *
   * @param str the string we want to convert.
   * @param ok the boolean that is set to false if it's not a valid time.
   *           If @p ok is 0, it will be ignored
   *
   * @return The string converted to a QTime
   */
  QTime readTime(const QString &str, bool* ok = 0) const;

  /**
   * Returns the language used by this object. The domain AND the
   * library translation must be available in this language.
   * "en_US" is default, if no other available.
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
   * Returns the preferred languages as ISO 639-1 codes. This means
   * that information about country is removed. If the internal language
   * code might be represented by more than one 639-1 code, they will all be
   * listed (but only once).
   *
   * If the selected languages are "nn, nb, pt_BR", you will get:
   * "nn, no, nb, pt".
   *
   * @return List of language codes
   *
   * @see languageList
   */
  QStringList languagesTwoAlpha() const;

  /**
   * Returns the languages selected by user. The codes returned here is the
   * internal language codes.
   *
   * @return List of language codes
   *
   * @see languagesTwoAlpha
   */
  QStringList languageList() const;

  /**
   * Returns the user's preferred encoding.
   *
   * @return The name of the preferred encoding
   *
   * @see codecForEncoding
   * @see encodingMib
   */
  const char * encoding() const;

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
   * Changes the current date format.
   *
   * The format of the date is a string which contains variables that will
   * be replaced:
   * @li %Y with the century (e.g. "19" for "1984")
   * @li %y with the lower 2 digits of the year (e.g. "84" for "1984")
   * @li %n with the month (January="1", December="12")
   * @li %m with the month with two digits (January="01", December="12")
   * @li %e with the day of the month (e.g. "1" on the first of march)
   * @li %d with the day of the month with two digits(e.g. "01" on the first of march)
   * @li %b with the short form of the month (e.g. "Jan" for January)
   * @li %a with the short form of the weekday (e.g. "Wed" for Wednesday)
   * @li %A with the long form of the weekday (e.g. "Wednesday" for Wednesday)
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
   * @li %Y with the century (e.g. "19" for "1984")
   * @li %y with the lower 2 digits of the year (e.g. "84" for "1984")
   * @li %n with the month (January="1", December="12")
   * @li %m with the month with two digits (January="01", December="12")
   * @li %e with the day of the month (e.g. "1" on the first of march)
   * @li %d with the day of the month with two digits(e.g. "01" on the first of march)
   * @li %b with the short form of the month (e.g. "Jan" for January)
   * @li %a with the short form of the weekday (e.g. "Wed" for Wednesday)
   * @li %A with the long form of the weekday (e.g. "Wednesday" for Wednesday)
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
   * @since 3.1
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
   * Everything else in the format string will be taken as is.
   * For example, 5.23pm with the format "%H:%M" results
   * in "17:23".
   *
   * @param format The new time format
   */
  void setTimeFormat(const QString & format);

  /**
   * @deprecated
   *
   * Please use @ref setWeekStartDay instead.
   *
   * Changes how KLocale defines the first day in week.
   *
   * @param start True if Monday is the first day in the week
   */
  void setWeekStartsMonday(bool start); //### remove for KDE 4.0
  /**
   * Changes how KLocale defines the first day in week.
   *
   * @param day first day of the week (Monday=1..Sunday=7) as integer
   * @since 3.1
   */
  void setWeekStartDay(int day);
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
   * @param digits The default number of digits to use.
   */
  void setFracDigits(int digits);
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
   * Changes the current currency symbol.
   *
   * @param symbol The new currency symbol
   */
  void setCurrencySymbol(const QString & symbol);

  /**
   * Returns the preferred page size for printing.
   *
   * @return The preferred page size, cast it to QPrinter::PageSize
   */
  int pageSize() const;

  /**
   * Changes the preferred page size when printing.
   *
   * @param The new preferred page size in the format QPrinter::PageSize
   */
  void setPageSize(int paperFormat);

  /**
   * The Metric system will give you information in mm, while the
   * Imperial system will give you information in inches.
   */
  enum MeasureSystem { Metric, Imperial };

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
   * Adds another catalogue to search for translation lookup.
   * This function is useful for extern libraries and/or code,
   * that provides its own messages.
   *
   * If the catalogue does not exist for the chosen language,
   * it will be ignored and en_US will be used.
   *
   * @param catalogue The catalogue to add.
   */
  void insertCatalogue(const QString& catalogue);

  /**
   * Removes a catalog for translation lookup.
   * @param catalogue The catalogue to remove.
   * @see insertCatalogue()
   */
  void removeCatalogue(const QString &catalogue);

  /**
   * Sets the active catalog for translation lookup.
   * @param catalogue The catalogue to activate.
   */
  void setActiveCatalogue(const QString &catalogue);

  /**
   * Translates a message as a QTranslator is supposed to.
   * The parameters are similar to i18n(), but the result
   * value has other semantics (it can be QString::null)
   * @since 3.1
   **/
  QString translateQt(const char *context,
		      const char *sourceText,
		      const char *message) const;

  /**
   * Returns list of all known ISO 639-1 codes.
   * @return a list of all language codes
   * @since 3.1
   */
  QStringList allLanguagesTwoAlpha() const;

  /**
   * Convert a ISO 639-1 code to a human readable form.
   * @param code the language ISO 639-1 code
   * @return the human readable form
   * @since 3.1
   */
  QString twoAlphaToLanguageName(const QString &code) const;

  /**
   * Returns list of all known country codes.
   * @return a list of all country codes
   * @since 3.1
   */
  QStringList allCountriesTwoAlpha() const;

  /**
   * Convert a country code to a human readable form.
   * @param code the country code
   * @return the human readable form of the country name
   * @since 3.1
   */
  QString twoAlphaToCountryName(const QString &code) const;

  /**
   * Returns the parts of the parameter str understood as language setting
   * the format is language_COUNTRY.charset
   *
   * @param str The string to split.
   * @param language This will be set to the language part of the string.
   * @param country This will be set to the country part of the string.
   * @param charset This will be set to the charset part of the string.
   */
  static void splitLocale(const QString & str,
			  QString & language,
			  QString & country,
			  QString & charset);

  /**
   * Use this to as main catalogue for *all* KLocales, if not the appname
   * will be used. This function is best to be the very first instruction
   * in your program's main function as it only has an effect before the
   * first KLocale object is created (and this is in common KDE applications
   * quite early).
   *
   * @param catalogue Catalogue to override all other main catalogues.
   */
  static void setMainCatalogue(const char *catalogue);

  /**
   * Finds localized resource in resourceDir( rtype ) + <lang> + fname.
   *
   * @param fname relative path to find
   * @param rtype resource type to use
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
   * @internal Called from KConfigBackend to initialize language.
   */
  static QString _initLanguage(KConfigBase *config);

#ifdef KDE_NO_COMPAT
private:
#endif

  /**
   * @deprecated
   * use formatMoney(double)
   */
  QString formatMoney(const QString &numStr) const;

  /**
   * @deprecated
   * use formatNumber(double)
   */
  QString formatNumber(const QString &numStr) const;

  /**
   * @deprecated
   * Use languageList()
   *
   * @return String containing language codes separated by colons
   */
  QString languages() const;

  /**
   * @deprecated
   * @return True
   */
  bool setCharset(const QString & charset) { return true; }

  /**
   * @deprecated
   * @see encoding
   */
  QString charset() const { return "UTF-8"; }

protected:
  /**
   * @internal Creates a KLocale object for KGlobal and inits the locale
   * pointer.
   */
  static void initInstance();

private:
  /**
   * @internal Inits the localization part of the instance with the config
   * object.
   *
   * @param config The configuration object used for init.
   */
  void initFormat(KConfig *config);

  /**
   * @internal Inits the language part of the instance with the given config
   * object. It should be valid and contain the global entries.
   *
   * @param config The configuration object used for init
   * @param useEnv True if we should use environment variables
   */
  void initLanguage(KConfig * config, bool useEnv);

  /**
   * @internal Figures out which encoding the user prefers.
   *
   * @param config The configuration object used for init
   */
  void initEncoding(KConfig * config);

  /**
   * @internal Figures out which catalogues to use.
   *
   * @param catalogue The name of the main catalogue
   */
  void initCatalogue(const QString & catalogue);

  /**
   * @internal Figures out which encoding the user prefers for filenames
   * and sets up the appropriate QFile encoding and decoding functions.
   */
  void initFileNameEncoding(KConfig *config);

  /**
   * @internal A QFile filename encoding function (QFile::encodeFn).
   */
  static QCString encodeFileNameUTF8( const QString & fileName );

  /**
   * @internal QFile filename decoding function (QFile::decodeFn).
   */
  static QString decodeFileNameUTF8( const QCString & localFileName );

  /**
   * @internal function used by readTime(const QString &) const.
   */
  QTime readTime(const QString &str, bool seconds, bool *ok) const;

  /**
   * @internal function used by readDate(const QString &) const.
   */
  QDate readDate(const QString &str, bool shortFormat, bool *ok) const;

  /**
   * @internal Changes the file name of the catalogue to the correct
   * one.
   */
  void initCatalogue( KCatalogue & catalogue );

  /**
   * @internal Reads the language and format configuration form disk.
   */
  void doBindInit();

  /**
   * @internal Ensures that the format configuration is read.
   */
  void doFormatInit() const;

  /**
   * @internal Reads the format configuration from disk.
   */
  void initFormat();

  /**
   * @internal function used by the two translate versions
   */
  QString translate_priv(const char *index,
			 const char *text,
			 const char ** original = 0) const;

  /**
   * @internal function used to determine if we are using the en_US translation
   */
  bool useDefaultLanguage() const;

  /**
   * @internal Checks if the specified language is installed
   */
  bool isLanguageInstalled(const QString & language) const;

  /**
   * @internal Retrieves the file name of the catalogue, or QString::null
   *           if not found.
   */
  static QString catalogueFileName(const QString & language,
				   const KCatalogue & catalogue);

private:
  // Numbers and money
  QString m_decimalSymbol;
  QString m_thousandsSeparator;
  QString m_currencySymbol;
  QString m_monetaryDecimalSymbol;
  QString m_monetaryThousandsSeparator;
  QString m_positiveSign;
  QString m_negativeSign;
  int m_fracDigits;
  SignPosition m_positiveMonetarySignPosition;
  SignPosition m_negativeMonetarySignPosition;

  // Date and time
  QString m_timeFormat;
  QString m_dateFormat;
  QString m_dateFormatShort;

  QString m_language;
  QString m_country;

  bool m_weekStartsMonday; //### remove for KDE 4.0
  bool m_positivePrefixCurrencySymbol;
  bool m_negativePrefixCurrencySymbol;

  KLocalePrivate *d;
};

#endif
