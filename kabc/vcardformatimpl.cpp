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
#include <qtextstream.h>

#include <kdebug.h>
#include <ksavefile.h>

#include <VCard.h>

#include "addressbook.h"

#include "vcardformatimpl.h"

using namespace KABC;
using namespace VCARD;

bool VCardFormatImpl::load( AddressBook *addressBook, Resource *resource, QFile *file )
{
  kdDebug(5700) << "VCardFormat::load()" << endl;
  QString data;

  QTextStream t( file );
  t.setEncoding( QTextStream::UnicodeUTF8 );
  data = t.read();
  
  VCardEntity e( data.utf8() );
  
  VCardListIterator it( e.cardList() );

  for (; it.current(); ++it) {
    VCard v(*it.current());
    QPtrList<ContentLine> contentLines = v.contentLineList();
    ContentLine *cl;

    Addressee a;

    for( cl = contentLines.first(); cl; cl = contentLines.next() ) {
      QCString n = cl->name();
      if ( n.left( 2 ) == "X-" ) {
        n = n.mid( 2 );
        int posDash = n.find( "-" );
        a.insertCustom( QString::fromUtf8( n.left( posDash ) ),
                        QString::fromUtf8( n.mid( posDash + 1 ) ),
                        QString::fromUtf8( cl->value()->asString() ) );
        continue;
      }
    
      EntityType type = cl->entityType();
      switch( type ) {

        case EntityUID:
          a.setUid( readTextValue( cl ) );
          break;

        case EntityEmail:
          a.insertEmail( readTextValue( cl ) );
          break;

        case EntityName:
          a.setName( readTextValue( cl ) );
          break;

        case EntityFullName:
          a.setFormattedName( readTextValue( cl ) );
          break;

        case EntityURL:
          a.setUrl( KURL( readTextValue( cl ) ) );
          break;

        case EntityNickname:
          a.setNickName( readTextValue( cl ) );
          break;

        case EntityLabel:
//          not supported by kabc
          break;

        case EntityMailer:
          a.setMailer( readTextValue( cl ) );
          break;

        case EntityTitle:
          a.setTitle( readTextValue( cl ) );
          break;

        case EntityRole:
          a.setRole( readTextValue( cl ) );
          break;

        case EntityOrganisation:
          a.setOrganization( readTextValue( cl ) );
          break;

        case EntityNote:
          a.setNote( readTextValue( cl ) );
          break;

        case EntityProductID:
          a.setProductId( readTextValue( cl ) );
          break;

        case EntitySortString:
          a.setSortString( readTextValue( cl ) );
          break;

        case EntityN:
          readNValue( cl, a );
          break;

        case EntityAddress:
          a.insertAddress( readAddressValue( cl ) );
          break;

        case EntityTelephone:
          a.insertPhoneNumber( readTelephoneValue( cl ) );
          break;

        case EntityCategories:
          a.setCategories( QStringList::split( ",", readTextValue( cl ) ) );
          break;

        case EntityBirthday:
          a.setBirthday( readDateValue( cl ) );
          break;

	case EntityRevision:
	  a.setRevision( readDateTimeValue( cl ) );
          break;

	case EntityGeo:
	  a.setGeo( readGeoValue( cl ) );
          break;

        case EntityVersion:
          break;
          
        default:
          kdDebug(5700) << "VCardFormat::load(): Unsupported entity: "
                    << int( type ) << ": " << cl->asString() << endl;
          break;
      }
    }

    for( cl = contentLines.first(); cl; cl = contentLines.next() ) {
      EntityType type = cl->entityType();
      if ( type == EntityLabel ) {
        int type = readAddressParam( cl );
        Address address = a.address( type );
        if ( address.isEmpty() ) {
          address.setType( type );
        }
        address.setLabel( QString::fromUtf8( cl->value()->asString() ) );
        a.insertAddress( address );
      }
    }
  
    a.setResource( resource );
    addressBook->insertAddressee( a );
  }

  
  return true;
}

