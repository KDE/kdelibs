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

#include <qfile.h>
#include <qtextstream.h>

#include "vcardtool.h"

int main()
{
  QFile file( "test.vcf" );
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
  text = tool.createVCards( list ); // uses version 3.0

  file.setName( "testout.vcf" );
  if ( !file.open( IO_WriteOnly ) ) {
    qDebug( "Unable to open file '%s' for writing!", file.name().latin1() );
    return 1;
  }

  s.setDevice( &file );
  s << text;
  file.close();

  return 0;
}
