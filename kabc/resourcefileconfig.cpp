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
#include "resourcefileconfig.h"
#include "stdaddressbook.h"

using namespace KABC;

ResourceFileConfig::ResourceFileConfig( QWidget* parent,  const char* name )
    : ResourceConfigWidget( parent, name )
{
  resize( 245, 115 ); 
  QGridLayout *mainLayout = new QGridLayout( this, 2, 2 );

  QLabel *label = new QLabel( i18n( "Format:" ), this );
  formatBox = new KComboBox( this );

  mainLayout->addWidget( label, 0, 0 );
  mainLayout->addWidget( formatBox, 0, 1 );

  label = new QLabel( i18n( "Location:" ), this );
  fileNameEdit = new KURLRequester( this );

  connect( fileNameEdit, SIGNAL( textChanged( const QString & ) ), SLOT( checkFilePermissions( const QString & ) ) );

  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addWidget( fileNameEdit, 1, 1 );

  FormatFactory *factory = FormatFactory::self();
  QStringList formats = factory->formats();
  QStringList::Iterator it;
  for ( it = formats.begin(); it != formats.end(); ++it ) {
    FormatInfo *info = factory->info( *it );
    if ( info ) {
      mFormatTypes << (*it);
      formatBox->insertItem( info->nameLabel );
    }
  }
}

void ResourceFileConfig::loadSettings( KConfig *config )
{
  QString format = config->readEntry( "FileFormat" );
  formatBox->setCurrentItem( mFormatTypes.findIndex( format ) );

  fileNameEdit->setURL( config->readEntry( "FileName" ) );    
  if ( fileNameEdit->url().isEmpty() )
    fileNameEdit->setURL( KABC::StdAddressBook::fileName() );
}

void ResourceFileConfig::saveSettings( KConfig *config )
{
  config->writeEntry( "FileFormat", mFormatTypes[ formatBox->currentItem() ] );
  config->writeEntry( "FileName", fileNameEdit->url() );
}

void ResourceFileConfig::checkFilePermissions( const QString& fileName )
{
  QFile file( fileName );
  if ( !file.open( IO_Append ) )
    emit setReadOnly( true );

  file.close();
}

#include "resourcefileconfig.moc"
