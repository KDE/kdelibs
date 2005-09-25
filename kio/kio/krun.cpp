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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "krun.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <typeinfo>

#include <qwidget.h>
#include <qpointer.h>

#include "kuserprofile.h"
#include "kmimetype.h"
#include "kmimemagic.h"
#include "kio/job.h"
#include "kio/global.h"
#include "kio/scheduler.h"
#include "kio/netaccess.h"
#include "kfile/kopenwith.h"
#include "kfile/krecentdocument.h"

#include <kdatastream.h>
#include <kmessageboxwrapper.h>
#include <kurl.h>
#include <kapplication.h>
#include <ktoolinvocation.h>
#include <kauthorized.h>
#include <kdebug.h>
#include <klocale.h>
#include <kprotocolinfo.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <dcopclient.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qdatetime.h>
#include <qregexp.h>
#include <kdesktopfile.h>
#include <kstartupinfo.h>
#include <kmacroexpander.h>
#include <kshell.h>
#include <kde_file.h>

#ifdef Q_WS_X11
#include <kwin.h>
#endif

class KRun::KRunPrivate
{
public:
    KRunPrivate() { m_showingError = false; }

    bool m_showingError;
    bool m_runExecutables;

    QString m_preferredService;
    QString m_externalBrowser;
    QString m_localPath;
    QPointer <QWidget> m_window;
};

pid_t KRun::runURL( const KURL& u, const QString& _mimetype )
{
    return runURL( u, _mimetype, false, true );
}

pid_t KRun::runURL( const KURL& u, const QString& _mimetype, bool tempFile )
{
    return runURL( u, _mimetype, tempFile, true );
}

bool KRun::isExecutableFile( const KURL& url, const QString &mimetype )
{
  if ( !url.isLocalFile() )
     return false;
  QFileInfo file( url.path() );
  if ( file.isExecutable() )  // Got a prospective file to run
  {
    KMimeType::Ptr mimeType = KMimeType::mimeType( mimetype );

    if ( mimeType->is("application/x-executable") || mimeType->is("application/x-executable-script") )
      return true;
  }
  return false;
}

// This is called by foundMimeType, since it knows the mimetype of the URL
pid_t KRun::runURL( const KURL& u, const QString& _mimetype, bool tempFile, bool runExecutables )
{
  bool noRun = false;
  bool noAuth = false;
  if ( _mimetype == "inode/directory-locked" )
  {
    KMessageBoxWrapper::error( 0L,
            i18n("<qt>Unable to enter <b>%1</b>.\nYou do not have access rights to this location.</qt>").arg(u.htmlURL()) );
    return 0;
  }
  else if ( _mimetype == "application/x-desktop" )
  {
    if ( u.isLocalFile() && runExecutables )
      return KDEDesktopMimeType::run( u, true );
  }
  else if ( isExecutableFile(u, _mimetype) )
  {
    if ( u.isLocalFile() && runExecutables)
    {
      if (KAuthorized::authorize("shell_access"))
      {
        QString path = u.path();
        shellQuote( path );
        return (KRun::runCommand(path)); // just execute the url as a command
        // ## TODO implement deleting the file if tempFile==true
      }
      else
      {
        noAuth = true;
      }
    }
    else if (_mimetype == "application/x-executable")
      noRun = true;
  }
  else if ( isExecutable(_mimetype) )
  {
    if (!runExecutables)
      noRun = true;

    if (!KAuthorized::authorize("shell_access"))
      noAuth = true;
  }

  if ( noRun )
  {
    KMessageBox::sorry( 0L,
        i18n("<qt>The file <b>%1</b> is an executable program. "
             "For safety it will not be started.</qt>").arg(u.htmlURL()));
    return 0;
  }
  if ( noAuth )
  {
    KMessageBoxWrapper::error( 0L,
        i18n("<qt>You do not have permission to run <b>%1</b>.</qt>").arg(u.htmlURL()) );
    return 0;
  }

  KURL::List lst;
  lst.append( u );

  static const QString& app_str = KGlobal::staticQString("Application");

  KService::Ptr offer = KServiceTypeProfile::preferredService( _mimetype, app_str );

  if ( !offer )
  {
    // Open-with dialog
    // TODO : pass the mimetype as a parameter, to show it (comment field) in the dialog !
    // Hmm, in fact KOpenWithDlg::setServiceType already guesses the mimetype from the first URL of the list...
    return displayOpenWithDialog( lst, tempFile );
  }

  return KRun::run( *offer, lst, tempFile );
}

