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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KABC_ADDRESSEEHELPER_H
#define KABC_ADDRESSEEHELPER_H

#include <qobject.h>
#include <qvaluelist.h>
#include <qstringlist.h>

#include <dcopobject.h>

/**
  static data, shared by ALL addressee objects
*/

namespace KABC {

class AddresseeHelper : public QObject, public DCOPObject
{
  K_DCOP
        
  public:
    static AddresseeHelper *self();

    bool containsTitle( const QString& title ) const;
    bool containsPrefix( const QString& prefix ) const;
    bool containsSuffix( const QString& suffix ) const;

  k_dcop:
    ASYNC initSettings();

  private:
    AddresseeHelper();

    static void addToSet( const QStringList& list, QStringList& container );
    QStringList mTitles;
    QStringList mPrefixes;
    QStringList mSuffixes;

    static AddresseeHelper *s_self;
};

}

#endif
