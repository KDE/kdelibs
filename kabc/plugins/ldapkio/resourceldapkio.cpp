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

#include <kdebug.h>
#include <kglobal.h>
#include <klineedit.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstringhandler.h>

#include <stdlib.h>

#include <kabc/ldifconverter.h>

#include "resourceldapkio.h"
#include "resourceldapkioconfig.h"

using namespace KABC;

ResourceLDAPKIO::ResourceLDAPKIO( const KConfig *config )
  : Resource( config ), mGetCounter( 0 ), mErrorOccured( false )
{
  if ( config ) {
    QMap<QString, QString> attrList;
    QStringList attributes = config->readListEntry( "LdapAttributes" );
    for ( uint pos = 0; pos < attributes.count(); pos += 2 )
      mAttributes.insert( attributes[ pos ], attributes[ pos + 1 ] );

    mUser = config->readEntry( "LdapUser" );
    mPassword = KStringHandler::obscure( config->readEntry( "LdapPassword" ) );
    mDn = config->readEntry( "LdapDn" );
    mHost = config->readEntry( "LdapHost" );
    mPort = config->readNumEntry( "LdapPort", 389 );
    mFilter = config->readEntry( "LdapFilter" );
    mAnonymous = config->readBoolEntry( "LdapAnonymous" );
  } else {
    mPort = 389;
    mAnonymous = true;
  }

  init();
}

void ResourceLDAPKIO::init()
{
  /**
    If you want to add new attributes, append them here, add a
    translation string in the ctor of AttributesDialog and
    handle them in the load() method below.
    These are the default values
   */
  if ( mPort == 0 ) mPort = 389;
  if ( mUser.isEmpty() && mPassword.isEmpty() ) mAnonymous = true;

  if ( mAttributes.count() == 0 ) {
    mAttributes.insert( "commonName", "cn" );
    mAttributes.insert( "formattedName", "displayName" );
    mAttributes.insert( "familyName", "sn" );
    mAttributes.insert( "givenName", "givenName" );
    mAttributes.insert( "mail", "mail" );
    mAttributes.insert( "mailAlias", "" );
    mAttributes.insert( "phoneNumber", "telephoneNumber" );
    mAttributes.insert( "uid", "uid" );
  }

  mLDAPUrl.setProtocol( "ldap" );
  if ( !mAnonymous ) {
    mLDAPUrl.setUser( mUser );
    mLDAPUrl.setPass( mPassword );
  }
  mLDAPUrl.setHost( mHost );
  mLDAPUrl.setPort( mPort );
  mLDAPUrl.setPath( "/" + mDn );

  QString query = "?dn?sub";
  if ( !mFilter.isEmpty() )
    query += "?" + mFilter;

  mLDAPUrl.setQuery( query );
}

void ResourceLDAPKIO::writeConfig( KConfig *config )
{
  Resource::writeConfig( config );

  config->writeEntry( "LdapUser", mUser );
  config->writeEntry( "LdapPassword", KStringHandler::obscure( mPassword ) );
  config->writeEntry( "LdapDn", mDn );
  config->writeEntry( "LdapHost", mHost );
  config->writeEntry( "LdapPort", mPort );
  config->writeEntry( "LdapFilter", mFilter );
  config->writeEntry( "LdapAnonymous", mAnonymous );

  QStringList attributes;
  QMap<QString, QString>::Iterator it;
  for ( it = mAttributes.begin(); it != mAttributes.end(); ++it )
    attributes << it.key() << it.data();

  config->writeEntry( "LdapAttributes", attributes );
}

Ticket *ResourceLDAPKIO::requestSaveTicket()
{
  if ( !addressBook() ) {
    kdDebug(5700) << "no addressbook" << endl;
    return 0;
  }

  return createTicket( this );
}

void ResourceLDAPKIO::releaseSaveTicket( Ticket *ticket )
{
  delete ticket;
}

bool ResourceLDAPKIO::doOpen()
{
  return true;
}

void ResourceLDAPKIO::doClose()
{
}

bool ResourceLDAPKIO::load()
{
  return true;
}

bool ResourceLDAPKIO::asyncLoad()
{
  KIO::Job *job = KIO::listDir( mLDAPUrl, false, false );

  connect( job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ),
           this, SLOT( entries( KIO::Job*, const KIO::UDSEntryList& ) ) );

  return true;
}

