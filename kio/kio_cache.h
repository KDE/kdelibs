#ifndef __kio_cache_h__
#define __kio_cache_h__

#include "kio_job.h"   

#include <qstring.h>
#include <qstrlist.h>
#include <qdict.h>
#include <qlist.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <qbuffer.h>

#include <kconfig.h> 

#define MAX_JOBS 4

class KIOCache;

/**
 * @short A cache entry.
 *
 * KIOCacheEntry encapsulates all data associated with one entry
 * in KIOCache. It is mainly used for communication between @see
 * #KIOCache and @see #KIOCachedJob.
 */
class KIOCacheEntry
{
public:
    /**
     * Constructs a new empty KIOCacheEntry.
     */
    KIOCacheEntry() {}

    /**
     * Constructs a new KIOCacheEntry referencing the same document as
     * 'ref'.
     */
    KIOCacheEntry (const KIOCacheEntry &ref);

    /**
     * Constructs a new KIOCacheEntry referencing _url
     */
    KIOCacheEntry (const QString& _url);

    /**
     * @return TRUE if this cache entry is empty, FALSE if it actually
     * references a document.
     */
    bool isEmpty() const
	{ return docURL.isEmpty(); }

    /**
     * Returns the URL of the document.
     */
    QString url() const
	{ return docURL; }

    /**
     * Returns the local file name for the document, without path.
     */
    QString localFile() const
	{ return localName; }

    QString localKey() const
	{ return localName; }
    /**
     * Sets the local file key, that is the name of the file used to
     * store the documents content. Don't set a complete path, only
     * the filename relativ to the cache path.
     */
    void setLocalKey(QString key)
       { localName = key; }
    
    /**
     * Returns the mime type of the document
     */
    QString mimeType() const
	{ return docMimeType; }
    /**
     * Sets the mime type of the document
     */
     void setMimeType(QString mimeType)
	{ docMimeType = mimeType; }

    /**
     * Returns the redirection, this means the real address of the document.
     */
    QString redirection() const
	{ return redirectionURL; }
    /**
     * Sets the redirection URL.
     */
    void setRedirection( const char* _redirection )
	{ redirectionURL = _redirection; }

    /**
     * Returns the document content or a NULL QByteArray in case something
     * went wrong.
     *
     * Caveat: content() returns a shallow copy of the data only, if
     * you want to modify the data, you need to detach it first!
     */
    QByteArray content() const;

    /**
     * Appends to the document content. Should not be used outside of @see
     * KIOCache and @see KIOCachedJob
     */
    void addData(const QByteArray &_data);
    void addData( const char *_data, int _len );

    /**
     * Returns date and time when this document expires. If an invalid
     * date is returned, this document will never expire.
     */
    QDateTime expiresAt() const 
	{ return expires; }
    /**
     * Set date and time when this document expires. You can pass a
     + null QDateTime to indicate that this document should never expire.
     */
    QDateTime setExpiresAt(const QDateTime &time) 
	{ return expires = time; }

    /**
     * Returns the creation date and time of this document. An invalid
     * QDateTime indicates there is no cretion date available for this
     * document.
     */
    QDateTime createdAt() const
	{ return created; }
    /**
     * Sets the creation date and time of this document. This function
     * should not be used by anything besides @see KIOCache and @see
     * CacheKIOJob.
     */
    QDateTime setCreatedAt(const QDateTime &time) 
	{ return created = time; }

    /**
     * Returns the last date and time this cache entry has been
     * read. Beware: This has nothing to do with the cached documents'
     * last access time, it should be used within KIOCache only.
     * (e.g. for an LRU algorithm to discard cached documents in order
     * to limit on-disk cache size).
     */
    QDateTime lastAccessedAt() const 
	{ return lastAccess; }

    /**
     * Sets the date and time returned as last access time for this
     * cache entry. Shouldn't be used outside of @see KIOCache.
     */
    QDateTime setLastAccessedAt(const QDateTime &time)
	{ return lastAccess = time; }

    /**
     * Returns the date and time this document has been last modified. An
     * invalid QDateTime indicates there is no modification date
     * available for this document.
     */
    QDateTime lastModifiedAt() const
	{ return lastModified; }
    /**
     * Sets the modification date and timefor this document.
     */
    QDateTime setLastModifiedAt(const QDateTime &time)
	{ return lastModified = time; }

