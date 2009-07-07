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
    if(status() == Authorized)
        return true;
    
    return backend->authorizeAction(m_name);
}

Action::AuthStatus Action::status()
{
    return backend->actionStatus(m_name);
}

bool Action::executeActions(const QList<Action> &actions, QList<Action> *deniedActions)
{
    return executeActions(actions, helperID(), deniedActions);
}

bool Action::executeActions(const QList<Action> &actions, const QString &helperID, QList<Action> *deniedActions)
{
    QList<QPair<QString, QVariantMap> > list;
    
    foreach(Action a, actions)
    {
        if(a.authorize())
            list.push_back(QPair<QString, QVariantMap>(a.name(), a.arguments()));
        else if(deniedActions)
            *deniedActions << a;
    }
    
    if(list.isEmpty())
        return false;
    
    return BackendsManager::helperProxy()->executeActions(list, helperID);
}

bool Action::executeAsync(QObject *target, const char *slot)
{
    return executeAsync(helperID(), target, slot);
}

// TODO: Deve restituire false se non è autorizzata?
bool Action::executeAsync(const QString &helperID, QObject *target, const char *slot)
{
    if(!authorize())
        return false;
    
    if(target && slot)
        QObject::connect(watcher(), SIGNAL(actionPerformed(ActionReply)), target, slot);
    
    return executeActions(QList<Action>() << *this, helperID);
}

ActionReply Action::execute()
{
    return execute(helperID());
}

ActionReply Action::execute(const QString &helperID)
{
    if(!authorize())
        return ActionReply::AuthorizationDeniedReply;
    return BackendsManager::helperProxy()->executeAction(m_name, helperID, m_args);
}

void Action::stop()
{
    stop(helperID());
}

void Action::stop(const QString &helperID)
{
    BackendsManager::helperProxy()->stopAction(m_name, helperID);
}

ActionWatcher *Action::watcher()
{
    return ActionWatcher::watcher(m_name);
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

