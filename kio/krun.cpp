/* This file is part of the KDE libraries
    Copyright (C) 2000 Torben Weis <weis@kde.org>

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

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "krun.h"
#include "kuserprofile.h"
#include "kmimetype.h"
#include "kmimemagic.h"
#include "kio/job.h"
#include "kio/global.h"
#include "kio/scheduler.h"

#include <kdatastream.h>
#include <kmessageboxwrapper.h>
#include <kurl.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kprotocolinfo.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <dcopclient.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdatetime.h>
#include <qregexp.h>
#include <kwin.h>
#include <kdesktopfile.h>
#include <kstartupinfo.h>

KOpenWithHandler * KOpenWithHandler::pOpenWithHandler = 0L;

class KRun::KRunPrivate
{
public:
    KRunPrivate() { m_showingError = false; }
    bool m_showingError;
};

pid_t KRun::runURL( const KURL& u, const QString& _mimetype )
{

  if ( _mimetype == "inode/directory-locked" )
  {
    KMessageBoxWrapper::error( 0L,
            i18n("<qt>Unable to enter <b>%1</b>.\nYou do not have access rights to this location.</qt>").arg(u.prettyURL()) );
    return 0;
  }
  else if ( _mimetype == "application/x-desktop" )
  {
    if ( u.isLocalFile() )
      return KDEDesktopMimeType::run( u, true );
  }
  else if ( _mimetype == "application/x-executable"  ||
            _mimetype == "application/x-shellscript")
   {
    if ( u.isLocalFile() )
    {
      QString path = u.path();
      shellQuote( path );
      return (KRun::run(path)); // just execute the url as a command
    }
  }

  KURL::List lst;
  lst.append( u );

  KService::Ptr offer = KServiceTypeProfile::preferredService( _mimetype, true /*need app*/ );

  if ( !offer )
  {
    // Open-with dialog
    // TODO : pass the mimetype as a parameter, to show it (comment field) in the dialog ! KDE 3.0
    return KOpenWithHandler::getOpenWithHandler()->displayOpenWithDialog( lst );
  }

  return KRun::run( *offer, lst );
}

void KRun::shellQuote( QString &_str )
{
    // Credits to Walter, says Bernd G. :)
    if (_str.isEmpty()) // Don't create an explicit empty parameter
        return;
    QString res = "'";
    res += _str.replace(QRegExp("'"), "'\"'\"'");
    res += "'";
    _str = res;
}

pid_t KRun::run( const KService& _service, const KURL::List& _urls )
{
  kdDebug(7010) << "KRun::run " << _service.desktopEntryPath() << endl;

  if (!_urls.isEmpty()) {
    kdDebug(7010) << "First url " << _urls.first().url() << endl;
  }

  QString exec = _service.exec();
  QString miniicon = _service.icon();
  QString user = _service.username();
  if (_service.substituteUid() && !user.isEmpty())
  {
    if (_service.terminal())
      exec = QString("konsole %1 -e su %2 -c %3").arg(_service.terminalOptions()).arg(user).arg(exec);
    else
      exec = QString("kdesu -u %1 -- %2").arg(user).arg(exec);
  } else if (_service.terminal())
     // Keep in mind that exec could include spaces. No quotes around %2, then.
     exec = QString("konsole %1 -e %2").arg(_service.terminalOptions()).arg(exec);
  else
  {
  QString error;
  int /*pid_t*/ pid;

  // First try using startServiceByDesktopPath, since that one benefits from kdeinit.
  // But only if we don't need kfmexec (i.e. url is local, or app supports remote urls)
  bool b_local_app = ( exec.find( "%u" ) == -1 && exec.find( "%U" ) == -1 );
  bool b_local_files = true;
  KURL::List::ConstIterator it = _urls.begin();
  for( ; it != _urls.end(); ++it )
    if ( !(*it).isLocalFile() )
      b_local_files = false;

  if ( !b_local_app || b_local_files )
  {
    QString error;

    int i = KApplication::startServiceByDesktopPath(
        _service.desktopEntryPath(), _urls.toStringList(), &error, 0L, &pid
        );

    if (i == 0)
    {
      kdDebug(7010) << "startServiceByDesktopPath worked fine" << endl;
      return pid;

    } else {

      kdDebug(7010) << error << endl;
      KMessageBox::sorry( 0L, error );
      return 0;
    }
  }
  }

  // Fall back on normal running
  QString name = _service.name();
  QString icon = _service.icon();
  return KRun::run( exec, _urls, name, icon, miniicon );
}

