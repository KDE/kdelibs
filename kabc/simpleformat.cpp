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
#include <kstandarddirs.h>
#include <kdebug.h>

#include "addressbook.h"

#include "simpleformat.h"

using namespace KABC;

bool SimpleFormat::load( AddressBook *addressBook, Resource *resource, const QString &fileName )
{
  kdDebug(5700) << "SimpleFormat::load(): " << fileName << endl;

  KSimpleConfig cfg( fileName );
  
  QStringList uids = cfg.groupList();

  QStringList::ConstIterator it;
  for( it = uids.begin(); it != uids.end(); ++it ) {
    if ( (*it) == "<default>" )
      continue;

    cfg.setGroup( *it );

    Addressee addr;
    addr.setResource( resource );
    addr.setUid( *it );
    addr.setName( cfg.readEntry( "name" ) );
    addr.setFormattedName( cfg.readEntry( "formattedName" ) );

    // emails
    {
      QStringList emails = cfg.readListEntry( "emails" );
      QStringList::ConstIterator it;
      bool preferred = true;
      for( it = emails.begin(); it != emails.end(); ++it ) {
        addr.insertEmail( (*it), preferred );
        preferred = false;
      }
    }

    // phonenumbers
    {
      QStringList phoneNumbers = cfg.readListEntry( "phonenumbers" );
      QStringList::ConstIterator it;
      for( it = phoneNumbers.begin(); it != phoneNumbers.end(); ++it ) {
        PhoneNumber n;
        n.setNumber( cfg.readEntry( "phonenumber" + (*it) ) );
        n.setType((*it).toInt());
        addr.insertPhoneNumber( n );
      }
    }

    addressBook->insertAddressee( addr );
  }

  return true;
}

bool SimpleFormat::save( AddressBook *addressBook, Resource *resource, const QString &fileName )
{
  kdDebug(5700) << "SimpleFormat::save(): " << fileName << endl;

  QFile::remove( locateLocal("config", fileName) );

  KSimpleConfig cfg( fileName );

  AddressBook::Iterator it;
  for ( it = addressBook->begin(); it != addressBook->end(); ++it ) {
    if ( (*it).resource() != resource && (*it).resource() != 0 )
	continue;

    cfg.setGroup( (*it).uid() );

    cfg.writeEntry( "name", (*it).name() );
    cfg.writeEntry( "formattedName", (*it).formattedName() );

    // emails
    {
      cfg.writeEntry( "emails", (*it).emails() );
    }

    // phonenumbers
    {
      QStringList phoneNumberList;
      PhoneNumber::List phoneNumbers = (*it).phoneNumbers();
      PhoneNumber::List::ConstIterator it;
      for( it = phoneNumbers.begin(); it != phoneNumbers.end(); ++it ) {
        cfg.writeEntry( "phonenumber" + QString::number( int((*it).type()) ),
                      (*it).number() );
        phoneNumberList.append( QString::number( int((*it).type()) ) );
      }
      cfg.writeEntry( "phonenumbers", phoneNumberList );
    }
  }

  return true;
}