bool VCardFormatImpl::save( Addressee *addressee, QFile *file )
{
  VCardEntity vcards;
  VCardList vcardlist;
  vcardlist.setAutoDelete( true );
  ContentLine cl;
  QString value;

  VCard *v = new VCard;

  addTextValue( v, EntityName, addressee->name() );
  addTextValue( v, EntityUID, addressee->uid() );
  addTextValue( v, EntityFullName, addressee->formattedName() );
    
  QStringList emails = addressee->emails();
  QStringList::ConstIterator it4;
  for( it4 = emails.begin(); it4 != emails.end(); ++it4 ) {
    addTextValue( v, EntityEmail, *it4 );
  }

  QStringList customs = addressee->customs();
  QStringList::ConstIterator it5;
  for( it5 = customs.begin(); it5 != customs.end(); ++it5 ) {
    addCustomValue( v, *it5 );
  }

  addTextValue( v, EntityURL, addressee->url().url() );

  addNValue( v, *addressee );

  addTextValue( v, EntityNickname, addressee->nickName() );
  addTextValue( v, EntityMailer, addressee->mailer() );
  addTextValue( v, EntityTitle, addressee->title() );
  addTextValue( v, EntityRole, addressee->role() );
  addTextValue( v, EntityOrganisation, addressee->organization() );
  addTextValue( v, EntityNote, addressee->note() );
  addTextValue( v, EntityProductID, addressee->productId() );
  addTextValue( v, EntitySortString, addressee->sortString() );

  Address::List addresses = addressee->addresses();
  Address::List::ConstIterator it3;
  for( it3 = addresses.begin(); it3 != addresses.end(); ++it3 ) {
    addAddressValue( v, *it3 );
    addLabelValue( v, *it3 );
  }

  PhoneNumber::List phoneNumbers = addressee->phoneNumbers();
  PhoneNumber::List::ConstIterator it2;
  for( it2 = phoneNumbers.begin(); it2 != phoneNumbers.end(); ++it2 ) {
    addTelephoneValue( v, *it2 );
  }

  addTextValue( v, EntityCategories, addressee->categories().join(",") );

  addDateValue( v, EntityBirthday, addressee->birthday().date() );
  addDateTimeValue( v, EntityRevision, addressee->revision() );
  addGeoValue( v, EntityGeo, addressee->geo() );

  vcardlist.append( v );

  vcards.setCardList( vcardlist );

  QTextStream t( file );
  t.setEncoding( QTextStream::UnicodeUTF8 );
  t << QString::fromUtf8( vcards.asString() );

  return true;
}

void VCardFormatImpl::addCustomValue( VCard *v, const QString &txt )
{
  if ( txt.isEmpty() ) return;

  ContentLine cl;
  cl.setName( "X-" + txt.left( txt.find( ":" ) ).utf8() );
  cl.setValue( new TextValue( txt.mid( txt.find( ":" ) + 1 ).utf8() ) );
  v->add(cl);
}

void VCardFormatImpl::addTextValue( VCard *v, EntityType type, const QString &txt )
{
  if ( txt.isEmpty() ) return;

  ContentLine cl;
  cl.setName( EntityTypeToParamName( type ) );
  cl.setValue( new TextValue( txt.utf8() ) );
  v->add(cl);
}

void VCardFormatImpl::addDateValue( VCard *vcard, EntityType type,
                                    const QDate &date )
{
  if ( !date.isValid() ) return;

  ContentLine cl;
  cl.setName( EntityTypeToParamName( type ) );

  DateValue *v = new DateValue( date );
  cl.setValue( v );
  vcard->add(cl);
}

void VCardFormatImpl::addDateTimeValue( VCard *vcard, EntityType type,
                                    const QDateTime &dateTime )
{
  if ( !dateTime.isValid() ) return;

  ContentLine cl;
  cl.setName( EntityTypeToParamName( type ) );

  DateValue *v = new DateValue( dateTime );
  cl.setValue( v );
  vcard->add(cl);
}

void VCardFormatImpl::addAddressValue( VCard *vcard, const Address &a )
{
  ContentLine cl;
  cl.setName( EntityTypeToParamName( EntityAddress ) );

  AdrValue *v = new AdrValue;
  v->setPOBox( a.postOfficeBox().utf8() );
  v->setExtAddress( a.extended().utf8() );
  v->setStreet( a.street().utf8() );
  v->setLocality( a.locality().utf8() );
  v->setRegion( a.region().utf8() );
  v->setPostCode( a.postalCode().utf8() );
  v->setCountryName( a.country().utf8() );
  cl.setValue( v );

  addAddressParam( &cl, a.type() );

  vcard->add( cl );
}

