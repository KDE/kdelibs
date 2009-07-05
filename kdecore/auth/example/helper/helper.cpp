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

#include <QtDebug>

#include "helper.h"

ActionReply MyHelper::read(QVariantMap args)
{
    qDebug() << QString("Action executed by the helper. PID: %1, UID: %2").arg(getpid()).arg(getuid());
    
    QVariantMap::const_iterator i = args.constBegin();
    while (i != args.constEnd()) {
        qDebug() << "Argument key:" << i.key() << "- value:" << i.value();
        ++i;
    }
    
    ActionReply reply;
    
    reply.data()["result"] = "OK";
    
    return reply;
}

KDE4_AUTH_HELPER("org.kde.auth.example", MyHelper)