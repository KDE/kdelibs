// -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 2; c-file-style: "stroustrup" -*-
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

    Recommended reading explaining FTP details and quirks:
      http://cr.yp.to/ftp.html  (by D.J. Bernstein)
*/


#define  KIO_FTP_PRIVATE_INCLUDE
#include "ftp.h"

#include <sys/stat.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#include <netinet/in.h>
#include <arpa/inet.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#if TIME_WITH_SYS_TIME
#include <time.h>
#endif

#include <qdir.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kinstance.h>
#include <kmimemagic.h>
#include <kmimetype.h>
#include <kio/ioslave_defaults.h>
#include <kio/slaveconfig.h>
#include <kremoteencoding.h>
#include <kde_file.h>

#ifdef HAVE_STRTOLL
  #define charToLongLong(a) strtoll(a, 0, 10)
#else
  #define charToLongLong(a) strtol(a, 0, 10)
#endif

#define FTP_LOGIN   "anonymous"
#define FTP_PASSWD  "anonymous@"

//#undef  kdDebug
#define ENABLE_CAN_RESUME

// JPF: somebody should find a better solution for this or move this to KIO
// JPF: anyhow, in KDE 3.2.0 I found diffent MAX_IPC_SIZE definitions!
namespace KIO {
    enum buffersizes
    {  /**
        * largest buffer size that should be used to transfer data between
        * KIO slaves using the data() function
        */
        maximumIpcSize = 32 * 1024,
        /**
         * this is a reasonable value for an initial read() that a KIO slave
         * can do to obtain data via a slow network connection.
         */
        initialIpcSize =  2 * 1024,
        /**
         * recommended size of a data block passed to findBufferFileType()
         */
        mimimumMimeSize =     1024
    };

    // JPF: this helper was derived from write_all in file.cc (FileProtocol).
    static // JPF: in ftp.cc we make it static
    /**
     * This helper handles some special issues (blocking and interrupted
     * system call) when writing to a file handle.
     *
     * @return 0 on success or an error code on failure (ERR_COULD_NOT_WRITE,
     * ERR_DISK_FULL, ERR_CONNECTION_BROKEN).
     */
   int WriteToFile(int fd, const char *buf, size_t len)
   {
      while (len > 0)
      {  // JPF: shouldn't there be a KDE_write?
         ssize_t written = write(fd, buf, len);
         if (written >= 0)
         {   buf += written;
             len -= written;
             continue;
         }
         switch(errno)
         {   case EINTR:   continue;
             case EPIPE:   return ERR_CONNECTION_BROKEN;
             case ENOSPC:  return ERR_DISK_FULL;
             default:      return ERR_COULD_NOT_WRITE;
         }
      }
      return 0;
   }
}

KIO::filesize_t Ftp::UnknownSize = (KIO::filesize_t)-1;

using namespace KIO;
using namespace KNetwork;

extern "C" { KDE_EXPORT int kdemain(int argc, char **argv); }

int kdemain( int argc, char **argv )
{
  KLocale::setMainCatalog("kdelibs");
  KInstance instance( "kio_ftp" );
  ( void ) KGlobal::locale();

  kdDebug(7102) << "Starting " << getpid() << endl;

  if (argc != 4)
  {
     fprintf(stderr, "Usage: kio_ftp protocol domain-socket1 domain-socket2\n");
     exit(-1);
  }

  Ftp slave(argv[2], argv[3]);
  slave.dispatchLoop();

  kdDebug(7102) << "Done" << endl;
  return 0;
}

#warning "FIXME: Make it possible to set KeepAlive and Linger"
#if 0
int FtpSocket::connectSocket(const QString& host, const QString& port,
                             int iTimeOutSec, bool bControl)
{
  closeSocket();

  m_socket.setTimeout(iTimeOutSec * 1000);
  m_socket.setResolutionEnabled(bControl);
  m_socket.setAddressReuseable(true);
  m_socket.setBlocking(true);

  if(!m_socket.connect(host, port))
  {
    int iErrorCode = ERR_COULD_NOT_CONNECT;
    if (m_socket.error() == KStreamSocket::LookupFailure)
      iErrorCode = ERR_HOST_NOT_FOUND;
    kdError(7102) << m_pszName << ": connect failed (code " << m_socket.error()
                  << "): " << m_socket.errorString() << endl;
    return iErrorCode;
  }

  if(!bControl)
  { int on=1;
    if( !setSocketOption(SO_KEEPALIVE, (char *)&on, sizeof(on)) )
      errorMessage(0, "Keepalive not allowed");

    struct linger lng = { 1, 120 };
    if( !setSocketOption(SO_LINGER, (char *)&lng, sizeof (lng)) )
      errorMessage(0, "Linger mode was not allowed.");
  }

  debugMessage("connected");
  return 0;
}
#endif

//===============================================================================
// Ftp
//===============================================================================

Ftp::Ftp( const QByteArray &pool, const QByteArray &app )
    : SlaveBase( "ftp", pool, app )
{
  // init the socket data
  m_data = m_control = NULL;
  ftpCloseControlConnection();

  // init other members
  m_port = 0;
  kdDebug(7102) << "Ftp::Ftp()" << endl;
}


Ftp::~Ftp()
{
  kdDebug(7102) << "Ftp::~Ftp()" << endl;
  closeConnection();
}

/**
 * This closes a data connection opened by ftpOpenDataConnection().
 */
void Ftp::ftpCloseDataConnection()
{
  delete m_data;
  m_data = NULL;
}

/**
 * This closes a control connection opened by ftpOpenControlConnection() and reinits the
 * related states.  This method gets called from the constructor with m_control = NULL.
 */
void Ftp::ftpCloseControlConnection()
{
  m_extControl = 0;
  delete m_control;
  m_control = NULL;
  m_cDataMode = 0;
  m_bLoggedOn = false;    // logon needs control connction
  m_bTextMode = false;
  m_bBusy = false;
}

/**
 * Returns the last response from the server (iOffset >= 0)  -or-  reads a new response
 * (iOffset < 0). The result is returned (with iOffset chars skipped for iOffset > 0).
 */
const char* Ftp::ftpResponse(int iOffset)
{
  assert(m_control != NULL);    // must have control connection socket
  QByteArray data = m_control->readLine();
  const char *pTxt = data.data();

  // read the next line ...
  if(iOffset < 0)
  {
    int  iMore = 0;
    m_iRespCode = 0;

    // If the server sends multiline responses "nnn-text" we loop here until
    // a final "nnn text" line is reached. Only data from the final line will
    // be stored. Some servers (OpenBSD) send a single "nnn-" followed by
    // optional lines that start with a space and a final "nnn text" line.
    do {
      int nBytes = data.size();
      int iCode  = atoi(pTxt);
      if(iCode > 0) m_iRespCode = iCode;

      // ignore lines starting with a space in multiline response
      if(iMore != 0 && pTxt[0] == 32)
        ;
      // otherwise the line should start with "nnn-" or "nnn "
      else if(nBytes < 4 || iCode < 100)
        iMore = 0;
      // we got a valid line, now check for multiline responses ...
      else if(iMore == 0 && pTxt[3] == '-')
        iMore = iCode;
      // "nnn " ends multiline mode ...
      else if(iMore != 0 && (iMore != iCode || pTxt[3] != '-'))
        iMore = 0;

      if(iMore != 0)
         kdDebug(7102) << "    > " << pTxt << endl;
    } while(iMore != 0);
    kdDebug(7102) << "resp> " << pTxt << endl;

    m_iRespType = (m_iRespCode > 0) ? m_iRespCode / 100 : 0;
  }

  // return text with offset ...
  while(iOffset-- > 0 && pTxt[0])
    pTxt++;
  return pTxt;
}


void Ftp::closeConnection()
{
  if(m_control != NULL || m_data != NULL)
    kdDebug(7102) << "Ftp::closeConnection m_bLoggedOn=" << m_bLoggedOn << " m_bBusy=" << m_bBusy << endl;

  if(m_bBusy)              // ftpCloseCommand not called
  {
    kdWarning(7102) << "Ftp::closeConnection Abandoned data stream" << endl;
    ftpCloseDataConnection();
  }

  if(m_bLoggedOn)           // send quit
  {
    if( !ftpSendCmd( "quit", 0 ) || (m_iRespType != 2) )
      kdWarning(7102) << "Ftp::closeConnection QUIT returned error: " << m_iRespCode << endl;
  }

  // close the data and control connections ...
  ftpCloseDataConnection();
  ftpCloseControlConnection();
}

void Ftp::setHost( const QString& _host, int _port, const QString& _user,
                   const QString& _pass )
{
  kdDebug(7102) << "Ftp::setHost (" << getpid() << "): " << _host << endl;

  m_proxyURL = metaData("UseProxy");
  m_bUseProxy = (m_proxyURL.isValid() && m_proxyURL.protocol() == "ftp");

  if ( m_host != _host || m_port != _port ||
       m_user != _user || m_pass != _pass )
    closeConnection();

  m_host = _host;
  m_port = _port;
  m_user = _user;
  m_pass = _pass;
}

void Ftp::openConnection()
{
  ftpOpenConnection(loginExplicit);
}