bool KRun::displayOpenWithDialog( const KURL::List& lst )
{
    return displayOpenWithDialog( lst, false );
}

bool KRun::displayOpenWithDialog( const KURL::List& lst, bool tempFiles )
{
    if (kapp && !KAuthorized::authorizeKAction("openwith"))
    {
       // TODO: Better message, i18n freeze :-(
       KMessageBox::sorry(0L, i18n("You are not authorized to open this file."));
       return false;
    }

    KOpenWithDlg l( lst, i18n("Open with:"), QString::null, 0L );
    if ( l.exec() )
    {
      KService::Ptr service = l.service();
      if ( !!service )
        return KRun::run( *service, lst, tempFiles );

      kdDebug(7010) << "No service set, running " << l.text() << endl;
      return KRun::run( l.text(), lst ); // TODO handle tempFiles
    }
    return false;
}

void KRun::shellQuote( QString &_str )
{
    // Credits to Walter, says Bernd G. :)
    if (_str.isEmpty()) // Don't create an explicit empty parameter
        return;
    QChar q('\'');
    _str.replace(q, "'\\''").prepend(q).append(q);
}


class KRunMX1 : public KMacroExpanderBase {
public:
    KRunMX1( const KService &_service ) :
        KMacroExpanderBase( '%' ), hasUrls( false ), hasSpec( false ), service( _service ) {}
    bool hasUrls:1, hasSpec:1;

protected:
    virtual int expandEscapedMacro( const QString &str, int pos, QStringList &ret );

private:
    const KService &service;
};

int
KRunMX1::expandEscapedMacro( const QString &str, int pos, QStringList &ret )
{
   uint option = str[pos + 1].unicode();
   switch( option ) {
   case 'c':
      ret << service.name().replace( '%', "%%" );
      break;
   case 'k':
      ret << service.desktopEntryPath().replace( '%', "%%" );
      break;
   case 'i':
      ret << "-icon" << service.icon().replace( '%', "%%" );
      break;
   case 'm':
      ret << "-miniicon" << service.icon().replace( '%', "%%" );
      break;
   case 'u':
   case 'U':
      hasUrls = true;
      /* fallthrough */
   case 'f':
   case 'F':
   case 'n':
   case 'N':
   case 'd':
   case 'D':
   case 'v':
      hasSpec = true;
      /* fallthrough */
   default:
      return -2; // subst with same and skip
   }
   return 2;
}

class KRunMX2 : public KMacroExpanderBase {
public:
    KRunMX2( const KURL::List &_urls ) :
        KMacroExpanderBase( '%' ), ignFile( false ), urls( _urls ) {}
    bool ignFile:1;

protected:
    virtual int expandEscapedMacro( const QString &str, int pos, QStringList &ret );

private:
    void subst( int option, const KURL &url, QStringList &ret );

    const KURL::List &urls;
};

void
KRunMX2::subst( int option, const KURL &url, QStringList &ret )
{
   switch( option ) {
   case 'u':
      ret << (url.isLocalFile() ? url.path() : url.url());
      break;
   case 'd':
      ret << url.directory();
      break;
   case 'f':
      ret << url.path();
      break;
   case 'n':
      ret << url.fileName();
      break;
   case 'v':
      if (url.isLocalFile() && QFile::exists( url.path() ) )
          ret << KDesktopFile( url.path(), true ).readEntry( "Dev" );
      break;
   }
   return;
}

int
KRunMX2::expandEscapedMacro( const QString &str, int pos, QStringList &ret )
{
   uint option = str[pos + 1].unicode();
   switch( option ) {
   case 'f':
   case 'u':
   case 'n':
   case 'd':
   case 'v':
      if( urls.isEmpty() ) {
         if (!ignFile)
            kdDebug() << "KRun::processDesktopExec: No URLs supplied to single-URL service " << str << endl;
      } else if( urls.count() > 1 )
          kdWarning() << "KRun::processDesktopExec: " << urls.count() << " URLs supplied to single-URL service " << str << endl;
      else
         subst( option, urls.first(), ret );
      break;
   case 'F':
   case 'U':
   case 'N':
   case 'D':
      option += 'a' - 'A';
      for( KURL::List::ConstIterator it = urls.begin(); it != urls.end(); ++it )
         subst( option, *it, ret );
      break;
   case '%':
      ret = QStringList(QLatin1String("%"));
      break;
   default:
      return -2; // subst with same and skip
   }
   return 2;
}