pid_t KRun::run( const QString& _exec, const KURL::List& _urls, const QString& _name,
                const QString& _icon, const QString& _mini_icon, const QString& _desktop_file )
{
  bool b_local_files = true;

  QString U = "",F = "",D = "",N = "";

  KURL::List::ConstIterator it = _urls.begin();
  for( ; it != _urls.end(); ++it )
  {
    KURL url( *it );
    if ( url.isMalformed() )
    {
      QString tmp = i18n( "Malformed URL" );
      tmp += "\n";
      tmp += (*it).url();
      KMessageBoxWrapper::error( 0L, tmp);
      return 0;
    }

    if ( !url.isLocalFile() )
      b_local_files = false;

    QString tmp = (*it).isLocalFile() ? (*it).path() : (*it).url(); // Non-kde applications prefer paths when possible (eg xmms)
    shellQuote( tmp );
    U += tmp;
    U += " ";
    tmp = url.directory();
    shellQuote( tmp );
    D += tmp;
    D += " ";
    tmp = url.fileName();
    shellQuote( tmp );
    N += tmp;
    N += " ";
    tmp = url.path( -1 );
    shellQuote( tmp );
    F += tmp;
    F += " ";
  }

  QString exec = _exec;
  bool b_local_app = false;
  if ( exec.find( "%u" ) == -1 && exec.find( "%U" ) == -1 )
    b_local_app = true;

  // Did the user forget to append something like '%f' ?
  // If so, then assume that '%f' is the right choice => the application
  // accepts only local files.
  if ( exec.find( "%f" ) == -1 && exec.find( "%u" ) == -1 && exec.find( "%n" ) == -1 &&
       exec.find( "%d" ) == -1 && exec.find( "%F" ) == -1 && exec.find( "%U" ) == -1 &&
       exec.find( "%N" ) == -1 && exec.find( "%D" ) == -1 && exec.find( "%v" ) == -1 )
    exec += " %f";

  // Can we pass multiple files on the command line or do we have to start the application for every single file ?
  bool b_allow_multiple = false;
  if ( exec.find( "%F" ) != -1 || exec.find( "%U" ) != -1 || exec.find( "%N" ) != -1 ||
       exec.find( "%D" ) != -1 )
    b_allow_multiple = true;

  int pos;

  QString name = _name;
  shellQuote( name );
  while ( ( pos = exec.find( "%c" ) ) != -1 )
    exec.replace( pos, 2, name );

  QString icon = _icon;
  shellQuote( icon );
  if ( !icon.isEmpty() )
    icon.insert( 0, "-icon " );
  while ( ( pos = exec.find( "%i" ) ) != -1 )
    exec.replace( pos, 2, icon );

  QString _bin_name = binaryName ( _exec, false /*keep path*/ );

  QString mini_icon = _mini_icon;
  shellQuote( mini_icon );
  if ( !mini_icon.isEmpty() )
    mini_icon.insert( 0, "-miniicon " );
  while ( ( pos = exec.find( "%m" ) ) != -1 )
    exec.replace( pos, 2, mini_icon );

  while ( ( pos = exec.find( "%k" ) ) != -1 )
    exec.replace( pos, 2, _desktop_file );

  while ( ( pos = exec.find( "%v" ) ) != -1 )
  {
      if ( QFile::exists( QFile::encodeName( _desktop_file ) ) )
      {
          KDesktopFile desktopFile(_desktop_file, true);
          exec.replace( pos, 2, desktopFile.readEntry( "Dev" ) );
      }
  }

  // The application accepts only local files ?
  if ( b_local_app && !b_local_files )
  {
      //kdDebug(7010) << "Using runOldApplication" << endl;
      return runOldApplication( exec, _urls, b_allow_multiple );
  }

  pid_t retval = 0;

  if ( b_allow_multiple || _urls.isEmpty() )
  {
    KURL firstURL = (_urls.isEmpty()) ? KURL() : _urls.first();
    while ( ( pos = exec.find( "%f" )) != -1 )
    {
      QString f ( firstURL.path( -1 ) );
      shellQuote( f );
      exec.replace( pos, 2, f );
    }
    while ( ( pos = exec.find( "%n" )) != -1 )
    {
      QString n ( firstURL.fileName() );
      shellQuote( n );
      exec.replace( pos, 2, n );
    }
    while ( ( pos = exec.find( "%d" )) != -1 )
    {
      QString d ( firstURL.directory() );
      shellQuote( d );
      exec.replace( pos, 2, d );
    }
    while ( ( pos = exec.find( "%u" )) != -1 )
    {
      // Non-kde applications prefer paths when possible (eg xmms)
      QString u ( firstURL.isLocalFile() ? firstURL.path() : firstURL.url() );
      shellQuote( u );
      exec.replace( pos, 2, u );
    }

    while ( ( pos = exec.find( "%F" )) != -1 )
      exec.replace( pos, 2, F );
    while ( ( pos = exec.find( "%N" )) != -1 )
      exec.replace( pos, 2, N );
    while ( ( pos = exec.find( "%D" )) != -1 )
      exec.replace( pos, 2, D );
    while ( ( pos = exec.find( "%U" )) != -1 )
      exec.replace( pos, 2, U );

    retval = KRun::runCommand( exec, _bin_name, mini_icon );
  }
  else
  {
    it = _urls.begin();
    for( ; it != _urls.end(); ++it )
    {
      QString e = exec;
      KURL url( *it );
      Q_ASSERT( !url.isMalformed() );
      QString f ( url.path( -1 ) );
      shellQuote( f );
      QString d ( url.directory() );
      shellQuote( d );
      QString n ( url.fileName() );
      shellQuote( n );
      // Non-kde applications prefer paths when possible (eg xmms)
      QString u ( url.isLocalFile() ? url.path() : url.url() );
      shellQuote( u );

      while ( ( pos = e.find( "%f" )) != -1 )
        e.replace( pos, 2, f );
      while ( ( pos = e.find( "%n" )) != -1 )
        e.replace( pos, 2, n );
      while ( ( pos = e.find( "%d" )) != -1 )
        e.replace( pos, 2, d );
      while ( ( pos = e.find( "%u" )) != -1 )
        e.replace( pos, 2, u );

      retval = KRun::runCommand( e, _bin_name, mini_icon );
    }
  }

  return retval;
}

