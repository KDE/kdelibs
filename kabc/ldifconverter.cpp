/*
    This file is part of libkabc.
    Copyright (c) 2003  Helge Deller <deller@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


/*
    Useful links:
        - http://tldp.org/HOWTO/LDAP-Implementation-HOWTO/schemas.html
        - http://www.faqs.org/rfcs/rfc2849.html

    Not yet handled items:
        - objectclass microsoftaddressbook
                - info,
                - initials,
                - otherfacsimiletelephonenumber,
                - otherpager,
                - physicaldeliveryofficename,
*/

#include <qstring.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qtextstream.h>

#include <klocale.h>
#include <kdebug.h>
#include <kcodecs.h>

#include "addressee.h"
#include "address.h"

#include "ldif.h"
#include "ldifconverter.h"
#include "vcardconverter.h"

using namespace KABC;

/* generate LDIF stream */

bool LDIFConverter::addresseeToLDIF( const AddresseeList &addrList, QString &str )
{
  AddresseeList::ConstIterator it;
  for ( it = addrList.begin(); it != addrList.end(); ++it ) {
    addresseeToLDIF( *it, str );
  }
  return true;
}



static void ldif_out( QTextStream &t, QString formatStr, QString value )
{
  if ( value.isEmpty() )
    return;

  QByteArray txt = LDIF::assembleLine( formatStr, value, 72 );

  // write the string
  t << QString::fromUtf8(txt) << "\n";
}

bool LDIFConverter::addresseeToLDIF( const Addressee &addr, QString &str )
{
  if ( addr.isEmpty() )
      return false;

  QTextStream t( &str, QIODevice::WriteOnly|QIODevice::Append );
  t.setEncoding( QTextStream::UnicodeUTF8 );

  const Address homeAddr = addr.address( Address::Home );
  const Address workAddr = addr.address( Address::Work );

  ldif_out( t, "dn", QString( "cn=%1,mail=%2" )
            .arg( addr.formattedName().simplified() )
            .arg( addr.preferredEmail() ) );
  ldif_out( t, "givenname", addr.givenName() );
  ldif_out( t, "sn", addr.familyName() );
  ldif_out( t, "cn", addr.formattedName().simplified() );
  ldif_out( t, "uid", addr.uid() );
  ldif_out( t, "nickname", addr.nickName() );
  ldif_out( t, "xmozillanickname", addr.nickName() );

  ldif_out( t, "mail", addr.preferredEmail() );
  if ( addr.emails().count() > 1 )
    ldif_out( t, "mozillasecondemail", addr.emails()[ 1 ] );
//ldif_out( t, "mozilla_AIMScreenName: %1\n", "screen_name" );

  ldif_out( t, "telephonenumber", addr.phoneNumber( PhoneNumber::Work ).number() );
  ldif_out( t, "facsimiletelephonenumber", addr.phoneNumber( PhoneNumber::Fax ).number() );
  ldif_out( t, "homephone", addr.phoneNumber( PhoneNumber::Home ).number() );
  ldif_out( t, "mobile", addr.phoneNumber( PhoneNumber::Cell ).number() ); // Netscape 7
  ldif_out( t, "cellphone", addr.phoneNumber( PhoneNumber::Cell ).number() ); // Netscape 4.x
  ldif_out( t, "pager", addr.phoneNumber( PhoneNumber::Pager ).number() );
  ldif_out( t, "pagerphone", addr.phoneNumber( PhoneNumber::Pager ).number() );

  ldif_out( t, "streethomeaddress", homeAddr.street() );
  ldif_out( t, "postalcode", workAddr.postalCode() );
  ldif_out( t, "postofficebox", workAddr.postOfficeBox() );

  QStringList streets = QStringList::split( '\n', homeAddr.street() );
  if ( streets.count() > 0 )
    ldif_out( t, "homepostaladdress", streets[ 0 ] ); // Netscape 7
  if ( streets.count() > 1 )
    ldif_out( t, "mozillahomepostaladdress2", streets[ 1 ] ); // Netscape 7
  ldif_out( t, "mozillahomelocalityname", homeAddr.locality() ); // Netscape 7
  ldif_out( t, "mozillahomestate", homeAddr.region() );
  ldif_out( t, "mozillahomepostalcode", homeAddr.postalCode() );
  ldif_out( t, "mozillahomecountryname", Address::ISOtoCountry(homeAddr.country()) );
  ldif_out( t, "locality", workAddr.locality() );
  ldif_out( t, "streetaddress", workAddr.street() ); // Netscape 4.x

  streets = QStringList::split( '\n', workAddr.street() );
  if ( streets.count() > 0 )
    ldif_out( t, "postaladdress", streets[ 0 ] );
  if ( streets.count() > 1 )
    ldif_out( t, "mozillapostaladdress2", streets[ 1 ] );
  ldif_out( t, "countryname", Address::ISOtoCountry(workAddr.country()) );
  ldif_out( t, "l", workAddr.locality() );
  ldif_out( t, "c", Address::ISOtoCountry(workAddr.country()) );
  ldif_out( t, "st", workAddr.region() );

  ldif_out( t, "title", addr.title() );
  ldif_out( t, "vocation", addr.prefix() );
  ldif_out( t, "ou", addr.role() );
  ldif_out( t, "o", addr.organization() );
  ldif_out( t, "organization", addr.organization() );
  ldif_out( t, "organizationname", addr.organization() );
  ldif_out( t, "department", addr.custom("KADDRESSBOOK", "X-Department") );
  ldif_out( t, "workurl", addr.url().prettyURL() );
  ldif_out( t, "homeurl", addr.url().prettyURL() );
  ldif_out( t, "description", addr.note() );
  if (addr.revision().isValid())
    ldif_out(t, "modifytimestamp", dateToVCardString( addr.revision()) );

  t << "objectclass: top\n";
  t << "objectclass: person\n";
  t << "objectclass: organizationalPerson\n";

  t << "\n";

  return true;
}