    /**
     * Write the content of @ref data to a file and erase data. The
     * filename is KIOCache::cachePath() + localKey(). This function
     * is intended for use by KIOCache only.
     *
     * @return TRUE on success, FALSE on failure.
     */
    bool storeData();

private:
    QDateTime expires;
    QDateTime created;
    QDateTime lastAccess;
    QDateTime lastModified;
    QBuffer data;
    QString docURL;
    QString localName;
    QString docMimeType;
    QString redirectionURL;
};


/**
 * @short Basic functionality for an on-disk cache.
 *
 * KIOCache provides basic functionality to maintain a local on-disk
 * cache for remote documents. KIOCache provides methods to check if a
 * document is in cache and valid (i.e. not yet expired), if a certain
 * URL can be cached at all, to retrieve documents and additional data
 * (such as mime type or expiration date) from cache, to insert
 * documents into the cache and finally to save the cache to and load
 * it from disk and to clear the cache.  Some additional methods cover
 * cache settings like which documents should be cached, how to handle
 * documents without explicit expiration date and so on.
 *
 * Right now, only documents retrieved by the HTTP protocol can be
 * cached, but don't depend on this, this may (and most probably will)
 * change in future.
 *
 * KIOCache should not be used directly to perform cached IO, the
 * recommended way to access documents from the cache is by using @ref
 * KIOCachedJob. This class provides transparent access to cached
 * documents and handles all the tedious details such as checking if a
 * document is in cache and valid (and retrieving it from the source
 * if not) for you.
 */

class KIOCache
{
public:
    static void initStatic();
  
    /**
     * Enables or disables the cache.
     */
    static void enableCache( bool enable )
	{ cacheEnabled = enable; }

    /**
     * Enables or disables saving the cache.
     */
    static void enableSaveCache( bool enable )
	{ saveCacheEnabled = enable; }
	
    /**
     * Check enable status
     */
    static bool isEnabled()
	{ return cacheEnabled; }
    static bool isSaveEnabled()
	{ return saveCacheEnabled; }

    /**
     * Reads the cache configuration from config. Cache config entries
     * are expected in section [Cache].
     */
    static void readConfig(KConfig &config);

    /**
     * Stores the cache configuration in config. Cache config entries
     * are written to section [Cache]. The modified KConfig object is
     * returned.
     */
    static KConfig &storeConfig(KConfig &config);
  
    /**
     * Reads the stored cache content from disk. Actually, only the
     * info on which documents are in cache and where they are stored
     * on disk is read, the actual document content is not read until it
     * is needed.
     *
     * Cache info is stored as a KDE config file.  Every cache entry
     * has its own section [Cacheentry #], where # is the number of
     * the entry, starting with 1. Note that these numbers are only
     * used to create unique section names when writing the cache,
     * they have nothing to do with local file names for the cached
     * document's content. Cache entry numbers must be consecutive for
     * readCache() to work correctly. If there are numbers missing,
     * only entries up to the first missing number will be read back.
     * 
     * return TRUE on success, FALSE if nothing can be read back.  */
    static bool readCache();
    /**
     * Writes the cache content to disk in a form that is understood
     * by @ref #readCache and creates an index.html file in the cache
     * directory reflecting the current cache content.
     *
     * Note that some part of the cache content (namely the cached
     * documents) will be written immediately when a document is
     * entered into cache, but other associated data will be kept in
     * memory. After a successful storeCache() however the complete
     * cache content at the time of calling is guaranteed to be on
     * disk.
     *
     * @return TRUE on success, FALSE if the cache content cannot be
     * stored. 
     */
    static bool storeCache();

    /**
     * @return the full path of the written index file or an empty string
     *         in case of an error.
     */
    static QString storeIndex();
  
    /**
     * Clears the cache.
     */
    static bool clear();

    /**
     * Returns a QString containing an HTML document which describes the
     * current cache contents. Useful for presenting in an HTML widget
     * or for storing as index.html file in the cache directory.
     */
    static QString htmlIndex();

    /**
     * Check if the document referenced by url is in cache and the
     * cached copy is valid, i.e. not yet expired. Use @ref isCached
     * if you want to check for cached documents that may already have
     * expired.
     */
    static bool isValid(const QString &url);

    /**
     * Check if the document referenced by url is in cache. This
     * returns true even if the document is no longer valid. Use 
     * @see isValid if you want to check document validity.
     */
    static bool isCached(const QString &url);

