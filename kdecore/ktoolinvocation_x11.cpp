/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (C) 1998, 1999, 2000 KDE Team

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

#include "config.h"

#include "ktoolinvocation.h"
#include "kcmdlineargs.h"
#include "kconfig.h"
#include "kcodecs.h"
#include "kdebug.h"
#include "kglobal.h"
#include "kshell.h"
#include "kmacroexpander.h"
#include "klocale.h"
#include <qmessagebox.h>
#include <qapplication.h>
#include <qhash.h>

#if defined Q_WS_X11
#include <QtGui/qx11info_x11.h>
#include <kstartupinfo.h>
#endif

#include <dcopclient.h>
#include <dcopref.h>

#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <sys/wait.h>

#ifndef Q_WS_WIN
#include "kwin.h"
#endif

#include <fcntl.h>
#include <stdlib.h> // getenv(), srand(), rand()
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>

#include "kprocctrl.h"

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/SM/SMlib.h>
#include <fixx11h.h>
#endif

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <qglobal.h>
#ifdef __GNUC__
#warning used non public api for now
#endif
Q_GLOBAL_STATIC_WITH_ARGS(QMutex,mutex,(QMutex::Recursive))


static void printError(const QString& text, QString* error)
{
    if (error)
        *error = text;
    else
        kError() << text << endl;
}


static bool isMainThreadActive(QString* error = 0)
{
    if (!qApp)
    {
        printError(i18n("QApplication required."), error);
        return false;
    }

    if (qApp->thread() != QThread::currentThread())
    {
        printError(i18n("Function must be called from the main thread."), error);
        return false;
    }

    return true;
}


KToolInvocation* KToolInvocation::s_self = 0L;

KToolInvocation::KToolInvocation(QObject *parent):QObject(parent) {
	m_dcopClient=new DCOPClient();
        if (qApp->thread()!=QThread::currentThread()) m_dcopClient->moveToThread(qApp->thread());
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");
	if (args && args->isSet("dcopserver")) {
		m_dcopClient->setServerAddress( args->getOption("dcopserver"));
	}
}

KToolInvocation *KToolInvocation::self() {
	QMutexLocker locker(mutex());
	if (s_self==0) {
		Q_ASSERT(qApp);
		if (!qApp) qFatal("KToolInvocation::self():No application object");
		s_self=new KToolInvocation(qApp);
	}
	return s_self;
}

KToolInvocation::~KToolInvocation() {
	QMutexLocker locker(mutex());
	s_self=0;
	delete m_dcopClient;
}

DCOPClient *KToolInvocation::dcopClient() {
	KToolInvocation *kti=self();
	if (!kti->m_dcopClient->isAttached())
		if (!kti->m_dcopClient->attach()) return 0;
	return kti->m_dcopClient;
}


void KToolInvocation::invokeHelp( const QString& anchor,
                               const QString& _appname,
                               const QByteArray& startup_id )
{
   if (!isMainThreadActive())
     return;

   QString url;
   QString appname;
   if (_appname.isEmpty()) {
     appname = qApp->applicationName();
   } else
     appname = _appname;

   if (!anchor.isEmpty())
     url = QString("help:/%1?anchor=%2").arg(appname).arg(anchor);
   else
     url = QString("help:/%1/index.html").arg(appname);

   DCOPClient *dcopClient=KToolInvocation::dcopClient();
   if (!dcopClient) return;
   QString error;
   if ( !dcopClient->isApplicationRegistered("khelpcenter") )
   {
       if (startServiceByDesktopName("khelpcenter", url, &error, 0, 0, startup_id, false))
       {
#ifdef __GNUC__
#warning fixme once there is a kcoreapp and kapp class
#endif
#if 0
           if (Tty != qApp->type())
               QMessageBox::critical(kapp->mainWidget(), i18n("Could not Launch Help Center"),
               i18n("Could not launch the KDE Help Center:\n\n%1").arg(error), i18n("&OK"));
           else
               kWarning() << "Could not launch help:\n" << error << endl;
	   return;
#endif
       }
   }
   else {
       DCOPRef ref( "khelpcenter", "KHelpCenterIface" );
       ref.setDCOPClient(dcopClient);
       ref.send( "openUrl", url, (const char*)startup_id );
   }
}