void VCardFormatImpl::addLabelValue( VCard *vcard, const Address &a )
{
  if ( a.label().isEmpty() ) return;

  ContentLine cl;
  cl.setName( EntityTypeToParamName( EntityLabel ) );
  cl.setValue( new TextValue( a.label().utf8() ) );
  
  addAddressParam( &cl, a.type() );
  
  vcard->add( cl );
}

void VCardFormatImpl::addAddressParam( ContentLine *cl, int type )
{
  ParamList params;
  if ( type & Address::Dom ) params.append( new Param( "TYPE", "dom" ) );
  if ( type & Address::Intl ) params.append( new Param( "TYPE", "intl" ) );
  if ( type & Address::Parcel ) params.append( new Param( "TYPE", "parcel" ) );
  if ( type & Address::Postal ) params.append( new Param( "TYPE", "postal" ) );
  if ( type & Address::Work ) params.append( new Param( "TYPE", "work" ) );
  if ( type & Address::Home ) params.append( new Param( "TYPE", "home" ) );
  if ( type & Address::Pref ) params.append( new Param( "TYPE", "pref" ) );
  cl->setParamList( params );
}

void VCardFormatImpl::addGeoValue( VCard *vcard, EntityType type,
                                    const Geo &geo )
{
  if ( !geo.isValid() ) return;

  ContentLine cl;
  cl.setName( EntityTypeToParamName( type ) );

  GeoValue *v = new GeoValue;
  v->setLatitude( geo.latitude() );
  v->setLongitude( geo.longitude() );

  cl.setValue( v );
  vcard->add(cl);
}


Address VCardFormatImpl::readAddressValue( ContentLine *cl )
{
  Address a;
  AdrValue *v = (AdrValue *)cl->value();
  a.setPostOfficeBox( QString::fromUtf8( v->poBox() ) );
  a.setExtended( QString::fromUtf8( v->extAddress() ) );
  a.setStreet( QString::fromUtf8( v->street() ) );
  a.setLocality( QString::fromUtf8( v->locality() ) );
  a.setRegion( QString::fromUtf8( v->region() ) );
  a.setPostalCode( QString::fromUtf8( v->postCode() ) );
  a.setCountry( QString::fromUtf8( v->countryName() ) );

  a.setType( readAddressParam( cl ) );

  return a;
}

int VCardFormatImpl::readAddressParam( ContentLine *cl )
{
  int type = 0;
  ParamList params = cl->paramList();
  ParamListIterator it( params );
  for( ; it.current(); ++it ) {
    if ( (*it)->name() == "TYPE" ) {
      if ( (*it)->value() == "dom" ) type |= Address::Dom;
      else if ( (*it)->value() == "intl" ) type |= Address::Intl;
      else if ( (*it)->value() == "parcel" ) type |= Address::Parcel;
      else if ( (*it)->value() == "postal" ) type |= Address::Postal;
      else if ( (*it)->value() == "work" ) type |= Address::Work;
      else if ( (*it)->value() == "home" ) type |= Address::Home;
      else if ( (*it)->value() == "pref" ) type |= Address::Pref;
    }
  }
  return type;
}

void VCardFormatImpl::addNValue( VCard *vcard, const Addressee &a )
{
  ContentLine cl;
  cl.setName(EntityTypeToParamName( EntityN ) );
  NValue *v = new NValue;
  v->setFamily( a.familyName().utf8() );
  v->setGiven( a.givenName().utf8() );
  v->setMiddle( a.additionalName().utf8() );
  v->setPrefix( a.prefix().utf8() );
  v->setSuffix( a.suffix().utf8() );
  
  cl.setValue( v );
  vcard->add(cl);
}

void VCardFormatImpl::readNValue( ContentLine *cl, Addressee &a )
{
  NValue *v = (NValue *)cl->value();
  a.setFamilyName( QString::fromUtf8( v->family() ) );
  a.setGivenName( QString::fromUtf8( v->given() ) );
  a.setAdditionalName( QString::fromUtf8( v->middle() ) );
  a.setPrefix( QString::fromUtf8( v->prefix() ) );
  a.setSuffix( QString::fromUtf8( v->suffix() ) );
}

