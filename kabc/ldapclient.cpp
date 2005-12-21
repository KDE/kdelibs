/* kldapclient.cpp - LDAP access
 *      Copyright (C) 2002 Klarälvdalens Datakonsult AB
 *
 *      Author: Steffen Hansen <hansen@kde.org>
 *
 *      Ported to KABC by Daniel Molkentin <molkentin@kde.org>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */



#include <qfile.h>
#include <qimage.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qtextstream.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kcodecs.h>
#include <kprotocolinfo.h>

#include "ldapclient.h"
#include "ldif.h"
#include "ldapurl.h"

using namespace KABC;

class LdapClient::LdapClientPrivate{
public:
  QString bindDN;
  QString pwdBindDN;
  LDIF ldif;
};

QString LdapObject::toString() const
{
  QString result = QString::fromLatin1( "\ndn: %1\n" ).arg( dn );
  for ( LdapAttrMap::ConstIterator it = attrs.begin(); it != attrs.end(); ++it ) {
    QString attr = it.key();
    for ( LdapAttrValue::ConstIterator it2 = (*it).begin(); it2 != (*it).end(); ++it2 ) {
      result += QString::fromUtf8( LDIF::assembleLine( attr, *it2, 76 ) ) + "\n";
    }
  }

  return result;
}

void LdapObject::clear()
{
  dn = QString::null;
  attrs.clear();
}

void LdapObject::assign( const LdapObject& that )
{
  if ( &that != this ) {
    dn = that.dn;
    attrs = that.attrs;
    client = that.client;
  }
}

LdapClient::LdapClient( QObject* parent)
  : QObject( parent), mJob( 0 ), mActive( false )
{
  d = new LdapClientPrivate;
}

LdapClient::~LdapClient()
{
  cancelQuery();
  delete d; d = 0;
}

void LdapClient::setHost( const QString& host )
{
  mHost = host;
}

void LdapClient::setPort( const QString& port )
{
  mPort = port;
}

void LdapClient::setBase( const QString& base )
{
  mBase = base;
}

void LdapClient::setBindDN( const QString& bindDN )
{
  d->bindDN = bindDN;
}

void LdapClient::setPwdBindDN( const QString& pwdBindDN )
{
  d->pwdBindDN = pwdBindDN;
}

void LdapClient::setAttrs( const QStringList& attrs )
{
  mAttrs = attrs;
}

void LdapClient::startQuery( const QString& filter )
{
  cancelQuery();
  LDAPUrl url;

  url.setProtocol( "ldap" );
  url.setUser( d->bindDN );
  url.setPass( d->pwdBindDN );
  url.setHost( mHost );
  url.setPort( mPort.toUInt() );
  url.setDn( mBase );
  url.setAttributes( mAttrs );
  url.setScope( mScope == "one" ? LDAPUrl::One : LDAPUrl::Sub );
  url.setFilter( "("+filter+")" );

  kdDebug(5700) << "Doing query: " << url.prettyURL() << endl;

  startParseLDIF();
  mActive = true;
  mJob = KIO::get( url, false, false );
  connect( mJob, SIGNAL( data( KIO::Job*, const QByteArray& ) ),
           this, SLOT( slotData( KIO::Job*, const QByteArray& ) ) );
  connect( mJob, SIGNAL( infoMessage( KIO::Job*, const QString& ) ),
           this, SLOT( slotInfoMessage( KIO::Job*, const QString& ) ) );
  connect( mJob, SIGNAL( result( KIO::Job* ) ),
           this, SLOT( slotDone() ) );
}

void LdapClient::cancelQuery()
{
  if ( mJob ) {
    mJob->kill();
    mJob = 0;
  }

  mActive = false;
}

void LdapClient::slotData( KIO::Job*, const QByteArray& data )
{
#ifndef NDEBUG // don't create the QString
//  QString str( data );
//  kdDebug(5700) << "LdapClient: Got \"" << str << "\"\n";
#endif
  parseLDIF( data );
}

