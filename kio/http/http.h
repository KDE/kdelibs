// $Id$

#ifndef __http_h__
#define __http_h__

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
#include <qstrlist.h>

#include <kio/global.h>
#include <kio/slavebase.h>
#include <ksock.h>
#include <kurl.h>
#include <kmdcodec.h>

// Default ports.. you might want to change this if you're trying to dodge
// a proxy with some creative network address translation..  HTTP_PORT
// is used for HTTP and WebDAV/WebFolders, HTTPS_PORT is well used for
// HTTPS. Duh.
#define DEFAULT_HTTP_PORT       80
#define DEFAULT_HTTPS_PORT      443
#define DEFAULT_FTP_PORT        21

class DCOPClient;

class HTTPProtocol : public KIO::SlaveBase
{
public:
  HTTPProtocol( const QCString &protocol, const QCString &pool, const QCString &app );
  virtual ~HTTPProtocol();

  enum HTTP_REV    {HTTP_Unknown, HTTP_10, HTTP_11};
  enum HTTP_AUTH   {AUTH_None, AUTH_Basic, AUTH_Digest};
  enum HTTP_PROTO  {PROTO_HTTP, PROTO_HTTPS, PROTO_WEBDAV};
  enum HTTP_METHOD {HTTP_GET, HTTP_PUT, HTTP_POST, HTTP_HEAD, HTTP_DELETE};

  enum CacheControl { CC_CacheOnly, // Fail request if not in cache
                      CC_Cache,     // Use cached entry if available
                      CC_Verify,    // Validate cached entry with remote site
                      CC_Reload     // Always fetch from remote site.
                      };

  typedef struct
  {
    QString hostname;
    short unsigned int port;
    QString user;
    QString passwd;
    bool  do_proxy;
    QString cef; // Cache Entry File belonging to this URL.
  } HTTPState;

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
    QString window;                 // Window Id this request is related to.
  } HTTPRequest;

  typedef struct
  {
    QCString nc;
    QCString qop;
    QCString realm;
    QCString nonce;
    QCString method;
    QCString cnonce;
    QCString username;
    QCString password;
    QStrList digestURI;
    QCString algorithm;
    QCString entity_body;
  } DigestAuthInfo;

  /**
   * Fills in m_request.url from the rest of the request data.
   */
  void buildURL();

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

  virtual void slave_status();

  virtual void get( const KURL& url );
  virtual void put( const KURL& url, int _mode, bool _overwrite, bool _resume );
  void post( const KURL& url );
  void cache_update( const KURL &url, bool no_cache, time_t expireDate);

  /**
   * Special commands supported by this slave :
   * 1 - HTTP POST
   */
  virtual void special( const QByteArray &data);

  virtual void mimetype( const KURL& url);

  virtual void stat( const KURL& url );

  virtual void reparseConfiguration();

  virtual void closeConnection(); // Forced close of connection

protected:

  void error( int _errid, const QString &_text );

  int readChunked();    // Read a chunk
  int readLimited();    // Read maximum m_iSize bytes.
  int readUnlimited();  // Read as much as possible.

  void decodeGzip();    //decodes data compressed with gzip algorithm
  void decodeDeflate(); //decodes data compressed with deflate algorithm

  int openStream();

  /**
    * A "smart" wrapper around write that will use SSL_write or
    * write(2) depending on whether you've got an SSL connection or not.
    * The only shortcomming is that it uses the "global" file handles and
    * soforth.  So you can't really use this on individual files/sockets.
    */
  ssize_t write(const void *buf, size_t nbytes);

  /**
    * Another "smart" wrapper, this time around read that will
    * use SSL_read or read(2) depending on whether you've got an
    * SSL connection or not.
    */
  ssize_t read (void *b, size_t nbytes);

  char *gets (char *str, int size);

  /**
    * An SSLified feof().
    */
  bool eof ();

  /**
    * Add an encoding on to the appropiate stack this
    * is nececesary because transfer encodings and
    * content encodings must be handled separately.
    */
  void addEncoding(QString, QStringList &);

  void configAuth(const char *, bool, bool firstCall = true );