void KToolInvocation::invokeMailer(const KUrl &mailtoURL, const QByteArray& startup_id, bool allowAttachments )
{
   if (!isMainThreadActive())
     return;

   QString address = KUrl::fromPercentEncoding(mailtoURL.path().toLatin1()), subject, cc, bcc, body;
   QStringList queries = mailtoURL.query().mid(1).split( '&');
   QStringList attachURLs;
   for (QStringList::Iterator it = queries.begin(); it != queries.end(); ++it)
   {
     QString q = (*it).toLower();
     if (q.startsWith("subject="))
       subject = KUrl::fromPercentEncoding((*it).mid(8).toLatin1());
     else
     if (q.startsWith("cc="))
       cc = cc.isEmpty()? KUrl::fromPercentEncoding((*it).mid(3).toLatin1()): cc + ',' + KUrl::fromPercentEncoding((*it).mid(3).toLatin1());
     else
     if (q.startsWith("bcc="))
       bcc = bcc.isEmpty()? KUrl::fromPercentEncoding((*it).mid(4).toLatin1()): bcc + ',' + KUrl::fromPercentEncoding((*it).mid(4).toLatin1());
     else
     if (q.startsWith("body="))
       body = KUrl::fromPercentEncoding((*it).mid(5).toLatin1());
     else
     if (allowAttachments && q.startsWith("attach="))
       attachURLs.push_back(KUrl::fromPercentEncoding((*it).mid(7).toLatin1()));
     else
     if (allowAttachments && q.startsWith("attachment="))
       attachURLs.push_back(KUrl::fromPercentEncoding((*it).mid(11).toLatin1()));
     else
     if (q.startsWith("to="))
       address = address.isEmpty()? KUrl::fromPercentEncoding((*it).mid(3).toLatin1()): address + ',' + KUrl::fromPercentEncoding((*it).mid(3).toLatin1());
   }

   invokeMailer( address, cc, bcc, subject, body, QString(), attachURLs, startup_id );
}


static QStringList splitEmailAddressList( const QString & aStr )
{
  // This is a copy of KPIM::splitEmailAddrList().
  // Features:
  // - always ignores quoted characters
  // - ignores everything (including parentheses and commas)
  //   inside quoted strings
  // - supports nested comments
  // - ignores everything (including double quotes and commas)
  //   inside comments

  QStringList list;

  if (aStr.isEmpty())
    return list;

  QString addr;
  uint addrstart = 0;
  int commentlevel = 0;
  bool insidequote = false;

  for (int index=0; index<aStr.length(); index++) {
    // the following conversion to latin1 is o.k. because
    // we can safely ignore all non-latin1 characters
    switch (aStr[index].toLatin1()) {
    case '"' : // start or end of quoted string
      if (commentlevel == 0)
        insidequote = !insidequote;
      break;
    case '(' : // start of comment
      if (!insidequote)
        commentlevel++;
      break;
    case ')' : // end of comment
      if (!insidequote) {
        if (commentlevel > 0)
          commentlevel--;
        else {
          //kDebug() << "Error in address splitting: Unmatched ')'"
          //          << endl;
          return list;
        }
      }
      break;
    case '\\' : // quoted character
      index++; // ignore the quoted character
      break;
    case ',' :
      if (!insidequote && (commentlevel == 0)) {
        addr = aStr.mid(addrstart, index-addrstart);
        if (!addr.isEmpty())
          list += addr.simplified();
        addrstart = index+1;
      }
      break;
    }
  }
  // append the last address to the list
  if (!insidequote && (commentlevel == 0)) {
    addr = aStr.mid(addrstart, aStr.length()-addrstart);
    if (!addr.isEmpty())
      list += addr.simplified();
  }
  //else
  //  kDebug() << "Error in address splitting: "
  //            << "Unexpected end of address list"
  //            << endl;

  return list;
}

void KToolInvocation::invokeMailer(const QString &address, const QString &subject, const QByteArray& startup_id)
{
   if (!isMainThreadActive())
     return;

   invokeMailer(address, QString(), QString(), subject, QString(), QString(),
       QStringList(), startup_id );
}

