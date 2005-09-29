/*
 *  Copyright (C) 2002 David Faure   <faure@kde.org>
 *  Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <kmimetype.h>
#include <kmimemagic.h>
#include <kinstance.h>
#include <stdio.h>

int main( int argc, char** argv )
{
  if (argc != 2)
  {
      printf("\n"
             "Usage : ./kmimemagictest file\n");
      return 1;
  }
  KInstance blah("kmimemagictest");

  QString file = QString::fromLocal8Bit( argv[1] );

  KMimeMagicResult * result = KMimeMagic::self()->findFileType( file );

  if ( result->isValid() )
      printf( "Found %s, accuracy %d\n", result->mimeType().latin1(), result->accuracy() );
  else
      printf( "Invalid result\n");

  KMimeType::Format f = KMimeType::findFormatByFileContent( file );
  if (f.text)
    printf("Text\n");
  else
    printf("Binary\n");

  if (f.compression == KMimeType::Format::GZipCompression)
    printf("GZipped\n");

  return 0;
}
