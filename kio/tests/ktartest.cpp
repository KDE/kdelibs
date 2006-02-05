/*
 *  Copyright (C) 2002 - 2005 David Faure   <faure@kde.org>
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

#include "ktar.h"
#include <stdio.h>
#include <kinstance.h>
#include <kdebug.h>

void recursive_print( const KArchiveDirectory * dir, const QString & path )
{
  QStringList l = dir->entries();
  l.sort();
  QStringList::ConstIterator it = l.begin();
  for( ; it != l.end(); ++it )
  {
    const KArchiveEntry* entry = dir->entry( (*it) );
    printf("mode=%07o %s %s %s%s isdir=%d\n", entry->permissions(), entry->user().latin1(), entry->group().latin1(), path.latin1(), (*it).latin1(), entry->isDirectory());
    if (entry->isDirectory())
      recursive_print( (KArchiveDirectory *)entry, path+(*it)+"/" );
  }
}

int main( int argc, char** argv )
{
    if (argc != 2)
    {
        printf("\n"
               " Usage :\n"
               " ./ktartest /path/to/existing_file.tar.gz       tests listing an existing tar.gz\n" );
        return 1;
    }
    KInstance instance("ktartest");
    KTar tar( argv[1] );

    if ( !tar.open( QIODevice::ReadOnly ) )
    {
      printf("Could not open %s for reading\n", argv[1] );
      return 1;
    }

    const KArchiveDirectory* dir = tar.directory();

    //printf("calling recursive_print\n");
    recursive_print( dir, "" );
    //printf("recursive_print called\n");

    tar.close();

    return 0;
}

