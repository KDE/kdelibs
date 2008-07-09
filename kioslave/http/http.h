/*
   Copyright (C) 2000,2001 Dawit Alemayehu <adawit@kde.org>
   Copyright (C) 2000,2001 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2000,2001 George Staikos <staikos@kde.org>
   Copyright (C) 2001,2002 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2007      Daniel Nicoletti <mirttex@users.sourceforge.net>


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
  enum HTTP_AUTH   {AUTH_None, AUTH_Basic, AUTH_NTLM, AUTH_Digest, AUTH_Negotiate};

  /** HTTP / DAV method **/
  // Removed to interfaces/kio/http.h
  //enum HTTP_METHOD {HTTP_GET, HTTP_PUT, HTTP_POST, HTTP_HEAD, HTTP_DELETE,
  //                  HTTP_OPTIONS, DAV_PROPFIND, DAV_PROPPATCH, DAV_MKCOL,
  //                  DAV_COPY, DAV_MOVE, DAV_LOCK, DAV_UNLOCK, DAV_SEARCH };

  /** State of the current Connection **/
  struct HTTPState
  {
    HTTPState ()
    {
      port = 0;
      doProxy = false;
    }

    QString hostname;
    QString encoded_hostname;
    short unsigned int port;
    QString user;
    QString passwd;
    bool doProxy;
    KUrl proxyUrl;
  };

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

  /** The request for the current connection **/
  struct HTTPRequest
  {
    HTTPRequest ()
    {
      port = 0;
      method = KIO::HTTP_UNKNOWN;
      offset = 0;
      endoffset = 0;
      doProxy = false;
      allowCompressedPage = false;
      disablePassDlg = false;
      bNoAuth = false;
      bUseCache = false;
      bCachedRead = false;
      bCachedWrite = false;
      fcache = 0;
      bMustRevalidate = false;
      cacheExpireDateOffset = 0;
      bErrorPage = false;
      bUseCookiejar = false;
      expireDate = 0;
      creationDate = 0;
      bytesCached=0;
    }

    QString hostname;
    QString encoded_hostname;
    short unsigned int port;
    QString user;
    QString passwd;
    QString path;
    QString query;
    KIO::HTTP_METHOD method;
    KIO::CacheControl cache;
    KIO::filesize_t offset;
    KIO::filesize_t endoffset;
    KUrl url;
    QString window;                 // Window Id this request is related to.
    QString referrer;
    QString charsets;
    QString languages;
    QString userAgent;
    QString id;
    DAVRequest davData;
    bool doProxy;
    KUrl proxyUrl;
    bool allowCompressedPage;
    bool disablePassDlg;
    bool bNoAuth; // Do not authenticate

    // Indicates whether an error-page or error-msg should is preferred.
    bool bErrorPage;

    // Cookie flags
    bool bUseCookiejar;

    // Cache related
    bool bUseCache; // Whether the cache is active
    bool bCachedRead; // Whether the file is to be read from m_fcache.
    bool bCachedWrite; // Whether the file is to be written to m_fcache.
    bool bMustRevalidate; // Cache entry is expired.
    QString cef; // Cache Entry File belonging to this URL.
    gzFile fcache; // File stream of a cache entry
    QString etag; // ETag header.
    QString lastModified; // Last modified.
    long cacheExpireDateOffset; // Position in the cache entry where the
                                  // 16 byte expire date is stored.
    long bytesCached;
    time_t expireDate; // Date when the cache entry will expire
    time_t creationDate; // Date when the cache entry was created
    QString strCharset;

    // Cookie flags
    enum { CookiesAuto, CookiesManual, CookiesNone } cookieMode;
  };

  struct DigestAuthInfo
  {
    QByteArray nc;
    QByteArray qop;
    QByteArray realm;
    QByteArray nonce;
    QByteArray method;
    QByteArray cnonce;
    QByteArray username;
    QByteArray password;
    KUrl::List digestURI;
    QByteArray algorithm;
    QByteArray entityBody;
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
  bool checkRequestUrl( const KUrl& );
  void cacheUpdate( const KUrl &url, bool nocache, time_t expireDate);

  void httpError(); // Generate error message based on response code

  bool isOffline(const KUrl &url); // Check network status

protected Q_SLOTS:
  void slotData(const QByteArray &);
  void error( int _errid, const QString &_text );

protected:
  int readChunked();    // Read a chunk
  int readLimited();    // Read maximum m_iSize bytes.
  int readUnlimited();  // Read as much as possible.

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

  void setRewindMarker();
  void rewind();

  /**
    * Add an encoding on to the appropriate stack this
    * is nececesary because transfer encodings and
    * content encodings must be handled separately.
    */
  void addEncoding(const QString &, QStringList &);

  void configAuth( char *, bool );

  bool sendQuery();
  void httpClose(bool keepAlive);  // Close transfer

  bool httpOpenConnection();   // Open connection
  void httpCloseConnection();  // Close connection
  bool httpShouldCloseConnection();  // Check whether to keep the connection.

  void forwardHttpResponseHeader();

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
  gzFile checkCacheEntry(bool readWrite = false);

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
   * Resets settings related to parsing a response.
   */
  void resetResponseSettings();

  /**
   * Resets any per connection settings.  These are different from
   * per-session settings in that they must be invalidates every time
   * a request is made, e.g. a retry to re-send the header to the
   * server, as compared to only when a new request arrives.
   */
  void resetConnectionSettings();

  /**
   * Returns any pre-cached proxy authentication info
   * info in HTTP header format.
   */
  QString proxyAuthenticationHeader();

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
   * Creates the entity-header for NTLM authentication.
   */
  QString createNTLMAuth( bool isForProxy = false );

  /**
   * Creates the entity-header for Negotiate authentication.
   */
  QString createNegotiateAuth();

  /**
   * create GSS error string
   */
  QByteArray gssError( int major_status, int minor_status );

  /**
   * Calcualtes the message digest response based on RFC 2617.
   */
  void calculateResponse( DigestAuthInfo &info, QByteArray &Response );

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
  QList<HTTPRequest*> m_requestQueue;
  quint16 m_defaultPort;

  // Processing related
  KIO::filesize_t m_iSize; // Expected size of message
  KIO::filesize_t m_iBytesLeft; // # of bytes left to receive in this message.
  KIO::filesize_t m_iContentLeft; // # of content bytes left
  QByteArray m_bufReceive; // Receive buffer
  char m_lineBuf[1024];
  char m_rewindBuf[8192];
  size_t m_rewindCount;
  size_t m_lineCount;
  size_t m_lineCountUnget;
  char *m_linePtr;
  char *m_lineBufUnget;
  char *m_linePtrUnget;
  bool m_dataInternal; // Data is for internal consumption
  bool m_bChunked; // Chunked transfer encoding

  bool m_bBusy; // Busy handling request queue.
  bool m_bEOF;
  bool m_bEOD;

  // First request on a connection
  bool m_bFirstRequest;

//--- Settings related to a single response only
  bool m_bRedirect; // Indicates current request is a redirection
  QStringList m_responseHeaders; // All headers


  // Language/Encoding related
  QStringList m_qTransferEncodings;
  QStringList m_qContentEncodings;
  QString m_sContentMD5;
  QString m_strMimeType;


//--- WebDAV
  // Data structure to hold data which will be passed to an internal func.
  QByteArray m_bufWebDavData;
  QStringList m_davCapabilities;

  bool m_davHostOk;
  bool m_davHostUnsupported;
//----------

  // Mimetype determination
  bool m_cpMimeBuffer;
  QByteArray m_mimeTypeBuffer;


  // Holds the POST data so it won't get lost on if we
  // happend to get a 401/407 response when submitting,
  // a form.
  QByteArray m_bufPOST;

  // Cache related
  int m_maxCacheAge; // Maximum age of a cache entry.
  long m_maxCacheSize; // Maximum cache size in Kb.
  QString m_strCacheDir; // Location of the cache.



//--- Proxy related members
  bool m_bUseProxy;
  bool m_bNeedTunnel; // Whether we need to make a SSL tunnel
  bool m_bIsTunneled; // Whether we have an active SSL tunnel
  bool m_bProxyAuthValid;
  int m_iProxyPort;
  KUrl m_proxyURL;
  QString m_strProxyRealm;

  // Operation mode
  QByteArray m_protocol;

  // Authentication
  QString m_strRealm;
  QString m_strAuthorization;
  QString m_strProxyAuthorization;
  HTTP_AUTH Authentication;
  HTTP_AUTH ProxyAuthentication;
  short unsigned int m_iProxyAuthCount;
  short unsigned int m_iWWWAuthCount;
  bool m_bUnauthorized;

  // Persistent proxy connections
  bool m_bPersistentProxyConnection;


  // Indicates whether there was some connection error.
  bool m_bError;

  // Persistent connections
  bool m_bKeepAlive;
  int m_keepAliveTimeout; // Timeout in seconds.

  // Previous and current response codes
  unsigned int m_responseCode;
  unsigned int m_prevResponseCode;

  // Values that determine the remote connection timeouts.
  int m_remoteRespTimeout;
};
#endif
