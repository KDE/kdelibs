/*
    This file is part of libkabc.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2004 Szombathelyi György <gyurco@freemail.hu>

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


#include <qapplication.h>
#include <qbuffer.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klineedit.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstringhandler.h>

#include <stdlib.h>
#include <netaccess.h>
#include <kabc/ldif.h>
#include <kabc/ldapurl.h>

#include "resourceldapkio.h"
#include "resourceldapkioconfig.h"

using namespace KABC;

// Hack from Netaccess - maybe a KProgressDialog would be better
void qt_enter_modal( QWidget *widget );
void qt_leave_modal( QWidget *widget );

class ResourceLDAPKIO::ResourceLDAPKIOPrivate 
{
  public:
    LDIF mLdif;
    bool mTLS,mSSL,mSubTree;
    QString mResultDn;
    Addressee mAddr;
    AddressBook::Iterator mSaveIt;
    bool mSASL;
    QString mMech;
    LDAPUrl mLDAPUrl;
};

ResourceLDAPKIO::ResourceLDAPKIO( const KConfig *config )
  : Resource( config )
{
  d = new ResourceLDAPKIOPrivate;
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
    d->mTLS = config->readBoolEntry( "LdapTLS" );
    d->mSSL = config->readBoolEntry( "LdapSSL" );
    d->mSubTree = config->readBoolEntry( "LdapSubTree" );
    d->mSASL = config->readBoolEntry( "LdapSASL" );
    d->mMech = config->readEntry( "LdapMech" );
  } else {
    mPort = 389;
    mAnonymous = true;
    mUser = mPassword = mHost =  mFilter =  mDn = d->mMech = "";
    d->mTLS = d->mSSL = d->mSubTree = d->mSASL = false;
  }
  init(); 
}

ResourceLDAPKIO::~ResourceLDAPKIO() 
{
  delete d;
}

void ResourceLDAPKIO::enter_loop()
{
  QWidget dummy(0,0,WType_Dialog | WShowModal);
  dummy.setFocusPolicy( QWidget::NoFocus );
  qt_enter_modal(&dummy);
  qApp->enter_loop();
  qt_leave_modal(&dummy);
}

void ResourceLDAPKIO::entries( KIO::Job*, const KIO::UDSEntryList & list )
{
  KIO::UDSEntryListConstIterator it = list.begin();
  KIO::UDSEntryListConstIterator end = list.end();
  for (; it != end; ++it) {
    KIO::UDSEntry::ConstIterator it2 = (*it).begin();
    for( ; it2 != (*it).end(); it2++ ) {
      if ( (*it2).m_uds == KIO::UDS_URL ) {
        KURL tmpurl( (*it2).m_str );
        d->mResultDn = tmpurl.path();
        if ( d->mResultDn.startsWith("/") ) d->mResultDn.remove(0,1);
        return;
      }
    }
  }
}

void ResourceLDAPKIO::listResult( KIO::Job *)
{
  qApp->exit_loop();
//FIXME: what to do if an error occured?
/*        
  if ( job->error() )
  else
*/
}

QString ResourceLDAPKIO::findUid( const QString &uid ) 
{
  LDAPUrl url;
  KIO::UDSEntry entry;
  
  d->mResultDn = "";
  if ( !mAnonymous ) {
    url.setUser( mUser );
    url.setPass( mPassword );
  }
  url.setProtocol( d->mSSL ? "ldaps" : "ldap");
  url.setHost( mHost );
  url.setPort( mPort );
  url.setDn( mDn );

  url.setAttributes("dn");

  url.setScope( d->mSubTree ? LDAPUrl::Sub : LDAPUrl::One );
  url.setFilter( "(" + mAttributes[ "uid" ] + "=" + uid + ")" + mFilter );
  url.setExtension( "x-dir", "one" );
  if ( d->mTLS ) url.setExtension( "x-tls", "" );
  if ( d->mSASL ) {
    url.setExtension( "x-sasl", "" );
    if ( !d->mMech.isEmpty() ) url.setExtension( "x-mech", d->mMech );
  }

  KIO::ListJob * listJob = KIO::listDir( url, false /* no GUI */ );
  connect( listJob, 
    SIGNAL( entries( KIO::Job *, const KIO::UDSEntryList& ) ),
    SLOT( entries( KIO::Job*, const KIO::UDSEntryList& ) ) );
  connect( listJob, SIGNAL( result( KIO::Job* ) ), 
    this, SLOT( listResult( KIO::Job* ) ) );

  enter_loop();
  return d->mResultDn;
}

