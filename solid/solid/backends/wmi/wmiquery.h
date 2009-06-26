/*  This file is part of the KDE project
    Copyright (C) 2008 Jeff Mitchell <mitchell@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef SOLID_BACKENDS_WMI_WMIQUERY_H
#define SOLID_BACKENDS_WMI_WMIQUERY_H

#include <QtCore/QDebug>
#include <QtCore/QVariant>
#include <QtCore/QList>

#include <solid/solid_export.h>


#ifdef INSIDE_WMIQUERY
#include <windows.h>
#include <rpc.h>
#include <comdef.h>
#include <Wbemidl.h>
#else
typedef void *IWbemClassObject;
typedef void *IWbemLocator;
typedef void *IWbemServices;
typedef void *IEnumWbemClassObject;
#endif

namespace Solid
{
namespace Backends
{
namespace Wmi
{

class WmiQuery
{
public:
    class Item {
    public:
        Item(IWbemClassObject *p) : m_p(p) {}
        ~Item() {} // how to delete the pointer ?
        QString getProperty(const QString &property );

    private: 
        IWbemClassObject * m_p;     
    };

    typedef QList<Item*> ItemList;

    WmiQuery();
    ~WmiQuery();
    ItemList sendQuery( const QString &wql );
    bool isLegit() const;
	static WmiQuery &instance();
    
private:
    bool m_failed;
    bool m_bNeedUninit;
    IWbemLocator *pLoc;
    IWbemServices *pSvc;
    IEnumWbemClassObject* pEnumerator;
};
}
}
}

#endif
