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

#include <QCoreApplication>
#include <QTimer>
#include <QtDebug>
#include <syslog.h>

#include "HelperProxy.h"
#include "BackendsManager.h"
#include "helper_debug.h"

#include "helper.h"

ActionReply MyHelper::read(ArgumentsMap args)
{
    qDebug() << QString("Action executed by the helper. PID: %1, UID: %2").arg(getpid()).arg(getuid());
    
    ArgumentsMap::const_iterator i = args.constBegin();
    while (i != args.constEnd()) {
        qDebug() << "Argument key:" << i.key() << "- value:" << i.value();
        ++i;
    }
    
    ActionReply reply;
    
    reply.data()["result"] = "OK";
    
    return reply;
}

int main(int argc, char **argv)
{
    init_debug_handler();
    
    qDebug() << "Helper started";
    MyHelper object;
    
    if(!BackendsManager::helperProxy()->initHelper("org.kde.auth.example"))
    {
        qFatal("initHelper() failed");
        return -1;
    }
    
    enable_remote_debug();
    
    BackendsManager::helperProxy()->setHelperResponder(&object);
    
    QCoreApplication app(argc, argv);
    QTimer::singleShot(10000, &app, SLOT(quit()));
    app.exec();
    
    end_debug();
    
    return 0;
}