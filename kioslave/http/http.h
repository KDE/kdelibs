// $Id$

#ifndef __http_h__
#define __http_h__

#ifdef HAVE_LIBZ
#define DO_GZIP
#endif

#ifdef HAVE_SSL_H
#define DO_SSL
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>

#include <errno.h>
#include <stdio.h>
#ifdef DO_SSL
#include <openssl/ssl.h>
#endif
#include <unistd.h>
#include <netdb.h>

#include <qstack.h>
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

class HTTPIOJob;
class DCOPClient;


class HTTPProtocol : public KIO::SlaveBase
{
public:
  HTTPProtocol( KIO::Connection *_conn, const QCString &protocol );
  virtual ~HTTPProtocol() { }

  enum HTTP_REV    {HTTP_Unknown, HTTP_10, HTTP_11};
  enum HTTP_AUTH   {AUTH_None, AUTH_Basic, AUTH_Digest};
  enum HTTP_PROTO  {PROTO_HTTP, PROTO_HTTPS, PROTO_WEBDAV};
  enum HTTP_METHOD {HTTP_GET, HTTP_PUT, HTTP_POST, 
                    HTTP_PROPFIND, HTTP_HEAD, HTTP_DELETE};

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
	bool  reload;
	unsigned long offset;
	bool do_proxy;
	KURL url;
  } HTTPRequest;

  /** 
   * Fills in m_request.url from the rest of the request data.
   */
  void buildURL();

  /**
   * Opens a connection
   * @param host
   * @param port
   * @param user
   * @param pass
   * Called directly by createSlave, this is why there is no equivalent in
   * SlaveInterface, unlike the other methods.
   */
  virtual void openConnection(const QString& host, int port, const QString& user, const QString& pass);

  /**
   * Closes the connection (forced)
   */
  virtual void closeConnection();


  virtual void get( const QString& path, const QString& query, bool reload );
  virtual void put( const QString& path, int _mode,
			bool _overwrite, bool _resume );
  void post( const QString& path, const QString& query );

  /**
   * Special commands supported by this slave :
   * 1 - HTTP POST
   */
  virtual void special( const QByteArray &data);

  virtual void mimetype( const QString& path);

#if 0
  // TODO (replaces testDir and getSize)
  virtual void stat( const QString& path );
  virtual void del( const QString& path, bool isfile);
#endif

protected:

  void error( int _errid, const QString &_text );

  int readChunked(); // Read a chunk
  int readLimited(); // Read maximum m_iSize bytes.
  int readUnlimited(); // Read as much as possible.

  void decodeGzip();

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
  void addEncoding(QString, QStack<char> *);

  void configAuth(const char *, bool);
#ifdef DO_SSL
  void initSSL();
#endif

  size_t sendData();

  bool http_open();
  void http_close(); // Close transfer

  void http_openConnection(); // Open connection
  void http_closeConnection(); // Close conection

  bool readHeader();
  bool sendBody();
  bool readBody();

  /**
    * Return the proper UserAgent string.
    * Sure, I could make this configurable so
    * someone could tweak this to their heart's
    * delight, but right now, this should be
    * left alone, so if someone complains,
    * I"m getting weird errors, I can track
    * down, what version they have.
    */
  const char *getUserAgentString();

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
   * @param CEF the Cache Entry File.
   *
   * @return a file stream open for reading and at the start of
   *         the header section when the Cache entry exists and is valid.
   *         0 if no cache entry could be found, or if the enry is not
   *         valid (any more).
   */
  FILE *checkCacheEntry(QString &CEF);

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
   * Quick check whether the cache needs cleaning.
   */
  void cleanCache();

protected: // Members
  QCString m_protocol;
  HTTPState m_state;
  HTTPRequest m_request;

  bool m_bEOF;
  int m_cmd;
  int m_sock;
  FILE* m_fsocket;
  enum HTTP_REV m_HTTPrev;
  enum HTTP_PROTO m_proto;

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

  // Language/Encoding
  QStack<char> m_qTransferEncodings, m_qContentEncodings;
  QByteArray big_buffer;
  QString m_sContentMD5;
  QString m_strMimeType;
  QString m_strCharsets;
  QString m_strLanguages;
  
  // Proxy related members
  bool m_bUseProxy;  // Whether we want a proxy
  int m_strProxyPort;
  QString m_strNoProxyFor;
  QString m_strProxyHost;
  QString m_strProxyUser;
  QString m_strProxyPass;
  ksockaddr_in m_proxySockaddr;

  // Authentication
  QString m_strRealm, 
          m_strAuthString, 
          m_strProxyAuthString;
  enum HTTP_AUTH Authentication, ProxyAuthentication;

  // Persistant connections
  bool m_bKeepAlive;
  
  // Chunked tranfer encoding
  bool m_bChunked;

  bool m_bCanResume;

  DCOPClient *m_dcopClient;

#ifdef DO_SSL
  // Stuff for OpenSSL/SSLeay
  bool m_bUseSSL2, m_bUseSSL3, m_bUseTLS1, m_bUseSSL;
  SSL_METHOD *meth;
  SSL_CTX *ctx;
  SSL *hand;
#endif

};

#endif