bool Ftp::ftpOpenConnection (LoginMode loginMode)
{
  // check for implicit login if we are already logged on ...
  if(loginMode == loginImplicit && m_bLoggedOn)
  {
    assert(m_control != NULL);    // must have control connection socket
    return true;
  }

  kdDebug(7102) << "ftpOpenConnection " << m_host << ":" << m_port << " "
                << m_user << " [password hidden]" << endl;

  infoMessage( i18n("Opening connection to host %1").arg(m_host) );

  if ( m_host.isEmpty() )
  {
    error( ERR_UNKNOWN_HOST, QString::null );
    return false;
  }

  assert( !m_bLoggedOn );

  m_initialPath = QString::null;
  m_currentPath = QString::null;

  QString host = m_bUseProxy ? m_proxyURL.host() : m_host;
  unsigned short int port = m_bUseProxy ? m_proxyURL.port() : m_port;

  if (!ftpOpenControlConnection(host, port) )
    return false;          // error emitted by ftpOpenControlConnection
  infoMessage( i18n("Connected to host %1").arg(m_host) );

  if(loginMode != loginDefered)
  {
    m_bLoggedOn = ftpLogin();
    if( !m_bLoggedOn )
      return false;       // error emitted by ftpLogin
  }

  m_bTextMode = config()->readBoolEntry("textmode", false);
  connected();
  return true;
}


/**
 * Called by @ref openConnection. It opens the control connection to the ftp server.
 *
 * @return true on success.
 */
bool Ftp::ftpOpenControlConnection( const QString &host, unsigned short int port )
{
  QString serv;
  if ( port != 0 )
    serv = QString::number( port );
  else
    serv = QLatin1String( "ftp" );

  // implicitly close, then try to open a new connection ...
  closeConnection();
  QString sErrorMsg;
  m_control = new KStreamSocket;

#warning "FIXME KDE4: reenable 'host not found' error"
  // now connect to the server and read the login message ...
  m_control->setTimeout(connectTimeout() * 1000);
  int iErrorCode = m_control->connect(host, serv) ? ERR_COULD_NOT_CONNECT : 0;
  sErrorMsg = QString("%1: %2").arg(host).arg(m_control->errorString());

  // on connect success try to read the server message...
  if(iErrorCode == 0)
  {
    const char* psz = ftpResponse(-1);
    if(m_iRespType != 2)
    { // login not successful, do we have an message text?
      if(psz[0])
        sErrorMsg = i18n("%1.\n\nReason: %2").arg(host).arg(psz);
      iErrorCode = ERR_COULD_NOT_CONNECT;
    }
  }

  // if there was a problem - report it ...
  if(iErrorCode == 0)             // OK, return success
    return true;
  closeConnection();              // clean-up on error
  error(iErrorCode, sErrorMsg);
  return false;
}

/**
 * Called by @ref openConnection. It logs us in.
 * @ref m_initialPath is set to the current working directory
 * if logging on was successful.
 *
 * @return true on success.
 */
bool Ftp::ftpLogin()
{
  infoMessage( i18n("Sending login information") );

  assert( !m_bLoggedOn );

  QString user = m_user;
  QString pass = m_pass;

  if ( config()->readBoolEntry("EnableAutoLogin") )
  {
    QString au = config()->readEntry("autoLoginUser");
    if ( !au.isEmpty() )
    {
        user = au;
        pass = config()->readEntry("autoLoginPass");
    }
  }

  // Try anonymous login if both username/password
  // information is blank.
  if (user.isEmpty() && pass.isEmpty())
  {
    user = FTP_LOGIN;
    pass = FTP_PASSWD;
  }

  AuthInfo info;
  info.url.setProtocol( "ftp" );
  info.url.setHost( m_host );
  info.url.setPort( m_port );
  info.url.setUser( user );

  QByteArray tempbuf;
  int failedAuth = 0;

  do
  {
    // Check the cache and/or prompt user for password if 1st
    // login attempt failed OR the user supplied a login name,
    // but no password.
    if ( failedAuth > 0 || (!user.isEmpty() && pass.isEmpty()) )
    {
      QString errorMsg;
      kdDebug(7102) << "Prompting user for login info..." << endl;

      // Ask user if we should retry after when login fails!
      if( failedAuth > 0 )
      {
        errorMsg = i18n("Message sent:\nLogin using username=%1 and "
                        "password=[hidden]\n\nServer replied:\n%2\n\n"
                        ).arg(user).arg(ftpResponse(0));
      }

      if ( user != FTP_LOGIN )
        info.username = user;

      info.prompt = i18n("You need to supply a username and a password "
                          "to access this site.");
      info.commentLabel = i18n( "Site:" );
      info.comment = i18n("<b>%1</b>").arg( m_host );
      info.keepPassword = true; // Prompt the user for persistence as well.
      info.readOnly = (!m_user.isEmpty() && m_user != FTP_LOGIN);

      bool disablePassDlg = config()->readBoolEntry( "DisablePassDlg", false );
      if ( disablePassDlg || !openPassDlg( info, errorMsg ) )
      {
        error( ERR_USER_CANCELED, m_host );
        return false;
      }
      else
      {
        user = info.username;
        pass = info.password;
      }
    }

    tempbuf = "user ";
    tempbuf += user.toLatin1();
    if ( m_bUseProxy )
    {
      tempbuf += '@';
      tempbuf += m_host.toLatin1();
      if ( m_port > 0 && m_port != DEFAULT_FTP_PORT )
      {
        tempbuf += ':';
        tempbuf += QString::number(m_port).toLatin1();
      }
    }

    kdDebug(7102) << "Sending Login name: " << tempbuf << endl;

    bool loggedIn = ( ftpSendCmd(tempbuf) && (m_iRespCode == 230) );
    bool needPass = (m_iRespCode == 331);
    // Prompt user for login info if we do not
    // get back a "230" or "331".
    if ( !loggedIn && !needPass )
    {
      kdDebug(7102) << "Login failed: " << ftpResponse(0) << endl;
      ++failedAuth;
      continue;  // Well we failed, prompt the user please!!
    }

    if( needPass )
    {
      tempbuf = "pass ";
      tempbuf += pass.toLatin1();
      kdDebug(7102) << "Sending Login password: " << "[protected]" << endl;
      loggedIn = ( ftpSendCmd(tempbuf) && (m_iRespCode == 230) );
    }

    if ( loggedIn )
    {
      // Do not cache the default login!!
      if( user != FTP_LOGIN && pass != FTP_PASSWD )
        cacheAuthentication( info );
      failedAuth = -1;
    }

  } while( ++failedAuth );


  kdDebug(7102) << "Login OK" << endl;
  infoMessage( i18n("Login OK") );

  // Okay, we're logged in. If this is IIS 4, switch dir listing style to Unix:
  // Thanks to jk@soegaard.net (Jens Kristian Sgaard) for this hint
  if( ftpSendCmd("syst") && (m_iRespType == 2) )
  {
    if( !strncmp( ftpResponse(0), "215 Windows_NT", 14 ) ) // should do for any version
    {
      ftpSendCmd( "site dirstyle" );
      // Check if it was already in Unix style
      // Patch from Keith Refson <Keith.Refson@earth.ox.ac.uk>
      if( !strncmp( ftpResponse(0), "200 MSDOS-like directory output is on", 37 ))
         //It was in Unix style already!
         ftpSendCmd( "site dirstyle" );
      // windows won't support chmod before KDE konquers their desktop...
      m_extControl |= chmodUnknown;
    }
  }
  else
    kdWarning(7102) << "syst failed" << endl;

  if ( config()->readBoolEntry ("EnableAutoLoginMacro") )
    ftpAutoLoginMacro ();

  // Get the current working directory
  kdDebug(7102) << "Searching for pwd" << endl;
  if( !ftpSendCmd("pwd") || (m_iRespType != 2) )
  {
    kdDebug(7102) << "Couldn't issue pwd command" << endl;
    error( ERR_COULD_NOT_LOGIN, i18n("Could not login to %1.").arg(m_host) ); // or anything better ?
    return false;
  }

  QString sTmp = remoteEncoding()->decode( ftpResponse(3) );
  int iBeg = sTmp.indexOf('"');
  int iEnd = sTmp.lastIndexOf('"');
  if(iBeg > 0 && iBeg < iEnd)
  {
    m_initialPath = sTmp.mid(iBeg+1, iEnd-iBeg-1);
    if(m_initialPath[0] != '/') m_initialPath.prepend('/');
    kdDebug(7102) << "Initial path set to: " << m_initialPath << endl;
    m_currentPath = m_initialPath;
  }
  return true;
}

void Ftp::ftpAutoLoginMacro ()
{
  QString macro = metaData( "autoLoginMacro" );

  if ( macro.isEmpty() )
    return;

  QStringList list = macro.split('\n',QString::SkipEmptyParts);

  for(QStringList::Iterator it = list.begin() ; it != list.end() ; ++it )
  {
    if ( (*it).startsWith("init") )
    {
      list = macro.split( '\\',QString::SkipEmptyParts);
      it = list.begin();
      ++it;  // ignore the macro name

      for( ; it != list.end() ; ++it )
      {
        // TODO: Add support for arbitrary commands
        // besides simply changing directory!!
        if ( (*it).startsWith( "cwd" ) )
          ftpFolder( (*it).mid(4).trimmed(), false );
      }

      break;
    }
  }
}


/**
 * ftpSendCmd - send a command (@p cmd) and read response
 *
 * @param maxretries number of time it should retry. Since it recursively
 * calls itself if it can't read the answer (this happens especially after
 * timeouts), we need to limit the recursiveness ;-)
 *
 * return true if any response received, false on error
 */
