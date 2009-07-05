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

#include <QDebug>
#include <QPluginLoader>

#include "Action.h"
#include "BackendsManager.h"

static AuthBackend *backend = NULL;
static QString s_helperID;

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
    return execute(helperID());
}

bool Action::executeAsync(QObject *target, const char *slot)
{
    return executeAsync(helperID(), target, slot);
}

// TODO: Check for helper id's syntax
ActionReply Action::execute(const QString &helperID)
{
    return BackendsManager::helperProxy()->executeAction(m_name, helperID, m_args, HelperProxy::Synchronous);
}

bool Action::executeAsync(const QString &helperID, QObject *target, const char *slot)
{
    if(target && slot)
        QObject::connect(BackendsManager::helperProxy(), SIGNAL(actionExecuted(ActionReply)), target, slot);
    
    ActionReply r = BackendsManager::helperProxy()->executeAction(m_name, helperID, m_args, HelperProxy::Asynchronous);
    if(r.failed())
    {
        qCritical("%s", r.errorDescription().toAscii().data());
        
        if(target && slot)
            QObject::disconnect(BackendsManager::helperProxy(), SIGNAL(actionExecuted(ActionReply)), target, slot);
        
        return false;
    }
    
    return true;
}

QString Action::helperID()
{    
    return s_helperID;
}

// TODO: Check for helper id's syntax
void Action::setHelperID(const QString &id)
{
    s_helperID = id;
}

