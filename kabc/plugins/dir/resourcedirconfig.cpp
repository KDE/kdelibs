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
#include <kstandarddirs.h>

#include "formatfactory.h"
#include "resourcedirconfig.h"
#include "stdaddressbook.h"

using namespace KABC;

ResourceDirConfig::ResourceDirConfig( QWidget* parent,  const char* name )
    : ResourceConfigWidget( parent, name )
{
  resize( 245, 115 ); 
  QGridLayout *mainLayout = new QGridLayout( this, 2, 2 );

  QLabel *label = new QLabel( i18n( "Format:" ), this );
  mFormatBox = new KComboBox( this );

  mainLayout->addWidget( label, 0, 0 );
  mainLayout->addWidget( mFormatBox, 0, 1 );

  label = new QLabel( i18n( "Location:" ), this );
  mFileNameEdit = new KURLRequester( this );
  mFileNameEdit->setMode( KFile::Directory );

  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addWidget( mFileNameEdit, 1, 1 );

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

void ResourceDirConfig::loadSettings( KConfig *config )
{
  QString format = config->readEntry( "FileFormat" );
  mFormatBox->setCurrentItem( mFormatTypes.findIndex( format ) );

  mFileNameEdit->setURL( config->readEntry( "FilePath" ) );    
  if ( mFileNameEdit->url().isEmpty() )
    mFileNameEdit->setURL( KABC::StdAddressBook::directoryName() );
}

void ResourceDirConfig::saveSettings( KConfig *config )
{
  config->writeEntry( "FileFormat", mFormatTypes[ mFormatBox->currentItem() ] );
  config->writeEntry( "FilePath", mFileNameEdit->url() );
}

#include "resourcedirconfig.moc"
