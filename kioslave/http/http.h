/*
   Copyright (C) 2000,2001 Dawit Alemayehu <adawit@kde.org>
   Copyright (C) 2000,2001 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2000,2001 George Staikos <staikos@kde.org>
   Copyright (C) 2001,2002 Hamish Rodda <meddie@yoyo.cc.monash.edu.au>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef HTTP_H_
#define HTTP_H_


#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <qptrlist.h>
#include <qstrlist.h>
#include <qstringlist.h>

#include <kurl.h>
#include "kio/tcpslavebase.h"
#include "kio/http.h"

class DCOPClient;
class QDomElement;
class QDomNodeList;

namespace KIO {
    class AuthInfo;
}

class HTTPProtocol : public QObject, public KIO::TCPSlaveBase
{
  Q_OBJECT
public:
  HTTPProtocol( const QCString &protocol, const QCString &pool,
                const QCString &app );
  virtual ~HTTPProtocol();

  /** HTTP version **/
  enum HTTP_REV    {HTTP_Unknown, HTTP_10, HTTP_11};

  /** Authorization method used **/
  enum HTTP_AUTH   {AUTH_None, AUTH_Basic, AUTH_Digest};

  /** HTTP / DAV method **/
  // Removed to interfaces/kio/http.h
  //enum HTTP_METHOD {HTTP_GET, HTTP_PUT, HTTP_POST, HTTP_HEAD, HTTP_DELETE,
  //                  HTTP_OPTIONS, DAV_PROPFIND, DAV_PROPPATCH, DAV_MKCOL,
  //                  DAV_COPY, DAV_MOVE, DAV_LOCK, DAV_UNLOCK, DAV_SEARCH };

  /** State of the current Connection **/
  typedef struct
  {
    QString hostname;
    short unsigned int port;
    QString user;
    QString passwd;
    bool  doProxy;
    QString cef; // Cache Entry File belonging to this URL.
  } HTTPState;

  /** DAV-specific request elements for the current connection **/
  typedef struct
  {
    QString desturl;
    bool overwrite;
    int depth;
  } DAVRequest;

  /** The request for the current connection **/
  typedef struct
  {
    QString hostname;
    short unsigned int port;
    QString user;
    QString passwd;
    QString path;
    QString query;
    KIO::HTTP_METHOD method;
    KIO::CacheControl cache;
    KIO::filesize_t offset;
    bool doProxy;
    KURL url;
    QString window;                 // Window Id this request is related to.
    QString referrer;
    QString charsets;
    QString languages;
    bool allowCompressedPage;
    bool disablePassDlg;
    QString userAgent;
    QString id;
    DAVRequest davData;
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
    QCString entityBody;
  } DigestAuthInfo;

//---------------------- Re-implemented methods ----------------
  virtual void setHost(const QString& host, int port, const QString& user,
                       const QString& pass);

  virtual void slave_status();

  virtual void get( const KURL& url );
  virtual void put( const KURL& url, int _mode, bool _overwrite,
                    bool _resume );

//----------------- Re-implemented methods for WebDAV -----------
  virtual void listDir( const KURL& url );
  virtual void mkdir( const KURL& url, int _permissions );

  virtual void rename( const KURL& src, const KURL& dest, bool overwrite );
  virtual void copy( const KURL& src, const KURL& dest, int _permissions, bool overwrite );
  virtual void del( const KURL& url, bool _isfile );

  // ask the host whether it supports WebDAV & cache this info
  bool davHostOk();

  // send generic DAV request
  void davGeneric( const KURL& url, KIO::HTTP_METHOD method );

  // Send requests to lock and unlock resources
  void davLock( const KURL& url, const QString& scope,
                const QString& type, const QString& owner );
  void davUnlock( const KURL& url );

  // Calls httpClose() and finished()
  void davFinished();

  // Handle error conditions
  QString davError( int code = -1, QString url = QString::null );
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

  virtual void mimetype( const KURL& url);

  virtual void stat( const KURL& url );

  virtual void reparseConfiguration();

  virtual void closeConnection(); // Forced close of connection

  void post( const KURL& url );
  void multiGet(const QByteArray &data);
  bool checkRequestURL( const KURL& );
  void cacheUpdate( const KURL &url, bool nocache, time_t expireDate);

