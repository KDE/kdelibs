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
#ifndef __kar_h
#define __kar_h

#include <sys/stat.h>
#include <sys/types.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdict.h>

#include <karchive.h>

/**
 * @short A class for reading ar archives.
 * @author Laurence Anderson <l.d.anderson@warwick.ac.uk>
 * @since 3.1
 */
class KAr : public KArchive
{
public:
    /**
     * Creates an instance that operates on the given filename.
     *
     * @param filename is a local path (e.g. "/home/holger/myfile.ar")
     */
    KAr( const QString& filename );

    /**
     * Creates an instance that operates on the given device.
     * The device can be compressed (KFilterDev) or not (QFile, etc.).
     */
    KAr( QIODevice * dev );

    /**
     * If the ar file is still opened, then it will be
     * closed automatically by the destructor.
     */
    virtual ~KAr();

    /**
     * The name of the ar file, as passed to the constructor
     * Null if you used the QIODevice constructor.
     */
    QString fileName() { return m_filename; }

    /*
     * Writing not supported by this class
     */
    virtual bool prepareWriting( const QString&, const QString&, const QString&, uint ) { return false; }
    virtual bool doneWriting( uint ) { return false; }
    virtual bool writeDir( const QString&, const QString&, const QString& )  { return false; }

protected:
    /**
     * Opens the archive for reading.
     * Parses the directory listing of the archive
     * and creates the KArchiveDirectory/KArchiveFile entries.
     *
     */
    virtual bool openArchive( int mode );
    virtual bool closeArchive();

protected:
    virtual void virtual_hook( int id, void* data );
private:
    QString m_filename;
    class KArPrivate;
    KArPrivate * d;
};

#endif