// BIC: merge with method below
QStringList KRun::processDesktopExec(const KService &_service, const KURL::List& _urls, bool has_shell) {
    return processDesktopExec( _service, _urls, has_shell, false );
}

QStringList KRun::processDesktopExec(const KService &_service, const KURL::List& _urls, bool has_shell /* KDE4: remove */, bool tempFiles)
{
  QString exec = _service.exec();
  QStringList result;
  bool appHasTempFileOption;

  KRunMX1 mx1( _service );
  KRunMX2 mx2( _urls );

  if( !mx1.expandMacrosShellQuote( exec ) )
    goto synerr; // error in shell syntax

  // FIXME: the current way of invoking kioexec disables term and su use

  // Check if we need "tempexec" (kioexec in fact)
  appHasTempFileOption = tempFiles && _service.property("X-KDE-HasTempFileOption").toBool();
  if( tempFiles && !appHasTempFileOption ) {
    result << "kioexec" << "--tempfiles" << exec;
    result += _urls.toStringList();
    if (has_shell)
      result = QStringList( KShell::joinArgs( result ) );
    return result;
  }

  // Check if we need kioexec
  if( !mx1.hasUrls ) {
    for( KURL::List::ConstIterator it = _urls.begin(); it != _urls.end(); ++it )
      if ( !(*it).isLocalFile() && !KProtocolInfo::isHelperProtocol(*it) ) {
        // We need to run the app through kioexec
        result << "kioexec";
        if ( tempFiles )
            result << "--tempfiles";
        result << exec;
        result += _urls.toStringList();
        if (has_shell)
          result = QStringList( KShell::joinArgs( result ) );
        return result;
      }
  }

  if ( appHasTempFileOption )
      exec += " --tempfile";

  // Did the user forget to append something like '%f'?
  // If so, then assume that '%f' is the right choice => the application
  // accepts only local files.
  if( !mx1.hasSpec ) {
    exec += " %f";
    mx2.ignFile = true;
  }

  mx2.expandMacrosShellQuote( exec ); // syntax was already checked, so don't check return value

/*
 1 = need_shell, 2 = terminal, 4 = su, 8 = has_shell

 0                                                           << split(cmd)
 1                                                           << "sh" << "-c" << cmd
 2 << split(term) << "-e"                                    << split(cmd)
 3 << split(term) << "-e"                                    << "sh" << "-c" << cmd

 4                        << "kdesu" << "-u" << user << "-c" << cmd
 5                        << "kdesu" << "-u" << user << "-c" << ("sh -c " + quote(cmd))
 6 << split(term) << "-e" << "su"            << user << "-c" << cmd
 7 << split(term) << "-e" << "su"            << user << "-c" << ("sh -c " + quote(cmd))

 8                                                           << cmd
 9                                                           << cmd
 a << term        << "-e"                                    << cmd
 b << term        << "-e"                                    << ("sh -c " + quote(cmd))

 c                        << "kdesu" << "-u" << user << "-c" << quote(cmd)
 d                        << "kdesu" << "-u" << user << "-c" << quote("sh -c " + quote(cmd))
 e << term        << "-e" << "su"            << user << "-c" << quote(cmd)
 f << term        << "-e" << "su"            << user << "-c" << quote("sh -c " + quote(cmd))

 "sh -c" is needed in the "su" case, too, as su uses the user's login shell, not sh.
 this could be optimized with the -s switch of some su versions (e.g., debian linux).
*/

  if (_service.terminal()) {
    KConfigGroupSaver gs(KGlobal::config(), "General");
    QString terminal = KGlobal::config()->readPathEntry("TerminalApplication", "konsole");
    if (terminal == "konsole")
      terminal += " -caption=%c %i %m";
    terminal += " ";
    terminal += _service.terminalOptions();
    if( !mx1.expandMacrosShellQuote( terminal ) ) {
      kdWarning() << "KRun: syntax error in command `" << terminal << "', service `" << _service.name() << "'" << endl;
      return QStringList();
    }
    mx2.expandMacrosShellQuote( terminal );
    if (has_shell)
      result << terminal;
    else
      result = KShell::splitArgs( terminal ); // assuming that the term spec never needs a shell!
    result << "-e";
  }

  int err;
  if (_service.substituteUid()) {
    if (_service.terminal())
      result << "su";
    else
      result << "kdesu" << "-u";
    result << _service.username() << "-c";
    KShell::splitArgs(exec, KShell::AbortOnMeta | KShell::TildeExpand, &err);
    if (err == KShell::FoundMeta) {
      shellQuote( exec );
      exec.prepend( "/bin/sh -c " );
    } else if (err != KShell::NoError)
      goto synerr;
    if (has_shell)
      shellQuote( exec );
    result << exec;
  } else {
    if (has_shell) {
      if (_service.terminal()) {
        KShell::splitArgs(exec, KShell::AbortOnMeta | KShell::TildeExpand, &err);
        if (err == KShell::FoundMeta) {
          shellQuote( exec );
          exec.prepend( "/bin/sh -c " );
        } else if (err != KShell::NoError)
          goto synerr;
      }
      result << exec;
    } else {
      result += KShell::splitArgs(exec, KShell::AbortOnMeta | KShell::TildeExpand, &err);
      if (err == KShell::FoundMeta)
        result << "/bin/sh" << "-c" << exec;
      else if (err != KShell::NoError)
        goto synerr;
    }
  }

  return result;

 synerr:
  kdWarning() << "KRun: syntax error in command `" << _service.exec() << "', service `" << _service.name() << "'" << endl;
  return QStringList();
}

