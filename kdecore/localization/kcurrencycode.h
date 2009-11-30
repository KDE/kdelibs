/*
    Copyright (c) 2009 John Layt <john@layt.net>

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

#ifndef KCURRENCYCODE_H
#define KCURRENCYCODE_H

#include <kdecore_export.h>

#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>

class QDate;
class QStringList;
class QFileInfo;

class KCurrencyCodePrivate;

/**
 * @since 4.4
 *
 * This is a class to implement the ISO 4217 Currency Code standard
 *
 * @b license GNU-LGPL v.2 or later
 *
 * @see KLocale
 *
 * @author John Layt <john@layt.net>
 */
class KDECORE_EXPORT KCurrencyCode
{
public:
    /**
     * The Status of the Currency
     *
     * @see CurrencyStatusFlags
     * @see currencyStatus()
     */
    enum CurrencyStatus {
        ActiveCurrency     = 0x01, /**< Currency is currently in use */
        SuspendedCurrency  = 0x02, /**< Currency is not currently in use but has not been replaced */
        ObsoleteCurrency   = 0x04  /**< Currency is no longer in use and has been replaced */
    };
    Q_DECLARE_FLAGS( CurrencyStatusFlags, CurrencyStatus )

    /**
     * Constructs a KCurrencyCode for a given ISO Currency Code.
     *
     * If the supplied Currency Code is not known then the KCurrencyCode will return isValid() == false
     *
     * @param isoCurrencyCode the ISO Currency Code to construct, defaults to USD
     * @param language the language to use for translations, default to the Locale language
     *
     */
    explicit KCurrencyCode( const QString &isoCurrencyCode, const QString &language = QString() );

    /**
     * Constructs a KCurrencyCode for a given config file and Language.
     *
     * Note that any translations must be supplied in the config file, none will be provided.
     *
     * If the supplied config file is not valid then the KCurrencyCode will return isValid() == false
     *
     * @param currencyCodeFile the ISO Currency Code to construct, defaults to USD
     * @param language the language to use for translations, default to the Locale language
     *
     */
    explicit KCurrencyCode( const QFileInfo &currencyCodeFile, const QString &language = QString() );

    /**
     * Copy Constructor
     *
     * @param rhs KCurrencyCode to copy
     *
     */
    KCurrencyCode( const KCurrencyCode &rhs );

    /**
     * Destructor.
     */
    virtual ~KCurrencyCode();
    
    /**
     * Assignment operator
     *
     * @param rhs KCurrencyCode to assign
     *
     */
    KCurrencyCode& operator=( const KCurrencyCode &rhs );

    /**
     * Return the ISO 4217 Currency Code in Alpha 3 format, e.g. USD
     *
     * @return the ISO Currency Code
     *
     * @see isoCurrencyCodeNumeric()
     */
    QString isoCurrencyCode() const;

    /**
     * Return the ISO 4217 Currency Code in Numeric 3 format, e.g. 840
     *
     * @return the ISO Currency Code
     *
     * @see isoCurrencyCode()
     */
    QString isoCurrencyCodeNumeric() const;

    /**
     * Return translated Currency Code Name in a standard display format
     * e.g. United States Dollar
     *
     * @return the display Currency Code Name
     *
     * @see isoName()
     */
    QString name() const;

    /**
     * Return untranslated official ISO Currency Code Name
     *
     * This name is not translated and should only be used where appropriate.
     * For displaying the name to a user, use name() instead.
     *
     * @return the official ISO Currency Code Name
     *
     * @see name()
     */
    QString isoName() const;

    /**
     * Return Currency Status for the currency, if Active, Suspended or Obsolete
     *
     * @return the Currency Status
     *
     * @see CurrencyStatus
     */
    CurrencyStatus status() const;

    /**
     * Return the date the currency was introduced
     *
     * @return the date the currency was introduced
     *
     * @see status()
     * @see dateSuspended()
     * @see dateWithdrawn()
     */
    QDate dateIntroduced() const;

    /**
     * Return the date the currency was suspended
     *
     * @return the date the currency was suspended, QDate() if active
     *
     * @see status()
     * @see dateIntroduced()
     * @see dateWithdrawn()
     */
    QDate dateSuspended() const;

    /**
     * Return the date the currency was withdrawn from circulation
     *
     * @return the date the currency was withdrawn, QDate() if active
     *
     * @see status()
     * @see dateIntroduced()
     * @see dateSuspended()
     */
    QDate dateWithdrawn() const;

