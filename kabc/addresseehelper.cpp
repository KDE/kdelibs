/* This file is part of the KDE libraries
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

#include "addresseehelper.h"

#include <qapplication.h>

#include <kconfig.h>
#include <klocale.h>

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

    connectDCOPSignal( "kaddressbook",
                       "KABC::AddressBookConfig",
                       "changed()",
                       "initSettings()",
                       false
        );
}

// static
void AddresseeHelper::addToSet( const QStringList& list, std::set<QString>& container )
{
    QStringList::ConstIterator it;
    for ( it = list.begin(); it != list.end(); ++it )
    {
        if ( !(*it).isEmpty() )
            container.insert( *it );
    }
}

void AddresseeHelper::initSettings()
{
    titles.clear();
    suffixes.clear();
    prefixes.clear();

    titles.insert( i18n( "Dr." ) );
    titles.insert( i18n( "Miss" ) );
    titles.insert( i18n( "Mr." ) );
    titles.insert( i18n( "Mrs." ) );
    titles.insert( i18n( "Ms." ) );
    titles.insert( i18n( "Prof." ) );

    suffixes.insert( i18n( "I" ) );
    suffixes.insert( i18n( "II" ) );
    suffixes.insert( i18n( "III" ) );
    suffixes.insert( i18n( "Jr." ) );
    suffixes.insert( i18n( "Sr." ) );

    prefixes.insert( "van" );
    prefixes.insert( "von" );
    prefixes.insert( "de" );

    KConfig config( "kabcrc", true, false ); // readonly, no kdeglobals
    config.setGroup( "General" );

    addToSet( config.readListEntry( "Prefixes" ),   titles );
    addToSet( config.readListEntry( "Inclusions" ), prefixes );
    addToSet( config.readListEntry( "Suffixes" ),   suffixes );
}

bool AddresseeHelper::containsTitle( const QString& title ) const
{
    return titles.find( title ) != titles.end();
}

bool AddresseeHelper::containsPrefix( const QString& prefix ) const
{
    return prefixes.find( prefix ) != prefixes.end();
}

bool AddresseeHelper::containsSuffix( const QString& suffix ) const
{
    return suffixes.find( suffix ) != suffixes.end();
}