//static
QString KRun::binaryName( const QString & execLine, bool removePath )
{
  // Remove parameters and/or trailing spaces.
  QStringList args = KShell::splitArgs( execLine );
  for (QStringList::ConstIterator it = args.begin(); it != args.end(); ++it)
    if (!(*it).contains('='))
      // Remove path if wanted
      return removePath ? (*it).mid((*it).lastIndexOf('/') + 1) : *it;
  return QString::null;
}

static pid_t runCommandInternal( KProcess* proc, const KService* service, const QString& binName,
    const QString &execName, const QString & iconName )
{
  if (service && !service->desktopEntryPath().isEmpty()
      && !KDesktopFile::isAuthorizedDesktopFile( service->desktopEntryPath() ))
  {
     kdWarning() << "No authorization to execute " << service->desktopEntryPath() << endl;
     KMessageBox::sorry(0, i18n("You are not authorized to execute this file."));
     return 0;
  }
  QString bin = KRun::binaryName( binName, true );
#ifdef Q_WS_X11 // Startup notification doesn't work with QT/E, service isn't needed without Startup notification
  bool silent;
  QByteArray wmclass;
  KStartupInfoId id;
  bool startup_notify = KRun::checkStartupNotify( binName, service, &silent, &wmclass );
  if( startup_notify )
  {
      id.initId();
      id.setupStartupEnv();
      KStartupInfoData data;
      data.setHostname();
      data.setBin( bin );
      if( !execName.isEmpty())
          data.setName( execName );
      else if( service && !service->name().isEmpty())
          data.setName( service->name());
      data.setDescription( i18n( "Launching %1" ).arg( data.name()));
      if( !iconName.isEmpty())
          data.setIcon( iconName );
      else if( service && !service->icon().isEmpty())
          data.setIcon( service->icon());
      if( !wmclass.isEmpty())
          data.setWMClass( wmclass );
      if( silent )
          data.setSilent( KStartupInfoData::Yes );
      data.setDesktop( KWin::currentDesktop());
      KStartupInfo::sendStartup( id, data );
  }
  pid_t pid = KProcessRunner::run( proc, binName, id );
  if( startup_notify && pid )
  {
      KStartupInfoData data;
      data.addPid( pid );
      KStartupInfo::sendChange( id, data );
      KStartupInfo::resetStartupEnv();
  }
  return pid;
#else
  Q_UNUSED( execName );
  Q_UNUSED( iconName );
  return KProcessRunner::run( proc, bin );
#endif
}

