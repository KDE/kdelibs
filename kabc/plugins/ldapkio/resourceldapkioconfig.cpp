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
#include <kcombobox.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <klineedit.h>

#include "resourceldapkio.h"

#include "resourceldapkioconfig.h"
#include "resourceldapkioconfig.moc"

using namespace KABC;

ResourceLDAPKIOConfig::ResourceLDAPKIOConfig( QWidget* parent,  const char* name )
  : KRES::ConfigWidget( parent, name )
{
  QBoxLayout *mainLayout = new QVBoxLayout( this );
  mainLayout->setAutoAdd( true );
  cfg = new LdapConfigWidget( this );

  mSubTree = new QCheckBox( i18n( "Sub-tree query" ), this );
  mEditButton = new QPushButton( i18n( "Edit Attributes..." ), this );

  connect( mEditButton, SIGNAL( clicked() ), SLOT( editAttributes() ) );
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
  cfg->setHost( resource->host() );
  cfg->setPort(  resource->port() );
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
  resource->setHost( cfg->host() );
  resource->setPort( cfg->port() );
  resource->setDn( cfg->dn() );
  resource->setFilter( cfg->filter() );
  resource->setIsAnonymous( cfg->isAuthAnon() );
  resource->setIsSASL( cfg->isAuthSASL() );
  resource->setMech( cfg->mech() );
  resource->setIsTLS( cfg->isSecTLS() );
  resource->setIsSSL( cfg->isSecSSL() );
  resource->setIsSubTree( mSubTree->isChecked() );
  resource->setAttributes( mAttributes );
  resource->init();

}

void ResourceLDAPKIOConfig::editAttributes()
{
  AttributesDialog dlg( mAttributes, this );
  if ( dlg.exec() )
    mAttributes = dlg.attributes();
}

AttributesDialog::AttributesDialog( const QMap<QString, QString> &attributes,
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
  mNameDict.insert( "mail", new QString( i18n( "Email" ) ) );
  mNameDict.insert( "mailAlias", new QString( i18n( "Email alias" ) ) );
  mNameDict.insert( "phoneNumber", new QString( i18n( "Telephone number" ) ) );
  mNameDict.insert( "uid", new QString( i18n( "UID" ) ) );
  mNameDict.insert( "jpegPhoto", new QString( i18n( "Photo" ) ) );

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
  QGridLayout *layout = new QGridLayout( page, 2, attributes.count() + 1,
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

  QMap<QString, QString>::ConstIterator it;
  int i;
  for ( i = 1, it = attributes.begin(); it != attributes.end(); ++it, ++i ) {
    if ( mNameDict[ it.key() ] == 0 )
      continue;
    label = new QLabel( *mNameDict[ it.key() ] + ":", page );
    KLineEdit *lineedit = new KLineEdit( page );
    mLineEditDict.insert( it.key(), lineedit );
    lineedit->setText( it.data() );
    label->setBuddy( lineedit );
    layout->addWidget( label, i, 0 );
    layout->addWidget( lineedit, i, 1 );
  }

  KAcceleratorManager::manage( this );
}

AttributesDialog::~AttributesDialog()
{
}

QMap<QString, QString> AttributesDialog::attributes() const
{
  QMap<QString, QString> map;

  QDictIterator<KLineEdit> it( mLineEditDict );
  for ( ; it.current(); ++it )
    map.insert( it.currentKey(), it.current()->text() );

  return map;
}

void AttributesDialog::mapChanged( int pos )
{
  // default map
  QMap<QString, QString> defaultMap;
  defaultMap.insert( "objectClass", "inetOrgPerson" );
  defaultMap.insert( "commonName", "cn" );
  defaultMap.insert( "formattedName", "displayName" );
  defaultMap.insert( "familyName", "sn" );
  defaultMap.insert( "givenName", "givenName" );
  defaultMap.insert( "mail", "mail" );
  defaultMap.insert( "mailAlias", "" );
  defaultMap.insert( "phoneNumber", "telephoneNumber" );
  defaultMap.insert( "uid", "uid" );
  defaultMap.insert( "jpegPhoto", "" );

  // apply first the default and than the spezific changes
  QMap<QString, QString>::Iterator it;
  for ( it = defaultMap.begin(); it != defaultMap.end(); ++it )
    mLineEditDict[ it.key() ]->setText( it.data() );

  for ( it = mMapList[ pos ].begin(); it != mMapList[ pos ].end(); ++it ) {
    if ( !it.data().isEmpty() )
      mLineEditDict[ it.key() ]->setText( it.data() );
  }
}

#include "ldapconfigwidget.moc"
