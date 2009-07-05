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

#ifndef DBUS_HELPER_BACKEND_H
#define DBUS_HELPER_BACKEND_H

#include <QObject>
#include "HelperProxy.h"
#include "ActionReply.h"

class DBusHelperProxy : public QObject, public HelperProxy
{
    Q_OBJECT
    Q_INTERFACES(HelperProxy)
    
    QObject *responder;
    QString m_name;
    
    public:
        DBusHelperProxy() : responder(NULL) {}
        
        QString name() { return m_name; }
        void setName(QString name) { m_name = name; }
        
        virtual ActionReply executeAction(const QString &action, const QString &helperID, const ArgumentsMap &arguments);
        
        virtual bool initHelper(const QString &name);
        
        virtual void setHelperResponder(QObject *o);
        
        virtual void sendDebugMessage(QtMsgType t, const char *msg);
        
    public slots:
        void performActionAsync(const QString &action, QByteArray callerID, QByteArray arguments);
        QByteArray performAction(const QString &action, QByteArray callerID, QByteArray arguments); // this is private
        
    signals:
        void actionPerformed(QByteArray reply);
        void debugMessage(int t, const QString &message);
        
    private slots:
        void debugMessageReceived(int t, QString message);
};

#endif