bool Ftp::ftpSendCmd( const QByteArray& cmd, int maxretries )
{
  assert(m_control != NULL);    // must have control connection socket

  if ( cmd.indexOf( '\r' ) != -1 || cmd.indexOf( '\n' ) != -1)
  {
    kdWarning(7102) << "Invalid command received (contains CR or LF):"
                    << cmd.data() << endl;
    error( ERR_UNSUPPORTED_ACTION, m_host );
    return false;
  }

  // Don't print out the password...
  bool isPassCmd = (cmd.left(4).toLower() == "pass");
  if ( !isPassCmd )
    kdDebug(7102) << "send> " << cmd.data() << endl;
  else
    kdDebug(7102) << "send> pass [protected]" << endl;

  // Send the message...
  QByteArray buf = cmd;
  buf += "\r\n";      // Yes, must use CR/LF - see http://cr.yp.to/ftp/request.html
  int num = m_control->write(buf.data(), buf.length());

  // If we were able to successfully send the command, then we will
  // attempt to read the response. Otherwise, take action to re-attempt
  // the login based on the maximum number of retires specified...
  if( num > 0 )
    ftpResponse(-1);
  else
  {
    m_iRespType = m_iRespCode = 0;
  }

  // If respCh is NULL or the response is 421 (Timed-out), we try to re-send
  // the command based on the value of maxretries.
  if( (m_iRespType <= 0) || (m_iRespCode == 421) )
  {
    // We have not yet logged on...
    if (!m_bLoggedOn)
    {
      // The command was sent from the ftpLogin function, i.e. we are actually
      // attempting to login in. NOTE: If we already sent the username, we
      // return false and let the user decide whether (s)he wants to start from
      // the beginning...
      if (maxretries > 0 && !isPassCmd)
      {
        closeConnection ();
        if( ftpOpenConnection(loginDefered) )
          ftpSendCmd ( cmd, maxretries - 1 );
      }

      return false;
    }
    else
    {
      if ( maxretries < 1 )
        return false;
      else
      {
        kdDebug(7102) << "Was not able to communicate with " << m_host << endl
                      << "Attempting to re-establish connection." << endl;

        closeConnection(); // Close the old connection...
        openConnection();  // Attempt to re-establish a new connection...

        if (!m_bLoggedOn)
        {
          if (m_control != NULL)  // if openConnection succeeded ...
          {
            kdDebug(7102) << "Login failure, aborting" << endl;
            error (ERR_COULD_NOT_LOGIN, m_host);
            closeConnection ();
          }
          return false;
        }

        kdDebug(7102) << "Logged back in, re-issuing command" << endl;

        // If we were able to login, resend the command...
        if (maxretries)
          maxretries--;

        return ftpSendCmd( cmd, maxretries );
      }
    }
  }

  return true;
}


/*
 * ftpOpenPASVDataConnection - set up data connection, using PASV mode
 *
 * return 1 if successful, 0 otherwise
 * doesn't set error message, since non-pasv mode will always be tried if
 * this one fails
 */
int Ftp::ftpOpenPASVDataConnection()
{
  assert(m_control != NULL);    // must have control connection socket
  assert(m_data == NULL);       // ... but no data connection

  // Check that we can do PASV
  const KSocketAddress &sa = m_control->peerAddress();
  if (sa.family() != PF_INET)
    return ERR_INTERNAL;       // no PASV for non-PF_INET connections

  if (m_extControl & pasvUnknown)
    return ERR_INTERNAL;       // already tried and got "unknown command"

  m_bPasv = true;

  /* Let's PASsiVe*/
  if( !ftpSendCmd("PASV") || (m_iRespType != 2) )
  {
    kdDebug(7102) << "PASV attempt failed" << endl;
    // unknown command?
    if( m_iRespType == 5 )
    {
        kdDebug(7102) << "disabling use of PASV" << endl;
        m_extControl |= pasvUnknown;
    }
    return ERR_INTERNAL;
  }

  // The usual answer is '227 Entering Passive Mode. (160,39,200,55,6,245)'
  // but anonftpd gives '227 =160,39,200,55,6,245'
  int i[6];
  char *start = strchr(ftpResponse(3), '(');
  if ( !start )
    start = strchr(ftpResponse(3), '=');
  if ( !start ||
       ( sscanf(start, "(%d,%d,%d,%d,%d,%d)",&i[0], &i[1], &i[2], &i[3], &i[4], &i[5]) != 6 &&
         sscanf(start, "=%d,%d,%d,%d,%d,%d", &i[0], &i[1], &i[2], &i[3], &i[4], &i[5]) != 6 ) )
  {
    kdError(7102) << "parsing IP and port numbers failed. String parsed: " << start << endl;
    return ERR_INTERNAL;
  }

  // Make hostname and port number ...
  KInetSocketAddress address((KIpAddress((i[0] << 24) | (i[1] << 16) |
                                         (i[2] << 8) | i[3]) ), i[4] << 8 | i[5]);

  // now connect the data socket ...
  m_data = new KStreamSocket("PASV");
  m_data->setResolutionEnabled(false);
  m_data->setTimeout(connectTimeout() * 1000);

  kdDebug(7102) << "Connecting to " << address.toString() << endl;
  return m_data->connect(address.nodeName(), address.serviceName());
}

/*
 * ftpOpenEPSVDataConnection - opens a data connection via EPSV
 */
int Ftp::ftpOpenEPSVDataConnection()
{
  assert(m_control != NULL);    // must have control connection socket
  assert(m_data == NULL);       // ... but no data connection

  const KInetSocketAddress& sa = m_control->peerAddress().asInet();
  int portnum;

  if (m_extControl & epsvUnknown)
    return ERR_INTERNAL;

  m_bPasv = true;
  if( !ftpSendCmd("EPSV") || (m_iRespType != 2) )
  {
    // unknown command?
    if( m_iRespType == 5 )
    {
       kdDebug(7102) << "disabling use of EPSV" << endl;
       m_extControl |= epsvUnknown;
    }
    return ERR_INTERNAL;
  }

  char *start = strchr(ftpResponse(3), '|');
  if ( !start || sscanf(start, "|||%d|", &portnum) != 1)
    return ERR_INTERNAL;

  m_data = new KStreamSocket;
  m_data->setResolutionEnabled(false);
  m_data->setTimeout(connectTimeout() * 1000);
  return m_data->connect(sa.nodeName(), QString::number(portnum));
}

/*
 * ftpOpenDataConnection - set up data connection
 *
 * The routine calls several ftpOpenXxxxConnection() helpers to find
 * the best connection mode. If a helper cannot connect if returns
 * ERR_INTERNAL - so this is not really an error! All other error
 * codes are treated as fatal, e.g. they are passed back to the caller
 * who is responsible for calling error(). ftpOpenPortDataConnection
 * can be called as last try and it does never return ERR_INTERNAL.
 *
 * @return 0 if successful, err code otherwise
 */
int Ftp::ftpOpenDataConnection()
{
  // make sure that we are logged on and have no data connection...
  assert( m_bLoggedOn );
  ftpCloseDataConnection();

  int  iErrCode = 0;
  int  iErrCodePASV = 0;  // Remember error code from PASV

  // First try passive (EPSV & PASV) modes
  if( !config()->readBoolEntry("DisablePassiveMode", false) )
  {
    iErrCode = ftpOpenPASVDataConnection();
    if(iErrCode == 0)
      return 0; // success
    iErrCodePASV = iErrCode;
    ftpCloseDataConnection();

    if( !config()->readBoolEntry("DisableEPSV", false) )
    {
      iErrCode = ftpOpenEPSVDataConnection();
      if(iErrCode == 0)
        return 0; // success
      ftpCloseDataConnection();
    }

    // if we sent EPSV ALL already and it was accepted, then we can't
    // use active connections any more
    if (m_extControl & epsvAllSent)
      return iErrCodePASV ? iErrCodePASV : iErrCode;
  }

  // fall back to port mode
  iErrCode = ftpOpenPortDataConnection();
  if(iErrCode == 0)
    return 0; // success

  ftpCloseDataConnection();
  // prefer to return the error code from PASV if any, since that's what should have worked in the first place
  return iErrCodePASV ? iErrCodePASV : iErrCode;
}

/*
 * ftpOpenPortDataConnection - set up data connection
 *
 * @return 0 if successfull, err code otherwise (but never ERR_INTERNAL
 *         because this is the last connection mode that is tried)
 */
int Ftp::ftpOpenPortDataConnection()
{
  assert(m_control != NULL);    // must have control connection socket
  assert(m_data == NULL);       // ... but no data connection

  m_bPasv = false;
  if (m_extControl & eprtUnknown)
    return ERR_INTERNAL;

  KServerSocket server;
  server.setAcceptBuffered(false);
  server.setResolutionEnabled(false);
  server.setFamily(KResolver::InternetFamily);
  server.setTimeout(connectTimeout() * 1000);

  {
    // yes, we are sure this is a KInetSocketAddress
    const KInetSocketAddress &sin = m_control->localAddress().asInet();

    // setting port to 0 will make us bind to a random, free port
    server.bind(sin.nodeName(), QLatin1String("0"));
  }

  if (!server.listen(1))
    return ERR_COULD_NOT_LISTEN;

  const KInetSocketAddress &sin = server.localAddress().asInet();

  QString command;
  if (sin.ianaFamily() == 1)
  {
    command = QLatin1String("PORT %1,%2,%3,%4,%5,%6");
    struct
    {
      quint32 ip4;
      quint16 port;
    } data;
    data.ip4 = sin.ipAddress().IPv4Addr();
    data.port = sin.port();

    char *pData = reinterpret_cast<char*>(&data);
    command.arg(pData[0]).arg(pData[1]).arg(pData[2]).arg(pData[3])
      .arg(pData[4]).arg(pData[5]);
  }
  else
  {
    command = QLatin1String("EPRT |%1|%2|%3|");
    command.arg(sin.ianaFamily())
      .arg(sin.nodeName())
      .arg(sin.port());
  }

  if( ftpSendCmd(command.toLatin1()) && (m_iRespType == 2) )
  {
    m_data = server.accept();
    return m_data ? 0 : ERR_COULD_NOT_CONNECT;
  }
  return ERR_INTERNAL;
}

