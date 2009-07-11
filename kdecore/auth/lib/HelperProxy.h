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
#include "ActionWatcher.h"

template<class Key, class T> class QMap;
class QString;
class QVariant;

class HelperProxy : public QObject
{
    Q_OBJECT
    
    public:      
        // Application-side methods
        virtual bool executeActions(const QList<QPair<QString, QVariantMap> > &list, const QString &helperID) = 0;
        virtual ActionReply executeAction(const QString &action, const QString &helperID, const QVariantMap &arguments) = 0;
        virtual void stopAction(const QString &action, const QString &helperID) = 0;
        
        // Helper-side methods
        virtual bool initHelper(const QString &name) = 0;
        virtual void setHelperResponder(QObject *o) = 0;
        virtual bool hasToStopAction() = 0;
        virtual void sendDebugMessage(int level, const char *msg) = 0;
        virtual void sendProgressStep(int step) = 0;
        virtual void sendProgressStep(QVariantMap step) = 0;
};

Q_DECLARE_INTERFACE(HelperProxy, "org.kde.auth.HelperProxy/0.1");

#endif
