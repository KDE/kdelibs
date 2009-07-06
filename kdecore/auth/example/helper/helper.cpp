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
#include <QFile>
#include <QTextStream>
#include <unistd.h>

#include "helper.h"

ActionReply MyHelper::read(QVariantMap args)
{
    ActionReply reply;
    QString filename = args["filename"].toString();
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
    {
        reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(file.error());
        
        return reply;
    }
    
    QTextStream stream(&file);
    QString contents;
    stream >> contents;
    reply.data()["contents"] = contents;
    
    return reply;
}

ActionReply MyHelper::write(QVariantMap args)
{
    QString filename = args["filename"].toString();
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly))
    {
        ActionReply reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(file.error());
        
        return reply;
    }
    
    QTextStream stream(&file);
    stream << args["contents"].toString();
    
    return ActionReply::SuccessReply;
}

ActionReply MyHelper::longaction(QVariantMap args)
{
    for(int i = 1; i <= 100; i++)
    {
        HelperSupport::progressStep(i);
        usleep(250000);
    }
    
    return ActionReply::SuccessReply;
}

KDE4_AUTH_HELPER("org.kde.auth.example", MyHelper)