void KToolInvocation::invokeMailer(const QString &_to, const QString &_cc, const QString &_bcc,
                                const QString &subject, const QString &body,
                                const QString & /*messageFile TODO*/, const QStringList &attachURLs,
                                const QByteArray& startup_id )
{
   if (!isMainThreadActive())
     return;

   KConfig config("emaildefaults");

   config.setGroup("Defaults");
   QString group = config.readEntry("Profile","Default");

   config.setGroup( QString("PROFILE_%1").arg(group) );
   QString command = config.readPathEntry("EmailClient");

   QString to, cc, bcc;
   if (command.isEmpty() || command == QLatin1String("kmail")
       || command.endsWith("/kmail"))
   {
     command = QLatin1String("kmail --composer -s %s -c %c -b %b --body %B --attach %A -- %t");
     if ( !_to.isEmpty() )
     {
       // put the whole address lists into RFC2047 encoded blobs; technically
       // this isn't correct, but KMail understands it nonetheless
       to = QString( "=?utf8?b?%1?=" )
            .arg( (const char*)KCodecs::base64Encode( _to.toUtf8(), false ) );
     }
     if ( !_cc.isEmpty() )
       cc = QString( "=?utf8?b?%1?=" )
            .arg( (const char*)KCodecs::base64Encode( _cc.toUtf8(), false ) );
     if ( !_bcc.isEmpty() )
       bcc = QString( "=?utf8?b?%1?=" )
             .arg( (const char*)KCodecs::base64Encode( _bcc.toUtf8(), false ) );
   } else {
     to = _to;
     cc = _cc;
     bcc = _bcc;
   }

   if (config.readEntry("TerminalClient", false))
   {
     KConfigGroup confGroup( KGlobal::config(), "General" );
     QString preferredTerminal = confGroup.readPathEntry("TerminalApplication", "konsole");
     command = preferredTerminal + " -e " + command;
   }

   QStringList cmdTokens = KShell::splitArgs(command);
   QString cmd = cmdTokens[0];
   cmdTokens.erase(cmdTokens.begin());

   KUrl url;
   //QStringList qry;
   if (!to.isEmpty())
   {
     QStringList tos = splitEmailAddressList( to );
     url.setPath( tos.first() );
     tos.erase( tos.begin() );
     for (QStringList::ConstIterator it = tos.begin(); it != tos.end(); ++it)
       url.addQueryItem("to",*it);
       //qry.append( "to=" + QLatin1String(KUrl::toPercentEncoding( *it ) ));
   }
   const QStringList ccs = splitEmailAddressList( cc );
   for (QStringList::ConstIterator it = ccs.begin(); it != ccs.end(); ++it)
      url.addQueryItem("cc",*it);
      //qry.append( "cc=" + QLatin1String(KUrl::toPercentEncoding( *it ) ));
   const QStringList bccs = splitEmailAddressList( bcc );
   for (QStringList::ConstIterator it = bccs.begin(); it != bccs.end(); ++it)
      url.addQueryItem("bcc",*it);
      //qry.append( "bcc=" + QLatin1String(KUrl::toPercentEncoding( *it ) ));
   for (QStringList::ConstIterator it = attachURLs.begin(); it != attachURLs.end(); ++it)
      url.addQueryItem("attach",*it);
      //qry.append( "attach=" + QLatin1String(KUrl::toPercentEncoding( *it ) ));
   if (!subject.isEmpty())
      url.addQueryItem("subject",subject);
      //qry.append( "subject=" + QLatin1String(KUrl::toPercentEncoding( subject ) ));
   if (!body.isEmpty())
      url.addQueryItem("body",body);
      //qry.append( "body=" + QLatin1String(KUrl::toPercentEncoding( body ) ));
   //url.setQuery( qry.join( "&" ) );
   
   const bool hasQuery = !url.encodedQuery().isEmpty();
#if QT_VERSION >= 0x040200
#ifdef __GNUC__
#warning Qt-4.2, use QUrl::hasQuery()
#endif
#endif

   if ( ! (to.isEmpty() && (!hasQuery)) )
      url.setProtocol("mailto");

   QHash<QChar, QString> keyMap;
   keyMap.insert('t', to);
   keyMap.insert('s', subject);
   keyMap.insert('c', cc);
   keyMap.insert('b', bcc);
   keyMap.insert('B', body);
   keyMap.insert('u', url.url());

   for (QStringList::Iterator it = cmdTokens.begin(); it != cmdTokens.end(); )
   {
     if (*it == "%A")
     {
         if (it == cmdTokens.begin()) // better safe than sorry ...
             continue;
         QStringList::ConstIterator urlit = attachURLs.begin();
         QStringList::ConstIterator urlend = attachURLs.end();
         if ( urlit != urlend )
         {
             QStringList::Iterator previt = it;
             --previt;
             *it = *urlit;
             ++it;
             while ( ++urlit != urlend )
             {
                 cmdTokens.insert( it, *previt );
                 cmdTokens.insert( it, *urlit );
             }
         } else {
             --it;
             it = cmdTokens.erase( cmdTokens.erase( it ) );
         }
     } else {
         *it = KMacroExpander::expandMacros(*it, keyMap);
         ++it;
     }
   }

   QString error;
   // TODO this should check if cmd has a .desktop file, and use data from it, together
   // with sending more ASN data
   if (kdeinitExec(cmd, cmdTokens, &error, NULL, startup_id ))
{}
#ifdef __GNUC__
#warning fixme once there is kcoreapp and kapp
#endif
#if 0
     if (Tty != qApp->type())
       QMessageBox::critical(kapp->mainWidget(), i18n("Could not Launch Mail Client"),
             i18n("Could not launch the mail client:\n\n%1").arg(error), i18n("&OK"));
     else
       kWarning() << "Could not launch mail client:\n" << error << endl;
#endif
}