bool ResourceLDAPKIO::AddresseeToLDIF( QByteArray &ldif, const Addressee &addr, 
  const QString &olddn )
{
  QCString tmp;
  QString dn;
  QByteArray data;
  
  dn = "cn=" + addr.assembledName() + "," +mDn;
  if ( !olddn.isEmpty() && olddn != dn ) {
    tmp = LDIF::assembleLine("dn", olddn.utf8()) + "\n";
    tmp += "changetype: modrdn\n";
    tmp += LDIF::assembleLine("newrdn", dn.section( ',', 0, 0 ).utf8()) + "\n";
    tmp += "deleteoldrdn: 1\n";
    dn = dn.section( ',', 0, 0 ) + "," + olddn.section( ',' , 1 );
  }
  
  bool mod = !olddn.isEmpty();
  tmp += "\n" + LDIF::assembleLine("dn", dn.utf8()) + "\n";
  if ( mod ) tmp += "changetype: modify\n";
  if ( !mod ) {
    tmp += "objectClass: top\n";
    QStringList obclass = QStringList::split( ',', mAttributes[ "objectClass" ] );
    for ( QStringList::iterator it = obclass.begin(); it != obclass.end(); it++ ) {
      tmp += LDIF::assembleLine("objectClass", (*it).utf8()) + "\n";
    }
  }
  if ( mod ) tmp += 
    LDIF::assembleLine("replace", mAttributes[ "commonName" ].utf8()) + "\n";
  tmp += LDIF::assembleLine( mAttributes[ "commonName" ], 
    addr.assembledName().utf8() ) + "\n";
  if ( mod ) tmp += "-\n"; 
  
  if ( mod ) tmp += 
    LDIF::assembleLine("replace", mAttributes[ "formattedName" ].utf8()) + "\n";
  tmp += LDIF::assembleLine( mAttributes[ "formattedName" ], 
    addr.formattedName().utf8()) + "\n";
  if ( mod ) tmp += "-\n"; 
  
  if ( mod ) tmp +=
    LDIF::assembleLine("replace", mAttributes[ "givenName" ].utf8()) + "\n";
  tmp += LDIF::assembleLine(mAttributes[ "givenName" ], 
    addr.givenName().utf8()) + "\n";
  if ( mod ) tmp += "-\n"; 
    
  if ( mod ) tmp +=
    LDIF::assembleLine("replace", mAttributes[ "familyName" ].utf8()) + "\n";
  tmp += LDIF::assembleLine(mAttributes[ "familyName" ], 
    addr.familyName().utf8()) + "\n";
  if ( mod ) tmp += "-\n"; 
    
  if ( mod ) tmp +=
    LDIF::assembleLine("replace", mAttributes[ "uid" ].utf8()) + "\n";
  tmp += LDIF::assembleLine(mAttributes[ "uid" ], addr.uid().utf8()) + "\n";
  if ( mod ) tmp += "-\n"; 
  
  QStringList emails = addr.emails();
  QStringList::ConstIterator mailIt;
  
  if ( mod ) tmp += 
    LDIF::assembleLine("replace", mAttributes[ "mail" ].utf8()) + "\n";
  mailIt = emails.begin();
  if ( mailIt != emails.end() ) {
    tmp += LDIF::assembleLine(mAttributes[ "mail" ], (*mailIt).utf8()) + "\n";
    mailIt ++;
    if ( mod ) tmp += "-\n"; 
  }
    
  if ( !mAttributes[ "mailAlias" ].isEmpty() ) {
    if ( mod ) tmp += 
      LDIF::assembleLine("replace", mAttributes[ "mailAlias" ].utf8()) + "\n";
    for ( ; mailIt != emails.end(); ++mailIt ) {
      tmp += LDIF::assembleLine(mAttributes[ "mailAlias" ].utf8(), 
        (*mailIt).utf8()) + "\n" ;
      if ( mod ) tmp += "-\n"; 
    }
  }
  
  PhoneNumber number = addr.phoneNumber( PhoneNumber::Home );
  if ( mod ) tmp += 
    LDIF::assembleLine("replace", mAttributes[ "phoneNumber" ].utf8()) + "\n";
  tmp += LDIF::assembleLine(mAttributes[ "phoneNumber" ], 
    number.number().utf8()) + "\n";
  if ( mod ) tmp += "-\n";

  if ( !mAttributes[ "jpegPhoto" ].isEmpty() ) {
    QByteArray pic;
    QBuffer buffer( pic );
    buffer.open( IO_WriteOnly );
    addr.photo().data().save( &buffer, "JPEG" );
    
    if ( mod ) tmp += 
      LDIF::assembleLine("replace", mAttributes[ "jpegPhoto" ].utf8()) + "\n";
    tmp += LDIF::assembleLine( mAttributes[ "jpegPhoto" ], pic, 76 ) + "\n";
    if ( mod ) tmp += "-\n";
  }
    
  kdDebug(7125) << "ldif: " << QString::fromUtf8(tmp) << endl;
  ldif = tmp;
  return true;
}

