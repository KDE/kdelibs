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
#include "kfile/kopenwith.h"
#include "kfile/krecentdocument.h"

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
#include <typeinfo>

class KRun::KRunPrivate
{
public:
    KRunPrivate() { m_showingError = false; }
    bool m_showingError;
    QString m_preferredService;
};

// This is called by foundMimeType, since it knows the mimetype of the URL
pid_t KRun::runURL( const KURL& u, const QString& _mimetype )
{

  if ( _mimetype == "inode/directory-locked" )
  {
    KMessageBoxWrapper::error( 0L,
            i18n("<qt>Unable to enter <b>%1</b>.\nYou do not have access rights to this location.</qt>").arg(u.htmlURL()) );
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
    if (!kapp->authorize("shell_access"))
    {
      KMessageBoxWrapper::error( 0L,
            i18n("<qt>You do not have permission to run <b>%1</b>.</qt>").arg(u.htmlURL()) );
      return 0;
    }
    if ( u.isLocalFile() )
    {
      QString path = u.path();
      shellQuote( path );
      return (KRun::runCommand(path)); // just execute the url as a command
    }
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
    return displayOpenWithDialog( lst );
  }

  return KRun::run( *offer, lst );
}

bool KRun::displayOpenWithDialog( const KURL::List& lst )
{
    KOpenWithDlg l( lst, i18n("Open with:"), QString::null, 0L );
    if ( l.exec() )
    {
      KService::Ptr service = l.service();
      if ( !!service )
        return KRun::run( *service, lst );

      kdDebug(250) << "No service set, running " << l.text() << endl;
      return KRun::run( l.text(), lst );
    }
    return false;
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

static QStringList breakup(const QString &exec, bool *need_shell = 0)
{
  QStringList result;
  // This small state machine is used to parse "exec" in order
  // to cut arguments at spaces, but also treat "..." and '...'
  // as a single argument even if they contain spaces. Those simple
  // and double quotes are also removed.
  enum { PARSE_ANY, PARSE_QUOTED, PARSE_DBLQUOTED } state = PARSE_ANY;
  QString arg;
  for ( uint pos = 0; pos < exec.length() ; ++pos )
  {
      QChar ch = exec[pos];
      switch (state) {
          case PARSE_ANY:
              if ( ch == '\'' && arg.isEmpty() )
                  state = PARSE_QUOTED;
              else if ( ch == '"' && arg.isEmpty() )
                  state = PARSE_DBLQUOTED;
              else if ( ch == ' ' ) /// TODO and last char wasn't a '\'
              {
                  if (!arg.isEmpty())
                      result.append(arg);
                  arg = QString::null;
                  state = PARSE_ANY;
              }
              else if (( ch == ';' ) || (ch == '|') || (ch == '<'))
              {
                  if (!arg.isEmpty())
                      result.append(arg);
                  result.append(QString(ch));
                  arg = QString::null;
                  state = PARSE_ANY;
                  if (need_shell)
                     *need_shell = true;
              }
              else
                  arg += ch;
              break;
          case PARSE_QUOTED:
              if ( ch == '\'' )
              {
                  result.append(arg);
                  arg = QString::null;
                  state = PARSE_ANY;
              }
              else
                  arg += ch;
              break;
          case PARSE_DBLQUOTED:
              if ( ch == '"' )
              {
                  result.append(arg);
                  arg = QString::null;
                  state = PARSE_ANY;
              }
              else
                  arg += ch;
              break;
      }
  }
  if (!arg.isEmpty())
          result.append(arg);
  if (need_shell && !result.isEmpty())
  {
     if (result[0].contains('='))
        *need_shell = true;
  }
  return result;
}

static QString conditionalQuote(const QString &s, bool quote)
{
   if (!quote) return s;
   QString r = s;
   KRun::shellQuote(r);
   return r;
}

static QString substitution(int option, const KURL &_url, bool quote)
{
   if (option == 'u')
      return conditionalQuote(_url.isLocalFile() ? _url.path() : _url.url(), quote);
   if (option == 'd')
      return conditionalQuote(_url.directory(), quote);
   if (option == 'f')
      return conditionalQuote(_url.path(), quote);
   if (option == 'n')
      return conditionalQuote(_url.fileName(), quote);
   if (option == 'v')
   {
      if ( _url.isLocalFile() && QFile::exists( _url.path() ) )
      {
          KDesktopFile desktopFile(_url.path(), true);
          return conditionalQuote(desktopFile.readEntry( "Dev" ), quote);
      }
   }
	return QString::null;
}

static QStringList substitution(int option, const KService &_service, bool quote)
{
   QStringList result;
   if (option == 'c')
      result << conditionalQuote(_service.name(), quote);
   else if (option == 'i')
      result << "-icon" << conditionalQuote(_service.icon(), quote);
   else if (option == 'm')
      result << "-miniicon" << conditionalQuote(_service.icon(), quote);
   else if (option == 'k')
      result << conditionalQuote(_service.desktopEntryPath(), quote);

   if (result.isEmpty())
      result << QString::null;
   return result;
}

static QStringList substitution(int option, const KURL::List &_urls, bool quote)
{
   QStringList result;
   option = option - 'A' + 'a'; // To lower
   for(KURL::List::ConstIterator it = _urls.begin();
       it != _urls.end(); ++it)
   {
       result.append(substitution(option, *it, quote));
   }
   return result;
}

static void substitute(QStringList &_list, QStringList::Iterator &it, const KService &_service, const KURL::List &_urls, bool quote, bool service_only=false)
{
  QString &arg = *it;
  if ((arg.length() == 2) && (arg[0] == '%'))
  {
     int option = arg[1].unicode();
     QStringList subs;
     switch(option)
     {
        case 'U':
        case 'F':
        case 'D':
        case 'N':
          if (service_only)
             return;
          subs = substitution(option, _urls, quote);
          break;

        case 'u':
        case 'f':
        case 'd':
        case 'n':
        case 'v':
          if (service_only)
             return;
          if (_urls.count())
             subs.append(substitution(option, _urls.first(), quote));
          break;

        case 'c':
        case 'i':
        case 'm':
        case 'k':
          subs = substitution(option, _service, quote);
          break;

        case '%':
          subs.append("%");
          break;
     }

     if (subs.count() == 1)
     {
        arg = subs[0];
     }
     else
     {
        for(QStringList::Iterator it_subs = subs.begin();
            it_subs != subs.end(); ++it_subs)
        {
           _list.insert(it, *it_subs);
        }
        QStringList::Iterator delete_it = it;
        --it;
        _list.remove(delete_it);
     }
     return;
  }

  QStringList args = breakup(arg);
  if (args.isEmpty())
  {
     arg = QString::null;
     return;
  }
  else if (args.count() != 1)
  {
     arg = QString::null;
     for(QStringList::Iterator it = args.begin();
         it != args.end(); ++it)
     {
        substitute(args, it, _service, _urls, true, service_only);
     }
     arg = QString::null;
     for(QStringList::Iterator it = args.begin();
         it != args.end(); ++it)
     {
        if (!arg.isEmpty())
           arg += " ";
        arg += *it;
     }
     if (quote)
        KRun::shellQuote(arg);
     return;
  }
  arg = args[0];

  bool need_quote = false;
  int l = arg.length();
  int p = 0;
  while (p < l-1)
  {
     if (arg[p] == '%')
     {
        need_quote = true;
        int option = arg[++p].unicode();
        if (service_only &&
            ((option == 'u') || (option == 'f') || (option == 'd') || (option == 'n')))
           continue;

        QString sub;
        QStringList subs;
        switch(option)
        {
          case 'u':
          case 'f':
          case 'd':
          case 'n':
          case 'v':
            sub = substitution(option, _urls.first(), false);
            break;

          case 'c':
          case 'k':
            subs = substitution(option, _service, false);
            if (!subs.isEmpty())
               sub = subs[0];
            break;
          case '%':
            sub = "%";
            break;
        }

        arg.replace(p-1, 2, sub);
        p += sub.length()-2;
        l = arg.length();
     }
     p++;
  }
  if (quote && need_quote)
  {
     KRun::shellQuote(arg);
  }
}

QStringList KRun::processDesktopExec(const KService &_service, const KURL::List& _urls, bool has_shell)
{
  QString exec = _service.exec();
  QString user = _service.username();
  // Did the user forget to append something like '%f' ?
  // If so, then assume that '%f' is the right choice => the application
  // accepts only local files.
  if ( exec.find( "%f" ) == -1 && exec.find( "%u" ) == -1 && exec.find( "%n" ) == -1 &&
       exec.find( "%d" ) == -1 && exec.find( "%F" ) == -1 && exec.find( "%U" ) == -1 &&
       exec.find( "%N" ) == -1 && exec.find( "%D" ) == -1 && exec.find( "%v" ) == -1 )
    exec += " %f";

  bool terminal_su = false;
  bool terminal_sh = false;
  bool kdesu = false;

  if (_service.substituteUid() && !user.isEmpty())
  {
    if (_service.terminal())
      terminal_su = true;
    else
      kdesu = true;
  }
  else if (_service.terminal())
  {
    terminal_sh = true;
  }

  // Check if we need kfmexec.
  bool b_local_app = ( exec.find( "%u" ) == -1 && exec.find( "%U" ) == -1 );
  bool b_local_files = true;
  KURL::List::ConstIterator it = _urls.begin();
  for( ; it != _urls.end(); ++it )
    if ( !(*it).isLocalFile() )
      b_local_files = false;

  if ( b_local_app && !b_local_files )
  {
     // We need to run the app through kfmexec
     QStringList result = breakup(exec);

     // Substitute everything that isn't file-related.
     for(QStringList::Iterator it = result.begin();
         it != result.end(); ++it)
     {
         substitute(result, it, _service, _urls, true, true);
     }
     QString cmd = result.join(" ");
     if (has_shell)
        shellQuote(cmd);
     result.clear();
     result << "kfmexec" << cmd;
     KURL::List::ConstIterator it = _urls.begin();
     for( ; it != _urls.end(); ++it )
     {
        QString url = (*it).url();
        if (has_shell)
           shellQuote(url);
        result << url;
     }
     return result;
  }

  // Move args to result
  bool need_shell = false;
  QStringList result = breakup(exec, &need_shell);

  for(QStringList::Iterator it = result.begin();
      it != result.end(); ++it)
  {
      substitute(result, it, _service, _urls, has_shell || need_shell);
  }

  if (need_shell && !terminal_su && !kdesu &&
      (!has_shell || terminal_sh))
  {
     QString cmd = result.join(" ");
     result.clear();
     result << "/bin/sh" << "-c" << cmd;
  }

  KConfigGroupSaver gs(KGlobal::config(), "General");
  QString terminal = KGlobal::config()->readEntry("TerminalApplication", "konsole");
  
  if (terminal == "konsole")
    terminal += " -caption=%c %i %m";

  if (terminal_su)
  {
    QString cmd = result.join(" ");
    result = breakup(QString("%1 %2 -e su %3 -c").arg(terminal).arg(_service.terminalOptions()).arg(user));
    for(QStringList::Iterator it = result.begin();
        it != result.end(); ++it)
    {
        substitute(result, it, _service, _urls, has_shell);
    }
    result.append(cmd);
  }
  else if (terminal_sh)
  {
     QStringList cmd = result;
     result = breakup(QString("%1 %2 -e").arg(terminal).arg(_service.terminalOptions()));
     for(QStringList::Iterator it = result.begin();
         it != result.end(); ++it)
     {
         substitute(result, it, _service, _urls, has_shell);
     }
      result += cmd;
  }
  else if (kdesu)
  {
     result = breakup(QString("kdesu -u %1 --").arg(user))+result;
  }

  return result;
}

//static
QString KRun::binaryName( const QString & execLine, bool removePath )
{
  // Remove parameters and/or trailing spaces.
  QStringList args = breakup( execLine );
  QString _bin_name;
  do {
      if ( args.isEmpty() )
         return QString::null;
      _bin_name = args.first();
      args.pop_front();
  } while (_bin_name.contains('='));
  // Remove path if wanted
  return removePath ? _bin_name.mid(_bin_name.findRev('/') + 1) : _bin_name;
}

static pid_t runCommandInternal( KProcess* proc, const QString& binName,
    const QString &execName_P, const QString & iconName_P )
{
  QString bin = KRun::binaryName( binName, false );
  QString execName = execName_P;
  QString iconName = iconName_P;
#ifdef Q_WS_X11 // Startup notification doesn't work with QT/E, service isn't needed without Startup notification
  KService::Ptr service = 0;
  // Find service, if starting a .desktop file
  // (not when starting an executable)
  if ( KDesktopFile::isDesktopFile( bin ) )
  {
      if (!KDesktopFile::isAuthorizedDesktopFile( bin))
      {
         KMessageBox::sorry(0, i18n("You are not authorized to execute this file."));
         return 0;
      }
      if( bin[0] == '/' ) // Full path
          service = new KService( bin );
      else
          service = KService::serviceByDesktopName( bin );
  }
  bool startup_notify = false;
  QCString wmclass;
  KStartupInfoId id;
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
      data.setBin( KRun::binaryName( binName, true ));
      data.setName( execName );
      data.setIcon( iconName );
      if( !wmclass.isEmpty())
          data.setWMClass( wmclass );
      data.setDesktop( KWin::currentDesktop());
      KStartupInfo::sendStartup( id, data );
  }
  pid_t pid = KProcessRunner::run( proc, KRun::binaryName( binName, true ), id );
  if( startup_notify )
  {
      KStartupInfoData data;
      data.addPid( pid );
      KStartupInfo::sendChange( id, data );
      KStartupInfo::resetStartupEnv();
  }
  return pid;
#else
  return KProcessRunner::run( proc, KRun::binaryName( binName, true ) );
#endif
}