void KToolInvocation::invokeBrowser( const QString &url, const QByteArray& startup_id )
{
   if (!isMainThreadActive())
      return;

   QString error;

   if (startServiceByDesktopName("kfmclient", url, &error, 0, 0, startup_id, false))
   {
#ifdef __GNUC__
#warning fixme once there is kcoreapp and kapp
#endif
#if 0
      if (Tty != qApp->type())
          QMessageBox::critical(kapp->mainWidget(), i18n("Could not Launch Browser"),
               i18n("Could not launch the browser:\n\n%1").arg(error), i18n("&OK"));
      else
          kWarning() << "Could not launch browser:\n" << error << endl;
      return;
#endif
   }
}

QByteArray
KToolInvocation::launcher()
{
   return "klauncher";
}

static int
startServiceInternal(DCOPClient *dcopClient, const QByteArray &function,
              const QString& _name, const QStringList &URLs,
              QString *error, QByteArray *dcopService, int *pid, const QByteArray& startup_id, bool noWait )
{
   struct serviceResult
   {
      int result;
      DCOPCString dcopName;
      QString error;
      pid_t pid;
   };

   // Register app as able to send DCOP messages
   if (!dcopClient) {
      printError(i18n("Could not register with DCOP.\n"), error);
      return EINVAL;
   }
   QByteArray params;
   QDataStream stream(&params, QIODevice::WriteOnly);
   stream.setVersion(QDataStream::Qt_3_1);
   stream << _name << URLs;
   DCOPCString replyType;
   QByteArray  replyData;
   QByteArray _launcher = KToolInvocation::launcher();
   QList<DCOPCString> envs;
#ifdef Q_WS_X11
   if (QX11Info::display()) {
       QByteArray dpystring(XDisplayString(QX11Info::display()));
       envs.append( QByteArray("DISPLAY=") + dpystring );
   } else if( getenv( "DISPLAY" )) {
       QByteArray dpystring( getenv( "DISPLAY" ));
       envs.append( QByteArray("DISPLAY=") + dpystring );
   }
#endif
   stream << envs;
#if defined Q_WS_X11
   // make sure there is id, so that user timestamp exists
   stream << ( startup_id.isEmpty() ? DCOPCString(KStartupInfo::createNewStartupId()) :
                                      DCOPCString(startup_id) );
#endif
   if( function.left( 12 ) != "kdeinit_exec" )
       stream << noWait;

   if (!dcopClient->call(_launcher, _launcher,
        function, params, replyType, replyData))
   {
      printError(i18n("KLauncher could not be reached via DCOP.\n"), error);
      return EINVAL;
   }

   if (noWait)
      return 0;

   QDataStream stream2(&replyData, QIODevice::ReadOnly);
   stream2.setVersion(QDataStream::Qt_3_1);
   serviceResult result;
   stream2 >> result.result >> result.dcopName >> result.error >> result.pid;
   if (dcopService)
      *dcopService = result.dcopName;
   if (error)
      *error = result.error;
   if (pid)
      *pid = result.pid;
   return result.result;
}

