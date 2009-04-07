/*
   Copyright (C) 2000,2001 Dawit Alemayehu <adawit@kde.org>
   Copyright (C) 2000,2001 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2000,2001 George Staikos <staikos@kde.org>
   Copyright (C) 2001,2002 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2007      Daniel Nicoletti <mirttex@users.sourceforge.net>
   Copyright (C) 2008,2009 Andreas Hartmetz <ahartmetz@gmail.com>

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

#ifndef HTTP_H_
#define HTTP_H_


#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <zlib.h>
#include <time.h>

#include <QtCore/QByteRef>
#include <QtCore/QList>
#include <QtCore/QStringList>

#include <kurl.h>
#include "kio/tcpslavebase.h"
#include "kio/http.h"

// HeaderTokenizer declarations
#include "parsinghelpers.h"
// KHttpAuthentication & KHttpAuthenticationOutcome declarations
#include "httpauthentication.h"

class QDomNodeList;

namespace KIO {
    class AuthInfo;
}

class HTTPProtocol : public QObject, public KIO::TCPSlaveBase
{
  Q_OBJECT
public:
  HTTPProtocol( const QByteArray &protocol, const QByteArray &pool,
                const QByteArray &app );
  virtual ~HTTPProtocol();

  /** HTTP version **/
  enum HTTP_REV    {HTTP_None, HTTP_Unknown, HTTP_10, HTTP_11, SHOUTCAST};

  /** Authorization method used **/
  enum AUTH_SCHEME   {AUTH_None, AUTH_Basic, AUTH_NTLM, AUTH_Digest, AUTH_Negotiate};

  /** DAV-specific request elements for the current connection **/
  struct DAVRequest
  {
    DAVRequest ()
    {
      overwrite = false;
      depth = 0;
    }

    QString desturl;
    bool overwrite;
    int depth;
  };

  struct CacheTag
  {
    CacheTag()
    {
      useCache = false;
      readFromCache = false;
      writeToCache = false;
      isExpired = false;
      bytesCached = 0;
      gzs = 0;
      expireDateOffset = 0;
      expireDate = 0;
      creationDate = 0;
    }

    KIO::CacheControl policy;    // ### initialize in the constructor?
    bool useCache; // Whether the cache is active
    bool readFromCache; // Whether the request is to be read from file.
    bool writeToCache; // Whether the request is to be written to file.
    bool isExpired;
    long bytesCached;
    QString etag; // entity tag header as described in the HTTP standard.
    QString file; // cache entry file belonging to this URL.
    gzFile gzs; // gzip stream of the cache entry
    QString lastModified; // Last modified.
    long expireDateOffset; // Position in the cache entry where the
                           // 16 byte expire date is stored.
    time_t expireDate; // Date when the cache entry will expire
    time_t creationDate; // Date when the cache entry was created
    QString charset;
  };

  /** The request for the current connection **/
  struct HTTPRequest
  {
    HTTPRequest ()
    {
      method = KIO::HTTP_UNKNOWN;
      offset = 0;
      endoffset = 0;
      allowTransferCompression = false;
      disablePassDialog = false;
      doNotAuthenticate = false;
      preferErrorPage = false;
      useCookieJar = false;
    }

    KUrl url;
    QString encoded_hostname; //### can be calculated on-the-fly
    // Persistent connections
    bool isKeepAlive;
    int keepAliveTimeout;   // Timeout in seconds.

    KIO::HTTP_METHOD method;
    KIO::filesize_t offset;
    KIO::filesize_t endoffset;
    QString windowId;                 // Window Id this request is related to.
    // Header fields
    QString referrer;
    QString charsets;
    QString languages;
    QString userAgent;
    // Previous and current response codes
    unsigned int responseCode;
    unsigned int prevResponseCode;
    // Miscellaneous
    QString id;
    DAVRequest davData;
    KUrl proxyUrl;
    bool isPersistentProxyConnection;
    bool allowTransferCompression;
    bool disablePassDialog;
    bool doNotAuthenticate;
    // Indicates whether an error page or error message is preferred.
    bool preferErrorPage;

    // Use the cookie jar (or pass cookies to the application as metadata instead)
    bool useCookieJar;
    // Cookie flags
    enum { CookiesAuto, CookiesManual, CookiesNone } cookieMode;

    CacheTag cacheTag;
  };

  /** State of the current connection to the server **/
  struct HTTPServerState
  {
    HTTPServerState()
    {
      isKeepAlive = false;
      isPersistentProxyConnection = false;
    }

    void initFrom(const HTTPRequest &request)
    {
      url = request.url;
      encoded_hostname = request.encoded_hostname;
      isKeepAlive = request.isKeepAlive;
      proxyUrl = request.proxyUrl;
      isPersistentProxyConnection = request.isPersistentProxyConnection;
    }

    void clear()
    {
      url.clear();
      encoded_hostname.clear();
      proxyUrl.clear();
      isKeepAlive = false;
      isPersistentProxyConnection = false;
    }

    KUrl url;
    QString encoded_hostname;
    KUrl proxyUrl;
    bool isKeepAlive;
    bool isPersistentProxyConnection;
  };

