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

#include <kfileitem.h>
#include <kio/job.h>

class QPixmap;

namespace KIO {
    class PreviewJob : public KIO::Job
    {
        Q_OBJECT
    public:
        PreviewJob( const KFileItemList &items, int width, int height,
            int iconSize, int iconAlpha, bool scale, bool save,
            const QStringList *enabledPlugins, bool deleteItems = false );
        virtual ~PreviewJob();

        /**
         * Removes an item from preview processing. Use this if you passed
         * an item to @ref filePreview and want to delete it now.
         *
         * @param item the item that should be removed from the preview queue
         */
        void removeItem( const KFileItem *item );

        /**
         * @return a list of all available preview plugins. The list
         * contains the basenames of the plugins' .desktop files (no path,
         * no .desktop).
         */
        static QStringList availablePlugins();

        /**
         * @return a list of all supported MIME types. The list can
         * contain entries like text/ * (without the space).
         */
        static QStringList supportedMimeTypes();

    signals:
        /**
         * Emitted when a thumbnail picture for @p item has been successfully
         * retrieved.
         */
        void gotPreview( const KFileItem *item, const QPixmap &preview );
        /**
         * Emitted when a thumbnail for @p item could not be created,
         * either because a ThumbCreator for its MIME type does not
         * exist, or because something went wrong.
         */
        void failed( const KFileItem *item );

    protected:
        void getOrCreateThumbnail();
        bool statResultThumbnail();
        void createThumbnail( QString );

    protected slots:
        virtual void slotResult( KIO::Job *job );

    private slots:
        void startPreview();
        void slotThumbData(KIO::Job *, const QByteArray &);

    private:
        void determineNextFile();
        void emitPreview(const QPixmap &pix);
        void emitFailed(const KFileItem *item = 0);
        void saveThumbnail(const QByteArray &imgData);

    protected:
	virtual void virtual_hook( int id, void* data );
    private:
        struct PreviewJobPrivate *d;
    };

    /**
     * Generates or retrieves a preview image for the given URL.
     *
     * @param items files to get previews for
     * @param width the maximum width to use
     * @param height the maximum height to use, if this is 0, the same
     * value as width is used.
     * @param iconSize the size of the mimetype icon to overlay over the
     * preview or zero to not overlay an icon. This has no effect if the
     * preview plugin that will be used doesn't use icon overlays.
     * @param iconAlpha transparency to use for the icon overlay
     * @param scale if the image is to be scaled to the requested size or
     * returned in its original size
     * @param save if the image should be cached for later use
     * @param enabledPlugins if non-zero, this points to a list containing
     * the names of the plugins that may be used.
     * see @ref availablePlugins.
     */
    PreviewJob *filePreview( const KFileItemList &items, int width, int height = 0, int iconSize = 0, int iconAlpha = 70, bool scale = true, bool save = true, const QStringList *enabledPlugins = 0 );

    /**
     * Same as above, but takes a URL list instead of KFileItemList
     */
    PreviewJob *filePreview( const KURL::List &items, int width, int height = 0, int iconSize = 0, int iconAlpha = 70, bool scale = true, bool save = true, const QStringList *enabledPlugins = 0 );
};

#endif
