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

#ifndef HELPER_BACKEND
#define HELPER_BACKEND

#include <QtPlugin>
#include <QObject>

#include "ActionReply.h"

template<class Key, class T> class QMap;
class QString;
class QVariant;

class HelperProxy : public QObject
{
    Q_OBJECT
    
    public:
        enum ExecMode
        {
            Synchronous,
            Asynchronous
        };
        
        // Application-side methods
        virtual ActionReply executeAction(const QString &action, const QString &helperID, const QVariantMap &arguments, ExecMode mode = Synchronous) = 0;
        
        // Helper-side methods
        virtual bool initHelper(const QString &name) = 0;
        virtual void setHelperResponder(QObject *o) = 0;
        virtual void sendDebugMessage(QtMsgType t, const char *msg) = 0;
        
    signals:
        virtual void actionExecuted(ActionReply reply);
};

Q_DECLARE_INTERFACE(HelperProxy, "org.kde.auth.HelperProxy/0.1");

#endif