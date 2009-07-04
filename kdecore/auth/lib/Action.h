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

#ifndef ACTION_H
#define ACTION_H

#include <QString>
#include <QVariant>
#include <QHash>

#include "ActionReply.h"

class Action
{    
    QString m_name;
    QMap<QString, QVariant> m_args;
    
    public:
        enum AuthStatus
        {
            Denied = 0,
            Authorized = 1,
            AuthRequired = 2
        };
        
        Action(const char *name) : m_name(name) { init(); }
        Action(const QString &name) : m_name(name) { init(); }
        
        QString name() { return m_name; }
        void setName(QString name) { m_name = name; }
        
        QMap<QString, QVariant> &arguments() { return m_args; }
        
        bool authorize();
        AuthStatus status();
        ActionReply execute();
        ActionReply execute(const QString &helperID);
        
        static QString helperID();
        static void setHelperID(const QString &id);
        
    private:
        void init();
};

#endif
