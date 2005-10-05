// -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 2; -*-
/*  This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>

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

#ifndef __ftp_h__
#define __ftp_h__

#include <config.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <QString>
#include <QByteArray>

#include <kurl.h>
#include <kio/slavebase.h>
#include <kstreamsocket.h>
#include <kserversocket.h>

struct FtpEntry
{
  QString name;
  QString owner;
  QString group;
  QString link;

  KIO::filesize_t size;
  mode_t type;
  mode_t access;
  time_t date;
};

//===============================================================================
// Ftp
//===============================================================================
class Ftp : public KIO::SlaveBase
{
  // Ftp()	{}

public:
  Ftp( const QByteArray &pool, const QByteArray &app );
  virtual ~Ftp();

  virtual void setHost( const QString& host, int port, const QString& user, const QString& pass );

  /**
   * Connects to a ftp server and logs us in
   * m_bLoggedOn is set to true if logging on was successful.
   * It is set to false if the connection becomes closed.
   *
   */
  virtual void openConnection();

  /**
   * Closes the connection
   */
  virtual void closeConnection();

  virtual void stat( const KURL &url );

  virtual void listDir( const KURL & url );
  virtual void mkdir( const KURL & url, int permissions );
  virtual void rename( const KURL & src, const KURL & dst, bool overwrite );
  virtual void del( const KURL & url, bool isfile );
  virtual void chmod( const KURL & url, int permissions );

  virtual void get( const KURL& url );
  virtual void put( const KURL& url, int permissions, bool overwrite, bool resume);
  //virtual void mimetype( const KURL& url );

  virtual void slave_status();

  /**
   * Handles the case that one side of the job is a local file
   */
  virtual void copy( const KURL &src, const KURL &dest, int permissions, bool overwrite );

private:
  // ------------------------------------------------------------------------
  // All the methods named ftpXyz are lowlevel methods that are not exported.
  // The implement functionality used by the public high-level methods. Some
  // low-level methods still use error() to emit errors. This behaviour is not
  // recommended - please return a boolean status or an error code instead!
  // ------------------------------------------------------------------------

  /**
   * Status Code returned from ftpPut() and ftpGet(), used to select
   * source or destination url for error messages
   */
  typedef enum {
    statusSuccess,
    statusClientError,
    statusServerError
  } StatusCode;

  /**
   * Login Mode for ftpOpenConnection
   */
  typedef enum {
    loginDefered,
    loginExplicit,
    loginImplicit
  } LoginMode;

  /**
   * Connect and login to the FTP server.
   *
   * @param loginMode controls if login info should be sent<br>
   *  loginDefered  - must not be logged on, no login info is sent<br>
   *  loginExplicit - must not be logged on, login info is sent<br>
   *  loginImplicit - login info is sent if not logged on
   *
   * @return true on success (a login failure would return false).
   */
  bool ftpOpenConnection (LoginMode loginMode);

  /**
   * Executes any auto login macro's as specified in a .netrc file.
   */
  void ftpAutoLoginMacro ();

  /**
   * Called by openConnection. It logs us in.
   * m_initialPath is set to the current working directory
   * if logging on was successful.
   *
   * @return true on success.
   */
  bool ftpLogin();

  /**
   * ftpSendCmd - send a command (@p cmd) and read response
   *
   * @param maxretries number of time it should retry. Since it recursively
   * calls itself if it can't read the answer (this happens especially after
   * timeouts), we need to limit the recursiveness ;-)
   *
   * return true if any response received, false on error
   */
  bool ftpSendCmd( const QByteArray& cmd, int maxretries = 1 );

  /**
   * Use the SIZE command to get the file size.
   * @param mode the size depends on the transfer mode, hence this arg.
   * @return true on success
   * Gets the size into m_size.
   */
  bool ftpSize( const QString & path, char mode );

  /**
   * Set the current working directory, but only if not yet current
   */
  bool ftpFolder(const QString& path, bool bReportError);

  /**
   * Runs a command on the ftp server like "list" or "retr". In contrast to
   * ftpSendCmd a data connection is opened. The corresponding socket
   * sData is available for reading/writing on success.
   * The connection must be closed afterwards with ftpCloseCommand.
   *
   * @param mode is 'A' or 'I'. 'A' means ASCII transfer, 'I' means binary transfer.
   * @param errorcode the command-dependent error code to emit on error
   *
   * @return true if the command was accepted by the server.
   */
  bool ftpOpenCommand( const char *command, const QString & path, char mode,
                       int errorcode, KIO::fileoffset_t offset = 0 );

  /**
   * The counterpart to openCommand.
   * Closes data sockets and then reads line sent by server at
   * end of command.
   * @return false on error (line doesn't start with '2')
   */
  bool ftpCloseCommand();

  /**
   * Send "TYPE I" or "TYPE A" only if required, see m_cDataMode.
   *
   * Use 'A' to select ASCII and 'I' to select BINARY mode.  If
   * cMode is '?' the m_bTextMode flag is used to choose a mode.
   */
  bool ftpDataMode(char cMode);

  //void ftpAbortTransfer();

  /**
   * Used by ftpOpenCommand, return 0 on success or an error code
   */
  int ftpOpenDataConnection();

  /**
   * closes a data connection, see ftpOpenDataConnection()
   */
  void ftpCloseDataConnection();

  /**
   * Helper for ftpOpenDataConnection
   */
  int ftpOpenPASVDataConnection();
  /**
   * Helper for ftpOpenDataConnection
   */
  int ftpOpenEPSVDataConnection();
  /**
   * Helper for ftpOpenDataConnection
   */
  int ftpOpenEPRTDataConnection();
  /**
   * Helper for ftpOpenDataConnection
   */
  int ftpOpenPortDataConnection();

  /**
   * ftpAcceptConnect - wait for incoming connection
   *
   * return -2 on error or timeout
   * otherwise returns socket descriptor
   */
  int ftpAcceptConnect();

  bool ftpChmod( const QString & path, int permissions );

  // used by listDir
  bool ftpOpenDir( const QString & path );
  /**
    * Called to parse directory listings, call this until it returns false
    */
  bool ftpReadDir(FtpEntry& ftpEnt);

  /**
    * Helper to fill an UDSEntry
    */
  void ftpCreateUDSEntry( const QString & filename, FtpEntry& ftpEnt, KIO::UDSEntry& entry, bool isDir );

  void ftpShortStatAnswer( const QString& filename, bool isDir );

  void ftpStatAnswerNotFound( const QString & path, const QString & filename );

  /**
   * This is the internal implementation of rename() - set put().
   *
   * @return true on success.
   */
  bool ftpRename( const QString & src, const QString & dst, bool overwrite );

  /**
   * Called by openConnection. It opens the control connection to the ftp server.
   *
   * @return true on success.
   */
  bool ftpOpenControlConnection( const QString & host, unsigned short int port );

  /**
   * closes the socket holding the control connection (see ftpOpenControlConnection)
   */
  void ftpCloseControlConnection();

  /**
   * read a response from the server (a trailing CR gets stripped)
   * @param iOffset -1 to read a new line from the server<br>
   *                 0 to return the whole response string
   *                >0 to return the response with iOffset chars skipped
   * @return the reponse message with iOffset chars skipped (or "" if iOffset points
   *         behind the available data)
   */
  const char* ftpResponse(int iOffset);

  /**
   * This is the internal implementation of get() - see copy().
   *
   * IMPORTANT: the caller should call ftpCloseCommand() on return.
   * The function does not call error(), the caller should do this.
   *
   * @param iError      set to an ERR_xxxx code on error
   * @param iCopyFile   -1 -or- handle of a local destination file
   * @param hCopyOffset local file only: non-zero for resume
   * @return 0 for success, -1 for server error, -2 for client error
   */
  StatusCode ftpGet(int& iError, int iCopyFile, const KURL& url, KIO::fileoffset_t hCopyOffset);

  /**
   * This is the internal implementation of put() - see copy().
   *
   * IMPORTANT: the caller should call ftpCloseCommand() on return.
   * The function does not call error(), the caller should do this.
   *
   * @param iError      set to an ERR_xxxx code on error
   * @param iCopyFile   -1 -or- handle of a local source file
   * @return 0 for success, -1 for server error, -2 for client error
   */
  StatusCode ftpPut(int& iError, int iCopyFile, const KURL& url, int permissions, bool overwrite, bool resume);

  /**
   * helper called from copy() to implement FILE -> FTP transfers
   *
   * @param iError      set to an ERR_xxxx code on error
   * @param iCopyFile   [out] handle of a local source file
   * @param sCopyFile   path of the local source file
   * @return 0 for success, -1 for server error, -2 for client error
   */
  StatusCode ftpCopyPut(int& iError, int& iCopyFile, QString sCopyFile, const KURL& url, int permissions, bool overwrite);

  /**
   * helper called from copy() to implement FTP -> FILE transfers
   *
   * @param iError      set to an ERR_xxxx code on error
   * @param iCopyFile   [out] handle of a local source file
   * @param sCopyFile   path of the local destination file
   * @return 0 for success, -1 for server error, -2 for client error
   */
  StatusCode ftpCopyGet(int& iError, int& iCopyFile, QString sCopyFile, const KURL& url, int permissions, bool overwrite);

