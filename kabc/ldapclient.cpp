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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <kmdcodec.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qlabel.h>
#include <qfile.h>

#include <kprotocolinfo.h>
#include <kconfig.h>
#include <kapplication.h>

#include <kdebug.h>

#include "ldapclient.h"

using namespace KABC;

QString LdapObject::toString() const
{
  QString result = QString::fromLatin1( "\ndn: %1\n" ).arg( dn );
  for ( LdapAttrMap::ConstIterator it = attrs.begin(); it != attrs.end(); ++it ) {
    QString attr = it.key();
    for ( LdapAttrValue::ConstIterator it2 = (*it).begin(); it2 != (*it).end(); ++it2 ) {
      if ( attr == "jpegPhoto" ) {
        QByteArray buf = *it2;
#if 0
        qDebug( "Trying to load image from buf with size %d", (*it2).size() );
        QPixmap pix;
        pix.loadFromData( buf, "JPEG" );
        qDebug( "Image loaded successfully" );
        QLabel* l = new QLabel( 0 );
        QFile f( "tmp.jpg" );
        f.open( IO_WriteOnly );
        f.writeBlock( buf );
        f.close();
        //l->setPixmap( QPixmap("tmp.jpg") );
        //l->show();
#endif
      } else {
        result += QString("%1: %2\n").arg( attr).arg( *it2 );
      }
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
  }    
}

LdapClient::LdapClient( QObject* parent, const char* name )
  : QObject( parent, name ), mJob( 0 ), mActive( false )
{
}

LdapClient::~LdapClient()
{
  cancelQuery();
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

void LdapClient::setAttrs( const QStringList& attrs )
{
  mAttrs = attrs;
}

void LdapClient::startQuery( const QString& filter )
{
  cancelQuery();
  QString query;
  if ( mScope.isEmpty() )
    mScope = "sub";

  QString host = mHost;
  if ( !mPort.isEmpty() ) {
    host += ':';
    host += mPort;
  }

  if ( mAttrs.empty() ) {
    query = QString("ldap://%1/%2?*?%3?(%4)").arg( host ).arg( mBase ).arg( mScope ).arg( filter );
  } else {
    query = QString("ldap://%1/%2?%3?%4?(%5)").arg( host ).arg( mBase )
      .arg( mAttrs.join(",") ).arg( mScope ).arg( filter );
  }
  kdDebug(5700) << "Doing query" << query.latin1() << endl;

  startParseLDIF();
  mActive = true;
  mJob = KIO::get( KURL( query ), false, false );
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
  QString str( data );
  kdDebug(5700) << "Got" << str.latin1() << endl;
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
  for ( QValueList<LdapObject>::Iterator it = mObjects.begin(); it != mObjects.end(); ++it ) {
    qDebug( (*it).toString().latin1() );
  }
#endif
  int err = mJob->error();
  if ( err ) {
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
}

void LdapClient::endParseLDIF()
{
  if ( !mCurrentObject.dn.isEmpty() ) {
    if ( !mLastAttrName.isNull() && !mLastAttrValue.isNull() ) {
      if ( mIsBase64 ) {
        QByteArray out;
        KCodecs::base64Decode( mLastAttrValue, out );
        //qDebug("_lastAttrValue=\"%s\", output length %d", _lastAttrValue.data(), out.size());
        mCurrentObject.attrs[ mLastAttrName ].append( out );
      } else {
        mCurrentObject.attrs[ mLastAttrName ].append( mLastAttrValue );
      }
    }
    emit result( mCurrentObject );
  }
}

void LdapClient::parseLDIF( const QByteArray& data )
{  
  //qDebug("%s", data.data());
  if ( data.isNull() || data.isEmpty() )
    return;
  mBuf += QCString( data ); // collect data in buffer
  int nl;
  while ( (nl = mBuf.find('\n')) != -1 ) {
    // Run through it line by line
    /* FIXME(steffen): This could be a problem
    * with "no newline at end of file" input
    */
    QCString line = mBuf.left( nl );
    if ( mBuf.length() > (unsigned int)(nl+1) )
      mBuf = mBuf.mid( nl+1 );
    else
      mBuf = "";

    if ( line.length() > 0 ) {
      if ( line[ 0 ] == '#' ) { // comment
        continue;
      } else if ( line[ 0 ] == ' ' || line[ 0 ] == '\t' ) { // continuation of last line
        line = line.stripWhiteSpace();
        //qDebug("Adding \"%s\"", line.data() );
        mLastAttrValue += line;
        continue;
      }
    } else
      continue;

    int colon = line.find(':');
    if ( colon != -1 ) { // Found new attribute	
      if ( mLastAttrName == "dn" ) { // New object, store the current
        if ( !mCurrentObject.dn.isNull() ) {
          emit result( mCurrentObject );
          mCurrentObject.clear();
        }
        mCurrentObject.dn = mLastAttrValue;
        mLastAttrValue = 0;
        mLastAttrName  = 0;
      } else if ( !mLastAttrName.isEmpty() ) {
        // Store current value, take care of decoding
        if ( mIsBase64 ) {
          QByteArray out;
          KCodecs::base64Decode( mLastAttrValue, out );
          //qDebug("_lastAttrValue=\"%s\", output length %d", _lastAttrValue.data(), out.size());
          mCurrentObject.attrs[ mLastAttrName ].append( out );
        } else {
          mCurrentObject.attrs[ mLastAttrName ].append( mLastAttrValue );
        }
      }

      mLastAttrName  = line.left( colon ).stripWhiteSpace();
      //qDebug("Found attr %s", _lastAttrName.data() );
      ++colon;
      if ( line[colon] == ':' ) {
        mIsBase64 = true;
        //qDebug("BASE64");
        ++colon;
      } else {
        //qDebug("UTF8");
        mIsBase64 = false;
      }

      mLastAttrValue = line.mid( colon ).stripWhiteSpace();
    }
  }
}

LdapSearch::LdapSearch()
    : mActiveClients( 0 ), mNoLDAPLookup( false )
{
  if ( !KProtocolInfo::isKnownProtocol( KURL("ldap://localhost") ) ) {
    mNoLDAPLookup = true;
    return;
  }

  // stolen from KAddressBook
  KConfig config( "kaddressbookrc", true );
  config.setGroup( "LDAP" );
  int numHosts = config.readUnsignedNumEntry( "NumSelectedHosts"); 
  if ( !numHosts ) {
    mNoLDAPLookup = true;
    return;
  } else {
    for ( int j = 0; j < numHosts; j++ ) {
      LdapClient* ldapClient = new LdapClient( this );
    
      QString host =  config.readEntry( QString( "SelectedHost%1" ).arg( j ), "" ).stripWhiteSpace();
      if ( host != "" )
        ldapClient->setHost( host );

      QString port = QString::number( config.readUnsignedNumEntry( QString( "SelectedPort%1" ).arg( j ) ) );
      if ( !port.isEmpty() )
        ldapClient->setPort( port );

      QString base = config.readEntry( QString( "SelectedBase%1" ).arg( j ), "" ).stripWhiteSpace();
      if ( base != "" )
        ldapClient->setBase( base );

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

  QValueList< LdapClient* >::Iterator it;
  for ( it = mClients.begin(); it != mClients.end(); ++it ) {
    (*it)->startQuery( filter );
    ++mActiveClients;
  }
}

void LdapSearch::cancelSearch()
{
  QValueList< LdapClient* >::Iterator it;
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
  emit searchData( makeSearchData() );
}

void LdapSearch::finish()
{
  mDataTimer.stop();

  emit searchData( makeSearchData() );
  emit searchDone();
}

QStringList LdapSearch::makeSearchData()
{
  QStringList ret;
  QString search_text_upper = mSearchText.upper();

  QValueList< KABC::LdapObject >::ConstIterator it1;
  for ( it1 = mResults.begin(); it1 != mResults.end(); ++it1 ) {
    QString name, mail, givenname, sn;

    LdapAttrMap::ConstIterator it2;
    for ( it2 = (*it1).attrs.begin(); it2 != (*it1).attrs.end(); ++it2 ) {
      if ( it2.key() == "cn" )
        name = QString( "%1" ).arg( (*it2).first() ); // TODO loop?
      else if( it2.key() == "mail" ) 
        mail = QString( "%1" ).arg( (*it2).first() );
	    else if( it2.key() == "givenName" )
        givenname = QString( "%1" ).arg( (*it2).first() );
	    else if( it2.key() == "sn" )
        sn = QString( "%1" ).arg( (*it2).first() );
    }

    if( mail.isEmpty())
        ; // nothing, bad entry
    else if ( name.isEmpty() )
      ret.append( mail );
    else {
        kdDebug() << "<" << name << "><" << mail << ">" << endl;
      ret.append( QString( "%1 <%2>" ).arg( name ).arg( mail ) );
      // this sucks
      if ( givenname.upper().startsWith( search_text_upper ) )
        ret.append( QString( "$$%1$%2 <%3>" ).arg( givenname ).arg( name ).arg( mail ) );
      if ( sn.upper().startsWith( search_text_upper ) )
        ret.append( QString( "$$%1$%2 <%3>" ).arg( sn ).arg( name ).arg( mail ) );
    }
  }

  mResults.clear();

  return ret;
}

bool LdapSearch::isAvailable() const
{
  return !mNoLDAPLookup;
}

#include "ldapclient.moc"