// This code is also used in klauncher.
bool KRun::checkStartupNotify( const QString& /*binName*/, const KService* service, bool* silent_arg, QByteArray* wmclass_arg )
{
  bool silent = false;
  QByteArray wmclass;
  if( service && service->property( "StartupNotify" ).isValid())
  {
      silent = !service->property( "StartupNotify" ).toBool();
      wmclass = service->property( "StartupWMClass" ).toString().latin1();
  }
  else if( service && service->property( "X-KDE-StartupNotify" ).isValid())
  {
      silent = !service->property( "X-KDE-StartupNotify" ).toBool();
      wmclass = service->property( "X-KDE-WMClass" ).toString().latin1();
  }
  else // non-compliant app
  {
      if( service )
      {
          if( service->type() == "Application" )
              wmclass = "0"; // doesn't have .desktop entries needed, start as non-compliant
          else
              return false; // no startup notification at all
      }
      else
      { // Create startup notification even for apps for which there shouldn't be any,
        // just without any visual feedback. This will ensure they'll be positioned on the proper
        // virtual desktop, and will get user timestamp from the ASN ID.
          wmclass = "0";
          silent = true;
      }
  }
  if( silent_arg != NULL )
      *silent_arg = silent;
  if( wmclass_arg != NULL )
      *wmclass_arg = wmclass;
  return true;
}

static pid_t runTempService( const KService& _service, const KURL::List& _urls, bool tempFiles )
{
  if (!_urls.isEmpty()) {
    kdDebug(7010) << "runTempService: first url " << _urls.first().url() << endl;
  }

  QStringList args;
  if ((_urls.count() > 1) && !_service.allowMultipleFiles())
  {
      // We need to launch the application N times. That sucks.
      // We ignore the result for application 2 to N.
      // For the first file we launch the application in the
      // usual way. The reported result is based on this
      // application.
      KURL::List::ConstIterator it = _urls.begin();
      while(++it != _urls.end())
      {
         KURL::List singleUrl;
         singleUrl.append(*it);
         runTempService( _service, singleUrl, tempFiles );
      }
      KURL::List singleUrl;
      singleUrl.append(_urls.first());
      args = KRun::processDesktopExec(_service, singleUrl, false, tempFiles);
  }
  else
  {
      args = KRun::processDesktopExec(_service, _urls, false, tempFiles);
  }
  kdDebug(7010) << "runTempService: KProcess args=" << args << endl;

  KProcess * proc = new KProcess;
  *proc << args;

  if (!_service.path().isEmpty())
     proc->setWorkingDirectory(_service.path());

  return runCommandInternal( proc, &_service, KRun::binaryName( _service.exec(), false ),
                             _service.name(), _service.icon() );
}

// WARNING: don't call this from processDesktopExec, since klauncher uses that too...
static KURL::List resolveURLs( const KURL::List& _urls, const KService& _service )
{
  // Check which protocols the application supports.
  // This can be a list of actual protocol names, or just KIO for KDE apps.
  QStringList supportedProtocols = _service.property("X-KDE-Protocols").toStringList();
  KRunMX1 mx1( _service );
  QString exec = _service.exec();
  if ( mx1.expandMacrosShellQuote( exec ) && !mx1.hasUrls ) {
    Q_ASSERT( supportedProtocols.isEmpty() ); // huh? If you support protocols you need %u or %U...
  } else {
    if ( supportedProtocols.isEmpty() ) // compat: assume KIO if not set
      supportedProtocols.append( "KIO" );
  }
  kdDebug(7010) << "supportedProtocols:" << supportedProtocols << endl;

  KURL::List urls( _urls );
  if ( supportedProtocols.find( "KIO" ) == supportedProtocols.end() ) {
    for( KURL::List::Iterator it = urls.begin(); it != urls.end(); ++it ) {
      const KURL url = *it;
      bool supported = url.isLocalFile() || supportedProtocols.find( url.protocol().toLower() ) != supportedProtocols.end();
      kdDebug(7010) << "Looking at url=" << url << " supported=" << supported << endl;
      if ( !supported && KProtocolInfo::protocolClass(url.protocol()) == ":local" )
      {
        // Maybe we can resolve to a local URL?
        KURL localURL = KIO::NetAccess::mostLocalURL( url, 0 );
        if ( localURL != url ) {
          *it = localURL;
          kdDebug(7010) << "Changed to " << localURL << endl;
        }
      }
    }
  }
  return urls;
}

// BIC merge with method below
pid_t KRun::run( const KService& _service, const KURL::List& _urls )
{
    return run( _service, _urls, false );
}

