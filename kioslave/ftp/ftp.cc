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

#include "ftp.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#include <netinet/in.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
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
#include <klocale.h>
#include <kinstance.h>
#include <kmimemagic.h>
#include <kmimetype.h>
#include <kextsock.h>
#include <ksockaddr.h>
#include <ksocks.h>
#include <kio/ioslave_defaults.h>
#include <kio/slaveconfig.h>

#define FTP_LOGIN QString::fromLatin1("anonymous")
#define FTP_PASSWD QString::fromLatin1("anonymous@")
size_t Ftp::UnknownSize = (size_t)-1;

using namespace KIO;

extern "C" { int kdemain(int argc, char **argv); }

int kdemain( int argc, char **argv )
{
  KLocale::setMainCatalogue("kdelibs");
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


Ftp::Ftp( const QCString &pool, const QCString &app )
    : SlaveBase( "ftp", pool, app )
{
  dirfile = 0L;
  m_extControl = sData = sDatal = 0;
  sControl = -1;
  ksControl = NULL;
  m_bLoggedOn = false;
  m_bFtpStarted = false;
  setMultipleAuthCaching( true );
  kdDebug(7102) << "Ftp::Ftp()" << endl;
}


Ftp::~Ftp()
{
  kdDebug(7102) << "Ftp::~Ftp()" << endl;
  closeConnection();
}

/* memccpy appeared first in BSD4.4 */
void *mymemccpy(void *dest, const void *src, int c, size_t n)
{
    char *d = (char*)dest;
    const char *s = (const char*)src;

    while (n-- > 0)
    if ((*d++ = *s++) == c)
      return d;

  return NULL;
}

/*
 * read a line of text
 *
 * return -1 on error, bytecount otherwise
 */
int Ftp::ftpReadline(char *buf,int max,netbuf *ctl)
{
  int x,retval = 0;
  char *end;
  int eof = 0;

  if ( max == 0 )
    return 0;
  do
  {
    if (ctl->cavail > 0)
    {
      x = (max >= ctl->cavail) ? ctl->cavail : max-1;
      end = (char*)mymemccpy(buf,ctl->cget,'\n',x);
      if (end != NULL)
        x = end - buf;
      retval += x;
      buf += x;
      *buf = '\0';
      max -= x;
      ctl->cget += x;
      ctl->cavail -= x;
      if (end != NULL)
        break;
    }
    if (max == 1)
    {
      *buf = '\0';
      break;
    }
    if (ctl->cput == ctl->cget)
    {
      ctl->cput = ctl->cget = ctl->buf;
      ctl->cavail = 0;
      ctl->cleft = FTP_BUFSIZ;
    }
    if (eof)
    {
      if (retval == 0)
        retval = -1;
      break;
    }
    if ((x = KSocks::self()->read(ctl->handle,ctl->cput,ctl->cleft)) == -1)
    {
      kdError(7102) << "read failed: " << strerror(errno) << endl;
      retval = -1;
      break;
    }
    if (x == 0)
      eof = 1;
    ctl->cleft -= x;
    ctl->cavail += x;
    ctl->cput += x;
  }
  while (1);

  return retval;
}

/**
 * read a response from the server, into rspbuf
 * @return first char of response (rspbuf[0]), '\0' if we couldn't read the response
 */
char Ftp::readresp()
{
  char match[5];
  if ( ftpReadline( rspbuf, 256, nControl ) == -1 )
  {
    // This can happen after the server closed the connection (after a timeout)
    kdWarning(7102) << "Could not read" << endl;
    //error( ERR_COULD_NOT_READ, QString::null );
    return '\0';
  }
  kdDebug(7102) << "resp> " << rspbuf << endl;
  if ( rspbuf[3] == '-' )  {
    strncpy( match, rspbuf, 3 );
    match[3] = ' ';
    match[4] = '\0';
    do {
      if ( ftpReadline( rspbuf, 256, nControl ) == -1 ) {
          kdWarning(7102) << "Could not read" << endl;
          //error( ERR_COULD_NOT_READ, QString::null );
          return '\0';
      }
      kdDebug(7102) << rspbuf << endl;
    }
    while ( strncmp( rspbuf, match, 4 ) );
  }

  return rspbuf[0];
}

void Ftp::closeConnection()
{
  kdDebug(7102) << "Ftp::closeConnection() m_bLoggedOn=" << m_bLoggedOn << " m_bFtpStarted=" << m_bFtpStarted << endl;
  if ( m_bLoggedOn || m_bFtpStarted )
  {
    Q_ASSERT( m_bFtpStarted ); // can't see how it could be false is loggedon is true
    if( sControl != 0 )
    {
      kdDebug(7102) << "Ftp::closeConnection() sending quit" << endl;
      if ( !ftpSendCmd( "quit", 0 ) || rspbuf[0] != '2' )
        kdWarning(7102) << "Ftp::closeConnection() 'quit' failed with err="
            << QString(QChar(rspbuf[0]))+QChar(rspbuf[1])+QChar(rspbuf[2]) << endl;
      free( nControl );
      if (ksControl != NULL)
	delete ksControl;
      //      ::close( sControl );
      sControl = 0;
    }
  }

  m_extControl = 0;
  m_bLoggedOn = false;
  m_bFtpStarted = false;
  //ready()
}


void Ftp::setHost( const QString& _host, int _port, const QString& _user,
                   const QString& _pass )
{
  kdDebug(7102) << "Ftp::setHost " << _host << " user=" << _user << endl;
  QString user = _user;
  QString pass = _pass;
  if ( !_user.isEmpty() )
  {
      user = _user;
      pass = _pass.isEmpty() ? QString::null:_pass;
  }
  else
  {
      user = FTP_LOGIN;
      pass = FTP_PASSWD;
  }

  m_proxyURL = metaData("UseProxy");
  kdDebug(7102) << "Proxy URL: " << m_proxyURL.url() << endl;
  m_bUseProxy = ( m_proxyURL.isValid() &&
                  m_proxyURL.protocol() == QString::fromLatin1("ftp") );

  if ( m_host != _host || m_port != _port ||
       m_user != user || m_pass != pass )
      closeConnection( );

  m_host = _host;
  m_port = _port;
  m_user = user;
  m_pass = pass;
}

void Ftp::openConnection()
{
  kdDebug(7102) << "openConnection " << m_host << ":" << m_port << " " << m_user << " [password hidden]" << endl;

  infoMessage( i18n("Opening connection to host <b>%1</b>").arg(m_host) );

  if ( m_host.isEmpty() )
  {
    error( ERR_UNKNOWN_HOST, QString::null );
    return;
  }

  assert( !m_bLoggedOn );

  m_initialPath = QString::null;

  QString host = m_bUseProxy ? m_proxyURL.host() : m_host;
  unsigned short int port = m_bUseProxy ? m_proxyURL.port() : m_port;

  if (!connect( host, port ))
    return; // error emitted by connect

  m_bFtpStarted = true;

  infoMessage( i18n("Connected to host <b>%1</b>").arg(m_host) );
  kdDebug(7102) << "Connected ...." << endl;

  m_bLoggedOn = ftpLogin();
  if ( !m_bLoggedOn )
    return; // error emitted by ftpLogin

  connected();
}


/**
 * Called by @ref openConnection. It opens the control connection to the ftp server.
 *
 * @return true on success.
 */
bool Ftp::connect( const QString &host, unsigned short int port )
{
  if ( port == 0 ) {
      struct servent *pse;
      if ( ( pse = getservbyname( "ftp", "tcp" ) ) == NULL )
          port = 21;
      else
          port = ntohs(pse->s_port);
  }
  int on = 1;
  // require an Internet Socket
  ksControl = new KExtendedSocket(host, port, KExtendedSocket::inetSocket);
  if (ksControl == NULL)
    {
      error( ERR_OUT_OF_MEMORY, QString::null );
      return false;
    }
  if (ksControl->connect() < 0)
    {
      if (ksControl->status() == IO_LookupError)
	error(ERR_UNKNOWN_HOST, host);
      else
	error(ERR_COULD_NOT_CONNECT, host);
      delete ksControl;
      ksControl = NULL;
      return false;
    }
  sControl = ksControl->fd();

  if ( setsockopt( sControl, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on) ) == -1 )
  {
    //    ::close( sControl );
    delete ksControl;
    ksControl = NULL;
    error( ERR_COULD_NOT_CREATE_SOCKET, host );
    return false;
  }

  nControl = (netbuf*)calloc(1,sizeof(netbuf));
  if (nControl == NULL)
  {
    //    ::close( sControl );
    delete ksControl;
    ksControl = NULL;
    error( ERR_OUT_OF_MEMORY, QString::null );
    return false;
  }
  nControl->handle = sControl;

  if ( readresp() != '2' )
  {
    //    ::close( sControl );
    delete ksControl;
    ksControl = NULL;
    free( nControl );
    error( ERR_COULD_NOT_CONNECT, host );
    return false;
  }

  return true;
}

