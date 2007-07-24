/* This file is part of the KDE libraries
    Copyright (C) 2000 Torben Weis <weis@kde.org>
    Copyright (C) 2006 David Faure <faure@kde.org>

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

#include "krun.h"
#include "krun_p.h"

#include <config.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <typeinfo>
#include <sys/stat.h>

#include <QtGui/QWidget>
#include <QtCore/QPointer>

#include "kmimetypetrader.h"
#include "kmimetype.h"
#include "kio/job.h"
#include "kio/jobuidelegate.h"
#include "kio/global.h"
#include "kio/scheduler.h"
#include "kio/netaccess.h"
#include "kfile/kopenwithdialog.h"
#include "kfile/krecentdocument.h"
#include "kdesktopfileactions.h"

#include <kmessageboxwrapper.h>
#include <kurl.h>
#include <kglobal.h>
#include <ktoolinvocation.h>
#include <kauthorized.h>
#include <kdebug.h>
#include <klocale.h>
#include <kprotocolmanager.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTextIStream>
#include <QtCore/QDate>
#include <QtCore/QRegExp>
#include <kdesktopfile.h>
#include <kmacroexpander.h>
#include <kshell.h>
#include <QTextDocument>
#include <kde_file.h>
#include <kconfiggroup.h>

#ifdef Q_WS_X11
#include <kwindowsystem.h>
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
    QString m_suggestedFileName;
    QPointer <QWidget> m_window;
    QByteArray m_asn;
};

bool KRun::isExecutableFile( const KUrl& url, const QString &mimetype )
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
bool KRun::runUrl( const KUrl& u, const QString& _mimetype, QWidget* window, bool tempFile, bool runExecutables, const QString& suggestedFileName, const QByteArray& asn )
{
  bool noRun = false;
  bool noAuth = false;
  if ( _mimetype == "inode/directory-locked" )
  {
    KMessageBoxWrapper::error( window,
            i18n("<qt>Unable to enter <b>%1</b>.\nYou do not have access rights to this location.</qt>", Qt::escape(u.prettyUrl())) );
    return false;
  }
  else if ( _mimetype == "application/x-desktop" )
  {
    if ( u.isLocalFile() && runExecutables )
      return KDesktopFileActions::run( u, true );
  }
  else if ( isExecutableFile(u, _mimetype) )
  {
    if ( u.isLocalFile() && runExecutables)
    {
      if (KAuthorized::authorize("shell_access"))
      {
        return (KRun::runCommand(KShell::quoteArg(u.path()), QString(), QString(), window, asn)); // just execute the url as a command
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
    KMessageBox::sorry( window,
        i18n("<qt>The file <b>%1</b> is an executable program. "
             "For safety it will not be started.</qt>", Qt::escape(u.prettyUrl())));
    return false;
  }
  if ( noAuth )
  {
    KMessageBoxWrapper::error( window,
        i18n("<qt>You do not have permission to run <b>%1</b>.</qt>", Qt::escape(u.prettyUrl())) );
    return false;
  }

  KUrl::List lst;
  lst.append( u );

  KService::Ptr offer = KMimeTypeTrader::self()->preferredService( _mimetype );

  if ( !offer )
  {
    // Open-with dialog
    // TODO : pass the mimetype as a parameter, to show it (comment field) in the dialog !
    // Hmm, in fact KOpenWithDialog::setServiceType already guesses the mimetype from the first URL of the list...
    return displayOpenWithDialog( lst, window, tempFile, suggestedFileName, asn );
  }

  return KRun::run( *offer, lst, window, tempFile, suggestedFileName, asn );
}

bool KRun::displayOpenWithDialog( const KUrl::List& lst, QWidget* window, bool tempFiles, const QString& suggestedFileName,
    const QByteArray& asn )
{
    if (!KAuthorized::authorizeKAction("openwith"))
    {
       KMessageBox::sorry(window, i18n("You are not authorized to select an application to open this file."));
       return false;
    }

    KOpenWithDialog l( lst, i18n("Open with:"), QString(), window );
    if ( l.exec() )
    {
      KService::Ptr service = l.service();
      if ( !!service )
        return KRun::run( *service, lst, window, tempFiles, suggestedFileName, asn );

      kDebug(7010) << "No service set, running " << l.text() << endl;
      return KRun::run( l.text(), lst, window, false, suggestedFileName, asn ); // TODO handle tempFiles
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
//       ret << "-miniicon" << service.icon().replace( '%', "%%" );
      kWarning() << k_funcinfo << "-miniicon isn't supported anymore (service "
                  << service.name() << ')' << endl;
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
    KRunMX2( const KUrl::List &_urls ) :
        KMacroExpanderBase( '%' ), ignFile( false ), urls( _urls ) {}
    bool ignFile:1;

protected:
    virtual int expandEscapedMacro( const QString &str, int pos, QStringList &ret );

private:
    void subst( int option, const KUrl &url, QStringList &ret );

    const KUrl::List &urls;
};

void
KRunMX2::subst( int option, const KUrl &url, QStringList &ret )
{
   switch( option ) {
   case 'u':
      ret << url.pathOrUrl();
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
          ret << KDesktopFile( url.path() ).desktopGroup().readEntry( "Dev" );
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
            kDebug() << "KRun::processDesktopExec: No URLs supplied to single-URL service " << str << endl;
      } else if( urls.count() > 1 )
          kWarning() << "KRun::processDesktopExec: " << urls.count() << " URLs supplied to single-URL service " << str << endl;
      else
         subst( option, urls.first(), ret );
      break;
   case 'F':
   case 'U':
   case 'N':
   case 'D':
      option += 'a' - 'A';
      for( KUrl::List::ConstIterator it = urls.begin(); it != urls.end(); ++it )
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

QStringList KRun::processDesktopExec(const KService &_service, const KUrl::List& _urls, bool tempFiles, const QString& suggestedFileName)
{
  QString exec = _service.exec();
  if ( exec.isEmpty() ) {
      kWarning() << "KRun: no Exec field in `" << _service.desktopEntryPath() << "' !" << endl;
      return QStringList();
  }

  QStringList result;
  bool appHasTempFileOption;

  KRunMX1 mx1( _service );
  KRunMX2 mx2( _urls );

  if( !mx1.expandMacrosShellQuote( exec ) )
    goto synerr; // error in shell syntax

  // FIXME: the current way of invoking kioexec disables term and su use

  // Check if we need "tempexec" (kioexec in fact)
  appHasTempFileOption = tempFiles && _service.property("X-KDE-HasTempFileOption").toBool();
  if( tempFiles && !appHasTempFileOption && _urls.size() ) {
    result << "kioexec" << "--tempfiles" << exec;
    if ( !suggestedFileName.isEmpty() ) {
        result << "--suggestedfilename";
        result << suggestedFileName;
    }
    result += _urls.toStringList();
    return result;
  }

  // Check if we need kioexec
  if( !mx1.hasUrls ) {
    for( KUrl::List::ConstIterator it = _urls.begin(); it != _urls.end(); ++it )
      if ( !(*it).isLocalFile() && !KProtocolInfo::isHelperProtocol(*it) ) {
        // We need to run the app through kioexec
        result << "kioexec";
        if ( tempFiles )
            result << "--tempfiles";
        if ( !suggestedFileName.isEmpty() ) {
            result << "--suggestedfilename";
            result << suggestedFileName;
        }
        result << exec;
        result += _urls.toStringList();
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
 1 = need_shell, 2 = terminal, 4 = su

 0                                                           << split(cmd)
 1                                                           << "sh" << "-c" << cmd
 2 << split(term) << "-e"                                    << split(cmd)
 3 << split(term) << "-e"                                    << "sh" << "-c" << cmd

 4                        << "kdesu" << "-u" << user << "-c" << cmd
 5                        << "kdesu" << "-u" << user << "-c" << ("sh -c " + quote(cmd))
 6 << split(term) << "-e" << "su"            << user << "-c" << cmd
 7 << split(term) << "-e" << "su"            << user << "-c" << ("sh -c " + quote(cmd))

 "sh -c" is needed in the "su" case, too, as su uses the user's login shell, not sh.
 this could be optimized with the -s switch of some su versions (e.g., debian linux).
*/

  if (_service.terminal()) {
    KConfigGroup cg(KGlobal::config(), "General");
    QString terminal = cg.readPathEntry("TerminalApplication", "konsole");
    if (terminal == "konsole")
      terminal += " -caption=%c %i %m";
    terminal += ' ';
    terminal += _service.terminalOptions();
    if( !mx1.expandMacrosShellQuote( terminal ) ) {
      kWarning() << "KRun: syntax error in command `" << terminal << "', service `" << _service.name() << "'" << endl;
      return QStringList();
    }
    mx2.expandMacrosShellQuote( terminal );
    result = KShell::splitArgs( terminal ); // assuming that the term spec never needs a shell!
    result << "-e";
  }

  KShell::Errors err;
  if (_service.substituteUid()) {
    if (_service.terminal())
      result << "su";
    else
      result << "kdesu" << "-u";
    result << _service.username() << "-c";
    KShell::splitArgs(exec, KShell::AbortOnMeta | KShell::TildeExpand, &err);
    if (err == KShell::FoundMeta) {
      exec = "/bin/sh -c " + KShell::quoteArg(exec);
    } else if (err != KShell::NoError)
      goto synerr;
    result << exec;
  } else {
    result += KShell::splitArgs(exec, KShell::AbortOnMeta | KShell::TildeExpand, &err);
    if (err == KShell::FoundMeta)
      result << "/bin/sh" << "-c" << exec;
    else if (err != KShell::NoError)
      goto synerr;
    else {
      if (!result.isEmpty()) {
        // resolve the executable according to the path, which also makes it possible to find kde4/libexec helpers.
        const QString exePath = KStandardDirs::findExe(result[0]);
        if (!exePath.isEmpty())
          result[0] = exePath;
      }
    }
  }

  return result;

 synerr:
  kWarning() << "KRun: syntax error in command `" << _service.exec() << "', service `" << _service.name() << "'" << endl;
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
  return QString();
}

