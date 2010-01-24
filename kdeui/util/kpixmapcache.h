/*
 *
 * This file is part of the KDE project.
 * Copyright (C) 2007 Rivo Laks <rivolaks@hot.ee>
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

#ifndef KPIXMAPCACHE_H
#define KPIXMAPCACHE_H

#include <kdeui_export.h>

#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtCore/QSize>

class QString;
class QPixmap;


/**
 * @brief General-purpose pixmap cache for KDE.
 *
 * The pixmap cache can be used to store pixmaps which can later be loaded
 *  from the cache very quickly.
 *
 * Its most common use is storing SVG images which might be expensive to
 *  render every time they are used. With the cache you can render each SVG
 *  only once and later use the stored version unless the SVG file or requested
 *  pixmap size changes.
 *
 * KPixmapCache's API is similar to that of the QPixmapCache so if you're
 *  already using the latter then all you need to do is creating a KPixmapCache
 *  object (unlike QPixmapCache, KPixmapCache doesn't have many static methods)
 *  and calling @ref insert() and @ref find() method on that object:
 *  @code
 *  // Create KPixmapCache object
 *  KPixmapCache* cache = new KPixmapCache("myapp-pixmaps");
 *  // Load a pixmap
 *  QPixmap pix;
 *  if (!cache->find("pixmap-1", pix)) {
 *      // Pixmap isn't in the cache, create it and insert to cache
 *      pix = createPixmapFromData();
 *      cache->insert("pixmap-1", pix);
 *  }
 *  // Use pix
 *  @endcode
 *
 * The above example illustrates that you can also cache pixmaps created from
 *  some data. In case such data is updated, you might need to discard cache
 *  contents using @ref discard() method:
 * @code
 * // Discard the cache if it's too old
 * if (cache->timestamp() < mydataTimestamp()) {
 *     cache->discard();
 * }
 * // Now the cache contains up-to-date data
 * @endcode
 * As demonstrated, you can use cache's @ref timestamp() method to see when
 *  the cache was created. If necessary, you can also change the timestamp
 *  using @ref setTimestamp() method.
 */
class KDEUI_EXPORT KPixmapCache
{
public:
    /**
     * Constucts the pixmap cache object.
     * @param name unique name of the cache
     **/
    explicit KPixmapCache(const QString& name);
    virtual ~KPixmapCache();

    /**
     * Tries to load pixmap with the specified key from cache.
     * @return true when pixmap was found and loaded from cache, false otherwise
     **/
    virtual bool find(const QString& key, QPixmap& pix);
    /**
     * Insert specified pixmap into the cache.
     * If the cache already contains pixmap with the specified key then it is
     *  overwritten.
     **/
    virtual void insert(const QString& key, const QPixmap& pix);

    /**
     * Loads pixmap from given file, using the cache.
     * If file's modified-time is more recent than cache's @ref timestamp() ,
     *  then the cache is discarded.
     **/
    QPixmap loadFromFile(const QString& filename);
    /**
     * Same as above, but uses SVG file instead.
     * @param size size of the pixmap where the SVG is render to. If not given
     *  then SVG's default size is used.
     **/
    QPixmap loadFromSvg(const QString& filename, const QSize& size = QSize());

    /**
     * @return timestamp of the cache, set using the @p setTimestamp method.
     * It can be used by the application to check whether app-specific cache
     *  has outdated.
     **/
    unsigned int timestamp() const;
    /**
     * Sets the timestamp of app-specific cache. It's saved in the cache file
     *  and can later be retrieved using @p timestamp() method.
     * By default the timestamp is set to the cache creation time.
     **/
    void setTimestamp(unsigned int time);
    /**
     * Sets whether QPixmapCache (memory caching) should be used in addition
     * to disk cache.
     * QPixmapCache is used by default.
     **/
    void setUseQPixmapCache(bool use);

