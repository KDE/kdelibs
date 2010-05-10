
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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kar.h"

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <time.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <QtCore/QRegExp>

#include "kfilterdev.h"
//#include "klimitediodevice_p.h"

////////////////////////////////////////////////////////////////////////
/////////////////////////// KAr ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////

class KAr::KArPrivate
{
public:
    KArPrivate() {}
};

KAr::KAr( const QString& filename )
    : KArchive( filename ), d(new KArPrivate)
{
}

KAr::KAr( QIODevice * dev )
    : KArchive( dev ), d(new KArPrivate)
{
}

KAr::~KAr()
{
    if( isOpen() )
        close();
    delete d;
}

bool KAr::doPrepareWriting( const QString&, const QString&, const QString&,
                            qint64, mode_t, time_t, time_t, time_t )
{
    return false;
}

bool KAr::doFinishWriting( qint64 )
{
    return false;
}

bool KAr::doWriteDir( const QString&, const QString&, const QString&,
                      mode_t, time_t, time_t, time_t )
{
    return false;
}

bool KAr::doWriteSymLink( const QString&, const QString&, const QString&,
                          const QString&, mode_t, time_t, time_t, time_t )
{
    return false;
}

bool KAr::openArchive( QIODevice::OpenMode mode )
{
    // Open archive

    if ( mode == QIODevice::WriteOnly )
        return true;
    if ( mode != QIODevice::ReadOnly && mode != QIODevice::ReadWrite )
    {
        kWarning(7042) << "Unsupported mode " << mode;
        return false;
    }

    QIODevice* dev = device();
    if ( !dev )
        return false;

    QByteArray magic = dev->read( 7 );
    if ( magic != "!<arch>" ) {
        kWarning(7042) << "Invalid main magic";
        return false;
    }

    char *ar_longnames = 0;
    while (! dev->atEnd()) {
        QByteArray ar_header;
        ar_header.resize(61);
        QByteArray name;
        int date, uid, gid, mode;
        qint64 size;

        dev->seek( dev->pos() + (2 - (dev->pos() % 2)) % 2 ); // Ar headers are padded to byte boundary

        if ( dev->read(ar_header.data(), 60) != 60 ) { // Read ar header
            kWarning(7042) << "Couldn't read header";
            delete[] ar_longnames;
            //return false;
            return true; // Probably EOF / trailing junk
        }

        if (!ar_header.endsWith("`\n")) { // Check header magic // krazy:exclude=strings
            kWarning(7042) << "Invalid magic";
            delete[] ar_longnames;
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
                delete[] ar_longnames;
                ar_longnames = new char[size + 1];
                ar_longnames[size] = '\0';
                dev->read(ar_longnames, size);
                skip_entry = true;
                kDebug(7042) << "Read in longnames entry";
            } else if (name.mid(1, 1) == " ") { // Symbol table entry
                kDebug(7042) << "Skipped symbol entry";
                dev->seek( dev->pos() + size );
                skip_entry = true;
            } else { // Longfilename
                kDebug(7042) << "Longfilename #" << name.mid(1, 15).toInt();
                if (! ar_longnames) {
                    kWarning(7042) << "Invalid longfilename reference";
                    delete[] ar_longnames;
                    return false;
                }
                name = &ar_longnames[name.mid(1, 15).toInt()];
                name = name.left(name.indexOf("/"));
            }
        }
        if (skip_entry) continue;

        name = name.trimmed(); // Process filename
        name.replace( '/', QByteArray() );
        kDebug(7042) << "Filename: " << name << " Size: " << size;

        KArchiveEntry* entry;
        entry = new KArchiveFile(this, name, mode, date, /*uid*/ 0, /*gid*/ 0, 0, dev->pos(), size);
        rootDir()->addEntry(entry); // Ar files don't support directories, so everything in root

        dev->seek( dev->pos() + size ); // Skip contents
    }
    delete[] ar_longnames;

    return true;
}

bool KAr::closeArchive()
{
    // Close the archive
    return true;
}

void KAr::virtual_hook( int id, void* data )
{ KArchive::virtual_hook( id, data ); }