/* convert from LDIF stream */

bool LDIFConverter::LDIFToAddressee( const QString &str, AddresseeList &addrList, QDateTime dt )
{
  if (str.isEmpty())
     return true;

  bool endldif = false, end = false;
  LDIF ldif;
  LDIF::ParseVal ret;
  const char *latinstr = str.latin1();
  int latinstrlen = qstrlen( latinstr );
  QByteArray data;
  Addressee a;
  Address homeAddr, workAddr;

  data.setRawData( latinstr, latinstrlen );
  ldif.setLDIF( data );
  if (!dt.isValid())
    dt = QDateTime::currentDateTime();
  a.setRevision(dt);
  homeAddr = Address( Address::Home );
  workAddr = Address( Address::Work );

  do {
    ret = ldif.nextItem();
    switch ( ret ) {
      case LDIF::Item: {
        QString fieldname = ldif.attr().toLower();
        QString value = QString::fromUtf8( ldif.val(), ldif.val().size() );
        evaluatePair( a, homeAddr, workAddr, fieldname, value );
        break;
      }
      case LDIF::EndEntry:
      // if the new address is not empty, append it
        if ( !a.formattedName().isEmpty() || !a.name().isEmpty() ||
          !a.familyName().isEmpty() ) {
          if ( !homeAddr.isEmpty() )
            a.insertAddress( homeAddr );
          if ( !workAddr.isEmpty() )
            a.insertAddress( workAddr );
          addrList.append( a );
        }
        a = Addressee();
        a.setRevision(dt);
        homeAddr = Address( Address::Home );
        workAddr = Address( Address::Work );
        break;
      case LDIF::MoreData: {
        if ( endldif )
          end = true;
        else {
          ldif.endLDIF();
          endldif = true;
          break;
        }
      }
      default:
        break;
    }
  } while ( !end );

  data.resetRawData( latinstr, latinstrlen );

  return true;
}

