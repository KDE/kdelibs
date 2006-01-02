/*
   This file is part of the KDE libraries
   Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

//#include "ktoolinvocation_x11.cpp" 

#define THREADGUARD 
#include "config.h"

#include "ktoolinvocation.h"
#include "kcmdlineargs.h"
#include "kconfig.h"
#include "kcodecs.h"
#include "kglobal.h"
#include "kshell.h"
#include "kmacroexpander.h"
#include "klocale.h"
#include <qmessagebox.h>
#include <qapplication.h>
#include <qhash.h>

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

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <qglobal.h>
#ifdef __GNUC__
#warning used non public api for now
#endif

KToolInvocation* KToolInvocation::s_self = 0L;

KToolInvocation::KToolInvocation(QObject *parent):QObject(parent) {
}

KToolInvocation *KToolInvocation::self() {
	if (s_self==0) {
		Q_ASSERT(qApp);
		if (!qApp) qFatal("KToolInvocation::self():No application object");
		s_self=new KToolInvocation(qApp);
	}
	return s_self;
}

KToolInvocation::~KToolInvocation() {
	s_self=0;
}

DCOPClient *KToolInvocation::dcopClient() {
	return 0;
}

static int
startServiceInternal(DCOPClient *dcopClient, const QByteArray &function,
              const QString& _name, const QStringList &URLs,
              QString *error, QByteArray *dcopService, int *pid, const QByteArray& startup_id, bool noWait )
{
	qWarning("%s %s %s %s\n",__FUNCTION__,function.data(),_name.latin1(),URLs.join(" ").latin1());
/*
   struct serviceResult
   {
      int result;
      DCOPCString dcopName;
      QString error;
      pid_t pid;
   };

   // Register app as able to send DCOP messages
   if (!dcopClient) {
         if (error)
            *error = i18n("Could not register with DCOP.\n");
         return -1;
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
        if (error)
           *error = i18n("KLauncher could not be reached via DCOP.\n");
        return -1;
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
*/
	return 0;
}

int
KToolInvocation::startServiceByName( const QString& _name, const QString &URL,
                  QString *error, QByteArray *dcopService, int *pid, const QByteArray& startup_id, bool noWait )
{
   THREADGUARD;
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
   THREADGUARD;
   return startServiceInternal(dcopClient(),
                      "start_service_by_name(QString,QStringList,QValueList<QCString>,QCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}

int
KToolInvocation::startServiceByDesktopPath( const QString& _name, const QString &URL,
                  QString *error, QByteArray *dcopService, int *pid, const QByteArray& startup_id, bool noWait )
{
   THREADGUARD;
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
   THREADGUARD;
   return startServiceInternal(dcopClient(),
                      "start_service_by_desktop_path(QString,QStringList,QValueList<QCString>,QCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}

int
KToolInvocation::startServiceByDesktopName( const QString& _name, const QString &URL,
                  QString *error, QByteArray *dcopService, int *pid, const QByteArray& startup_id, bool noWait )
{
   THREADGUARD;
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
   THREADGUARD;
   return startServiceInternal(dcopClient(),
                      "start_service_by_desktop_name(QString,QStringList,QValueList<QCString>,QCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}


int
KToolInvocation::kdeinitExec( const QString& name, const QStringList &args,
                           QString *error, int *pid, const QByteArray& startup_id )
{
   THREADGUARD;
   return startServiceInternal(dcopClient(),"kdeinit_exec(QString,QStringList,QValueList<QCString>,QCString)",
        name, args, error, 0, pid, startup_id, false);
}


int
KToolInvocation::kdeinitExecWait( const QString& name, const QStringList &args,
                           QString *error, int *pid, const QByteArray& startup_id )
{
   THREADGUARD;
   return startServiceInternal(dcopClient(),"kdeinit_exec_wait(QString,QStringList,QValueList<QCString>,QCString)",
        name, args, error, 0, pid, startup_id, false);
}


#if 1  // TODO: merge with relating parts from x11 

#include <kapplication.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kurl.h>

#include "kcheckaccelerators.h"
#include "kappdcopiface.h"

#include <qassistantclient.h>
#include <qdir.h>

#include "windows.h"
#include "shellapi.h"


void KToolInvocation::invokeHelp( const QString& anchor,
                               const QString& _appname,
                               const QByteArray& startup_id )
{
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

   QString error;
   startServiceByDesktopName("khelpcenter", url, &error, 0, 0, startup_id, false);
}


void KToolInvocation::invokeBrowser( const QString &url, const QByteArray& startup_id )
{
	QByteArray s = url.latin1();
	const unsigned short *l = (const unsigned short *)s.data();
	ShellExecuteA(0, "open", s.data(), 0, 0, SW_NORMAL);
}

void KToolInvocation::invokeMailer(const QString &address, const QString &subject, const QByteArray& startup_id)
{
   invokeMailer(address, QString::null, QString::null, subject, QString::null, QString::null,
       QStringList(), startup_id );
}

void KToolInvocation::invokeMailer(const KURL &mailtoURL, const QByteArray& startup_id, bool allowAttachments )
{
   QString address = KURL::decode_string(mailtoURL.path()), subject, cc, bcc, body;
   QStringList queries = mailtoURL.query().mid(1).split( '&');
   QStringList attachURLs;
   for (QStringList::Iterator it = queries.begin(); it != queries.end(); ++it)
   {
     QString q = (*it).toLower();
     if (q.startsWith("subject="))
       subject = KURL::decode_string((*it).mid(8));
     else
     if (q.startsWith("cc="))
       cc = cc.isEmpty()? KURL::decode_string((*it).mid(3)): cc + ',' + KURL::decode_string((*it).mid(3));
     else
     if (q.startsWith("bcc="))
       bcc = bcc.isEmpty()? KURL::decode_string((*it).mid(4)): bcc + ',' + KURL::decode_string((*it).mid(4));
     else
     if (q.startsWith("body="))
       body = KURL::decode_string((*it).mid(5));
     else
     if (allowAttachments && q.startsWith("attach="))
       attachURLs.push_back(KURL::decode_string((*it).mid(7)));
     else
     if (allowAttachments && q.startsWith("attachment="))
       attachURLs.push_back(KURL::decode_string((*it).mid(11)));
     else
     if (q.startsWith("to="))
       address = address.isEmpty()? KURL::decode_string((*it).mid(3)): address + ',' + KURL::decode_string((*it).mid(3));
   }

   invokeMailer( address, cc, bcc, subject, body, QString::null, attachURLs, startup_id );
}

void KToolInvocation::invokeMailer(const QString &_to, const QString &_cc, const QString &_bcc,
                                const QString &subject, const QString &body,
                                const QString & /*messageFile TODO*/, const QStringList &attachURLs,
                                const QByteArray& startup_id )
{
	KURL url("mailto:"+_to);
	url.setQuery("?subject="+subject);
	url.addQueryItem("cc", _cc);
	url.addQueryItem("bcc", _bcc);
	url.addQueryItem("body", body);
	for (QStringList::ConstIterator it = attachURLs.constBegin(); it != attachURLs.constEnd(); ++it)
		url.addQueryItem("attach", KURL::encode_string(*it));

	QByteArray s = url.url().latin1();
	const unsigned short *l = (const unsigned short *)s.data();
	ShellExecuteA(0, "open", s.data(), 0, 0, SW_NORMAL);
}
#endif
