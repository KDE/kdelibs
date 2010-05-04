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
 *
 * @deprecated KPixmapCache is susceptible to various non-trivial locking bugs and
 * inefficiencies, and is supported for backward compatibility only (since it exposes
 * a QDataStream API for subclasses). Users should port to KImageCache for a very close
 * work-alike, or KSharedDataCache if they need more control.
 *
 * @see KImageCache, KSharedDataCache
 *
 * @author Rivo Laks
 */
class KDEUI_EXPORT KPixmapCache
{
public:
    /**
     * Constucts the pixmap cache object.
     * @param name unique name of the cache
     */
    explicit KPixmapCache(const QString& name);
    virtual ~KPixmapCache();

    /**
     * Tries to load pixmap with the specified @a key from cache. If the pixmap
     * is found it is stored in @a pix, otherwise @a pix is unchanged.
     *
     * @return true when pixmap was found and loaded from cache, false otherwise
     */
    virtual bool find(const QString& key, QPixmap& pix);

    /**
     * Inserts the pixmap @a pix into the cache, associated with the key @a key.
     *
     * Any existing pixmaps associated with @a key are overwritten.
     */
    virtual void insert(const QString& key, const QPixmap& pix);

    /**
     * Loads a pixmap from given file, using the cache. If the file does not
     * exist on disk, an empty pixmap is returned, even if that file had
     * previously been cached.  In addition, if the file's modified-time is
     * more recent than cache's @ref timestamp(), the @em entire cache is
     * discarded (to be regenerated). This behavior may change in a future
     * KDE Platform release. If the cached data is current the pixmap
     * is returned directly from the cache without any file loading.
     *
     * @note The mapping between @a filename and the actual key used internally
     * is implementation-dependent and can change without warning. Use insert()
     * manually if you need control of the key, otherwise consistently use this
     * function.
     *
     * @param filename The name of the pixmap to load, cache, and return.
     * @return The given pixmap, or an empty pixmap if the file was invalid or did
     *         not exist.
     */
    QPixmap loadFromFile(const QString& filename);

    /**
     * Same as loadFromFile(), but using an SVG file instead. You may optionally
     * pass in a @a size to control the size of the output pixmap.
     *
     * @note The returned pixmap is only cached for identical filenames and sizes.
     *       If you change the size in between calls to this function then the
     *       pixmap will have to be regenerated again.
     *
     * @param filename The filename of the SVG file to load.
     * @param size size of the pixmap where the SVG is render to. If not given
     *        then the SVG file's default size is used.
     * @return an empty pixmap if the file does not exist or was invalid, otherwise
     *         a pixmap of the desired @a size.
     */
    QPixmap loadFromSvg(const QString& filename, const QSize& size = QSize());

    /**
     * @note KPixmapCache does not ever change the timestamp, so the application
     * must set the timestamp if it to be used.
     * @return Timestamp of the cache, set using the setTimestamp() method.
     */
    unsigned int timestamp() const;

    /**
     * Sets the timestamp of app-specific cache. It's saved in the cache file
     * and can later be retrieved using the timestamp() method.
     * By default the timestamp is set to the cache creation time.
     */
    void setTimestamp(unsigned int time);

    /**
     * Sets whether QPixmapCache (memory caching) should be used in addition
     * to disk cache. QPixmapCache is used by default.
     *
     * @note On most systems KPixmapCache can use shared-memory to share cached
     * pixmaps with other applications attached to the same shared pixmap,
     * which means additional memory caching is unnecessary and actually
     * wasteful of memory.
     *
     * @warning QPixmapCache is shared among the entire process and therefore
     * can cause strange interactions with other instances of KPixmapCache.
     * This may be fixed in the future and should be not relied upon.
     */
    // KDE5 Get rid of QPixmapCache and use a sane cache instead.
    void setUseQPixmapCache(bool use);

    /**
     * Whether QPixmapCache should be used to cache pixmaps in memory in
     * addition to caching them on the disk.
     *
     * @b NOTE: The design of QPixmapCache means that the entries stored in
     * the cache are shared throughout the entire process, and not just in
     * this particular KPixmapCache. KPixmapCache makes an effort to ensure
     * that entries from other KPixmapCaches do not inadvertently spill over
     * into this one, but is not entirely successful (see discard())
     */
    bool useQPixmapCache() const;

    /**
     * @return approximate size of the cache, in kilobytes (1 kilobyte == 1024 bytes)
     */
    int size() const;

    /**
     * @return maximum size of the cache (in kilobytes).
     * Default setting is 3 megabytes (1 megabyte = 2^20 bytes).
     */
    int cacheLimit() const;

    /**
     * Sets the maximum size of the cache (in kilobytes). If cache gets bigger
     * than the limit then some entries are removed (according to
     * removeEntryStrategy()).
     *
     * Setting the cache limit to 0 disables caching (as all entries will get
     * immediately removed).
     *
     * Note that the cleanup might not be done immediately, so the cache might
     * temporarily (for a few seconds) grow bigger than the limit.
     */
    void setCacheLimit(int kbytes);

