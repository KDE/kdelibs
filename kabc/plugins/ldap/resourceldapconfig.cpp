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
#include <qspinbox.h>
#include <qvbox.h>

#include <klocale.h>
#include <klineedit.h>

#include "resource.h"
#include "resourceldapconfig.h"

using namespace KABC;

ResourceLDAPConfig::ResourceLDAPConfig( QWidget* parent,  const char* name )
    : ResourceConfigWidget( parent, name )
{
  resize( 250, 120 ); 
  QGridLayout *mainLayout = new QGridLayout( this, 6, 2 );

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
  QVBox *box = new QVBox(this);
  mPort = new QSpinBox(0, 65535, 1, box );
  mPort->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
  mPort->setValue(389);
  new QWidget(box, "dummy");

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

  mAnonymous = new QCheckBox( i18n( "Anonymous Login" ), this );
  mainLayout->addMultiCellWidget( mAnonymous, 6, 6, 0, 1 );

  connect( mAnonymous, SIGNAL( toggled(bool) ), mUser, SLOT( setDisabled(bool) ) );
  connect( mAnonymous, SIGNAL( toggled(bool) ), mPassword, SLOT( setDisabled(bool) ) );
}

void ResourceLDAPConfig::loadSettings( KConfig *config )
{
  mUser->setText( config->readEntry( "LdapUser" ) );
  mPassword->setText( KABC::Resource::cryptStr( config->readEntry( "LdapPassword" ) ) );
  mHost->setText( config->readEntry( "LdapHost" ) );
  mPort->setValue(  config->readNumEntry( "LdapPort", 389 ) );
  mDn->setText( config->readEntry( "LdapDn" ) );
  mFilter->setText( config->readEntry( "LdapFilter" ) );
  mAnonymous->setChecked( config->readBoolEntry( "LdapAnonymous" ) );
}

void ResourceLDAPConfig::saveSettings( KConfig *config )
{
  config->writeEntry( "LdapUser", mUser->text() );
  config->writeEntry( "LdapPassword", KABC::Resource::cryptStr( mPassword->text() ) );
  config->writeEntry( "LdapHost", mHost->text() );
  config->writeEntry( "LdapPort", mPort->value() );
  config->writeEntry( "LdapDn", mDn->text() );
  config->writeEntry( "LdapFilter", mFilter->text() );
  config->writeEntry( "LdapAnonymous", mAnonymous->isChecked() );
}

#include "resourceldapconfig.moc"
