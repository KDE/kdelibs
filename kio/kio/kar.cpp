
/* This file is part of the KDE libraries
   Copyright (C) 2002 Laurence Anderson <l.d.anderson@warwick.ac.uk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

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
#include <qdir.h>
#include <time.h>
#include <kdebug.h>
#include <qptrlist.h>
#include <kmimetype.h>
#include <qregexp.h>

#include "kfilterdev.h"
#include "kar.h"
//#include "klimitediodevice.h"

////////////////////////////////////////////////////////////////////////
/////////////////////////// KAr ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////

class KAr::KArPrivate
{
public:
    KArPrivate() {}
};

KAr::KAr( const QString& filename )
    : KArchive( 0L )
{
    //kdDebug(7040) << "KAr(filename) reached." << endl;
    m_filename = filename;
    d = new KArPrivate;
    setDevice( new QFile( filename ) );
}

KAr::KAr( QIODevice * dev )
    : KArchive( dev )
{
    //kdDebug(7040) << "KAr::KAr( QIODevice * dev) reached." << endl;
    d = new KArPrivate;
}

KAr::~KAr()
{
    // mjarrett: Closes to prevent ~KArchive from aborting w/o device
    //kdDebug(7040) << "~KAr reached." << endl;
    if( isOpened() )
        close();
    if ( !m_filename.isEmpty() )
        delete device(); // we created it ourselves
    delete d;
}

bool KAr::openArchive( int mode )
{
    // Open archive
    
    //kdDebug(7040) << "openarchive reached." << endl;

    if ( mode == IO_WriteOnly )
        return true;
    if ( mode != IO_ReadOnly && mode != IO_ReadWrite )
    {
        kdWarning(7040) << "Unsupported mode " << mode << endl;
        return false;
    }

    QIODevice* dev = device();
    
    char magic[8];
    dev->readBlock (magic, 8);
    if (qstrncmp(magic, "!<arch>", 7) != 0) {
        kdDebug() << "Invalid main magic" << endl;
        return false;
    }

    char *ar_longnames = 0;
    while (! dev->atEnd()) {
        QCString ar_header;
        ar_header.resize(61);
        QCString name;
        int date, uid, gid, mode, size;

        dev->at( dev->at() + (2 - (dev->at() % 2)) % 2 ); // Ar headers are padded to byte boundry

        if ( dev->readBlock (ar_header.data(), 60) != 60 ) { // Read ar header
            kdDebug() << "Couldn't read header" << endl;
            if (ar_longnames) delete[] ar_longnames;
            //return false;
            return true; // Probably EOF / trailing junk
        }

        if (ar_header.right(2) != "`\n") { // Check header magic
            kdDebug() << "Invalid magic" << endl;
            if (ar_longnames) delete[] ar_longnames;
            return false;
        }

        name = ar_header.mid( 0, 16 ); // Process header
        date = ar_header.mid( 16, 12 ).toInt();
        uid = ar_header.mid( 28, 6 ).toInt();
        gid = ar_header.mid( 34, 6 ).toInt();
        mode = ar_header.mid( 40, 8 ).toInt();
        size = ar_header.mid( 48, 10 ).toInt();

        bool skip_entry = false; // Deal with special entries
        if (name.mid(0, 1) == "/") {
            if (name.mid(1, 1) == "/") { // Longfilename table entry
                if (ar_longnames) delete[] ar_longnames;
                ar_longnames = new char[size + 1];
                ar_longnames[size] = '\0';
                dev->readBlock (ar_longnames, size);
                skip_entry = true;
                kdDebug() << "Read in longnames entry" << endl;
            } else if (name.mid(1, 1) == " ") { // Symbol table entry
                kdDebug() << "Skipped symbol entry" << endl;
                dev->at( dev->at() + size );
                skip_entry = true;
            } else { // Longfilename
                kdDebug() << "Longfilename #" << name.mid(1, 15).toInt() << endl;
                if (! ar_longnames) {
                    kdDebug() << "Invalid longfilename reference" << endl;
                    return false;
                }
                name = &ar_longnames[name.mid(1, 15).toInt()];
                name = name.left(name.find("/"));
            }
        }
        if (skip_entry) continue;

        name = name.stripWhiteSpace(); // Process filename
        name.replace( QRegExp("/"), "" );
        kdDebug() << "Filename: " << name << " Size: " << size << endl;

        KArchiveEntry* entry;
        entry = new KArchiveFile(this, name, mode, date, /*uid*/ 0, /*gid*/ 0, 0, dev->at(), size);
        rootDir()->addEntry(entry); // Ar files don't support directorys, so everything in root    
        
        dev->at( dev->at() + size ); // Skip contents
    }
    if (ar_longnames) delete[] ar_longnames;

    return true;
}

bool KAr::closeArchive()
{
    // Close the archive
    return true;
}

void KAr::virtual_hook( int id, void* data )
{ KArchive::virtual_hook( id, data ); }