#ifdef DO_SSL
  void initSSL();
  void closeSSL();
  void resetSSL();
#endif

  size_t sendData();

  bool http_open();             // Open transfer
  void http_close();            // Close transfer

  bool http_openConnection();   // Open connection
  void http_checkConnection();  // Check whether to keep connection.
  void http_closeConnection();  // Close conection
  bool http_isConnected();      // Checks for existing connection.

  bool readHeader();
  bool sendBody();
  bool readBody();
  bool checkSSL();

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
  void updateExpireDate(time_t expireDate, bool updateCreationDate=false);

  /**
   * Quick check whether the cache needs cleaning.
   */
  void cleanCache();

  /**
   * Performs a GET HTTP request.
   */
  void retrieveContent(bool check_ssl = false);

  /**
   * Performs a HEAD HTTP request.
   */
  bool retrieveHeader(bool close_connection = true);

  /**
   * Resets any per session settings.
   */
  void resetSessionSettings();

  /**
   * Retrieves authorization info from cache or user.
   */
  bool getAuthorization();

  /**
   * Saves valid authorization info in the cache daemon.
   */
  void saveAuthorization();

  /**
   * Creates the entity-header for Basic authentication.
   */
  QString createBasicAuth( bool isForProxy = false );

  /**
   * Creates the entity-header for Digest authentication.
   */
  QString createDigestAuth( bool isForProxy = false );

  /**
   * Calcualtes the message digest response based on RFC 2617.
   */
  void calculateResponse( DigestAuthInfo& info, HASHHEX Response );

  /**
   * Prompts the user for authorization retry.
   */
  bool retryPrompt();

  /**
   * Creates authorization prompt info.
   */
  void promptInfo( KIO::AuthInfo& info );

protected:
  HTTPState m_state;
  HTTPRequest m_request;

  bool m_bEOF;
  int m_sock;
  FILE* m_fsocket;
  HTTP_REV m_HTTPrev;
  HTTP_PROTO m_proto;

  int m_iSize; // Expected size of message
  long m_iBytesLeft; // # of bytes left to receive in this message.
  QByteArray m_bufReceive; // Receive buffer

  // Holds the POST data so it won't get lost on if we
  // happend to get a 401/407 response when submitting,
  // a form.
  QByteArray m_bufPOST;

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

  // Language/Encoding
  QStringList m_qTransferEncodings;
  QStringList m_qContentEncodings;
  QByteArray big_buffer;
  QString m_sContentMD5;
  QString m_strMimeType;
  QString m_strCharset;
  QString m_strCharsets;
  QString m_strLanguages;

  // Proxy related members
  bool m_bUseProxy;
  int m_iProxyPort;
  KURL m_proxyURL;
  QString m_strNoProxyFor;
  QString m_strProxyRealm;

  ksockaddr_in m_proxySockaddr;
  QCString m_protocol;

  // Authentication
  QString m_strRealm;
  QString m_strAuthorization;
  QString m_strProxyAuthorization;
  HTTP_AUTH Authentication;
  HTTP_AUTH ProxyAuthentication;
  bool m_bUnauthorized;

  // Persistant connections
  bool m_bKeepAlive;

  // Resumable connections
  bool m_bCanResume;

  // Chunked tranfer encoding
  bool m_bChunked;

  // Cookie flags
  bool m_bUseCookiejar;
  enum { CookiesAuto, CookiesManual, CookiesNone } m_cookieMode;

  // Flag that indicates whether there was some connection
  // error...
  bool m_bError;

  // Flag that indicates application prefers errorPage() instead over error.
  bool m_bErrorPage;

  DCOPClient *m_dcopClient;

  short unsigned int m_DefaultPort;

  // Previous and current response codes
  unsigned int m_responseCode;
  unsigned int m_prevResponseCode;

  // Values that determine the remote connection timeouts.
  int m_proxyConnTimeout;
  int m_remoteConnTimeout;
  int m_remoteRespTimeout;

#ifdef DO_SSL
  bool m_bUseSSL;
  KSSL m_ssl;
  QString m_ssl_ip;
#endif

};
#endif
