/*
    This file is part of libkabc.
    Copyright (c) 2002 - 2003 Tobias Koenig <tokoe@kde.org>

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

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <kvbox.h>
#include <qradiobutton.h>

#include <kacceleratormanager.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>

#include "resourceldapkio.h"

#include "resourceldapkioconfig.h"
#include "resourceldapkioconfig.moc"

using namespace KABC;

ResourceLDAPKIOConfig::ResourceLDAPKIOConfig( QWidget* parent )
    : KRES::ConfigWidget( parent )
{
  QBoxLayout *mainLayout = new QVBoxLayout( this );
  mainLayout->setAutoAdd( true );
  cfg = new LdapConfigWidget( LdapConfigWidget::W_ALL, this );

  mSubTree = new QCheckBox( i18n( "Sub-tree query" ), this );
  KHBox *box = new KHBox( this );
  box->setSpacing( KDialog::spacingHint() );
  mEditButton = new QPushButton( i18n( "Edit Attributes..." ), box );
  mCacheButton = new QPushButton( i18n( "Offline Use..." ), box );

  connect( mEditButton, SIGNAL( clicked() ), SLOT( editAttributes() ) );
  connect( mCacheButton, SIGNAL( clicked() ), SLOT( editCache() ) );
}

void ResourceLDAPKIOConfig::loadSettings( KRES::Resource *res )
{
  ResourceLDAPKIO *resource = dynamic_cast<ResourceLDAPKIO*>( res );

  if ( !resource ) {
    kdDebug(5700) << "ResourceLDAPKIOConfig::loadSettings(): cast failed" << endl;
    return;
  }

  cfg->setUser( resource->user() );
  cfg->setPassword( resource->password() );
  cfg->setRealm( resource->realm() );
  cfg->setBindDN( resource->bindDN() );
  cfg->setHost( resource->host() );
  cfg->setPort(  resource->port() );
  cfg->setVer(  resource->ver() );
  cfg->setTimeLimit( resource->timeLimit() );
  cfg->setSizeLimit( resource->sizeLimit() );
  cfg->setDn( resource->dn() );
  cfg->setFilter( resource->filter() );
  cfg->setMech( resource->mech() );
  if ( resource->isTLS() ) cfg->setSecTLS();
  else if ( resource->isSSL() ) cfg->setSecSSL();
  else cfg->setSecNO();
  if ( resource->isAnonymous() ) cfg->setAuthAnon();
  else if ( resource->isSASL() ) cfg->setAuthSASL();
  else cfg->setAuthSimple();

  mSubTree->setChecked( resource->isSubTree() );
  mAttributes = resource->attributes();
  mRDNPrefix = resource->RDNPrefix();
  mCachePolicy = resource->cachePolicy();
  mCacheDst = resource->cacheDst();
  mAutoCache = resource->autoCache();
}

void ResourceLDAPKIOConfig::saveSettings( KRES::Resource *res )
{
  ResourceLDAPKIO *resource = dynamic_cast<ResourceLDAPKIO*>( res );

  if ( !resource ) {
    kdDebug(5700) << "ResourceLDAPKIOConfig::saveSettings(): cast failed" << endl;
    return;
  }

  resource->setUser( cfg->user() );
  resource->setPassword( cfg->password() );
  resource->setRealm( cfg->realm() );
  resource->setBindDN( cfg->bindDN() );
  resource->setHost( cfg->host() );
  resource->setPort( cfg->port() );
  resource->setVer( cfg->ver() );
  resource->setTimeLimit( cfg->timeLimit() );
  resource->setSizeLimit( cfg->sizeLimit() );
  resource->setDn( cfg->dn() );
  resource->setFilter( cfg->filter() );
  resource->setIsAnonymous( cfg->isAuthAnon() );
  resource->setIsSASL( cfg->isAuthSASL() );
  resource->setMech( cfg->mech() );
  resource->setIsTLS( cfg->isSecTLS() );
  resource->setIsSSL( cfg->isSecSSL() );
  resource->setIsSubTree( mSubTree->isChecked() );
  resource->setAttributes( mAttributes );
  resource->setRDNPrefix( mRDNPrefix );
  resource->setCachePolicy( mCachePolicy );
  resource->init();

}

void ResourceLDAPKIOConfig::editAttributes()
{
  AttributesDialog dlg( mAttributes, mRDNPrefix, this );
  if ( dlg.exec() ) {
    mAttributes = dlg.attributes();
    mRDNPrefix = dlg.rdnprefix();
  }
}

void ResourceLDAPKIOConfig::editCache()
{
  LDAPUrl src;
  QStringList attr;

  src = cfg->url();
  src.setScope( mSubTree->isChecked() ? LDAPUrl::Sub : LDAPUrl::One );
  if (!mAttributes.empty()) {
    QMap<QString,QString>::Iterator it;
    QStringList attr;
    for ( it = mAttributes.begin(); it != mAttributes.end(); ++it ) {
      if ( !it.data().isEmpty() && it.key() != "objectClass" )
        attr.append( it.data() );
    }
    src.setAttributes( attr );
  }
  src.setExtension( "x-dir", "base" );
  OfflineDialog dlg( mAutoCache, mCachePolicy, src, mCacheDst, this );
  if ( dlg.exec() ) {
    mCachePolicy = dlg.cachePolicy();
    mAutoCache = dlg.autoCache();
  }

}

AttributesDialog::AttributesDialog( const QMap<QString, QString> &attributes,
                                    int rdnprefix,
                                    QWidget *parent, const char *name )
  : KDialogBase( Plain, i18n( "Attributes Configuration" ), Ok | Cancel,
                 Ok, parent, name, true, true )
{
  mNameDict.setAutoDelete( true );
  mNameDict.insert( "objectClass", new QString( i18n( "Object classes" ) ) );
  mNameDict.insert( "commonName", new QString( i18n( "Common name" ) ) );
  mNameDict.insert( "formattedName", new QString( i18n( "Formatted name" ) ) );
  mNameDict.insert( "familyName", new QString( i18n( "Family name" ) ) );
  mNameDict.insert( "givenName", new QString( i18n( "Given name" ) ) );
  mNameDict.insert( "organization", new QString( i18n( "Organization" ) ) );
  mNameDict.insert( "title", new QString( i18n( "Title" ) ) );
  mNameDict.insert( "street", new QString( i18n( "Street" ) ) );
  mNameDict.insert( "state", new QString( i18n( "State" ) ) );
  mNameDict.insert( "city", new QString( i18n( "City" ) ) );
  mNameDict.insert( "postalcode", new QString( i18n( "Postal code" ) ) );
  mNameDict.insert( "mail", new QString( i18n( "Email" ) ) );
  mNameDict.insert( "mailAlias", new QString( i18n( "Email alias" ) ) );
  mNameDict.insert( "phoneNumber", new QString( i18n( "Telephone number" ) ) );
  mNameDict.insert( "telephoneNumber", new QString( i18n( "Work telephone number" ) ) );
  mNameDict.insert( "facsimileTelephoneNumber", new QString( i18n( "Fax number" ) ) );
  mNameDict.insert( "mobile", new QString( i18n( "Cell phone number" ) ) );
  mNameDict.insert( "pager", new QString( i18n( "Pager" ) ) );
  mNameDict.insert( "description", new QString( i18n( "Note" ) ) );
  mNameDict.insert( "uid", new QString( i18n( "UID" ) ) );
  mNameDict.insert( "jpegPhoto", new QString( i18n( "Photo" ) ) );

  // default map
  mDefaultMap.insert( "objectClass", "inetOrgPerson" );
  mDefaultMap.insert( "commonName", "cn" );
  mDefaultMap.insert( "formattedName", "displayName" );
  mDefaultMap.insert( "familyName", "sn" );
  mDefaultMap.insert( "givenName", "givenName" );
  mDefaultMap.insert( "title", "title" );
  mDefaultMap.insert( "street", "street" );
  mDefaultMap.insert( "state", "st" );
  mDefaultMap.insert( "city", "l" );
  mDefaultMap.insert( "organization", "o" );
  mDefaultMap.insert( "postalcode", "postalCode" );
  mDefaultMap.insert( "mail", "mail" );
  mDefaultMap.insert( "mailAlias", "" );
  mDefaultMap.insert( "phoneNumber", "homePhone" );
  mDefaultMap.insert( "telephoneNumber", "telephoneNumber" );
  mDefaultMap.insert( "facsimileTelephoneNumber", "facsimileTelephoneNumber" );
  mDefaultMap.insert( "mobile", "mobile" );
  mDefaultMap.insert( "pager", "pager" );
  mDefaultMap.insert( "description", "description" );
  mDefaultMap.insert( "uid", "uid" );
  mDefaultMap.insert( "jpegPhoto", "jpegPhoto" );

  // overwrite the default values here
  QMap<QString, QString> kolabMap, netscapeMap, evolutionMap, outlookMap;

  // kolab
  kolabMap.insert( "formattedName", "display-name" );
  kolabMap.insert( "mailAlias", "mailalias" );

  // evolution
  evolutionMap.insert( "formattedName", "fileAs" );

  mMapList.append( attributes );
  mMapList.append( kolabMap );
  mMapList.append( netscapeMap );
  mMapList.append( evolutionMap );
  mMapList.append( outlookMap );

  QFrame *page = plainPage();
  QGridLayout *layout = new QGridLayout( page, 4, ( attributes.count() + 4 ) >> 1,
                                         0, spacingHint() );

  QLabel *label = new QLabel( i18n( "Template:" ), page );
  layout->addWidget( label, 0, 0 );
  mMapCombo = new KComboBox( page );
  layout->addWidget( mMapCombo, 0, 1 );

  mMapCombo->insertItem( i18n( "User Defined" ) );
  mMapCombo->insertItem( i18n( "Kolab" ) );
  mMapCombo->insertItem( i18n( "Netscape" ) );
  mMapCombo->insertItem( i18n( "Evolution" ) );
  mMapCombo->insertItem( i18n( "Outlook" ) );
  connect( mMapCombo, SIGNAL( activated( int ) ), SLOT( mapChanged( int ) ) );

  label = new QLabel( i18n( "RDN prefix attribute:" ), page );
  layout->addWidget( label, 1, 0 );
  mRDNCombo = new KComboBox( page );
  layout->addWidget( mRDNCombo, 1, 1 );
  mRDNCombo->insertItem( i18n( "commonName" ) );
  mRDNCombo->insertItem( i18n( "UID" ) );
  mRDNCombo->setCurrentItem( rdnprefix );

  QMap<QString, QString>::ConstIterator it;
  int i, j = 0;
  for ( i = 2, it = attributes.begin(); it != attributes.end(); ++it, ++i ) {
    if ( mNameDict[ it.key() ] == 0 ) {
      i--;
      continue;
    }
    if ( (uint)(i - 2) == ( mNameDict.count()  >> 1 ) ) {
      i = 0;
      j = 2;
    }
    kdDebug(7125) << "itkey: " << it.key() << " i: " << i << endl;
    label = new QLabel( *mNameDict[ it.key() ] + ":", page );
    KLineEdit *lineedit = new KLineEdit( page );
    mLineEditDict.insert( it.key(), lineedit );
    lineedit->setText( it.data() );
    label->setBuddy( lineedit );
    layout->addWidget( label, i, j );
    layout->addWidget( lineedit, i, j+1 );
  }

  for ( i = 1; i < mMapCombo->count(); i++ ) {
    Q3DictIterator<KLineEdit> it2( mLineEditDict );
    for ( ; it2.current(); ++it2 ) {
      if ( mMapList[ i ].contains( it2.currentKey() ) ) {
        if ( mMapList[ i ][ it2.currentKey() ] != it2.current()->text() ) break;
      } else {
        if ( mDefaultMap[ it2.currentKey() ] != it2.current()->text() ) break;
      }
    }
    if ( !it2.current() ) {
      mMapCombo->setCurrentItem( i );
      break;
    }
  }

  KAcceleratorManager::manage( this );
}

AttributesDialog::~AttributesDialog()
{
}

QMap<QString, QString> AttributesDialog::attributes() const
{
  QMap<QString, QString> map;

  Q3DictIterator<KLineEdit> it( mLineEditDict );
  for ( ; it.current(); ++it )
    map.insert( it.currentKey(), it.current()->text() );

  return map;
}

int AttributesDialog::rdnprefix() const
{
  return mRDNCombo->currentItem();
}

void AttributesDialog::mapChanged( int pos )
{

  // apply first the default and than the spezific changes
  QMap<QString, QString>::Iterator it;
  for ( it = mDefaultMap.begin(); it != mDefaultMap.end(); ++it )
    mLineEditDict[ it.key() ]->setText( it.data() );

  for ( it = mMapList[ pos ].begin(); it != mMapList[ pos ].end(); ++it ) {
    if ( !it.data().isEmpty() ) {
      KLineEdit *le = mLineEditDict[ it.key() ];
      if ( le ) le->setText( it.data() );
    }
  }
}

OfflineDialog::OfflineDialog( bool autoCache, int cachePolicy, const KURL &src,
  const QString &dst, QWidget *parent, const char *name )
  : KDialogBase( Plain, i18n( "Offline Configuration" ), Ok | Cancel,
                 Ok, parent, name, true, true )
{
  QFrame *page = plainPage();
  QVBoxLayout *layout = new QVBoxLayout( page );
  layout->setAutoAdd( true );

  mSrc = src; mDst = dst;
  mCacheGroup = new Q3ButtonGroup( 1, Qt::Horizontal,
    i18n("Offline Cache Policy"), page );

  QRadioButton *bt;
  new QRadioButton( i18n("Do not use offline cache"), mCacheGroup );
  bt = new QRadioButton( i18n("Use local copy if no connection"), mCacheGroup );
  new QRadioButton( i18n("Always use local copy"), mCacheGroup );
  mCacheGroup->setButton( cachePolicy );

  mAutoCache = new QCheckBox( i18n("Refresh offline cache automatically"),
    page );
  mAutoCache->setChecked( autoCache );
  mAutoCache->setEnabled( bt->isChecked() );

  connect( bt, SIGNAL(toggled(bool)), mAutoCache, SLOT(setEnabled(bool)) );

  QPushButton *lcache = new QPushButton( i18n("Load into Cache"), page );
  connect( lcache, SIGNAL( clicked() ), SLOT( loadCache() ) );
}

OfflineDialog::~OfflineDialog()
{
}

bool OfflineDialog::autoCache() const
{
  return mAutoCache->isChecked();
}

int OfflineDialog::cachePolicy() const
{
  return mCacheGroup->selectedId();
}

void OfflineDialog::loadCache()
{
  if ( KIO::NetAccess::download( mSrc, mDst, this ) ) {
    KMessageBox::information( this,
      i18n("Successfully downloaded directory server contents!") );
  } else {
    KMessageBox::error( this,
      i18n("An error occurred downloading directory server contents into file %1.").arg(mDst) );
  }
}
