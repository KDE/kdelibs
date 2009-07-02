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

#include "ActionReply.h"

ActionReply::ActionReply() : m_errorCode(0), m_type(Success) {}
ActionReply::ActionReply(ActionReply::Type type) : m_errorCode(0), m_type(type) {}
ActionReply::ActionReply(int error) : m_errorCode(error), m_type(KAuthError) {}

QMap<QString, QVariant> &ActionReply::data()
{
    return m_data;
}

QMap<QString, QVariant> ActionReply::data() const
{
    return m_data;
}

ActionReply::Type ActionReply::type() const
{
    return m_type;
}

bool ActionReply::succeded()
{
    return m_type == Success;
}

bool ActionReply::failed()
{
    return m_type != Success;
}

int ActionReply::errorCode() const
{
    switch(m_type)
    {
        case KAuthError:
            return m_errorCode;
        case HelperError:
            if(m_data.contains("errorCode"))
                return m_data.value("errorCode").toInt();
            else
                return 0;
        case Success:
            return 0;
    }
}

void ActionReply::setErrorCode(int errorCode)
{
    m_errorCode = errorCode;
    m_type = KAuthError;
}

QDataStream &operator<<(QDataStream &d, const ActionReply &reply)
{
    return d << reply.m_data << reply.m_errorCode << (quint32)reply.m_type;
}

QDataStream &operator>>(QDataStream &d, ActionReply &reply)
{
    quint32 i;
    d >> i >> reply.m_errorCode >> reply.m_data;
    reply.m_type = (ActionReply::Type) i;
    
    return d;
}