int
KToolInvocation::startServiceByName( const QString& _name, const QString &URL,
                  QString *error, QByteArray *dcopService, int *pid, const QByteArray& startup_id, bool noWait )
{
   if (!isMainThreadActive(error))
      return EINVAL;

   QStringList URLs;
   if (!URL.isEmpty())
      URLs.append(URL);
   return startServiceInternal(dcopClient(),
                      "start_service_by_name(QString,QStringList,QValueList<QCString>,QCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}

int
KToolInvocation::startServiceByName( const QString& _name, const QStringList &URLs,
                  QString *error, QByteArray *dcopService, int *pid, const QByteArray& startup_id, bool noWait )
{
   if (!isMainThreadActive(error))
      return EINVAL;

   return startServiceInternal(dcopClient(),
                      "start_service_by_name(QString,QStringList,QValueList<QCString>,QCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}

int
KToolInvocation::startServiceByDesktopPath( const QString& _name, const QString &URL,
                  QString *error, QByteArray *dcopService, int *pid, const QByteArray& startup_id, bool noWait )
{
   if (!isMainThreadActive(error))
      return EINVAL;

   QStringList URLs;
   if (!URL.isEmpty())
      URLs.append(URL);
   return startServiceInternal(dcopClient(),
                      "start_service_by_desktop_path(QString,QStringList,QValueList<QCString>,QCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}

int
KToolInvocation::startServiceByDesktopPath( const QString& _name, const QStringList &URLs,
                  QString *error, QByteArray *dcopService, int *pid, const QByteArray& startup_id, bool noWait )
{
   if (!isMainThreadActive(error))
      return EINVAL;

   return startServiceInternal(dcopClient(),
                      "start_service_by_desktop_path(QString,QStringList,QValueList<QCString>,QCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}

int
KToolInvocation::startServiceByDesktopName( const QString& _name, const QString &URL,
                  QString *error, QByteArray *dcopService, int *pid, const QByteArray& startup_id, bool noWait )
{
   if (!isMainThreadActive(error))
      return EINVAL;

   QStringList URLs;
   if (!URL.isEmpty())
      URLs.append(URL);
   return startServiceInternal(dcopClient(),
                      "start_service_by_desktop_name(QString,QStringList,QValueList<QCString>,QCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}

int
KToolInvocation::startServiceByDesktopName( const QString& _name, const QStringList &URLs,
                  QString *error, QByteArray *dcopService, int *pid, const QByteArray& startup_id, bool noWait )
{
   if (!isMainThreadActive(error))
      return EINVAL;

   return startServiceInternal(dcopClient(),
                      "start_service_by_desktop_name(QString,QStringList,QValueList<QCString>,QCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}


int
KToolInvocation::kdeinitExec( const QString& name, const QStringList &args,
                           QString *error, int *pid, const QByteArray& startup_id )
{
   if (!isMainThreadActive(error))
      return EINVAL;

   return startServiceInternal(dcopClient(),"kdeinit_exec(QString,QStringList,QValueList<QCString>,QCString)",
        name, args, error, 0, pid, startup_id, false);
}


int
KToolInvocation::kdeinitExecWait( const QString& name, const QStringList &args,
                           QString *error, int *pid, const QByteArray& startup_id )
{
   if (!isMainThreadActive(error))
      return EINVAL;

   return startServiceInternal(dcopClient(),"kdeinit_exec_wait(QString,QStringList,QValueList<QCString>,QCString)",
        name, args, error, 0, pid, startup_id, false);
}

