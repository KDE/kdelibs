/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *                2000, 2007 David Faure   <faure@kde.org>
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
 **/

#ifndef KDEDESKTOPMIMETYPE_H
#define KDEDESKTOPMIMETYPE_H

#include "kmimetype.h"

class KDesktopFile;

/**
 * Mime type for desktop files.
 * Handles mount/umount icon, trash icon, reading the comment from the desktop file.
 * @short Mimetype for a .desktop file
 */
class KIO_EXPORT KDEDesktopMimeType : public KMimeType
{
    K_SYCOCATYPE( KST_KDEDesktopMimeType, KMimeType )

public:
    /** \internal */
    KDEDesktopMimeType( const QString & fullpath, const QString& type,
                        const QString& comment )
        : KMimeType(fullpath, type, comment ) {}
    /** \internal */
    KDEDesktopMimeType( QDataStream& str, int offset ) : KMimeType( str, offset ) { }

    virtual QString icon( const KUrl& url ) const;
    virtual QString comment( const KUrl& url ) const;

protected:
    virtual void virtual_hook( int id, void* data );
};

#endif
