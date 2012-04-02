/*
*   Copyright (C) 2012 Dario Freddi <drf@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation; either version 2.1 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
*/

#include "BackendsManager.h"

#include "BackendsConfig.h"

// Include backends directly
#include "TestBackend.h"
#include "backends/dbus/DBusHelperProxy.h"

#include <QPluginLoader>
#include <QDir>
#include <QDebug>
#include <QThread>

namespace KAuth
{

AuthBackend *BackendsManager::auth = 0;
QHash< QThread*, HelperProxy* > proxiesForThreads = QHash< QThread*, HelperProxy* >();

BackendsManager::BackendsManager()
{
}

void BackendsManager::init()
{
    if (!auth) {
        // Load the test backend
        auth = new TestBackend;
    }

    if (!proxiesForThreads.contains(QThread::currentThread())) {
        // Load the test helper backend
        proxiesForThreads.insert(QThread::currentThread(), new DBusHelperProxy(QDBusConnection::sessionBus()));
    }
}

AuthBackend *BackendsManager::authBackend()
{
    if (!auth) {
        init();
    }

    return auth;
}

HelperProxy *BackendsManager::helperProxy()
{
    if (!proxiesForThreads.contains(QThread::currentThread())) {
        qDebug() << "Creating new proxy for thread" << QThread::currentThread();
        init();
    }

    return proxiesForThreads[QThread::currentThread()];
}

void BackendsManager::setProxyForThread(QThread *thread, HelperProxy *proxy)
{
    qDebug() << "Adding proxy for thread" << thread;

    proxiesForThreads.insert(thread, proxy);
}

} // namespace Auth