void LdapClient::slotInfoMessage( KIO::Job*, const QString & )
{
  //qDebug("Job said \"%s\"", info.latin1());
}

void LdapClient::slotDone()
{
  endParseLDIF();
  mActive = false;
#if 0
  for ( QList<LdapObject>::Iterator it = mObjects.begin(); it != mObjects.end(); ++it ) {
    qDebug( (*it).toString().latin1() );
  }
#endif
  int err = mJob->error();
  if ( err && err != KIO::ERR_USER_CANCELED ) {
    emit error( KIO::buildErrorString( err, QString("%1:%2").arg( mHost ).arg( mPort ) ) );
  }
  emit done();
}

void LdapClient::startParseLDIF()
{
  mCurrentObject.clear();
  mLastAttrName  = 0;
  mLastAttrValue = 0;
  mIsBase64 = false;
  d->ldif.startParsing();
}

void LdapClient::endParseLDIF()
{
}

void LdapClient::parseLDIF( const QByteArray& data )
{
  if ( data.size() ) {
    d->ldif.setLDIF( data );
  } else {
    d->ldif.endLDIF();
  }

  LDIF::ParseVal ret;
  do {
    ret = d->ldif.nextItem();
    switch ( ret ) {
      case LDIF::Item:
      {
        QString name = d->ldif.attr();
        QByteArray value = d->ldif.val();
        mCurrentObject.attrs[ name ].append( value );
        break;
      }
     case LDIF::EndEntry:
        mCurrentObject.dn = d->ldif.dn();
        mCurrentObject.client = this;
        emit result( mCurrentObject );
        mCurrentObject.clear();
        break;
      default:
        break;
    }
  } while ( ret != LDIF::MoreData );
}

QString LdapClient::bindDN() const
{
  return d->bindDN;
}

QString LdapClient::pwdBindDN() const
{
  return d->pwdBindDN;
}

LdapSearch::LdapSearch()
    : mActiveClients( 0 ), mNoLDAPLookup( false )
{
  if ( !KProtocolInfo::isKnownProtocol( KURL("ldap://localhost") ) ) {
    mNoLDAPLookup = true;
    return;
  }

  // stolen from KAddressBook
  KConfig config( "kabldaprc", true );
  config.setGroup( "LDAP" );
  int numHosts = config.readUnsignedNumEntry( "NumSelectedHosts");
  if ( !numHosts ) {
    mNoLDAPLookup = true;
    return;
  } else {
    for ( int j = 0; j < numHosts; j++ ) {
      LdapClient* ldapClient = new LdapClient( this );

      QString host =  config.readEntry( QString( "SelectedHost%1" ).arg( j ) ).trimmed();
      if ( !host.isEmpty() )
        ldapClient->setHost( host );

      QString port = QString::number( config.readUnsignedNumEntry( QString( "SelectedPort%1" ).arg( j ) ) );
      if ( !port.isEmpty() )
        ldapClient->setPort( port );

      QString base = config.readEntry( QString( "SelectedBase%1" ).arg( j ) ).trimmed();
      if ( !base.isEmpty() )
        ldapClient->setBase( base );

      QString bindDN = config.readEntry( QString( "SelectedBind%1" ).arg( j ) ).trimmed();
      if ( !bindDN.isEmpty() )
        ldapClient->setBindDN( bindDN );

      QString pwdBindDN = config.readEntry( QString( "SelectedPwdBind%1" ).arg( j ) );
      if ( !pwdBindDN.isEmpty() )
        ldapClient->setPwdBindDN( pwdBindDN );

      QStringList attrs;
      attrs << "cn" << "mail" << "givenname" << "sn";
      ldapClient->setAttrs( attrs );

      connect( ldapClient, SIGNAL( result( const KABC::LdapObject& ) ),
               this, SLOT( slotLDAPResult( const KABC::LdapObject& ) ) );
      connect( ldapClient, SIGNAL( done() ),
               this, SLOT( slotLDAPDone() ) );
      connect( ldapClient, SIGNAL( error( const QString& ) ),
               this, SLOT( slotLDAPError( const QString& ) ) );

      mClients.append( ldapClient );
    }
  }

  connect( &mDataTimer, SIGNAL( timeout() ), SLOT( slotDataTimer() ) );
}