pid_t KRun::run( const KService& _service, const KURL::List& _urls, bool tempFiles )
{
  if (!_service.desktopEntryPath().isEmpty() &&
      !KDesktopFile::isAuthorizedDesktopFile( _service.desktopEntryPath()))
  {
     kdWarning() << "No authorization to execute " << _service.desktopEntryPath() << endl;
     KMessageBox::sorry(0, i18n("You are not authorized to execute this service."));
     return 0;
  }

  if ( !tempFiles )
  {
      // Remember we opened those urls, for the "recent documents" menu in kicker
      KURL::List::ConstIterator it = _urls.begin();
      for(; it != _urls.end(); ++it) {
          //kdDebug(7010) << "KRecentDocument::adding " << (*it).url() << endl;
          KRecentDocument::add( *it, _service.desktopEntryName() );
      }
  }

  if ( tempFiles || _service.desktopEntryPath().isEmpty())
  {
     return runTempService(_service, _urls, tempFiles);
  }

  kdDebug(7010) << "KRun::run " << _service.desktopEntryPath() << endl;

  if (!_urls.isEmpty()) {
    kdDebug(7010) << "First url " << _urls.first().url() << endl;
  }

  // Resolve urls if needed, depending on what the app supports
  const KURL::List urls = resolveURLs( _urls, _service );

  QString error;
  int pid = 0;

  int i = KToolInvocation::startServiceByDesktopPath(
        _service.desktopEntryPath(), urls.toStringList(), &error, 0L, &pid
        );

  if (i != 0)
  {
     kdDebug(7010) << error << endl;
     KMessageBox::sorry( 0L, error );
     return 0;
  }

  kdDebug(7010) << "startServiceByDesktopPath worked fine" << endl;
  return (pid_t) pid;
}


pid_t KRun::run( const QString& _exec, const KURL::List& _urls, const QString& _name,
                const QString& _icon, const QString&, const QString&)
{
  KService::Ptr service = new KService(_name, _exec, _icon);

  return run(*service, _urls);
}

pid_t KRun::runCommand( QString cmd )
{
  return KRun::runCommand( cmd, QString::null, QString::null );
}

pid_t KRun::runCommand( const QString& cmd, const QString &execName, const QString & iconName )
{
  kdDebug(7010) << "runCommand " << cmd << "," << execName << endl;
  KProcess * proc = new KProcess;
  proc->setUseShell(true);
  *proc << cmd;
  KService::Ptr service = KService::serviceByDesktopName( binaryName( execName, true ) );
  return runCommandInternal( proc, service.data(), binaryName( execName, false ), execName, iconName );
}

KRun::KRun( const KURL& url, QWidget* window, mode_t mode, bool isLocalFile,
            bool showProgressInfo )
     :m_timer(0,"KRun::timer")
{
  init (url, window, mode, isLocalFile, showProgressInfo);
}

