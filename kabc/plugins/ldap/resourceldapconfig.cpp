/*
    This file is part of libkabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qvbox.h>

#include <kaccelmanager.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <klineedit.h>

#include "resourceldap.h"

#include "resourceldapconfig.h"

using namespace KABC;

ResourceLDAPConfig::ResourceLDAPConfig( QWidget* parent,  const char* name )
  : KRES::ResourceConfigWidget( parent, name )
{
  resize( 250, 120 ); 
  QGridLayout *mainLayout = new QGridLayout( this, 8, 2 );

  QLabel *label = new QLabel( i18n( "User:" ), this );
  mUser = new KLineEdit( this );

  mainLayout->addWidget( label, 0, 0 );
  mainLayout->addWidget( mUser, 0, 1 );

  label = new QLabel( i18n( "Password:" ), this );
  mPassword = new KLineEdit( this );
  mPassword->setEchoMode( KLineEdit::Password );

  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addWidget( mPassword, 1, 1 );

  label = new QLabel( i18n( "Host:" ), this );
  mHost = new KLineEdit( this );

  mainLayout->addWidget( label, 2, 0 );
  mainLayout->addWidget( mHost, 2, 1 );

  label = new QLabel( i18n( "Port:" ), this );
  QVBox *box = new QVBox( this );
  mPort = new QSpinBox( 0, 65535, 1, box );
  mPort->setSizePolicy( QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred ) );
  mPort->setValue( 389 );
  new QWidget( box, "dummy" );

  mainLayout->addWidget( label, 3, 0 );
  mainLayout->addWidget( box, 3, 1 );

  label = new QLabel( i18n( "Dn:" ), this );
  mDn = new KLineEdit( this );

  mainLayout->addWidget( label, 4, 0 );
  mainLayout->addWidget( mDn, 4, 1 );

  label = new QLabel( i18n( "Filter:" ), this );
  mFilter = new KLineEdit( this );

  mainLayout->addWidget( label, 5, 0 );
  mainLayout->addWidget( mFilter, 5, 1 );

  mAnonymous = new QCheckBox( i18n( "Anonymous login" ), this );
  mainLayout->addMultiCellWidget( mAnonymous, 6, 6, 0, 1 );

  mEditButton = new QPushButton( i18n( "Edit Attributes..." ), this );
  mainLayout->addMultiCellWidget( mEditButton, 7, 7, 0, 1 );

  connect( mAnonymous, SIGNAL( toggled(bool) ), mUser, SLOT( setDisabled(bool) ) );
  connect( mAnonymous, SIGNAL( toggled(bool) ), mPassword, SLOT( setDisabled(bool) ) );
  connect( mEditButton, SIGNAL( clicked() ), SLOT( editAttributes() ) );
}

void ResourceLDAPConfig::loadSettings( KRES::Resource *res )
{
  ResourceLDAP *resource = dynamic_cast<ResourceLDAP*>( res );
  
  if ( !resource ) {
    kdDebug(5700) << "ResourceLDAPConfig::loadSettings(): cast failed" << endl;
    return;
  }

  mUser->setText( resource->user() );
  mPassword->setText( resource->password() );
  mHost->setText( resource->host() );
  mPort->setValue(  resource->port() );
  mDn->setText( resource->dn() );
  mFilter->setText( resource->filter() );
  mAnonymous->setChecked( resource->isAnonymous() );
  mAttributes = resource->attributes();
}

void ResourceLDAPConfig::saveSettings( KRES::Resource *res )
{
  ResourceLDAP *resource = dynamic_cast<ResourceLDAP*>( res );
  
  if ( !resource ) {
    kdDebug(5700) << "ResourceLDAPConfig::saveSettings(): cast failed" << endl;
    return;
  }

  resource->setUser( mUser->text() );
  resource->setPassword( mPassword->text() );
  resource->setHost( mHost->text() );
  resource->setPort( mPort->value() );
  resource->setDn( mDn->text() );
  resource->setFilter( mFilter->text() );
  resource->setIsAnonymous( mAnonymous->isChecked() );
  resource->setAttributes( mAttributes );
}

void ResourceLDAPConfig::editAttributes()
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
  mNameDict.insert( "commonName", new QString( i18n( "Common Name" ) ) );
  mNameDict.insert( "formattedName", new QString( i18n( "Formatted Name" ) ) );
  mNameDict.insert( "familyName", new QString( i18n( "Family Name" ) ) );
  mNameDict.insert( "givenName", new QString( i18n( "Given name" ) ) );
  mNameDict.insert( "mail", new QString( i18n( "E-Mail" ) ) );
  mNameDict.insert( "mailAlias", new QString( i18n( "E-Mail alias" ) ) );
  mNameDict.insert( "phoneNumber", new QString( i18n( "Telephone Number" ) ) );
  mNameDict.insert( "uid", new QString( i18n( "UID" ) ) );

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
                                         marginHint(), spacingHint() );

  QLabel *label = new QLabel( i18n( "Templates" ), page );
  layout->addWidget( label, 0, 0 );
  mMapCombo = new KComboBox( page );
  layout->addWidget( mMapCombo, 0, 1 );

  mMapCombo->insertItem( i18n( "User defined" ) );
  mMapCombo->insertItem( i18n( "Kolab" ) );
  mMapCombo->insertItem( i18n( "Netscape" ) );
  mMapCombo->insertItem( i18n( "Evolution" ) );
  mMapCombo->insertItem( i18n( "Outlook" ) );
  connect( mMapCombo, SIGNAL( activated( int ) ), SLOT( mapChanged( int ) ) );

  QMap<QString, QString>::ConstIterator it;
  int i;
  for ( i = 1, it = attributes.begin(); it != attributes.end(); ++it, ++i ) {
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
  defaultMap.insert( "commonName", "cn" );
  defaultMap.insert( "formattedName", "displayName" );
  defaultMap.insert( "familyName", "sn" );
  defaultMap.insert( "givenName", "givenName" );
  defaultMap.insert( "mail", "mail" );
  defaultMap.insert( "mailAlias", "" );
  defaultMap.insert( "phoneNumber", "telephoneNumber" );
  defaultMap.insert( "uid", "uid" );

  // apply first the default and than the spezific changes
  QMap<QString, QString>::Iterator it;
  for ( it = defaultMap.begin(); it != defaultMap.end(); ++it )
    mLineEditDict[ it.key() ]->setText( it.data() );

  for ( it = mMapList[ pos ].begin(); it != mMapList[ pos ].end(); ++it ) {
    if ( !it.data().isEmpty() )
      mLineEditDict[ it.key() ]->setText( it.data() );
  }
}

#include "resourceldapconfig.moc"