bool Ftp::ftpOpenCommand( const char *_command, const QString & _path, char _mode,
                          int errorcode, KIO::fileoffset_t _offset )
{
  int errCode = 0;
  if( !ftpDataMode(_mode) )
    errCode = ERR_COULD_NOT_CONNECT;
  else
    errCode = ftpOpenDataConnection();

  if(errCode != 0)
  {
    error(errCode, m_host);
    return false;
  }

  if ( _offset > 0 ) {
    // send rest command if offset > 0, this applies to retr and stor commands
    char buf[100];
    sprintf(buf, "rest %lld", _offset);
    if ( !ftpSendCmd( buf ) )
       return false;
    if( m_iRespType != 3 )
    {
      error( ERR_CANNOT_RESUME, _path ); // should never happen
      return false;
    }
  }

  QByteArray tmp = _command;
  QString errormessage;

  if ( !_path.isEmpty() ) {
    tmp += " ";
    tmp += remoteEncoding()->encode(_path);
  }

  if( !ftpSendCmd( tmp ) || (m_iRespType != 1) )
  {
    if( _offset > 0 && strcmp(_command, "retr") == 0 && (m_iRespType == 4) )
      errorcode = ERR_CANNOT_RESUME;
    // The error here depends on the command
    errormessage = _path;
  }

  else
  {
    // Only now we know for sure that we can resume
    if ( _offset > 0 && strcmp(_command, "retr") == 0 )
      canResume();

    m_bBusy = true;              // cleared in ftpCloseCommand
    return true;
  }

  error(errorcode, errormessage);
  return false;
}


bool Ftp::ftpCloseCommand()
{
  // first close data sockets (if opened), then read response that
  // we got for whatever was used in ftpOpenCommand ( should be 226 )
  if(m_data)
  {
    delete  m_data;
    m_data = NULL;
  }
  if(!m_bBusy)
    return true;

  kdDebug(7102) << "ftpCloseCommand: reading command result" << endl;
  m_bBusy = false;

  if(ftpResponse(-1) <= 0 || (m_iRespType != 2) )
  {
    kdDebug(7102) << "ftpCloseCommand: no transfer complete message" << endl;
    return false;
  }
  return true;
}

void Ftp::mkdir( const KURL & url, int permissions )
{
  if( !ftpOpenConnection(loginImplicit) )
        return;

  QString path = remoteEncoding()->encode(url);
  QByteArray buf = "mkd ";
  buf += remoteEncoding()->encode(path);

  if( !ftpSendCmd( buf ) || (m_iRespType != 2) )
  {
    QString currentPath( m_currentPath );

    // Check whether or not mkdir failed because
    // the directory already exists...
    if( ftpFolder( path, false ) )
    {
      error( ERR_DIR_ALREADY_EXIST, path );
      // Change the directory back to what it was...
      (void) ftpFolder( currentPath, false );
      return;
    }

    error( ERR_COULD_NOT_MKDIR, path );
    return;
  }

  if ( permissions != -1 )
  {
    // chmod the dir we just created, ignoring errors.
    (void) ftpChmod( path, permissions );
  }

  finished();
}

void Ftp::rename( const KURL& src, const KURL& dst, bool overwrite )
{
  if( !ftpOpenConnection(loginImplicit) )
        return;

  // The actual functionality is in ftpRename because put needs it
  if ( ftpRename( src.path(), dst.path(), overwrite ) )
    finished();
  else
    error( ERR_CANNOT_RENAME, src.path() );
}

bool Ftp::ftpRename( const QString & src, const QString & dst, bool /* overwrite */ )
{
  // TODO honor overwrite
  assert( m_bLoggedOn );

  int pos = src.lastIndexOf("/");
  if( !ftpFolder(src.left(pos+1), false) )
      return false;

  QByteArray from_cmd = "RNFR ";
  from_cmd += remoteEncoding()->encode(src.mid(pos+1));
  if( !ftpSendCmd( from_cmd ) || (m_iRespType != 3) )
      return false;

  QByteArray to_cmd = "RNTO ";
  to_cmd += remoteEncoding()->encode(dst);
  if( !ftpSendCmd( to_cmd ) || (m_iRespType != 2) )
      return false;

  return true;
}

void Ftp::del( const KURL& url, bool isfile )
{
  if( !ftpOpenConnection(loginImplicit) )
        return;

  // When deleting a directory, we must exit from it first
  // The last command probably went into it (to stat it)
  if ( !isfile )
    ftpFolder(remoteEncoding()->directory(url), false); // ignore errors

  QByteArray cmd = isfile ? "DELE " : "RMD ";
  cmd += remoteEncoding()->encode(url);

  if( !ftpSendCmd( cmd ) || (m_iRespType != 2) )
    error( ERR_CANNOT_DELETE, url.path() );
  else
    finished();
}

bool Ftp::ftpChmod( const QString & path, int permissions )
{
  assert( m_bLoggedOn );

  if(m_extControl & chmodUnknown)      // previous errors?
    return false;

  // we need to do bit AND 777 to get permissions, in case
  // we were sent a full mode (unlikely)
  QString cmd = QString::fromLatin1("SITE CHMOD ") + QString::number( permissions & 511, 8 /*octal*/ ) + " ";
  cmd += path;

  ftpSendCmd(remoteEncoding()->encode(cmd));
  if(m_iRespType == 2)
     return true;

  if(m_iRespCode == 500)
  {
    m_extControl |= chmodUnknown;
    kdDebug(7102) << "ftpChmod: CHMOD not supported - disabling";
  }
  return false;
}

void Ftp::chmod( const KURL & url, int permissions )
{
  if( !ftpOpenConnection(loginImplicit) )
        return;

  if ( !ftpChmod( url.path(), permissions ) )
    error( ERR_CANNOT_CHMOD, url.path() );
  else
    finished();
}

void Ftp::ftpCreateUDSEntry( const QString & filename, FtpEntry& ftpEnt, UDSEntry& entry, bool isDir )
{
  assert(entry.count() == 0); // by contract :-)

  entry.insert( UDS_NAME, filename );
  entry.insert( UDS_SIZE, ftpEnt.size );
  entry.insert( UDS_MODIFICATION_TIME, ftpEnt.date );
  entry.insert( UDS_ACCESS, ftpEnt.access );
  entry.insert( UDS_USER, ftpEnt.owner );
  if ( !ftpEnt.group.isEmpty() )
  {
    entry.insert( UDS_GROUP, ftpEnt.group );
  }

  if ( !ftpEnt.link.isEmpty() )
  {
    entry.insert( UDS_LINK_DEST, ftpEnt.link );

    KMimeType::Ptr mime = KMimeType::findByURL( KURL("ftp://host/" + filename ) );
    // Links on ftp sites are often links to dirs, and we have no way to check
    // that. Let's do like Netscape : assume dirs generally.
    // But we do this only when the mimetype can't be known from the filename.
    // --> we do better than Netscape :-)
    if ( mime->name() == KMimeType::defaultMimeType() )
    {
      kdDebug(7102) << "Setting guessed mime type to inode/directory for " << filename << endl;
      entry.insert( UDS_GUESSED_MIME_TYPE, QString::fromLatin1( "inode/directory" ) );
      isDir = true;
    }
  }

  entry.insert( UDS_FILE_TYPE, isDir ? S_IFDIR : ftpEnt.type );
  // entry.insert UDS_ACCESS_TIME,buff.st_atime);
  // entry.insert UDS_CREATION_TIME,buff.st_ctime);
}


void Ftp::ftpShortStatAnswer( const QString& filename, bool isDir )
{
    UDSEntry entry;


    entry.insert( KIO::UDS_NAME, filename );
    entry.insert( KIO::UDS_FILE_TYPE, isDir ? S_IFDIR : S_IFREG );
    entry.insert( KIO::UDS_ACCESS, S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH );
    // No details about size, ownership, group, etc.

    statEntry(entry);
    finished();
}

void Ftp::ftpStatAnswerNotFound( const QString & path, const QString & filename )
{
    // Only do the 'hack' below if we want to download an existing file (i.e. when looking at the "source")
    // When e.g. uploading a file, we still need stat() to return "not found"
    // when the file doesn't exist.
    QString statSide = metaData("statSide");
    kdDebug(7102) << "Ftp::stat statSide=" << statSide << endl;
    if ( statSide == "source" )
    {
        kdDebug(7102) << "Not found, but assuming found, because some servers don't allow listing" << endl;
        // MS Server is incapable of handling "list <blah>" in a case insensitive way
        // But "retr <blah>" works. So lie in stat(), to get going...
        //
        // There's also the case of ftp://ftp2.3ddownloads.com/90380/linuxgames/loki/patches/ut/ut-patch-436.run
        // where listing permissions are denied, but downloading is still possible.
        ftpShortStatAnswer( filename, false /*file, not dir*/ );

        return;
    }

    error( ERR_DOES_NOT_EXIST, path );
}