bool LDIFConverter::evaluatePair( Addressee &a, Address &homeAddr,
                                  Address &workAddr,
                                  QString &fieldname, QString &value )
{
  if ( fieldname == QLatin1String( "dn" ) ) // ignore & return false!
    return false;

  if ( fieldname.startsWith("#") ) {
    return true;
  }

  if ( fieldname.isEmpty() && !a.note().isEmpty() ) {
    // some LDIF export filters are borken and add additional
    // comments on stand-alone lines. Just add them to the notes for now.
    a.setNote( a.note() + "\n" + value );
    return true;
  }

  if ( fieldname == QLatin1String( "givenname" ) ) {
    a.setGivenName( value );
    return true;
  }

  if ( fieldname == QLatin1String( "xmozillanickname") ||
       fieldname == QLatin1String( "nickname") ) {
    a.setNickName( value );
    return true;
  }

  if ( fieldname == QLatin1String( "sn" ) ) {
    a.setFamilyName( value );
    return true;
  }

  if ( fieldname == QLatin1String( "uid" ) ) {
    a.setUid( value );
    return true;
  }
  if ( fieldname == QLatin1String( "mail" ) ||
       fieldname == QLatin1String( "mozillasecondemail" ) ) { // mozilla
    if ( a.emails().findIndex( value ) == -1 )
      a.insertEmail( value );
    return true;
  }

  if ( fieldname == QLatin1String( "title" ) ) {
    a.setTitle( value );
    return true;
  }

  if ( fieldname == QLatin1String( "vocation" ) ) {
    a.setPrefix( value );
    return true;
  }

  if ( fieldname == QLatin1String( "cn" ) ) {
    a.setFormattedName( value );
    return true;
  }

  if ( fieldname == QLatin1String( "o" ) ||
       fieldname == QLatin1String( "organization" ) ||      // Exchange
       fieldname == QLatin1String( "organizationname" ) ) { // Exchange
    a.setOrganization( value );
    return true;
  }

  if ( fieldname == QLatin1String( "description" ) ) {
addComment:
    if ( !a.note().isEmpty() )
      a.setNote( a.note() + "\n" );
    a.setNote( a.note() + value );
    return true;
  }

  if ( fieldname == QLatin1String( "custom1" ) ||
       fieldname == QLatin1String( "custom2" ) ||
       fieldname == QLatin1String( "custom3" ) ||
       fieldname == QLatin1String( "custom4" ) ) {
    goto addComment;
  }

  if ( fieldname == QLatin1String( "homeurl" ) ||
       fieldname == QLatin1String( "workurl" ) ) {
    if (a.url().isEmpty()) {
      a.setUrl( KURL( value ) );
      return true;
    }
    if ( a.url().prettyURL() == KURL(value).prettyURL() )
      return true;
    // TODO: current version of kabc only supports one URL.
    // TODO: change this with KDE 4
  }

  if ( fieldname == QLatin1String( "homephone" ) ) {
    a.insertPhoneNumber( PhoneNumber( value, PhoneNumber::Home ) );
    return true;
  }

  if ( fieldname == QLatin1String( "telephonenumber" ) ) {
    a.insertPhoneNumber( PhoneNumber( value, PhoneNumber::Work ) );
    return true;
  }

  if ( fieldname == QLatin1String( "mobile" ) ) { 	// mozilla/Netscape 7
    a.insertPhoneNumber( PhoneNumber( value, PhoneNumber::Cell ) );
    return true;
  }

  if ( fieldname == QLatin1String( "cellphone" ) ) {
    a.insertPhoneNumber( PhoneNumber( value, PhoneNumber::Cell ) );
    return true;
  }

  if ( fieldname == QLatin1String( "pager" )  ||       // mozilla
       fieldname == QLatin1String( "pagerphone" ) ) {  // mozilla
    a.insertPhoneNumber( PhoneNumber( value, PhoneNumber::Pager ) );
    return true;
  }

  if ( fieldname == QLatin1String( "facsimiletelephonenumber" ) ) {
    a.insertPhoneNumber( PhoneNumber( value, PhoneNumber::Fax ) );
    return true;
  }

  if ( fieldname == QLatin1String( "xmozillaanyphone" ) ) { // mozilla
    a.insertPhoneNumber( PhoneNumber( value, PhoneNumber::Work ) );
    return true;
  }

  if ( fieldname == QLatin1String( "street" ) ||
       fieldname == QLatin1String( "streethomeaddress" ) ) {
    homeAddr.setStreet( value );
    return true;
  }

  if ( fieldname == QLatin1String( "postaladdress" ) ) {  // mozilla
    workAddr.setStreet( value );
    return true;
  }

  if ( fieldname == QLatin1String( "mozillapostaladdress2" ) ) {  // mozilla
    workAddr.setStreet( workAddr.street() + QLatin1String( "\n" ) + value );
    return true;
  }

  if ( fieldname == QLatin1String( "postalcode" ) ) {
    workAddr.setPostalCode( value );
    return true;
  }

  if ( fieldname == QLatin1String( "postofficebox" ) ) {
    workAddr.setPostOfficeBox( value );
    return true;
  }

  if ( fieldname == QLatin1String( "homepostaladdress" ) ) {  // Netscape 7
    homeAddr.setStreet( value );
    return true;
  }

  if ( fieldname == QLatin1String( "mozillahomepostaladdress2" ) ) {  // mozilla
    homeAddr.setStreet( homeAddr.street() + QLatin1String( "\n" ) + value );
    return true;
  }

  if ( fieldname == QLatin1String( "mozillahomelocalityname" ) ) {  // mozilla
    homeAddr.setLocality( value );
    return true;
  }

  if ( fieldname == QLatin1String( "mozillahomestate" )	) { // mozilla
    homeAddr.setRegion( value );
    return true;
  }

  if ( fieldname == QLatin1String( "mozillahomepostalcode" ) ) {  // mozilla
    homeAddr.setPostalCode( value );
    return true;
  }

  if ( fieldname == QLatin1String( "mozillahomecountryname" ) ) { // mozilla
    if ( value.length() <= 2 )
      value = Address::ISOtoCountry(value);
    homeAddr.setCountry( value );
    return true;
  }

  if ( fieldname == QLatin1String( "locality" ) ) {
    workAddr.setLocality( value );
    return true;
  }

  if ( fieldname == QLatin1String( "streetaddress" ) ) { // Netscape 4.x
    workAddr.setStreet( value );
    return true;
  }

  if ( fieldname == QLatin1String( "countryname" ) ||
       fieldname == QLatin1String( "c" ) ) {  // mozilla
    if ( value.length() <= 2 )
      value = Address::ISOtoCountry(value);
    workAddr.setCountry( value );
    return true;
  }

  if ( fieldname == QLatin1String( "l" ) ) {  // mozilla
    workAddr.setLocality( value );
    return true;
  }

  if ( fieldname == QLatin1String( "st" ) ) {
    workAddr.setRegion( value );
    return true;
  }

  if ( fieldname == QLatin1String( "ou" ) ) {
    a.setRole( value );
    return true;
  }

  if ( fieldname == QLatin1String( "department" ) ) {
    a.insertCustom( "KADDRESSBOOK", "X-Department", value );
    return true;
  }

  if ( fieldname == QLatin1String( "member" ) ) {
    // this is a mozilla list member (cn=xxx, mail=yyy)
    QStringList list( QStringList::split( ',', value ) );
    QString name, email;

    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
      if ( (*it).startsWith( "cn=" ) )
        name = (*it).mid( 3 ).trimmed();
      if ( (*it).startsWith( "mail=" ) )
        email = (*it).mid( 5 ).trimmed();
    }
    if ( !name.isEmpty() && !email.isEmpty() )
      email = " <" + email + ">";
    a.insertEmail( name + email );
    a.insertCategory( i18n( "List of Emails" ) );
    return true;
  }

  if ( fieldname == QLatin1String( "modifytimestamp" ) ) {
    if (value == QLatin1String("0Z")) // ignore
      return true;
    QDateTime dt = VCardStringToDate( value );
    if ( dt.isValid() ) {
      a.setRevision(dt);
      return true;
    }
  }

  if ( fieldname == QLatin1String( "objectclass" ) ) // ignore
    return true;

  kdWarning() << QString("LDIFConverter: Unknown field for '%1': '%2=%3'\n")
                             .arg(a.formattedName()).arg(fieldname).arg(value);

  return true;
}