void KRun::init ( const KURL& url, QWidget* window, mode_t mode, bool isLocalFile,
                  bool showProgressInfo )
{
  m_bFault = false;
  m_bAutoDelete = true;
  m_bProgressInfo = showProgressInfo;
  m_bFinished = false;
  m_job = 0L;
  m_strURL = url;
  m_bScanFile = false;
  m_bIsDirectory = false;
  m_bIsLocalFile = isLocalFile;
  m_mode = mode;
  d = new KRunPrivate;
  d->m_runExecutables = true;
  d->m_window = window;
  setEnableExternalBrowser(true);

  // Start the timer. This means we will return to the event
  // loop and do initialization afterwards.
  // Reason: We must complete the constructor before we do anything else.
  m_bInit = true;
  connect( &m_timer, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
  m_timer.start( 0, true );
  kdDebug(7010) << " new KRun " << this << " " << url.prettyURL() << " timer=" << &m_timer << endl;

  kapp->ref();
}

void KRun::init()
{
  kdDebug(7010) << "INIT called" << endl;
  if ( !m_strURL.isValid() )
  {
    d->m_showingError = true;
    KMessageBoxWrapper::error( d->m_window, i18n( "Malformed URL\n%1" ).arg( m_strURL.url() ) );
    d->m_showingError = false;
    m_bFault = true;
    m_bFinished = true;
    m_timer.start( 0, true );
    return;
  }
  if ( !KAuthorized::authorizeURLAction( "open", KURL(), m_strURL))
  {
    QString msg = KIO::buildErrorString(KIO::ERR_ACCESS_DENIED, m_strURL.prettyURL());
    d->m_showingError = true;
    KMessageBoxWrapper::error( d->m_window, msg );
    d->m_showingError = false;
    m_bFault = true;
    m_bFinished = true;
    m_timer.start( 0, true );
    return;
  }

  if ( !m_bIsLocalFile && m_strURL.isLocalFile() )
    m_bIsLocalFile = true;

  QString exec;
  if (m_strURL.protocol().startsWith("http"))
  {
    exec = d->m_externalBrowser;
  }

  if ( m_bIsLocalFile )
  {
    if ( m_mode == 0 )
    {
      KDE_struct_stat buff;
      if ( KDE_stat( QFile::encodeName(m_strURL.path()), &buff ) == -1 )
      {
        d->m_showingError = true;
        KMessageBoxWrapper::error( d->m_window, i18n( "<qt>Unable to run the command specified. The file or folder <b>%1</b> does not exist.</qt>" ).arg( m_strURL.htmlURL() ) );
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
  else if ( !exec.isEmpty() || KProtocolInfo::isHelperProtocol( m_strURL ) ) {
    kdDebug(7010) << "Helper protocol" << endl;

    bool ok = false;
    KURL::List urls;
    urls.append( m_strURL );
    if (exec.isEmpty())
    {
       exec = KProtocolInfo::exec( m_strURL.protocol() );
       if (exec.isEmpty())
       {
          foundMimeType(KProtocolInfo::defaultMimetype(m_strURL));
          return;
       }
       run( exec, urls );
       ok = true;
    }
    else if (exec.startsWith("!"))
    {
       exec = exec.mid(1); // Literal command
       exec += " %u";
       run( exec, urls );
       ok = true;
    }
    else
    {
       KService::Ptr service = KService::serviceByStorageId( exec );
       if (service)
       {
          run( *service, urls );
          ok = true;
       }
    }

    if (ok)
    {
       m_bFinished = true;
       // will emit the error and autodelete this
       m_timer.start( 0, true );
       return;
    }
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
  job->setWindow (d->m_window);
  connect( job, SIGNAL( result( KIO::Job * ) ),
           this, SLOT( slotStatResult( KIO::Job * ) ) );
  m_job = job;
  kdDebug(7010) << " Job " << job << " is about stating " << m_strURL.url() << endl;
}

KRun::~KRun()
{
  kdDebug(7010) << "KRun::~KRun() " << this << endl;
  m_timer.stop();
  killJob();
  kapp->deref();
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
  // We need to apply the 'KIO' method, i.e. either asking the server or
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

  KIO::TransferJob *job = KIO::get( m_strURL, false /*reload*/, m_bProgressInfo );
  job->setWindow (d->m_window);
  connect(job, SIGNAL( result(KIO::Job *)),
          this, SLOT( slotScanFinished(KIO::Job *)));
  connect(job, SIGNAL( mimetype(KIO::Job *, const QString &)),
          this, SLOT( slotScanMimeType(KIO::Job *, const QString &)));
  m_job = job;
  kdDebug(7010) << " Job " << job << " is about getting from " << m_strURL.url() << endl;
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

  if ( m_bFault ) {
      emit error();
  }
  if ( m_bFinished ) {
      emit finished();
  }
  else
  {
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
    //kdDebug(7010) << this << " KRun returning from showErrorDialog, starting timer to delete us" << endl;
    d->m_showingError = false;

    m_bFault = true;
    m_bFinished = true;

    // will emit the error and autodelete this
    m_timer.start( 0, true );

  } else {

    kdDebug(7010) << "Finished" << endl;
    if(!dynamic_cast<KIO::StatJob*>(job))
        kdFatal() << "job is a " << typeid(*job).name() << " should be a StatJob" << endl;

    QString knownMimeType;
    KIO::UDSEntry entry = ((KIO::StatJob*)job)->statResult();
    KIO::UDSEntry::ConstIterator it = entry.begin();
    for( ; it != entry.end(); it++ ) {
        switch( (*it).m_uds ) {
        case KIO::UDS_FILE_TYPE:
            if ( S_ISDIR( (mode_t)((*it).m_long) ) )
                m_bIsDirectory = true; // it's a dir
            else
                m_bScanFile = true; // it's a file
            break;
        case KIO::UDS_MIME_TYPE: // mimetype already known? (e.g. print:/manager)
            knownMimeType = (*it).m_str;
            break;
        case KIO::UDS_LOCAL_PATH:
            d->m_localPath = (*it).m_str;
            break;
        default:
            break;
        }
    }
    if ( !knownMimeType.isEmpty() )
    {
        foundMimeType( knownMimeType );
        m_bFinished = true;
    }

    // We should have found something
    assert ( m_bScanFile || m_bIsDirectory );

    // Start the timer. Once we get the timer event this
    // protocol server is back in the pool and we can reuse it.
    // This gives better performance than starting a new slave
    m_timer.start( 0, true );
  }
}

void KRun::slotScanMimeType( KIO::Job *, const QString &mimetype )
{
  if ( mimetype.isEmpty() )
    kdWarning(7010) << "KRun::slotScanFinished : MimetypeJob didn't find a mimetype! Probably a kioslave bug." << endl;
  foundMimeType( mimetype );
  m_job = 0;
}

void KRun::slotScanFinished( KIO::Job *job )
{
  m_job = 0;
  if (job->error())
  {
    d->m_showingError = true;
    kdError(7010) << this << " ERROR (stat) : " << job->error() << " " << job->errorString() << endl;
    job->showErrorDialog();
    //kdDebug(7010) << this << " KRun returning from showErrorDialog, starting timer to delete us" << endl;
    d->m_showingError = false;

    m_bFault = true;
    m_bFinished = true;

    // will emit the error and autodelete this
    m_timer.start( 0, true );
  }
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
  KIO::TransferJob *job = qobject_cast<KIO::TransferJob *>( m_job );
  if ( job )
  {
     job->putOnHold();
     KIO::Scheduler::publishSlaveOnHold();
     m_job = 0;
  }

  Q_ASSERT( !m_bFinished );

  // Suport for preferred service setting, see setPreferredService
  if ( !d->m_preferredService.isEmpty() ) {
      kdDebug(7010) << "Attempting to open with preferred service: " << d->m_preferredService << endl;
      KService::Ptr serv = KService::serviceByDesktopName( d->m_preferredService );
      if ( serv && serv->hasServiceType( type ) )
      {
          KURL::List lst;
          lst.append( m_strURL );
          m_bFinished = KRun::run( *serv, lst );
          /// Note: the line above means that if that service failed, we'll
          /// go to runURL to maybe find another service, even though a dialog
          /// box was displayed. That's good if runURL tries another service,
          /// but it's not good if it tries the same one :}
      }
  }

  // Resolve .desktop files from media:/, remote:/, applications:/ etc.
  if ( type == "application/x-desktop" /* or inheriting? */ && !d->m_localPath.isEmpty() )
  {
    m_strURL = KURL();
    m_strURL.setPath( d->m_localPath );
  }

  if (!m_bFinished && KRun::runURL( m_strURL, type, false, d->m_runExecutables )){
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
  kdDebug(7010) << "KRun::abort " << this << " m_showingError=" << d->m_showingError << endl;
  killJob();
  // If we're showing an error message box, the rest will be done
  // after closing the msgbox -> don't autodelete nor emit signals now.
  if ( d->m_showingError )
    return;
  m_bFault = true;
  m_bFinished = true;
  m_bInit = false;
  m_bScanFile = false;

  // will emit the error and autodelete this
  m_timer.start( 0, true );
}

void KRun::setEnableExternalBrowser(bool b)
{
   if (b)
      d->m_externalBrowser = KConfigGroup(KGlobal::config(), "General").readEntry("BrowserApplication");
   else
      d->m_externalBrowser = QString::null;
}

void KRun::setPreferredService( const QString& desktopEntryName )
{
    d->m_preferredService = desktopEntryName;
}

void KRun::setRunExecutables(bool b)
{
    d->m_runExecutables = b;
}

bool KRun::isExecutable( const QString& serviceType )
{
    return ( serviceType == "application/x-desktop" ||
             serviceType == "application/x-executable" ||
             serviceType == "application/x-msdos-program" ||
             serviceType == "application/x-shellscript" );
}

/****************/

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
  if ( !process_->pid() )
      slotProcessExited( process_ );
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
  if ( !process_->pid() )
      slotProcessExited( process_ );
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
  bool showErr = process_->normalExit()
                 && ( process_->exitStatus() == 127 || process_->exitStatus() == 1 );
  if ( !binName.isEmpty() && ( showErr || process_->pid() == 0 ) )
  {
    // Often we get 1 (zsh, csh) or 127 (ksh, bash) because the binary doesn't exist.
    // We can't just rely on that, but it's a good hint.
    // Before assuming its really so, we'll try to find the binName
    // relatively to current directory,  and then in the PATH.
    if ( !QFile( binName ).exists() && KStandardDirs::findExe( binName ).isEmpty() )
    {
      kapp->ref();
      KMessageBox::sorry( 0L, i18n("Could not find the program '%1'").arg( binName ) );
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
  deleteLater();
}

void KRun::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "krun.moc"
