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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qstring.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qtextstream.h>

#include <klocale.h>
#include <kdebug.h>
#include <kmdcodec.h>

#include "addressee.h"
#include "address.h"

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

static void ldif_out( QTextStream &t, const QString &str, const QString &field )
{
  if ( field.isEmpty() )
    return;
  t << str.arg( field );
}

bool LDIFConverter::addresseeToLDIF( const Addressee &addr, QString &str )
{
  if ( addr.isEmpty() )
      return false;

  QTextStream t( str, IO_WriteOnly|IO_Append );
  t.setEncoding( QTextStream::UnicodeUTF8 );

  const Address homeAddr = addr.address( Address::Home );
  const Address workAddr = addr.address( Address::Work );

  ldif_out( t, "%1", QString( "dn: cn=%1,mail=%2\n" )
            .arg( addr.formattedName() )
            .arg( addr.preferredEmail() ) );
  ldif_out( t, "givenname: %1\n", addr.givenName() );
  ldif_out( t, "sn: %1\n", addr.familyName() );
  ldif_out( t, "cn: %1\n", addr.formattedName() );
  ldif_out( t, "uid: %1\n", addr.uid() );
  ldif_out( t, "nickname: %1\n", addr.nickName() );
  ldif_out( t, "xmozillanickname: %1\n", addr.nickName() );

  ldif_out( t, "mail: %1\n", addr.preferredEmail() );
  if ( addr.emails().count() > 1 )
    ldif_out( t, "mozillasecondemail: %1\n", addr.emails()[ 1 ] );
//ldif_out( t, "mozilla_AIMScreenName: %1\n", "screen_name" );

  ldif_out( t, "telephonenumber: %1\n", addr.phoneNumber( PhoneNumber::Work ).number() );
  ldif_out( t, "facsimiletelephonenumber: %1\n", addr.phoneNumber( PhoneNumber::Fax ).number() );
  ldif_out( t, "homephone: %1\n", addr.phoneNumber( PhoneNumber::Home ).number() );
  ldif_out( t, "mobile: %1\n", addr.phoneNumber( PhoneNumber::Cell ).number() );
  ldif_out( t, "cellphone: %1\n", addr.phoneNumber( PhoneNumber::Cell ).number() );
  ldif_out( t, "pager: %1\n", addr.phoneNumber( PhoneNumber::Pager ).number() );

  ldif_out( t, "streethomeaddress: %1\n", homeAddr.street() );
  ldif_out( t, "postalcode: %1\n", workAddr.postalCode() );

  QStringList streets = QStringList::split( '\n', homeAddr.street() );
  if ( streets.count() > 0 )
    ldif_out( t, "homepostaladdress: %1\n", streets[ 0 ] );
  if ( streets.count() > 1 )
    ldif_out( t, "mozillahomepostaladdress2: %1\n", streets[ 1 ] );
  ldif_out( t, "mozillahomelocalityname: %1\n", homeAddr.locality() );
  ldif_out( t, "mozillahomestate: %1\n", homeAddr.region() );
  ldif_out( t, "mozillahomepostalcode: %1\n", homeAddr.postalCode() );
  ldif_out( t, "mozillahomecountryname: %1\n", homeAddr.country() );
  ldif_out( t, "locality: %1\n", workAddr.locality() );
  ldif_out( t, "streetaddress: %1\n", workAddr.street() );

  streets = QStringList::split( '\n', workAddr.street() );
  if ( streets.count() > 0 )
    ldif_out( t, "postaladdress: %1\n", streets[ 0 ] );
  if ( streets.count() > 1 )
    ldif_out( t, "mozillapostaladdress2: %1\n", streets[ 1 ] );
  ldif_out( t, "countryname: %1\n", workAddr.country() );
  ldif_out( t, "l: %1\n", workAddr.locality() );
  ldif_out( t, "c: %1\n", workAddr.country() );
  ldif_out( t, "st: %1\n", workAddr.region() );

  ldif_out( t, "title: %1\n", addr.title() );
  ldif_out( t, "ou: %1\n", addr.role() );
  ldif_out( t, "o: %1\n", addr.organization() );
  ldif_out( t, "organization: %1\n", addr.organization() );
  ldif_out( t, "organizationname: %1\n", addr.organization() );
  ldif_out( t, "department: %1\n", addr.custom("KADDRESSBOOK", "X-Department") );
  ldif_out( t, "workurl: %1\n", addr.url().prettyURL() );
  ldif_out( t, "homeurl: %1\n", addr.url().prettyURL() );
  ldif_out( t, "description:: %1\n", KCodecs::base64Encode( addr.note().utf8() ) );
  if (addr.revision().isValid())
    ldif_out(t, "modifytimestamp: %1\n", dateToVCardString( addr.revision()) );

  t << "objectclass: top\n";
  t << "objectclass: person\n";
  t << "objectclass: organizationalPerson\n";

  t << "\n";

  return true;
}


/* convert from LDIF stream */