void ResourceLDAPKIO::entries( KIO::Job*, const KIO::UDSEntryList &list )
{
  KIO::UDSEntryList::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    KIO::UDSEntry::ConstIterator atomIt;

    bool isFile = false;
    for ( atomIt = (*it).begin(); atomIt != (*it).end(); ++atomIt ) {
      if ( (*atomIt).m_uds == KIO::UDS_FILE_TYPE && (*atomIt).m_long == S_IFREG )
        isFile = true;
    }

    if ( isFile ) {
      for ( atomIt = (*it).begin(); atomIt != (*it).end(); ++atomIt ) {
        if ( (*atomIt).m_uds == KIO::UDS_URL ) {
          mGetCounter++;
          KIO::Job *job = KIO::get( (*atomIt).m_str, true, false );
          connect( job, SIGNAL( data( KIO::Job*, const QByteArray& ) ),
                   this, SLOT( data( KIO::Job*, const QByteArray& ) ) );
          connect( job, SIGNAL( result( KIO::Job* ) ),
                   this, SLOT( result( KIO::Job* ) ) );

          mJobMap.insert( job, QByteArray() );
        }
      }
    }
  }
}

void ResourceLDAPKIO::data( KIO::Job *job, const QByteArray &d )
{
  QByteArray &data = mJobMap[ job ];

  unsigned int oldSize = data.size();
  data.resize( oldSize + d.size() );
  memcpy( data.data() + oldSize, d.data(), d.size());
}

void ResourceLDAPKIO::result( KIO::Job *job )
{
  mGetCounter--;

  if ( job->error() ) {
    mErrorOccured = true;
    mErrorMsg = job->errorString();
    mJobMap.remove( job );
    return;
  }

  QByteArray &data = mJobMap[ job ];

  AddresseeList addrList;
  bool ok = LDIFConverter::LDIFToAddressee( data, addrList );

  if ( !ok ) {
    mErrorOccured = true;
    mErrorMsg = i18n( "Error while parsing the LDIF file." );
  } else {
    AddresseeList::Iterator it;
    for ( it = addrList.begin(); it != addrList.end(); ++it ) {
      (*it).setChanged( false );
      (*it).setResource( this );
      insertAddressee( *it );
    }
  }

  mJobMap.remove( job );

  if ( mGetCounter == 0 ) {
    if ( mErrorOccured )
      emit loadingError( this, mErrorMsg );
    else
      emit loadingFinished( this );
  }
}

bool ResourceLDAPKIO::save( Ticket* )
{
  return false; // readonly
}

bool ResourceLDAPKIO::asyncSave( Ticket* )
{
  return false; // readonly
}

void ResourceLDAPKIO::removeAddressee( const Addressee& )
{
}

void ResourceLDAPKIO::setUser( const QString &user )
{
  mUser = user;
}

QString ResourceLDAPKIO::user() const
{
  return mUser;
}

void ResourceLDAPKIO::setPassword( const QString &password )
{
  mPassword = password;
}

QString ResourceLDAPKIO::password() const
{
  return mPassword;
}

void ResourceLDAPKIO::setDn( const QString &dn )
{
  mDn = dn;
}

QString ResourceLDAPKIO::dn() const
{
  return mDn;
}

void ResourceLDAPKIO::setHost( const QString &host )
{
  mHost = host;
}

QString ResourceLDAPKIO::host() const
{
  return mHost;
}

void ResourceLDAPKIO::setPort( int port )
{
  mPort = port;
}

int ResourceLDAPKIO::port() const
{
  return mPort;
}

void ResourceLDAPKIO::setFilter( const QString &filter )
{
  mFilter = filter;
}

QString ResourceLDAPKIO::filter() const
{
  return mFilter;
}

void ResourceLDAPKIO::setIsAnonymous( bool value )
{
  mAnonymous = value;
}

bool ResourceLDAPKIO::isAnonymous() const
{
  return mAnonymous;
}

void ResourceLDAPKIO::setAttributes( const QMap<QString, QString> &attributes )
{
  mAttributes = attributes;
}

QMap<QString, QString> ResourceLDAPKIO::attributes() const
{
  return mAttributes;
}

#include "resourceldapkio.moc"