    /**
     * Return a list of valid Symbols for the Currency in order of preference
     *
     * This list will normally contain the Default and Unambiguous symbols and the ISO Currency Code
     *
     * @return list of Currency Symbols
     *
     * @see defaultSymbol()
     * @see unambiguousSymbol()
     */
    QStringList symbolList() const;

    /**
     * Return the default Symbol for the Currency, e.g. $ or £
     *
     * @return the default Currency Symbol
     *
     * @see symbols()
     * @see unambiguousSymbol()
     */
    QString defaultSymbol() const;

    /**
     * Return the unambiguous Symbol for the Currency, e.g. US$ or NZ$
     *
     * @return the unambiguous Currency Symbol
     *
     * @see symbols()
     * @see defaultSymbol()
     */
    QString unambiguousSymbol() const;

    /**
     * Return if the Currency has subunits or not,
     * e.g. USD has cents, VUV has none
     *
     * @return true if the Currency has subunits
     *
     * @see hasSubunitsInCirculation()
     * @see subunitName()
     * @see subunitSymbol()
     * @see subunitsPerUnit()
     */
    bool hasSubunits() const;

    /**
     * Return if the Currency has subunits in circulation,
     * e.g. JPY has sen but these are no longer used due to inflation
     *
     * @return true if the Currency has subunits in circulation
     *
     * @see hasSubunits()
     */
    bool hasSubunitsInCirculation() const;

    /**
     * Return the Currency subunit symbol if it has one
     * e.g. ¢ for USD cent
     *
     * @return the currency subunit symbol
     *
     * @see hasSubunits()
     */
    QString subunitSymbol() const;

    /**
     * Return the number of subunits in every unit, e.g. 100 cents in the dollar
     *
     * @return number of subunits per unit, 0 if no subunits
     *
     * @see hasSubunits()
     */
    int subunitsPerUnit() const;

    /**
     * Return the number of decimal places required to display the currency subunits
     *
     * @return number of decimal places
     */
    int decimalPlaces() const;

    /**
     * Return a list of countries known to be using the currency
     *
     * @return list of ISO Country Codes using the currency
     */
    QStringList countriesUsingCurrency() const;

    /**
     * Return if the currency object loaded/initialised correctly
     *
     * @return true if valid KCurrencyCode object
     */
    bool isValid() const;

    /**
     * Return if a given Currency Code is supported in KDE.
     * Optionally validate if an Active, Suspended, or Obsolete currency, default is if any.
     *
     * @param currencyCode the Currency Code to validate
     * @param currencyStatus the CurrencyStatus to validate
     *
     * @return true if valid currency code
     */
    static bool isValid( const QString &currencyCode, CurrencyStatusFlags currencyStatus =
                                                      CurrencyStatusFlags( ActiveCurrency |
                                                                           SuspendedCurrency |
                                                                           ObsoleteCurrency ) );

    /**
     * Provides list of all known ISO Currency Codes.
     *
     * Use currencyCodeToName(currencyCode) to get human readable, localized currency names.
     *
     * By default returns all Active, Suspended and Obsolete currencies, set the currencyStatus
     * flags as appropriate to return required status currencies
     *
     * @param currencyStatus which status currencies to return
     *
     * @return a list of all ISO Currency Codes
     *
     * @see currencyCodeToName
     */
    static QStringList allCurrencyCodesList( CurrencyStatusFlags currencyStatus =
                                             CurrencyStatusFlags( ActiveCurrency |
                                                                  SuspendedCurrency |
                                                                  ObsoleteCurrency ) );

    /**
     * Convert a known ISO Currency Code to a human readable, localized form.
     *
     * If an unknown Currency Code is supplied, empty string is returned;
     * this will never happen if the code has been obtained by one of the
     * KCurrencyCode methods.
     *
     * @param currencyCode the ISO Currency Code
     * @param language the language to use for translations, default to the Locale language
     *
     * @return the human readable and localized form of the Currency name
     *
     * @see currencyCode
     * @see allCurrencyCodesList
     */
    static QString currencyCodeToName( const QString &currencyCode, const QString &language = QString() );


private:
    QSharedDataPointer<KCurrencyCodePrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( KCurrencyCode::CurrencyStatusFlags )


#endif // KCURRENCYCODE_H
