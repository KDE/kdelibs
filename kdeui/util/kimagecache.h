/*
 * This file is part of the KDE project.
 * Copyright © 2010 Michael Pyne <mpyne@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIMAGECACHE_H
#define KIMAGECACHE_H

#include <kdeui_export.h>
#include <kshareddatacache.h>

#include <time.h>

class QImage;
class QPixmap;

/**
 * @brief A simple wrapping layer over KSharedDataCache to support caching
 * images and pixmaps.
 *
 * This class can be used to share images between different processes, which
 * is useful when it is known that such images will be used across many
 * processes, or when creating the image is expensive.
 *
 * In addition, the class also supports caching QPixmaps <em>in a single
 * process</em> using the setPixmapCaching() function.
 *
 * Tips for use: If you already have QPixmaps that you intend to use, and
 * you do not need access to the actual image data, then try to store and
 * retrieve QPixmaps for use.
 *
 * On the other hand, if you will need to store and retrieve actual image
 * data (to modify the image after retrieval for instance) then you should
 * use QImage to save the conversion cost from QPixmap to QImage.
 *
 * KImageCache is a subclass of KSharedDataCache, so all of the methods that
 * can be used with KSharedDataCache can be used with KImageCache,
 * <em>with the exception of KSharedDataCache::insert() and
 * KSharedDataCache::find()</em>.
 *
 * @author Michael Pyne <mpyne@kde.org>
 * @since 4.5
 */
class KDEUI_EXPORT KImageCache : public KSharedDataCache
{
    public:
    /**
     * Constructs an image cache, named by @p cacheName, with a default
     * size of @p defaultCacheSize.
     *
     * @param cacheName Name of the cache to use.
     * @param defaultCacheSize The default size, in bytes, of the cache.
     *  The actual on-disk size will be slightly larger. If the cache already
     *  exists, it will not be resized. If it is required to resize the
     *  cache then use the deleteCache() function to remove that cache first.
     * @param expectedItemSize The expected general size of the items to be
     *  added to the image cache, in bytes. Use 0 if you just want a default
     *  item size.
     */
    KImageCache(const QString &cacheName,
                unsigned defaultCacheSize,
                unsigned expectedItemSize = 0);

    /**
     * Inserts the pixmap given by @p pixmap to the cache, accessible with
     * @p key. The pixmap must be converted to a QImage in order to be stored
     * into shared memory. In order to prevent unnecessary conversions from
     * taking place @p pixmap will also be cached (but not in stored
     * memory) and would be accessible using findPixmap() if pixmap caching is
     * enabled.
     *
     * @param key Name to access @p pixmap with.
     * @param pixmap The pixmap to add to the cache.
     * @return true if the pixmap was successfully cached, false otherwise.
     * @see setPixmapCaching()
     */
    bool insertPixmap(const QString &key, const QPixmap &pixmap);

    /**
     * Inserts the @p image into the shared cache, accessible with @p key. This
     * variant is preferred over insertPixmap() if your source data is already a
     * QImage, if it is essential that the image be in shared memory (such as
     * for SVG icons which have a high render time), or if it will need to be
     * in QImage form after it is retrieved from the cache.
     *
     * @param key Name to access @p image with.
     * @param image The image to add to the shared cache.
     * @return true if the image was successfully cached, false otherwise.
     */
    bool insertImage(const QString &key, const QImage &image);

    /**
     * Copies the cached pixmap identified by @p key to @p destination. If no such
     * pixmap exists @p destination is unchanged.
     *
     * @return true if the pixmap identified by @p key existed, false otherwise.
     * @see setPixmapCaching()
     */
    bool findPixmap(const QString &key, QPixmap *destination) const;

    /**
     * Copies the cached image identified by @p key to @p destination. If no such
     * image exists @p destination is unchanged.
     *
     * @return true if the image identified by @p key existed, false otherwise.
     */
    bool findImage(const QString &key, QImage *destination) const;

    /**
     * Removes all entries from the cache. In addition any cached pixmaps (as per
     * setPixmapCaching()) are also removed.
     */
    void clear();

    /**
     * @return The time that an image or pixmap was last inserted into a cache.
     */
    time_t lastModifiedTime() const;

    /**
     * @return if QPixmaps added with insertPixmap() will be stored in a local
     * pixmap cache as well as the shared image cache. The default is to cache
     * pixmaps locally.
     */
    bool pixmapCaching() const;

    /**
     * Enables or disables local pixmap caching. If it is anticipated that a pixmap
     * will be frequently needed then this can actually save memory overall since the
     * X server or graphics card will not have to store duplicate copies of the same
     * image.
     *
     * @param enable Enables pixmap caching if true, disables otherwise.
     */
    void setPixmapCaching(bool enable);

    private:
    class Private;
    Private *const d; ///< @internal
};

#endif /* KIMAGECACHE_H */