//---------------------- Re-implemented methods ----------------
  virtual void setHost(const QString& host, quint16 port, const QString& user,
                       const QString& pass);

  virtual void slave_status();

  virtual void get( const KUrl& url );
  virtual void put( const KUrl& url, int _mode, KIO::JobFlags flags );

//----------------- Re-implemented methods for WebDAV -----------
  virtual void listDir( const KUrl& url );
  virtual void mkdir( const KUrl& url, int _permissions );

  virtual void rename( const KUrl& src, const KUrl& dest, KIO::JobFlags flags );
  virtual void copy( const KUrl& src, const KUrl& dest, int _permissions, KIO::JobFlags flags );
  virtual void del( const KUrl& url, bool _isfile );

  // ask the host whether it supports WebDAV & cache this info
  bool davHostOk();

  // send generic DAV request
  void davGeneric( const KUrl& url, KIO::HTTP_METHOD method );

  // Send requests to lock and unlock resources
  void davLock( const KUrl& url, const QString& scope,
                const QString& type, const QString& owner );
  void davUnlock( const KUrl& url );

  // Calls httpClose() and finished()
  void davFinished();

  // Handle error conditions
  QString davError( int code = -1, const QString &url = QString() );
//---------------------------- End WebDAV -----------------------

  /**
   * Special commands supported by this slave :
   * 1 - HTTP POST
   * 2 - Cache has been updated
   * 3 - SSL Certificate Cache has been updated
   * 4 - HTTP multi get
   * 5 - DAV LOCK     (see
   * 6 - DAV UNLOCK     README.webdav)
   */
  virtual void special( const QByteArray &data );

  virtual void mimetype( const KUrl& url);

  virtual void stat( const KUrl& url );

  virtual void reparseConfiguration();

  virtual void closeConnection(); // Forced close of connection

  void post( const KUrl& url );
  void multiGet(const QByteArray &data);
  bool maybeSetRequestUrl(const KUrl &);
  void cacheUpdate( const KUrl &url, bool nocache, time_t expireDate);

  void httpError(); // Generate error message based on response code
  void setLoadingErrorPage(); // Call SlaveBase::errorPage() and remember that we've called it

  bool isOffline(const KUrl &url); // Check network status

protected Q_SLOTS:
  void slotData(const QByteArray &);
  void error(int errid, const QString &text);
  void proxyAuthenticationForSocket(const QNetworkProxy &, QAuthenticator *);
  void saveProxyAuthenticationForSocket();

protected:
  int readChunked();    // Read a chunk
  int readLimited();    // Read maximum m_iSize bytes.
  int readUnlimited();  // Read as much as possible.

  /**
    * A thin wrapper around TCPSlaveBase::write() that will retry writing as
    * long as no error occurs.
    */
  ssize_t write(const void *buf, size_t nbytes);

  /**
    * Add an encoding on to the appropriate stack this
    * is nececesary because transfer encodings and
    * content encodings must be handled separately.
    */
  void addEncoding(const QString &, QStringList &);

  //void processAuthenticationRequest();

  // The methods between here and sendQuery() are helpers for sendQuery().

  // Return true if the request is already "done", false otherwise.
  // *sucesss will be set to true if the page was found, false otherwise.
  bool satisfyRequestFromCache(bool *success);
  QString formatRequestUri() const;
  // create HTTP authentications response(s), if any
  QString authenticationHeader();
  bool sendQuery();
  
  void httpClose(bool keepAlive);  // Close transfer
  bool httpOpenConnection();   // Open connection
  void httpCloseConnection();  // Close connection
  bool httpShouldCloseConnection();  // Check whether to keep or close the connection.

  void forwardHttpResponseHeader();

  // Helper for readResponseHeader - fix common mimetype errors by webservers.
  void fixupResponseMimetype();
  bool readResponseHeader();
  bool readHeaderFromCache();
  void parseContentDisposition(const QString &disposition);

  bool sendBody();

  // where dataInternal == true, the content is to be made available
  // to an internal function.
  bool readBody( bool dataInternal = false );

  /**
   * Performs a WebDAV stat or list
   */
  void davSetRequest( const QByteArray& requestXML );
  void davStatList( const KUrl& url, bool stat = true );
  void davParsePropstats( const QDomNodeList& propstats, KIO::UDSEntry& entry );
  void davParseActiveLocks( const QDomNodeList& activeLocks,
                            uint& lockCount );

  /**
   * Parses a date & time string
   */
  long parseDateTime( const QString& input, const QString& type );

  /**
   * Returns the error code from a "HTTP/1.1 code Code Name" string
   */
  int codeFromResponse( const QString& response );

  /**
   * Extracts locks from metadata
   * Returns the appropriate If: header
   */
  QString davProcessLocks();

  /**
   * Send a cookie to the cookiejar
   */
  void addCookies( const QString &url, const QByteArray &cookieHeader);

  /**
   * Look for cookies in the cookiejar
   */
  QString findCookies( const QString &url);

  /**
   * Do a cache lookup for the current url. (m_server.url)
   *
   * @param readWrite If true, file is opened read/write.
   *                  If false, file is opened read-only.
   *
   * @return a file stream open for reading and at the start of
   *         the header section when the Cache entry exists and is valid.
   *         0 if no cache entry could be found, or if the entry is not
   *         valid (any more).
   */
  gzFile checkCacheEntry(bool readWrite = false);

  /**
   * Create a cache entry for the current url. (m_server.url)
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
   * Do everything proceedUntilResponseHeader does, and also get the response body.
   * This is being used as a replacement for proceedUntilResponseHeader() in
   * situations where we actually expect the response to have a body / payload data.
   */
  // where dataInternal == true, the content is to be made available
  // to an internal function.
  void proceedUntilResponseContent( bool dataInternal = false );

  /**
   * Ensure we are connected, send our query, and get the response header.
   */
  bool proceedUntilResponseHeader();

  /**
   * Resets any per session settings.
   */
  void resetSessionSettings();

  /**
   * Resets variables related to parsing a response.
   */
  void resetResponseParsing();

  /**
   * Resets any per connection settings. These are different from
   * per-session settings in that they must be invalidated every time
   * a request is made, e.g. a retry to re-send the header to the
   * server, as compared to only when a new request arrives.
   */
  void resetConnectionSettings();

  /**
   * Creates the entity-header for Negotiate authentication.
   */
  QString createNegotiateAuth();

  /**
   * create GSS error string
   */
  QByteArray gssError( int major_status, int minor_status );

  /**
   * Creates authorization prompt info.
   */
  void fillPromptInfo(KIO::AuthInfo *info);

