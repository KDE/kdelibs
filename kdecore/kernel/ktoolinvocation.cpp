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
#include <klocalizedstring.h>

#include <qlockfile.h>
#include <QUrl>
#include <QCoreApplication>
#include <QThread>
#include <qstandardpaths.h>
#include <config-kernel.h> // HAVE_X11

#include <errno.h> // for EINVAL

class KToolInvocationSingleton
{
public:
    KToolInvocation instance;
};

Q_GLOBAL_STATIC(KToolInvocationSingleton, s_self)

KToolInvocation *KToolInvocation::self()
{
    return &s_self()->instance;
}

KToolInvocation::KToolInvocation() : QObject(0), d(0)
{
}

KToolInvocation::~KToolInvocation()
{
}

Q_GLOBAL_STATIC_WITH_ARGS(org::kde::KLauncher, klauncherIface,
                          (QString::fromLatin1("org.kde.klauncher5"), QString::fromLatin1("/KLauncher"), QDBusConnection::sessionBus()))

org::kde::KLauncher *KToolInvocation::klauncher()
{
    KToolInvocation::ensureKdeinitRunning();
    return ::klauncherIface();
}

static void printError(const QString& text, QString* error)
{
    if (error)
        *error = text;
    else
        qWarning() << text;
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
    KToolInvocation::ensureKdeinitRunning();
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.klauncher5"),
                                                      QStringLiteral("/KLauncher"),
                                                      QStringLiteral("org.kde.KLauncher"),
                                                      function);
    msg << _name << URLs;
    if (function == QLatin1String("kdeinit_exec_with_workdir"))
        msg << workdir;
#if HAVE_X11
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

void KToolInvocation::invokeMailer(const QString &address, const QString &subject, const QByteArray& startup_id)
{
    if (!isMainThreadActive())
        return;

    invokeMailer(address, QString(), QString(), subject, QString(), QString(),
                 QStringList(), startup_id );
}

void KToolInvocation::invokeMailer(const QUrl &mailtoURL, const QByteArray& startup_id, bool allowAttachments )
{
    if (!isMainThreadActive())
        return;

    QString address = mailtoURL.path();
    QString subject;
    QString cc;
    QString bcc;
    QString body;

    QList<QPair<QString, QString> > queryItems = QUrlQuery(mailtoURL).queryItems();
    const QChar comma = QChar::fromLatin1(',');
    QStringList attachURLs;
    for (int i = 0; i < queryItems.count(); ++i) {
        const QString q = queryItems.at(i).first.toLower();
        const QString value = queryItems.at(i).second;
        if (q == QLatin1String("subject"))
            subject = value;
        else if (q == QLatin1String("cc"))
            cc = cc.isEmpty() ? value : cc + comma + value;
        else if (q == QLatin1String("bcc"))
            bcc = bcc.isEmpty()? value : bcc + comma + value;
        else if (q == QLatin1String("body"))
            body = value;
        else if (allowAttachments && q == QLatin1String("attach"))
            attachURLs.push_back(value);
        else if (allowAttachments && q == QLatin1String("attachment"))
            attachURLs.push_back(value);
        else if (q == QLatin1String("to"))
            address = address.isEmpty() ? value : address + comma + value;
    }

    invokeMailer( address, cc, bcc, subject, body, QString(), attachURLs, startup_id );
}

void KToolInvocation::ensureKdeinitRunning()
{
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QString::fromLatin1("org.kde.klauncher5"))) {
        return;
    }
    qDebug() << "klauncher not running... launching kdeinit";

  QLockFile lock(QDir::tempPath() + QLatin1Char('/') + QLatin1String("startkdeinitlock"));
  if (!lock.tryLock()) {
     lock.lock();
     if( QDBusConnection::sessionBus().interface()->isServiceRegistered(QString::fromLatin1("org.kde.klauncher5")))
         return; // whoever held the lock has already started it
  }
  // Try to launch kdeinit.
  QString srv = QStandardPaths::findExecutable(QLatin1String("kdeinit5"));
  if (srv.isEmpty())
     return;
//   this is disabled because we are in kdecore
//  const bool gui = qApp && qApp->type() != QApplication::Tty;
//  if ( gui )
//    qApp->setOverrideCursor( Qt::WaitCursor );
  QStringList args;
#ifndef Q_OS_WIN
  args += QString::fromLatin1("--suicide");
#endif
  QProcess::execute(srv, args);
//  if ( gui )
//    qApp->restoreOverrideCursor();
}

#include "ktoolinvocation.moc"