void VCardFormatImpl::addTelephoneValue( VCard *v, const PhoneNumber &p )
{
  ContentLine cl;
  cl.setName(EntityTypeToParamName(EntityTelephone));
  cl.setValue(new TelValue( p.number().utf8() ));

  ParamList params;
  if( p.type() & PhoneNumber::Home ) params.append( new Param( "TYPE", "home" ) );
  if( p.type() & PhoneNumber::Work ) params.append( new Param( "TYPE", "work" ) );
  if( p.type() & PhoneNumber::Msg ) params.append( new Param( "TYPE", "msg" ) );
  if( p.type() & PhoneNumber::Pref ) params.append( new Param( "TYPE", "pref" ) );
  if( p.type() & PhoneNumber::Voice ) params.append( new Param( "TYPE", "voice" ) );
  if( p.type() & PhoneNumber::Fax ) params.append( new Param( "TYPE", "fax" ) );
  if( p.type() & PhoneNumber::Cell ) params.append( new Param( "TYPE", "cell" ) );
  if( p.type() & PhoneNumber::Video ) params.append( new Param( "TYPE", "video" ) );
  if( p.type() & PhoneNumber::Bbs ) params.append( new Param( "TYPE", "bbs" ) );
  if( p.type() & PhoneNumber::Modem ) params.append( new Param( "TYPE", "modem" ) );
  if( p.type() & PhoneNumber::Car ) params.append( new Param( "TYPE", "car" ) );
  if( p.type() & PhoneNumber::Isdn ) params.append( new Param( "TYPE", "isdn" ) );
  if( p.type() & PhoneNumber::Pcs ) params.append( new Param( "TYPE", "pcs" ) );
  if( p.type() & PhoneNumber::Pager ) params.append( new Param( "TYPE", "pager" ) );
  cl.setParamList( params );

  v->add(cl);
}

PhoneNumber VCardFormatImpl::readTelephoneValue( ContentLine *cl )
{
  PhoneNumber p;
  TelValue *value = (TelValue *)cl->value();
  p.setNumber( QString::fromUtf8( value->asString() ) );

  int type = 0;
  ParamList params = cl->paramList();
  ParamListIterator it( params );
  for( ; it.current(); ++it ) {
    if ( (*it)->name() == "TYPE" ) {
      if ( (*it)->value() == "home" ) type |= PhoneNumber::Home;
      else if ( (*it)->value() == "work" ) type |= PhoneNumber::Work;
      else if ( (*it)->value() == "msg" ) type |= PhoneNumber::Msg;
      else if ( (*it)->value() == "pref" ) type |= PhoneNumber::Pref;
      else if ( (*it)->value() == "voice" ) type |= PhoneNumber::Voice;
      else if ( (*it)->value() == "fax" ) type |= PhoneNumber::Fax;
      else if ( (*it)->value() == "cell" ) type |= PhoneNumber::Cell;
      else if ( (*it)->value() == "video" ) type |= PhoneNumber::Video;
      else if ( (*it)->value() == "bbs" ) type |= PhoneNumber::Bbs;
      else if ( (*it)->value() == "modem" ) type |= PhoneNumber::Modem;
      else if ( (*it)->value() == "car" ) type |= PhoneNumber::Car;
      else if ( (*it)->value() == "isdn" ) type |= PhoneNumber::Isdn;
      else if ( (*it)->value() == "pcs" ) type |= PhoneNumber::Pcs;
      else if ( (*it)->value() == "pager" ) type |= PhoneNumber::Pager;
    }
  }
  p.setType( type );

  return p;
}

QString VCardFormatImpl::readTextValue( ContentLine *cl )
{
  VCARD::Value *value = cl->value();
  if ( value ) {
    return QString::fromUtf8( value->asString() );
  } else {
    kdDebug(5700) << "No value: " << cl->asString() << endl;
    return QString::null;
  }
}

QDate VCardFormatImpl::readDateValue( ContentLine *cl )
{
  DateValue *dateValue = (DateValue *)cl->value();
  if ( dateValue )
    return dateValue->qdate();
  else
    return QDate();
}

QDateTime VCardFormatImpl::readDateTimeValue( ContentLine *cl )
{
  DateValue *dateValue = (DateValue *)cl->value();
  if ( dateValue )
    return dateValue->qdt();
  else
    return QDateTime();
}

Geo VCardFormatImpl::readGeoValue( ContentLine *cl )
{
  GeoValue *geoValue = (GeoValue *)cl->value();
  if ( geoValue ) {
    Geo geo( geoValue->latitude(), geoValue->longitude() );
    return geo;
  } else
    return Geo();
}
