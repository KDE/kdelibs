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

#ifndef ACTION_REPLY_H
#define ACTION_REPLY_H

#include <QString>
#include <QVariant>
#include <QMap>
#include <QDataStream>

class ActionReply
{
public:
    enum Type
    {
        KAuthError,
        HelperError,
        Success
    };
    
    static ActionReply SuccessReply;
    static ActionReply HelperErrorReply;
    static ActionReply NoResponderReply;
    static ActionReply NoSuchActionReply;
    static ActionReply AuthorizationDeniedReply;
    static ActionReply DBusErrorReply;
    static ActionReply WrongReplyDataReply;

    enum Error
    {
        NoError = 0,
        NoResponder,
        NoSuchAction,
        AuthorizationDenied,
        DBusError,
        WrongReplyData
    };
    
    ActionReply();
    ActionReply(Type type);
    ActionReply(int errorCode);
    ActionReply(QByteArray data);
    
    QVariantMap &data();
    QVariantMap data() const;
    Type type() const;
    void setType(Type type);
    
    bool succeded();
    bool failed();
    
    int errorCode() const;
    void setErrorCode(int errorCode);
    QString errorDescription();
    void setErrorDescription(const QString &error);
    
    QByteArray serialized();
    
    friend QDataStream &operator<<(QDataStream &, const ActionReply &);
    friend QDataStream &operator>>(QDataStream &, ActionReply &);
    
private:
    QVariantMap m_data; // User-defined data for success and helper error replies, empty for kauth errors
    int m_errorCode;
    QString m_errorDescription;
    Type m_type;
};

#endif