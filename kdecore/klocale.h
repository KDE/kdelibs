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

class QStrList;
class QDate;
class QTime;
class QDateTime;

/*
  #ifndef klocale
  #define klocale KApplication::getKApplication()->getLocale()
  #endif
*/

/**
  *
  * KLocale provides support for country specific stuff like
  * the national language.
  *
  * KLocale supports translating, as well as specifying the format
  * for numbers, currency, time, and date.
  * 
  * @author Stephan Kulow <coolo@kde.org>, Preston Brown <pbrown@kde.org>
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
      * @param catalogue the name of the language file
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
      * There is a KDE wide message file, that contains the most
      * often used phrases, so we can avoid to duplicate the
      * translation of this phrases. If a phrase is not found
      * in the catalogue given to the constructor, it will search
      * in the system catalogue. This makes it possible to override
      * some phrases for your needs.
      * @param index the lookup text and default text, if not found
      */
    QString translate( const char *index ) const;
    
    /**
     * various positions for where to place the positive or negative
     * sign when they are related to a monetary value.
     */
    enum SignPosition { ParensAround = 0, BeforeQuantityMoney = 1,
			AfterQuantityMoney = 2,
			BeforeMoney = 3, AfterMoney = 4 };

    /**
     * Specicies what a decimal point should look like ("." or "," etc.)
     * according to the current locale or user settings.
     */
    QString decimalSymbol() const;
    
    /**
     */
    QString thousandsSeparator() const;
    
    /**
     * Specifies what the symbol denoting currency in the current locale
     * as as defined by user settings should look like.
     */
    QString currencySymbol() const;
    
    /**
     * Specifies what a decimal point should look like ("." or "," etc.)
     * for monetary values, according to the current locale or user
     * settings.
     */
    QString monetaryDecimalSymbol() const;
    
    /**
     * Specifies what a thousands separator for monetary values should
     * look like ("," or " " etc.) according to the current locale or
     * user settings.
     */
    QString monetaryThousandsSeparator() const;
    
    /**
     * Specifies what a positive sign should look like ("+", " ", etc.)
     * according to the current locale or user settings.
     */
    QString positiveSign() const;
    
    /**
     * Specifies what a negative sign should look like ("-", etc.)
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
     * Denotes where to place a positive sign in relation to a 
     * monetary value.
     *
     * @see #SignPosition
     */
    SignPosition positiveMonetarySignPosition() const;

    /**
     * Denotes where to place a negative sign in relation to a 
     * monetary value.
     *
     * @see #SignPosition
     */
    SignPosition negativeMonetarySignPosition() const;

    /**
     * Given an double, convert that to a numeric string containing
     * the localized monetary equivalent.
     * 
     * e.g. given 123456, return "$123,456".  
     */
    QString formatMoney(double num) const;

    /**
     * This function differs from the above only in that it can take
     * a QString as the argument for convenience.
     */
    QString formatMoney(const QString &numStr) const;

    /**
     * Given an double, convert that to a numeric string containing
     * the localized numeric equivalent.
     * 
     * e.g. given 123456.78, return "123.456.78" (for Europe).  
     */
    QString formatNumber(double num) const;

    /**
     * This function differs from the above only in that it can take
     * a QString as the argument for convenience.
     */
    QString formatNumber(const QString &numStr) const;

    /**
     * Return a string formatted to the current locale's conventions
     * regarding dates.
     */
    QString formatDate(const QDate &pDate) const;

    /**
     * Return a string formatted to the current locale's conventions
     * regarding times.
     */
    QString formatTime(const QTime &pTime) const;

    /**
     * Return a string formatted to the current locale's conventions
     * regarding both date and time.
     */
    QString formatDateTime(const QDateTime &pDateTime) const;

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
      * Returns the charset name used by selected locale.
      * Special file with charset name must be available
      * "us-ascii" is default 
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
       The category argument tells the setlocale() function which attributes to
       set.  The choices are:
       
       LC_COLLATE      Changes the behavior of the strcoll() and strxfrm() functions.
       LC_CTYPE        Changes the behavior of the character-handling functions:
                       isalpha(), islower(), isupper(), isprint(), ...
       LC_MESSAGES     Changes the language in which messages are displayed.
       LC_MONETARY     Changes the information returned by localeconv().
       LC_NUMERIC      Changes the radix character for numeric conversions.
       LC_TIME         Changes the behavior of the strftime() function.
       LC_ALL          Changes all of the above.
    **/
    QString getLocale(const QString& CATEGORY);

    /** 
     * returns the parts of the parameter str understood as language setting
     * the format is language_country.charset
     */ 
    static void splitLocale(const QString& str,QString& language, QString& country,
                     QString &charset);

    /** 
     * does the same as the above, just reverse
     */
    static QString mergeLocale(const QString& lang, const QString& country,
			      const QString& charset);

    /**
     * if the application can handle localized numeric and monetary
     * values, it should call this function.
     * 
     * By default, this is disabled.
     */
    void enableNumericLocale(bool on = true);

    /**
     * @return True if the application has instructed KLocale that it can
     * handle localized numeric and monetary values, if POSIX only (the
     * default), false.
     */
    bool numericLocaleEnabled() const;

    /**
     * Init the instance with the given config object. It should
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

private:
    QStrList *catalogues;
    QIntDict<QString> aliases;
    bool _inited;
    QString lang;
    QString chset;
    QString lc_numeric;
    QString lc_monetary;
    QString lc_time;
    bool numeric_enabled;
    QString langs;
    
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
    QString MonthName(int i) const;
    QString WeekDayName(int i) const;

    // Disallow assignment and copy-construction
    KLocale( const KLocale& );
    KLocale& operator= ( const KLocale& );
  
    static void initInstance();
};

QString i18n(const char *text);

#endif
