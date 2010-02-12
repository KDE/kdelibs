/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
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

#include <QPluginLoader>
#include <QDir>

namespace KAuth
{

AuthBackend *BackendsManager::auth = 0;
HelperProxy *BackendsManager::helper = 0;

BackendsManager::BackendsManager()
{
}

QList< QObject* > BackendsManager::retrieveInstancesIn(const QString& path)
{
    QDir pluginPath(path);

    if (!pluginPath.exists()) {
        return QList< QObject* >();
    }

    QFileInfoList entryList = pluginPath.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);

    if (entryList.isEmpty()) {
        return QList< QObject* >();
    }

    QList< QObject* > retlist;

    foreach(const QFileInfo &fi, entryList) {
        QString filePath = fi.filePath(); // file name with path
        QString fileName = fi.fileName(); // just file name

        if(!QLibrary::isLibrary(filePath)) {
            continue;
        }

        QString errstr;
        QPluginLoader loader(filePath);
        QObject *instance = loader.instance();
        if (instance) {
            retlist.append(instance);
        }
    }

    return retlist;
}

void BackendsManager::init()
{
    // Backend plugin
    QList< QObject* > backends = retrieveInstancesIn(KAUTH_BACKEND_PLUGIN_DIR);

    foreach (QObject *instance, backends) {
        auth = qobject_cast< KAuth::AuthBackend* >(instance);
        if (auth) {
            break;
        }
    }

    // Helper plugin
    QList< QObject* > helpers = retrieveInstancesIn(KAUTH_HELPER_PLUGIN_DIR);

    foreach (QObject *instance, helpers) {
        helper = qobject_cast< KAuth::HelperProxy* >(instance);
        if (helper) {
            break;
        }
    }

    Q_ASSERT_X(auth, __FUNCTION__, "No AuthBackend found.");
    Q_ASSERT_X(helper, __FUNCTION__, "No HelperBackend found.");
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
    if (!helper) {
        init();
    }

    return helper;
}

} // namespace Auth