void Ftp::stat( const KURL &url)
{
  kdDebug(7102) << "Ftp::stat : path='" << url.path() << "'" << endl;
  if( !ftpOpenConnection(loginImplicit) )
        return;

  QString path = QDir::cleanPath( url.path() );
  kdDebug(7102) << "Ftp::stat : cleaned path='" << path << "'" << endl;

  // We can't stat root, but we know it's a dir.
  if( path.isEmpty() || path == "/" )
  {
    UDSEntry entry;
    //entry.insert( KIO::UDS_NAME, UDSField( QString::null ) );
    entry.insert( KIO::UDS_NAME, QString::fromLatin1( "." ) );
    entry.insert( KIO::UDS_FILE_TYPE, S_IFDIR );
    entry.insert( KIO::UDS_ACCESS, S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH );
    entry.insert( KIO::UDS_USER, QString::fromLatin1( "root" ) );
    entry.insert( KIO::UDS_GROUP, QString::fromLatin1( "root" ) );
    // no size

    statEntry( entry );
    finished();
    return;
  }

  KURL tempurl( url );
  tempurl.setPath( path ); // take the clean one
  QString listarg; // = tempurl.directory(false /*keep trailing slash*/);
  QString parentDir;
  QString filename = tempurl.fileName();
  Q_ASSERT(!filename.isEmpty());
  QString search = filename;

  // Try cwd into it, if it works it's a dir (and then we'll list the parent directory to get more info)
  // if it doesn't work, it's a file (and then we'll use dir filename)
  bool isDir = ftpFolder(path, false);

  // if we're only interested in "file or directory", we should stop here
  QString sDetails = metaData("details");
  int details = sDetails.isEmpty() ? 2 : sDetails.toInt();
  kdDebug(7102) << "Ftp::stat details=" << details << endl;
  if ( details == 0 )
  {
     if ( !isDir && !ftpSize( path, 'I' ) ) // ok, not a dir -> is it a file ?
     {  // no -> it doesn't exist at all
        ftpStatAnswerNotFound( path, filename );
        return;
     }
     ftpShortStatAnswer( filename, isDir ); // successfully found a dir or a file -> done
     return;
  }

  if (!isDir)
  {
    // It is a file or it doesn't exist, try going to parent directory
    parentDir = tempurl.directory(false /*keep trailing slash*/);
    // With files we can do "LIST <filename>" to avoid listing the whole dir
    listarg = filename;
  }
  else
  {
    // --- New implementation:
    // Don't list the parent dir. Too slow, might not show it, etc.
    // Just return that it's a dir.
    UDSEntry entry;
    entry.insert( KIO::UDS_NAME, filename );
    entry.insert( KIO::UDS_FILE_TYPE, S_IFDIR );
    entry.insert( KIO::UDS_ACCESS, S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH );
    // No clue about size, ownership, group, etc.

    statEntry(entry);
    finished();
    return;

    // --- Old implementation:
#if 0
    // It's a dir, remember that
    // Reason: it could be a symlink to a dir, in which case ftpReadDir
    // in the parent dir will have no idea about that. But we know better.
    isDir = true;
    // If the dir starts with '.', we'll need '-a' to see it in the listing.
    if ( search[0] == '.' )
       listarg = "-a";
    parentDir = "..";
#endif
  }

  // Now cwd the parent dir, to prepare for listing
  if( !ftpFolder(parentDir, true) )
    return;

  if( !ftpOpenCommand( "list", listarg, 'I', ERR_DOES_NOT_EXIST ) )
  {
    kdError(7102) << "COULD NOT LIST" << endl;
    return;
  }
  kdDebug(7102) << "Starting of list was ok" << endl;

  Q_ASSERT( !search.isEmpty() && search != "/" );

  bool bFound = false;
  KURL      linkURL;
  FtpEntry  ftpEnt;
  while( ftpReadDir(ftpEnt) )
  {
    // We look for search or filename, since some servers (e.g. ftp.tuwien.ac.at)
    // return only the filename when doing "dir /full/path/to/file"
    if ( !bFound )
    {
        if ( ( search == ftpEnt.name || filename == ftpEnt.name ) ) {
            if ( !filename.isEmpty() ) {
              bFound = true;
              UDSEntry entry;
              ftpCreateUDSEntry( filename, ftpEnt, entry, isDir );
              statEntry( entry );
            }
        } else if ( isDir && ( ftpEnt.name == listarg || ftpEnt.name+'/' == listarg ) ) {
            // Damn, the dir we're trying to list is in fact a symlink
            // Follow it and try again
            if ( ftpEnt.link.isEmpty() )
                kdWarning(7102) << "Got " << listarg << " as answer, but empty link!" << endl;
            else
            {
                linkURL = url;
                kdDebug(7102) << "ftpEnt.link=" << ftpEnt.link << endl;
                if ( ftpEnt.link[0] == '/' )
                    linkURL.setPath( ftpEnt.link ); // Absolute link
                else
                {
                    // Relative link (stat will take care of cleaning ../.. etc.)
                    linkURL.setPath( listarg ); // this is what we were listing (the link)
                    linkURL.setPath( linkURL.directory() ); // go up one dir
                    linkURL.addPath( ftpEnt.link ); // replace link by its destination
                    kdDebug(7102) << "linkURL now " << linkURL.prettyURL() << endl;
                }
                // Re-add the filename we're looking for
                linkURL.addPath( filename );
            }
            bFound = true;
        }
    }

    // kdDebug(7102) << ftpEnt.name << endl;
  }

  ftpCloseCommand();        // closes the data connection only

  if ( !bFound )
  {
    ftpStatAnswerNotFound( path, filename );
    return;
  }

  if ( !linkURL.isEmpty() )
  {
      if ( linkURL == url || linkURL == tempurl )
      {
          error( ERR_CYCLIC_LINK, linkURL.prettyURL() );
          return;
      }
      stat( linkURL );
      return;
  }

  kdDebug(7102) << "stat : finished successfully" << endl;
  finished();
}


void Ftp::listDir( const KURL &url )
{
  kdDebug(7102) << "Ftp::listDir " << url.prettyURL() << endl;
  if( !ftpOpenConnection(loginImplicit) )
        return;

  // No path specified ?
  QString path = url.path();
  if ( path.isEmpty() )
  {
    KURL realURL;
    realURL.setProtocol( "ftp" );
    if ( m_user != FTP_LOGIN )
      realURL.setUser( m_user );
    // We set the password, so that we don't ask for it if it was given
    if ( m_pass != FTP_PASSWD )
      realURL.setPass( m_pass );
    realURL.setHost( m_host );
    realURL.setPort( m_port );
    if ( m_initialPath.isEmpty() )
        m_initialPath = "/";
    realURL.setPath( m_initialPath );
    kdDebug(7102) << "REDIRECTION to " << realURL.prettyURL() << endl;
    redirection( realURL );
    finished();
    return;
  }

  kdDebug(7102) << "hunting for path '" << path << "'" << endl;

  if (!ftpOpenDir( path ) )
  {
    if ( ftpSize( path, 'I' ) ) // is it a file ?
    {
      error( ERR_IS_FILE, path );
      return;
    }
    // not sure which to emit
    //error( ERR_DOES_NOT_EXIST, path );
    error( ERR_CANNOT_ENTER_DIRECTORY, path );
    return;
  }

  UDSEntry entry;
  FtpEntry  ftpEnt;
  while( ftpReadDir(ftpEnt) )
  {
    //kdDebug(7102) << ftpEnt.name << endl;
    //Q_ASSERT( !ftpEnt.name.isEmpty() );
    if ( !ftpEnt.name.isEmpty() )
    {
      //if ( S_ISDIR( (mode_t)ftpEnt.type ) )
      //   kdDebug(7102) << "is a dir" << endl;
      //if ( !ftpEnt.link.isEmpty() )
      //   kdDebug(7102) << "is a link to " << ftpEnt.link << endl;
      entry.clear();
      ftpCreateUDSEntry( ftpEnt.name, ftpEnt, entry, false );
      listEntry( entry, false );
    }
  }
  listEntry( entry, true ); // ready
  ftpCloseCommand();        // closes the data connection only
  finished();
}

void Ftp::slave_status()
{
  kdDebug(7102) << "Got slave_status host = " << (!m_host.toAscii().isEmpty() ? m_host.toAscii() : "[None]") << " [" << (m_bLoggedOn ? "Connected" : "Not connected") << "]" << endl;
  slaveStatus( m_host, m_bLoggedOn );
}

bool Ftp::ftpOpenDir( const QString & path )
{
  //QString path( _url.path(-1) );

  // We try to change to this directory first to see whether it really is a directory.
  // (And also to follow symlinks)
  QString tmp = path.isEmpty() ? QString("/") : path;

  // We get '550', whether it's a file or doesn't exist...
  if( !ftpFolder(tmp, false) )
      return false;

  // Don't use the path in the list command:
  // We changed into this directory anyway - so it's enough just to send "list".
  // We use '-a' because the application MAY be interested in dot files.
  // The only way to really know would be to have a metadata flag for this...
  // Since some windows ftp server seems not to support the -a argument, we use a fallback here.
  // In fact we have to use -la otherwise -a removes the default -l (e.g. ftp.trolltech.com)
  if( !ftpOpenCommand( "list -la", QString::null, 'I', ERR_CANNOT_ENTER_DIRECTORY ) )
  {
    if ( !ftpOpenCommand( "list", QString::null, 'I', ERR_CANNOT_ENTER_DIRECTORY ) )
    {
      kdWarning(7102) << "Can't open for listing" << endl;
      return false;
    }
  }
  kdDebug(7102) << "Starting of list was ok" << endl;
  return true;
}

