/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
*/

#include "BackendsManager.h"

#include <QPluginLoader>

AuthBackend *BackendsManager::auth = NULL;
HelperProxy *BackendsManager::helper = NULL;

Q_IMPORT_PLUGIN(auth_backend);
Q_IMPORT_PLUGIN(helper_proxy);

void BackendsManager::init()
{
    QObjectList l = QPluginLoader::staticInstances();
    foreach(QObject *o, l) {
        AuthBackend *a = qobject_cast<AuthBackend *>(o);
        if (a)
            auth = a;
        HelperProxy *h = qobject_cast<HelperProxy *>(o);
        if (h)
            helper = h;
    }

    Q_ASSERT_X(auth, __FUNCTION__, "No AuthBackend found.");
    Q_ASSERT_X(helper, __FUNCTION__, "No HelperBackend found.");
}

AuthBackend *BackendsManager::authBackend()
{
    if (!auth)
        init();

    return auth;
}

HelperProxy *BackendsManager::helperProxy()
{
    if (!helper)
        init();

    return helper;
}
