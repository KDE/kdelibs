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

#include <kaboutdata.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kabapi.h>

#include "addressbook.h"
#include "stdaddressbook.h"

using namespace KABC;

int main(int argc,char **argv)
{
  KAboutData aboutData("kab2kabc",I18N_NOOP("Kab to Kabc Converter"),"0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

  KApplication app;

  kdDebug(5700) << "Converting old-style kab addressbook to "
               "new-style kabc addressbook." << endl;

  KabAPI kab(0);
  if (kab.init() != ::AddressBook::NoError) {
    kdDebug(5700) << "Error initing kab" << endl;
    exit(1);
  }

  KABC::AddressBook *kabcBook = StdAddressBook::self();

  KabKey key;
  ::AddressBook::Entry entry;

  int num = kab.addressbook()->noOfEntries();
  
  kdDebug(5700) << "kab Addressbook has " << num << " entries." << endl;
  
  for (int i = 0; i < num; ++i) {
    if (::AddressBook::NoError != kab.addressbook()->getKey(i,key)) {
      kdDebug(5700) << "Error getting key for index " << i << " from kab." << endl;
      continue;
    }
    if (::AddressBook::NoError != kab.addressbook()->getEntry(key,entry))
    {
      kdDebug(5700) << "Error getting entry for index " << i << " from kab." << endl;
      continue;
    }

    Addressee a;

    QStringList::ConstIterator customIt;
    for( customIt = entry.custom.begin(); customIt != entry.custom.end(); ++customIt ) {
      if ( (*customIt).startsWith( "X-KABC-UID:" ) ) {
        a.setUid( (*customIt).mid( (*customIt).find( ":" ) + 1 ) );
        break;
      } else {
        int count = 0;
        a.insertCustom( "kab2kabc", QString::number( count++ ), *customIt );
      }
    }
    if( customIt == entry.custom.end() ) {
      entry.custom << "X-KABC-UID:" + a.uid();
      ::AddressBook::ErrorCode error = kab.addressbook()->change( key, entry );
      if (error != ::AddressBook::NoError) {
        kdDebug(5700) << "kab.change returned with error " << error << endl;
      } else {
        kdDebug(5700) << "Wrote back to kab uid " << a.uid() << endl;
      }
    }
    
    a.setTitle( entry.title );
    a.setFormattedName( entry.fn );
    a.setPrefix( entry.nameprefix );
    a.setGivenName( entry.firstname );
    a.setAdditionalName( entry.middlename );
    a.setFamilyName( entry.lastname );
    a.setBirthday( entry.birthday );

    QStringList::ConstIterator emailIt;
    for( emailIt = entry.emails.begin(); emailIt != entry.emails.end(); ++emailIt ) {
      a.insertEmail( *emailIt );
    }

    QStringList::ConstIterator phoneIt;
    for( phoneIt = entry.telephone.begin(); phoneIt != entry.telephone.end(); ++phoneIt ) {
      int kabType = (*phoneIt++).toInt();
      QString number = *phoneIt;
      int type = 0;
      if ( kabType == ::AddressBook::Fixed ) type = PhoneNumber::Voice;
      else if ( kabType == ::AddressBook::Mobile ) type = PhoneNumber::Cell | PhoneNumber::Voice;
      else if ( kabType == ::AddressBook::Fax ) type = PhoneNumber::Fax;
      else if ( kabType == ::AddressBook::Modem ) type = PhoneNumber::Modem;
      a.insertPhoneNumber( PhoneNumber( number, type ) );
    }

    if ( entry.URLs.count() > 0 ) {
      a.setUrl( entry.URLs.first() );
      if ( entry.URLs.count() > 1 ) {
        kdWarning() << "More than one URL. Ignoring all but the first." << endl;
      }
    }

    int noAdr = entry.noOfAddresses();
    for( int j = 0; j < noAdr; ++j ) {
      ::AddressBook::Entry::Address kabAddress;
      entry.getAddress( j, kabAddress );
      
      Address adr;
      
      adr.setStreet( kabAddress.address );
      adr.setPostalCode( kabAddress.zip );
      adr.setLocality( kabAddress.town );
      adr.setCountry( kabAddress.country );
      adr.setRegion( kabAddress.state );

      QString label;
      if ( !kabAddress.headline.isEmpty() ) label += kabAddress.headline + "\n";
      if ( !kabAddress.position.isEmpty() ) label += kabAddress.position + "\n";
      if ( !kabAddress.org.isEmpty() ) label += kabAddress.org + "\n";
      if ( !kabAddress.orgUnit.isEmpty() ) label += kabAddress.orgUnit + "\n";
      if ( !kabAddress.orgSubUnit.isEmpty() ) label += kabAddress.orgSubUnit + "\n";
      if ( !kabAddress.deliveryLabel.isEmpty() ) label += kabAddress.deliveryLabel + "\n";
      adr.setLabel( label );
      
      a.insertAddress( adr );
    }

    QString note = entry.comment;
    
    if ( !entry.user1.isEmpty() ) note += "\nUser1: " + entry.user1;
    if ( !entry.user2.isEmpty() ) note += "\nUser2: " + entry.user2;
    if ( !entry.user3.isEmpty() ) note += "\nUser3: " + entry.user3;
    if ( !entry.user4.isEmpty() ) note += "\nUser4: " + entry.user4;
    
    if ( !entry.keywords.count() == 0 ) note += "\nKeywords: " + entry.keywords.join( ", " );
    
    QStringList::ConstIterator talkIt;
    for( talkIt = entry.talk.begin(); talkIt != entry.talk.end(); ++talkIt ) {
      note += "\nTalk: " + (*talkIt);
    }
    
    a.setNote( note );

    a.setPrefix( entry.rank + a.prefix() );  // Add rank to prefix
    
    a.setCategories( entry.categories );

    kdDebug(5700) << "Addressee: " << a.familyName() << endl;

    kabcBook->insertAddressee( a );
  }

  kab.save( true );

  StdAddressBook::save();
  
  kdDebug(5700) << "Saved kabc addressbook to '" << kabcBook->fileName() << "'" << endl;
}
