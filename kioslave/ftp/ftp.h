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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// $Id$

#ifndef __ftp_h__
#define __ftp_h__

#include <config.h>

#include <stdio.h>
#include <sys/types.h>

#define FTP_BUFSIZ 1024

#include <qcstring.h>
#include <qstring.h>

#include <kurl.h>
#include <kio/slavebase.h>

class KExtendedSocket;

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

struct netbuf
{
  char *cput,*cget;
  int handle;
  int cavail,cleft;
  char buf[FTP_BUFSIZ];
};

class Ftp : public KIO::SlaveBase
{
public:
  Ftp( const QCString &pool, const QCString &app );
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

  // unsupported, AFAIK
  // virtual void copy( const KURL &src, const KURL &dest, int permissions, bool overwrite );

private:

  // All the methods named ftpXyz do NOT emit errors, they simply return true
  // or false (they are lowlevel methods). The methods not named this way
  // emit error on error (they are highlevel methods).

  /**
   * Connect and login to the FTP server.
   *
   * If login is set to false, this function will not attempt
   * to login to the server.
   *
   * @param login if true send login info to the FTP server.
   */
  void ftpOpenConnection ( bool login = true );

  /**
   * Executes any auto login macro's as specified in a .netrc file.
   */
  void ftpAutoLoginMacro ();

  /**
   * Called by openConnection. It opens the control connection to the ftp server.
   *
   * @return true on success.
   */
  bool connect( const QString & host, unsigned short int port = 0 );

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
  bool ftpSendCmd( const QCString& cmd, int maxretries = 1 );

  /**
   * Use the SIZE command to get the file size.
   * @param mode the size depends on the transfer mode, hence this arg.
   * @return true on success
   * Gets the size into m_size.
   */
  bool ftpSize( const QString & path, char mode );

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

  void closeSockets();

  //void ftpAbortTransfer();

  /**
   * Used by openCommand
   */
  bool ftpOpenPASVDataConnection();
  /**
   * Used by openCommand
   */
  bool ftpOpenEPSVDataConnection();
  /**
   * Used by openCommand
   */
  bool ftpOpenEPRTDataConnection();
  /**
   * Used by openCommand
   */
  bool ftpOpenDataConnection();
  /**
   * ftpAcceptConnect - wait for incoming connection
   *
   * return -2 on error or timeout
   * otherwise returns socket descriptor
   */
  int ftpAcceptConnect();

  KIO::filesize_t ftpRead( void *buffer, long len );
  KIO::filesize_t ftpWrite( void *buffer, long len );

  bool ftpChmod( const QString & path, int permissions );

  // used by listDir
  bool ftpOpenDir( const QString & path );
  bool ftpCloseDir();
  // return 0L on end
  FtpEntry * ftpReadDir();
  // used by ftpReadDir
  FtpEntry * ftpParseDir( char* buffer );
  void createUDSEntry( const QString & filename, FtpEntry * e, KIO::UDSEntry & entry, bool isDir );
  void shortStatAnswer( const QString& filename, bool isDir );
  void statAnswerNotFound( const QString & path, const QString & filename );

  bool ftpRename( const QString & src, const QString & dst, bool overwrite );

  /**
   * read a line of text
   *
   * return -1 on error, bytecount otherwise
   */
  int ftpReadline( char *buf, int max, netbuf *ctl );

  /**
   * read a response from the server, into rspbuf
   * @return first char of response (rspbuf[0]), '\0' if we couldn't read the response
   */
  char readresp();

private: // data members

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

  QString m_host;
  unsigned short int m_port;
  QString m_user;
  QString m_pass;
  /**
   * Where we end up after connecting
   */
  QString m_initialPath;
  KURL m_proxyURL;

  netbuf *nControl;
  char rspbuf[256];

  bool m_bLoggedOn;
  bool m_bFtpStarted;
  bool m_bPasv;
  bool m_bUseProxy;
  bool m_bPersistent;

  KIO::filesize_t m_size;
  static KIO::filesize_t UnknownSize;

  enum
  {
    epsvUnknown = 0x01,
    epsvAllUnknown = 0x02,
    eprtUnknown = 0x04,
    epsvAllSent = 0x10,
    pasvUnknown = 0x20
  };
  int m_extControl;
  KExtendedSocket *ksControl;

};

#endif
