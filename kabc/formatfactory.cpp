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

#include <kdebug.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include <qfile.h>

#include "vcardformat.h"

#include "formatfactory.h"

using namespace KABC;

FormatFactory *FormatFactory::mSelf = 0;

FormatFactory *FormatFactory::self()
{
  kdDebug(5700) << "FormatFactory::self()" << endl;

  if ( !mSelf ) {
    mSelf = new FormatFactory;
  }

  return mSelf;
}

FormatFactory::FormatFactory()
{
  mFormatList.setAutoDelete( true );

  // dummy entry for default format
  FormatInfo *info = new FormatInfo;
  info->library = "<NoLibrary>";
  info->nameLabel = i18n( "vCard" );
  info->descriptionLabel = i18n( "vCard Format" );
  mFormatList.insert( "vcard", info );

  QStringList list = KGlobal::dirs()->findAllResources( "data" ,"kabc/formats/*.desktop", true, true );
  for ( QStringList::iterator it = list.begin(); it != list.end(); ++it )
  {
    KSimpleConfig config( *it, true );

    if ( !config.hasGroup( "Misc" ) || !config.hasGroup( "Plugin" ) )
	    continue;

    info = new FormatInfo;

    config.setGroup( "Plugin" );
    QString type = config.readEntry( "Type" );
    info->library = config.readEntry( "X-KDE-Library" );
	
    config.setGroup( "Misc" );
    info->nameLabel = config.readEntry( "Name" );
    info->descriptionLabel = config.readEntry( "Comment", i18n( "No description available." ) );

    mFormatList.insert( type, info );
  }
}

FormatFactory::~FormatFactory()
{
  mFormatList.clear();
}

QStringList FormatFactory::formats()
{
  QStringList retval;
	
  // make sure 'vcard' is the first entry
  retval << "vcard";

  QDictIterator<FormatInfo> it( mFormatList );
  for ( ; it.current(); ++it )
    if ( it.currentKey() != "vcard" )
      retval << it.currentKey();

  return retval;
}

FormatInfo *FormatFactory::info( const QString &type )
{
  if ( type.isEmpty() )
    return 0;
  else
    return mFormatList[ type ];
}

Format *FormatFactory::format( const QString& type )
{
  Format *format = 0;

  if ( type.isEmpty() )
    return 0;

  if ( type == "vcard" ) {
	  format = new VCardFormat;
    format->setType( type );
    format->setNameLabel( i18n( "vCard" ) );
    format->setDescriptionLabel( i18n( "vCard Format" ) );
    return format;
  }

  FormatInfo *fi = mFormatList[ type ];
  if (!fi)
	  return 0;
  QString libName = fi->library;

  KLibrary *library = openLibrary( libName );
  if ( !library )
    return 0;

  void *format_func = library->symbol( "format" );

  if ( format_func ) {
    format = ((Format* (*)())format_func)();
    format->setType( type );
    format->setNameLabel( fi->nameLabel );
    format->setDescriptionLabel( fi->descriptionLabel );
  } else {
    kdDebug( 5700 ) << "'" << libName << "' is not a format plugin." << endl;
    return 0;
  }

  return format;
}


KLibrary *FormatFactory::openLibrary( const QString& libName )
{
  KLibrary *library = 0;

  QString path = KLibLoader::findLibrary( QFile::encodeName( libName ) );

  if ( path.isEmpty() ) {
    kdDebug( 5700 ) << "No format plugin library was found!" << endl;
    return 0;
  }

  library = KLibLoader::self()->library( QFile::encodeName( path ) );

  if ( !library ) {
    kdDebug( 5700 ) << "Could not load library '" << libName << "'" << endl;
    return 0;
  }

  return library;
}
