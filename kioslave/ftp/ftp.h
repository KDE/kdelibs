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
#define ACCEPT_TIMEOUT 30

#include <qcstring.h>
#include <qstring.h>
#include <kio/slavebase.h>

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
  Ftp( KIO::Connection *connection = 0 );
  virtual ~Ftp();

  /**
   * Connects to a ftp server and logs us in
   * @ref m_bLoggedOn is set to true if logging on was successfull.
   * It is set to false if the connection becomes closed.
   *
  // * @return true on success.
   */
  virtual void openConnection( const QString& host, int port, const QString& user, const QString& pass );

  /**
   * Closes the connection
   */
  virtual void closeConnection();

  virtual void stat( const QString & path );

  virtual void listDir( const QString & path );
  virtual void mkdir( const QString &  path, int permissions );
  virtual void rename( const QString & src, const QString & dst, bool overwrite );
  virtual void del( const QString & path, bool isfile );
  virtual void chmod( const QString & path, int permissions );

  virtual void get( const QString& path, const QString& query, bool reload );
  virtual void put( const QString& path, int permissions, bool overwrite, bool resume);

  // unsupported, AFAIK
  // virtual void copy( const QString &src, const QString &dest, int permissions, bool overwrite );

private:

  // All the methods named ftpXyz do NOT emit errors, they simply return true
  // or false (they are lowlevel methods). The methods not named this way
  // emit error on error (they are highlevel methods).

  /**
   * Called by @ref openConnection. It opens the control connection to the ftp server.
   *
   * @return true on success.
   */
  bool connect( const QString & host, unsigned short int port = 0 );

  /**
   * The counterpart to @ref connect
   * Does nothing if persistent connections are on
   * except if @p really is true.
   */
  void disconnect( bool really = false );

  bool open( const QString & path, unsigned long offset = 0 );

  bool close();

protected:
 // virtual void redirection( const char* /* _url */ ) { }


  /**
   * Called by @ref openConnection. It logs us in.
   * @ref m_redirect is set to the current working directory
   * if logging on was successfull.
   *
   * @return true on success.
   */
  bool ftpLogin( const QString & user, const QString & pass );

  /**
   * ftpSendCmd - send a command (@p cmd) and wait for expected response
   *
   * @param expresp the expected first char. '\0' for no check
   *
   * return true if proper response received, false on error
   * or if @p expresp doesn't match
   */
  bool ftpSendCmd( const QCString& cmd, char expresp = '\0' );

  /**
   * Get information about a file or directory, from its URL
   * @param _url full URL
   * @param bFullDetails if true, a complete FtpEntry is returned.
   * If false, the functions returns as soon as it knows whether it's
   * a file or a directory. Don't use the other fields in this case !
   * This is because getting the full details, on a directory, is a long
   * operation (involves listing the parent dir). For a file, it's fast, though.
   */
  //FtpEntry* ftpStat( const QString & path /*, bool bFullDetails*/ );

  /**
   * Use the SIZE command to get the file size.
   * @param mode the size depends on the transfer mode, hence this arg.
   * @return true on success
   * Gets the size into m_size.
   */
  bool ftpSize( const QString & path, char mode );

  /**
   * Runs a command on the ftp server like "list" or "retr". In contrast to
   * @ref ftpSendCmd a data connection is opened. The corresponding socket
   * @ref sData is available for reading/writing on success.
   * The connection must be closed afterwards with @ref ftpCloseCommand.
   *
   * @param mode is 'A' or 'I'. 'A' means ASCII transfer, 'I' means binary transfer.
   * @param errorcode the command-dependent error code to emit on error
   *
   * @return true if the command was accepted by the server.
   */
  bool openCommand( const char *command, const QString & path, char mode,
                    int errorcode, unsigned long offset = 0 );

private:

  /**
   * Used by @ref openCommand
   */
  bool ftpOpenPASVDataConnection();
  /**
   * Used by @ref openCommand
   */
  bool ftpOpenDataConnection();
  /**
   * ftpAcceptConnect - wait for incoming connection
   *
   * return -2 on error or timeout
   * otherwise returns socket descriptor
   */
  int ftpAcceptConnect();
  /**
   * The counterpart to @ref openCommand.
   * Closes data sockets and then reads line sent by server at
   * end of command.
   * @return false on error (line doesn't start with '2')
   */
  bool ftpCloseCommand();

  size_t ftpRead( void *buffer, long len );
  size_t ftpWrite( void *buffer, long len );

  // used by listDir
  bool ftpOpenDir( const QString & path );
  bool ftpCloseDir();
  // return 0L on end
  FtpEntry * ftpReadDir();
  // used by ftpReadDir
  FtpEntry * ftpParseDir( char* buffer );
  void createUDSEntry( FtpEntry * e, KIO::UDSEntry & entry );

  bool ftpRename( const QString & src, const QString & dst, bool overwrite );

  /**
   * read a line of text
   *
   * return -1 on error, bytecount otherwise
   */
  int ftpReadline( char *buf, int max, netbuf *ctl );

  /**
   * read a response from the server
   *
   * @param expresp expected response ('\0' for disabling builtin check)
   * @return false if first char doesn't match @p expresp
   *  true if first char matches @p expresp
   */
  bool readresp( char expresp );

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
  QString m_user;
  /**
   * Where we end up after connecting
   */
  QString m_redirect;

  netbuf *nControl;
  char rspbuf[256];

  bool m_bLoggedOn;
  bool m_bFtpStarted;
  bool m_bPasv;

  bool m_bPersistent;

  size_t m_size;
  size_t m_bytesLeft;

};

#endif