protected slots:
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

  void ungets(char *str, int size);

  /**
    * Add an encoding on to the appropiate stack this
    * is nececesary because transfer encodings and
    * content encodings must be handled separately.
    */
  void addEncoding(QString, QStringList &);

  void configAuth( const char *, bool );

  bool httpOpen();             // Open transfer
  void httpClose(bool keepAlive);  // Close transfer

  bool httpOpenConnection();   // Open connection
  void httpCloseConnection();  // Close connection
  void httpCheckConnection();  // Check whether to keep connection.

  void forwardHttpResponseHeader();
  
  bool readHeader();

  bool sendBody();

  // where dataInternal == true, the content is to be made available
  // to an internal function.
  bool readBody( bool dataInternal = false );

  /**
   * Performs a WebDAV stat or list
   */
  void davSetRequest( const QCString& requestXML );
  void davStatList( const KURL& url, bool stat = true );
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
  // where dataInternal == true, the content is to be made available
  // to an internal function.
  void retrieveContent( bool dataInternal = false );

  /**
   * Performs a HEAD HTTP request.
   */
  bool retrieveHeader(bool close_connection = true);

  /**
   * Resets any per session settings.
   */
  void resetSessionSettings();
  
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
   * Calcualtes the message digest response based on RFC 2617.
   */
  void calculateResponse( DigestAuthInfo &info, QCString &Response );

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
  QPtrList<HTTPRequest> m_requestQueue;

  HTTP_REV m_HTTPrev;

  bool m_bBusy; // Busy handling request queue.
  bool m_bEOF;

  int m_iSize; // Expected size of message
  long m_iBytesLeft; // # of bytes left to receive in this message.
  QByteArray m_bufReceive; // Receive buffer
  bool m_dataInternal; // Data is for internal consumption
  char m_lineBuf[1024];
  char *m_linePtr;
  size_t m_lineCount;
  char *m_lineBufUnget;
  char *m_linePtrUnget;
  size_t m_lineCountUnget;
  
  QStringList m_responseHeader;

//--- WebDAV
  // Data structure to hold data which will be passed to an internal func.
  QString m_intData;
  QString m_davVersions;

  bool m_davHostOk;
  bool m_davHostUnsupported;
//----------

  // Holds the POST data so it won't get lost on if we
  // happend to get a 401/407 response when submitting,
  // a form.
  QByteArray m_bufPOST;

  // Cache related
  bool m_bUseCache; // Whether the cache is active
  bool m_bCachedRead; // Whether the file is to be read from m_fcache.
  bool m_bCachedWrite; // Whether the file is to be written to m_fcache.
  int m_maxCacheAge; // Maximum age of a cache entry.
  long m_maxCacheSize; // Maximum cache size in Kb.
  FILE* m_fcache; // File stream of a cache entry
  QString m_strCacheDir; // Location of the cache.
  QString m_etag; // ETag header.
  QString m_lastModified; // Last modified.
  bool m_bMustRevalidate; // Cache entry is expired.
  long m_cacheExpireDateOffset; // Position in the cache entry where the
                                // 16 byte expire date is stored.
  time_t m_expireDate;	// Date when the cache entry will expire
  time_t m_creationDate; // Date when the cache entry was created

  // Language/Encoding
  QStringList m_qTransferEncodings;
  QStringList m_qContentEncodings;
  QString m_sContentMD5;
  QString m_strMimeType;
  QString m_strCharset;

  // Proxy related members
  bool m_bUseProxy;
  bool m_bNeedTunnel; // Whether we need to make a SSL tunnel
  bool m_bIsTunneled; // Whether we have an active SSL tunnel 
  int m_iProxyPort;
  KURL m_proxyURL;
  QString m_strProxyRealm;

  QCString m_protocol;

  // Authentication
  QString m_strRealm;
  QString m_strAuthorization;
  QString m_strProxyAuthorization;
  HTTP_AUTH Authentication;
  HTTP_AUTH ProxyAuthentication;
  bool m_bUnauthorized;
  short unsigned int m_iProxyAuthCount;
  short unsigned int m_iWWWAuthCount;

  // First request on a connection
  bool m_bFirstRequest;

  // Persistent connections
  bool m_bKeepAlive;

  // Persistent proxy connections
  bool m_bPersistentProxyConnection;

  // Resumable connections
  bool m_bCanResume;

  // Chunked tranfer encoding
  bool m_bChunked;

  // Cookie flags
  bool m_bUseCookiejar;
  enum { CookiesAuto, CookiesManual, CookiesNone } m_cookieMode;

  // Indicates whether there was some connection error.
  bool m_bError;

  // Indicates current request is a redirection
  bool m_bRedirect;

  // Indicates whether an error-page or error-msg should is preferred.
  bool m_bErrorPage;

  DCOPClient *m_dcopClient;

  // Previous and current response codes
  unsigned int m_responseCode;
  unsigned int m_prevResponseCode;

  // Values that determine the remote connection timeouts.
  int m_proxyConnTimeout;
  int m_remoteConnTimeout;
  int m_remoteRespTimeout;

  int m_pid;
};
#endif
