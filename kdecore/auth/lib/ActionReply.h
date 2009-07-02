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
    
    /*
    * FIXME: Is this needed? What should we return as an error code?
    */
    enum Error
    {
        NoError = 0,
        Error2
    };
    
    ActionReply();
    ActionReply(Type type);
    ActionReply(int errorCode);
    
    QMap<QString, QVariant> &data();
    QMap<QString, QVariant> data() const;
    Type type() const;
    
    bool succeded();
    bool failed();
    
    int errorCode() const;
    void setErrorCode(int errorCode);
    
    friend QDataStream &operator<<(QDataStream &, const ActionReply &);
    friend QDataStream &operator>>(QDataStream &, ActionReply &);
    
private:
    QMap<QString, QVariant> m_data; // User-defined data for success and helper error replies, empty for kauth errors
    int m_errorCode;
    Type m_type;
};

#endif