    /**
     * Get the document referenced by url from the cache. If there is
     * no matching document in cache, an empty @ref KIOCacheEntry is
     * returned. Note that a returned document may no longer be valid,
     * use @ref isValid to check for validity.
     */
    static const KIOCacheEntry& lookup(const QString &url);

    /**
     * Put a document into the cache. At least the url field (see 
     * @ref KIOCacheEntry) of entry must be set to a (syntactically)
     * correct url. The document will be stored in cache if its URL is
     * cacheable (see @ref #isCacheable) and if the document has not
     * yet expired (as indicated by entry->expiresAt()).
     *
     * Note: entry must point to a KIOCacheEntry allocated with new
     * and may not be deleted or modified by the caller if insert()
     * returns TRUE. KIOCache stores this pointer in its internal
     * dictionary in this case. If insert() returns FALSE however, the
     * caller is responsible for deleting entry.
     *
     * @returns TRUE if entry is stored in cache, FALSE if not.  */
    static bool insert(KIOCacheEntry *entry);
    
    /**
     * Indicates if the document referenced by url may be stored in
     * cache at all. The test is based on the URL only, document
     * content, expiration date or other information is not
     * used. Hence, a document may be rejected by @ref #insert even if
     * isCacheable returned TRUE, e.g. because it has already expired
     * at the time insert is called for this document.
     *
     * Currently, isCacheable returns TRUE if the protocol part of url
     * is listed in @ref cacheProtocols and the hostname part is not
     * listed in @ref excludeHosts. These settings are user
     * configurable.
     *
     * In future, isCacheable probably will be extended to check other
     * criteria as well.
     *
     * @returns FALSE if the document referenced by URL may not be
     * stored in cache, TRUE otherwise.
     */
    static bool isCacheable(const QString &_url);

    /**
     * Returns the name of the directory where all the cached files
     * reside.
     */
     static QString cacheRoot()
	{ return cachePath; }
  
    /**
     * Trim url, that is remove all parts of url that should not be
     * used in the cache key. Right now, only the reference part is
     * removed, so 'http://www.somewhere.org/something#index_1' and
     * 'http://www.somewhere.org/something#index_2' will both be cached
     * as 'http://www.somewhere.org/something'.
     *
     * @returns the trimmed URL
     */
    static QString trimURL(const QString &url);

protected:
    /**
     * Calculate a default expiration date time for entry based on user
     * settings and the date fields that are set in entry.
     *
     * @returns A default expiration date and time.
     * @see KIOCacheEntry
     */
    static QDateTime defaultExpire(const KIOCacheEntry *entry);
    /**
     * Calculate a local file name for the document in entry. This
     * local file name consists of the prefix 'entry-', the current
     * date and time, a '.', a sequence number, another '.' and the
     * filename part of the documents URL.
     */
    static QString localKey(const KIOCacheEntry *entry);
private:
    /**
     * This dictionary is used to store information about every
     * document currently in cache. 
     */
    static QDict<KIOCacheEntry> cacheDict;

    /**
     * A list of protocols that should be handled by the cache.
     * User configurable through the entry 'Protocols' in the 'Cache'
     * section of the applications config file. Defaults to 'http' and
     * 'cgi' if not set.
     */
    static QStrList cacheProtocols;

    /**
     * A list of host/domain names that should be excluded from
     * caching. The names may contain wildcards, see the KIO proxy
     * exclude list for an example.
     * User configurable through the entry 'ExcludeHosts' in the 'Cache'
     * section of the applications config file. Empty by default.
     */
    static QStrList excludeHosts;

    /**
     * The path where KIOCache stores the cached documents and related
     * information.
     */
    static QString cachePath;

    /**
     * This flag tells wheter the cache is enabled or not. If
     * cacheEnabled is FALSE, @ref #isCacheable and @ref #isValid will
     * always return FALSE. Note however that it still is possible to
     * enter documents with @ref #insert and that @ref #isCached will
     * still return TRUE if a document is cached.
     *
     * This flag defaults to FALSE before the first call to
     * @ref #loadConfig. After that, it has the value given by the
     * 'UseCache' entry in the [Cache] config section, TRUE if no such
     * entry is given.
     */
    static bool cacheEnabled;

