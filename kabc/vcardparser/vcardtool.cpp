/*
    This file is part of libkabc.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include <qdatastream.h>
#include <qstring.h>

#include <kdebug.h>

#include "agent.h"
#include "key.h"
#include "picture.h"
#include "secrecy.h"
#include "sound.h"

#include "vcardtool.h"

using namespace KABC;

VCardTool::VCardTool()
{
  mAddressTypeMap.insert( "dom", Address::Dom );
  mAddressTypeMap.insert( "intl", Address::Intl );
  mAddressTypeMap.insert( "postal", Address::Postal );
  mAddressTypeMap.insert( "parcel", Address::Parcel );
  mAddressTypeMap.insert( "home", Address::Home );
  mAddressTypeMap.insert( "work", Address::Work );
  mAddressTypeMap.insert( "pref", Address::Pref );

  mPhoneTypeMap.insert( "HOME", PhoneNumber::Home );
  mPhoneTypeMap.insert( "WORK", PhoneNumber::Work );
  mPhoneTypeMap.insert( "MSG", PhoneNumber::Msg );
  mPhoneTypeMap.insert( "PREF", PhoneNumber::Pref );
  mPhoneTypeMap.insert( "VOICE", PhoneNumber::Voice );
  mPhoneTypeMap.insert( "FAX", PhoneNumber::Fax );
  mPhoneTypeMap.insert( "CELL", PhoneNumber::Cell );
  mPhoneTypeMap.insert( "VIDEO", PhoneNumber::Video );
  mPhoneTypeMap.insert( "BBS", PhoneNumber::Bbs );
  mPhoneTypeMap.insert( "MODEM", PhoneNumber::Modem );
  mPhoneTypeMap.insert( "CAR", PhoneNumber::Car );
  mPhoneTypeMap.insert( "ISDN", PhoneNumber::Isdn );
  mPhoneTypeMap.insert( "PCS", PhoneNumber::Pcs );
  mPhoneTypeMap.insert( "PAGER", PhoneNumber::Pager );
}

VCardTool::~VCardTool()
{
}

QString VCardTool::createVCards( Addressee::List list, VCard::Version version )
{
  VCard::List vCardList;

  Addressee::List::Iterator addrIt;
  for ( addrIt = list.begin(); addrIt != list.end(); ++addrIt ) {
    VCard card;
    QStringList::ConstIterator strIt;

    // ADR + LABEL
    Address::List addresses = (*addrIt).addresses();
    for ( Address::List::Iterator it = addresses.begin(); it != addresses.end(); ++it ) {
      QStringList address;

      address.append( (*it).postOfficeBox() );
      address.append( (*it).extended() );
      address.append( (*it).street() );
      address.append( (*it).locality() );
      address.append( (*it).region() );
      address.append( (*it).postalCode() );
      address.append( (*it).country() );

      VCardLine adrLine( "adr", address.join( ";" ) );
      VCardLine labelLine( "label", (*it).label() );

      bool hasLabel = !(*it).label().isEmpty();
      QMap<QString, int>::Iterator typeIt;
      for ( typeIt = mAddressTypeMap.begin(); typeIt != mAddressTypeMap.end(); ++typeIt ) {
        if ( typeIt.data() & (*it).type() ) {
          adrLine.addParameter( "type", typeIt.key() );
          if ( hasLabel )
            labelLine.addParameter( "type",  typeIt.key() );
        }
      }

      card.addLine( adrLine );
      if ( hasLabel )
        card.addLine( labelLine );
    }

    // AGENT
    card.addLine( createAgent( version, (*addrIt).agent() ) );

    // BDAY
    card.addLine( VCardLine( "bday", createDateTime( (*addrIt).birthday() ) ) );

    // CATEGORIES
    if ( version == VCard::v3_0 )
      card.addLine( VCardLine( "categories", (*addrIt).categories().join( "," ) ) );

    // CLASS
    if ( version == VCard::v3_0 )
      card.addLine( createSecrecy( (*addrIt).secrecy() ) );

    // EMAIL
    QStringList emails = (*addrIt).emails();
    bool pref = true;
    for ( strIt = emails.begin(); strIt != emails.end(); ++strIt ) {
      VCardLine line( "email", *strIt );
      if ( pref == true ) {
        line.addParameter( "type", "PREF" );
        pref = false;
      }
      card.addLine( line );
    }

    // FN
    card.addLine( VCardLine( "fn", (*addrIt).formattedName() ) );

    // GEO
    Geo geo = (*addrIt).geo();
    if ( geo.isValid() ) {
      QString str;
      str.sprintf( "%.6f;%.6f", geo.latitude(), geo.longitude() );
      card.addLine( VCardLine( "geo", str ) );
    }

    // KEY
    Key::List keys = (*addrIt).keys();
    Key::List::ConstIterator keyIt;
    for ( keyIt = keys.begin(); keyIt != keys.end(); ++keyIt )
      card.addLine( createKey( *keyIt ) );

    // LOGO
    card.addLine( createPicture( "logo", (*addrIt).logo() ) );

    // MAILER
    card.addLine( VCardLine( "mailer", (*addrIt).mailer() ) );

    // N
    QStringList name;
    name.append( (*addrIt).familyName() );
    name.append( (*addrIt).givenName() );
    name.append( (*addrIt).additionalName() );
    name.append( (*addrIt).prefix() );
    name.append( (*addrIt).suffix() );

    card.addLine( VCardLine( "n", name.join( ";" ) ) );

    // NICKNAME
    if ( version == VCard::v3_0 )
      card.addLine( VCardLine( "nickname", (*addrIt).nickName() ) );

    // NOTE
    card.addLine( VCardLine( "note", (*addrIt).note() ) );

    // ORG
    card.addLine( VCardLine( "org", (*addrIt).organization() ) );

    // PHOTO
    card.addLine( createPicture( "photo", (*addrIt).photo() ) );

    // PROID
    if ( version == VCard::v3_0 )
      card.addLine( VCardLine( "prodid", (*addrIt).productId() ) );

    // REV
    card.addLine( VCardLine( "rev", createDateTime( (*addrIt).revision() ) ) );

    // ROLE
    card.addLine( VCardLine( "role", (*addrIt).role() ) );

    // SORT-STRING
    if ( version == VCard::v3_0 )
      card.addLine( VCardLine( "sort-string", (*addrIt).sortString() ) );

    // SOUND
    card.addLine( createSound( (*addrIt).sound() ) );

    // TEL
    PhoneNumber::List phoneNumbers = (*addrIt).phoneNumbers();
    PhoneNumber::List::ConstIterator phoneIt;
    for ( phoneIt = phoneNumbers.begin(); phoneIt != phoneNumbers.end(); ++phoneIt ) {
      VCardLine line( "tel", (*phoneIt).number() );

      QMap<QString, int>::Iterator typeIt;
      for ( typeIt = mPhoneTypeMap.begin(); typeIt != mPhoneTypeMap.end(); ++typeIt ) {
        if ( typeIt.data() & (*phoneIt).type() )
          line.addParameter( "type", typeIt.key() );
      }

      card.addLine( line );
    }

    // TITLE
    card.addLine( VCardLine( "title", (*addrIt).title() ) );

    // TZ
    TimeZone timeZone = (*addrIt).timeZone();
    if ( timeZone.isValid() ) {
      QString str;

      int neg = 1;
      if ( timeZone.offset() < 0 )
        neg = -1;

      str.sprintf( "%c%02d:%02d", ( timeZone.offset() >= 0 ? '+' : '-' ),
                                  ( timeZone.offset() / 60 ) * neg,
                                  ( timeZone.offset() % 60 ) * neg );

      card.addLine( VCardLine( "tz", str ) );
    }

    // UID
    card.addLine( VCardLine( "uid", (*addrIt).uid() ) );

    // URL
    card.addLine( VCardLine( "url", (*addrIt).url().url() ) );

    // VERSION    
    if ( version == VCard::v2_1 )
      card.addLine( VCardLine( "version", "2.1" ) );
    if ( version == VCard::v3_0 )
      card.addLine( VCardLine( "version", "3.0" ) );

    // X-
    QStringList customs = (*addrIt).customs();
    for ( strIt = customs.begin(); strIt != customs.end(); ++strIt ) {
      QString identifier = "X-" + (*strIt).left( (*strIt).find( ":" ) );
      QString value = (*strIt).mid( (*strIt).find( ":" ) + 1 );
      if ( value.isEmpty() )
        continue;

      card.addLine( VCardLine( identifier, value ) );
    }

    vCardList.append( card );
  }

  return VCardParser::createVCards( vCardList );
}

Addressee::List VCardTool::parseVCards( const QString& vcard )
{
  Addressee::List addrList;

  VCard::List vCardList = VCardParser::parseVCards( vcard );

  VCard::List::Iterator cardIt;
  for ( cardIt = vCardList.begin(); cardIt != vCardList.end(); ++cardIt ) {
    Addressee addr;
    QStringList idents = (*cardIt).identifiers();
    QStringList::ConstIterator identIt;
    for ( identIt = idents.begin(); identIt != idents.end(); ++identIt ) {
      VCard card = (*cardIt);
      VCardLine::List lines = card.lines( (*identIt) );
      VCardLine::List::Iterator lineIt;

      // iterate over the lines
      for ( lineIt = lines.begin(); lineIt != lines.end(); ++lineIt ) {
        QStringList params = (*lineIt).parameterList();

        // ADR
        if ( (*lineIt).identifier() == "adr" ) {
          Address address;
          QStringList addrParts = QStringList::split( ';', (*lineIt).value().asString(), true );
          if ( addrParts.count() > 0 )
            address.setPostOfficeBox( addrParts[0] );
          if ( addrParts.count() > 1 )
            address.setExtended( addrParts[1] );
          if ( addrParts.count() > 2 )
            address.setStreet( addrParts[2] );
          if ( addrParts.count() > 3 )
            address.setLocality( addrParts[3] );
          if ( addrParts.count() > 4 )
            address.setRegion( addrParts[4] );
          if ( addrParts.count() > 5 )
            address.setPostalCode( addrParts[5] );
          if ( addrParts.count() > 6 )
            address.setCountry( addrParts[6] );

          int type = 0;

          QStringList types = (*lineIt).parameters( "type" );
          for ( QStringList::Iterator it = types.begin(); it != types.end(); ++it )
            type += mAddressTypeMap[ (*it).lower() ];

          if ( !type )
            type = Address::Home; // default

          address.setType( type );
          addr.insertAddress( address );
        }

        // AGENT
        if ( (*lineIt).identifier() == "agent" )
          addr.setAgent( parseAgent( *lineIt ) );

        // BDAY
        if ( (*lineIt).identifier() == "bday" )
          addr.setBirthday( parseDateTime( (*lineIt).value().asString() ) );

        // CATEGORIES
        if ( (*lineIt).identifier() == "categories" ) {
          QStringList categories = QStringList::split( ',', (*lineIt).value().asString(), true );
          addr.setCategories( categories );
        }

        // CLASS
        if ( (*lineIt).identifier() == "class" )
          addr.setSecrecy( parseSecrecy( *lineIt ) );

        // EMAIL
        if ( (*lineIt).identifier() == "email" ) {
          QStringList types = (*lineIt).parameters( "type" );
          addr.insertEmail( (*lineIt).value().asString(), types.contains( "PREF" ) );
        }

        // FN
        if ( (*lineIt).identifier() == "fn" )
          addr.setFormattedName( (*lineIt).value().asString() );

        // GEO
        if ( (*lineIt).identifier() == "geo" ) {
          Geo geo;
          
          QStringList geoParts = QStringList::split( ';', (*lineIt).value().asString(), true );
          geo.setLatitude( geoParts[0].toFloat() );
          geo.setLongitude( geoParts[1].toFloat() );

          addr.setGeo( geo );
        }

        // KEY
        if ( (*lineIt).identifier() == "key" )
          addr.insertKey( parseKey( *lineIt ) );

        // LABEL
        if ( (*lineIt).identifier() == "label" ) {
          int type = 0;

          QStringList types = (*lineIt).parameters( "type" );
          for ( QStringList::Iterator it = types.begin(); it != types.end(); ++it )
            type += mAddressTypeMap[ (*it).lower() ];

          if ( !type )
            type = Address::Home;

          KABC::Address::List addressList = addr.addresses();
          KABC::Address::List::Iterator it;
          for ( it = addressList.begin(); it != addressList.end(); ++it ) {
            if ( (*it).type() == type ) {
              (*it).setLabel( (*lineIt).value().asString() );
              addr.insertAddress( *it );
            }
          }
        }

        // LOGO
        if ( (*lineIt).identifier() == "logo" )
          addr.setLogo( parsePicture( *lineIt ) );

        // MAILER
        if ( (*lineIt).identifier() == "mailer" )
          addr.setMailer( (*lineIt).value().asString() );

        // N
        if ( (*lineIt).identifier() == "n" ) {
          QStringList nameParts = QStringList::split( ';', (*lineIt).value().asString(), true );
          if ( nameParts.count() > 0 )
            addr.setFamilyName( nameParts[0] );
          if ( nameParts.count() > 1 )
            addr.setGivenName( nameParts[1] );
          if ( nameParts.count() > 2 )
            addr.setAdditionalName( nameParts[2] );
          if ( nameParts.count() > 3 )
            addr.setPrefix( nameParts[3] );
          if ( nameParts.count() > 4 )
            addr.setSuffix( nameParts[4] );
        }

        // NICKNAME
        if ( (*lineIt).identifier() == "nickname" )
          addr.setNickName( (*lineIt).value().asString() );

        // NOTE
        if ( (*lineIt).identifier() == "note" )
          addr.setNote( (*lineIt).value().asString() );

        // ORGANIZATION
        if ( (*lineIt).identifier() == "org" )
          addr.setOrganization( (*lineIt).value().asString() );

        // PHOTO
        if ( (*lineIt).identifier() == "photo" )
          addr.setPhoto( parsePicture( *lineIt ) );

        // PROID
        if ( (*lineIt).identifier() == "prodid" )
          addr.setProductId( (*lineIt).value().asString() );

        // REV
        if ( (*lineIt).identifier() == "rev" )
          addr.setRevision( parseDateTime( (*lineIt).value().asString() ) );

        // ROLE
        if ( (*lineIt).identifier() == "role" )
          addr.setRole( (*lineIt).value().asString() );
        
        // SORT-STRING
        if ( (*lineIt).identifier() == "sort-string" )
          addr.setSortString( (*lineIt).value().asString() );

        // SOUND
        if ( (*lineIt).identifier() == "sound" )
          addr.setSound( parseSound( *lineIt ) );

        // TEL
        if ( (*lineIt).identifier() == "tel" ) {
          PhoneNumber phone;
          phone.setNumber( (*lineIt).value().asString() );

          int type = PhoneNumber::Home; // default

          QStringList types = (*lineIt).parameters( "type" );
          for ( QStringList::Iterator it = types.begin(); it != types.end(); ++it )
            type += mPhoneTypeMap[(*it).upper()];

          phone.setType( type );

          addr.insertPhoneNumber( phone );
        }

        // TITLE
        if ( (*lineIt).identifier() == "title" )
          addr.setTitle( (*lineIt).value().asString() );

        // TZ
        if ( (*lineIt).identifier() == "tz" ) {
          TimeZone tz;
          QString date = (*lineIt).value().asString();

          int hours = date.mid( 1, 2).toInt();
          int minutes = date.mid( 4, 2 ).toInt();
          int offset = ( hours * 60 ) + minutes;
          offset = offset * ( date[0] == '+' ? 1 : -1 );

          tz.setOffset( offset );
          addr.setTimeZone( tz );
        }

        // UID
        if ( (*lineIt).identifier() == "uid" )
          addr.setUid( (*lineIt).value().asString() );

        // URL
        if ( (*lineIt).identifier() == "url" )
          addr.setUrl( (*lineIt).value().asString() );

        // X-
        if ( (*lineIt).identifier().startsWith( "x-" ) ) {
          QString key = (*lineIt).identifier().mid( 2 );
          int dash = key.find( "-" );
          addr.insertCustom( key.left( dash ), key.mid( dash + 1 ), (*lineIt).value().asString() );
        }

      }
    }
    addrList.append( addr );
  }  

  return addrList;
}

QDateTime VCardTool::parseDateTime( const QString &str )
{
  QDateTime dateTime;

  if ( str.find( '-' ) == -1 ) { // is base format (yyyymmdd)
    dateTime.setDate( QDate( str.left( 4 ).toInt(), str.mid( 4, 2 ).toInt(),
                             str.mid( 6, 2 ).toInt() ) );

    if ( str.find( 'T' ) ) // has time information yyyymmddThh:mm:ss
      dateTime.setTime( QTime( str.mid( 11, 2 ).toInt(), str.mid( 14, 2 ).toInt(),
                               str.mid( 17, 2 ).toInt() ) );

  } else { // is extended format yyyy-mm-dd
    dateTime.setDate( QDate( str.left( 4 ).toInt(), str.mid( 5, 2 ).toInt(),
                             str.mid( 8, 2 ).toInt() ) );

    if ( str.find( 'T' ) ) // has time information yyyy-mm-ddThh:mm:ss
      dateTime.setTime( QTime( str.mid( 11, 2 ).toInt(), str.mid( 14, 2 ).toInt(),
                               str.mid( 17, 2 ).toInt() ) );
  }

  return dateTime;
}

QString VCardTool::createDateTime( const QDateTime &dateTime )
{
  QString str;
  
  if ( dateTime.date().isValid() ) {
    str.sprintf( "%4d-%02d-%02d", dateTime.date().year(), dateTime.date().month(),
                 dateTime.date().day() );
    if ( dateTime.time().isValid() ) {
      QString tmp;
      tmp.sprintf( "T%02d:%02d:%02dZ", dateTime.time().hour(), dateTime.time().minute(),
                   dateTime.time().second() );
      str += tmp;
    }
  }

  return str;
}

Picture VCardTool::parsePicture( const VCardLine &line )
{
  Picture pic;

  QStringList params = line.parameterList();
  if ( params.contains( "encoding" ) )
    pic.setData( line.value().asByteArray() );
  else if ( params.contains( "value" ) ) {
    if ( line.parameter( "value" ).lower() == "uri" )
      pic.setUrl( line.value().asString() );
  }

  if ( params.contains( "type" ) )
    pic.setType( line.parameter( "type" ) );
        
  return pic;
}

VCardLine VCardTool::createPicture( const QString &identifier, const Picture &pic )
{
  VCardLine line( identifier );

  if ( pic.isIntern() ) {
    if ( !pic.data().isNull() ) {
      QByteArray input;
      QDataStream s( input, IO_WriteOnly );
      s << pic.data();
      line.setValue( input );
      line.addParameter( "encoding", "b" );
      line.addParameter( "type", "image/png" );
    }
  } else if ( !pic.url().isEmpty() ) {
    line.setValue( pic.url() );
    line.addParameter( "value", "URI" );
  }

  return line;
}

Sound VCardTool::parseSound( const VCardLine &line )
{
  Sound snd;

  QStringList params = line.parameterList();
  if ( params.contains( "encoding" ) )
    snd.setData( line.value().asByteArray() );
  else if ( params.contains( "value" ) ) {
    if ( line.parameter( "value" ).lower() == "uri" )
      snd.setUrl( line.value().asString() );
  }

/* TODO: support sound types
  if ( params.contains( "type" ) )
    snd.setType( line.parameter( "type" ) );
*/      

  return snd;
}