pid_t KRun::runCommand( QString cmd )
{
  return KRun::runCommand( cmd, QString::null, QString::null );
}

pid_t KRun::runCommand( const QString& cmd, const QString &execName, const QString & iconName )
{
  kdDebug(7010) << "runCommand " << cmd << "," << execName << endl;
  KShellProcess * proc = new KShellProcess;
  *proc << cmd;
  return runCommandInternal( proc, binaryName( cmd, false ), execName, iconName );
}

pid_t KRun::runCommandInternal( KProcess* proc, const QString& binName,
    const QString &execName_P, const QString & iconName_P )
{
  QString bin = binaryName( binName, false );
  QString execName = execName_P;
  QString iconName = iconName_P;
#ifdef Q_WS_X11 // Startup notification doesn't work with QT/E, service isn't needed without Startup notification
  KStartupInfoId id;
  // Find service, if any
  KService::Ptr service = 0;
  if( bin[0] == '/' ) // Full path
      service = new KService( bin );
  else
      service = KService::serviceByDesktopName( bin );
#endif
  bool startup_notify = false;
#ifdef Q_WS_X11 // Startup notification doesn't work yet on Qt Embedded
  QCString wmclass;
  if( service != NULL )
  {
      if( service->property( "X-KDE-StartupNotify" ).isValid())
      {
          startup_notify = service->property( "X-KDE-StartupNotify" ).toBool();
          wmclass = service->property( "X-KDE-WMClass" ).toString().latin1();
      }
      else // non-compliant app ( .desktop file )
      {
          if( service->type() == "Application" )
          {
              startup_notify = true; // doesn't have .desktop entries needed
              wmclass = "0";         // start as non-compliant
          }
      }
  }
  if( startup_notify )
  {
      id.initId();
      id.setupStartupEnv();
      if( execName.isEmpty())
          execName = service->name();
      if( iconName.isEmpty())
          iconName = service->icon();
      KStartupInfoData data;
      data.setHostname();
      data.setBin( binaryName( binName, true ));
      data.setName( execName );
      data.setIcon( iconName );
      if( !wmclass.isEmpty())
          data.setWMClass( wmclass );
      data.setDesktop( KWin::currentDesktop());
      KStartupInfo::sendStartup( id, data );
  }
  pid_t pid = KProcessRunner::run( proc, binaryName( binName, true ), id );
  if( startup_notify )
  {
      KStartupInfoData data;
      data.addPid( pid );
      KStartupInfo::sendChange( id, data );
      KStartupInfo::resetStartupEnv();
  }
  return pid;
#else
  return KProcessRunner::run( proc, binaryName( binName, true ) );
#endif
}

