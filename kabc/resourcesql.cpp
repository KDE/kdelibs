#include <qsqldatabase.h>
#include <qsqlcursor.h>

#include <kdebug.h>

#include "resourcesql.h"

using namespace KABC;

ResourceSql::ResourceSql( AddressBook *ab, const QString &user,
                          const QString &password, const QString &host ) :
  Resource( ab ), mUser( user ), mPassword( password ), mHost( host ), mDb( 0 )
{
}

bool ResourceSql::open()
{
  QStringList drivers = QSqlDatabase::drivers();
  for ( QStringList::Iterator it = drivers.begin(); it != drivers.end(); ++it ) {
    kdDebug() << "Driver: " << (*it) << endl;
  }

  mDb = QSqlDatabase::addDatabase( "QMYSQL3" );

  if ( !mDb ) {
    kdDebug() << "Error. Can't connect to database." << endl;
    return false;
  }

  mDb->setDatabaseName( "address" );
  mDb->setUserName( mUser );
  mDb->setPassword( mPassword );
  mDb->setHostName( mHost );

  if ( !mDb->open() ) {
    kdDebug() << "Error. Can't open database." << endl;
    return false;
  }

  return true;
}

void ResourceSql::close()
{
  mDb->close();
}

bool ResourceSql::load( AddressBook *ab )
{
  QSqlCursor cur( "addresses" ); // Specify the table/view name
  cur.select(); // We'll retrieve every record
  while ( cur.next() ) {
    kdDebug() << cur.value( "entry" ).toString() << endl;
  }

  QSqlQuery query( "select distinct addressId from addresses" );
  while ( query.next() ) {
    QString id = query.value(0).toString();
    kdDebug() << "id: " << id << endl;

    Addressee a;
    a.setUid( id );

    QSqlQuery entryQuery( "select * from addresses where addressId = " + id );
    while ( entryQuery.next() ) {
      int entryKind = entryQuery.value(1).toInt();
      QString entry = entryQuery.value(2).toString();
      kdDebug() << "  " << QString::number( entryKind ) << ": " << entry << endl;
      switch ( entryKind ) {
      	case GivenName:
	  a.setGivenName( entry );
	  break;
	case FamilyName:
	  a.setFamilyName( entry );
	  break;
	case Email:
	  a.insertEmail( entry );
	  break;
	default:
	  break;
      }
    }
    
    ab->insertAddressee( a );
  }
 
  setAddressBook( ab );
  
  return true;
}

bool ResourceSql::save()
{
  return false;
}