    /**
     * This flag tells wheter the cache will save itself to disk.     
     */
    static bool saveCacheEnabled;

    /**
     * Maximum length of URL's to display in full in @ref #index. Any
     * URL in cache which is longer than maxURLLenght will be
     * abbreviated for display.
     */
    static unsigned int maxURLLength;
};

/**
 * @short Transparent access to cached data.
 *
 * KIOCachedJob provides a means to transparently access data that may
 * reside in a local on-disk cache. Transparent means you don't need
 * to worry about the details, just use KIOCachedJob like you would
 * use KIOJob. KIOCachedJob will fetch the requested documents from the
 * cache if appropriate and get them from their original source if not
 * (see @ref #KIOCache for details like caching strategy and the
 * like).
 *
 * KIOCachedJob provides the same interface as KIOJob with one
 * addition: @ref #forceReload can be used to force reloading of a
 * document even if it is in cache and valid.
 *
 * Right now, data is only cached (and searched in cache) when it is
 * requested by calling get(), all other request methods ignore the cache.
 * Do not depend on this however, this can and most probably will
 * change in future.
 */

class KIOCachedJob : public KIOJob
{
    Q_OBJECT
public:
    enum Step 
    {
      STEP_NONE, STEP_REDIRECTION, STEP_DATE, STEP_MIME, STEP_DATA, STEP_FINISHED
    };
  
    /**
     * Constructs a new job. Overloaded from @ref KIOJob
     */
    KIOCachedJob();
    ~KIOCachedJob();
  
    /**
      * Calling forceReload(TRUE) (or forceReload() for convenience)
      * forces KIOCachedJob to retrieve all documents requested before a
      * subsequent call to forceReload(FALSE) directly from the original
      * source, even if they are in cache and valid. It doesn't turn off
      * caching of these documents however, use KIOCache::enable(FALSE)
      * to turn off caching altogether or use KIOJob instead of
      * CachedIOJob to bypass the cache for certain documents.
      */
    void forceReload(bool _force_reload = TRUE) 
	{ m_bLookInCache = !_force_reload; }
    
    /**
      * Retrieve _url, get it from cache if possible and if _reload is
      * FALSE. Data is delivered by sending appropriate signals (see
      * @ref KIOJob for details).
      */
    void get( const char *_url, bool _reload = false );
    
protected slots:
    ////////////////
    // Slots used internally for handling cached data.
    ////////////////

    /**
     * This slot is connected to the see #finished signal of the same
     * KIOCachedJob when a document is retrieved and should be stored
     * in cache. The document is written to the cache in this function.
     */
    virtual void slotFinished();
    
    /**
     * This slot is connected to the @see #data signal of this object
     * while a document is retrieved that should be stored in cache.
     * It simply adds all data received to @see dataBuffer which is
     * used later to store all of the data in cache (and maybe to pass
     * the data already available to KIOCachedJobs in repeater mode).
     */
    virtual void slotData( void *_data, int _len);

    virtual void slotRedirection( const char *_url );
    virtual void slotMimeType( const char *_mime );
    virtual void slotError( int, const char* );
  
    /**
     * This slot is connected to the @see #date signal of this object 
     * while a document is retrieved that should be stored in
     * cache. It stores the appropriate dates (most notably EXPIRES)
     * in order to commit them to cache later when @see #commitToCache
     * is called.
     */
    // void storeDate();

    /////////////////////////
    // Slots used if we got a cache hit.
    ////////////////////////

    void slotTimeout();
  
protected:
    /**
      * If TRUE, get documents from cache if possible. If false,
      * always retrieve documents from the original source. See also
      * @ref #forceReload. By default, look_in_cache is TRUE.
      */
    bool m_bLookInCache;

    /**
      * A pointer to the new cache entry for the current document. 
      */
    KIOCacheEntry *m_pCurrentDoc;  

    QTimer m_timer;
    Step m_step;
};

#ifndef NO_KIO_COMPATABILITY

/**
 * For backwards compatability, do NOT rely on the presence
 * of this typedef, convert your code to use KIOCachedJob instead.
 * To check if your code is ok, try compiling with NO_KIO_COMPATABILITY
 * defined.
 */
typedef KIOCachedJob CachedKIOJob;

#warning "KIO Compability is enabled, define NO_KIO_COMPATABILTY to test your code"

#endif // NO_KIO_COMPATABILITY

#endif
