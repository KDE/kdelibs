/*
    This file is part of libkabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

    $Id$
*/

#include <qsqldatabase.h>
#include <qsqlcursor.h>

#include <kdebug.h>
#include <klineedit.h>
#include <klocale.h>

#include "resourcesql.h"
#include "resourcesqlconfig.h"

using namespace KABC;

extern "C"
{
    ResourceConfigWidget *config_widget( QWidget *parent )
    {
	return new ResourceSqlConfig( parent, "ResourceSqlConfig" );
    }

    Resource *resource( AddressBook *ab, const KConfig *config )
    {
	return new ResourceSql( ab, config );
    }
}

ResourceSql::ResourceSql( AddressBook *ab, const KConfig *config ) :
    Resource( ab ), mDb( 0 )
{
    QString user, password, db, host;

    user = config->readEntry( "SqlUser" );
    password = cryptStr( config->readEntry( "SqlPassword " ) );
    db = config->readEntry( "SqlName" );
    host = config->readEntry( "SqlHost" );

    init( user, password, db, host );
}

ResourceSql::ResourceSql( AddressBook *ab, const QString &user,
	const QString &password, const QString &db, const QString &host ) :
  Resource( ab ), mDb( 0 )
{
    init( user, password, db, host );
}

void ResourceSql::init( const QString &user, const QString &password,
	const QString &db, const QString &host )
{
    mUser = user;
    mPassword = password;
    mDbName = db;
    mHost = host;
}

Ticket *ResourceSql::requestSaveTicket()
{
    if ( !addressBook() ) {
	kdDebug(5700) << "no addressbook" << endl;
	return 0;
    }

    return createTicket( this );
}

bool ResourceSql::open()
{
    QStringList drivers = QSqlDatabase::drivers();
    for ( QStringList::Iterator it = drivers.begin(); it != drivers.end(); ++it ) {
	kdDebug(5700) << "Driver: " << (*it) << endl;
    }

    mDb = QSqlDatabase::addDatabase( "QMYSQL3" );

    if ( !mDb ) {
	kdDebug(5700) << "Error. Can't connect to database." << endl;
	return false;
    }

    mDb->setDatabaseName( mDbName );
    mDb->setUserName( mUser );
    mDb->setPassword( mPassword );
    mDb->setHostName( mHost );

    if ( !mDb->open() ) {
	kdDebug(5700) << "Error. Can't open database '" << mDbName << "'." << endl;
	return false;
    }

    return true;
}

void ResourceSql::close()
{
    mDb->close();
}

bool ResourceSql::load()
{
    QSqlQuery query( "select addressId, name, familyName, givenName, "
	    "additionalName, prefix, suffix, nickname, birthday, "
	    "mailer, timezone, geo_latitude, geo_longitude, title, "
	    "role, organization, note, productId, revision, "
	    "sortString, url from kaddressbook_main_" + mUser );

    while ( query.next() ) {
	QString addrId = query.value(0).toString();

	Addressee addr;
	addr.setResource( this );
	addr.setUid( addrId );
	addr.setName( query.value(1).toString() );
	addr.setFamilyName( query.value(2).toString() );
	addr.setGivenName( query.value(3).toString() );
	addr.setAdditionalName( query.value(4).toString() );
	addr.setPrefix( query.value(5).toString() );
	addr.setSuffix( query.value(6).toString() );
	addr.setNickName( query.value(7).toString() );
	addr.setBirthday( query.value(8).toDateTime() );
	addr.setMailer( query.value(9).toString() );
	addr.setTimeZone( TimeZone( query.value(10).toInt() ) );
	addr.setGeo( Geo( query.value(11).toDouble(), query.value(12).toDouble() ) );
	addr.setTitle( query.value(13).toString() );
	addr.setRole( query.value(14).toString() );
	addr.setOrganization( query.value(15).toString() );
	addr.setNote( query.value(16).toString() );
	addr.setProductId( query.value(17).toString() );
	addr.setRevision( query.value(18).toDateTime() );
	addr.setSortString( query.value(19).toString() );
	addr.setUrl( query.value(20).toString() );

	// emails
	{
	    QSqlQuery emailsQuery( "select email, preferred from kaddressbook_emails "
		    "where addressId = '" + addrId + "'" );
	    while ( emailsQuery.next() )
		addr.insertEmail( emailsQuery.value( 0 ).toString(),
			emailsQuery.value( 1 ).toInt() );
	}

	// phones
	{
	    QSqlQuery phonesQuery( "select number, type from kaddressbook_phones "
		    "where addressId = '" + addrId + "'" );
	    while ( phonesQuery.next() )
		addr.insertPhoneNumber( PhoneNumber( phonesQuery.value( 0 ).toString(),
			phonesQuery.value( 1 ).toInt() ) );
	}    

	// addresses
	{
	    QSqlQuery addressesQuery( "select postOfficeBox, extended, street, "
		    "locality, region, postalCode, country, label, type "
		    "from kaddressbook_addresses where addressId = '" + addrId + "'" );
	    while ( addressesQuery.next() ) {
		Address a;
		a.setPostOfficeBox( addressesQuery.value(0).toString() );
		a.setExtended( addressesQuery.value(1).toString() );
		a.setStreet( addressesQuery.value(2).toString() );
		a.setLocality( addressesQuery.value(3).toString() );
		a.setRegion( addressesQuery.value(4).toString() );
		a.setPostalCode( addressesQuery.value(5).toString() );
		a.setCountry( addressesQuery.value(6).toString() );
		a.setLabel( addressesQuery.value(7).toString() );
		a.setType( addressesQuery.value(8).toInt() );

		addr.insertAddress( a );
	    }
	}

	// categories
	{
	    QSqlQuery categoriesQuery( "select category from kaddressbook_categories "
		    "where addressId = '" + addrId + "'" );
	    while ( categoriesQuery.next() )
		addr.insertCategory( categoriesQuery.value( 0 ).toString() );
	}

	// customs
	{
	    QSqlQuery customsQuery( "select app, name, value from kaddressbook_customs "
		    "where addressId = '" + addrId + "'" );
	    while ( customsQuery.next() )
		addr.insertCustom( customsQuery.value( 0 ).toString(),
			customsQuery.value( 1 ).toString(),
			customsQuery.value( 2 ).toString());
	}

	addressBook()->insertAddressee( addr );
	addr.setChanged( false );
    }
 
    return true;
}