/* The following functions are obsoleted. Similar functionality can be found
 * in the LDIF class */

bool LDIFConverter::parseSingleLine( Addressee &a, Address &homeAddr,
                                     Address &workAddr, QString &line )
{
  if ( line.isEmpty() )
    return true;

  QString fieldname, value;
  QByteArray val;

  LDIF::splitLine( line.latin1(), fieldname, val );
  value = QString::fromUtf8( val.data(), val.size() );
  return evaluatePair( a, homeAddr, workAddr, fieldname, value);
}


bool LDIFConverter::splitLine( QString &line, QString &fieldname, QString &value)
{
  QByteArray val;
  bool ret = LDIF::splitLine( line.latin1(), fieldname, val );
  value = QString::fromUtf8( val.data(), val.size() );
  return ret;
}


QString LDIFConverter::makeLDIFfieldString( QString formatStr, QString value, bool allowEncode )
{
  if ( value.isEmpty() )
    return QString();

  // append format if not given
  if (formatStr.find(':') == -1)
    formatStr.append(": %1\n");

  // check if base64-encoding is needed
  bool printable = true;
  unsigned int i, len;
  len = value.length();
  for (i = 0; i<len; ++i ) {
     if (!value[i].isPrint()) {
        printable = false;
        break;
     }
  }

  if (printable) // always encode if we find special chars...
    printable = (value.find('\n') == -1);

  if (!printable && allowEncode) {
    // encode to base64
    value = KCodecs::base64Encode( value.toUtf8() );
    int p = formatStr.find(':');
    if (p>=0)
      formatStr.insert(p, ':');
  }

  // generate the new string and split it to 72 chars/line
  QByteArray txt = (formatStr.arg(value)).toUtf8();

  if (allowEncode) {
    len = txt.length();
    if (len && txt[len-1] == '\n')
      --len;
    i = 72;
    while (i < len) {
      txt.insert(i, "\n ");
      i += 72+1;
      len += 2;
    }
  }

  return QString::fromUtf8(txt);
}