    /**
     * Describes which entries will be removed first during cache cleanup.
     * @see removeEntryStrategy(), @see setRemoveEntryStrategy()
     */
    enum RemoveStrategy { /// oldest entries are removed first.
                          RemoveOldest,
                          /// least used entries are removed first.
                          RemoveSeldomUsed,
                          /// least recently used entries are removed first.
                          RemoveLeastRecentlyUsed
                        };
    /**
     * @return current entry removal strategy.
     * Default is RemoveLeastRecentlyUsed.
     */
    RemoveStrategy removeEntryStrategy() const;

    /**
     * Sets the removeEntryStrategy used when removing entries.
     */
    void setRemoveEntryStrategy(RemoveStrategy strategy);

    /**
     * Cache will be disabled when e.g. its data file cannot be created or
     * read.
     *
     * @return true when the cache is enabled.
     */
    bool isEnabled() const;

    /**
     * @return true when the cache is ready to be used. Not being valid usually
     * means that some additional initialization has to be done before the
     * cache can be used.
     */
    bool isValid() const;

    /**
     * Deletes a pixmap cache.
     * @param name unique name of the cache to be deleted
     */
    // KDE5: Static function oh how I hate you, this makes it very difficult to perform
    // appropriate locking and synchronization to actually remove the cache.
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
     * removeEntryStrategy().
     *
     * @param newsize wanted size of the cache, in bytes. If 0 is given then
     * current cacheLimit() is used.
     *
     * @warning This currently works by copying some entries to a new cache and
     * then replacing the old cache with the new one. Thus it might be slow and
     * will temporarily use extra disk space.
     */
    void removeEntries(int newsize = 0);

protected:
    /**
     * Makes sure that the cache is initialized correctly, including the loading of the
     * cache index and data, and any shared memory attachments (for systems where that
     * is enabled).
     *
     * @note Although this method is protected you should not use it from any subclasses.
     *
     * @internal
     */
    // KDE5: rename to ensureInitialized()
    // KDE5: Make private or move to Private
    void ensureInited() const;

    /**
     * Can be used by subclasses to load custom data from the stream.
     * This function will be called by KPixmapCache immediately following the
     * image data for a single image being read from @a stream.
     * (This function is called once for every single image).
     *
     * @see writeCustomData
     * @see loadCustomIndexHeader
     * @param stream the QDataStream to read data from
     * @return true if custom data was successfully loaded, false otherwise. If
     *         false is returned then the cached item is assumed to be invalid and
     *         will not be available to find() or contains().
     */
    virtual bool loadCustomData(QDataStream& stream);

    /**
     * Can be used by subclasses to write custom data into the stream.
     * This function will be called by KPixmapCache immediately after the
     * image data for a single image has been written to @a stream.
     * (This function is called once for every single image).
     *
     * @see loadCustomData
     * @see writeCustomIndexHeader
     * @param stream the QDataStream to write data to
     */
    virtual bool writeCustomData(QDataStream& stream);

    /**
     * Can be used by subclasses to load custom data from cache's header.
     * This function will be called by KPixmapCache immediately after the
     * index header has been written out. (This function is called one time
     * only for the entire cache).
     *
     * @see loadCustomData
     * @see writeCustomIndexHeader
     * @param stream the QDataStream to read data from
     * @return true if custom index header data was successfully read, false
     *         otherwise. If false is returned then the cache is assumed to
     *         be invalid and further processing does not occur.
     */
    virtual bool loadCustomIndexHeader(QDataStream& stream);

    /**
     * Can be used by subclasses to write custom data into cache's header.
     * This function will be called by KPixmapCache immediately following the
     * index header has being loaded. (This function is called one time
     * only for the entire cache).
     *
     * @see writeCustomData
     * @see loadCustomIndexHeader
     * @param stream the QDataStream to write data to
     */
    virtual void writeCustomIndexHeader(QDataStream& stream);

    /**
     * Sets whether this cache is valid or not. (The cache must be enabled in addition
     * for isValid() to return true. @see isEnabled(), @see setEnabled()).
     *
     * Most cache functions do not work if the cache is not valid. KPixmapCache assumes
     * the cache is valid as long as its cache files were able to be created (see
     * recreateCacheFiles()) even if the cache is not enabled.
     *
     * Can be used by subclasses to indicate that cache needs some additional
     * initialization before it can be used (note that KPixmapCache will @em not handle
     * actually performing this extra initialization).
     */
    void setValid(bool valid);

    /**
     * This function causes the cache files to be recreate by invalidating the cache.
     * Any shared memory mappings (if enabled) are dropped temporarily as well.
     *
     * @note The recreated cache will be initially empty, but with the same size limits
     * and entry removal strategy (see removeEntryStrategy()).
     *
     * If you use this in a subclass be prepared to handle writeCustomData() and
     * writeCustomIndexHeader().
     *
     * @return true if the cache was successfully recreated.
     */
    bool recreateCacheFiles();

private:
    /// @internal
    class Private;
    friend class Private;
    Private * const d; ///< @internal
};

#endif // KPIXMAPCACHE_H