bool LDIFConverter::LDIFToAddressee( const QString &str, AddresseeList &addrList, QDateTime dt )
{
  QStringList lines;

  if (!dt.isValid())
    dt = QDateTime::currentDateTime();

  lines = QStringList::split( QRegExp("[\x0d\x0a]"), str, false );

  // clean up comments and prepare folded entries and multi-line BASE64 encoded lines
  QStringList::Iterator last = lines.end();
  for ( QStringList::Iterator it = lines.begin(); it != lines.end(); ++it ) {
    if ( (*it).startsWith("#") ) { // comment ?
	lines.remove(it);
	continue;
    }
    if ( last == lines.end() ) {
	last = it;
	continue;
    }
    if ((*last).find("::")!=-1 && (*it).find(":")==-1) { // this is a multi-line BASE64
	*last += (*it);
	lines.remove(it);
	continue;
    }
    if ((*last).find(":")!=-1 && (*it).startsWith(" ")) { // this is a folded item
	*last += (*it).mid(1);
        lines.remove(it);
	continue;
    }
    last = it;
  }

  // variables
  addrList = AddresseeList();

  Addressee a;
  Address homeAddr, workAddr;
  bool ok;

  // do the loop...
  for ( QStringList::Iterator it = lines.begin(); it != lines.end(); ++it ) {

    // create a new (empty) address entry
    a = Addressee();
    a.setRevision(dt);
    homeAddr = Address( Address::Home );
    workAddr = Address( Address::Work );

    do {
      ok = parseSingleLine( a, homeAddr, workAddr, *it );
      if (ok && it!=lines.end()) {
	++it;
      }
    } while (ok && it!=lines.end());

    // if new address it is not empty, append it
    if ( !a.formattedName().isEmpty() || !a.name().isEmpty() || 
         !a.familyName().isEmpty() ) {
      a.insertAddress( homeAddr );
      a.insertAddress( workAddr );
      addrList.append( a );
    }

    if ( it == lines.end() )
	break;
  } // for()...

  return true;
};

bool LDIFConverter::parseSingleLine( Addressee &a, 
	Address &homeAddr, Address &workAddr, QString &line )
{
  if ( line.isEmpty() )
    return true;

  QString fieldname, value;
  splitLine( line, fieldname, value);
  return evaluatePair( a, homeAddr, workAddr, fieldname, value);
}


bool LDIFConverter::splitLine( QString &line, QString &fieldname, QString &value)
{
  int position;

  position = line.find( "::" );
  if ( position != -1 ) {
    // String is BASE64 encoded
    fieldname = line.left( position ).lower();
    value = QString::fromUtf8( KCodecs::base64Decode(
              line.mid( position + 3, line.length() - position - 2 ).latin1() ) )
              .simplifyWhiteSpace();
    return true;
  }

  position = line.find( ":" );
  if ( position != -1 ) {
    fieldname = line.left( position ).lower();
    // Convert Utf8 string to unicode so special characters are preserved
    // We need this since we are reading normal strings from the file
    // which are not converted automatically
    value = QString::fromUtf8( line.mid( position + 2, line.length() - position - 2 ).latin1() );
    return true;
  }
  
  // strange: we did not find a fieldname
  fieldname = "";
  value = line;
  return true;
}


