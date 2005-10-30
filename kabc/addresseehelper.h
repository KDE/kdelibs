/*
    This file is part of the KDE libraries
    Copyright (C) 2003 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation, version 2.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KABC_ADDRESSEEHELPER_H
#define KABC_ADDRESSEEHELPER_H

#include <qobject.h>
#include <qstringlist.h>

#include <dcopobject.h>

#include <set>

namespace KABC {

/**
 * This singleton class stores static data, which is shared
 * by all Addressee objects. It maintains three lists of
 * strings, which can be queried using this class:
 *
 * - a list of honoric prefixes, like "Mrs.", "Prof." etc,
 *   see containsTitle()
 * - a list of inclusions, such as "van" or "de", see
 *   containsPrefix()
 * - a list of honoric suffixes, such as "I" or "Jr.", see
 *   containsSuffix()
 *
 * All of these lists have a hardcoded and a configurable
 * part. The configurable part is found in @c kabcrc, group
 * @c General, fields @c Prefixes, @c Inclusions, and
 * @c Suffixes.
 *
 * In addition to the above, this class stores one conveniece
 * setting: it stores whether or not a single name component
 * should be interpreted as a family name (see
 * tradeAsFamilyName()). The corresponding configuration
 * field is @c TradeAsFamilyName.
 */
class KABC_EXPORT AddresseeHelper : public QObject, public DCOPObject
{
  K_DCOP
        
  public:
    /**
     * Singleton interface to this class
     *
     * @return a pointer to the unique instance of this class.
     */
    static AddresseeHelper *self();

    /**
     * Queries the list of honoric prefixes.
     *
     * @param title the honoric prefix to search for
     * @return @c true, if @p title was found in the list,
     *         @c false otherwise
     */
    bool containsTitle( const QString& title ) const;

    /**
     * Queries the list of inclusions.
     *
     * @param prefix the inclusion to search for
     * @return @c true, if @p prefix was found in the list,
     *         @c false otherwise
     */
    bool containsPrefix( const QString& prefix ) const;

    /**
     * Queries the list of honoric suffixes.
     *
     * @param suffix the honoric suffix to search for
     * @return @c true, if @p suffix was found in the list,
     *         @c false otherwise
     */
    bool containsSuffix( const QString& suffix ) const;

    /**
     * Returns whether or not a single name component should
     * be interpreted as a family name.
     *
     * @return @c true if single name component is a family name,
     *         @c false otherwise.
     */
    bool tradeAsFamilyName() const;

  k_dcop:
    /**
     * Recreates the static data and reparses the configuration.
     */
    ASYNC initSettings();

  private:
    AddresseeHelper();

    static void addToSet( const QStringList& list,
                          std::set<QString>& container );
    std::set<QString> mTitles;
    std::set<QString> mPrefixes;
    std::set<QString> mSuffixes;
    bool mTradeAsFamilyName;

    static AddresseeHelper *s_self;
};

}

#endif
