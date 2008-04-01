/*
   This file is part of the KDE libraries
   Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>
   Copyright (C) 2006 Ralf Habacker <ralf.habacker@freenet.de>

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

#include "ktoolinvocation.h"
#include <config.h>

#include "kmessage.h"
#include "klocale.h"
#include <QProcess>
#include <QtCore/QCoreApplication>
#include <QtCore/QHash>
#include <QtDBus/QtDBus>
#include <kcomponentdata.h>
#include <klockfile.h>
#include <kstandarddirs.h>

#include "windows.h"
#include "shellapi.h"


void KToolInvocation::invokeHelp( const QString& anchor,
                               const QString& _appname,
                               const QByteArray& startup_id )
{
    if (!isMainThreadActive())
        return;

    QString url;
    QString appname;
    if (_appname.isEmpty()) {
        appname = QCoreApplication::instance()->applicationName();
    } else
        appname = _appname;

    if (!anchor.isEmpty())
        url = QString("help:/%1?anchor=%2").arg(appname).arg(anchor);
    else
        url = QString("help:/%1/index.html").arg(appname);

    QDBusInterface *iface = new QDBusInterface(QLatin1String("org.kde.khelpcenter"),
                                               QLatin1String("/KHelpCenter"),
                                               QLatin1String("org.kde.khelpcenter.khelpcenter"),
                                               QDBusConnection::sessionBus());
    if ( !iface->isValid() )
    {
        QString error;
#if 1
		if (kdeinitExec( "khelpcenter", QStringList() << url, &error, 0, startup_id ))
#else
		// does not work yet KRun:processDesktopExec returned 'KRun: syntax error in command "khelpcenter %u" , service "KHelpCenter" '
        if (startServiceByDesktopName("khelpcenter", url, &error, 0, 0, startup_id, false))
#endif
        {
            KMessage::message(KMessage::Error,
                              i18n("Could not launch the KDE Help Center:\n\n%1", error),
                              i18n("Could not Launch Help Center"));
            return;
        }
        delete iface;
        iface = new QDBusInterface(QLatin1String("org.kde.khelpcenter"),
                                   QLatin1String("/KHelpCenter"),
                                   QLatin1String("org.kde.khelpcenter.khelpcenter"),
                                   QDBusConnection::sessionBus());
    }

    iface->call("openUrl", url, startup_id );
    delete iface;
}


void KToolInvocation::invokeBrowser( const QString &url, const QByteArray& startup_id )
{
/*
   QString sOpen( "open" );
   ShellExecuteW(0, ( LPCWSTR )sOpen.utf16(), ( LPCWSTR )url.utf16(), 0, 0, SW_NORMAL);
*/
}

void KToolInvocation::invokeMailer(const QString &address, const QString &subject, const QByteArray& startup_id)
{
/*
   invokeMailer(address, QString::null, QString::null, subject, QString::null, QString::null,
       QStringList(), startup_id );
*/
}

void KToolInvocation::invokeMailer(const KUrl &mailtoURL, const QByteArray& startup_id, bool allowAttachments )
{
/*
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
		 cc = cc.isEmpty()? KUrl::fromPercentEncoding((*it).mid(3).toLatin1()): cc.toLatin1() + ',' + KUrl::fromPercentEncoding((*it).mid(3).toLatin1());
     else
     if (q.startsWith("bcc="))
		 bcc = bcc.isEmpty()? KUrl::fromPercentEncoding((*it).mid(4).toLatin1()): bcc.toLatin1() + ',' + KUrl::fromPercentEncoding((*it).mid(4).toLatin1());
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

   invokeMailer( address, cc, bcc, subject, body, QString::null, attachURLs, startup_id );
*/
}

void KToolInvocation::invokeMailer(const QString &_to, const QString &_cc, const QString &_bcc,
                                const QString &subject, const QString &body,
                                const QString & /*messageFile TODO*/, const QStringList &attachURLs,
                                const QByteArray& startup_id )
{
/*
	KUrl url("mailto:"+_to);
	url.setQuery("?subject="+subject);
	url.addQueryItem("cc", _cc);
	url.addQueryItem("bcc", _bcc);
	url.addQueryItem("body", body);
	for (QStringList::ConstIterator it = attachURLs.constBegin(); it != attachURLs.constEnd(); ++it)
		url.addQueryItem("attach", QLatin1String( KUrl::toPercentEncoding(*it) ));

   QString sOpen( "open" );
   ShellExecuteW(0, ( LPCWSTR )sOpen.utf16(), ( LPCWSTR )url.url().utf16(), 0, 0, SW_NORMAL);
*/
}

void KToolInvocation::invokeTerminal(const QString &command, const QByteArray &startup_id)
{
/*
    if (!isMainThreadActive()) {
        return;
    }

    KConfigGroup confGroup( KGlobal::config(), "General" );
    QString exec = confGroup.readPathEntry("TerminalApplication", "konsole");
    exec = exec + " -e " + command;

    QStringList cmdTokens = KShell::splitArgs(command);
    QString cmd = cmdTokens.takeFirst();

    QString error;
    if (kdeinitExec(cmd, cmdTokens, &error, NULL, startup_id ))
    {
      KMessage::message(KMessage::Error,
                      i18n("Could not launch the mail client:\n\n%1", error),
                      i18n("Could not Launch Mail Client"));
    }
*/
}

void KToolInvocation::startKdeinit()
{
   KComponentData inst( "startkdeinitlock" );
   KLockFile lock( KStandardDirs::locateLocal( "tmp", "startkdeinitlock", inst ));
   if( lock.lock( KLockFile::NoBlockFlag ) != KLockFile::LockOK ) {
     lock.lock();
     if( QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.kde.klauncher" ))
         return; // whoever held the lock has already started it
   }
   QProcess::execute("kdeinit4");
}