/**
 * Called by @ref openConnection. It logs us in.
 * @ref m_initialPath is set to the current working directory
 * if logging on was successfull.
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

  //kdDebug(7102) << "ftpLogin " << user << ":" << pass << endl; // shows the pass!

  if ( !user.isEmpty() )
  {
    AuthInfo info;
    QCString tempbuf;
    int failedAuth = 0;

    // Construct the URL to be used as key for caching.
    info.url.setProtocol( QString::fromLatin1("ftp") );
    info.url.setHost( m_host );
    info.url.setPort( m_port );
    info.url.setUser( user );

    do
    {
      // Check the cache and/or prompt user for password if 1st
      // login attempt failed OR the user supplied a login name,
      // but no password.
      if ( failedAuth > 0 || (!user.isEmpty() && pass.isEmpty()) )
      {
        QString errorMsg;
        kdDebug(7102) << "Prompt the user for password..." << endl;
        // Ask user if we should retry after when login fails!
        if( failedAuth > 0 )
        {
          errorMsg = i18n("Message sent:\nLogin using username: %1 and "
                          "password: [hidden]\n\nServer replied:\n%2\n\n"
                          ).arg(user).arg(rspbuf);
        }

        if ( user != FTP_LOGIN && pass != FTP_PASSWD )
          info.username = m_user;

        kdDebug(7102) << "Is FTP URL valid? " << info.url.isValid() << endl;
        kdDebug(7102) << "Username: " << info.username << endl;
        info.prompt = i18n("You need to supply a username and a password "
                           "to access this site.");
        info.commentLabel = i18n( "Site:" );
        info.comment = i18n("<b>%1</b>").arg( m_host );
        info.keepPassword = true; // Prompt the user for persistence as well.
        info.readOnly = !info.username.isEmpty();

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
      tempbuf += user.latin1();
      if ( m_bUseProxy )
      {
        tempbuf += '@';
        tempbuf += m_host.latin1();
        if ( m_port > 0 && m_port != DEFAULT_FTP_PORT )
        {
          tempbuf += ':';
          tempbuf += QString::number(m_port).latin1();
        }
      }
      kdDebug(7102) << "Sending Login name: " << user << endl;
      bool loggedIn = (ftpSendCmd( tempbuf, 0 ) &&
                       !strncmp( rspbuf, "230", 3));
      bool needPass = !strncmp( rspbuf, "331", 3);
      // Prompt user for login info if we do not
      // get back a "230" or "331".
      if ( !loggedIn && !needPass )
      {
        kdDebug(7102) << "1> " << rspbuf << endl;
        ++failedAuth;
        continue;  // Well we failed, prompt the user please!!
      }

      if( needPass )
      {
        tempbuf = "pass ";
        tempbuf += pass.latin1();
        kdDebug(7102) << "Sending Login password: " << "[protected]" << endl;
        loggedIn = (ftpSendCmd( tempbuf, 0 ) && !strncmp(rspbuf, "230", 3));
      }

      if ( loggedIn )
      {
        // Do not cache the default login!!
        if( user != FTP_LOGIN && pass != FTP_PASSWD )
          cacheAuthentication( info );
        failedAuth = -1;
      }
    } while( ++failedAuth );
  }

  kdDebug(7102) << "Login OK" << endl;
  infoMessage( i18n("Login OK") );

  // Okay, we're logged in. If this is IIS 4, switch dir listing style to Unix:
  // Thanks to jk@soegaard.net (Jens Kristian S�gaard) for this hint
  if( ftpSendCmd( "syst", 0 ) && rspbuf[0] == '2' )
  {
    if( !strncmp( rspbuf, "215 Windows_NT version", 22 ) ) // should do for any version
    {
      (void)ftpSendCmd( "site dirstyle", 0 );
      // Check if it was already in Unix style
      // Patch from Keith Refson <Keith.Refson@earth.ox.ac.uk>
      if( !strncmp( rspbuf, "200 MSDOS-like directory output is on", 37 ))
         //It was in Unix style already!
         (void)ftpSendCmd( "site dirstyle", 0 );

    }
  }
  else
    kdWarning(7102) << "syst failed" << endl;


  QString macro = metaData( "autoLoginMacro" );
  if ( !macro.isEmpty() && config()->readBoolEntry("EnableAutoLoginMacro") )
  {
      QStringList list = QStringList::split('\n', macro);
      if ( !list.isEmpty() )
      {
          QStringList::Iterator it = list.begin();
          for( ; it != list.end() ; ++it )
          {
              if ( (*it).find("init") == 0 )
              {
                  list = QStringList::split( '\\', macro);
                  it = list.begin();
                  ++it;  // ignore the macro name
                  for( ; it != list.end() ; ++it )
                  {
                      // TODO: Add support for arbitrary commands
                      // besides simply changing directory!!
                      if ( (*it).startsWith( "cwd" ) )
                          ftpSendCmd( (*it).latin1(), 0 );
                  }
                  break;
              }
          }
      }
  }

  // Get the current working directory
  kdDebug(7102) << "Searching for pwd" << endl;
  if ( !ftpSendCmd( "pwd", 0 ) || rspbuf[0] != '2' )
  {
    kdDebug(7102) << "Couldn't issue pwd command" << endl;
    error( ERR_COULD_NOT_LOGIN, i18n("Could not login to %1.").arg(m_host) ); // or anything better ?
    return false;
  }

  kdDebug(7102) << "2> " << rspbuf << endl;

  char *p = strchr( rspbuf+3, '"' ); // Look for first "
  if ( p != 0 )
  {
    char *p2 = strchr( p + 1, '"' ); // Look for second "
    if ( p2 != 0 )
    {
        *p2 = '\0';
        m_initialPath = p + 1;
        if ( *(p+1) != '/' ) // safety check, for servers that return C:/TEMP/
          m_initialPath.prepend('/');
        kdDebug(7102) << "Initial path set to: " << m_initialPath << endl;
    }
  }
  return true;
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
bool Ftp::ftpSendCmd( const QCString& cmd, int maxretries )
{
  assert( sControl > 0 );

  QCString buf = cmd;
  buf += "\r\n";

  if ( cmd.left(4).lower() != "pass" ) // don't print out the password
    kdDebug(7102) << cmd.data() << endl;
  int num = KSocks::self()->write(sControl, buf.data(), buf.length());
  if (num <= 0 )  {
    error( ERR_COULD_NOT_WRITE, QString::null );
    return false;
  }

  char rsp = readresp();
  if (!rsp || ( rsp == '4' && rspbuf[1] == '2' && rspbuf[2] == '1' ))
  {
    // 421 is "421 No Transfer Timeout (300 seconds): closing control connection"
    // But when logging it, it can be "421 Too many users - please try again later."!
    // (This is why we only set maxretries for some operations)
    if ( maxretries > 0 )
    {
      kdDebug(7102) << "got timeout. maxretries=" << maxretries << endl;
      // It might mean a timeout occured, let's try logging in again
      m_bLoggedOn = false;
      kdDebug(7102) << "Couldn't read answer - perhaps timeout - trying logging in again" << endl;
      openConnection();
      if (!m_bLoggedOn)
      {
        kdDebug(7102) << "Login failure, aborting" << endl;
        return false;
      }
      kdDebug(7102) << "Logged back in, reissuing command" << endl;
      // On success, try the command again
      return ftpSendCmd( cmd, maxretries - 1 );
    } else if (cmd != "quit")
    {
      error( ERR_SERVER_TIMEOUT, m_host );
      return false;
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
bool Ftp::ftpOpenPASVDataConnection()
{
  int i[6], j;
  unsigned char n[6];
  int on=1;
  struct linger lng = { 1, 120 };
  KExtendedSocket ks;
  const KSocketAddress *sa = ksControl->peerAddress();
  QString host;

  // Check that we can do PASV
  if (sa != NULL && sa->family() != PF_INET)
    return false;		// no PASV for non-PF_INET connections

  if (m_extControl & pasvUnknown)
    return false;              // already tried and got "unknown command"

  m_bPasv = true;

  /* Let's PASsiVe*/
  if (!ftpSendCmd("PASV") || rspbuf[0] != '2')
  {
    kdDebug(7102) << "PASV attempt failed" << endl;
    // unknown command?
    if (rspbuf[0] == '5')
    {
        kdDebug(7102) << "disabling use of PASV" << endl;
        m_extControl |= pasvUnknown;
    }
    return false;
  }

  // The usual answer is '227 Entering Passive Mode. (160,39,200,55,6,245)'
  // but anonftpd gives '227 =160,39,200,55,6,245'
  char *start = strchr(rspbuf,'(');
  if ( !start )
    start = strchr(rspbuf,'=');
  if ( !start ||
       ( sscanf(start, "(%d,%d,%d,%d,%d,%d)",&i[0], &i[1], &i[2], &i[3], &i[4], &i[5]) != 6 &&
         sscanf(start, "=%d,%d,%d,%d,%d,%d", &i[0], &i[1], &i[2], &i[3], &i[4], &i[5]) != 6 ) )
  {
    kdError(7102) << "parsing IP and port numbers failed. String parsed: " << start << endl;
    return false;
  }

  for (j=0; j<6; j++)
  {
    n[j] = (unsigned char) (i[j] & 0xff);
  }

  // Make hostname
  host.sprintf("%d.%d.%d.%d", i[0], i[1], i[2], i[3]);
  // port number is given in network byte order
	int port = i[4] << 8 | i[5];
  ks.setAddress(host, port);
  ks.setSocketFlags(KExtendedSocket::noResolve);

  if (ks.connect() < 0)
    {
      kdError(7102) << "PASV: ks.connect failed. host=" << host << " port=" << port << endl;
      return false;
    }

  sDatal = ks.fd();
  if ( (setsockopt( sDatal,SOL_SOCKET,SO_REUSEADDR,(char*)&on, sizeof(on) ) == -1)
       || (sDatal < 0) )
  {
    kdError(7102) << "PASV: setsockopt failed" << endl;
    return false;
  }

  if ( setsockopt(sDatal, SOL_SOCKET,SO_KEEPALIVE, (char *) &on, (int) sizeof(on)) < 0 )
    kdError(7102) << "Keepalive not allowed" << endl;
  if ( setsockopt(sDatal, SOL_SOCKET,SO_LINGER, (char *) &lng,(int) sizeof (lng)) < 0 )
    kdError(7102) << "Linger mode was not allowed." << endl;

  ks.release();
  return true;
}