pid_t KRun::run( const QString& _cmd )
{
  kdDebug(7010) << "Running " << _cmd << endl;

  KShellProcess * proc = new KShellProcess;
  *proc << _cmd;
  // CHECKME I think it's better without app-start notification here
  // This version can't check whether the binary exists
  return KProcessRunner::run(proc, QString::null);
}


pid_t KRun::runOldApplication( const QString& app, const KURL::List& _urls, bool _allow_multiple )
{
  // find kfmexec in $PATH
  QString kfmexec = KStandardDirs::findExe( "kfmexec" );
  if (kfmexec.isEmpty())
  {
    // this is written this way to avoid a new string to translate
    KMessageBox::sorry( 0L, i18n("Couldn't launch %1").arg( "kfmexec" ) );
    return 0;
  }

  if ( _allow_multiple )
  {
    kdDebug(7010) << "Allow Multiple" << endl;

    KProcess * proc = new KProcess;
    *proc << kfmexec;
    *proc << app;
    KURL::List::ConstIterator it = _urls.begin();
    for( ; it != _urls.end(); ++it )
        *proc << (*it).url();

    return runCommandInternal( proc, binaryName( app, false ), "", "" );
  }
  else
  {
    kdDebug(7010) << "Not multiple" << endl;
    KURL::List::ConstIterator it = _urls.begin();
    pid_t retval = 0;
    for( ; it != _urls.end(); ++it )
    {
        KProcess * proc = new KProcess;
        *proc << kfmexec;
        *proc << app;
        *proc << (*it).url();

        retval = runCommandInternal( proc, binaryName( app, false ), "", "" );
    }

    return retval;
  }
}

QString KRun::binaryName( const QString & execLine, bool removePath )
{
  QString _bin_name = execLine;

  // Remove parameters and/or trailing spaces.

  int firstSpace = _bin_name.find(' ');

  if (-1 != firstSpace)
    _bin_name = _bin_name.left(firstSpace);

  // Remove path if wanted

  return removePath ? _bin_name.mid(_bin_name.findRev('/') + 1) : _bin_name;
}