static bool runCommandInternal( KProcess* proc, const KService* service, const QString& binName,
    const QString &execName, const QString & iconName, QWidget* window, const QByteArray& asn )
{
  if( window != NULL )
      window = window->topLevelWidget();
  if (service && !service->desktopEntryPath().isEmpty()
      && !KDesktopFile::isAuthorizedDesktopFile( service->desktopEntryPath() ))
  {
     kWarning() << "No authorization to execute " << service->desktopEntryPath() << endl;
     KMessageBox::sorry( window, i18n("You are not authorized to execute this file."));
     return false;
  }
  QString bin = KRun::binaryName( binName, true );
#ifdef Q_WS_X11 // Startup notification doesn't work with QT/E, service isn't needed without Startup notification
  bool silent;
  QByteArray wmclass;
  KStartupInfoId id;
  bool startup_notify = ( asn != "0" && KRun::checkStartupNotify( binName, service, &silent, &wmclass ));
  if( startup_notify )
  {
      id.initId( asn );
      id.setupStartupEnv();
      KStartupInfoData data;
      data.setHostname();
      data.setBin( bin );
      if( !execName.isEmpty())
          data.setName( execName );
      else if( service && !service->name().isEmpty())
          data.setName( service->name());
      data.setDescription( i18n( "Launching %1" ,  data.name()));
      if( !iconName.isEmpty())
          data.setIcon( iconName );
      else if( service && !service->icon().isEmpty())
          data.setIcon( service->icon());
      if( !wmclass.isEmpty())
          data.setWMClass( wmclass );
      if( silent )
          data.setSilent( KStartupInfoData::Yes );
      data.setDesktop( KWindowSystem::currentDesktop());
      if( window )
          data.setLaunchedBy( window->winId());
      KStartupInfo::sendStartup( id, data );
  }
  int pid = KProcessRunner::run( proc, binName, id );
  if( startup_notify && pid )
  {
      KStartupInfoData data;
      data.addPid( pid );
      KStartupInfo::sendChange( id, data );
      KStartupInfo::resetStartupEnv();
  }
  return pid != 0;
#else
  Q_UNUSED( execName );
  Q_UNUSED( iconName );
  return KProcessRunner::run( proc, bin ) != 0;
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
      wmclass = service->property( "StartupWMClass" ).toString().toLatin1();
  }
  else if( service && service->property( "X-KDE-StartupNotify" ).isValid())
  {
      silent = !service->property( "X-KDE-StartupNotify" ).toBool();
      wmclass = service->property( "X-KDE-WMClass" ).toString().toLatin1();
  }
  else // non-compliant app
  {
      if( service )
      {
          if( service->isApplication() )
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

static bool runTempService( const KService& _service, const KUrl::List& _urls, QWidget* window,
    bool tempFiles, const QString& suggestedFileName, const QByteArray& asn )
{
  if (!_urls.isEmpty()) {
    kDebug(7010) << "runTempService: first url " << _urls.first().url() << endl;
  }

  QStringList args;
  if ((_urls.count() > 1) && !_service.allowMultipleFiles())
  {
      // We need to launch the application N times. That sucks.
      // We ignore the result for application 2 to N.
      // For the first file we launch the application in the
      // usual way. The reported result is based on this
      // application.
      KUrl::List::ConstIterator it = _urls.begin();
      while(++it != _urls.end())
      {
         KUrl::List singleUrl;
         singleUrl.append(*it);
         runTempService( _service, singleUrl, window, tempFiles, suggestedFileName, "" );
      }
      KUrl::List singleUrl;
      singleUrl.append(_urls.first());
      args = KRun::processDesktopExec( _service, singleUrl, tempFiles, suggestedFileName );
  }
  else
  {
      args = KRun::processDesktopExec(_service, _urls, tempFiles, suggestedFileName );
  }
  kDebug(7010) << "runTempService: KProcess args=" << args << endl;

  KProcess * proc = new KProcess;
  *proc << args;

  if (!_service.path().isEmpty())
     proc->setWorkingDirectory(_service.path());

  return runCommandInternal( proc, &_service, KRun::binaryName( _service.exec(), false ),
                             _service.name(), _service.icon(), window, asn );
}

// WARNING: don't call this from processDesktopExec, since klauncher uses that too...
static KUrl::List resolveURLs( const KUrl::List& _urls, const KService& _service )
{
  // Check which protocols the application supports.
  // This can be a list of actual protocol names, or just KIO for KDE apps.
  QStringList supportedProtocols = _service.property("X-KDE-Protocols").toStringList();
  KRunMX1 mx1( _service );
  QString exec = _service.exec();
  if ( mx1.expandMacrosShellQuote( exec ) && !mx1.hasUrls ) {
    Q_ASSERT( supportedProtocols.isEmpty() ); // huh? If you support protocols you need %u or %U...
  } else {
    if ( supportedProtocols.isEmpty() )
    {
      // compat mode: assume KIO if not set and it's a KDE app
      QStringList categories = _service.property("Categories").toStringList();
      if ( categories.contains("KDE") )
         supportedProtocols.append( "KIO" );
      else { // if no KDE app, be a bit over-generic
         supportedProtocols.append( "http");
         supportedProtocols.append( "ftp");
      }
    }
  }
  kDebug(7010) << "supportedProtocols:" << supportedProtocols << endl;

  KUrl::List urls( _urls );
  if ( !supportedProtocols.contains( "KIO" ) ) {
    for( KUrl::List::Iterator it = urls.begin(); it != urls.end(); ++it ) {
      const KUrl url = *it;
      bool supported = url.isLocalFile() || supportedProtocols.contains( url.protocol().toLower() );
      kDebug(7010) << "Looking at url=" << url << " supported=" << supported << endl;
      if ( !supported && KProtocolInfo::protocolClass(url.protocol()) == ":local" )
      {
        // Maybe we can resolve to a local URL?
        KUrl localURL = KIO::NetAccess::mostLocalUrl( url, 0 );
        if ( localURL != url ) {
          *it = localURL;
          kDebug(7010) << "Changed to " << localURL << endl;
        }
      }
    }
  }
  return urls;
}

bool KRun::run( const KService& _service, const KUrl::List& _urls, QWidget* window,
    bool tempFiles, const QString& suggestedFileName, const QByteArray& asn )
{
  if (!_service.desktopEntryPath().isEmpty() &&
      !KDesktopFile::isAuthorizedDesktopFile( _service.desktopEntryPath()))
  {
     kWarning() << "No authorization to execute " << _service.desktopEntryPath() << endl;
     KMessageBox::sorry( window, i18n("You are not authorized to execute this service.") );
     return false;
  }

  if ( !tempFiles )
  {
      // Remember we opened those urls, for the "recent documents" menu in kicker
      KUrl::List::ConstIterator it = _urls.begin();
      for(; it != _urls.end(); ++it) {
          //kDebug(7010) << "KRecentDocument::adding " << (*it).url() << endl;
          KRecentDocument::add( *it, _service.desktopEntryName() );
      }
  }

  if ( tempFiles || _service.desktopEntryPath().isEmpty() || !suggestedFileName.isEmpty() )
  {
     return runTempService( _service, _urls, window, tempFiles, suggestedFileName, asn );
  }

  kDebug(7010) << "KRun::run " << _service.desktopEntryPath() << endl;

  if (!_urls.isEmpty()) {
    kDebug(7010) << "First url " << _urls.first().url() << endl;
  }

  // Resolve urls if needed, depending on what the app supports
  const KUrl::List urls = resolveURLs( _urls, _service );

  QString error;
  int pid = 0;

  QByteArray myasn = asn;
  // startServiceByDesktopPath() doesn't take QWidget*, add it to the startup info now
  if( window != NULL )
  {
    if( myasn.isEmpty())
        myasn = KStartupInfo::createNewStartupId();
    if( myasn != "0" )
    {
        KStartupInfoId id;
        id.initId( myasn );
        KStartupInfoData data;
        data.setLaunchedBy( window->winId());
        KStartupInfo::sendChange( id, data );
    }
  }

  int i = KToolInvocation::startServiceByDesktopPath(
        _service.desktopEntryPath(), urls.toStringList(), &error, 0L, &pid, myasn
        );

  if (i != 0)
  {
     kDebug(7010) << error << endl;
     KMessageBox::sorry( window, error );
     return false;
  }

  kDebug(7010) << "startServiceByDesktopPath worked fine" << endl;
  return true;
}


bool KRun::run( const QString& _exec, const KUrl::List& _urls, QWidget* window, const QString& _name,
                 const QString& _icon, const QByteArray& asn )
{
  KService::Ptr service(new KService(_name, _exec, _icon));

  return run( *service, _urls, window, false, QString(), asn );
}

bool KRun::runCommand( const QString &cmd, QWidget* window )
{
  return KRun::runCommand( cmd, QString(), QString(), window, "" );
}

bool KRun::runCommand( const QString& cmd, const QString &execName, const QString & iconName, QWidget* window, const QByteArray& asn )
{
  kDebug(7010) << "runCommand " << cmd << "," << execName << endl;
  KProcess * proc = new KProcess;
  proc->setShellCommand( cmd );
  QString bin = binaryName( execName, true );
  KService::Ptr service = KService::serviceByDesktopName( bin );
  return runCommandInternal( proc, service.data(), bin, execName, iconName, window, asn );
}

KRun::KRun( const KUrl& url, QWidget* window, mode_t mode, bool isLocalFile,
            bool showProgressInfo, const QByteArray& asn )
     : m_timer(), d(new KRunPrivate)
{
  m_timer.setObjectName( "KRun::timer" );
  m_timer.setSingleShot( true );
  init (url, window, mode, isLocalFile, showProgressInfo, asn );
}

void KRun::init ( const KUrl& url, QWidget* window, mode_t mode, bool isLocalFile,
                  bool showProgressInfo, const QByteArray& asn )
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
  d->m_runExecutables = true;
  d->m_window = window;
  d->m_asn = asn;
  setEnableExternalBrowser(true);

  // Start the timer. This means we will return to the event
  // loop and do initialization afterwards.
  // Reason: We must complete the constructor before we do anything else.
  m_bInit = true;
  connect( &m_timer, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
  m_timer.start( 0 );
  kDebug(7010) << " new KRun " << this << " " << url.prettyUrl() << " timer=" << &m_timer << endl;

  KGlobal::ref();
}

void KRun::init()
{
  kDebug(7010) << "INIT called" << endl;
  if ( !m_strURL.isValid() )
  {
    d->m_showingError = true;
    KMessageBoxWrapper::error( d->m_window, i18n( "Malformed URL\n%1" ,  m_strURL.url() ) );
    d->m_showingError = false;
    m_bFault = true;
    m_bFinished = true;
    m_timer.start( 0 );
    return;
  }
  if ( !KAuthorized::authorizeUrlAction( "open", KUrl(), m_strURL))
  {
    QString msg = KIO::buildErrorString(KIO::ERR_ACCESS_DENIED, m_strURL.prettyUrl());
    d->m_showingError = true;
    KMessageBoxWrapper::error( d->m_window, msg );
    d->m_showingError = false;
    m_bFault = true;
    m_bFinished = true;
    m_timer.start( 0 );
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
        KMessageBoxWrapper::error( d->m_window, i18n( "<qt>Unable to run the command specified. The file or folder <b>%1</b> does not exist.</qt>" ,  Qt::escape(m_strURL.prettyUrl()) ) );
        d->m_showingError = false;
        m_bFault = true;
        m_bFinished = true;
        m_timer.start( 0 );
        return;
      }
      m_mode = buff.st_mode;
    }

    KMimeType::Ptr mime = KMimeType::findByUrl( m_strURL, m_mode, m_bIsLocalFile );
    assert( mime );
    kDebug(7010) << "MIME TYPE is " << mime->name() << endl;
    foundMimeType( mime->name() );
    return;
  }
  else if ( !exec.isEmpty() || KProtocolInfo::isHelperProtocol( m_strURL ) ) {
    kDebug(7010) << "Helper protocol" << endl;

    bool ok = false;
    KUrl::List urls;
    urls.append( m_strURL );
    if (exec.isEmpty())
    {
       exec = KProtocolInfo::exec( m_strURL.protocol() );
       if (exec.isEmpty())
       {
          foundMimeType(KProtocolManager::defaultMimetype(m_strURL));
          return;
       }
       run( exec, urls, d->m_window, false, QString(), d->m_asn );
       ok = true;
    }
    else if (exec.startsWith('!'))
    {
       exec = exec.mid(1); // Literal command
       exec += " %u";
       run( exec, urls, d->m_window, false, QString(), d->m_asn );
       ok = true;
    }
    else
    {
       KService::Ptr service = KService::serviceByStorageId( exec );
       if (service)
       {
          run( *service, urls, d->m_window, false, QString(), d->m_asn );
          ok = true;
       }
    }

    if (ok)
    {
       m_bFinished = true;
       // will emit the error and autodelete this
       m_timer.start( 0 );
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

  if ( !KProtocolManager::supportsListing( m_strURL ) )
  {
    //kDebug(7010) << "Protocol has no support for listing" << endl;
    // No support for listing => it can't be a directory (example: http)
    scanFile();
    return;
  }

  kDebug(7010) << "Testing directory (stating)" << endl;

  // It may be a directory or a file, let's stat
  KIO::StatJob *job = KIO::stat( m_strURL, true, 0 /* no details */, m_bProgressInfo );
  job->ui()->setWindow (d->m_window);
  connect( job, SIGNAL( result( KJob * ) ),
           this, SLOT( slotStatResult( KJob * ) ) );
  m_job = job;
  kDebug(7010) << " Job " << job << " is about stating " << m_strURL.url() << endl;
}

KRun::~KRun()
{
  kDebug(7010) << "KRun::~KRun() " << this << endl;
  m_timer.stop();
  killJob();
  KGlobal::deref();
  kDebug(7010) << "KRun::~KRun() done " << this << endl;
  delete d;
}

void KRun::scanFile()
{
  kDebug(7010) << "###### KRun::scanFile " << m_strURL.url() << endl;
  // First, let's check for well-known extensions
  // Not when there is a query in the URL, in any case.
  if ( m_strURL.query().isEmpty() )
  {
    KMimeType::Ptr mime = KMimeType::findByUrl( m_strURL );
    assert( mime );
    if ( mime->name() != "application/octet-stream" || m_bIsLocalFile )
    {
      kDebug(7010) << "Scanfile: MIME TYPE is " << mime->name() << endl;
      foundMimeType( mime->name() );
      return;
    }
  }

  // No mimetype found, and the URL is not local  (or fast mode not allowed).
  // We need to apply the 'KIO' method, i.e. either asking the server or
  // getting some data out of the file, to know what mimetype it is.

  if ( !KProtocolManager::supportsReading( m_strURL ) )
  {
    kError(7010) << "#### NO SUPPORT FOR READING!" << endl;
    m_bFault = true;
    m_bFinished = true;
    m_timer.start( 0 );
    return;
  }
  kDebug(7010) << this << " Scanning file " << m_strURL.url() << endl;

  KIO::TransferJob *job = KIO::get( m_strURL, false /*reload*/, m_bProgressInfo );
  job->ui()->setWindow (d->m_window);
  connect(job, SIGNAL( result(KJob *)),
          this, SLOT( slotScanFinished(KJob *)));
  connect(job, SIGNAL( mimetype(KIO::Job *, const QString &)),
          this, SLOT( slotScanMimeType(KIO::Job *, const QString &)));
  m_job = job;
  kDebug(7010) << " Job " << job << " is about getting from " << m_strURL.url() << endl;
}

void KRun::slotTimeout()
{
  kDebug(7010) << this << " slotTimeout called" << endl;
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

void KRun::slotStatResult( KJob * job )
{
  m_job = 0L;
  if (job->error())
  {
    d->m_showingError = true;
    kError(7010) << this << " ERROR " << job->error() << " " << job->errorString() << endl;
    job->uiDelegate()->showErrorMessage();
    //kDebug(7010) << this << " KRun returning from showErrorDialog, starting timer to delete us" << endl;
    d->m_showingError = false;

    m_bFault = true;
    m_bFinished = true;

    // will emit the error and autodelete this
    m_timer.start( 0 );

  } else {

    kDebug(7010) << "Finished" << endl;
    if(!qobject_cast<KIO::StatJob*>(job))
        kFatal() << "job is a " << typeid(*job).name() << " should be a StatJob" << endl;

    const KIO::UDSEntry entry = ((KIO::StatJob*)job)->statResult();
    const mode_t mode = entry.numberValue( KIO::UDSEntry::UDS_FILE_TYPE );
    if ( S_ISDIR( mode ) )
        m_bIsDirectory = true; // it's a dir
    else
        m_bScanFile = true; // it's a file

    d->m_localPath = entry.stringValue( KIO::UDSEntry::UDS_LOCAL_PATH );

    // mimetype already known? (e.g. print:/manager)
    const QString knownMimeType = entry.stringValue( KIO::UDSEntry::UDS_MIME_TYPE ) ;

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
    m_timer.start( 0 );
  }
}

void KRun::slotScanMimeType( KIO::Job *, const QString &mimetype )
{
  if ( mimetype.isEmpty() )
    kWarning(7010) << "KRun::slotScanFinished : MimetypeJob didn't find a mimetype! Probably a kioslave bug." << endl;
  foundMimeType( mimetype );
  m_job = 0;
}

void KRun::slotScanFinished( KJob *job )
{
  m_job = 0;
  if (job->error())
  {
    d->m_showingError = true;
    kError(7010) << this << " ERROR (stat) : " << job->error() << " " << job->errorString() << endl;
    job->uiDelegate()->showErrorMessage();
    //kDebug(7010) << this << " KRun returning from showErrorDialog, starting timer to delete us" << endl;
    d->m_showingError = false;

    m_bFault = true;
    m_bFinished = true;

    // will emit the error and autodelete this
    m_timer.start( 0 );
  }
}

void KRun::foundMimeType( const QString& type )
{
  kDebug(7010) << "Resulting mime type is " << type << endl;

/*
  // Automatically unzip stuff

  // Disabled since the new KIO doesn't have filters yet.

  if ( type == "application/x-gzip"  ||
       type == "application/x-bzip"  ||
       type == "application/x-bzip"  )
  {
    KUrl::List lst = KUrl::split( m_strURL );
    if ( lst.isEmpty() )
    {
      QString tmp = i18n( "Malformed URL" );
      tmp += "\n";
      tmp += m_strURL.url();
      KMessageBoxWrapper::error( 0L, tmp );
      return;
    }

    if ( type == "application/x-gzip" )
      lst.prepend( KUrl( "gzip:/decompress" ) );
    else if ( type == "application/x-bzip" )
      lst.prepend( KUrl( "bzip:/decompress" ) );
    else if ( type == "application/x-bzip" )
      lst.prepend( KUrl( "bzip2:/decompress" ) );
    else if ( type == "application/x-tar" )
      lst.prepend( KUrl( "tar:/" ) );

    // Move the HTML style reference to the leftmost URL
    KUrl::List::Iterator it = lst.begin();
    ++it;
    (*lst.begin()).setRef( (*it).ref() );
    (*it).setRef( QString() );

    // Create the new URL
    m_strURL = KUrl::join( lst );

    kDebug(7010) << "Now trying with " << debugString(m_strURL.url()) << endl;

    killJob();

    // We don't know if this is a file or a directory. Let's test this first.
    // (For instance a tar.gz is a directory contained inside a file)
    // It may be a directory or a file, let's stat
    KIO::StatJob *job = KIO::stat( m_strURL, m_bProgressInfo );
    connect( job, SIGNAL( result( KJob * ) ),
             this, SLOT( slotStatResult( KJob * ) ) );
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

  KMimeType::Ptr mime = KMimeType::mimeType( type );
  if ( !mime )
      kWarning(7010) << "Unknown mimetype " << type << endl;

  // Suport for preferred service setting, see setPreferredService
  if ( !d->m_preferredService.isEmpty() ) {
      kDebug(7010) << "Attempting to open with preferred service: " << d->m_preferredService << endl;
      KService::Ptr serv = KService::serviceByDesktopName( d->m_preferredService );
      if ( serv && serv->hasMimeType( mime.data() ) )
      {
          KUrl::List lst;
          lst.append( m_strURL );
          m_bFinished = KRun::run( *serv, lst, d->m_window, false, QString(), d->m_asn );
          /// Note: the line above means that if that service failed, we'll
          /// go to runUrl to maybe find another service, even though a dialog
          /// box was displayed. That's good if runUrl tries another service,
          /// but it's not good if it tries the same one :}
      }
  }

  // Resolve .desktop files from media:/, remote:/, applications:/ etc.
  if ( mime && mime->is( "application/x-desktop" ) && !d->m_localPath.isEmpty() )
  {
    m_strURL = KUrl();
    m_strURL.setPath( d->m_localPath );
  }

  if (!m_bFinished && KRun::runUrl( m_strURL, type, d->m_window, false /*tempfile*/, d->m_runExecutables, d->m_suggestedFileName, d->m_asn )){
    m_bFinished = true;
  }
  else{
    m_bFinished = true;
     m_bFault = true;
  }

  m_timer.start( 0 );
}

void KRun::killJob()
{
  if ( m_job )
  {
    kDebug(7010) << "KRun::killJob run=" << this << " m_job=" << m_job << endl;
    m_job->kill();
    m_job = 0L;
  }
}

void KRun::abort()
{
  kDebug(7010) << "KRun::abort " << this << " m_showingError=" << d->m_showingError << endl;
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
  m_timer.start( 0 );
}

bool KRun::hasError() const
{
    return m_bFault;
}

bool KRun::hasFinished() const
{
    return m_bFinished;
}

bool KRun::autoDelete() const
{
    return m_bAutoDelete;
}

void KRun::setAutoDelete(bool b)
{
    m_bAutoDelete = b;
}

void KRun::setEnableExternalBrowser(bool b)
{
   if (b)
      d->m_externalBrowser = KConfigGroup(KGlobal::config(), "General").readEntry("BrowserApplication");
   else
      d->m_externalBrowser.clear();
}

void KRun::setPreferredService( const QString& desktopEntryName )
{
    d->m_preferredService = desktopEntryName;
}

void KRun::setRunExecutables(bool b)
{
    d->m_runExecutables = b;
}

void KRun::setSuggestedFileName( const QString& fileName )
{
    d->m_suggestedFileName = fileName;
}

QString KRun::suggestedFileName() const
{
  return d->m_suggestedFileName;
}

bool KRun::isExecutable( const QString& serviceType )
{
    return ( serviceType == "application/x-desktop" ||
             serviceType == "application/x-executable" ||
             serviceType == "application/x-ms-dos-executable" ||
             serviceType == "application/x-shellscript" );
}

/****************/

#ifndef Q_WS_X11
int KProcessRunner::run(KProcess * p, const QString & binName)
{
    return (new KProcessRunner(p, binName))->pid();
}
#else
int KProcessRunner::run(KProcess * p, const QString & binName, const KStartupInfoId& id)
{
    return (new KProcessRunner(p, binName, id))->pid();
}
#endif

#ifndef Q_WS_X11
KProcessRunner::KProcessRunner(KProcess * p, const QString & _binName)
#else
KProcessRunner::KProcessRunner(KProcess * p, const QString & _binName, const KStartupInfoId& _id) :
    id(_id)
#endif
{
    process = p;
    binName = _binName;
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotProcessExited(int, QProcess::ExitStatus)));

    process->start();
    if (!process->waitForStarted()) {
        slotProcessExited(127, QProcess::CrashExit);
    }
}

