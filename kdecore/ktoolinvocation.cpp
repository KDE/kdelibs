/* This file is part of the KDE libraries
    Copyright (C) 2005 Brad Hards
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
#include "kstaticdeleter.h"

#include <QMutex>
#include <QMutexLocker>
#include <QApplication>
#include <klocale.h>

#include <errno.h>
#if defined Q_WS_X11
#include <stdlib.h> // getenv()
#include <QtGui/qx11info_x11.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <fixx11h.h>
#endif

#include <QThread>

Q_GLOBAL_STATIC_WITH_ARGS(QMutex,mutex,(QMutex::Recursive))

KToolInvocation* KToolInvocation::s_self = 0L;

KToolInvocation::KToolInvocation() : QObject(0)
{
}

KStaticDeleter<KToolInvocation> ktoolinvocation_sd;
KToolInvocation *KToolInvocation::self()
{
    QMutexLocker locker(mutex());
    if (s_self==0) {
        ktoolinvocation_sd.setObject( s_self, new KToolInvocation );
    }
    return s_self;
}

KToolInvocation::~KToolInvocation()
{
    QMutexLocker locker(mutex());
    s_self=0;
}

Q_GLOBAL_STATIC_WITH_ARGS(org::kde::KLauncher, klauncherIface,
                          ("org.kde.klauncher", "/KLauncher", QDBusConnection::sessionBus()))

org::kde::KLauncher *KToolInvocation::klauncher()
{
    if ( !QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.kde.klauncher" ) )
    {
        kDebug() << "klauncher not running... launching kdeinit" << endl;
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
    if (qApp && qApp->thread() != QThread::currentThread())
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
    QStringList envs;
#ifdef Q_WS_X11
    if (QX11Info::display()) {
        QByteArray dpystring(XDisplayString(QX11Info::display()));
        envs << QString::fromLatin1( QByteArray("DISPLAY=") + dpystring );
    } else if( getenv( "DISPLAY" )) {
        QByteArray dpystring( getenv( "DISPLAY" ));
        envs << QString::fromLatin1( QByteArray("DISPLAY=") + dpystring );
    }
#endif
    msg << envs;
#if defined Q_WS_X11
    // make sure there is id, so that user timestamp exists
    QByteArray s = startup_id;
    if (s.isEmpty()) {
        emit needNewStartupId(s); // allows KApplication to set the startup id
    }
    msg << QString(s);
#else
    msg << QString();
#endif
    if( !function.startsWith( QLatin1String("kdeinit_exec") ) )
        msg << noWait;

    QDBusMessage reply = QDBusConnection::sessionBus().call(msg);
    if ( reply.type() != QDBusMessage::ReplyMessage )
    {

        printError(i18n("KLauncher could not be reached via D-Bus, error when calling %1:\n%2\n",function, reply.arguments().at(0).toString()), error);
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
