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

#include <qlabel.h>
#include <qlayout.h>

#include <klocale.h>

#include "resource.h"
#include "resourcesqlconfig.h"

using namespace KABC;

ResourceSqlConfig::ResourceSqlConfig( QWidget* parent,  const char* name )
    : ResourceConfigWidget( parent, name )
{
  resize( 285, 167 ); 

  QGridLayout *mainLayout = new QGridLayout( this, 4, 2 );

  QLabel *label = new QLabel( i18n( "Username:" ), this );
  user = new KLineEdit( this );

  mainLayout->addWidget( label, 0, 0 );
  mainLayout->addWidget( user, 0, 1 );

  label = new QLabel( i18n( "Password:" ), this );
  password = new KLineEdit( this );
  password->setEchoMode( KLineEdit::Password );

  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addWidget( password, 1, 1 );

  label = new QLabel( i18n( "Host:" ), this );
  host = new KLineEdit( this );

  mainLayout->addWidget( label, 2, 0 );
  mainLayout->addWidget( host, 2, 1 );

  label = new QLabel( i18n( "Database:" ), this );
  dbName = new KLineEdit( this );

  mainLayout->addWidget( label, 3, 0 );
  mainLayout->addWidget( dbName, 3, 1 );
}

void ResourceSqlConfig::loadSettings( KConfig *config )
{
  user->setText( config->readEntry( "SqlUser" ) );
  password->setText( KABC::Resource::cryptStr( config->readEntry( "SqlPassword" ) ) );
  dbName->setText( config->readEntry( "SqlName" ) );
  host->setText( config->readEntry( "SqlHost" ) );
}

void ResourceSqlConfig::saveSettings( KConfig *config )
{
  config->writeEntry( "SqlUser", user->text() );
  config->writeEntry( "SqlPassword", KABC::Resource::cryptStr( password->text() ) );
  config->writeEntry( "SqlName", dbName->text() );
  config->writeEntry( "SqlHost", host->text() );
}

#include "resourcesqlconfig.moc"
