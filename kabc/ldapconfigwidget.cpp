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
#include <qvbox.h>
#include <qvgroupbox.h>
#include <qhbuttongroup.h>
#include <qradiobutton.h>

#include <kaccelmanager.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klocale.h>

#include <kabc/ldapurl.h>

#include "ldapconfigwidget.h"
#include "ldapconfigwidget.moc"

using namespace KABC;

// Hack from Netaccess
void qt_enter_modal( QWidget *widget );
void qt_leave_modal( QWidget *widget );

LdapConfigWidget::LdapConfigWidget( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl ) 
{

  QGridLayout *mainLayout = new QGridLayout( this, 9, 3, 0,
      KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "User:" ), this );
  mUser = new KLineEdit( this, "kcfg_ldapuser" );

  mainLayout->addWidget( label, 0, 0 );
  mainLayout->addMultiCellWidget( mUser, 0, 0, 1, 2 );

  label = new QLabel( i18n( "Password:" ), this );
  mPassword = new KLineEdit( this, "kcfg_ldappassword" );
  mPassword->setEchoMode( KLineEdit::Password );

  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addMultiCellWidget( mPassword, 1, 1, 1, 2 );

  label = new QLabel( i18n( "Host:" ), this );
  mHost = new KLineEdit( this, "kcfg_ldaphost" );

  mainLayout->addWidget( label, 2, 0 );
  mainLayout->addWidget( mHost, 2, 1 );
  mainLayout->addMultiCellWidget( mHost, 2, 2, 1, 2 );

  label = new QLabel( i18n( "Port:" ), this );
  QVBox *box = new QVBox( this );
  mPort = new QSpinBox( 0, 65535, 1, box, "kcfg_ldapport" );
  mPort->setSizePolicy( QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred ) );
  mPort->setValue( 389 );
  new QWidget( box, "dummy" );

  mainLayout->addWidget( label, 3, 0 );
  mainLayout->addMultiCellWidget( box, 3, 3, 1, 2 );

  label = new QLabel( i18n( "Distinguished Name", "DN:" ), this );
  mDn = new KLineEdit( this, "kcfg_ldapdn" );
  QPushButton *dnquery = new QPushButton( i18n( "Query server" ), this );
  
  mainLayout->addWidget( label, 4, 0 );
  mainLayout->addMultiCellWidget( mDn, 4, 4, 1, 1 );
  mainLayout->addMultiCellWidget( dnquery, 4, 4, 2, 2 );

  label = new QLabel( i18n( "Filter:" ), this );
  mFilter = new KLineEdit( this, "kcfg_ldapfilter" );

  mainLayout->addWidget( label, 5, 0 );
  mainLayout->addMultiCellWidget( mFilter, 5, 5, 1, 2 );

  QHButtonGroup *btgroup = new QHButtonGroup( i18n( "Security" ), this );
  mSecNO = new QRadioButton( i18n( "No" ), btgroup, "kcfg_ldapnosec" );
  mSecTLS = new QRadioButton( i18n( "TLS" ), btgroup, "kcfg_ldaptls" );
  mSecSSL = new QRadioButton( i18n( "SSL" ), btgroup, "kcfg_ldapssl" );
  mainLayout->addMultiCellWidget( btgroup, 6, 6, 0, 2 );
  
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
  
  mQueryMech = new QPushButton( i18n( "Query server" ), authbox );
  
  mainLayout->addMultiCellWidget( authbox, 7, 8, 0, 2 );

  connect( mAnonymous, SIGNAL( clicked() ), SLOT( setAnonymous() ) );
  connect( mSimple, SIGNAL( clicked() ), SLOT( setSimple() ) );
  connect( mSASL, SIGNAL( clicked() ), SLOT( setSASL() ) );
  
  connect( mSecNO, SIGNAL( clicked() ), SLOT( setLDAPPort() ) );
  connect( mSecTLS, SIGNAL( clicked() ), SLOT( setLDAPPort() ) );
  connect( mSecSSL, SIGNAL( clicked() ), SLOT( setLDAPSPort( ) ) );
  
  connect( dnquery, SIGNAL( clicked() ), SLOT( mQueryDNClicked() ) );
  connect( mQueryMech, SIGNAL( clicked() ), SLOT( mQueryMechClicked() ) );

  mSecNO->setChecked( true );
  mAnonymous->setChecked( true );

}

LdapConfigWidget::~LdapConfigWidget() 
{
}

//hack from NetAccess
void LdapConfigWidget::enter_loop()
{
  QWidget dummy(0,0,WType_Dialog | WShowModal);
  dummy.setFocusPolicy( QWidget::NoFocus );
  qt_enter_modal(&dummy);
  qApp->enter_loop();
  qt_leave_modal(&dummy);
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
    if ( ret == LDIF::Item && mLdif.attr().lower() == mAttr ) 
      mQResult.push_back( QString::fromUtf8( mLdif.val(), mLdif.val().size() ) );
  } while ( ret != LDIF::MoreData );
}

void LdapConfigWidget::loadResult( KIO::Job* job)
{
  qApp->exit_loop();
}

void LdapConfigWidget::sendQuery()
{
  LDAPUrl url;
  
  mQResult.clear();

  url.setProtocol( mSecSSL->isChecked() ? "ldaps" : "ldap");
  url.setHost( mHost->text() );
  url.setPort( mPort->value() );
  url.setDn( "" );
  url.setAttributes(mAttr);
  url.setScope(LDAPUrl::Base);
  if ( mSecTLS->isChecked() ) url.setExtension("x-tls","");
  
  kdDebug(7125) << "sendQuery url: " << url.prettyURL() << endl;
  mLdif.startParsing();
  KIO::Job *job = KIO::get( url, true, false );
  job->addMetaData("no-auth-prompt","true");
  connect( job, SIGNAL( data( KIO::Job*, const QByteArray& ) ),
    this, SLOT( loadData( KIO::Job*, const QByteArray& ) ) );
  connect( job, SIGNAL( result( KIO::Job* ) ),
    this, SLOT( loadResult( KIO::Job* ) ) );
  enter_loop();
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

void LdapConfigWidget::setAnonymous()
{

  mUser->setEnabled(false);
  mPassword->setEnabled(false);
  mMech->setEnabled(false);
  mQueryMech->setEnabled(false);

}

void LdapConfigWidget::setSimple()
{
  mUser->setEnabled(true);
  mPassword->setEnabled(true);
  mMech->setEnabled(false);
  mQueryMech->setEnabled(false);
}

void LdapConfigWidget::setSASL()
{
  mUser->setEnabled(true);
  mPassword->setEnabled(true);
  mMech->setEnabled(true);
  mQueryMech->setEnabled(true);
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
