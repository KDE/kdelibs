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

#include <kmessageboxwrapper.h>
#include <kurl.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kprotocolmanager.h>
#include <kstddirs.h>
#include <kprocess.h>
#include <dcopclient.h>
#include <qfile.h>
#include <qtextstream.h>
#include <kwin.h>
#include <kdesktopfile.h>

KOpenWithHandler * KOpenWithHandler::pOpenWithHandler = 0L;

bool KRun::runURL( const KURL& u, const QString& _mimetype )
{

  if ( _mimetype == "inode/directory-locked" )
  {
    KMessageBoxWrapper::error( 0L,
            i18n("<qt>Unable to enter <b>%1</b>.\nYou do not have access rights to this location.</qt>").arg(u.url()) );
    return false;
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
      return (KRun::run( u.path() ) != -1); // just execute the url as a command
  }

  KURL::List lst;
  lst.append( u );

  KService::Ptr offer = KServiceTypeProfile::preferredService( _mimetype, true /*need app*/ );

  if ( !offer )
  {
    // Open-with dialog
    return KOpenWithHandler::getOpenWithHandler()->displayOpenWithDialog( lst );
  }

  return KRun::run( *offer, lst );
}

void KRun::shellQuote( QString &_str )
{
    // Credits to Walter, says Bernd G. :)
    QString res = "'";
    res += _str.replace(QRegExp("'"), "'\"'\"'");
    res += "'";
    _str = res;
}

bool KRun::run( const KService& _service, const KURL::List& _urls )
{
  kdDebug(7010) << "KRun::run " << _service.desktopEntryPath() << endl;
  QString miniicon = _service.icon();
  QString exec = _service.exec();
  QString error;
  int /*pid_t*/ pid;
  // First try using startServiceByDesktopPath, since that one benefits from kdeinit.
  if ( KApplication::startServiceByDesktopPath( _service.desktopEntryPath(), _urls.toStringList(), 0L, 0L, &pid ) == 0 )
  {
    //kdDebug(7010) << "startServiceByDesktopPath worked fine!" << endl;
    // App-starting notification
    clientStarted( binaryName(exec), miniicon, pid);
    return true;
  }
  else
  {
    // Fall back on normal running
    QString name = _service.name();
    QString icon = _service.icon();
    bool ret = run( _service.exec(), _urls, name, icon, miniicon );
    //kdDebug(7010) << "KRun::run returned " << ret << endl;
    return ret;
  }
}

bool KRun::run( const QString& _exec, const KURL::List& _urls, const QString& _name,
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
      return false;
    }

    if ( !url.isLocalFile() )
      b_local_files = false;

    QString tmp = (*it).url();
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
  if ( exec.find( "%u" ) == -1 )
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

  QString _bin_name = binaryName ( _exec );

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

  bool retval = true;

  if ( b_allow_multiple || _urls.isEmpty() )
  {
    while ( ( pos = exec.find( "%f" )) != -1 )
      exec.replace( pos, 2, "" );
    while ( ( pos = exec.find( "%n" )) != -1 )
      exec.replace( pos, 2, "" );
    while ( ( pos = exec.find( "%d" )) != -1 )
      exec.replace( pos, 2, "" );
    while ( ( pos = exec.find( "%u" )) != -1 )
      exec.replace( pos, 2, "" );

    while ( ( pos = exec.find( "%F" )) != -1 )
      exec.replace( pos, 2, F );
    while ( ( pos = exec.find( "%N" )) != -1 )
      exec.replace( pos, 2, N );
    while ( ( pos = exec.find( "%D" )) != -1 )
      exec.replace( pos, 2, D );
    while ( ( pos = exec.find( "%U" )) != -1 )
      exec.replace( pos, 2, U );

    pid_t pid = run(exec);

    // App starting notification.

    if (pid == -1)
      retval = false;
    else
      clientStarted(_bin_name, mini_icon, pid);
  }
  else
  {
    it = _urls.begin();
    for( ; it != _urls.end(); ++it )
    {
      QString e = exec;
      KURL url( *it );
      ASSERT( !url.isMalformed() );
      QString f ( url.path( -1 ) );
      shellQuote( f );
      QString d ( url.directory() );
      shellQuote( d );
      QString n ( url.fileName() );
      shellQuote( n );
      QString u ( (*it).url() );
      shellQuote( u );

      while ( ( pos = e.find( "%f" )) != -1 )
        e.replace( pos, 2, f );
      while ( ( pos = e.find( "%n" )) != -1 )
        e.replace( pos, 2, n );
      while ( ( pos = e.find( "%d" )) != -1 )
        e.replace( pos, 2, d );
      while ( ( pos = e.find( "%u" )) != -1 )
        e.replace( pos, 2, u );

      pid_t pid = run(e);

      // App starting notification.

      if (pid == -1)
        retval = false;
      else
        clientStarted(_bin_name, mini_icon, pid);
    }
  }

  return retval;
}

