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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include <qapplication.h>

#include <qobject.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <Q3HButtonGroup>
#include <Q3ButtonGroup>
#include <qradiobutton.h>

#include <kmessagebox.h>
#include <kacceleratormanager.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kprogress.h>

#include <kdebug.h>

#include "ldapconfigwidget.h"
#include "ldapconfigwidget.moc"

using namespace KABC;

LdapConfigWidget::LdapConfigWidget( QWidget* parent,
                                    Qt::WFlags fl ) : QWidget( parent, fl )
{
  mProg = 0;
  mFlags = 0;
  mainLayout = new QGridLayout( this, 12, 4, 0,
      KDialog::spacingHint() );
}

LdapConfigWidget::LdapConfigWidget( int flags, QWidget* parent,
                                    Qt::WFlags fl ) : QWidget( parent, fl )
{
  mFlags = flags;
  mProg = 0;
  mainLayout = new QGridLayout( this, 12, 4, 0,
      KDialog::spacingHint() );
  initWidget();
}

LdapConfigWidget::~LdapConfigWidget()
{
}

void LdapConfigWidget::initWidget()
{
  QLabel *label;

  mUser = mPassword = mHost = mDn = mBindDN = mRealm = mFilter = 0;
  mPort = mVer = mTimeLimit = mSizeLimit = 0;
  mAnonymous = mSimple = mSASL = mSecNO = mSecTLS = mSecSSL = 0;
  mEditButton =  mQueryMech = 0;
  mMech = 0;
  int row = 0;
  int col;

  if ( mFlags & W_USER ) {
    label = new QLabel( i18n( "User:" ), this );
    mUser = new KLineEdit( this );
    mUser->setObjectName( "kcfg_ldapuser" );

    mainLayout->addWidget( label, row, 0 );
    mainLayout->addMultiCellWidget( mUser, row, row, 1, 3 );
    row++;
  }

  if ( mFlags & W_BINDDN ) {
    label = new QLabel( i18n( "Bind DN:" ), this );
    mBindDN = new KLineEdit( this);
    mBindDN->setObjectName( "kcfg_ldapbinddn" );

    mainLayout->addWidget( label, row, 0 );
    mainLayout->addMultiCellWidget( mBindDN, row, row, 1, 3 );
    row++;
  }

  if ( mFlags & W_REALM ) {
    label = new QLabel( i18n( "Realm:" ), this );
    mRealm = new KLineEdit( this);
    mRealm->setObjectName("kcfg_ldaprealm" );

    mainLayout->addWidget( label, row, 0 );
    mainLayout->addMultiCellWidget( mRealm, row, row, 1, 3 );
    row++;
  }

  if ( mFlags & W_PASS ) {
    label = new QLabel( i18n( "Password:" ), this );
    mPassword = new KLineEdit( this);
    mPassword->setObjectName( "kcfg_ldappassword" );
    mPassword->setEchoMode( KLineEdit::Password );

    mainLayout->addWidget( label, row, 0 );
    mainLayout->addMultiCellWidget( mPassword, row, row, 1, 3 );
    row++;
  }

  if ( mFlags & W_HOST ) {
    label = new QLabel( i18n( "Host:" ), this );
    mHost = new KLineEdit( this);
    mHost->setObjectName( "kcfg_ldaphost" );

    mainLayout->addWidget( label, row, 0 );
    mainLayout->addMultiCellWidget( mHost, row, row, 1, 3 );
    row++;
  }

  col = 0;
  if ( mFlags & W_PORT ) {
    label = new QLabel( i18n( "Port:" ), this );
    mPort = new QSpinBox( 0, 65535, 1, this);
    mPort->setObjectName("kcfg_ldapport" );
    mPort->setSizePolicy( QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred ) );
    mPort->setValue( 389 );

    mainLayout->addWidget( label, row, col );
    mainLayout->addWidget( mPort, row, col+1 );
    col += 2;
  }

  if ( mFlags & W_VER ) {
    label = new QLabel( i18n( "LDAP version:" ), this );
    mVer = new QSpinBox( 2, 3, 1, this);
    mVer->setObjectName( "kcfg_ldapver" );
    mVer->setSizePolicy( QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred ) );
    mVer->setValue( 3 );
    mainLayout->addWidget( label, row, col );
    mainLayout->addWidget( mVer, row, col+1 );
  }
  if ( mFlags & ( W_PORT | W_VER ) ) row++;

  col = 0;
  if ( mFlags & W_SIZELIMIT ) {
    label = new QLabel( i18n( "Size limit:" ), this );
    mSizeLimit = new QSpinBox( 0, 9999999, 1, this);
    mSizeLimit->setObjectName("kcfg_ldapsizelimit" );
    mSizeLimit->setSizePolicy( QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred ) );
    mSizeLimit->setValue( 0 );
    mSizeLimit->setSpecialValueText( i18n("Default") );
    mainLayout->addWidget( label, row, col );
    mainLayout->addWidget( mSizeLimit, row, col+1 );
    col += 2;
  }

  if ( mFlags & W_TIMELIMIT ) {
    label = new QLabel( i18n( "Time limit:" ), this );
    mTimeLimit = new QSpinBox( 0, 9999999, 1, this);
    mTimeLimit->setObjectName("kcfg_ldaptimelimit" );
    mTimeLimit->setSizePolicy( QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred ) );
    mTimeLimit->setValue( 0 );
    mTimeLimit->setSuffix( i18n(" sec") );
    mTimeLimit->setSpecialValueText( i18n("Default") );
    mainLayout->addWidget( label, row, col );
    mainLayout->addWidget( mTimeLimit, row, col+1 );
  }
  if ( mFlags & ( W_SIZELIMIT | W_TIMELIMIT ) ) row++;

  if ( mFlags & W_DN ) {
    label = new QLabel( i18n( "Distinguished Name", "DN:" ), this );
    mDn = new KLineEdit( this);
    mDn->setObjectName("kcfg_ldapdn" );

    mainLayout->addWidget( label, row, 0 );
    mainLayout->addMultiCellWidget( mDn, row, row, 1, 1 );
    //without host query doesn't make sense
    if ( mHost ) {
      QPushButton *dnquery = new QPushButton( i18n( "Query Server" ), this );
      connect( dnquery, SIGNAL( clicked() ), SLOT( mQueryDNClicked() ) );
      mainLayout->addMultiCellWidget( dnquery, row, row, 2, 3 );
    }
    row++;
  }

  if ( mFlags & W_FILTER ) {
    label = new QLabel( i18n( "Filter:" ), this );
    mFilter = new KLineEdit( this);
    mFilter->setObjectName("kcfg_ldapfilter" );

    mainLayout->addWidget( label, row, 0 );
    mainLayout->addMultiCellWidget( mFilter, row, row, 1, 3 );
    row++;
  }

  if ( mFlags & W_SECBOX ) {
    Q3HButtonGroup *btgroup = new Q3HButtonGroup( i18n( "Security" ), this );
    mSecNO = new QRadioButton( i18n( "No" ), btgroup);
    mSecNO->setObjectName( "kcfg_ldapnosec" );
    mSecTLS = new QRadioButton( i18n( "TLS" ), btgroup);
    mSecTLS->setObjectName( "kcfg_ldaptls" );
    mSecSSL = new QRadioButton( i18n( "SSL" ), btgroup);
    mSecSSL->setObjectName("kcfg_ldapssl" );
    mainLayout->addMultiCellWidget( btgroup, row, row, 0, 3 );

    connect( mSecNO, SIGNAL( clicked() ), SLOT( setLDAPPort() ) );
    connect( mSecTLS, SIGNAL( clicked() ), SLOT( setLDAPPort() ) );
    connect( mSecSSL, SIGNAL( clicked() ), SLOT( setLDAPSPort( ) ) );

    mSecNO->setChecked( true );
    row++;
  }

  if ( mFlags & W_AUTHBOX ) {

    Q3ButtonGroup *authbox =
      new Q3ButtonGroup( 3, Qt::Horizontal, i18n( "Authentication" ), this );

    mAnonymous = new QRadioButton( i18n( "Anonymous" ), authbox);
    mAnonymous->setObjectName("kcfg_ldapanon" );
    mSimple = new QRadioButton( i18n( "Simple" ), authbox);
    mSimple->setObjectName( "kcfg_ldapsimple" );
    mSASL = new QRadioButton( i18n( "SASL" ), authbox);
    mSASL->setObjectName("kcfg_ldapsasl" );

    label = new QLabel( i18n( "SASL mechanism:" ), authbox );
    mMech = new KComboBox( false, authbox);
    mMech->setObjectName("kcfg_ldapsaslmech");
    mMech->setEditable( true );
    mMech->insertItem( "DIGEST-MD5" );
    mMech->insertItem( "GSSAPI" );
    mMech->insertItem( "PLAIN" );

    //without host query doesn't make sense
    if ( mHost ) {
      mQueryMech = new QPushButton( i18n( "Query Server" ), authbox );
      connect( mQueryMech, SIGNAL( clicked() ), SLOT( mQueryMechClicked() ) );
    }

    mainLayout->addMultiCellWidget( authbox, row, row+1, 0, 3 );

    connect( mAnonymous, SIGNAL( stateChanged(int) ), SLOT( setAnonymous(int) ) );
    connect( mSimple, SIGNAL( stateChanged(int) ), SLOT( setSimple(int) ) );
    connect( mSASL, SIGNAL( stateChanged(int) ), SLOT( setSASL(int) ) );

    mAnonymous->setChecked( true );
  }

}

