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

// Here comes all the logic for compiling the chosen backends
#ifdef KAUTH_COMPILING_OSX_BACKEND
#include "backends/mac/AuthServicesBackend.h"
typedef KAuth::AuthServicesBackend KAuthAuthBackend;
#endif
#ifdef KAUTH_COMPILING_POLKITQT_BACKEND
#include "backends/policykit/PolicyKitBackend.h"
typedef KAuth::PolicyKitBackend KAuthAuthBackend;
#endif
#ifdef KAUTH_COMPILING_POLKITQT1_BACKEND
#include "backends/polkit-1/Polkit1Backend.h"
typedef KAuth::Polkit1Backend KAuthAuthBackend;
#endif
#ifdef KAUTH_COMPILING_FAKE_BACKEND
#include "backends/fake/FakeBackend.h"
typedef KAuth::FakeBackend KAuthAuthBackend;
#endif

// Helper backends
#ifdef KAUTH_COMPILING_DBUS_HELPER_BACKEND
#include "backends/dbus/DBusHelperProxy.h"
typedef KAuth::DBusHelperProxy KAuthHelperBackend;
#endif

namespace KAuth
{

AuthBackend *BackendsManager::auth = 0;
HelperProxy *BackendsManager::helper = 0;

BackendsManager::BackendsManager()
{
}

void BackendsManager::init()
{
    // Beware: here comes all the logic for loading the correct backend
    auth = new KAuthAuthBackend;
    helper = new KAuthHelperBackend;

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
