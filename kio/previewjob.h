// -*- c++ -*-
// vim: ts=4 sw=4 et
/*  This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>
                  2000 Carsten Pfeiffer <pfeiffer@kde.org>
                  2001 Malte Starostik <malte.starostik@t-online.de>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __kio_previewjob_h__
#define __kio_previewjob_h__

#include <kio/job.h>

class QPixmap;

namespace KIO {
    class PreviewJob : public KIO::Job
    {
        Q_OBJECT
    public:
        PreviewJob( const KURL::List &items, int width, int height, int iconSize, int iconAlpha );
        virtual ~PreviewJob();

    signals:
        void gotPreview( const KURL &url, const QPixmap &preview );

    protected:
        void getOrCreateThumbnail();
        bool statResultThumbnail( KIO::StatJob *job = 0 );
        void createThumbnail( QString );

    protected slots:
        virtual void slotResult( KIO::Job *job );

    private slots:
        void determineNextFile();
        void slotThumbData(KIO::Job *, const QByteArray &);

    private:
        void saveThumbnail(const QByteArray &imgData);

    private:
        struct PreviewJobPrivate *d;
    };

    /**
     * Generate or retrieve a preview image for the given URL.
     *
     * @param items files to get previews for
     * @param width the maximum width to use
     * @param height the maximum height to use, if this is 0, the same
     * value as width is used.
     * @param iconSize the size of the mimetype icon to overlay over the
     * preview or zero to not overlay an icon. This has no effect if the
     * preview plugin that will be used doesn't use icon overlays.
     * @param iconAlpha transparency to use for the icon overlay
     */
    PreviewJob *filePreview( const KURL::List &items, int width, int height = 0, int iconSize = 0, int iconAlpha = 70 );
};

#endif