void LdapConfigWidget::loadData( KIO::Job*, const QByteArray& d )
{
  LDIF::ParseVal ret;

  if ( d.size() ) {
    mLdif.setLDIF( d );
  } else {
    mLdif.endLDIF();
  }
  do {
    ret = mLdif.nextItem();
    if ( ret == LDIF::Item && mLdif.attr().toLower() == mAttr ) {
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
  _url.setAttributes( QStringList( mAttr ) );
  _url.setScope( LDAPUrl::Base );
  if ( mVer ) _url.setExtension( "x-ver", QString::number( mVer->value() ) );
  if ( mSecTLS && mSecTLS->isChecked() ) _url.setExtension( "x-tls", "" );

  kdDebug(5700) << "sendQuery url: " << _url.prettyURL() << endl;
  mLdif.startParsing();
  KIO::Job *job = KIO::get( _url, true, false );
  job->addMetaData("no-auth-prompt","true");
  connect( job, SIGNAL( data( KIO::Job*, const QByteArray& ) ),
    this, SLOT( loadData( KIO::Job*, const QByteArray& ) ) );
  connect( job, SIGNAL( result( KIO::Job* ) ),
    this, SLOT( loadResult( KIO::Job* ) ) );

  if ( mProg == NULL )
    mProg = new KProgressDialog( this, 0, i18n("LDAP Query"), _url.prettyURL(), true );
  else
    mProg->setLabel( _url.prettyURL() );
  mProg->progressBar()->setValue( 0 );
  mProg->progressBar()->setTotalSteps( 1 );
  mProg->exec();
  if ( mCancelled ) {
    kdDebug(5700) << "query cancelled!" << endl;
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
  if ( state == QCheckBox::Off ) return;
  if ( mUser ) mUser->setEnabled(false);
  if ( mPassword ) mPassword->setEnabled(false);
  if ( mBindDN ) mBindDN->setEnabled(false);
  if ( mRealm ) mRealm->setEnabled(false);
  if ( mMech ) mMech->setEnabled(false);
  if ( mQueryMech ) mQueryMech->setEnabled(false);
}

void LdapConfigWidget::setSimple( int state )
{
  if ( state == QCheckBox::Off ) return;
  if ( mUser ) mUser->setEnabled(true);
  if ( mPassword ) mPassword->setEnabled(true);
  if ( mBindDN ) mBindDN->setEnabled(false);
  if ( mRealm ) mRealm->setEnabled(false);
  if ( mMech ) mMech->setEnabled(false);
  if ( mQueryMech ) mQueryMech->setEnabled(false);
}

void LdapConfigWidget::setSASL( int state )
{
  if ( state == QCheckBox::Off ) return;
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
  if ( mSizeLimit && mSizeLimit->value() != 0 )
    _url.setExtension( "x-sizelimit", QString::number( mSizeLimit->value() ) );
  if ( mTimeLimit && mTimeLimit->value() != 0 )
    _url.setExtension( "x-timelimit", QString::number( mTimeLimit->value() ) );
  if ( mSecTLS && mSecTLS->isChecked() ) _url.setExtension( "x-tls","" );
  if ( mFilter && !mFilter->text().isEmpty() )
    _url.setFilter( mFilter->text() );
  if ( mSASL && mSASL->isChecked() ) {
    _url.setExtension( "x-sasl", "" );
    _url.setExtension( "x-mech", mMech->currentText() );
    if ( mBindDN && !mBindDN->text().isEmpty() )
      _url.setExtension( "bindname", mBindDN->text() );
    if ( mRealm && !mRealm->text().isEmpty() )
      _url.setExtension( "x-realm", mRealm->text() );
  }
  return ( _url );
}

void LdapConfigWidget::setUser( const QString &user )
{
  if ( mUser ) mUser->setText( user );
}

QString LdapConfigWidget::user() const
{
  return ( mUser ? mUser->text() : QString::null );
}

void LdapConfigWidget::setPassword( const QString &password )
{
  if ( mPassword ) mPassword->setText( password );
}

QString LdapConfigWidget::password() const
{
  return ( mPassword ? mPassword->text() : QString::null );
}

void LdapConfigWidget::setBindDN( const QString &binddn )
{
  if ( mBindDN ) mBindDN->setText( binddn );
}

QString LdapConfigWidget::bindDN() const
{
  return ( mBindDN ? mBindDN->text() : QString::null );
}

void LdapConfigWidget::setRealm( const QString &realm )
{
  if ( mRealm ) mRealm->setText( realm );
}

QString LdapConfigWidget::realm() const
{
  return ( mRealm ? mRealm->text() : QString::null );
}

void LdapConfigWidget::setHost( const QString &host )
{
  if ( mHost ) mHost->setText( host );
}

QString LdapConfigWidget::host() const
{
  return ( mHost ? mHost->text() : QString::null );
}

void LdapConfigWidget::setPort( int port )
{
  if ( mPort ) mPort->setValue( port );
}

int LdapConfigWidget::port() const
{
  return ( mPort ? mPort->value() : 389 );
}

void LdapConfigWidget::setVer( int ver )
{
  if ( mVer ) mVer->setValue( ver );
}

int LdapConfigWidget::ver() const
{
  return ( mVer ? mVer->value() : 3 );
}

void LdapConfigWidget::setDn( const QString &dn )
{
  if ( mDn ) mDn->setText( dn );
}

QString LdapConfigWidget::dn() const
{
  return ( mDn ? mDn->text() : QString::null );
}

void LdapConfigWidget::setFilter( const QString &filter )
{
  if ( mFilter ) mFilter->setText( filter );
}

QString LdapConfigWidget::filter() const
{
  return ( mFilter ? mFilter->text() : QString::null );
}

void LdapConfigWidget::setMech( const QString &mech )
{
  if ( mMech == 0 ) return;
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

QString LdapConfigWidget::mech() const
{
  return ( mMech ? mMech->currentText() : QString::null );
}

void LdapConfigWidget::setSecNO( bool b )
{
  if ( mSecNO ) mSecNO->setChecked( b );
}

bool LdapConfigWidget::isSecNO() const
{
  return ( mSecNO ? mSecNO->isChecked() : true );
}

void LdapConfigWidget::setSecTLS( bool b )
{
  if ( mSecTLS ) mSecTLS->setChecked( b );
}

bool LdapConfigWidget::isSecTLS() const
{
  return ( mSecTLS ? mSecTLS->isChecked() : false );
}

void LdapConfigWidget::setSecSSL( bool b )
{
  if ( mSecSSL ) mSecSSL->setChecked( b );
}

bool LdapConfigWidget::isSecSSL() const
{
  return ( mSecSSL ? mSecSSL->isChecked() : false );
}

void LdapConfigWidget::setAuthAnon( bool b )
{
  if ( mAnonymous ) mAnonymous->setChecked( b );
}

bool LdapConfigWidget::isAuthAnon() const
{
  return ( mAnonymous ? mAnonymous->isChecked() : true );
}

void LdapConfigWidget::setAuthSimple( bool b )
{
  if ( mSimple ) mSimple->setChecked( b );
}

bool LdapConfigWidget::isAuthSimple() const
{
  return ( mSimple ? mSimple->isChecked() : false );
}

void LdapConfigWidget::setAuthSASL( bool b )
{
  if ( mSASL ) mSASL->setChecked( b );
}

bool LdapConfigWidget::isAuthSASL() const
{
  return ( mSASL ? mSASL->isChecked() : false );
}

void LdapConfigWidget::setSizeLimit( int sizelimit )
{
  if ( mSizeLimit ) mSizeLimit->setValue( sizelimit );
}

int LdapConfigWidget::sizeLimit() const
{
  return ( mSizeLimit ? mSizeLimit->value() : 0 );
}

void LdapConfigWidget::setTimeLimit( int timelimit )
{
  if ( mTimeLimit ) mTimeLimit->setValue( timelimit );
}

int LdapConfigWidget::timeLimit() const
{
  return ( mTimeLimit ? mTimeLimit->value() : 0 );
}

int LdapConfigWidget::flags() const
{
  return mFlags;
}

void LdapConfigWidget::setFlags( int flags )
{
  mFlags = flags;

  // First delete all the child widgets.
  // FIXME: I hope it's correct
  QList<QObject*> ch = children();

  foreach ( QObject*obj, ch ) {
	  QWidget *widget;
	  widget = dynamic_cast<QWidget*> (obj);
	  if ( widget && widget->parent() == this ) {
		  mainLayout->remove( widget );
		  delete ( widget );
	  }
  }
  // Re-create child widgets according to the new flags
  initWidget();
}
