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
#include "resourceldapconfig.h"

ResourceLDAPConfig::ResourceLDAPConfig( QWidget* parent,  const char* name )
    : ResourceConfigWidget( parent, name )
{
    resize( 250, 120 ); 
    QGridLayout *mainLayout = new QGridLayout( this, 6, 2 );

    QLabel *label = new QLabel( i18n( "User:" ), this );
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

    label = new QLabel( i18n( "Port:" ), this );
    port = new KLineEdit( this );

    mainLayout->addWidget( label, 3, 0 );
    mainLayout->addWidget( port, 3, 1 );

    label = new QLabel( i18n( "Dn:" ), this );
    dn = new KLineEdit( this );

    mainLayout->addWidget( label, 4, 0 );
    mainLayout->addWidget( dn, 4, 1 );

    label = new QLabel( i18n( "Filter:" ), this );
    filter = new KLineEdit( this );

    mainLayout->addWidget( label, 5, 0 );
    mainLayout->addWidget( filter, 5, 1 );
}

void ResourceLDAPConfig::loadSettings( KConfig *config )
{
    user->setText( config->readEntry( "LdapUser" ) );
    password->setText( KABC::Resource::cryptStr( config->readEntry( "LdapPassword" ) ) );
    host->setText( config->readEntry( "LdapHost" ) );
    port->setText( config->readEntry( "LdapPort" ) );
    dn->setText( config->readEntry( "LdapDn" ) );
    filter->setText( config->readEntry( "LdapFilter" ) );
}

void ResourceLDAPConfig::saveSettings( KConfig *config )
{
    config->writeEntry( "LdapUser", user->text() );
    config->writeEntry( "LdapPassword", KABC::Resource::cryptStr( password->text() ) );
    config->writeEntry( "LdapHost", host->text() );
    config->writeEntry( "LdapPort", port->text() );
    config->writeEntry( "LdapDn", dn->text() );
    config->writeEntry( "LdapFilter", filter->text() );
}