bool Ftp::ftpReadDir(FtpEntry& de)
{
  assert(m_data != NULL);

  // get a line from the data connecetion ...
  while( true )
  {
    QByteArray data = m_data->readLine();
    if (data.size() == 0)
      break;

    const char* buffer = data.data();
    kdDebug(7102) << "dir > " << buffer << endl;

    //Normally the listing looks like
    // -rw-r--r--   1 dfaure   dfaure        102 Nov  9 12:30 log
    // but on Netware servers like ftp://ci-1.ci.pwr.wroc.pl/ it looks like (#76442)
    // d [RWCEAFMS] Admin                     512 Oct 13  2004 PSI

    // we should always get the following 5 fields ...
    const char *p_access, *p_junk, *p_owner, *p_group, *p_size;
    if( (p_access = strtok((char*)buffer," ")) == 0) continue;
    if( (p_junk  = strtok(NULL," ")) == 0) continue;
    if( (p_owner = strtok(NULL," ")) == 0) continue;
    if( (p_group = strtok(NULL," ")) == 0) continue;
    if( (p_size  = strtok(NULL," ")) == 0) continue;

    //kdDebug(7102) << "p_access=" << p_access << " p_junk=" << p_junk << " p_owner=" << p_owner << " p_group=" << p_group << " p_size=" << p_size << endl;

    de.access = 0;
    if ( strlen( p_access ) == 1 && p_junk[0] == '[' ) { // Netware
      de.access = S_IRWXU | S_IRWXG | S_IRWXO; // unknown -> give all permissions
    }

    const char *p_date_1, *p_date_2, *p_date_3, *p_name;

    // A special hack for "/dev". A listing may look like this:
    // crw-rw-rw-   1 root     root       1,   5 Jun 29  1997 zero
    // So we just ignore the number in front of the ",". Ok, its a hack :-)
    if ( strchr( p_size, ',' ) != 0L )
    {
      //kdDebug(7102) << "Size contains a ',' -> reading size again (/dev hack)" << endl;
      if ((p_size = strtok(NULL," ")) == 0)
        continue;
    }

    // Check whether the size we just read was really the size
    // or a month (this happens when the server lists no group)
    // Used to be the case on sunsite.uio.no, but not anymore
    // This is needed for the Netware case, too.
    if ( !isdigit( *p_size ) )
    {
      p_date_1 = p_size;
      p_size = p_group;
      p_group = 0;
      //kdDebug(7102) << "Size didn't have a digit -> size=" << p_size << " date_1=" << p_date_1 << endl;
    }
    else
    {
      p_date_1 = strtok(NULL," ");
      //kdDebug(7102) << "Size has a digit -> ok. p_date_1=" << p_date_1 << endl;
    }

    if ( p_date_1 != 0 &&
         (p_date_2 = strtok(NULL," ")) != 0 &&
         (p_date_3 = strtok(NULL," ")) != 0 &&
         (p_name = strtok(NULL,"\r\n")) != 0 )
    {
      {
        QByteArray tmp( p_name );
        if ( p_access[0] == 'l' )
        {
          int i = tmp.lastIndexOf( " -> " );
          if ( i != -1 ) {
            de.link = remoteEncoding()->decode(p_name + i + 4);
            tmp.truncate( i );
          }
          else
            de.link = QString::null;
        }
        else
          de.link = QString::null;

        if ( tmp[0] == '/' ) // listing on ftp://ftp.gnupg.org/ starts with '/'
          tmp.remove( 0, 1 );

        if (tmp.indexOf('/') != -1)
          continue; // Don't trick us!
        // Some sites put more than one space between the date and the name
        // e.g. ftp://ftp.uni-marburg.de/mirror/
        de.name     = remoteEncoding()->decode(tmp.trimmed());
      }

      de.type = S_IFREG;
      switch ( p_access[0] ) {
      case 'd':
        de.type = S_IFDIR;
        break;
      case 's':
        de.type = S_IFSOCK;
        break;
      case 'b':
        de.type = S_IFBLK;
        break;
      case 'c':
        de.type = S_IFCHR;
        break;
      case 'l':
        de.type = S_IFREG;
        // we don't set S_IFLNK here.  de.link says it.
        break;
      default:
        break;
      }

      if ( p_access[1] == 'r' )
        de.access |= S_IRUSR;
      if ( p_access[2] == 'w' )
        de.access |= S_IWUSR;
      if ( p_access[3] == 'x' || p_access[3] == 's' )
        de.access |= S_IXUSR;
      if ( p_access[4] == 'r' )
        de.access |= S_IRGRP;
      if ( p_access[5] == 'w' )
        de.access |= S_IWGRP;
      if ( p_access[6] == 'x' || p_access[6] == 's' )
        de.access |= S_IXGRP;
      if ( p_access[7] == 'r' )
        de.access |= S_IROTH;
      if ( p_access[8] == 'w' )
        de.access |= S_IWOTH;
      if ( p_access[9] == 'x' || p_access[9] == 't' )
        de.access |= S_IXOTH;
      if ( p_access[3] == 's' || p_access[3] == 'S' )
        de.access |= S_ISUID;
      if ( p_access[6] == 's' || p_access[6] == 'S' )
        de.access |= S_ISGID;
      if ( p_access[9] == 't' || p_access[9] == 'T' )
        de.access |= S_ISVTX;

      de.owner    = remoteEncoding()->decode(p_owner);
      de.group    = remoteEncoding()->decode(p_group);
      de.size     = charToLongLong(p_size);

      // Parsing the date is somewhat tricky
      // Examples : "Oct  6 22:49", "May 13  1999"

      // First get current time - we need the current month and year
      time_t currentTime = time( 0L );
      struct tm * tmptr = gmtime( &currentTime );
      int currentMonth = tmptr->tm_mon;
      //kdDebug(7102) << "Current time :" << asctime( tmptr ) << endl;
      // Reset time fields
      tmptr->tm_sec = 0;
      tmptr->tm_min = 0;
      tmptr->tm_hour = 0;
      // Get day number (always second field)
      tmptr->tm_mday = atoi( p_date_2 );
      // Get month from first field
      // NOTE : no, we don't want to use KLocale here
      // It seems all FTP servers use the English way
      //kdDebug(7102) << "Looking for month " << p_date_1 << endl;
      static const char * s_months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
      for ( int c = 0 ; c < 12 ; c ++ )
        if ( !strcmp( p_date_1, s_months[c]) )
        {
          //kdDebug(7102) << "Found month " << c << " for " << p_date_1 << endl;
          tmptr->tm_mon = c;
          break;
        }

      // Parse third field
      if ( strlen( p_date_3 ) == 4 ) // 4 digits, looks like a year
        tmptr->tm_year = atoi( p_date_3 ) - 1900;
      else
      {
        // otherwise, the year is implicit
        // according to man ls, this happens when it is between than 6 months
        // old and 1 hour in the future.
        // So the year is : current year if tm_mon <= currentMonth+1
        // otherwise current year minus one
        // (The +1 is a security for the "+1 hour" at the end of the month issue)
        if ( tmptr->tm_mon > currentMonth + 1 )
          tmptr->tm_year--;

        // and p_date_3 contains probably a time
        char * semicolon;
        if ( ( semicolon = (char*)strchr( p_date_3, ':' ) ) )
        {
          *semicolon = '\0';
          tmptr->tm_min = atoi( semicolon + 1 );
          tmptr->tm_hour = atoi( p_date_3 );
        }
        else
          kdWarning(7102) << "Can't parse third field " << p_date_3 << endl;
      }

      //kdDebug(7102) << asctime( tmptr ) << endl;
      de.date = mktime( tmptr );
      return true;
    }
  } // line invalid, loop to get another line
  return false;
}

//===============================================================================
// public: get           download file from server
// helper: ftpGet        called from get() and copy()
//===============================================================================
void Ftp::get( const KURL & url )
{
  kdDebug(7102) << "Ftp::get " << url.url() << endl;
  int iError = 0;
  ftpGet(iError, -1, url, 0);               // iError gets status
  if(iError)                                // can have only server side errs
     error(iError, url.path());
  ftpCloseCommand();                        // must close command!
}

Ftp::StatusCode Ftp::ftpGet(int& iError, int iCopyFile, const KURL& url, KIO::fileoffset_t llOffset)
{
  // Calls error() by itself!
  if( !ftpOpenConnection(loginImplicit) )
    return statusServerError;

  // Try to find the size of the file (and check that it exists at
  // the same time). If we get back a 550, "File does not exist"
  // or "not a plain file", check if it is a directory. If it is a
  // directory, return an error; otherwise simply try to retrieve
  // the request...
  if ( !ftpSize( url.path(), '?' ) && (m_iRespCode == 550) &&
       ftpFolder(url.path(), false) )
  {
    // Ok it's a dir in fact
    kdDebug(7102) << "ftpGet: it is a directory in fact" << endl;
    iError = ERR_IS_DIRECTORY;
    return statusServerError;
  }

  QString resumeOffset = metaData("resume");
  if ( !resumeOffset.isEmpty() )
  {
    llOffset = resumeOffset.toLongLong();
    kdDebug(7102) << "ftpGet: got offset from metadata : " << llOffset << endl;
  }

  if( !ftpOpenCommand("retr", url.path(), '?', ERR_CANNOT_OPEN_FOR_READING, llOffset) )
  {
    kdWarning(7102) << "ftpGet: Can't open for reading" << endl;
    return statusServerError;
  }

  // Read the size from the response string
  if(m_size == UnknownSize)
  {
    const char* psz = strrchr( ftpResponse(4), '(' );
    if(psz) m_size = charToLongLong(psz+1);
  }

  KIO::filesize_t bytesLeft = 0;
  if ( m_size != UnknownSize )
    bytesLeft = m_size - llOffset;

  kdDebug(7102) << "ftpGet: starting with offset=" << llOffset << endl;
  KIO::fileoffset_t processed_size = llOffset;

  QByteArray array;
  bool mimetypeEmitted = false;
  char buffer[maximumIpcSize];
  // start whith small data chunks in case of a slow data source (modem)
  // - unfortunately this has a negative impact on performance for large
  // - files - so we will increase the block size after a while ...
  int iBlockSize = initialIpcSize;
  int iBufferCur = 0;

  while(m_size == UnknownSize || bytesLeft > 0)
  {  // let the buffer size grow if the file is larger 64kByte ...
    if(processed_size-llOffset > 1024 * 64)
      iBlockSize = maximumIpcSize;

    // read the data and detect EOF or error ...
    if(iBlockSize+iBufferCur > (int)sizeof(buffer))
      iBlockSize = sizeof(buffer) - iBufferCur;
    int n = m_data->read( buffer+iBufferCur, iBlockSize );
    if(n <= 0)
    {   // this is how we detect EOF in case of unknown size
      if( m_size == UnknownSize && n == 0 )
        break;
      // unexpected eof. Happens when the daemon gets killed.
      iError = ERR_COULD_NOT_READ;
      return statusServerError;
    }
    processed_size += n;

    // collect very small data chunks in buffer before processing ...
    if(m_size != UnknownSize)
    {
      bytesLeft -= n;
      iBufferCur += n;
      if(iBufferCur < mimimumMimeSize && bytesLeft > 0)
      {
        processedSize( processed_size );
        continue;
      }
      n = iBufferCur;
      iBufferCur = 0;
    }

    // get the mime type and set the total size ...
    if(!mimetypeEmitted)
    {
      mimetypeEmitted = true;
      array = array.fromRawData(buffer, n);
      KMimeMagicResult * result = KMimeMagic::self()->findBufferFileType(array, url.fileName());
      array.clear();
      kdDebug(7102) << "ftpGet: Emitting mimetype " << result->mimeType() << endl;
      mimeType( result->mimeType() );
      if( m_size != UnknownSize )	// Emit total size AFTER mimetype
        totalSize( m_size );
    }

    // write output file or pass to data pump ...
    if(iCopyFile == -1)
    {
       array = array.fromRawData(buffer, n);
       data( array );
       array.clear();
    }
    else if( (iError = WriteToFile(iCopyFile, buffer, n)) != 0)
       return statusClientError;              // client side error
    processedSize( processed_size );
  }

  kdDebug(7102) << "ftpGet: done" << endl;
  if(iCopyFile == -1)          // must signal EOF to data pump ...
    data(array);               // array is empty and must be empty!

  processedSize( m_size == UnknownSize ? processed_size : m_size );
  kdDebug(7102) << "ftpGet: emitting finished()" << endl;
  finished();
  return statusSuccess;
}

