/*
 * Copyright (C)  2001, 2006 Holger Freyther <freyther@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "kio/renamedialogplugin.h"

using namespace KIO;

class RenameDialogPlugin::FileItem::FileItemPrivate
{
public:
    FileItemPrivate( const KUrl& url, const QString& mimeType,
                     const KIO::filesize_t size, time_t ctime,
                     time_t mtime )
        : m_url( url )
        , m_mimeType( mimeType )
        , m_fileSize( size )
        , m_ctime( ctime )
        , m_mtime( mtime )
    {}
    KUrl m_url;
    QString m_mimeType;
    KIO::filesize_t m_fileSize;
    time_t m_ctime;
    time_t m_mtime;
};

/**
 * @param url     The location of the item
 * @param mimeType The actual mimetype of the item
 * @param size    The size of this item
 * @param ctime   The changed time as of stat(2)
 * @param mtime   The modified time as of stat(2)
 */
RenameDialogPlugin::FileItem::FileItem( const KUrl& url, const QString& mimeType,
                                     KIO::filesize_t size, time_t ctime,
                                     time_t mtime )
    : d( new FileItemPrivate(url, mimeType, size, ctime, mtime) )
{}

RenameDialogPlugin::FileItem::~FileItem()
{
    delete d;
}

KUrl RenameDialogPlugin::FileItem::url() const {
    return d->m_url;
}

QString RenameDialogPlugin::FileItem::mimeType() const {
    return d->m_mimeType;
}

KIO::filesize_t RenameDialogPlugin::FileItem::fileSize() const {
    return d->m_fileSize;
}

time_t RenameDialogPlugin::FileItem::cTime() const {
    return d->m_ctime;
}

time_t RenameDialogPlugin::FileItem::mTime() const {
    return d->m_mtime;
}


RenameDialogPlugin::RenameDialogPlugin( QDialog* dlg)
    : QWidget( dlg )
{}

#include "renamedialogplugin.moc"