    /**
     * Whether QPixmapCache should be used to cache pixmaps in memory in
     * addition to caching them on the disk.
     *
     * @b NOTE: The design of QPixmapCache means that the entries stored in
     * the cache are shared throughout the entire process, and not just in
     * this particular KPixmapCache. KPixmapCache makes an effort to ensure
     * that entries from other KPixmapCaches do not inadvertently spill over
     * into this one, but is not entirely successful (@see discard())
     **/
    bool useQPixmapCache() const;

    /**
     * @return approximate size of the cache, in kilobytes.
     **/
    int size() const;
    /**
     * @return maximum size of the cache (in kilobytes).
     * Default setting is 3 megabytes.
     **/
    int cacheLimit() const;
    /**
     * Sets the maximum size of the cache (in kilobytes). If cache gets bigger
     * the limit then some entries are removed (according to
     *  @ref removeEntryStrategy() ).
     * Setting cache limit to 0 disables automatic cache size limiting.
     *
     * Note that the cleanup might not be done immediately, so the cache might
     *  temporarily (for a few seconds) grow bigger than the limit.
     **/
    void setCacheLimit(int kbytes);
    /**
     * Describes which entries will be removed first during cache cleanup.
     * @li RemoveOldest oldest entries are removed first.
     * @li RemoveSeldomUsed least used entries are removed first.
     * @li RemoveLeastRecentlyUsed least recently used entries are removed first.
     **/
    enum RemoveStrategy { RemoveOldest, RemoveSeldomUsed, RemoveLeastRecentlyUsed };
    /**
     * @return current entry removal strategy.
     * Default is RemoveLeastRecentlyUsed.
     **/
    RemoveStrategy removeEntryStrategy() const;
    /**
     * Sets the @ref removeEntryStrategy used when removing entries.
     **/
    void setRemoveEntryStrategy(RemoveStrategy strategy);

    /**
     * @return true when the cache is enabled.
     * Cache will be disabled when e.g. its data file cannot be created or
     *  read.
     **/
    bool isEnabled() const;
    /**
     * @return true when the cache is ready to be used.
     * False usually means that some additional initing has to be done before
     *  the cache can be used.
     **/
    bool isValid() const;
    /**
     * Deletes a pixmap cache.
     * @param name unique name of the cache to be deleted
     **/
    static void deleteCache(const QString& name);

    /**
     * Deletes all entries and reinitializes this cache.
     *
     * @b NOTE: If useQPixmapCache is set to true then that cache must also
     * be cleared. There is only one QPixmapCache for the entire process
     * however so other KPixmapCaches and other QPixmapCache users may also
     * be affected, leading to a temporary slowdown until the QPixmapCache is
     * repopulated.
     */
    void discard();

    /**
     * Removes some of the entries in the cache according to current
     *  @ref removeEntryStrategy().
     * @param newsize wanted size of the cache, in bytes. If 0 is given then
     *  current @ref cacheLimit() is used.
     *
     * Warning: this works by copying some entries to a new cache and then
     *  replacing the old cache with the new one. Thus it might be slow and
     *  will temporarily use extra disk space.
     **/
    void removeEntries(int newsize = 0);

protected:
    /**
     * Makes sure that the cache is initialized correctly.
     */
    void ensureInited() const;

    /**
     * Can be used by subclasses to write custom data into the stream.
     **/
    virtual bool loadCustomData(QDataStream& stream);

    /**
     * Can be used by subclasses to load custom data from the stream.
     **/
    virtual bool writeCustomData(QDataStream& stream);

    /**
     * Can be used by subclasses to write custom data into cache's header.
     **/
    virtual bool loadCustomIndexHeader(QDataStream& stream);

    /**
     * Can be used by subclasses to load custom data from cache's header.
     **/
    virtual void writeCustomIndexHeader(QDataStream& stream);

    /**
     * Can be used by subclasses to indicate that cache needs some additional
     *  initing before it can be used.
     **/
    void setValid(bool valid);

    /**
     * Recreates the cache files.
     */
    bool recreateCacheFiles();

private:
    class Private;
    friend class Private;
    Private * const d;
};

#endif // KPIXMAPCACHE_H