VCardLine VCardTool::createSound( const Sound &snd )
{
  VCardLine line( "sound" );

  if ( snd.isIntern() ) {
    if ( !snd.data().isEmpty() ) {
      line.setValue( snd.data() );
      line.addParameter( "encoding", "b" );
      // TODO: need to store sound type!!!
    }
  } else if ( !snd.url().isEmpty() ) {
    line.setValue( snd.url() );
    line.addParameter( "value", "URI" );
  }

  return line;
}

Key VCardTool::parseKey( const VCardLine &line )
{
  Key key;

  QStringList params = line.parameterList();
  if ( params.contains( "encoding" ) )
    key.setBinaryData( line.value().asByteArray() );
  else
    key.setTextData( line.value().asString() );

  if ( params.contains( "type" ) ) {
    if ( line.parameter( "type" ).lower() == "x509" )
      key.setType( Key::X509 );
    else if ( line.parameter( "type" ).lower() == "pgp" )
      key.setType( Key::PGP );
    else {
      key.setType( Key::Custom );
      key.setCustomTypeString( line.parameter( "type" ) );
    }
  }

  return key;
}

VCardLine VCardTool::createKey( const Key &key )
{
  VCardLine line( "key" );

  if ( key.isBinary() ) {
    if ( !key.binaryData().isEmpty() ) {
      line.setValue( key.binaryData() );
      line.addParameter( "encoding", "b" );
    }
  } else if ( !key.textData().isEmpty() )
    line.setValue( key.textData() );

  if ( key.type() == Key::X509 )
    line.addParameter( "type", "X509" );
  else if ( key.type() == Key::PGP )
    line.addParameter( "type", "PGP" );
  else if ( key.type() == Key::Custom )
    line.addParameter( "type", key.customTypeString() );

  return line;
}

