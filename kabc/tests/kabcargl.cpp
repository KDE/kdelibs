/*
    This file is part of libkabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include "stdaddressbook.h"

using namespace KABC;

int main(int argc,char **argv)
{
  KAboutData aboutData("kabcargl","Fix broken pre3.0rc3 format","0.1");
  aboutData.addAuthor("Cornelius Schumacher", 0, "schumacher@kde.org");

  KCmdLineArgs::init(argc,argv,&aboutData);

  KApplication app;

  QString filename = StdAddressBook::fileName();
  
  QFile f( filename );
  if ( !f.open( IO_ReadOnly ) ) {
    kdDebug() << "Error opening file '" << filename << "' for reading." << endl;
    return 1;
  }
  
  QTextStream t( &f );
  t.setEncoding(QTextStream::UnicodeUTF8);
  QString text = t.read();
  f.close();
  
  text = QString::fromUtf8( text.local8Bit() );
  text.replace( "\n", "\r\n" );
  
  if ( !f.open( IO_WriteOnly ) ) {
    kdDebug() << "Error opening file '" << filename << "' for writing." << endl;
    return 1;
  }
  
  QTextStream t2( &f );
  t2.setEncoding(QTextStream::UnicodeUTF8);
  t2 << text;
  f.close();
}