private: // data members

  QString m_host;
  unsigned short int m_port;
  QString m_user;
  QString m_pass;
  /**
   * Where we end up after connecting
   */
  QString m_initialPath;
  KURL m_proxyURL;

 /**
   * the current working directory - see ftpFolder
   */
  QString m_currentPath;

  /**
   * the status returned by the FTP protocol, set in ftpResponse()
   */
  int  m_iRespCode;

  /**
   * the status/100 returned by the FTP protocol, set in ftpResponse()
   */
  int  m_iRespType;

  /**
   * This flag is maintained by ftpDataMode() and contains I or A after
   * ftpDataMode() has successfully set the mode.
   */
  char m_cDataMode;

  /**
   * true if logged on (m_control should also be non-NULL)
   */
  bool m_bLoggedOn;

  /**
   * true if a "textmode" metadata key was found by ftpLogin(). This
   * switches the ftp data transfer mode from binary to ASCII.
   */
  bool m_bTextMode;

  /**
   * true if a data stream is open, used in closeConnection().
   *
   * When the user cancels a get or put command the Ftp dtor will be called,
   * which in turn calls closeConnection(). The later would try to send QUIT
   * which won't work until timeout. ftpOpenCommand sets the m_bBusy flag so
   * that the sockets will be closed immedeately - the server should be
   * capable of handling this and return an error code on thru the control
   * connection. The m_bBusy gets cleared by the ftpCloseCommand() routine.
   */
  bool m_bBusy;

  bool m_bPasv;
  bool m_bUseProxy;

  KIO::filesize_t m_size;
  static KIO::filesize_t UnknownSize;

  enum
  {
    epsvUnknown = 0x01,
    epsvAllUnknown = 0x02,
    eprtUnknown = 0x04,
    epsvAllSent = 0x10,
    pasvUnknown = 0x20,
    chmodUnknown = 0x100
  };
  int m_extControl;

  /**
   * control connection socket, only set if openControl() succeeded
   */
  KNetwork::KStreamSocket  *m_control;

  /**
   * data connection socket
   */
  KNetwork::KStreamSocket  *m_data;
};

#endif
