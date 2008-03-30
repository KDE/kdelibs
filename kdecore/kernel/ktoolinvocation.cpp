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
#include <QMutex>
#include <QMutexLocker>
#include <QCoreApplication>
#include <klocale.h>

#include <errno.h>

#include <QThread>

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
                          ("org.kde.klauncher", "/KLauncher", QDBusConnection::sessionBus()))

org::kde::KLauncher *KToolInvocation::klauncher()
{
    if ( !QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.kde.klauncher" ) )
    {
        kDebug() << "klauncher not running... launching kdeinit";
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
                                          const QByteArray& startup_id, bool noWait )
{
    QString function = QLatin1String(_function);
    org::kde::KLauncher *launcher = KToolInvocation::klauncher();
    QDBusMessage msg = QDBusMessage::createMethodCall(launcher->service(),
                                                launcher->path(),
                                                launcher->interface(),
                                                function);
    msg << _name << URLs;
#ifdef Q_WS_X11
    // make sure there is id, so that user timestamp exists
    QStringList envs;
    QByteArray s = startup_id;
    emit kapplication_hook(envs, s);
    msg << envs;
    msg << QString(s);
#else
    msg << QStringList();
    msg << QString();
#endif
    if( !function.startsWith( QLatin1String("kdeinit_exec") ) )
        msg << noWait;

    QDBusMessage reply = QDBusConnection::sessionBus().call(msg);
    if ( reply.type() != QDBusMessage::ReplyMessage )
    {
        QString rpl = reply.arguments().count() > 0 ? reply.arguments().at(0).toString() : QLatin1String("empty");
        printError(i18n("KLauncher could not be reached via D-Bus. Error when calling %1:\n%2\n",function, rpl), error);
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


#include "ktoolinvocation.moc"