KRun::KRun( const KURL& _url, mode_t _mode, bool _is_local_file, bool _showProgressInfo )
  : m_timer(0,"KRun::timer")
{
  m_bFault = false;
  m_bAutoDelete = true;
  m_bProgressInfo = _showProgressInfo;
  m_bFinished = false;
  m_job = 0L;
  m_strURL = _url;
  m_bScanFile = false;
  m_bIsDirectory = false;
  m_bIsLocalFile = _is_local_file;
  m_mode = _mode;
  d = new KRunPrivate;

  // Start the timer. This means we will return to the event
  // loop and do initialization afterwards.
  // Reason: We must complete the constructor before we do anything else.
  m_bInit = true;
  connect( &m_timer, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
  m_timer.start( 0, true );
  kdDebug(7010) << " new KRun " << this << " timer=" << &m_timer << endl;
}

void KRun::init()
{
  kdDebug(7010) << "INIT called" << endl;
  if ( m_strURL.isMalformed() )
  {
    d->m_showingError = true;
    KMessageBoxWrapper::error( 0L, i18n( "Malformed URL\n%1" ).arg( m_strURL.url() ) );
    d->m_showingError = false;
    m_bFault = true;
    m_bFinished = true;
    m_timer.start( 0, true );
    return;
  }

  if ( !m_bIsLocalFile && m_strURL.isLocalFile() )

    m_bIsLocalFile = true;

  if ( m_bIsLocalFile )
  {
    if ( m_mode == 0 )
    {
      struct stat buff;
      if ( stat( QFile::encodeName(m_strURL.path()), &buff ) == -1 )
      {
        d->m_showingError = true;
        KMessageBoxWrapper::error( 0L, i18n( "<qt>Unable to run the command specified. The file or directory <b>%1</b> does not exist.</qt>" ).arg( m_strURL.prettyURL() ) );
        d->m_showingError = false;
        m_bFault = true;
        m_bFinished = true;
        m_timer.start( 0, true );
        return;
      }
      m_mode = buff.st_mode;
    }

    KMimeType::Ptr mime = KMimeType::findByURL( m_strURL, m_mode, m_bIsLocalFile );
    assert( mime != 0L );
    kdDebug(7010) << "MIME TYPE is " << mime->name() << endl;
    foundMimeType( mime->name() );
    return;
  }
  else if ( KProtocolInfo::isHelperProtocol( m_strURL ) ) {
    kdDebug(7010) << "Helper protocol" << endl;

    KURL::List urls;
    urls.append( m_strURL );
    QString exec = KProtocolInfo::exec( m_strURL.protocol() );
    run( exec, urls );

    m_bFinished = true;
    // will emit the error and autodelete this
    m_timer.start( 0, true );
    return;
  }

  // Did we already get the information that it is a directory ?
  if ( S_ISDIR( m_mode ) )
  {
    foundMimeType( "inode/directory" );
    return;
  }

  // Let's see whether it is a directory

  if ( !KProtocolInfo::supportsListing( m_strURL ) )
  {
    //kdDebug(7010) << "Protocol has no support for listing" << endl;
    // No support for listing => it can't be a directory (example: http)
    scanFile();
    return;
  }

  kdDebug(7010) << "Testing directory (stating)" << endl;

  // It may be a directory or a file, let's stat
  KIO::StatJob *job = KIO::stat( m_strURL, true, 0 /* no details */, m_bProgressInfo );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           this, SLOT( slotStatResult( KIO::Job * ) ) );
  m_job = job;
}

KRun::~KRun()
{
  kdDebug(7010) << "KRun::~KRun() " << this << endl;
  m_timer.stop();
  killJob();
  kdDebug(7010) << "KRun::~KRun() done " << this << endl;
  delete d;
}

void KRun::scanFile()
{
  kdDebug(7010) << "###### KRun::scanFile " << m_strURL.url() << endl;
  // First, let's check for well-known extensions
  // Not when there is a query in the URL, in any case.
  if ( m_strURL.query().isEmpty() )
  {
    KMimeType::Ptr mime = KMimeType::findByURL( m_strURL );
    assert( mime != 0L );
    if ( mime->name() != "application/octet-stream" || m_bIsLocalFile )
    {
      kdDebug(7010) << "Scanfile: MIME TYPE is " << mime->name() << endl;
      foundMimeType( mime->name() );
      return;
    }
  }

  // No mimetype found, and the URL is not local  (or fast mode not allowed).
  // We need to apply the 'KIO' method, i.e. either asking the server ot
  // getting some data out of the file, to know what mimetype it is.

  if ( !KProtocolInfo::supportsReading( m_strURL ) )
  {
    kdError(7010) << "#### NO SUPPORT FOR READING!" << endl;
    m_bFault = true;
    m_bFinished = true;
    m_timer.start( 0, true );
    return;
  }
  kdDebug(7010) << this << " Scanning file " << m_strURL.url() << endl;

  KIO::TransferJob *job = KIO::get( m_strURL, m_bProgressInfo );
  connect(job, SIGNAL( result(KIO::Job *)),
          this, SLOT( slotScanFinished(KIO::Job *)));
  connect(job, SIGNAL( mimetype(KIO::Job *, const QString &)),
          this, SLOT( slotScanMimeType(KIO::Job *, const QString &)));
  m_job = job;
}