void ResourceLDAPKIO::init()
{

  if ( mPort == 0 ) mPort = 389;
//  if ( mUser.isEmpty() && mPassword.isEmpty() ) mAnonymous = true;

  /**
    If you want to add new attributes, append them here, add a
    translation string in the ctor of AttributesDialog and
    handle them in the load() method below.
    These are the default values
   */
  if ( !mAttributes.contains("objectClass") )
    mAttributes.insert( "objectClass", "inetOrgPerson" );
  if ( !mAttributes.contains("commonName") )
    mAttributes.insert( "commonName", "cn" );
  if ( !mAttributes.contains("formattedName") )
    mAttributes.insert( "formattedName", "displayName" );
  if ( !mAttributes.contains("familyName") )
    mAttributes.insert( "familyName", "sn" );
  if ( !mAttributes.contains("givenName") )
    mAttributes.insert( "givenName", "givenName" );
  if ( !mAttributes.contains("mail") )
    mAttributes.insert( "mail", "mail" );
  if ( !mAttributes.contains("mailAlias") )
    mAttributes.insert( "mailAlias", "" );
  if ( !mAttributes.contains("phoneNumber") )
    mAttributes.insert( "phoneNumber", "telephoneNumber" );
  if ( !mAttributes.contains("uid") )
    mAttributes.insert( "uid", "uid" );
  if ( !mAttributes.contains("jpegPhoto") )
    mAttributes.insert( "jpegPhoto", "jpegPhoto" );

  if ( !mAnonymous ) {
    d->mLDAPUrl.setUser( mUser );
    d->mLDAPUrl.setPass( mPassword );
  }
  d->mLDAPUrl.setProtocol( d->mSSL ? "ldaps" : "ldap");
  d->mLDAPUrl.setHost( mHost );
  d->mLDAPUrl.setPort( mPort );
  d->mLDAPUrl.setDn( mDn );

  if (!mAttributes.empty()) {
    QMap<QString,QString>::Iterator it;
    QStringList attr;
    for ( it = mAttributes.begin(); it != mAttributes.end(); ++it ) {
      if ( !it.data().isEmpty() && it.key() != "objectClass" ) 
        attr.append( it.data() );
    }
    d->mLDAPUrl.setAttributes( attr );
  }

  d->mLDAPUrl.setScope( d->mSubTree ? LDAPUrl::Sub : LDAPUrl::One );
  if ( !mFilter.isEmpty() && mFilter != "(objectClass=*)" ) 
    d->mLDAPUrl.setFilter( mFilter );
  d->mLDAPUrl.setExtension( "x-dir", "base" );
  if ( d->mTLS ) d->mLDAPUrl.setExtension( "x-tls","" );
  if ( d->mSASL ) {
    d->mLDAPUrl.setExtension( "x-sasl", "" );
    if ( !d->mMech.isEmpty() ) d->mLDAPUrl.setExtension( "x-mech", "d->mMech" );
  }

  kdDebug(7125) << "resource_ldapkio url: " << d->mLDAPUrl.prettyURL() << endl;

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
  config->writeEntry( "LdapTLS", d->mTLS );
  config->writeEntry( "LdapSSL", d->mSSL );
  config->writeEntry( "LdapSubTree", d->mSubTree );
  config->writeEntry( "LdapSASL", d->mSASL );
  config->writeEntry( "LdapMech", d->mMech );

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
  //clear the addressee
  Addressee tmp;
  d->mAddr = tmp;
  //initialize ldif parser
  d->mLdif.startParsing();

  KIO::Job *job = KIO::get( d->mLDAPUrl, true, false );
  connect( job, SIGNAL( data( KIO::Job*, const QByteArray& ) ),
    this, SLOT( data( KIO::Job*, const QByteArray& ) ) );
  connect( job, SIGNAL( result( KIO::Job* ) ),
    this, SLOT( result( KIO::Job* ) ) );
  return true;
}

