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

#include <kio_interface.h>
#include <kio_base.h>
#include <kurl.h>
#include <ksock.h>


// Default ports.. you might want to change this if you're trying to dodge
// a proxy with some creative network address translation..  HTTP_PORT
// is used for HTTP and WebDAV/WebFolders, HTTPS_PORT is well used for
// HTTPS. Duh.
#define DEFAULT_HTTP_PORT	80
#define DEFAULT_HTTPS_PORT	443

class HTTPIOJob;
class DCOPClient;


typedef struct
{
	KURL  url;
	int   postDataSize;
	bool  reload;
	unsigned long offset;
	short unsigned int port;
	bool  do_proxy;
	QString cef; // Cache Entry File belonging to this URL.
} HTTPState;

class HTTPProtocol : public KIOProtocol
{
public:
  HTTPProtocol( KIOConnection *_conn );
  virtual ~HTTPProtocol() { }

  enum HTTP_REV    {HTTP_Unknown, HTTP_10, HTTP_11};
  enum HTTP_AUTH   {AUTH_None, AUTH_Basic, AUTH_Digest};
  enum HTTP_PROTO  {PROTO_HTTP, PROTO_HTTPS, PROTO_WEBDAV};
  enum HTTP_METHOD {HTTP_GET, HTTP_PUT, HTTP_PROPFIND, HTTP_HEAD, HTTP_DELETE};

  virtual void slotGet( const char *_url );
  virtual void slotGetSize( const char *_url );
  virtual void slotPut( const char *_url, int _mode, bool _overwrite,
		                bool _resume, int _len );
  virtual void slotCopy( QStringList& _source, const char *_dest );
  virtual void slotCopy( const char *_source, const char *_dest );
  
  virtual void slotData(void *_p, int _len);
  virtual void slotDataEnd( HTTPIOJob *job = 0 );

  virtual bool error( int _err, const char *_txt );

  void jobError( int _errid, const char *_txt );
  
  KIOConnection* connection() { return KIOConnectionSignals::m_pConnection; }

protected:

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

  bool isValidProtocol (const char *);
  bool isValidProtocol (KURL *);

  void configAuth(const char *, bool);
#ifdef DO_SSL
  void initSSL();
#endif

  size_t sendData( HTTPIOJob *job = 0 );

  bool http_open( KURL &_url, int _post_data_len, bool _reload, unsigned long _offset = 0 );
  void http_close(); // Close transfer
  void http_closeConnection(); // Close conection

  void clearError() { m_iSavedError = 0; }
  void releaseError() {
    if ( m_iSavedError )
      KIOProtocol::error( m_iSavedError, m_strSavedError );
    m_iSavedError = 0;
  }

  bool readHeader();

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
  bool m_bEOF;
  int m_cmd;
  int m_sock;
  FILE* m_fsocket;
  HTTPState m_state;
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

  // Stuff to hold various error state information
  int m_iSavedError;
  QString m_strSavedError;
  bool m_bIgnoreJobErrors,
       m_bIgnoreErrors; 

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

class HTTPIOJob : public KIOJobBase
{
public:
  HTTPIOJob( KIOConnection *_conn, HTTPProtocol *_gzip );
  
  virtual void slotError( int _errid, const char *_txt );

protected:
  HTTPProtocol* m_pHTTP;
};

#endif