KProcessRunner::~KProcessRunner()
{
    delete process;
}

int KProcessRunner::pid() const
{
    return process ? process->pid() : 0;
}

void
KProcessRunner::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    kDebug(7010) << "slotProcessExited " << binName << endl;
    kDebug(7010) << "normalExit " << (exitStatus == QProcess::NormalExit) << endl;
    kDebug(7010) << "exitCode " << exitCode << endl;
    bool showErr = exitStatus == QProcess::NormalExit
                   && (exitCode == 127 || exitCode == 1);
    if (!binName.isEmpty() && (showErr || pid() == 0 )) {
        // Often we get 1 (zsh, csh) or 127 (ksh, bash) because the binary doesn't exist.
        // We can't just rely on that, but it's a good hint.
        // Before assuming its really so, we'll try to find the binName
        // relatively to current directory,  and then in the PATH.
        if (!QFile(binName).exists() && KStandardDirs::findExe(binName).isEmpty()) {
            KGlobal::ref();
            KMessageBox::sorry(0L, i18n("Could not find the program '%1'", binName));
            KGlobal::deref();
        }
    }
#ifdef Q_WS_X11
    if (!id.none()) {
        KStartupInfoData data;
        data.addPid(pid()); // announce this pid for the startup notification has finished
        data.setHostname();
        KStartupInfo::sendFinish(id, data);
    }
#endif
    deleteLater();
}

#include "krun.moc"
#include "krun_p.moc"