pid_t KRun::run( const QString& _cmd )
{
  kdDebug(7010) << "Running " << _cmd << endl;

  // Figure out current desktop
  int desktop = KWin::currentDesktop();

  KShellProcess proc;

  QString lib = libmapnotify();

  // If we have the notify lib somewhere, use it.

  if (!lib.isEmpty()) {

    // Hack to work around csh being non-sh-compatible.

    QString prefix =
      (QString(getenv("SHELL")).right(3) == "csh") ?
      "setenv LD_PRELOAD %1 ; setenv X_INITIAL_DESKTOP %2 ;" :
      "LD_PRELOAD=%1 X_INITIAL_DESKTOP=%2 ";

    proc << prefix.arg(lib).arg(desktop);
    kdDebug(7010) << prefix.arg(lib).arg(desktop) << endl;
  }

  proc << _cmd;
  proc.start(KShellProcess::DontCare);

  return proc.getPid();
}

bool KRun::runOldApplication( const QString& app, const KURL::List& _urls, bool _allow_multiple )
{
  // find kfmexec in $PATH
  QString kfmexec = KStandardDirs::findExe( "kfmexec" );

  // Hey, did someone forget about the case where the app
  // is given without any arguments ?? (DA)
  if( _urls.isEmpty() )
  {
    KProcess proc;
    proc << kfmexec;
    proc << app;
    proc.start(KProcess::DontCare);
    pid_t pid = proc.getPid();

    if ( pid != -1 )
      clientStarted(app, "" /* mini_icon */, pid);

    return (pid != -1);
  }

  if ( _allow_multiple )
  {
    kdDebug(7010) << "Allow Multiple" << endl;

    KProcess proc;
    proc << kfmexec;
    proc << app;
    KURL::List::ConstIterator it = _urls.begin();
    for( ; it != _urls.end(); ++it )
        proc << (*it).url();
    proc.start(KProcess::DontCare);

    pid_t pid = proc.getPid();

    if ( pid != -1 )
      clientStarted(app, "" /* mini_icon */, pid);

    return (pid != -1);
  }
  else
  {
    kdDebug(7010) << "Not multiple" << endl;
    KURL::List::ConstIterator it = _urls.begin();
    bool retval = true;
    for( ; it != _urls.end(); ++it )
    {
        KProcess proc;
        proc << kfmexec;
        proc << app;
        proc << (*it).url();
        proc.start(KProcess::DontCare);

        pid_t pid = proc.getPid();
        if (pid == -1)
          retval = false;
        else
          clientStarted(app, "" /* mini_icon */, pid);
    }

    return retval;
  }
}

QString KRun::binaryName( const QString & execLine )
{
  QString _bin_name = execLine;

  // Remove parameters and/or trailing spaces.

  int firstSpace = _bin_name.find(' ');

  if (-1 != firstSpace)
    _bin_name = _bin_name.left(firstSpace);

  // Remove path.

  int lastSlash = _bin_name.findRev('/');

  return _bin_name.mid(lastSlash + 1);
}