/*
 * ftpOpenEPSVDataConnection - opens a data connection via EPSV
 */
bool Ftp::ftpOpenEPSVDataConnection()
{
  // for SO_LINGER
  int on=1;
  struct linger lng = { 1, 120 };

  KExtendedSocket ks;
  const KSocketAddress *sa = ksControl->peerAddress();
  int portnum;
  // we are sure sa is a KInetSocketAddress, because we asked for KExtendedSocket::inetSocket
  // when we connected
  const KInetSocketAddress *sin = static_cast<const KInetSocketAddress*>(sa);

  if (m_extControl & epsvUnknown || sa == NULL)
    return false;

  m_bPasv = true;
  if (!ftpSendCmd("EPSV") || rspbuf[0] != '2')
    {
      // unknown command?
      if (rspbuf[0] == '5')
	{
	  kdDebug(7102) << "disabling use of EPSV" << endl;
	  m_extControl |= epsvUnknown;
	}
      return false;
    }

  char *start = strchr(rspbuf,'|');
  if ( !start ||
       sscanf(start, "|||%d|", &portnum) != 1)
    {
      // invalid response?
      return false;
    }

  ks.setSocketFlags(KExtendedSocket::noResolve);
  ks.setAddress(sin->nodeName(), portnum);

  if (ks.connect() < 0)
    {
      return false;
    }

  sDatal = ks.fd();
  if ( (setsockopt( sDatal,SOL_SOCKET,SO_REUSEADDR,(char*)&on, sizeof(on) ) == -1)
       || (sDatal < 0) )
  {
    return false;
  }

  if ( setsockopt(sDatal, SOL_SOCKET,SO_KEEPALIVE, (char *) &on, (int) sizeof(on)) < 0 )
    kdError(7102) << "Keepalive not allowed" << endl;
  if ( setsockopt(sDatal, SOL_SOCKET,SO_LINGER, (char *) &lng,(int) sizeof (lng)) < 0 )
    kdError(7102) << "Linger mode was not allowed." << endl;

  ks.release();
  return true;
}

/*
 * ftpOpenEPRTDataConnection
 */