static pid_t runTempService( const KService& _service, const KURL::List& _urls )
{
  if (!_urls.isEmpty()) {
    kdDebug(7010) << "First url " << _urls.first().url() << endl;
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
      for(++it; it != _urls.end(); ++it)
      {
         KURL::List singleUrl;
         singleUrl.append(*it);
         runTempService( _service, singleUrl );
      }
      KURL::List singleUrl;
      singleUrl.append(_urls.first());
      args = KRun::processDesktopExec(_service, singleUrl, false);
  }
  else
  {
      args = KRun::processDesktopExec(_service, _urls, false);
  }

  KProcess * proc = new KProcess;
  for(QStringList::Iterator it = args.begin();
      it != args.end(); ++it)
  {
     QString arg = *it;
     *proc << arg;
  }
  return runCommandInternal( proc, _service.exec(), _service.name(), _service.icon() );
}

pid_t KRun::run( const KService& _service, const KURL::List& _urls )
{
  if (!KDesktopFile::isAuthorizedDesktopFile( _service.desktopEntryPath()))
  {
     KMessageBox::sorry(0, i18n("You are not authorized to execute this service."));
     return 0;
  }

  // Remember we opened those urls, for the "recent documents" menu in kicker
  KURL::List::ConstIterator it = _urls.begin();
  for(; it != _urls.end(); ++it) {
     //kdDebug(7010) << "KRecentDocument::adding " << (*it).url() << endl;
     KRecentDocument::add( *it, _service.desktopEntryName() );
  }

  if (_service.desktopEntryPath().isEmpty())
  {
     return runTempService(_service, _urls);
  }

  kdDebug(7010) << "KRun::run " << _service.desktopEntryPath() << endl;

  if (!_urls.isEmpty()) {
    kdDebug(7010) << "First url " << _urls.first().url() << endl;
  }

  QString error;
  int pid = 0;

  int i = KApplication::startServiceByDesktopPath(
        _service.desktopEntryPath(), _urls.toStringList(), &error, 0L, &pid
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
  return runCommandInternal( proc, binaryName( cmd, false ), execName, iconName );
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
  kdDebug(7010) << " new KRun " << this << " " << _url.prettyURL() << " timer=" << &m_timer << endl;

  kapp->ref();
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
        KMessageBoxWrapper::error( 0L, i18n( "<qt>Unable to run the command specified. The file or directory <b>%1</b> does not exist.</qt>" ).arg( m_strURL.htmlURL() ) );
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
  kdDebug() << " Job " << job << " is about stating " << m_strURL.url() << endl;
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
  connect(job, SIGNAL( result(KIO::Job *)),
          this, SLOT( slotScanFinished(KIO::Job *)));
  connect(job, SIGNAL( mimetype(KIO::Job *, const QString &)),
          this, SLOT( slotScanMimeType(KIO::Job *, const QString &)));
  m_job = job;
  kdDebug() << " Job " << job << " is about getting from " << m_strURL.url() << endl;
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
  if (m_job && m_job->inherits("KIO::TransferJob"))
  {
     KIO::TransferJob *job = static_cast<KIO::TransferJob *>(m_job);
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

  if (!m_bFinished && KRun::runURL( m_strURL, type )){
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
  kdDebug() << "KRun::abort " << this << " m_showingError=" << d->m_showingError << endl;
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

void KRun::setPreferredService( const QString& desktopEntryName )
{
    d->m_preferredService = desktopEntryName;
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
    // Before assuming its really so, we'll try to find the binName
    // relatively to current directory,  and then in the PATH.
    if ( !QFile( binName ).exists() && KStandardDirs::findExe( binName ).isEmpty() )
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

void KRun::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "krun.moc"