void ResourceLDAPKIO::data( KIO::Job *, const QByteArray &data )
{
  if ( data.size() ) {
    d->mLdif.setLDIF( data );
  } else {
    QByteArray dummy( 3 );
    dummy[ 0 ] = '\n';
    dummy[ 1 ] = '\n';
    dummy[ 2 ] = '\n';
    d->mLdif.setLDIF( dummy );
  }
  
  LDIF::ParseVal ret;
  QString name;
  QByteArray value;
  do {
    ret = d->mLdif.nextItem();
    switch ( ret ) {
      case LDIF::Item:
        name = d->mLdif.attr().lower();  
        value = d->mLdif.val();      
//        value = QString::fromUtf8( d->mLdif.val(), d->mLdif.val().size()-1 );
        if ( name == mAttributes[ "commonName" ].lower() ) {
          if ( !d->mAddr.formattedName().isEmpty() ) {
            QString fn = d->mAddr.formattedName();
            d->mAddr.setNameFromString( QString::fromUtf8( value, value.size() ) );
            d->mAddr.setFormattedName( fn );
          } else
            d->mAddr.setNameFromString( QString::fromUtf8( value, value.size() ) );
        } else if ( name == mAttributes[ "formattedName" ].lower() ) {
          d->mAddr.setFormattedName( QString::fromUtf8( value, value.size() ) );
        } else if ( name == mAttributes[ "givenName" ].lower() ) {
          d->mAddr.setGivenName( QString::fromUtf8( value, value.size() ) );
        } else if ( name == mAttributes[ "mail" ].lower() ) {
          d->mAddr.insertEmail( QString::fromUtf8( value, value.size() ), true );
        } else if ( name == mAttributes[ "mailAlias" ].lower() ) {
          d->mAddr.insertEmail( QString::fromUtf8( value, value.size() ), false );
        } else if ( name == mAttributes[ "phoneNumber" ].lower() ) {
          PhoneNumber phone;
          phone.setNumber( QString::fromUtf8( value, value.size() ) );
          d->mAddr.insertPhoneNumber( phone );
          break; // read only the home number
        } else if ( name == mAttributes[ "familyName" ].lower() ) {
          d->mAddr.setFamilyName( QString::fromUtf8( value, value.size() ) );
        } else if ( name == mAttributes[ "uid" ].lower() ) {
          d->mAddr.setUid( QString::fromUtf8( value, value.size() ) );
        } else if ( name == mAttributes[ "jpegPhoto" ].lower() ) {
          KABC::Picture photo;
          QImage img( value );
          if ( !img.isNull() ) {
            photo.setData( img );
            photo.setType( "image/jpeg" );
            d->mAddr.setPhoto( photo );
          }
        }

        break;
      case LDIF::EndEntry: {
        d->mAddr.setResource( this );
        insertAddressee( d->mAddr );
        //clear the addressee
        Addressee tmp;
        d->mAddr = tmp;
        }
        break;
      default:
        break;
    }
  } while ( ret != LDIF::MoreData );
}

void ResourceLDAPKIO::result( KIO::Job *job )
{
  if ( job->error() )
    emit loadingError( this, job->errorString() );
  else
    emit loadingFinished( this );
}

