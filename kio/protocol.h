// $Id$

#ifndef __protocol_h__
#define __protocol_h__

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>

#include <errno.h>
#include <stdio.h>
#ifdef DO_SSL
#include <kssl.h>
#endif
#include <unistd.h>
#include <netdb.h>

#include <qstringlist.h>
#include <qstring.h>

#include <kio/global.h>
#include <kio/slavebase.h>
#include <ksock.h>
#include <kurl.h>


// Default ports.. you might want to change this if you're trying to dodge
// a proxy with some creative network address translation..  HTTP_PORT
// is used for HTTP and WebDAV/WebFolders, HTTPS_PORT is well used for
// HTTPS. Duh.
#define DEFAULT_HTTP_PORT	80
#define DEFAULT_HTTPS_PORT	443
#define DEFAULT_FTP_PORT	21

class Protocol : public KIO::SlaveBase
{
 public:
    Protocol(const QCString &protocol, const QCString &pool, const QCString &app);
    virtual ~Protocol();
    enum PROTOCOL  {HTTP, HTTPS, WEBDAV, FTP};

    enum CacheControl { CC_CacheOnly, // Fail request if not in cache
			CC_Cache, // Use cached entry if available
			CC_Verify, // Validate cached entry with remote site
			CC_Reload // Always fetch from remote site.
    };

    class State
	{
	public:
	    QString hostname;
	    short unsigned int port;
	    QString user;
	    QString passwd;
	    bool  do_proxy;
	    QString cef; // Cache Entry File belonging to this URL.
	};

    typedef struct
    {
	QString hostname;
	short unsigned int port;
	QString user;
	QString passwd;
	QString path;
	QString query;
	HTTP_METHOD method;
	CacheControl cache;
	unsigned long offset;
	bool do_proxy;
	KURL url;
	QString window; // The window Id this request is related to.
	QString user_headers;
    } HTTPRequest;

    void cache_update( const KURL &url, bool no_cache, time_t expireDate);

    /**
     * Send a cookie to the cookiejar
     */
    void addCookies( const QString &url, const QCString &cookieHeader);

    /**
     * Look for cookies in the cookiejar
     */
    QString findCookies( const QString &url);

    /**
     * Do a cache lookup for the current url. (m_state.url)
     *
     * @param readWrite If true, file is opened read/write.
     *                  If false, file is opened read-only.
     *
     * @return a file stream open for reading and at the start of
     *         the header section when the Cache entry exists and is valid.
     *         0 if no cache entry could be found, or if the entry is not
     *         valid (any more).
     */
    FILE *checkCacheEntry(bool readWrite = false);

    /**
     * Create a cache entry for the current url. (m_state.url)
     *
     * Set the contents type of the cache entry to 'mimetype'.
     */
    void createCacheEntry(const QString &mimetype, time_t expireDate);

    /**
     * Write data to cache.
     *
     * Write 'nbytes' from 'buffer' to the Cache Entry File
     */
    void writeCacheEntry( const char *buffer, int nbytes);

    /**
     * Close cache entry
     */
    void closeCacheEntry();

    /**
     * Update expire time of current cache entry.
     */
    void updateExpireDate(time_t expireDate);

    /**
     * Quick check whether the cache needs cleaning.
     */
    void cleanCache();

 protected:
    enum PROTOCOL m_proto;

    State m_state;
    Request m_request;

    bool m_bEOF;
    int m_sock;
    FILE* m_fsocket;

    int m_iSize; // Expected size of message
    long m_iBytesLeft; // # of bytes left to receive in this message.
    QByteArray m_bufReceive; // Receive buffer

    // Cache related
    bool m_bUseCache; // Whether the cache is active
    bool m_bCachedRead; // Whether the file is to be read from m_fcache.
    bool m_bCachedWrite; // Whether the file is to be written to m_fcache.
    int m_maxCacheAge; // Maximum age of a cache entry.
    FILE* m_fcache; // File stream of a cache entry
    QString m_strCacheDir; // Location of the cache.
    QString m_etag; // ETag header.
    QString m_lastModified; // Last modified.
    bool m_bMustRevalidate; // Cache entry is expired.
    long m_cacheExpireDateOffset; // Position in the cache entry where the
                                // 16 byte expire date is stored.
    time_t m_expireDate;

    // Persistant connections
    bool m_bKeepAlive;

    /**
     * Sets the host
     * @param host
     * @param port
     * @param user
     * @param pass
     * Called directly by createSlave, this is why there is no equivalent in
     * SlaveInterface, unlike the other methods.
     */
    virtual void setHost(const QString& host, int port, const QString& user, const QString& pass);

    void error( int _errid, const QString &_text );

    int openStream();

    // Proxy related members
    bool m_bUseProxy;  // Whether we want a proxy
    int m_iProxyPort;
    KURL m_proxyURL;
    QString m_strNoProxyFor;
    QString m_strProxyRealm;

    ksockaddr_in m_proxySockaddr;
    QCString m_protocol;

    DCOPClient *m_dcopClient;

    short unsigned int mDefaultPort;

    // Values that determine the remote connection timeouts.
    int m_proxyConnTimeout, m_remoteConnTimeout, m_remoteRespTimeout;

}

#endif