void LdapSearch::startSearch( const QString& txt )
{
  if ( mNoLDAPLookup )
    return;

  cancelSearch();

  int pos = txt.find( '\"' );
  if( pos >= 0 )
  {
    ++pos;
    int pos2 = txt.find( '\"', pos );
    if( pos2 >= 0 )
        mSearchText = txt.mid( pos , pos2 - pos );
    else
        mSearchText = txt.mid( pos );
  } else
    mSearchText = txt;

  QString filter = QString( "|(cn=%1*)(mail=%2*)(givenName=%3*)(sn=%4*)" )
      .arg( mSearchText ).arg( mSearchText ).arg( mSearchText ).arg( mSearchText );

  QList< LdapClient* >::Iterator it;
  for ( it = mClients.begin(); it != mClients.end(); ++it ) {
    (*it)->startQuery( filter );
    ++mActiveClients;
  }
}

void LdapSearch::cancelSearch()
{
  QList< LdapClient* >::Iterator it;
  for ( it = mClients.begin(); it != mClients.end(); ++it )
    (*it)->cancelQuery();

  mActiveClients = 0;
  mResults.clear();
}

void LdapSearch::slotLDAPResult( const KABC::LdapObject& obj )
{
  mResults.append( obj );
  if ( !mDataTimer.isActive() )
    mDataTimer.start( 500, true );
}

void LdapSearch::slotLDAPError( const QString& )
{
  slotLDAPDone();
}

void LdapSearch::slotLDAPDone()
{
  if ( --mActiveClients > 0 )
    return;

  finish();
}

void LdapSearch::slotDataTimer()
{
  QStringList lst;
  LdapResultList reslist;
  makeSearchData( lst, reslist );
  if ( !lst.isEmpty() )
    emit searchData( lst );
  if ( !reslist.isEmpty() )
    emit searchData( reslist );
}

void LdapSearch::finish()
{
  mDataTimer.stop();

  slotDataTimer(); // emit final bunch of data
  emit searchDone();
}

void LdapSearch::makeSearchData( QStringList& ret, LdapResultList& resList )
{
  QString search_text_upper = mSearchText.toUpper();

  QList< KABC::LdapObject >::ConstIterator it1;
  for ( it1 = mResults.begin(); it1 != mResults.end(); ++it1 ) {
    QString name, mail, givenname, sn;

    LdapAttrMap::ConstIterator it2;
    for ( it2 = (*it1).attrs.begin(); it2 != (*it1).attrs.end(); ++it2 ) {
      QString tmp = QString::fromUtf8( (*it2).first(), (*it2).first().size() );
      if ( it2.key() == "cn" )
        name = tmp; // TODO loop?
      else if( it2.key() == "mail" )
        mail = tmp;
      else if( it2.key() == "givenName" )
        givenname = tmp;
      else if( it2.key() == "sn" )
        sn = tmp;
    }

    if( mail.isEmpty())
      continue; // nothing, bad entry
    else if ( name.isEmpty() )
      ret.append( mail );
    else {
      kdDebug(5700) << "<" << name << "><" << mail << ">" << endl;
      ret.append( QString( "%1 <%2>" ).arg( name ).arg( mail ) );
    }

    LdapResult sr;
    sr.clientNumber = mClients.findIndex( (*it1).client );
    sr.name = name;
    sr.email = mail;
    resList.append( sr );
  }

  mResults.clear();
}

bool LdapSearch::isAvailable() const
{
  return !mNoLDAPLookup;
}



#include "ldapclient.moc"