bool ResourceLDAPKIO::save( Ticket* )
{
  kdDebug(7125) << "ResourceLDAPKIO save" << endl;
  
  d->mSaveIt = addressBook()->begin();
  KIO::Job *job = KIO::put( d->mLDAPUrl, -1, true, false, false );
  connect( job, SIGNAL( dataReq( KIO::Job*, QByteArray& ) ),
    this, SLOT( saveData( KIO::Job*, QByteArray& ) ) );
  connect( job, SIGNAL( result( KIO::Job* ) ),
    this, SLOT( syncSaveResult( KIO::Job* ) ) );
  enter_loop();
  if ( mErrorMsg.isEmpty() ) 
    return true;
  else {
    addressBook()->error( mErrorMsg );
    return false;
  }
}

bool ResourceLDAPKIO::asyncSave( Ticket* )
{
  kdDebug(7125) << "ResourceLDAPKIO asyncSave" << endl;
  d->mSaveIt = addressBook()->begin();
  KIO::Job *job = KIO::put( d->mLDAPUrl, -1, true, false, false );
  connect( job, SIGNAL( dataReq( KIO::Job*, QByteArray& ) ),
    this, SLOT( saveData( KIO::Job*, QByteArray& ) ) );
  connect( job, SIGNAL( result( KIO::Job* ) ),
    this, SLOT( saveResult( KIO::Job* ) ) );
  return true;
}

void ResourceLDAPKIO::syncSaveResult( KIO::Job *job )
{
  if ( job->error() )
    mErrorMsg = job->errorString();
  else
    mErrorMsg = "";
  
  qApp->exit_loop();
}

void ResourceLDAPKIO::saveResult( KIO::Job *job )
{
  if ( job->error() )
    emit savingError( this, job->errorString() );
  else
    emit savingFinished( this );
}

void ResourceLDAPKIO::saveData( KIO::Job*, QByteArray& data )
{
  
  while ( d->mSaveIt != addressBook()->end() &&
         ((*d->mSaveIt).resource() != this || 
         !(*d->mSaveIt).changed()) ) d->mSaveIt++;

  if (d->mSaveIt == addressBook()->end()) {
    kdDebug(7125) << "ResourceLDAPKIO endData" << endl;
    data.resize(0);
    return;
  }
  
  kdDebug(7125) << "ResourceLDAPKIO saveData: " << (*d->mSaveIt).assembledName() << endl;
  
  AddresseeToLDIF( data, *d->mSaveIt, findUid( (*d->mSaveIt).uid() ) );  
  kdDebug(7125) << "ResourceLDAPKIO save LDIF: " << QString::fromUtf8(data) << endl;
  // mark as unchanged
  (*d->mSaveIt).setChanged( false );

  d->mSaveIt++;  
}

void ResourceLDAPKIO::removeAddressee( const Addressee& addr )
{
  QString dn = findUid( addr.uid() );
  
  kdDebug(7125) << "ldapkio: removeAddressee" << dn << endl;

  if ( !dn.isEmpty() ) {
    KURL url( d->mLDAPUrl );
    url.setPath( "/" + dn );
    if ( KIO::NetAccess::del( url, NULL ) ) mAddrMap.erase( addr.uid() );
  }
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

void ResourceLDAPKIO::setIsTLS( bool value )
{
  d->mTLS = value;
}

bool ResourceLDAPKIO::isTLS() const
{
  return d->mTLS;
}
void ResourceLDAPKIO::setIsSSL( bool value )
{
  d->mSSL = value;
}

bool ResourceLDAPKIO::isSSL() const
{
  return d->mSSL;
}

void ResourceLDAPKIO::setIsSubTree( bool value )
{
  d->mSubTree = value;
}

bool ResourceLDAPKIO::isSubTree() const
{
  return d->mSubTree;
}

void ResourceLDAPKIO::setAttributes( const QMap<QString, QString> &attributes )
{
  mAttributes = attributes;
}

QMap<QString, QString> ResourceLDAPKIO::attributes() const
{
  return mAttributes;
}

void ResourceLDAPKIO::setIsSASL( bool value )
{
  d->mSASL = value;
}

bool ResourceLDAPKIO::isSASL() const
{
  return d->mSASL;
}

void ResourceLDAPKIO::setMech( const QString &mech )
{
  d->mMech = mech;
}

QString ResourceLDAPKIO::mech() const
{
  return d->mMech;
}
    
#include "resourceldapkio.moc"
