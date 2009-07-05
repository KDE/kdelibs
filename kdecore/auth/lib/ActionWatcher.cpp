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

#include "ActionWatcher.h"
#include "BackendsManager.h"

#include <QHash>

static QHash<QString, ActionWatcher *> watchers;

ActionWatcher::ActionWatcher(const QString &action) : QObject(NULL), m_action(action) {}

ActionWatcher *ActionWatcher::watcher(const QString &action)
{
    if(!watchers.contains(action))
        watchers[action] = new ActionWatcher(action);
    
    return watchers[action];
}

QString ActionWatcher::action()
{
    return m_action;
}

void ActionWatcher::emitActionPerformed(ActionReply reply)
{
    emit actionPerformed(reply);
}

void ActionWatcher::emitProgressStep(int progress)
{
    emit progressStep(progress);
}

void ActionWatcher::emitProgressStep(QVariantMap data)
{
    emit progressStep(data);
}

