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

#include <qapplication.h>

#include <kconfig.h>
#include <klocale.h>

#include "addresseehelper.h"

using namespace KABC;

AddresseeHelper * AddresseeHelper::s_self;

// static
AddresseeHelper *AddresseeHelper::self()
{
  if ( !s_self )
    s_self = new AddresseeHelper();

  return s_self;
}

AddresseeHelper::AddresseeHelper()
  : QObject( qApp ),
    DCOPObject( "KABC::AddresseeHelper" )
{
  initSettings();

  connectDCOPSignal( "kaddressbook", "KABC::AddressBookConfig",
                     "changed()", "initSettings()", false );
}

// static
void AddresseeHelper::addToSet( const QStringList& list,
                                QStringList& container )
{
  QStringList::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    if ( !(*it).isEmpty() )
      container.append( *it );
  }
}

void AddresseeHelper::initSettings()
{
  mTitles.clear();
  mSuffixes.clear();
  mPrefixes.clear();

  mTitles.append( i18n( "Dr." ) );
  mTitles.append( i18n( "Miss" ) );
  mTitles.append( i18n( "Mr." ) );
  mTitles.append( i18n( "Mrs." ) );
  mTitles.append( i18n( "Ms." ) );
  mTitles.append( i18n( "Prof." ) );

  mSuffixes.append( i18n( "I" ) );
  mSuffixes.append( i18n( "II" ) );
  mSuffixes.append( i18n( "III" ) );
  mSuffixes.append( i18n( "Jr." ) );
  mSuffixes.append( i18n( "Sr." ) );

  mPrefixes.append( "van" );
  mPrefixes.append( "von" );
  mPrefixes.append( "de" );

  KConfig config( "kabcrc", true, false ); // readonly, no kdeglobals
  config.setGroup( "General" );

  addToSet( config.readListEntry( "Prefixes" ),   mTitles );
  addToSet( config.readListEntry( "Inclusions" ), mPrefixes );
  addToSet( config.readListEntry( "Suffixes" ),   mSuffixes );
}

bool AddresseeHelper::containsTitle( const QString& title ) const
{
  return mTitles.find( title ) != mTitles.end();
}

bool AddresseeHelper::containsPrefix( const QString& prefix ) const
{
  return mPrefixes.find( prefix ) != mPrefixes.end();
}

bool AddresseeHelper::containsSuffix( const QString& suffix ) const
{
  return mSuffixes.find( suffix ) != mSuffixes.end();
}
