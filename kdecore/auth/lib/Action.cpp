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

#include "Action.h"

#include <QDebug>
#include <QPluginLoader>

#include "BackendsManager.h"

class ActionPrivate
{
    public:
        ActionPrivate(QString name) : name(name) {}
        
        QString name;
        QVariantMap args;
};

static QString s_helperID;

// Constructors
Action::Action(const Action &action)
{
    d = new ActionPrivate(action.d->name);
    d->args = action.d->args;
}

Action::Action(const char *name)
{
    d = new ActionPrivate(name);
    BackendsManager::authBackend()->setupAction(d->name);
}

Action::Action(const QString &name)
{
    d = new ActionPrivate(name);
    BackendsManager::authBackend()->setupAction(d->name);
}

Action::~Action()
{
    delete d;
}

// Operators
Action &Action::operator=(const Action &action)
{
    d->name = action.d->name;
    d->args = action.d->args;
    
    return *this;
}

// Accessors
QString Action::name()
{
    return d->name;
}

void Action::setName(QString name)
{
    d->name = name;
}

QVariantMap &Action::arguments()
{
    return d->args;
}

ActionWatcher *Action::watcher()
{
    return ActionWatcher::watcher(d->name);
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

// Authorizaton methods
bool Action::authorize()
{
    if(status() == Authorized)
        return true;
    
    return BackendsManager::authBackend()->authorizeAction(d->name);
}

Action::AuthStatus Action::status()
{
    return BackendsManager::authBackend()->actionStatus(d->name);
}

// Execution methods
bool Action::executeActions(const QList<Action> &actions, QList<Action> *deniedActions, const QString &helperID)
{
    QString _helperID = (helperID == "" ? Action::helperID() : helperID);
    
    QList<QPair<QString, QVariantMap> > list;
    
    foreach(Action a, actions)
    {
        if(a.authorize()) {
            list.push_back(QPair<QString, QVariantMap>(a.name(), a.arguments()));
	} else if(deniedActions) {
            *deniedActions << a;
	}
    }
    
    if(list.isEmpty()) {
        return false;
    }
    
    return BackendsManager::helperProxy()->executeActions(list, _helperID);
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
    
    return executeActions(QList<Action>() << *this, NULL, helperID);
}

ActionReply Action::execute()
{
    return execute(helperID());
}

ActionReply Action::execute(const QString &helperID)
{
    if(!authorize())
        return ActionReply::AuthorizationDeniedReply;
    return BackendsManager::helperProxy()->executeAction(d->name, helperID, d->args);
}

void Action::stop()
{
    stop(helperID());
}

void Action::stop(const QString &helperID)
{
    BackendsManager::helperProxy()->stopAction(d->name, helperID);
}

