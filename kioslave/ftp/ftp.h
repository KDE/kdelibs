// $Id$

#ifndef __ftp_h__
#define __ftp_h__

#include <config.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <ctype.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#include <unistd.h>
#include <sys/stat.h>

#define FTP_BUFSIZ 1024
#define ACCEPT_TIMEOUT 30

#include <string>
#include <qcstring.h>
#include <qstring.h>

class KURL;

struct FtpEntry
{
  QString name;
  QString owner;
  QString group;
  QString link;
  
  long size;
  mode_t type;
  mode_t access;
  time_t date;
};


class Ftp
{
private:
  /**
   * Connected to the socket from which we read a directory listing.
   * If it is not zero, then a "list" command is in progress.
   */
  FILE *dirfile;

  /**
   * This is the data connection socket from which we read the data.
   */
  int sData;
  /**
   * The control stream socket
   */
  int sControl;
  /**
   * The server socket for a data connection. This is needed since the
   * ftp server must open a connection to us.
   */
  int sDatal;

  int m_error;
  QString m_errorText;

  QString m_host;
  
  struct netbuf
  {
    char *cput,*cget;
    int handle;
    int cavail,cleft;
    char buf[FTP_BUFSIZ];
  };

  netbuf *nControl;
  char rspbuf[256];
  int ftplib_debug;

  bool m_bLoggedOn;
  bool m_bFtpStarted;
  bool m_bPasv;

  bool m_bPersistent;
  
  size_t m_size;
  size_t m_bytesLeft;

  int readline( char *buf, int max, netbuf *ctl );
  bool readresp(char c);

  /**
   * Connects to a ftp server, logs us in and returns a redirection
   * @ref #m_bLoggedOn is set to true if logging on was successfull. It is set to false if the connection
   * becomes closed.
   *
   * @param _path is set to the actual working directory. There is alway a '/' appeneded to the working directory.
   *
   * @return true on success.
   */
  bool ftpConnect( const char *_host, unsigned short int _port, const char *_user, const char *_pass, QString& _path );
  /**
   * Called by @ref #ftpConnect. It opens the control connection to the ftp server.
   *
   * @return true on success.
   */
  bool ftpConnect2( const char *host, unsigned short int _port = 0 );
  /**
   * Called by @ref #ftpConnect. It logs us in.
   *
   * @param redirect is set to the current working directory if logging on was successfull.
   *
   * @return true on success.
   */
  bool ftpLogin( const char *user, const char *pass, QString& redirect = (QString &)QString::null );

  bool ftpSendCmd( const QCString& cmd, char expresp );

  bool ftpChdir( const char *path );
  bool ftpSize( const char *path, char mode );
//   FtpEntry* ftpStat( const char *_path );
  
  /**
   * Runs a command on the ftp server like "list" or "retr". In contrast to
   * @ref #ftpSendCmd a data connection is opened. The corresponding socket
   * @ref #sData is available for reading/writing on success.
   * The connection must be closed afterwards with @ref #ftpCloseCommand.
   *
   * @param is 'A' or 'I'. 'A' means ASCII transfer, 'I' means binary transfer.
   *
   * @return true if the command was accepted by the server.
   */
  bool ftpOpenCommand( const char *command, const char *path, char mode, unsigned long offset = 0 );

  /**
   * Used by @ref #ftpOpenCommand
   */
  bool ftpOpenPASVDataConnection();
  /**
   * Used by @ref #ftpOpenCommand
   */
  bool ftpOpenDataConnection();
  /**
   * Used by @ref #ftpOpenCommand
   */
  int ftpAcceptConnect();
  /**
   * The counterpart to @ref #ftpOpenCommand.
   * 
   * @return true if the transmission was successfull.
   */
  bool ftpCloseCommand();

  FtpEntry* ftpParseDir( char* _info );

protected:
  virtual void redirection( const char* /* _url */ ) { }
  
public:
  enum Mode { READ = 1, WRITE = 2, OVERWRITE = 4 }; 

  Ftp();
  virtual ~Ftp();

  bool ftpConnect( KURL& _url );
  /**
   * The counterpart to @ref #ftpConnect
   */
  void ftpDisconnect( bool really = false );
 
  bool isConnected() { return m_bLoggedOn; }

  bool ftpOpenDir( KURL& _url );
  bool ftpCloseDir();
  FtpEntry* ftpStat( KURL& _url );

  bool ftpOpen( KURL& _url, Mode mode, unsigned long offset = 0 );
  bool ftpClose();
  bool ftpPort();

  bool ftpRmdir( const char *path );
  bool ftpMkdir( const char *path );
  bool ftpDelete( const char *fnm );
  bool ftpRename( const char *src, const char *dst );
  bool ftpChmod( const char *path, int mode );

  // this will send "rest offset" , use it before ftpOpen()
  bool ftpResume( unsigned long offset );

  bool opendir( KURL& _url );
  FtpEntry *readdir();
  bool closedir();

  FtpEntry* stat( KURL& _url );

  bool open( KURL& _url, Mode mode );
  bool close();
  size_t size();
  size_t read( void *buffer, long len );
  size_t write( void *buffer, long len );
  bool atEOF();
  
  bool mkdir( KURL& _url );

  int error() { return m_error; }
  QString errorText() { return m_errorText; }
};

#endif