/*
void Ftp::mimetype( const KURL& url )
{
  if( !ftpOpenConnection(loginImplicit) )
        return;

  if ( !ftpOpenCommand( "retr", url.path(), 'I', ERR_CANNOT_OPEN_FOR_READING, 0 ) ) {
    kdWarning(7102) << "Can't open for reading" << endl;
    return;
  }
  char buffer[ 2048 ];
  QByteArray array;
  // Get one chunk of data only and send it, KIO::Job will determine the
  // mimetype from it using KMimeMagic
  int n = m_data->read( buffer, 2048 );
  array.setRawData(buffer, n);
  data( array );
  array.resetRawData(buffer, n);

  kdDebug(7102) << "aborting" << endl;
  ftpAbortTransfer();

  kdDebug(7102) << "finished" << endl;
  finished();
  kdDebug(7102) << "after finished" << endl;
}

void Ftp::ftpAbortTransfer()
{
  // RFC 959, page 34-35
  // IAC (interpret as command) = 255 ; IP (interrupt process) = 254
  // DM = 242 (data mark)
   char msg[4];
   // 1. User system inserts the Telnet "Interrupt Process" (IP) signal
   //   in the Telnet stream.
   msg[0] = (char) 255; //IAC
   msg[1] = (char) 254; //IP
   (void) send(sControl, msg, 2, 0);
   // 2. User system sends the Telnet "Sync" signal.
   msg[0] = (char) 255; //IAC
   msg[1] = (char) 242; //DM
   if (send(sControl, msg, 2, MSG_OOB) != 2)
     ; // error...

   // Send ABOR
   kdDebug(7102) << "send ABOR" << endl;
   QCString buf = "ABOR\r\n";
   if ( KSocks::self()->write( sControl, buf.data(), buf.length() ) <= 0 )  {
     error( ERR_COULD_NOT_WRITE, QString::null );
     return;
   }

   //
   kdDebug(7102) << "read resp" << endl;
   if ( readresp() != '2' )
   {
     error( ERR_COULD_NOT_READ, QString::null );
     return;
   }

  kdDebug(7102) << "close sockets" << endl;
  closeSockets();
}
*/

//===============================================================================
// public: put           upload file to server
// helper: ftpPut        called from put() and copy()
//===============================================================================
void Ftp::put(const KURL& url, int permissions, bool overwrite, bool resume)
{
  kdDebug(7102) << "Ftp::put " << url.url() << endl;
  int iError = 0;                           // iError gets status
  ftpPut(iError, -1, url, permissions, overwrite, resume);
  if(iError)                                // can have only server side errs
     error(iError, url.path());
  ftpCloseCommand();                        // must close command!
}

Ftp::StatusCode Ftp::ftpPut(int& iError, int iCopyFile, const KURL& dest_url,
                            int permissions, bool overwrite, bool resume)
{
  if( !ftpOpenConnection(loginImplicit) )
    return statusServerError;

  // Don't use mark partial over anonymous FTP.
  // My incoming dir allows put but not rename...
  bool bMarkPartial;
  if (m_user.isEmpty () || m_user == FTP_LOGIN)
    bMarkPartial = false;
  else
    bMarkPartial = config()->readBoolEntry("MarkPartial", true);

  QString dest_orig = dest_url.path();
  QString dest_part( dest_orig );
  dest_part += ".part";

  if ( ftpSize( dest_orig, 'I' ) )
  {
    if ( m_size == 0 )
    { // delete files with zero size
      QByteArray cmd = "DELE ";
      cmd += remoteEncoding()->encode(dest_orig);
      if( !ftpSendCmd( cmd ) || (m_iRespType != 2) )
      {
        iError = ERR_CANNOT_DELETE_PARTIAL;
        return statusServerError;
      }
    }
    else if ( !overwrite && !resume )
    {
       iError = ERR_FILE_ALREADY_EXIST;
       return statusServerError;
    }
    else if ( bMarkPartial )
    { // when using mark partial, append .part extension
      if ( !ftpRename( dest_orig, dest_part, true ) )
      {
        iError = ERR_CANNOT_RENAME_PARTIAL;
        return statusServerError;
      }
    }
    // Don't chmod an existing file
    permissions = -1;
  }
  else if ( bMarkPartial && ftpSize( dest_part, 'I' ) )
  { // file with extension .part exists
    if ( m_size == 0 )
    {  // delete files with zero size
      QByteArray cmd = "DELE ";
      cmd += remoteEncoding()->encode(dest_part);
      if ( !ftpSendCmd( cmd ) || (m_iRespType != 2) )
      {
        iError = ERR_CANNOT_DELETE_PARTIAL;
        return statusServerError;
      }
    }
    else if ( !overwrite && !resume )
    {
      resume = canResume (m_size);
      if (!resume)
      {
        iError = ERR_FILE_ALREADY_EXIST;
        return statusServerError;
      }
    }
  }
  else
    m_size = 0;

  QString dest;

  // if we are using marking of partial downloads -> add .part extension
  if ( bMarkPartial ) {
    kdDebug(7102) << "Adding .part extension to " << dest_orig << endl;
    dest = dest_part;
  } else
    dest = dest_orig;

  KIO::fileoffset_t offset = 0;

  // set the mode according to offset
  if( resume && m_size > 0 )
  {
    offset = m_size;
    if(iCopyFile != -1)
    {
      if( KDE_lseek(iCopyFile, offset, SEEK_SET) < 0 )
      {
        iError = ERR_CANNOT_RESUME;
        return statusClientError;
      }
    }
  }

  if (! ftpOpenCommand( "stor", dest, '?', ERR_COULD_NOT_WRITE, offset ) )
     return statusServerError;

  kdDebug(7102) << "ftpPut: starting with offset=" << offset << endl;
  KIO::fileoffset_t processed_size = offset;

  QByteArray buffer;
  int result;
  int iBlockSize = initialIpcSize;
  // Loop until we got 'dataEnd'
  do
  {
    if(iCopyFile == -1)
    {
      dataReq(); // Request for data
      result = readData( buffer );
    }
    else
    { // let the buffer size grow if the file is larger 64kByte ...
      if(processed_size-offset > 1024 * 64)
        iBlockSize = maximumIpcSize;
      buffer.resize(iBlockSize);
      result = ::read(iCopyFile, buffer.data(), buffer.size());
      if(result < 0)
        iError = ERR_COULD_NOT_WRITE;
      else
        buffer.resize(result);
    }

    if (result > 0)
    {
      m_data->write( buffer.data(), buffer.size() );
      processed_size += result;
      processedSize (processed_size);
    }
  }
  while ( result > 0 );

  if (result != 0) // error
  {
    ftpCloseCommand();               // don't care about errors
    kdDebug(7102) << "Error during 'put'. Aborting." << endl;
    if (bMarkPartial)
    {
      // Remove if smaller than minimum size
      if ( ftpSize( dest, 'I' ) &&
           ( processed_size < (unsigned long) config()->readNumEntry("MinimumKeepSize", DEFAULT_MINIMUM_KEEP_SIZE) ) )
      {
        QByteArray cmd = "DELE ";
        cmd += remoteEncoding()->encode(dest);
        (void) ftpSendCmd( cmd );
      }
    }
    return statusServerError;
  }

  if ( !ftpCloseCommand() )
  {
    iError = ERR_COULD_NOT_WRITE;
    return statusServerError;
  }

  // after full download rename the file back to original name
  if ( bMarkPartial )
  {
    kdDebug(7102) << "renaming dest (" << dest << ") back to dest_orig (" << dest_orig << ")" << endl;
    if ( !ftpRename( dest, dest_orig, true ) )
    {
      iError = ERR_CANNOT_RENAME_PARTIAL;
      return statusServerError;
    }
  }

  // set final permissions
  if ( permissions != -1 )
  {
    if ( m_user == FTP_LOGIN )
      kdDebug(7102) << "Trying to chmod over anonymous FTP ???" << endl;
    // chmod the file we just put
    if ( ! ftpChmod( dest_orig, permissions ) )
    {
        // To be tested
        //if ( m_user != FTP_LOGIN )
        //    warning( i18n( "Could not change permissions for\n%1" ).arg( dest_orig ) );
    }
  }

  // We have done our job => finish
  finished();
  return statusSuccess;
}


