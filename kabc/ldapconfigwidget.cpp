/*
    This file is part of libkabc.
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

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qvgroupbox.h>
#include <qhbuttongroup.h>
#include <qradiobutton.h>

#include <kmessagebox.h>
#include <kaccelmanager.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klocale.h>

#include "ldapconfigwidget.h"
#include "ldapconfigwidget.moc"

using namespace KABC;

LdapConfigWidget::LdapConfigWidget( int flags, QWidget* parent, 
  const char* name, WFlags fl ) : QWidget( parent, name, fl )
{
  QLabel *label;
  QGridLayout *mainLayout = new QGridLayout( this, 11, 4, 0,
      KDialog::spacingHint() );

  mUser = mPassword = mHost =  mDn = mBindDN = mRealm = mFilter = 0;
  mPort = mVer = 0;
  mAnonymous = mSimple = mSASL = mSecNO = mSecTLS = mSecSSL = 0;
  mEditButton =  mQueryMech = 0;
  mMech = 0;      
  mProg = 0;
      
  if ( flags & W_USER ) {  
    label = new QLabel( i18n( "User:" ), this );
    mUser = new KLineEdit( this, "kcfg_ldapuser" );

    mainLayout->addWidget( label, 0, 0 );
    mainLayout->addMultiCellWidget( mUser, 0, 0, 1, 3 );
  }
  
  if ( flags & W_BINDDN ) {  
    label = new QLabel( i18n( "Bind DN:" ), this );
    mBindDN = new KLineEdit( this, "kcfg_ldapbinddn" );

    mainLayout->addWidget( label, 1, 0 );
    mainLayout->addMultiCellWidget( mBindDN, 1, 1, 1, 3 );
  }
  
  if ( flags & W_REALM ) {  
    label = new QLabel( i18n( "Realm:" ), this );
    mRealm = new KLineEdit( this, "kcfg_ldaprealm" );

    mainLayout->addWidget( label, 2, 0 );
    mainLayout->addMultiCellWidget( mRealm, 2, 2, 1, 3 );
  }

  if ( flags & W_PASS ) {
    label = new QLabel( i18n( "Password:" ), this );
    mPassword = new KLineEdit( this, "kcfg_ldappassword" );
    mPassword->setEchoMode( KLineEdit::Password );

    mainLayout->addWidget( label, 3, 0 );
    mainLayout->addMultiCellWidget( mPassword, 3, 3, 1, 3 );
  }

  if ( flags & W_HOST ) {
    label = new QLabel( i18n( "Host:" ), this );
    mHost = new KLineEdit( this, "kcfg_ldaphost" );

    mainLayout->addWidget( label, 4, 0 );
    mainLayout->addWidget( mHost, 4, 1 );
    mainLayout->addMultiCellWidget( mHost, 4, 4, 1, 3 );
  }

  if ( flags & W_PORT ) {
    label = new QLabel( i18n( "Port:" ), this );
    mPort = new QSpinBox( 0, 65535, 1, this, "kcfg_ldapport" );
    mPort->setSizePolicy( QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred ) );
    mPort->setValue( 389 );

    mainLayout->addWidget( label, 5, 0 );
    mainLayout->addWidget( mPort, 5, 1 );
  }
  
  if ( flags & W_VER ) {
    label = new QLabel( i18n( "LDAP Version:" ), this );
    mVer = new QSpinBox( 2, 3, 1, this, "kcfg_ldapver" );
    mVer->setSizePolicy( QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred ) );
    mVer->setValue( 3 );
    mainLayout->addWidget( label, 5, 2 );
    mainLayout->addWidget( mVer, 5, 3 );
  }
  
  if ( flags & W_DN ) {
    label = new QLabel( i18n( "Distinguished Name", "DN:" ), this );
    mDn = new KLineEdit( this, "kcfg_ldapdn" );
    
    mainLayout->addWidget( label, 6, 0 );
    mainLayout->addMultiCellWidget( mDn, 6, 6, 1, 1 );
    //without host query doesn't make sense
    if ( mHost ) {
      QPushButton *dnquery = new QPushButton( i18n( "Query server" ), this );
      connect( dnquery, SIGNAL( clicked() ), SLOT( mQueryDNClicked() ) );
      mainLayout->addMultiCellWidget( dnquery, 6, 6, 2, 3 );
    }  
  }

  if ( flags & W_FILTER ) {
    label = new QLabel( i18n( "Filter:" ), this );
    mFilter = new KLineEdit( this, "kcfg_ldapfilter" );

    mainLayout->addWidget( label, 7, 0 );
    mainLayout->addMultiCellWidget( mFilter, 7, 7, 1, 3 );
  }
  
  if ( flags & W_SECBOX ) {
    QHButtonGroup *btgroup = new QHButtonGroup( i18n( "Security" ), this );
    mSecNO = new QRadioButton( i18n( "No" ), btgroup, "kcfg_ldapnosec" );
    mSecTLS = new QRadioButton( i18n( "TLS" ), btgroup, "kcfg_ldaptls" );
    mSecSSL = new QRadioButton( i18n( "SSL" ), btgroup, "kcfg_ldapssl" );
    mainLayout->addMultiCellWidget( btgroup, 8, 8, 0, 3 );
  
    connect( mSecNO, SIGNAL( clicked() ), SLOT( setLDAPPort() ) );
    connect( mSecTLS, SIGNAL( clicked() ), SLOT( setLDAPPort() ) );
    connect( mSecSSL, SIGNAL( clicked() ), SLOT( setLDAPSPort( ) ) );
    
    mSecNO->setChecked( true );
  }
  
  if ( flags & W_AUTHBOX ) {
  
    QButtonGroup *authbox = 
      new QButtonGroup( 3, Qt::Horizontal, i18n( "Authentication" ), this );

    mAnonymous = new QRadioButton( i18n( "Anonymous" ), authbox, "kcfg_ldapanon" );
    mSimple = new QRadioButton( i18n( "Simple" ), authbox, "kcfg_ldapsimple" );
    mSASL = new QRadioButton( i18n( "SASL" ), authbox, "kcfg_ldapsasl" );

    label = new QLabel( i18n( "SASL Mechanism:" ), authbox );
    mMech = new KComboBox( false, authbox, "kcfg_ldapsaslmech" );
    mMech->setEditable( true );
    mMech->insertItem( "DIGEST-MD5" );
    mMech->insertItem( "GSSAPI" );
    mMech->insertItem( "PLAIN" );
  
    //without host query doesn't make sense
    if ( mHost ) {
      mQueryMech = new QPushButton( i18n( "Query server" ), authbox );
      connect( mQueryMech, SIGNAL( clicked() ), SLOT( mQueryMechClicked() ) );
    }
      
    mainLayout->addMultiCellWidget( authbox, 9, 10, 0, 3 );
      
    connect( mAnonymous, SIGNAL( stateChanged(int) ), SLOT( setAnonymous(int) ) );
    connect( mSimple, SIGNAL( stateChanged(int) ), SLOT( setSimple(int) ) );
    connect( mSASL, SIGNAL( stateChanged(int) ), SLOT( setSASL(int) ) );
    
    mAnonymous->setChecked( true );
  }
}

LdapConfigWidget::~LdapConfigWidget() 
{
}

void LdapConfigWidget::loadData( KIO::Job*, const QByteArray& d )
{
  LDIF::ParseVal ret;
  
  if ( d.size() ) {
    mLdif.setLDIF( d );
  } else {
    QByteArray dummy( 3 );
    dummy[ 0 ] = '\n';
    dummy[ 1 ] = '\n';
    dummy[ 2 ] = '\n';
    mLdif.setLDIF( dummy );
  }
  do {
    ret = mLdif.nextItem();
    if ( ret == LDIF::Item && mLdif.attr().lower() == mAttr ) {
      mProg->progressBar()->advance( 1 );
      mQResult.push_back( QString::fromUtf8( mLdif.val(), mLdif.val().size() ) );
    }
  } while ( ret != LDIF::MoreData );
}

void LdapConfigWidget::loadResult( KIO::Job* job)
{
  int error = job->error();
  if ( error && error != KIO::ERR_USER_CANCELED )
    mErrorMsg = job->errorString();
  else
    mErrorMsg = "";

  mCancelled = false;
  mProg->close();
}

void LdapConfigWidget::sendQuery()
{
  LDAPUrl _url;
  
  mQResult.clear();
  mCancelled = true;

  _url.setProtocol( ( mSecSSL && mSecSSL->isChecked() ) ? "ldaps" : "ldap" );
  if ( mHost ) _url.setHost( mHost->text() );
  if ( mPort ) _url.setPort( mPort->value() );
  _url.setDn( "" );
  _url.setAttributes( mAttr );
  _url.setScope( LDAPUrl::Base );
  if ( mVer ) _url.setExtension( "x-ver", QString::number( mVer->value() ) );
  if ( mSecTLS && mSecTLS->isChecked() ) _url.setExtension( "x-tls", "" );
  
  kdDebug(7125) << "sendQuery url: " << _url.prettyURL() << endl;
  mLdif.startParsing();
  KIO::Job *job = KIO::get( _url, true, false );
  job->addMetaData("no-auth-prompt","true");
  connect( job, SIGNAL( data( KIO::Job*, const QByteArray& ) ),
    this, SLOT( loadData( KIO::Job*, const QByteArray& ) ) );
  connect( job, SIGNAL( result( KIO::Job* ) ),
    this, SLOT( loadResult( KIO::Job* ) ) );

  if ( mProg == NULL )
    mProg = new KProgressDialog( this, 0, i18n("LDAP query"), _url.prettyURL(), true );
  else
    mProg->setLabel( _url.prettyURL() );    
  mProg->progressBar()->setValue( 0 );
  mProg->progressBar()->setTotalSteps( 1 );
  mProg->exec();
  if ( mCancelled ) {
    kdDebug(7125) << "query cancelled!" << endl;
    job->kill( true );
  } else {
    if ( !mErrorMsg.isEmpty() ) KMessageBox::error( this, mErrorMsg );
  }
}

void LdapConfigWidget::mQueryMechClicked()
{
  mAttr = "supportedsaslmechanisms";
  sendQuery();
  if ( !mQResult.isEmpty() ) {
    mQResult.sort();
    mMech->clear();
    mMech->insertStringList( mQResult );
  }
}

void LdapConfigWidget::mQueryDNClicked()
{
  mAttr = "namingcontexts";
  sendQuery();
  if ( !mQResult.isEmpty() ) mDn->setText( mQResult.first() );
}

void LdapConfigWidget::setAnonymous( int state )
{
  if ( state == QButton::Off ) return;
  if ( mUser ) mUser->setEnabled(false);
  if ( mPassword ) mPassword->setEnabled(false);
  if ( mBindDN ) mBindDN->setEnabled(false);
  if ( mRealm ) mRealm->setEnabled(false);
  if ( mMech ) mMech->setEnabled(false);
  if ( mQueryMech ) mQueryMech->setEnabled(false);
}

void LdapConfigWidget::setSimple( int state )
{
  if ( state == QButton::Off ) return;
  if ( mUser ) mUser->setEnabled(true);
  if ( mPassword ) mPassword->setEnabled(true);
  if ( mBindDN ) mBindDN->setEnabled(false);
  if ( mRealm ) mRealm->setEnabled(false);
  if ( mMech ) mMech->setEnabled(false);
  if ( mQueryMech ) mQueryMech->setEnabled(false);
}

void LdapConfigWidget::setSASL( int state )
{
  if ( state == QButton::Off ) return;
  if ( mUser ) mUser->setEnabled(true);
  if ( mPassword ) mPassword->setEnabled(true);
  if ( mBindDN ) mBindDN->setEnabled(true);
  if ( mRealm ) mRealm->setEnabled(true);
  if ( mMech ) mMech->setEnabled(true);
  if ( mQueryMech ) mQueryMech->setEnabled(true);
}
    
void LdapConfigWidget::setLDAPPort() 
{
  mPort->setValue( 389 );
}

void LdapConfigWidget::setLDAPSPort()
{
  mPort->setValue( 636 );
}

void LdapConfigWidget::setMech( const QString &mech )
{
  if ( !mech.isEmpty() ) {
    int i = 0;
    while ( i < mMech->count() ) {
      if ( mMech->text( i ) == mech ) break;
      i++;
    }
    if ( i == mMech->count() ) mMech->insertItem( mech );
    mMech->setCurrentItem( i );
  }
}

LDAPUrl LdapConfigWidget::url() const
{
  LDAPUrl _url;
  if ( mSecSSL && mSecSSL->isChecked() )
    _url.setProtocol( "ldaps" );
  else 
    _url.setProtocol( "ldap" );
  
  if ( mUser ) _url.setUser( mUser->text() );
  if ( mPassword ) _url.setPass( mPassword->text() );
  if ( mHost ) _url.setHost( mHost->text() );
  if ( mPort ) _url.setPort( mPort->value() );
  if ( mDn ) _url.setDn( mDn->text() );
  if ( mVer ) _url.setExtension( "x-ver", QString::number( mVer->value() ) );
  if ( mSecTLS && mSecTLS->isChecked() ) _url.setExtension( "x-tls","" );
  if ( mFilter && !mFilter->text().isEmpty() )
    _url.setFilter( mFilter->text() );
  if ( mSASL && mSASL->isChecked() ) {
    _url.setExtension( "x-sasl", "" );
    _url.setExtension( "x-mech", mMech->currentText() );
    if ( mBindDN && !mBindDN->text().isEmpty() ) 
      _url.setExtension( "basename", mBindDN->text() );
    if ( mRealm && !mRealm->text().isEmpty() ) 
      _url.setExtension( "x-realm", mRealm->text() );
  }
  return ( _url );
}