void KRun::slotTimeout()
{
  kdDebug(7010) << this << " slotTimeout called" << endl;
  if ( m_bInit )
  {
    m_bInit = false;
    init();
    return;
  }

  if ( m_bFault ){
      emit error();
  }
  if ( m_bFinished ){
      emit finished();
  }

  if ( m_bScanFile )
  {
    m_bScanFile = false;
    scanFile();
    return;
  }
  else if ( m_bIsDirectory )
  {
    m_bIsDirectory = false;
    foundMimeType( "inode/directory" );
    return;
  }

  if ( m_bAutoDelete )
  {
    delete this;
    return;
  }
}

void KRun::slotStatResult( KIO::Job * job )
{
  m_job = 0L;
  if (job->error())
  {
    d->m_showingError = true;
    kdError(7010) << this << " ERROR " << job->error() << " " << job->errorString() << endl;
    job->showErrorDialog();
    kdDebug(7010) << this << " KRun returning from showErrorDialog, starting timer to delete us" << endl;
    d->m_showingError = false;

    m_bFault = true;
    m_bFinished = true;

    // will emit the error and autodelete this
    m_timer.start( 0, true );

  } else {

    kdDebug(7010) << "Finished" << endl;

    KIO::UDSEntry entry = ((KIO::StatJob*)job)->statResult();
    KIO::UDSEntry::ConstIterator it = entry.begin();
    for( ; it != entry.end(); it++ ) {
        if ( (*it).m_uds == KIO::UDS_FILE_TYPE )
        {
            if ( S_ISDIR( (mode_t)((*it).m_long) ) )
                m_bIsDirectory = true; // it's a dir
            else
                m_bScanFile = true; // it's a file
            break;
        }
    }
    // We should have found something
    assert ( m_bScanFile || m_bIsDirectory );

    // Start the timer. Once we get the timer event this
    // protocol server is back in the pool and we can reuse it.
    // This gives better performance than starting a new slave
    m_timer.start( 0, true );
  }
}

void KRun::slotScanMimeType( KIO::Job *job, const QString &mimetype )
{
  if ( mimetype.isEmpty() )
    kdWarning(7010) << "KRun::slotScanFinished : MimetypeJob didn't find a mimetype! Probably a kioslave bug." << endl;
  foundMimeType( mimetype );
  m_job = 0;
}

void KRun::slotScanFinished( KIO::Job *job )
{
  m_job = 0;
  slotStatResult( job ); // hacky - we just want to use the same code on error
  return;
}

void KRun::foundMimeType( const QString& type )
{
  kdDebug(7010) << "Resulting mime type is " << type << endl;

/*
  // Automatically unzip stuff

  // Disabled since the new KIO doesn't have filters yet.

  if ( type == "application/x-gzip"  ||
       type == "application/x-bzip"  ||
       type == "application/x-bzip2"  )
  {
    KURL::List lst = KURL::split( m_strURL );
    if ( lst.isEmpty() )
    {
      QString tmp = i18n( "Malformed URL" );
      tmp += "\n";
      tmp += m_strURL.url();
      KMessageBoxWrapper::error( 0L, tmp );
      return;
    }

    if ( type == "application/x-gzip" )
      lst.prepend( KURL( "gzip:/decompress" ) );
    else if ( type == "application/x-bzip" )
      lst.prepend( KURL( "bzip:/decompress" ) );
    else if ( type == "application/x-bzip2" )
      lst.prepend( KURL( "bzip2:/decompress" ) );
    else if ( type == "application/x-tar" )
      lst.prepend( KURL( "tar:/" ) );

    // Move the HTML style reference to the leftmost URL
    KURL::List::Iterator it = lst.begin();
    ++it;
    (*lst.begin()).setRef( (*it).ref() );
    (*it).setRef( QString::null );

    // Create the new URL
    m_strURL = KURL::join( lst );

    kdDebug(7010) << "Now trying with " << debugString(m_strURL.url()) << endl;

    killJob();

    // We don't know if this is a file or a directory. Let's test this first.
    // (For instance a tar.gz is a directory contained inside a file)
    // It may be a directory or a file, let's stat
    KIO::StatJob *job = KIO::stat( m_strURL, m_bProgressInfo );
    connect( job, SIGNAL( result( KIO::Job * ) ),
             this, SLOT( slotStatResult( KIO::Job * ) ) );
    m_job = job;

    return;
  }
*/
  if (m_job && m_job->inherits("KIO::TransferJob"))
  {
     KIO::TransferJob *job = static_cast<KIO::TransferJob *>(m_job);
     job->putOnHold();
     KIO::Scheduler::publishSlaveOnHold();
     m_job = 0;
  }

  if (KRun::runURL( m_strURL, type )){
    m_bFinished = true;
  }
  else{
    m_bFinished = true;
    m_bFault = true;
  }

  m_timer.start( 0, true );
}

