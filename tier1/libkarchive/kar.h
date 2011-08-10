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
#ifndef KAR_H
#define KAR_H

#include <karchive.h>

/**
 * KAr is a class for reading archives in ar format. Writing
 * is not supported.
 * @short A class for reading ar archives.
 * @author Laurence Anderson <l.d.anderson@warwick.ac.uk>
 */
class KARCHIVE_EXPORT KAr : public KArchive
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
     * @param dev the device to read from
     */
    KAr( QIODevice * dev );

    /**
     * If the ar file is still opened, then it will be
     * closed automatically by the destructor.
     */
    virtual ~KAr();

protected:

    /*
     * Writing not supported by this class, will always fail.
     * @return always false
     */
    virtual bool doPrepareWriting( const QString& name, const QString& user, const QString& group, qint64 size,
                                 mode_t perm, time_t atime, time_t mtime, time_t ctime );

    /*
     * Writing not supported by this class, will always fail.
     * @return always false
     */
    virtual bool doFinishWriting( qint64 size );

    /*
     * Writing not supported by this class, will always fail.
     * @return always false
     */
    virtual bool doWriteDir( const QString& name, const QString& user, const QString& group,
                             mode_t perm, time_t atime, time_t mtime, time_t ctime );

    virtual bool doWriteSymLink( const QString &name, const QString &target,
                                 const QString &user, const QString &group, mode_t perm, time_t atime, time_t mtime, time_t ctime );

    /**
     * Opens the archive for reading.
     * Parses the directory listing of the archive
     * and creates the KArchiveDirectory/KArchiveFile entries.
     *
     */
    virtual bool openArchive( QIODevice::OpenMode mode );
    virtual bool closeArchive();

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KArPrivate;
    KArPrivate* const d;
};

#endif