bool Ftp::ftpOpenEPRTDataConnection()
{
  KExtendedSocket ks;
  // yes, we are sure this is a KInetSocketAddress
  const KInetSocketAddress *sin = static_cast<const KInetSocketAddress*>(ksControl->localAddress());

  m_bPasv = false;

  if (m_extControl & eprtUnknown || sin == NULL)
    return false;
  ks.setHost(sin->nodeName());
  ks.setPort(0);		// setting port to 0 will make us bind to a random, free port
  ks.setSocketFlags(KExtendedSocket::noResolve | KExtendedSocket::passiveSocket |
		    KExtendedSocket::inetSocket);

  if (ks.listen(1) < 0)
    {
      error(ERR_COULD_NOT_LISTEN, m_host);
      return false;
    }

  sin = static_cast<const KInetSocketAddress*>(ks.localAddress());
  if (sin == NULL)
    // error ?
    return false;

  //  QString command = QString::fromLatin1("eprt |%1|%2|%3|").arg(sin->ianaFamily())
  //  .arg(sin->nodeName())
  //  .arg(sin->port());
  QCString command;
  command.sprintf("eprt |%d|%s|%d|", sin->ianaFamily(),
		  sin->nodeName().latin1(), sin->port());

  // FIXME! Encoding for hostnames?
  if (!ftpSendCmd(command) || rspbuf[0] != '2')
    {
      // unknown command?
      if (rspbuf[0] == '5')
	{
	  kdDebug(7102) << "disabling use of EPRT" << endl;
	  m_extControl |= eprtUnknown;
	}
      return false;
    }

  sDatal = ks.fd();
  ks.release();
  return true;
}


/*
 * ftpOpenDataConnection - set up data connection
 *
 * return 1 if successful, 0 otherwise
 */
bool Ftp::ftpOpenDataConnection()
{
  assert( m_bLoggedOn );

  union
  {
    struct sockaddr sa;
    struct sockaddr_in in;
  } sin;

  struct linger lng = { 0, 0 };
  ksocklen_t l;
  char buf[64];
  int on = 1;

  ////////////// First try passive (EPSV & PASV) modes
  if ( config()->readBoolEntry( "DisablePassiveMode", false ) == false )
  {
    if ( config()->readBoolEntry( "DisableEPSV", false ) == false )
      if (ftpOpenEPSVDataConnection())
        return true;
    if (ftpOpenPASVDataConnection())
      return true;

    // if we sent EPSV ALL already and it was accepted, then we can't
    // use active connections any more
    if (m_extControl & epsvAllSent)
      return false;

  }

  if (ftpOpenEPRTDataConnection())
    return true;
  ////////////// Fallback : PORT mode
  m_bPasv = false;

  l = sizeof(sin);
  if ( KSocks::self()->getsockname( sControl, &sin.sa, &l ) < 0 )
    return false;
  if (sin.sa.sa_family != PF_INET)
    return false;		// wrong family

  sDatal = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
  if ( sDatal == 0 )
  {
    error( ERR_COULD_NOT_CREATE_SOCKET, QString::null );
    return false;
  }
  if ( setsockopt( sDatal, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on) ) == -1 )
  {
    ::close( sDatal );
    error( ERR_COULD_NOT_CREATE_SOCKET, QString::null );
    return false;
  }
  if ( setsockopt( sDatal, SOL_SOCKET, SO_LINGER, (char*)&lng, sizeof(lng) ) == -1 )
  {
    ::close( sDatal );
    error( ERR_COULD_NOT_CREATE_SOCKET, QString::null );
    return false;
  }

  sin.in.sin_port = 0;
  if ( KSocks::self()->bind( sDatal, &sin.sa, sizeof(sin) ) == -1 )
  {
    ::close( sDatal );
    sDatal = 0;
    error( ERR_COULD_NOT_BIND, m_host );
    return false;
  }

  if ( KSocks::self()->listen( sDatal, 1 ) < 0 )
  {
    error( ERR_COULD_NOT_LISTEN, m_host );
    ::close( sDatal );
    sDatal = 0;
    return 0;
  }

  if ( KSocks::self()->getsockname( sDatal, &sin.sa, &l ) < 0 )
    // error ?
    return false;

  sprintf(buf,"port %d,%d,%d,%d,%d,%d",
          (unsigned char)sin.sa.sa_data[2],(unsigned char)sin.sa.sa_data[3],
          (unsigned char)sin.sa.sa_data[4],(unsigned char)sin.sa.sa_data[5],
          (unsigned char)sin.sa.sa_data[0],(unsigned char)sin.sa.sa_data[1]);

  return ftpSendCmd( buf ) && rspbuf[0] == '2';
}


/*
 * ftpAcceptConnect - wait for incoming connection
 * Used by @ref ftpOpenCommand
 *
 * return -2 on error or timeout
 * otherwise returns socket descriptor
 */
int Ftp::ftpAcceptConnect()
{
  struct sockaddr addr;
  int sData;
  ksocklen_t l;
  fd_set mask;

  FD_ZERO(&mask);
  FD_SET(sDatal,&mask);

  if ( m_bPasv )
    return sDatal;
  if ( KSocks::self()->select( sDatal + 1, &mask, NULL, NULL, 0L ) == 0)
  {
    ::close( sDatal );
    return -2;
  }

  l = sizeof(addr);
  if ( ( sData = KSocks::self()->accept( sDatal, &addr, &l ) ) > 0 )
    return sData;

  ::close( sDatal );
  return -2;
}

bool Ftp::ftpOpenCommand( const char *_command, const QString & _path, char _mode,
                          int errorcode, unsigned long _offset )
{
  QCString buf = "type ";
  buf += _mode;

  if ( !ftpSendCmd( buf, 0 ) || rspbuf[0] != '2' )
  {
    error( ERR_COULD_NOT_CONNECT, QString::null );
    return false;
  }
  if ( !ftpOpenDataConnection() )
  {
    error( ERR_COULD_NOT_CONNECT, QString::null );
    return false;
  }

  if ( _offset > 0 ) {
    // send rest command if offset > 0, this applies to retr and stor commands
    char buf[100];
    sprintf(buf, "rest %ld", _offset);
    if ( !ftpSendCmd( buf, 0 ) )
       return false;
    if ( rspbuf[0] != '3' ) {
      error( ERR_CANNOT_RESUME, _path ); // should never happen
      return false;
    }
  }

  QCString tmp = _command;

  if ( !_path.isEmpty() ) {
    tmp += " ";
    tmp += _path.ascii();
  }

  if ( !ftpSendCmd( tmp, 0 ) || rspbuf[0] != '1' ) {
    if ( _offset > 0 && strcmp(_command, "retr") == 0 && rspbuf[0] == '4')
    {
      // Failed to resume
      errorcode = ERR_CANNOT_RESUME;
    }
    // The error here depends on the command
    error( errorcode, _path );
    return false;
  }

  // Only now we know for sure that we can resume
  if ( _offset > 0 && strcmp(_command, "retr") == 0 )
    canResume();

  if ( ( sData = ftpAcceptConnect() ) < 0 )
  {
    error( ERR_COULD_NOT_ACCEPT, QString::null );
    return false;
  }

  return true;
}


void Ftp::closeSockets()
{
  if( sData != 0 )
  {
    shutdown( sData, 2 );
    ::close( sData );
    sData = 0;
  }

  if( sDatal != 0 )
  {
    ::close( sDatal );
    sDatal = 0;
  }
}

