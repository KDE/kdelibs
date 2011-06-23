/* This file is part of the KDE libraries
    Copyright (C) 2005 Brad Hards <bradh@frogmouth.net>
    Copyright (C) 2006 Thiago Macieira <thiago@kde.org>

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

#include "ktoolinvocation.h"
#include "klauncher_iface.h"
#include "kdebug.h"
#include "kglobal.h"
#include "kstandarddirs.h"
#include "kcomponentdata.h"
#include "kurl.h"
#include "kmessage.h"
#include "kservice.h"
#include <klockfile.h>
#include <klocale.h>

#include <QMutex>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QThread>

#include <errno.h>


KToolInvocation *KToolInvocation::self()
{
    K_GLOBAL_STATIC(KToolInvocation, s_self)
    return s_self;
}

KToolInvocation::KToolInvocation() : QObject(0), d(0)
{
}

KToolInvocation::~KToolInvocation()
{
}

Q_GLOBAL_STATIC_WITH_ARGS(org::kde::KLauncher, klauncherIface,
                          (QString::fromLatin1("org.kde.klauncher"), QString::fromLatin1("/KLauncher"), QDBusConnection::sessionBus()))

org::kde::KLauncher *KToolInvocation::klauncher()
{
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(QString::fromLatin1("org.kde.klauncher"))) {
        kDebug(180) << "klauncher not running... launching kdeinit";
        KToolInvocation::startKdeinit();
    }
    return ::klauncherIface();
}

static void printError(const QString& text, QString* error)
{
    if (error)
        *error = text;
    else
        kError() << text << endl;
}

bool KToolInvocation::isMainThreadActive(QString* error)
{
    if (QCoreApplication::instance() && QCoreApplication::instance()->thread() != QThread::currentThread())
    {
        printError(i18n("Function must be called from the main thread."), error);
        return false;
    }

    return true;
}

int KToolInvocation::startServiceInternal(const char *_function,
                                          const QString& _name, const QStringList &URLs,
                                          QString *error, QString *serviceName, int *pid,
                                          const QByteArray& startup_id, bool noWait,
                                          const QString& workdir)
{
    QString function = QLatin1String(_function);
    org::kde::KLauncher *launcher = KToolInvocation::klauncher();
    QDBusMessage msg = QDBusMessage::createMethodCall(launcher->service(),
                                                launcher->path(),
                                                launcher->interface(),
                                                function);
    msg << _name << URLs;
    if (function == QLatin1String("kdeinit_exec_with_workdir"))
        msg << workdir;
#ifdef Q_WS_X11
    // make sure there is id, so that user timestamp exists
    QStringList envs;
    QByteArray s = startup_id;
    emit kapplication_hook(envs, s);
    msg << envs;
    msg << QString::fromLatin1(s);
#else
    msg << QStringList();
    msg << QString();
#endif
    if( !function.startsWith( QLatin1String("kdeinit_exec") ) )
        msg << noWait;

    QDBusMessage reply = QDBusConnection::sessionBus().call(msg, QDBus::Block, INT_MAX);
    if ( reply.type() != QDBusMessage::ReplyMessage )
    {
        QDBusReply<QString> replyObj(reply);
        if (replyObj.error().type() == QDBusError::NoReply) {
            printError(i18n("Error launching %1. Either KLauncher is not running anymore, or it failed to start the application.", _name), error);
        } else {
            const QString rpl = reply.arguments().count() > 0 ? reply.arguments().at(0).toString() : reply.errorMessage();
            printError(i18n("KLauncher could not be reached via D-Bus. Error when calling %1:\n%2\n",function, rpl), error);
        }
        //qDebug() << reply;
        return EINVAL;
    }

    if (noWait)
        return 0;

    Q_ASSERT(reply.arguments().count() == 4);
    if (serviceName)
        *serviceName = reply.arguments().at(1).toString();
    if (error)
        *error = reply.arguments().at(2).toString();
    if (pid)
        *pid = reply.arguments().at(3).toInt();
    return reply.arguments().at(0).toInt();
}

#ifndef KDE_NO_DEPRECATED
int
KToolInvocation::startServiceByName( const QString& _name, const QString &URL,
                                     QString *error, QString *serviceName, int *pid,
                                     const QByteArray& startup_id, bool noWait )
{
    if (!isMainThreadActive(error))
        return EINVAL;

    QStringList URLs;
    if (!URL.isEmpty())
        URLs.append(URL);
    return self()->startServiceInternal("start_service_by_name",
                                        _name, URLs, error, serviceName, pid, startup_id, noWait);
}
#endif

#ifndef KDE_NO_DEPRECATED
int
KToolInvocation::startServiceByName( const QString& _name, const QStringList &URLs,
                                     QString *error, QString *serviceName, int *pid,
                                     const QByteArray& startup_id, bool noWait )
{
    if (!isMainThreadActive(error))
        return EINVAL;

    return self()->startServiceInternal("start_service_by_name",
                                _name, URLs, error, serviceName, pid, startup_id, noWait);
}
#endif

int
KToolInvocation::startServiceByDesktopPath( const QString& _name, const QString &URL,
                                            QString *error, QString *serviceName,
                                            int *pid, const QByteArray& startup_id, bool noWait )
{
    if (!isMainThreadActive(error))
        return EINVAL;

    QStringList URLs;
    if (!URL.isEmpty())
        URLs.append(URL);
    return self()->startServiceInternal("start_service_by_desktop_path",
                                _name, URLs, error, serviceName, pid, startup_id, noWait);
}

int
KToolInvocation::startServiceByDesktopPath( const QString& _name, const QStringList &URLs,
                                            QString *error, QString *serviceName, int *pid,
                                            const QByteArray& startup_id, bool noWait )
{
    if (!isMainThreadActive(error))
        return EINVAL;

    return self()->startServiceInternal("start_service_by_desktop_path",
                                _name, URLs, error, serviceName, pid, startup_id, noWait);
}

int
KToolInvocation::startServiceByDesktopName( const QString& _name, const QString &URL,
                                            QString *error, QString *serviceName, int *pid,
                                            const QByteArray& startup_id, bool noWait )
{
    if (!isMainThreadActive(error))
        return EINVAL;

    QStringList URLs;
    if (!URL.isEmpty())
        URLs.append(URL);
    return self()->startServiceInternal("start_service_by_desktop_name",
                                _name, URLs, error, serviceName, pid, startup_id, noWait);
}

int
KToolInvocation::startServiceByDesktopName( const QString& _name, const QStringList &URLs,
                                            QString *error, QString *serviceName, int *pid,
                                            const QByteArray& startup_id, bool noWait )
{
    if (!isMainThreadActive(error))
        return EINVAL;

    return self()->startServiceInternal("start_service_by_desktop_name",
                                _name, URLs, error, serviceName, pid, startup_id, noWait);
}

int
KToolInvocation::kdeinitExec( const QString& name, const QStringList &args,
                              QString *error, int *pid, const QByteArray& startup_id )
{
    if (!isMainThreadActive(error))
        return EINVAL;

    return self()->startServiceInternal("kdeinit_exec",
                                name, args, error, 0, pid, startup_id, false);
}


int
KToolInvocation::kdeinitExecWait( const QString& name, const QStringList &args,
                                  QString *error, int *pid, const QByteArray& startup_id )
{
    if (!isMainThreadActive(error))
        return EINVAL;

    return self()->startServiceInternal("kdeinit_exec_wait",
                                name, args, error, 0, pid, startup_id, false);
}

void KToolInvocation::invokeHelp( const QString& anchor,
                                  const QString& _appname,
                                  const QByteArray& startup_id )
{
    if (!isMainThreadActive())
        return;

    KUrl url;
    QString appname;
    QString docPath;
    if (_appname.isEmpty()) {
        appname = QCoreApplication::instance()->applicationName();
    } else
        appname = _appname;

    KService::Ptr service(KService::serviceByDesktopName(appname));
    if (service) {
        docPath = service->docPath();
    }

    if (!docPath.isEmpty()) {
        url = KUrl(KUrl("help:/"), docPath);
    } else {
        url = QString::fromLatin1("help:/%1/index.html").arg(appname);
    }

    if (!anchor.isEmpty()) {
        url.addQueryItem(QString::fromLatin1("anchor"), anchor);
    }

    // launch a browser for URIs not handled by khelpcenter
    // (following KCMultiDialog::slotHelpClicked())
    if (!(url.protocol() == QLatin1String("help") || url.protocol() == QLatin1String("man") || url.protocol() == QLatin1String("info"))) {
        invokeBrowser(url.url());
        return;
    }

    QDBusInterface *iface = new QDBusInterface(QLatin1String("org.kde.khelpcenter"),
                                               QLatin1String("/KHelpCenter"),
                                               QLatin1String("org.kde.khelpcenter.khelpcenter"),
                                               QDBusConnection::sessionBus());
    if ( !iface->isValid() )
    {
        QString error;
#ifdef Q_WS_WIN
        // startServiceByDesktopName() does not work yet; KRun:processDesktopExec returned 'KRun: syntax error in command "khelpcenter %u" , service "KHelpCenter" '
        if (kdeinitExec(QLatin1String("khelpcenter"), QStringList() << url.url(), &error, 0, startup_id))
#else
        if (startServiceByDesktopName(QLatin1String("khelpcenter"), url.url(), &error, 0, 0, startup_id, false))
#endif
        {
            KMessage::message(KMessage::Error,
                              i18n("Could not launch the KDE Help Center:\n\n%1", error),
                              i18n("Could not Launch Help Center"));
            delete iface;
	    return;
        }

        delete iface;
        iface = new QDBusInterface(QLatin1String("org.kde.khelpcenter"),
                                   QLatin1String("/KHelpCenter"),
                                   QLatin1String("org.kde.khelpcenter.khelpcenter"),
                                   QDBusConnection::sessionBus());
    }

    iface->call(QString::fromLatin1("openUrl"), url.url(), startup_id );
    delete iface;
}

void KToolInvocation::invokeMailer(const QString &address, const QString &subject, const QByteArray& startup_id)
{
    if (!isMainThreadActive())
        return;

    invokeMailer(address, QString(), QString(), subject, QString(), QString(),
                 QStringList(), startup_id );
}

void KToolInvocation::invokeMailer(const KUrl &mailtoURL, const QByteArray& startup_id, bool allowAttachments )
{
    if (!isMainThreadActive())
        return;

    QString address = mailtoURL.path();
    QString subject;
    QString cc;
    QString bcc;
    QString body;

    const QStringList queries = mailtoURL.query().mid(1).split(QLatin1Char('&'));
    const QChar comma = QChar::fromLatin1(',');
    QStringList attachURLs;
    for (QStringList::ConstIterator it = queries.begin(); it != queries.end(); ++it)
    {
        QString q = (*it).toLower();
        if (q.startsWith(QLatin1String("subject=")))
            subject = KUrl::fromPercentEncoding((*it).mid(8).toLatin1());
        else
            if (q.startsWith(QLatin1String("cc=")))
                cc = cc.isEmpty()? KUrl::fromPercentEncoding((*it).mid(3).toLatin1()): cc + comma + KUrl::fromPercentEncoding((*it).mid(3).toLatin1());
            else
                if (q.startsWith(QLatin1String("bcc=")))
                    bcc = bcc.isEmpty()? KUrl::fromPercentEncoding((*it).mid(4).toLatin1()): bcc + comma + KUrl::fromPercentEncoding((*it).mid(4).toLatin1());
                else
                    if (q.startsWith(QLatin1String("body=")))
                        body = KUrl::fromPercentEncoding((*it).mid(5).toLatin1());
                    else
                        if (allowAttachments && q.startsWith(QLatin1String("attach=")))
                            attachURLs.push_back(KUrl::fromPercentEncoding((*it).mid(7).toLatin1()));
                        else
                            if (allowAttachments && q.startsWith(QLatin1String("attachment=")))
                                attachURLs.push_back(KUrl::fromPercentEncoding((*it).mid(11).toLatin1()));
                            else
                                if (q.startsWith(QLatin1String("to=")))
                                    address = address.isEmpty()? KUrl::fromPercentEncoding((*it).mid(3).toLatin1()): address + comma + KUrl::fromPercentEncoding((*it).mid(3).toLatin1());
    }

    invokeMailer( address, cc, bcc, subject, body, QString(), attachURLs, startup_id );
}

void KToolInvocation::startKdeinit()
{
  KComponentData inst( "startkdeinitlock" );
  KLockFile lock( KStandardDirs::locateLocal("tmp", QString::fromLatin1("startkdeinitlock"), inst ));
  if( lock.lock( KLockFile::NoBlockFlag ) != KLockFile::LockOK ) {
     lock.lock();
     if( QDBusConnection::sessionBus().interface()->isServiceRegistered(QString::fromLatin1("org.kde.klauncher")))
         return; // whoever held the lock has already started it
  }
  // Try to launch kdeinit.
  QString srv = KStandardDirs::findExe(QLatin1String("kdeinit4"));
  if (srv.isEmpty())
     return;
//   this is disabled because we are in kdecore
//  const bool gui = qApp && qApp->type() != QApplication::Tty;
//  if ( gui )
//    qApp->setOverrideCursor( Qt::WaitCursor );
  QStringList args;
#ifndef Q_WS_WIN
  args += QString::fromLatin1("--suicide");
#endif
  QProcess::execute(srv, args);
//  if ( gui )
//    qApp->restoreOverrideCursor();
}

#include "ktoolinvocation.moc"
