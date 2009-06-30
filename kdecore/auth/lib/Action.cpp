#include <QDebug>
#include <QPluginLoader>

#include "Action.h"
#include "BackendsManager.h"

static AuthBackend *backend = NULL;

void Action::init()
{
    backend = BackendsManager::authBackend();
    
    backend->setupAction(m_name);
}

bool Action::authorize()
{
    return backend->authorizeAction(m_name);
}

Action::AuthStatus Action::status()
{
    return backend->actionStatus(m_name);
}

ActionReply Action::execute()
{
    if(authorize())
        BackendsManager::helperProxy()->executeAction(m_name, QMap<QString, QVariant>());
}