protected:
  HTTPServerState m_server;
  HTTPRequest m_request;
  QList<HTTPRequest> m_requestQueue;
  quint16 m_defaultPort;

  // Processing related
  KIO::filesize_t m_iSize; // Expected size of message
  KIO::filesize_t m_iBytesLeft; // # of bytes left to receive in this message.
  KIO::filesize_t m_iContentLeft; // # of content bytes left
  QByteArray m_receiveBuf; // Receive buffer
  bool m_dataInternal; // Data is for internal consumption
  bool m_isChunked; // Chunked transfer encoding

  bool m_isBusy; // Busy handling request queue.
  bool m_isEOF;
  bool m_isEOD;

  // First request on a connection
  bool m_isFirstRequest;

//--- Settings related to a single response only
  bool m_isRedirection; // Indicates current request is a redirection
  QStringList m_responseHeaders; // All headers


  // Language/Encoding related
  QStringList m_transferEncodings;
  QStringList m_contentEncodings;
  QString m_contentMD5;
  QString m_mimeType;


//--- WebDAV
  // Data structure to hold data which will be passed to an internal func.
  QByteArray m_webDavDataBuf;
  QStringList m_davCapabilities;

  bool m_davHostOk;
  bool m_davHostUnsupported;
//----------

  // Mimetype determination
  bool m_cpMimeBuffer;
  QByteArray m_mimeTypeBuffer;


  // Holds the POST data so it won't get lost on if we
  // happend to get a 401/407 response when submitting
  // a form.
  QByteArray m_POSTbuf;

  // Cache related
  int m_maxCacheAge; // Maximum age of a cache entry.
  long m_maxCacheSize; // Maximum cache size in Kb.
  QString m_strCacheDir; // Location of the cache.


//--- Proxy related members

  // Operation mode
  QByteArray m_protocol;

  //TODO use PrevRequest fully
  //TODO isAuthValid/m_isUnauthorized -> auth.scheme != AUTH_None ?
#if 0
  struct AuthState
  {
    AUTH_SCHEME scheme;
    QString realm;
    QString authorization;
    int authCount;
  };
#endif

  KAbstractHttpAuthentication *m_wwwAuth;
  KAbstractHttpAuthentication *m_proxyAuth;
  // For proxy auth when it's handled by the Qt/KDE socket classes
  QAuthenticator *m_socketProxyAuth;

  // Indicates whether there was some connection error.
  bool m_isError;
  // Whether we are loading an error page (we should close the connection afterwards)
  bool m_isLoadingErrorPage;

  // Values that determine the remote connection timeouts.
  int m_remoteRespTimeout;


  QByteArray m_unreadBuf;
  void clearUnreadBuffer();
  void unread(char *buf, size_t size);
  size_t readBuffered(char *buf, size_t size);
  bool readDelimitedText(char *buf, int *idx, int end, int numNewlines);
};
#endif