bool LDIFConverter::evaluatePair( Addressee &a, 
	Address &homeAddr, Address &workAddr, 
        QString &fieldname, QString &value )
{
  if ( fieldname == QString::fromLatin1( "dn" ) ) // ignore & return false!
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

  if ( fieldname == QString::fromLatin1( "givenname" ) ) {
    a.setGivenName( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "xmozillanickname") || 
       fieldname == QString::fromLatin1( "nickname") ) {
    a.setNickName( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "sn" ) ) {
    a.setFamilyName( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "uid" ) ) {
    a.setUid( value );
    return true;
  }
  if ( fieldname == QString::fromLatin1( "mail" ) ) {
    a.insertEmail( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "mozillasecondemail" ) ) { // mozilla
    a.insertEmail( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "title" ) ) {
    a.setTitle( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "cn" ) ) {
    a.setFormattedName( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "o" ) || 
       fieldname == QString::fromLatin1( "organization" ) ||      // Exchange
       fieldname == QString::fromLatin1( "organizationname" ) ) { // Exchange
    a.setOrganization( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "description" ) ) {
addComment:
    if ( !a.note().isEmpty() )
      a.setNote( a.note() + "\n" );
    a.setNote( a.note() + value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "custom1" ) ||
       fieldname == QString::fromLatin1( "custom2" ) ||
       fieldname == QString::fromLatin1( "custom3" ) ||
       fieldname == QString::fromLatin1( "custom4" ) ) {
    goto addComment;
  }

  if ( fieldname == QString::fromLatin1( "homeurl" ) ||
       fieldname == QString::fromLatin1( "workurl" ) ) {
    if (a.url().isEmpty()) {
      a.setUrl( value );
      return true;
    }
    if ( a.url().prettyURL() == KURL(value).prettyURL() )
      return true;
    // TODO: current version of kabc only supports one URL.
    // TODO: change this with KDE 4
  }

  if ( fieldname == QString::fromLatin1( "homephone" ) ) {
    a.insertPhoneNumber( PhoneNumber( value, PhoneNumber::Home ) );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "telephonenumber" ) ) {
    a.insertPhoneNumber( PhoneNumber( value, PhoneNumber::Work ) );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "mobile" ) ) { // mozilla
    a.insertPhoneNumber( PhoneNumber( value, PhoneNumber::Cell ) );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "cellphone" ) ) {
    a.insertPhoneNumber( PhoneNumber( value, PhoneNumber::Cell ) );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "pager" ) ) {  // mozilla
    a.insertPhoneNumber( PhoneNumber( value, PhoneNumber::Pager ) );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "facsimiletelephonenumber" ) ) {
    a.insertPhoneNumber( PhoneNumber( value, PhoneNumber::Fax ) );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "xmozillaanyphone" ) ) { // mozilla
    a.insertPhoneNumber( PhoneNumber( value, PhoneNumber::Work ) );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "streethomeaddress" ) ) {
    homeAddr.setStreet( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "postaladdress" ) ) {  // mozilla
    workAddr.setStreet( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "mozillapostaladdress2" ) ) {  // mozilla
    workAddr.setStreet( workAddr.street() + QString::fromLatin1( "\n" ) + value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "postalcode" ) ) {
    workAddr.setPostalCode( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "homepostaladdress" ) ) {  // mozilla
    homeAddr.setStreet( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "mozillahomepostaladdress2" ) ) {  // mozilla
    homeAddr.setStreet( homeAddr.street() + QString::fromLatin1( "\n" ) + value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "mozillahomelocalityname" ) ) {  // mozilla
    homeAddr.setLocality( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "mozillahomestate" )	) { // mozilla
    homeAddr.setRegion( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "mozillahomepostalcode" ) ) {  // mozilla
    homeAddr.setPostalCode( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "mozillahomecountryname" ) ) { // mozilla
    if ( value.length() > 2 )
	value = Address::countryToISO(value);
    homeAddr.setCountry( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "locality" ) ) {
    workAddr.setLocality( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "streetaddress" ) ) {
    workAddr.setStreet( value );
    return true;
  }
  
  if ( fieldname == QString::fromLatin1( "countryname" ) ) {
    if ( value.length() > 2 )
	value = Address::countryToISO(value);
    workAddr.setCountry( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "l" ) ) {  // mozilla
    workAddr.setLocality( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "c" ) ) {  // mozilla
    workAddr.setCountry( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "st" ) ) {
    workAddr.setRegion( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "ou" ) ) {
    a.setRole( value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "department" ) ) {
    a.insertCustom( "KADDRESSBOOK", "X-Department", value );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "member" ) ) {
    // this is a mozilla list member (cn=xxx, mail=yyy)
    QStringList list( QStringList::split( ',', value ) );
    QString name, email;

    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
      if ( (*it).startsWith( "cn=" ) )
        name = (*it).mid( 3 ).stripWhiteSpace();
      if ( (*it).startsWith( "mail=" ) )
        email = (*it).mid( 5 ).stripWhiteSpace();
    }
    if ( !name.isEmpty() && !email.isEmpty() )
      email = " <" + email + ">";
    a.insertEmail( name + email );
    a.insertCategory( i18n( "List of E-Mails" ) );
    return true;
  }

  if ( fieldname == QString::fromLatin1( "modifytimestamp" ) ) {
    QDateTime dt = VCardStringToDate( value );
    if ( dt.isValid() ) {
        a.setRevision(dt);
        return true;
    }
  }

  if ( fieldname == QString::fromLatin1( "objectclass" ) ) // ignore 
    return true;

  kdWarning() << QString("LDIFConverter: Unknown field: '%1=%2' for '%3'\n")
	.arg(fieldname).arg(value).arg(a.formattedName());

  return true;
}


/*

URL: 
http://tldp.org/HOWTO/LDAP-Implementation-HOWTO/schemas.html
http://www.faqs.org/rfcs/rfc2849.html

# Addressbook related classes

objectclass netscapeaddressbook
        requires
                objectclass,
                cn
        allows
                cellphone,
                countryname,
                description,
                facsimiletelephonenumber,
                givenname,
                homephone,
                homeurl,
                locality,
                mail,
                nickname,
                o,
                ou,
                pagerphone,
                postalcode,
                sn,
                st,
                streetaddress,
                telephonenumber,
                title,
                xmozillanickname,
                xmozillausehtmlmail,
                xmozillaanyphone

objectclass microsoftaddressbook
        requires
                objectclass,
                cn
        allows
                c,
                department,
                facsimiletelephonenumber,
                givenname,
                homephone,
                homepostaladdress,
                info,
                initials,
                l,
                mail,
                mobile,
                organizationname,
                otherfacsimiletelephonenumber,
                otherpager,
                physicaldeliveryofficename,
                postaladdress,
                postalcode,
                sn,
                st,
                telephonenumber,
                title,
                url
*/
