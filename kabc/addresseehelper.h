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

/**
  static data, shared by ALL addressee objects
*/

namespace KABC {

class KABC_EXPORT AddresseeHelper : public QObject, public DCOPObject
{
  K_DCOP
        
  public:
    static AddresseeHelper *self();

    bool containsTitle( const QString& title ) const;
    bool containsPrefix( const QString& prefix ) const;
    bool containsSuffix( const QString& suffix ) const;
    bool tradeAsFamilyName() const;

  k_dcop:
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
