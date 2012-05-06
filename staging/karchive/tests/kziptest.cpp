/*
 *  Copyright (C) 2002-2006 David Faure   <faure@kde.org>
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

#include "kzip.h"
#include <stdio.h>
#include <QtCore/QDebug>
#include <QtCore/QFile>

#include <assert.h>

void recursive_print( const KArchiveDirectory * dir, const QString & path )
{
  QStringList l = dir->entries();
  QStringList::Iterator it = l.begin();
  for( ; it != l.end(); ++it )
  {
    const KArchiveEntry* entry = dir->entry( (*it) );
    printf("mode=%07o %s %s size: %lld pos: %lld %s%s isdir=%d%s", entry->permissions(),
	entry->user().toLatin1().constData(), entry->group().toLatin1().constData(),
	entry->isDirectory() ? 0 : ((KArchiveFile*)entry)->size(),
	entry->isDirectory() ? 0 : ((KArchiveFile*)entry)->position(),
	path.toLatin1().constData(), (*it).toLatin1().constData(), entry->isDirectory(),
	entry->symLinkTarget().isEmpty() ? "" : QString(" symlink: %1").arg(entry->symLinkTarget()).toLatin1().constData() );

//    if (!entry->isDirectory()) printf("%d", ((KArchiveFile*)entry)->size());
    printf("\n");
    if (entry->isDirectory())
      recursive_print( (KArchiveDirectory *)entry, path+(*it)+'/' );
  }
}


void recursive_transfer(const KArchiveDirectory * dir,
	    const QString & path, KZip * zip)
{
    QStringList l = dir->entries();
    QStringList::Iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
        const KArchiveEntry* e = dir->entry( (*it) );
	qDebug() << "actual file: " << e->name();
	if (e->isFile())
	{
    	    Q_ASSERT( e && e->isFile() );
    	    const KArchiveFile* f = (KArchiveFile*)e;
    	    printf("FILE=%s\n", e->name().toLatin1().constData());

    	    QByteArray arr( f->data() );
    	    printf("SIZE=%i\n",arr.size() );
    	    QString str( arr );
    	    printf("DATA=%s\n", str.toLatin1().constData());

	    if (e->symLinkTarget().isEmpty()) {
	        zip->writeFile( path + e->name(),
                                "holgi", "holgrp",
                                arr.constData(), arr.size() );
	    } else
	        zip->writeSymLink(path+e->name(), e->symLinkTarget(), "leo", "leo",
				0120777, 1000000000l, 1000000000l, 1000000000l);
	}
	else if (e->isDirectory())
	{
	    recursive_transfer((KArchiveDirectory *)e ,
			path+e->name()+'/', zip);
	}
    }
}

int main( int argc, char** argv )
{
  if (argc < 3)
  {
    // ###### Note: please consider adding new tests to karchivetest (so that they can be automated)
    // rather than here (interactive)
    printf("\n"
 " Usage :\n"
 " ./kziptest list /path/to/existing_file.zip       tests listing an existing zip\n"
 " ./kziptest print file.zip                        prints contents of all files.\n"
 " ./kziptest print2 file.zip filename              prints contents of one file.\n"
 " ./kziptest update file.zip filename              updates contents of one file.\n"
 " ./kziptest transfer file.zip newfile.zip         complete transfer.\n" );
    return 1;
  }
  QString command = argv[1];
  if ( command == "list" )
  {
    KZip zip( argv[2] );

    if ( !zip.open( QIODevice::ReadOnly ) )
    {
      printf("Could not open %s for reading\n", argv[2] );
      return 1;
    }

    const KArchiveDirectory* dir = zip.directory();

    //printf("calling recursive_print\n");
    recursive_print( dir, "" );
    //printf("recursive_print called\n");

    zip.close();

    return 0;
  }
  else if (command == "print" )
  {
    KZip zip( argv[2] );
    qDebug() << "Opening zip file";
    if ( !zip.open( QIODevice::ReadOnly ) )
    {
      printf("Could not open %s for reading\n", argv[2] );
      return 1;
    }
    const KArchiveDirectory* dir = zip.directory();
    qDebug() << "Listing toplevel of zip file";
    QStringList l = dir->entries();
    QStringList::Iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
        const KArchiveEntry* e = dir->entry( (*it) );
	qDebug() << "Printing " << (*it);
	if (e->isFile())
	{
    	    Q_ASSERT( e && e->isFile() );
    	    const KArchiveFile* f = (KArchiveFile*)e;

    	    QByteArray arr( f->data() );
    	    printf("SIZE=%i\n",arr.size() );
    	    QString str( arr );
    	    printf("DATA=%s\n", str.toLatin1().constData());
	}
    }
    zip.close();
    return 0;
  }
  else if (command == "print2" )
  {
    if (argc != 4)
    {
        printf("usage: kziptest print2 archivename filename");
	return 1;
    }
    KZip zip( argv[2] );
    if ( !zip.open( QIODevice::ReadOnly ) )
    {
      printf("Could not open %s for reading\n", argv[2] );
      return 1;
    }
    const KArchiveDirectory* dir = zip.directory();
    const KArchiveEntry* e = dir->entry( argv[3] );
    Q_ASSERT( e && e->isFile() );
    const KArchiveFile* f = (KArchiveFile*)e;

    QByteArray arr( f->data() );
    printf("SIZE=%i\n",arr.size() );
    QString str( arr );
//    printf("DATA=%s\n", str.toLatin1().constData());
    printf("%s", str.toLatin1().constData());
    zip.close();

    return 0;

  }
  else if (command == "update" )
  {
    if (argc != 4)
    {
        printf("usage: kziptest update archivename filename");
       return 1;
    }
    KZip zip( argv[2] );
    if ( !zip.open( QIODevice::ReadWrite ) )
    {
      printf("Could not open %s for read/write\n", argv[2] );
      return 1;
    }
//    const KArchiveEntry* e = zip.directory()->entry( argv[3] );
//    Q_ASSERT( e && e->isFile() );
//    const KArchiveFile* f = (KArchiveFile*)e;

//    QCString data( "This is some new data that goes into " );
  //  data += argv[3];
    QFile f ( argv[3] );
    if (!f.open( QIODevice::ReadOnly ))
    {
      printf("Could not open %s for reading\n", argv[2] );
      return 1;
    }

    QDataStream s( &f );

    zip.writeFile( argv[3], "", "", f.readAll().constData(), f.size() );
    zip.close();

    return 0;

  }
  else if (command == "transfer" )
  {
    if (argc != 4)
    {
        printf("usage: kziptest transfer sourcefile destfile");
	return 1;
    }
    KZip zip1( argv[2] );
    KZip zip2( argv[3] );
    if ( !zip1.open( QIODevice::ReadOnly ) )
    {
      printf("Could not open %s for reading\n", argv[2] );
      return 1;
    }
    if ( !zip2.open( QIODevice::WriteOnly ) )
    {
      printf("Could not open %s for writing\n", argv[3] );
      return 1;
    }
    const KArchiveDirectory* dir1 = zip1.directory();

    recursive_transfer(dir1, "", &zip2 );

    zip1.close();
    zip2.close();

    return 0;

  }
  else
    printf("Unknown command\n");
}