bool Ftp::ftpCloseCommand()
{
  kdDebug(7102) << "Ftp::ftpCloseCommand" << endl;
  // first close data sockets (if opened), then read response that
  // we got for whatever was used in ftpOpenCommand ( should be 226 )
  closeSockets();
  if ( readresp() != '2' )
  {
    kdDebug(7102) << "Did not get transfer complete message" << endl;
    return false;
  }
  return true;
}

void Ftp::mkdir( const KURL & url, int permissions )
{
  QString path = url.path();
  if (!m_bLoggedOn)
  {
      openConnection();
      if (!m_bLoggedOn)
      {
        kdDebug(7102) << "Login failure, aborting" << endl;
        return;
      }
  }

  assert( m_bLoggedOn );

  QCString buf = "mkd ";
  buf += path.latin1();

  if ( !ftpSendCmd( buf ) || rspbuf[0] != '2' )
  {
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
  if (!m_bLoggedOn)
  {
      openConnection();
      if (!m_bLoggedOn)
      {
        kdDebug(7102) << "Login failure, aborting" << endl;
        return;
      }
  }

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

  QCString cmd;
  cmd = "RNFR ";
  cmd += src.ascii();
  if ( !ftpSendCmd( cmd ) || rspbuf[0] != '3')
    return false;
  cmd = "RNTO ";
  cmd += dst.ascii();
  return ftpSendCmd( cmd, 0 ) && rspbuf[0] == '2';
}

void Ftp::del( const KURL& url, bool isfile )
{
  QString path = url.path();
  if (!m_bLoggedOn)
  {
      openConnection();
      if (!m_bLoggedOn)
      {
        kdDebug(7102) << "Login failure, aborting" << endl;
        return;
      }
  }

  assert( m_bLoggedOn );

  if ( !isfile )
  {
    // When deleting a directory, we must exit from it first
    // The last command probably went into it (to stat it)
    QCString tmp = "cwd ";
    tmp += url.directory().ascii();

    (void) ftpSendCmd( tmp );
    // ignore errors
  }

  QCString cmd = isfile ? "DELE " : "RMD ";
  cmd += path.ascii();

  if ( !ftpSendCmd( cmd, isfile ? 1 : 0 ) || rspbuf[0] != '2' )
    error( ERR_CANNOT_DELETE, path );
  else
    finished();
}

bool Ftp::ftpChmod( const QString & path, int permissions )
{
  assert( m_bLoggedOn );

  QCString cmd = "SITE CHMOD ";

  char buf[10];
  // we need to do bit AND 777 to get permissions, in case
  // we were sent a full mode (unlikely)
  sprintf(buf, "%o ", permissions & 511 );

  cmd += buf;
  cmd += path.ascii();

  return ftpSendCmd( cmd ) && rspbuf[0] == '2';
}

void Ftp::chmod( const KURL & url, int permissions )
{
  if (!m_bLoggedOn)
  {
      openConnection();
      if (!m_bLoggedOn)
      {
        kdDebug(7102) << "Login failure, aborting" << endl;
        return;
      }
  }

  if ( !ftpChmod( url.path(), permissions ) )
    error( ERR_CANNOT_CHMOD, url.path() );
  else
    finished();
}

void Ftp::createUDSEntry( const QString & filename, FtpEntry * e, UDSEntry & entry, bool isDir )
{
  assert(entry.count() == 0); // by contract :-)
  UDSAtom atom;
  atom.m_uds = UDS_NAME;
  atom.m_str = filename;
  entry.append( atom );

  atom.m_uds = UDS_SIZE;
  atom.m_long = e->size;
  entry.append( atom );

  atom.m_uds = UDS_MODIFICATION_TIME;
  atom.m_long = e->date;
  entry.append( atom );

  atom.m_uds = UDS_ACCESS;
  atom.m_long = e->access;
  entry.append( atom );

  atom.m_uds = UDS_USER;
  atom.m_str = e->owner;
  entry.append( atom );

  if ( !e->group.isEmpty() )
  {
    atom.m_uds = UDS_GROUP;
    atom.m_str = e->group;
    entry.append( atom );
  }

  if ( !e->link.isEmpty() )
  {
    atom.m_uds = UDS_LINK_DEST;
    atom.m_str = e->link;
    entry.append( atom );

    KMimeType::Ptr mime = KMimeType::findByURL( KURL(QString::fromLatin1("ftp://host/") + filename ) );
    // Links on ftp sites are often links to dirs, and we have no way to check
    // that. Let's do like Netscape : assume dirs generally.
    // But we do this only when the mimetype can't be known from the filename.
    // --> we do better than Netscape :-)
    if ( mime->name() == KMimeType::defaultMimeType() )
    {
      kdDebug(7102) << "Setting guessed mime type to inode/directory for " << filename << endl;
      atom.m_uds = UDS_GUESSED_MIME_TYPE;
      atom.m_str = "inode/directory";
      entry.append( atom );
      isDir = true;
    }
  }

  atom.m_uds = UDS_FILE_TYPE;
  atom.m_long = isDir ? S_IFDIR : e->type;
  entry.append( atom );

  /* atom.m_uds = UDS_ACCESS_TIME;
     atom.m_long = buff.st_atime;
     entry.append( atom );

     atom.m_uds = UDS_CREATION_TIME;
     atom.m_long = buff.st_ctime;
     entry.append( atom ); */
}


void Ftp::shortStatAnswer( const QString& filename, bool isDir )
{
    UDSEntry entry;
    UDSAtom atom;

    atom.m_uds = KIO::UDS_NAME;
    atom.m_str = filename;
    entry.append( atom );

    atom.m_uds = KIO::UDS_FILE_TYPE;
    atom.m_long = isDir ? S_IFDIR : S_IFREG;
    entry.append( atom );

    atom.m_uds = KIO::UDS_ACCESS;
    atom.m_long = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    entry.append( atom );

    // No details about size, ownership, group, etc.

    statEntry(entry);
    finished();
}

void Ftp::statAnswerNotFound( const QString & path, const QString & filename )
{
    // Only do the 'hack' below if we want to download an existing file (i.e. when looking at the "source")
    // When e.g. uploading a file, we still need stat() to return "not found"
    // when the file doesn't exist.
    QString statSide = metaData(QString::fromLatin1("statSide"));
    kdDebug(7102) << "Ftp::stat statSide=" << statSide << endl;
    if ( statSide == "source" )
    {
	kdDebug(7102) << "Not found, but assuming found, because some servers don't allow listing" << endl;
        // MS Server is incapable of handling "list <blah>" in a case insensitive way
        // But "retr <blah>" works. So lie in stat(), to get going...
	//
	// There's also the case of ftp://ftp2.3ddownloads.com/90380/linuxgames/loki/patches/ut/ut-patch-436.run
	// where listing permissions are denied, but downloading is still possible.
        shortStatAnswer( filename, false /*file, not dir*/ );
    }
    else
    {
        error( ERR_DOES_NOT_EXIST, path );
    }
}

void Ftp::stat( const KURL &url)
{
  kdDebug(7102) << "Ftp::stat : path='" << url.path() << "'" << endl;
  QString path = QDir::cleanDirPath( url.path() );
  if (!m_bLoggedOn)
  {
      openConnection();
      if (!m_bLoggedOn)
      {
        kdDebug(7102) << "Login failure, aborting" << endl;
        return;
      }
  }

  kdDebug(7102) << "Ftp::stat : cleaned path='" << path << "'" << endl;

  // We can't stat root, but we know it's a dir.
  if ( path.isEmpty() || path == QString::fromLatin1("/") ) {
    UDSEntry entry;
    UDSAtom atom;

    atom.m_uds = KIO::UDS_NAME;
    atom.m_str = QString::null;
    entry.append( atom );

    atom.m_uds = KIO::UDS_FILE_TYPE;
    atom.m_long = S_IFDIR;
    entry.append( atom );

    atom.m_uds = KIO::UDS_ACCESS;
    atom.m_long = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    entry.append( atom );

    atom.m_uds = KIO::UDS_USER;
    atom.m_str = "root";
    entry.append( atom );
    atom.m_uds = KIO::UDS_GROUP;
    entry.append( atom );

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
  QCString tmp = "cwd ";
  tmp += path.latin1();
  if ( !ftpSendCmd( tmp ) )
  {
    kdDebug(7102) << "stat: ftpSendCmd returned false" << endl;
    // error already emitted, if e.g. transmission failure
    return;
  }

  bool isDir = ( rspbuf[0] != '5' );

  // if we're only interested in "file or directory", we should stop here
  QString sDetails = metaData(QString::fromLatin1("details"));
  int details = sDetails.isEmpty() ? 2 : sDetails.toInt();
  kdDebug(7102) << "Ftp::stat details=" << details << endl;
  if ( details == 0 )
  {
     if ( !isDir && !ftpSize( path, 'I' ) ) // ok, not a dir -> is it a file ?
     {  // no -> it doesn't exist at all
        statAnswerNotFound( path, filename );
        return;
     }
     shortStatAnswer( filename, isDir ); // succesfully found a dir or a file -> done
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
    UDSAtom atom;

    atom.m_uds = KIO::UDS_NAME;
    atom.m_str = filename;
    entry.append( atom );

    atom.m_uds = KIO::UDS_FILE_TYPE;
    atom.m_long = S_IFDIR;
    entry.append( atom );

    atom.m_uds = KIO::UDS_ACCESS;
    atom.m_long = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    entry.append( atom );

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
  tmp = "cwd ";
  tmp += parentDir.latin1();
  if ( !ftpSendCmd( tmp, 0 ) )
    // error already emitted
    return;

  if ( rspbuf[0] != '2' )
  {
    kdDebug(7102) << "stat: Could not go to parent directory" << endl;
    error( ERR_CANNOT_ENTER_DIRECTORY, parentDir );
    return;
  }

  if( !ftpOpenCommand( "list", listarg, 'A', ERR_DOES_NOT_EXIST ) )
  {
    kdError(7102) << "COULD NOT LIST" << endl;
    return;
  }

  dirfile = fdopen( sData, "r" );
  if( !dirfile ) {
    error( ERR_DOES_NOT_EXIST, path );
    return;
  }

  kdDebug(7102) << "Starting of list was ok" << endl;

  Q_ASSERT( !search.isEmpty() && search != QString::fromLatin1("/") );

  FtpEntry *e;
  bool bFound = false;
  KURL linkURL;
  while( ( e = ftpReadDir() ) )
  {
    // We look for search or filename, since some servers (e.g. ftp.tuwien.ac.at)
    // return only the filename when doing "dir /full/path/to/file"
    if ( !bFound )
    {
        if ( ( search == e->name || filename == e->name ) ) {
            if ( !filename.isEmpty() ) {
              bFound = true;
              UDSEntry entry;
              createUDSEntry( filename, e, entry, isDir );
              statEntry( entry );
            }
        } else if ( isDir && ( e->name == listarg || e->name+'/' == listarg ) ) {
            // Damn, the dir we're trying to list is in fact a symlink
            // Follow it and try again
            if ( e->link.isEmpty() )
                kdWarning(7102) << "Got " << listarg << " as answer, but empty link!" << endl;
            else
            {
                linkURL = url;
                kdDebug(7102) << "e->link=" << e->link << endl;
                if ( e->link[0] == '/' )
                    linkURL.setPath( e->link ); // Absolute link
                else
                {
                    // Relative link (stat will take care of cleaning ../.. etc.)
                    linkURL.setPath( listarg ); // this is what we were listing (the link)
                    linkURL.setPath( linkURL.directory() ); // go up one dir
                    linkURL.addPath( e->link ); // replace link by its destination
                    kdDebug(7102) << "linkURL now " << linkURL.prettyURL() << endl;
                }
                // Re-add the filename we're looking for
                linkURL.addPath( filename );
            }
            bFound = true;
        }
    }

    kdDebug(7102) << e->name << endl;
  }

  (void) ftpCloseDir();

  if ( !bFound )
  {
    statAnswerNotFound( path, filename );
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
  if (!m_bLoggedOn)
  {
      openConnection();
      if (!m_bLoggedOn)
      {
        kdDebug(7102) << "Login failure, aborting" << endl;
        return;
      }
  }

  QString path = url.path();
  // No path specified ?
  if ( path.isEmpty() )
  {
    KURL realURL;
    realURL.setProtocol( QString::fromLatin1("ftp") );
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
    redirection( realURL.url() );
    path = m_initialPath;
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
  FtpEntry * e;
  while( ( e = ftpReadDir() ) )
  {
    kdDebug(7102) << e->name << endl;
    Q_ASSERT( !e->name.isEmpty() );
    if ( !e->name.isEmpty() )
    {
      //if ( S_ISDIR( (mode_t)e->type ) )
      //   kdDebug(7102) << "is a dir" << endl;
      //if ( !e->link.isEmpty() )
      //   kdDebug(7102) << "is a link to " << e->link << endl;
      entry.clear();
      createUDSEntry( e->name, e, entry, false );
      listEntry( entry, false );
    }
  }
  listEntry( entry, true ); // ready

  (void) ftpCloseDir();

  finished();
}

void Ftp::slave_status()
{
  kdDebug(7102) << "Got slave_status host = " << (m_host.ascii() ? m_host.ascii() : "[None]") << " [" << (m_bLoggedOn ? "Connected" : "Not connected") << "]" << endl;
  slaveStatus( m_host, m_bLoggedOn );
}

bool Ftp::ftpOpenDir( const QString & path )
{
  //QString path( _url.path(-1) );

  // We try to change to this directory first to see whether it really is a directory.
  // (And also to follow symlinks)
  QCString tmp = "cwd ";
  tmp += ( !path.isEmpty() ) ? path.latin1() : "/";

  if ( !ftpSendCmd( tmp ) || rspbuf[0] != '2' )
  {
    // We get '550', whether it's a file or doesn't exist...
      return false;
  }

  // Don't use the path in the list command:
  // We changed into this directory anyway ("cwd"), so it's enough just to send "list".
  // We use '-a' because the application MAY be interested in dot files.
  // The only way to really know would be to have a metadata flag for this...
  // ####### can't use -a, ftp://160.39.200.55 answers "Sorry, I don't see that file"
  if( !ftpOpenCommand( "list", QString::null, 'A', ERR_CANNOT_ENTER_DIRECTORY ) )
  {
    kdWarning(7102) << "Can't open for listing" << endl;
    return false;
  }

  dirfile = fdopen( sData, "r" );
  if( !dirfile )
    return false;

  kdDebug(7102) << "Starting of list was ok" << endl;

  return true;
}

FtpEntry *Ftp::ftpReadDir()
{
  char buffer[1024];

  while( fgets( buffer, sizeof(buffer), dirfile ) != 0 ) {
    FtpEntry* e = ftpParseDir( buffer );
    if ( e )
      return e;
  }
  return 0L;
}


FtpEntry* Ftp::ftpParseDir( char* buffer )
{
  QString tmp;
  kdDebug(7102) << "ftpParseDir " << buffer << endl;

  static FtpEntry de;
  const char *p_access, *p_junk, *p_owner, *p_group;
  const char *p_size, *p_date_1, *p_date_2, *p_date_3, *p_name;
  if ((p_access = strtok(buffer," ")) != 0)
    if ((p_junk = strtok(NULL," ")) != 0)
      if ((p_owner = strtok(NULL," ")) != 0)
        if ((p_group = strtok(NULL," ")) != 0)
          if ((p_size = strtok(NULL," ")) != 0)
          {
            // A special hack for "/dev". A listing may look like this:
            // crw-rw-rw-   1 root     root       1,   5 Jun 29  1997 zero
            // So we just ignore the number in front of the ",". Ok, its a hack :-)
            if ( strchr( p_size, ',' ) != 0L )
            {
              //kdDebug(7102) << "Size contains a ',' -> reading size again (/dev hack)" << endl;
              if ((p_size = strtok(NULL," ")) == 0)
                return 0L;
            }

            // Check whether the size we just read was really the size
            // or a month (this happens when the server lists no group)
            // Test on sunsite.uio.no, for instance
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

            if ( p_date_1 != 0 )
              if ((p_date_2 = strtok(NULL," ")) != 0)
                if ((p_date_3 = strtok(NULL," ")) != 0)
                  if ((p_name = strtok(NULL,"\r\n")) != 0)
                  {
                    if ( p_access[0] == 'l' )
                    {
                      tmp = p_name;
                      int i = tmp.findRev( QString::fromLatin1(" -> ") );
                      if ( i != -1 ) {
                        de.link = p_name + i + 4;
                        tmp.truncate( i );
                        p_name = tmp.ascii();
                      }
                      else
                        de.link = QString::null;
                    }
                    else
                      de.link = QString::null;
                      
                    if (strchr(p_name, '/'))
                       return 0L; // Don't trick us!
                    
                    de.access = 0;
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
                    if ( p_access[3] == 'x' )
                      de.access |= S_IXUSR;
                    if ( p_access[4] == 'r' )
                      de.access |= S_IRGRP;
                    if ( p_access[5] == 'w' )
                      de.access |= S_IWGRP;
                    if ( p_access[6] == 'x' )
                      de.access |= S_IXGRP;
                    if ( p_access[7] == 'r' )
                      de.access |= S_IROTH;
                    if ( p_access[8] == 'w' )
                      de.access |= S_IWOTH;
                    if ( p_access[9] == 'x' )
                      de.access |= S_IXOTH;

                    // maybe fromLocal8Bit would be better in some cases,
                    // but what proves that the ftp server is in the same encoding
                    // than the user ??
                    de.owner    = QString::fromLatin1(p_owner);
                    de.group    = QString::fromLatin1(p_group);
                    de.size     = atoi(p_size);
                    QCString tmp( p_name );
                    // Some sites put more than one space between the date and the name
                    // e.g. ftp://ftp.uni-marburg.de/mirror/
                    de.name     = QString::fromLatin1(tmp.stripWhiteSpace());

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
                    return( &de );
                  }
          }
  return 0L;
}


bool Ftp::ftpCloseDir()
{
  if( dirfile )
  {
    kdDebug(7102) << "... closing" << endl;

    if ( ! ftpCloseCommand() )
      return false;

    fclose( dirfile );
    dirfile = 0L;

  } else
    kdDebug(7102) << "ftpCloseDir but no dirfile ??" << endl;
  return true;
}

//////////// get, put ////////

void Ftp::get( const KURL & url )
{
  kdDebug(7102) << "Ftp::get " << url.url() << endl;
  if (!m_bLoggedOn)
  {
      openConnection();
      if (!m_bLoggedOn)
      {
        kdDebug(7102) << "Login failure, aborting" << endl;
        return;
      }
  }

  // try to find the size of the file (and check that it exists at the same time)
  // 550 is "File does not exist"/"not a plain file"
  // If we got something else, maybe SIZE isn't supported.
  if ( !ftpSize( url.path(), 'I' ) && strncmp( rspbuf, "550", 3) == 0 )
  {
      // Not a file, or doesn't exist. We need to find out.
      QCString tmp = "cwd ";
      tmp += url.path().latin1();
      if ( ftpSendCmd( tmp ) && rspbuf[0] == '2' )
      {
          // Ok it's a dir in fact
          kdDebug(7102) << "Ftp::get: it is a directory in fact" << endl;
          error( ERR_IS_DIRECTORY, url.path() );
      }
      else
      {
          kdDebug(7102) << "Ftp::get: doesn't exist" << endl;
          error( ERR_DOES_NOT_EXIST, url.path() );
      }
      return;
  }

  unsigned long offset = 0;
  QString resumeOffset = metaData(QString::fromLatin1("resume"));
  if ( !resumeOffset.isEmpty() )
  {
      offset = resumeOffset.toInt();
      kdDebug(7102) << "Ftp::get got offset from medata : " << offset << endl;
  }

  if ( !ftpOpenCommand( "retr", url.path(), 'I', ERR_CANNOT_OPEN_FOR_READING, offset ) ) {
    kdWarning(7102) << "Can't open for reading" << endl;
    return;
  }

  // Read the size from the response string
  if ( strlen( rspbuf ) > 4 && m_size == UnknownSize ) {
    const char * p = strrchr( rspbuf, '(' );
    if ( p != 0L ) m_size = atol( p + 1 );
  }

  size_t bytesLeft = 0;
  if ( m_size != UnknownSize )
    bytesLeft = m_size - offset;

  kdDebug(7102) << "Ftp::get starting with offset=" << offset << endl;
  int processed_size = offset;

  char buffer[ 2048 ];
  QByteArray array;
  QByteArray mimetypeBuffer;

  bool mimetypeEmitted = false;

  while( m_size == UnknownSize || bytesLeft > 0 )
  {
    int n = ftpRead( buffer, 2048 );
    if ( m_size != UnknownSize )
      bytesLeft -= n;

    // Buffer the first 1024 bytes for mimetype determination
    if ( !mimetypeEmitted )
    {
      int oldSize = mimetypeBuffer.size();
      mimetypeBuffer.resize(oldSize + n);
      memcpy(mimetypeBuffer.data()+oldSize, buffer, n);

      // Found enough data - or we're arriving to the end of the file -> emit mimetype
      if (mimetypeBuffer.size() >= 1024 || (m_size != UnknownSize && bytesLeft <= 0) )
      {
        KMimeMagicResult * result = KMimeMagic::self()->findBufferFileType( mimetypeBuffer, url.fileName() );
        kdDebug(7102) << "Emitting mimetype " << result->mimeType() << endl;
        mimeType( result->mimeType() );
        mimetypeEmitted = true;
        data( mimetypeBuffer );
        mimetypeBuffer.resize(0);
        // Emit total size AFTER mimetype
        if ( m_size != UnknownSize )
          totalSize( m_size );
      }
    }
    else if ( n > 0 )
    {
      array.setRawData(buffer, n);
      data( array );
      array.resetRawData(buffer, n);
    }
    else if ( m_size == UnknownSize && n == 0 ) // this is how we detect EOF in case of unknown size
    {
      break;
    }
    else // unexpected eof. Happens when the daemon gets killed.
    {
      error( ERR_COULD_NOT_READ, url.path() );
      return;
    }

    processed_size += n;
    processedSize( processed_size );
  }

  kdDebug(7102) << "Get: done, sending empty QByteArray" << endl;
  data( QByteArray() );

  kdDebug(7102) << "Get: calling ftpCloseCommand()" << endl;
  (void) ftpCloseCommand();
  // proceed even on error

  processedSize( m_size == UnknownSize ? processed_size : m_size );

  kdDebug(7102) << "Get: emitting finished()" << endl;
  finished();
}

/*
void Ftp::mimetype( const KURL& url )
{
  if (!m_bLoggedOn)
  {
      openConnection();
      if (!m_bLoggedOn)
      {
        kdDebug(7102) << "Login failure, aborting" << endl;
        return;
      }
  }

  if ( !ftpOpenCommand( "retr", url.path(), 'I', ERR_CANNOT_OPEN_FOR_READING, 0 ) ) {
    kdWarning(7102) << "Can't open for reading" << endl;
    return;
  }
  char buffer[ 2048 ];
  QByteArray array;
  // Get one chunk of data only and send it, KIO::Job will determine the
  // mimetype from it using KMimeMagic
  int n = ftpRead( buffer, 2048 );
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

void Ftp::put( const KURL& dest_url, int permissions, bool overwrite, bool resume )
{
  QString dest_orig = dest_url.path();
  if (!m_bLoggedOn)
  {
      openConnection();
      if (!m_bLoggedOn)
      {
        kdDebug(7102) << "Login failure, aborting" << endl;
        return;
      }
  }

  kdDebug(7102) << "Put " << dest_orig << endl;
  QString dest_part( dest_orig );
  dest_part += QString::fromLatin1(".part");

  bool bMarkPartial = config()->readBoolEntry("MarkPartial", true);

  // Don't use mark partial over anonymous FTP.
  // My incoming dir allows put but not rename...
  if (m_user == FTP_LOGIN)
      bMarkPartial = false;

  if ( ftpSize( dest_orig, 'I' ) )
  {
    if ( m_size == 0 ) {  // delete files with zero size
      QCString cmd = "DELE ";
      cmd += dest_orig.ascii();
      if ( !ftpSendCmd( cmd ) || rspbuf[0] != '2' )
      {
        error( ERR_CANNOT_DELETE_PARTIAL, dest_orig );
        return;
      }
    } else if ( !overwrite && !resume ) {
      error( ERR_FILE_ALREADY_EXIST, dest_orig );
      return;
    } else if ( bMarkPartial ) { // when using mark partial, append .part extension
      if ( !ftpRename( dest_orig, dest_part, true ) )
      {
        error( KIO::ERR_CANNOT_RENAME_PARTIAL, dest_orig );
        return;
      }
    }
    // Don't chmod an existing file
    permissions = -1;
  } else if ( ftpSize( dest_part, 'I' ) ) { // file with extension .part exists
    if ( m_size == 0 ) {  // delete files with zero size
      QCString cmd = "DELE ";
      cmd += dest_part.ascii();
      if ( !ftpSendCmd( cmd ) || rspbuf[0] != '2' )
      {
        error( ERR_CANNOT_DELETE_PARTIAL, dest_orig );
        return;
      }
    } else if ( !overwrite && !resume ) {
      error( ERR_FILE_ALREADY_EXIST, dest_orig );
      return;
    } else if ( !bMarkPartial ) { // when using mark partial, remove .part extension
      if ( !ftpRename( dest_part, dest_orig, true ) )
      {
        error( KIO::ERR_CANNOT_RENAME_PARTIAL, dest_orig );
        return;
      }
    }

  }

  QString dest;

  // if we are using marking of partial downloads -> add .part extension
  if ( bMarkPartial ) {
    kdDebug(7102) << "Adding .part extension to " << dest_orig << endl;
    dest = dest_part;
  } else
    dest = dest_orig;

  unsigned long offset = 0;

  // set the mode according to offset
  if ( resume ) {
    offset = m_size;
    kdDebug(7102) << "Offset = " << (unsigned int) offset << "d" << endl;
  }

  if (! ftpOpenCommand( "stor", dest, 'I', ERR_COULD_NOT_WRITE, offset ) )
    return;

  int result;
  // Loop until we got 'dataEnd'
  do
  {
    QByteArray buffer;
    dataReq(); // Request for data
    result = readData( buffer );
    if (result > 0)
    {
      ftpWrite( buffer.data(), buffer.size() );
    }
  }
  while ( result > 0 );

  if (result != 0) // error
  {
    (void) ftpCloseCommand(); // don't care about errors
    kdDebug(7102) << "Error during 'put'. Aborting." << endl;
    if (bMarkPartial)
    {
      // Remove if smaller than minimum size
      if ( ftpSize( dest, 'I' ) &&
           ( m_size < (unsigned long) config()->readNumEntry("MinimumKeepSize", DEFAULT_MINIMUM_KEEP_SIZE) ) )
      {
        QCString cmd = "DELE ";
        cmd += dest.ascii();
        (void) ftpSendCmd( cmd, 0 );
      }
    }
    return;
  }

  if ( !ftpCloseCommand() )
  {
    error( KIO::ERR_COULD_NOT_WRITE, dest_orig);
    return;
  }

  // after full download rename the file back to original name
  if ( bMarkPartial )
  {
    kdDebug(7102) << "renaming dest (" << dest << ") back to dest_orig (" << dest_orig << ")" << endl;
    if ( !ftpRename( dest, dest_orig, true ) )
    {
      error( KIO::ERR_CANNOT_RENAME_PARTIAL, dest_orig );
      return;
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
}

/** Use the SIZE command to get the file size.
    Warning : the size depends on the transfer mode, hence the second arg. */
bool Ftp::ftpSize( const QString & path, char mode )
{
  QCString buf;
  buf.sprintf("type %c", mode);
  if ( !ftpSendCmd( buf ) || rspbuf[0] !='2' ) {
      return false;
  }

  buf="SIZE ";
  buf+=path.ascii();
  if ( !ftpSendCmd( buf, 0 ) || rspbuf[0] !='2' ) {
    m_size = UnknownSize;
    return false;
  }

  m_size = atol(rspbuf+4); // skip leading "213 " (response code)
  return true;
}


size_t Ftp::ftpRead(void *buffer, long len)
{
  size_t n = KSocks::self()->read( sData, buffer, len );
  return n;
}

size_t Ftp::ftpWrite(void *buffer, long len)
{
  return( KSocks::self()->write( sData, buffer, len ) );
}