/** Use the SIZE command to get the file size.
    Warning : the size depends on the transfer mode, hence the second arg. */
bool Ftp::ftpSize( const QString & path, char mode )
{
  m_size = UnknownSize;
  if( !ftpDataMode(mode) )
      return false;

  QByteArray buf;
  buf = "SIZE ";
  buf += remoteEncoding()->encode(path);
  if( !ftpSendCmd( buf ) || (m_iRespType != 2) )
    return false;

  // skip leading "213 " (response code)
  const char* psz = ftpResponse(4);
  if(!psz)
    return false;
  m_size = charToLongLong(psz);
  return true;
}

// Today the differences between ASCII and BINARY are limited to
// CR or CR/LF line terminators. Many servers ignore ASCII (like
// win2003 -or- vsftp with default config). In the early days of
// computing, when even text-files had structure, this stuff was
// more important.
// Theoretically "list" could return different results in ASCII
// and BINARY mode. But again, most servers ignore ASCII here.
bool Ftp::ftpDataMode(char cMode)
{
  if(cMode == '?') cMode = m_bTextMode ? 'A' : 'I';
  else if(cMode == 'a') cMode = 'A';
  else if(cMode != 'A') cMode = 'I';

  kdDebug(7102) << "ftpDataMode: want '" << cMode << "' has '" << m_cDataMode << "'" << endl;
  if(m_cDataMode == cMode)
    return true;

  QByteArray buf = "TYPE ";
  buf += cMode;
  if( !ftpSendCmd(buf) || (m_iRespType != 2) )
      return false;
  m_cDataMode = cMode;
  return true;
}


bool Ftp::ftpFolder(const QString& path, bool bReportError)
{
  QString newPath = path;
  int iLen = newPath.length();
  if(iLen > 1 && newPath[iLen-1] == '/') newPath.truncate(iLen-1);

  //kdDebug(7102) << "ftpFolder: want '" << newPath << "' has '" << m_currentPath << "'" << endl;
  if(m_currentPath == newPath)
    return true;

  QByteArray tmp = "cwd ";
  tmp += remoteEncoding()->encode(newPath);
  if( !ftpSendCmd(tmp) )
    return false;                  // connection failure
  if(m_iRespType != 2)
  {
    if(bReportError)
      error(ERR_CANNOT_ENTER_DIRECTORY, path);
    return false;                  // not a folder
  }
  m_currentPath = newPath;
  return true;
}


//===============================================================================
// public: copy          don't use kio data pump if one side is a local file
// helper: ftpCopyPut    called from copy() on upload
// helper: ftpCopyGet    called from copy() on download
//===============================================================================
void Ftp::copy( const KURL &src, const KURL &dest, int permissions, bool overwrite )
{
  int iError = 0;
  int iCopyFile = -1;
  StatusCode cs = statusSuccess;
  bool bSrcLocal = src.isLocalFile();
  bool bDestLocal = dest.isLocalFile();
  QString  sCopyFile;

  if(bSrcLocal && !bDestLocal)                    // File -> Ftp
  {
    sCopyFile = src.path();
    kdDebug(7102) << "Ftp::copy local file '" << sCopyFile << "' -> ftp '" << dest.path() << "'" << endl;
    cs = ftpCopyPut(iError, iCopyFile, sCopyFile, dest, permissions, overwrite);
    if( cs == statusServerError) sCopyFile = dest.url();
  }
  else if(!bSrcLocal && bDestLocal)               // Ftp -> File
  {
    sCopyFile = dest.path();
    kdDebug(7102) << "Ftp::copy ftp '" << src.path() << "' -> local file '" << sCopyFile << "'" << endl;
    cs = ftpCopyGet(iError, iCopyFile, sCopyFile, src, permissions, overwrite);
    if( cs == statusServerError ) sCopyFile = src.url();
  }
  else {
    error( ERR_UNSUPPORTED_ACTION, QString::null );
    return;
  }

  // perform clean-ups and report error (if any)
  if(iCopyFile != -1)
    ::close(iCopyFile);
  if(iError)
    error(iError, sCopyFile);
  ftpCloseCommand();                        // must close command!
}


Ftp::StatusCode Ftp::ftpCopyPut(int& iError, int& iCopyFile, QString sCopyFile,
                                const KURL& url, int permissions, bool overwrite)
{
  // check if source is ok ...
  KDE_struct_stat buff;
  QByteArray sSrc( QFile::encodeName(sCopyFile) );
  bool bSrcExists = (KDE_stat( sSrc.data(), &buff ) != -1);
  if(bSrcExists)
  { if(S_ISDIR(buff.st_mode))
    {
      iError = ERR_IS_DIRECTORY;
      return statusClientError;
    }
  }
  else
  {
    iError = ERR_DOES_NOT_EXIST;
    return statusClientError;
  }

  iCopyFile = KDE_open( sSrc.data(), O_RDONLY );
  if(iCopyFile == -1)
  {
    iError = ERR_CANNOT_OPEN_FOR_READING;
    return statusClientError;
  }

  // delegate the real work (iError gets status) ...
  totalSize(buff.st_size);
#ifdef  ENABLE_CAN_RESUME
  return ftpPut(iError, iCopyFile, url, permissions, overwrite, false);
#else
  return ftpPut(iError, iCopyFile, url, permissions, overwrite, true);
#endif
}


Ftp::StatusCode Ftp::ftpCopyGet(int& iError, int& iCopyFile, const QString sCopyFile,
                                const KURL& url, int permissions, bool overwrite)
{
  // check if destination is ok ...
  KDE_struct_stat buff;
  QByteArray sDest( QFile::encodeName(sCopyFile) );
  bool bDestExists = (KDE_stat( sDest.data(), &buff ) != -1);
  if(bDestExists)
  { if(S_ISDIR(buff.st_mode))
    {
      iError = ERR_IS_DIRECTORY;
      return statusClientError;
    }
    if(!overwrite)
    {
      iError = ERR_FILE_ALREADY_EXIST;
      return statusClientError;
    }
  }

  // do we have a ".part" file?
  QByteArray sPart = QFile::encodeName(sCopyFile + ".part");
  bool bResume = false;
  bool bPartExists = (KDE_stat( sPart.data(), &buff ) != -1);
  bool bMarkPartial = config()->readBoolEntry("MarkPartial", true);
  if(bMarkPartial && bPartExists && buff.st_size > 0)
  { // must not be a folder! please fix a similar bug in kio_file!!
    if(S_ISDIR(buff.st_mode))
    {
      iError = ERR_DIR_ALREADY_EXIST;
      return statusClientError;                            // client side error
    }
    //doesn't work for copy? -> design flaw?
#ifdef  ENABLE_CAN_RESUME
    bResume = canResume( buff.st_size );
#else
    bResume = true;
#endif
  }

  if(bPartExists && !bResume)                  // get rid of an unwanted ".part" file
    remove(sPart.data());

  // JPF: in kio_file overwrite disables ".part" operations. I do not believe
  // JPF: that this is a good behaviour!
  if(bDestExists)                             // must delete for overwrite
    remove(sDest.data());

  // WABA: Make sure that we keep writing permissions ourselves,
  // otherwise we can be in for a surprise on NFS.
  mode_t initialMode;
  if (permissions != -1)
    initialMode = permissions | S_IWUSR;
  else
    initialMode = 0666;

  // open the output file ...
  KIO::fileoffset_t hCopyOffset = 0;
  if(bResume)
  {
    iCopyFile = KDE_open( sPart.data(), O_RDWR );  // append if resuming
    hCopyOffset = KDE_lseek(iCopyFile, 0, SEEK_END);
    if(hCopyOffset < 0)
    {
      iError = ERR_CANNOT_RESUME;
      return statusClientError;                            // client side error
    }
    kdDebug(7102) << "copy: resuming at " << hCopyOffset << endl;
  }
  else
    iCopyFile = KDE_open(sPart.data(), O_CREAT | O_TRUNC | O_WRONLY, initialMode);

  if(iCopyFile == -1)
  {
    kdDebug(7102) << "copy: ### COULD NOT WRITE " << sCopyFile << endl;
    iError = (errno == EACCES) ? ERR_WRITE_ACCESS_DENIED
                               : ERR_CANNOT_OPEN_FOR_WRITING;
    return statusClientError;
  }

  // delegate the real work (iError gets status) ...
  StatusCode iRes = ftpGet(iError, iCopyFile, url, hCopyOffset);
  if( ::close(iCopyFile) && iRes == statusSuccess )
  {
    iError = ERR_COULD_NOT_WRITE;
    iRes = statusClientError;
  }

  // handle renaming or deletion of a partial file ...
  if(bMarkPartial)
  {
    if(iRes == statusSuccess)
    { // rename ".part" on success
      if ( ::rename( sPart.data(), sDest.data() ) )
      {
        kdDebug(7102) << "copy: cannot rename " << sPart << " to " << sDest << endl;
        iError = ERR_CANNOT_RENAME_PARTIAL;
        iRes = statusClientError;
      }
    }
    else if(KDE_stat( sPart.data(), &buff ) == 0)
    { // should a very small ".part" be deleted?
      int size = config()->readNumEntry("MinimumKeepSize", DEFAULT_MINIMUM_KEEP_SIZE);
      if (buff.st_size <  size)
        remove(sPart.data());
    }
  }
  return iRes;
}