Secrecy VCardTool::parseSecrecy( const VCardLine &line )
{
  Secrecy secrecy;

  if ( line.value().asString().lower() == "public" )
    secrecy.setType( Secrecy::Public );
  if ( line.value().asString().lower() == "private" )
    secrecy.setType( Secrecy::Private );
  if ( line.value().asString().lower() == "confidential" )
    secrecy.setType( Secrecy::Confidential );

  return secrecy;
}

VCardLine VCardTool::createSecrecy( const Secrecy &secrecy )
{
  VCardLine line( "class" );

  // use default type if no type is set
  int type = secrecy.type();
  if ( type == 0 )
    type = Secrecy::Private;

  if ( type == Secrecy::Public )
    line.setValue( "PUBLIC" );
  else if ( type == Secrecy::Private )
    line.setValue( "PRIVATE" );
  else if ( type == Secrecy::Confidential )
    line.setValue( "CONFIDENTIAL" );

  return line;
}

Agent VCardTool::parseAgent( const VCardLine &line )
{
  Agent agent;

  QStringList params = line.parameterList();
  if ( params.contains( "value" ) ) {
    if ( line.parameter( "value" ).lower() == "uri" )
      agent.setUrl( line.value().asString() );
  } else {
    QString str = line.value().asString();
    str.replace( "\\n", "\r\n" );
    str.replace( "\\N", "\r\n" );
    str.replace( "\\;", ";" );
    str.replace( "\\:", ":" );
    str.replace( "\\,", "," );

    Addressee::List list = parseVCards( str );
    if ( list.count() > 0 ) {
      Addressee *addr = new Addressee;
      *addr = list[ 0 ];
      agent.setAddressee( addr );
    }
  }

  return agent;
}

VCardLine VCardTool::createAgent( VCard::Version version, const Agent &agent )
{
  VCardLine line( "agent" );

  if ( agent.isIntern() ) {
    if ( agent.addressee() != 0 ) {
      Addressee::List list;
      list.append( *agent.addressee() );

      QString str = createVCards( list, version );
      str.replace( "\r\n", "\\n" );
      str.replace( ";", "\\;" );
      str.replace( ":", "\\:" );
      str.replace( ",", "\\," );
      line.setValue( str );
    }
  } else if ( !agent.url().isEmpty() ) {
    line.setValue( agent.url() );
    line.addParameter( "value", "URI" );
  }

  return line;
}