KRun::KRun( const KURL& _url, mode_t _mode, bool _is_local_file, bool _auto_delete )
{
  m_bFault = false;
  m_bAutoDelete = _auto_delete;
  m_bFinished = false;
  m_job = 0L;
  m_strURL = _url;
  m_bScanFile = false;
  m_bIsDirectory = false;
  m_bIsLocalFile = _is_local_file;
  m_mode = _mode;

  // Start the timer. This means we will return to the event
  // loop and do initialization afterwards.
  // Reason: We must complete the constructor before we do anything else.
  m_bInit = true;
  connect( &m_timer, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
  m_timer.start( 0, true );

}

QString KRun::libmapnotify()
{
  static QString libkmapnotify("(none)");

  if (libkmapnotify != "(none)")
    return libkmapnotify;

  // Look for the libkmapnotify by searching through the .la file
  QString lib = "";
  QString la_file = locate("lib", "libkmapnotify.la");
  if (!la_file.isEmpty())
    {
      QFile la(la_file);
      if (la.open(IO_ReadOnly))
	{
	  QTextStream is(&la);
	
	  QString line;
	  while (!is.atEnd())
	    {
	      line = is.readLine();
	      if (line.left(15) == "library_names='")
		{
		  lib = line.mid(15);
		  int pos = lib.find(" ");
		  if (pos > 0)
		    lib = lib.left(pos);
		}
	    }

	  la.close();
	}

      // look up the path
      if (!lib.isEmpty())
	lib = locate("lib", lib);
    }
  kdDebug(7010) << "Found libkmapnotify at: " << lib << endl;

  if (!lib.isEmpty())
    libkmapnotify = lib;

  return lib;
}

void KRun::init()
{
  kdDebug(7010) << "INIT called" << endl;

  if ( m_strURL.protocol() == "mailto" )
  {
    emit finished();

    QString addr = m_strURL.path();
    QString subj;

    if (m_strURL.query().left(9) == "?subject=")
    {
       subj = KURL::decode_string( m_strURL.query().mid(9) );
    }

    kapp->invokeMailer( addr, subj );

    if ( m_bAutoDelete )
      delete this;

    return;
  }

  if ( !m_bIsLocalFile && m_strURL.isLocalFile() )

    m_bIsLocalFile = true;

  if ( m_bIsLocalFile )
  {
    if ( m_mode == 0 )
    {
      struct stat buff;
      if ( stat( m_strURL.path(), &buff ) == -1 )
      {
	KMessageBoxWrapper::error( 0L, i18n( "<qt>Unable to run the command specified. The file or directory <b>%1</b> does not exist.</qt>" ).arg( m_strURL.url() ) );
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
  else if ( KProtocolManager::self().isHelperProtocol( m_strURL.protocol() ) ) {
    kdDebug(7010) << "Helper protocol" << endl;

    emit finished();

    KURL::List urls;
    urls.append( m_strURL );
    run( KProtocolManager::self().exec( m_strURL.protocol() ),
	 urls );

    kdDebug(7010) << "Launched helper:" << endl;

    if ( m_bAutoDelete )
      delete this;

    return;
  }

  // Did we already get the information that it is a directory ?
  if ( S_ISDIR( m_mode ) )
  {
    foundMimeType( "inode/directory" );
    return;
  }

  // Let's see whether it is a directory
  if ( !KProtocolManager::self().supportsListing( m_strURL.protocol() ) )
  {
    kdDebug(7010) << "##### NO SUPPORT FOR LISTING" << endl;
    // No support for listing => we can scan the file
    scanFile();
    return;
  }

  // Let's see whether it is a directory
  kdDebug(7010) << "##### TESTING DIRECTORY (STATING)" << endl;

  // It may be a directory or a file, let's stat
  KIO::StatJob *job = KIO::stat( m_strURL );
  connect( job, SIGNAL( result( KIO::Job * ) ),
	   this, SLOT( slotStatResult( KIO::Job * ) ) );
  m_job = job;
}

KRun::~KRun()
{
  kdDebug(7010) << "KRun::~KRun()" << endl;
  m_timer.stop();
  killJob();
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

  if ( !KProtocolManager::self().supportsReading( m_strURL.protocol() ) )
  {
    kdError(7010) << "#### NO SUPPORT FOR READING!" << endl;
    m_bFault = true;
    m_bFinished = true;
    m_timer.start( 0, true );
    return;
  }
  kdDebug(7010) << "###### Scanning file " << m_strURL.url() << endl;

  KIO::MimetypeJob *job = KIO::mimetype( m_strURL);
  connect(job, SIGNAL( result( KIO::Job *)),
          this, SLOT( slotScanFinished(KIO::Job *)));
  m_job = job;
}

void KRun::slotTimeout()
{
  kdDebug(7010) << "slotTimeout called" << endl;
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
    kdError(7010) << "ERROR " << job->error() << " " << job->errorText() << endl;
    job->showErrorDialog();

    m_bFault = true;
    m_bFinished = true;

    // will emit the error and autodelete this
    m_timer.start( 0, true );

  } else {

    kdDebug(7010) << "####### FINISHED" << endl;

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

void KRun::slotScanFinished( KIO::Job *job )
{
  m_job = 0;
  if (job->error())
  {
     slotStatResult( job ); // hacky - we just want to use the same code on error
     return;
  }
  foundMimeType( ((KIO::MimetypeJob *)job)->mimetype() );
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
    KIO::StatJob *job = KIO::stat( m_strURL );
    connect( job, SIGNAL( result( KIO::Job * ) ),
	     this, SLOT( slotStatResult( KIO::Job * ) ) );
    m_job = job;

    return;
  }
*/

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
    m_job->kill();
    m_job = 0L;
  }
}

void KRun::clientStarted(
  const QString & execName,
  const QString & iconName,
  pid_t pid
)
{
  kdDebug(7010) << "clientStarted pid=" << (int)pid << endl;
  QByteArray params;
  QDataStream stream(params, IO_WriteOnly);
  stream << execName << iconName << (int)pid;
  kapp->dcopClient()->send(
    "kicker",
    "TaskbarApplet",
    "clientStarted(QString,QString,int)",
    params
  );
}

/****************/
bool KOpenWithHandler::displayOpenWithDialog( const KURL::List& )
{
    kdError(7010) << "displayOpenWithDialog : Application " << kapp->name()
                  << " - should create a KFileOpenWithHandler !" << endl;
    return false;
}

#include "krun.moc"
