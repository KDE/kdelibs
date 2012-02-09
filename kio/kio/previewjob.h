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
         * Specifies the type of scaling that is applied to the generated preview.
         * @since 4.7
         */
        enum ScaleType {
            /**
             * The original size of the preview will be returned. Most previews
             * will return a size of 256 x 256 pixels.
             */
            Unscaled,
            /**
             * The preview will be scaled to the size specified when constructing
             * the PreviewJob. The aspect ratio will be kept.
             */
            Scaled,
            /**
             * The preview will be scaled to the size specified when constructing
             * the PreviewJob. The result will be cached for later use. Per default
             * ScaledAndCached is set.
             */
            ScaledAndCached
        };

#ifndef KDE_NO_DEPRECATED
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
         * @param enabledPlugins If non-zero, this points to a list containing
         * the names of the plugins that may be used. If enabledPlugins is zero
         * all available plugins are used.
         *
         * @deprecated Use PreviewJob(KFileItemList, QSize, QStringList) in combination
         *             with the setter-methods instead. Note that the semantics of
         *             \p enabledPlugins has been slightly changed.
         */
        KIO_DEPRECATED PreviewJob(const KFileItemList& items, int width, int height,
                                  int iconSize, int iconAlpha, bool scale, bool save,
                                  const QStringList *enabledPlugins);
#endif

        /**
         * @param items          List of files to create previews for.
         * @param size           Desired size of the preview.
         * @param enabledPlugins If non-zero it defines the list of plugins that
         *                       are considered for generating the preview. If
         *                       enabledPlugins is zero the plugins specified in the
         *                       KConfigGroup "PreviewSettings" are used.
         * @since 4.7
         */
        PreviewJob(const KFileItemList &items,
                   const QSize &size,
                   const QStringList *enabledPlugins = 0);

        virtual ~PreviewJob();

        /**
         * Sets the size of the MIME-type icon which overlays the preview. If zero
         * is passed no overlay will be shown at all. The setting has no effect if
         * the preview plugin that will be used does not use icon overlays. Per
         * default the size is set to 0.
         * @since 4.7
         */
        void setOverlayIconSize(int size);

        /**
         * @return The size of the MIME-type icon which overlays the preview.
         * @see PreviewJob::setOverlayIconSize()
         * @since 4.7
         */
        int overlayIconSize() const;

        /**
         * Sets the alpha-value for the MIME-type icon which overlays the preview.
         * The alpha-value may range from 0 (= fully transparent) to 255 (= opaque).
         * Per default the value is set to 70.
         * @see PreviewJob::setOverlayIconSize()
         * @since 4.7
         */
        void setOverlayIconAlpha(int alpha);

        /**
         * @return The alpha-value for the MIME-type icon which overlays the preview.
         *         Per default 70 is returned.
         * @see PreviewJob::setOverlayIconAlpha()
         * @since 4.7
         */
        int overlayIconAlpha() const;

        /**
         * Sets the scale type for the generated preview. Per default
         * PreviewJob::ScaledAndCached is set.
         * @see PreviewJob::ScaleType
         * @since 4.7
         */
        void setScaleType(ScaleType type);

        /**
         * @return The scale type for the generated preview.
         * @see PreviewJob::ScaleType
         * @since 4.7
         */
        ScaleType scaleType() const;

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
         * @deprecated PreviewJob uses different maximum file sizes dependent on the URL since 4.5.
         *             The returned file size is only valid for local URLs.
         */
#ifndef KDE_NO_DEPRECATED
        KIO_DEPRECATED static KIO::filesize_t maximumFileSize();
#endif


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

#ifndef KDE_NO_DEPRECATED
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
     * @deprecated Use KIO::filePreview(KFileItemList, QSize, QStringList) in combination
     *             with the setter-methods instead. Note that the semantics of
     *             \p enabledPlugins has been slightly changed.
     */
    KIO_DEPRECATED_EXPORT PreviewJob *filePreview( const KFileItemList &items, int width, int height = 0, int iconSize = 0, int iconAlpha = 70, bool scale = true, bool save = true, const QStringList *enabledPlugins = 0 ); // KDE5: use enums instead of bool scale + bool save

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
     * @deprecated Use KIO::filePreview(KFileItemList, QSize, QStringList) in combination
     *             with the setter-methods instead. Note that the semantics of
     *             \p enabledPlugins has been slightly changed.
     */
    KIO_DEPRECATED_EXPORT PreviewJob *filePreview( const KUrl::List &items, int width, int height = 0, int iconSize = 0, int iconAlpha = 70, bool scale = true, bool save = true, const QStringList *enabledPlugins = 0 );
#endif

    /**
     * Creates a PreviewJob to generate a preview image for the given items.
     * @param items          List of files to create previews for.
     * @param size           Desired size of the preview.
     * @param enabledPlugins If non-zero it defines the list of plugins that
     *                       are considered for generating the preview. If
     *                       enabledPlugins is zero the plugins specified in the
     *                       KConfigGroup "PreviewSettings" are used.
     * @since 4.7
     */
    KIO_EXPORT PreviewJob *filePreview(const KFileItemList &items, const QSize &size, const QStringList *enabledPlugins = 0);
}

#endif