void KRun::killJob()
{
  if ( m_job )
  {
    kdDebug(7010) << "KRun::killJob run=" << this << " m_job=" << m_job << endl;
    m_job->kill();
    m_job = 0L;
  }
}

void KRun::abort()
{
  killJob();
  // If we're showing an error message box, the rest will be done
  // after closing the msgbox -> don't autodelete nor emit signals now.
  if ( d->m_showingError )
    return;
  m_bFault = true;
  m_bFinished = true;

  // will emit the error and autodelete this
  m_timer.start( 0, true );
}

// obsolete
void KRun::clientStarted(
  const QString &,
  const QString &,
  pid_t,
  const QString &,
  bool,
  int
)
{
  kdDebug(7010) << "obsolete clientStarted called" << endl;
}

/****************/
bool KOpenWithHandler::displayOpenWithDialog( const KURL::List& )
{
    kdError(7010) << "displayOpenWithDialog : Application " << kapp->name()
                  << " - should create a KFileOpenWithHandler !" << endl;
    return 0;
}

pid_t
KProcessRunner::run(KProcess * p, const QString & binName)
{
  return (new KProcessRunner(p, binName))->pid();
}

#ifdef Q_WS_X11
pid_t
KProcessRunner::run(KProcess * p, const QString & binName, const KStartupInfoId& id )
{
  return (new KProcessRunner(p, binName, id))->pid();
}
#endif

KProcessRunner::KProcessRunner(KProcess * p, const QString & _binName )
  : QObject(),
    process_(p),
    binName( _binName )
{
  QObject::connect(
      process_, SIGNAL(processExited(KProcess *)),
      this,     SLOT(slotProcessExited(KProcess *)));

  process_->start();
}

#ifdef Q_WS_X11
KProcessRunner::KProcessRunner(KProcess * p, const QString & _binName, const KStartupInfoId& id )
  : QObject(),
    process_(p),
    binName( _binName ),
    id_( id )
{
  QObject::connect(
      process_, SIGNAL(processExited(KProcess *)),
      this,     SLOT(slotProcessExited(KProcess *)));

  process_->start();
}
#endif

KProcessRunner::~KProcessRunner()
{
  delete process_;
}

  pid_t
KProcessRunner::pid() const
{
  return process_->pid();
}

  void
KProcessRunner::slotProcessExited(KProcess * p)
{
  if (p != process_)
    return; // Eh ?

  kdDebug(7010) << "slotProcessExited " << binName << endl;
  kdDebug(7010) << "normalExit " << process_->normalExit() << endl;
  kdDebug(7010) << "exitStatus " << process_->exitStatus() << endl;
  if ( !binName.isEmpty() && process_->normalExit()
          && ( process_->exitStatus() == 127 || process_->exitStatus() == 1 ) )
  {
    // Often we get 1 (zsh, csh) or 127 (ksh, bash) because the binary doesn't exist.
    // We can't just rely on that, but it's a good hint.
    if ( KStandardDirs::findExe( binName ).isEmpty() )
    {
      kapp->ref();
      KMessageBox::sorry( 0L, i18n("Couldn't find the program '%1'").arg( binName ) );
      kapp->deref();
    }
  }
#ifdef Q_WS_X11
  if( !id_.none())
  {
      KStartupInfoData data;
      data.addPid( pid()); // announce this pid for the startup notification has finished
      data.setHostname();
      KStartupInfo::sendFinish( id_, data );
  }
#endif
  delete this;
}

#include "krun.moc"