bool ResourceSql::save( Ticket * )
{
    // we have to delete all entries for this user and reinsert them
    QSqlQuery query( "select addressId from kaddressbook_main_" + mUser );

    while ( query.next() ) {
	QString addrId = query.value( 0 ).toString();
	QSqlQuery q;
	
	q.exec( "DELETE FROM kaddressbook_emails WHERE addressId = '" + addrId + "'" );
	q.exec( "DELETE FROM kaddressbook_phones WHERE addressId = '" + addrId + "'" );
	q.exec( "DELETE FROM kaddressbook_addresses WHERE addressId = '" + addrId + "'" );
	q.exec( "DELETE FROM kaddressbook_categories WHERE addressId = '" + addrId + "'" );
	q.exec( "DELETE FROM kaddressbook_customs WHERE addressId = '" + addrId + "'" );

	q.exec( "DELETE FROM kaddressbook_main_" + mUser + " WHERE addressId = '" + addrId + "'" );
    }

    // let's start...
    AddressBook::Iterator it;
    for ( it = addressBook()->begin(); it != addressBook()->end(); ++it ) {
	if ( (*it).resource() != this && (*it).resource() != 0 ) // save only my and new entries
	    continue;

	QString uid = (*it).uid();

	query.exec( "INSERT INTO kaddressbook_main_" + mUser + " VALUES ('" +
	    (*it).uid() + "','" +
	    (*it).name() + "','" +
	    (*it).familyName() + "','" +
	    (*it).givenName() + "','" +
	    (*it).additionalName() + "','" +
	    (*it).prefix() + "','" +
	    (*it).suffix() + "','" +
	    (*it).nickName() + "','" +
	    (*it).birthday().toString( Qt::ISODate ) + "','" +
	    (*it).mailer() + "','" +
	    QString::number( (*it).timeZone().offset() ) + "','" +
	    QString::number( (*it).geo().latitude() ) + "','" +
	    QString::number( (*it).geo().longitude() ) + "','" +
	    (*it).title() + "','" +
	    (*it).role() + "','" +
	    (*it).organization() + "','" +
	    (*it).note() + "','" +
	    (*it).productId() + "','" +
	    (*it).revision().toString( Qt::ISODate ) + "','" +
	    (*it).sortString() + "','" +
	    (*it).url().url() + "')"
	);

	// emails
	{
	    QStringList emails = (*it).emails();
	    QStringList::ConstIterator it;
	    bool preferred = true;
	    for( it = emails.begin(); it != emails.end(); ++it ) {
		query.exec("INSERT INTO kaddressbook_emails VALUES ('" +
			uid + "','" +
			(*it) + "','" +
			QString::number(preferred) + "')");
		preferred = false;
	    }
	}

	// phonenumbers
	{
	    PhoneNumber::List phoneNumberList = (*it).phoneNumbers();
	    PhoneNumber::List::ConstIterator it;
	    for( it = phoneNumberList.begin(); it != phoneNumberList.end(); ++it ) {
		query.exec("INSERT INTO kaddressbook_phones VALUES ('" +
			uid + "','" +
			(*it).number() + "','" +
			QString::number( (*it).type() ) + "')");
	    }
	}

	// postal addresses
	{
	    Address::List addressList = (*it).addresses();
	    Address::List::ConstIterator it;
	    for( it = addressList.begin(); it != addressList.end(); ++it ) {
		query.exec("INSERT INTO kaddressbook_addresses VALUES ('" +
			uid + "','" +
			(*it).postOfficeBox() + "','" +
			(*it).extended() + "','" +
			(*it).street() + "','" +
			(*it).locality() + "','" +
			(*it).region() + "','" +
			(*it).postalCode() + "','" +
			(*it).country() + "','" +
			(*it).label() + "','" +
			QString::number( (*it).type() ) + "')");
	    }
	}

	// categories
	{
	    QStringList categories = (*it).categories();
	    QStringList::ConstIterator it;
	    for( it = categories.begin(); it != categories.end(); ++it )
		query.exec("INSERT INTO kaddressbook_categories VALUES ('" +
			uid + "','" +
			(*it) + "')");
	}

	// customs
	{
	    QStringList list = (*it).customs();
	    QStringList::ConstIterator it;
	    for( it = list.begin(); it != list.end(); ++it ) {
		int dashPos = (*it).find( '-' );
		int colonPos = (*it).find( ':' );
		QString app = (*it).left( dashPos );
		QString name = (*it).mid( dashPos + 1, colonPos - dashPos - 1 );
		QString value = (*it).right( (*it).length() - colonPos - 1 );

		query.exec("INSERT INTO kaddressbook_categories VALUES ('" +
			uid + "','" + app + "','" + name + "','" + value + "')");
	    }
	}
    }

    return true;
}

QString ResourceSql::identifier() const
{
    return mHost + "_" + mDbName;
}

QString ResourceSql::typeInfo() const
{
    return i18n( "sql" );
}
