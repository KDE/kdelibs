#include <QPluginLoader>

#include "BackendsManager.h"

AuthBackend *BackendsManager::auth = NULL;
HelperProxy *BackendsManager::helper = NULL;

Q_IMPORT_PLUGIN(auth_backend);
Q_IMPORT_PLUGIN(helper_proxy);

void BackendsManager::init()
{
    QObjectList l = QPluginLoader::staticInstances();
    foreach(QObject *o, l)
    {
        AuthBackend *a = qobject_cast<AuthBackend *>(o);
        if(a)
            auth = a;
        HelperProxy *h = qobject_cast<HelperProxy *>(o);
        if(h)
            helper = h;
    }
    
    Q_ASSERT_X(auth, __FUNCTION__, "No AuthBackend found.");
    Q_ASSERT_X(helper, __FUNCTION__, "No HelperBackend found.");
}

AuthBackend *BackendsManager::authBackend()
{
    if(!auth)
        init();
    
    return auth;
}

HelperProxy *BackendsManager::helperProxy()
{
    if(!helper)
        init();
    
    return helper;
}