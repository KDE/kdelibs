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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KIO_PREVIEWJOB_H
#define KIO_PREVIEWJOB_H

#include <kfileitem.h>
#include <kio/job.h>

class QPixmap;

namespace KIO {
    class PreviewJobPrivate;
    /*!
     * This class catches a preview (thumbnail) for files.
     * @short KIO Job to get a thumbnail picture
     */
    class KIO_EXPORT PreviewJob : public KIO::Job
    {
        Q_OBJECT
    public:
	/**
	 * Creates a new PreviewJob.
	 * @param items a list of files to create previews for
	 * @param width the desired width
	 * @param height the desired height, 0 to use the @p width
	 * @param iconSize the size of the mimetype icon to overlay over the
	 * preview or zero to not overlay an icon. This has no effect if the
	 * preview plugin that will be used doesn't use icon overlays.
	 * @param iconAlpha transparency to use for the icon overlay
	 * @param scale if the image is to be scaled to the requested size or
	 * returned in its original size
	 * @param save if the image should be cached for later use
	 * @param enabledPlugins if non-zero, this points to a list containing
	 * the names of the plugins that may be used.
	 */
        PreviewJob( const KFileItemList& items, int width, int height,
            int iconSize, int iconAlpha, bool scale, bool save,
            const QStringList *enabledPlugins );
        virtual ~PreviewJob();

        /**
         * Removes an item from preview processing. Use this if you passed
         * an item to filePreview and want to delete it now.
         *
         * @param url the url of the item that should be removed from the preview queue
         */
        void removeItem( const KUrl& url );

        /**
         * If @p ignoreSize is true, then the preview is always
         * generated regardless of the settings
         **/
        void setIgnoreMaximumSize(bool ignoreSize = true);

        /**
         * Sets the sequence index given to the thumb creators.
         * Use the sequence index, it is possible to create alternative
         * icons for the same item. For example it may allow iterating through
         * the items of a directory, or the frames of a video.
         *
         * @since KDE 4.3
         **/
        void setSequenceIndex(int index);

        /**
         * Returns the currently set sequence index
         *
         * @since KDE 4.3
         **/
        int sequenceIndex() const;
        
        /**
         * Returns a list of all available preview plugins. The list
         * contains the basenames of the plugins' .desktop files (no path,
         * no .desktop).
	 * @return the list of plugins
         */
        static QStringList availablePlugins();

        /**
         * Returns a list of all supported MIME types. The list can
         * contain entries like text/ * (without the space).
	 * @return the list of mime types
         */
        static QStringList supportedMimeTypes();

        /**
         * Returns the default "maximum file size", in bytes, used by PreviewJob.
         * This is useful for applications providing a GUI for letting the user change the size.
         * @since 4.1
         */
        static KIO::filesize_t maximumFileSize();


    Q_SIGNALS:
        /**
         * Emitted when a thumbnail picture for @p item has been successfully
         * retrieved.
	 * @param item the file of the preview
	 * @param preview the preview image
         */
        void gotPreview( const KFileItem& item, const QPixmap &preview );
        /**
         * Emitted when a thumbnail for @p item could not be created,
         * either because a ThumbCreator for its MIME type does not
         * exist, or because something went wrong.
	 * @param item the file that failed
         */
        void failed( const KFileItem& item );

    protected Q_SLOTS:
        virtual void slotResult( KJob *job );

    private:
        Q_PRIVATE_SLOT(d_func(), void startPreview())
        Q_PRIVATE_SLOT(d_func(), void slotThumbData(KIO::Job *, const QByteArray &))
        Q_DECLARE_PRIVATE(PreviewJob)
    };

    /**
     * Creates a PreviewJob to generate or retrieve a preview image
     * for the given URL.
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
     * @return the new PreviewJob
     * @see PreviewJob::availablePlugins()
     */
    KIO_EXPORT PreviewJob *filePreview( const KFileItemList &items, int width, int height = 0, int iconSize = 0, int iconAlpha = 70, bool scale = true, bool save = true, const QStringList *enabledPlugins = 0 );

    /**
     * Creates a PreviewJob to generate or retrieve a preview image
     * for the given URL.
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
     * @return the new PreviewJob
     * @see PreviewJob::availablePlugins()
     */
    KIO_EXPORT PreviewJob *filePreview( const KUrl::List &items, int width, int height = 0, int iconSize = 0, int iconAlpha = 70, bool scale = true, bool save = true, const QStringList *enabledPlugins = 0 );
}

#endif
