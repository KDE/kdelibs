/*
    This file is part of libkabc.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include <qlabel.h>
#include <qlayout.h>

#include <kdebug.h>
#include <klocale.h>
#include <kdialog.h>

#include "formatfactory.h"
#include "resourcenet.h"
#include "stdaddressbook.h"

#include "resourcenetconfig.h"

using namespace KABC;

ResourceNetConfig::ResourceNetConfig( QWidget* parent, const char* name )
    : ConfigWidget( parent, name ), mInEditMode( false )
{
  QGridLayout *mainLayout = new QGridLayout( this, 2, 2, 0,
      KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "Format:" ), this );
  mFormatBox = new KComboBox( this );

  mainLayout->addWidget( label, 0, 0 );
  mainLayout->addWidget( mFormatBox, 0, 1 );

  label = new QLabel( i18n( "Location:" ), this );
  mUrlEdit = new KURLRequester( this );
  mUrlEdit->setMode( KFile::File );

  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addWidget( mUrlEdit, 1, 1 );

  FormatFactory *factory = FormatFactory::self();
  QStringList formats = factory->formats();
  QStringList::Iterator it;
  for ( it = formats.begin(); it != formats.end(); ++it ) {
    FormatInfo *info = factory->info( *it );
    if ( info ) {
      mFormatTypes << (*it);
      mFormatBox->insertItem( info->nameLabel );
    }
  }
}

void ResourceNetConfig::setEditMode( bool value )
{
  mFormatBox->setEnabled( !value );
  mInEditMode = value;
}

void ResourceNetConfig::loadSettings( KRES::Resource *res )
{
  ResourceNet *resource = dynamic_cast<ResourceNet*>( res );

  if ( !resource ) {
    kdDebug(5700) << "ResourceNetConfig::loadSettings(): cast failed" << endl;
    return;
  }

  mFormatBox->setCurrentItem( mFormatTypes.findIndex( resource->format() ) );

  mUrlEdit->setURL( resource->url().url() );
}

void ResourceNetConfig::saveSettings( KRES::Resource *res )
{
  ResourceNet *resource = dynamic_cast<ResourceNet*>( res );

  if ( !resource ) {
    kdDebug(5700) << "ResourceNetConfig::saveSettings(): cast failed" << endl;
    return;
  }

  if ( !mInEditMode )
    resource->setFormat( mFormatTypes[ mFormatBox->currentItem() ] );

  resource->setUrl( KURL( mUrlEdit->url() ) );
}

#include "resourcenetconfig.moc"
