/*
    This file is part of libkabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <qfile.h>

#include <ksimpleconfig.h>
#include <kdebug.h>

#include "addressbook.h"

#include "simpleformat.h"

using namespace KABC;

bool SimpleFormat::load( AddressBook *addressBook, const QString &fileName )
{
  kdDebug(5700) << "SimpleFormat::load(): " << fileName << endl;

  KSimpleConfig cfg( fileName );
  
  QStringList uids = cfg.groupList();
  QStringList::ConstIterator it;
  for( it = uids.begin(); it != uids.end(); ++it ) {
    if ( (*it) == "<default>" ) continue;
    cfg.setGroup( *it );
    Addressee a;
    a.setUid( *it );
    a.setName( cfg.readEntry( "name" ) );
    a.setFormattedName( cfg.readEntry( "formattedName" ) );
    a.insertEmail( cfg.readEntry( "email" ) );
#if 0
    QStringList phoneNumbers = cfg.readListEntry( "phonenumbers" );
    QStringList::ConstIterator it2;
    for( it2 = phoneNumbers.begin(); it2 != phoneNumbers.end(); ++it2 ) {
      PhoneNumber n;
      n.setNumber( cfg.readEntry( "phonenumber" + (*it2) ) );
      n.setType( PhoneNumber::Type((*it2).toInt()));
      a.insertPhoneNumber( n );
    }
#endif
    addressBook->insertAddressee( a );
  }

  return true;
}

bool SimpleFormat::save( AddressBook *addressBook, const QString &fileName )
{
  kdDebug(5700) << "SimpleFormat::save(): " << fileName << endl;

  QFile::remove( fileName );

  KSimpleConfig cfg( fileName );

  AddressBook::Iterator it;
  for ( it = addressBook->begin(); it != addressBook->end(); ++it ) {
    cfg.setGroup( (*it).uid() );
    cfg.writeEntry( "name", (*it).name() );
    cfg.writeEntry( "formattedName", (*it).formattedName() );
    cfg.writeEntry( "email", (*it).preferredEmail() );

#if 0
    QStringList phoneNumberList;
    PhoneNumber::List phoneNumbers = (*it).phoneNumbers();
    PhoneNumber::List::ConstIterator it2;
    for( it2 = phoneNumbers.begin(); it2 != phoneNumbers.end(); ++it2 ) {
      cfg.writeEntry( "phonenumber" + QString::number( int((*it2).type()) ),
                      (*it2).number() );
      phoneNumberList.append( QString::number( int((*it2).type()) ) );
    }
    cfg.writeEntry( "phonenumbers", phoneNumberList );
#endif
  }

  return true;
}
