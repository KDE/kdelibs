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

#include <qdatastream.h>
#include <qdir.h>
#include <qfile.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <klocale.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "addressbook.h"
#include "binaryformat.h"
#include "vcardformat.h"

static const KCmdLineOptions options[] =
{
  { "input <file>", I18N_NOOP("Name of input addressbook."), 0 },
  { "output <directory>", I18N_NOOP("Name of output directory."), 0 },
  { "format <format>", I18N_NOOP("Format of the input addressbook (vcard or binary)."), 0 },
  { 0, 0, 0 }
};

bool loadBinaryFormat( KABC::AddressBook *ab, QFile *file )
{
  QDataStream s( file );

  Q_UINT32 magic, version, entries;
    
  s >> magic >> version >> entries;

  if ( magic != 0x2e93e ) {
    kdError() << "File '" << file->name() << "' is not binary format." << endl;
    return false;
  }

  if ( version != 1 ) {
    kdError() << "File '" << file->name() << "' has wrong version." << endl;
    return false;
  }

  for ( uint i = 0; i < entries; ++i ) {
    KABC::Addressee addressee;
    s >> addressee;
    ab->insertAddressee( addressee );
  }

  return true;
}

int main(int argc,char **argv)
{
  KAboutData aboutData("splitformat", I18N_NOOP("file-format to directory-format converter"), "0.1");
  aboutData.addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  QString srcName, destName, formatName;


  if ( args->isSet( "input" ) )
    srcName = args->getOption( "input" );
  if ( args->isSet( "output" ) )
    destName = args->getOption( "output" );
  if ( args->isSet( "format" ) )
    formatName = args->getOption( "format" );

  if ( srcName.isEmpty() ) {
    kdError() << "Missing name of input file" << endl;
    return -1;
  }

  if ( destName.isEmpty() ) {
    kdError() << "Missing name of output directory" << endl;
    return -1;
  }
  
  if ( formatName.isEmpty() ) {
    kdError() << "Missing format type" << endl;
    return -1;
  }

  KABC::Format *format;
  if ( formatName == "vcard" ) {
    format = new KABC::VCardFormat;
  } else if ( formatName == "binary" ) {
    format = new KABC::BinaryFormat;
  } else {
    kdError() << "Unknown format '" << formatName << "'. Use 'vcard' or 'binary'!" << endl;
    return -1;
  }

  QFile file( srcName );
  if ( !file.open( IO_ReadOnly ) ) {
    kdError() << "Can't open '" << srcName << "'" << endl;
    return -1;
  }

  QDir dir( destName );
  if ( !dir.exists() ) {
    if ( mkdir( QFile::encodeName( dir.path() ), 0777 ) != 0 ) {
      kdError() << "Can't create '" << destName << "'" << endl;
      return -1;
    }
  }

  KABC::AddressBook ab;

  bool ok = false;
  if ( formatName == "vcard" ) {
    ok = format->load( &ab, 0, &file );
  } else if ( formatName == "binary" ) {
    ok = loadBinaryFormat( &ab, &file );
  }

  if ( !ok ) {
    kdError() << "Can't load addressees from '" << srcName << "'" << endl;
    return -1;
  }

  KABC::AddressBook::Iterator it;
  for ( it = ab.begin(); it != ab.end(); ++it ) {
    QFile addrFile( destName + "/" + (*it).uid() );
    if ( addrFile.exists() ) {
      kdError() << "File '" << addrFile.name() << "' already exists. Skipped!" << endl;
      continue;
    }

    if ( !addrFile.open( IO_ReadWrite ) ) {
      kdError() << "Can't open '" << addrFile.name() << "'. Skipped!" << endl;
      continue;
    }

    if ( !format->save( *it, &addrFile ) ) {
      kdError() << "Can't save '" << addrFile.name() << "'. Skipped!" << endl;
      continue;
    }

    addrFile.close();
  }

  file.close();

  return 0;
}
