/*
    This file is part of libkabc.

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

#include <iostream>
#include <stdlib.h>

#include <qfile.h>
#include <qtextstream.h>

#include <kprocess.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kaboutdata.h>

#include "vcardtool.h"
#include "vcard.h"

static const KCmdLineOptions options[] =
{
  {"vcard21", I18N_NOOP("vCard 2.1"), 0},
  {"+inputfile", I18N_NOOP("Input File"), 0},
  {0,0,0}
};

int main( int argc, char **argv )
{
  KAboutData aboutData( "testread", "vCard test reader", "0.1" );
  aboutData.addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app( false, false );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  if ( args->count() != 1 ) {
    std::cerr << "Missing argument" << std::endl;
    return 1;
  }

  QString inputFile( args->arg( 0 ) );

  QFile file( inputFile );
  if ( !file.open( IO_ReadOnly ) ) {
    qDebug( "Unable to open file '%s' for reading!", file.name().latin1() );
    return 1;
  }

  QString text;

  QTextStream s( &file );
  s.setEncoding( QTextStream::UnicodeUTF8 );
  text = s.read();
  file.close();

  KABC::VCardTool tool;
  KABC::Addressee::List list = tool.parseVCards( text );

  if ( args->isSet( "vcard21" ) ) {
    text = tool.createVCards( list, KABC::VCard::v2_1 ); // uses version 2.1
  } else {
    text = tool.createVCards( list ); // uses version 3.0
  }

  std::cout << text.utf8();

  return 0;
}
