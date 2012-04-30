/*
    Copyright 2012 Patrick von Reth <vonreth@kde.org>
    Copyright 2008 Jeff Mitchell <mitchell@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

//#define _WIN32_DCOM
//#include <comdef.h>

#define INSIDE_WMIQUERY
#include "wmiquery.h"

#ifdef _DEBUG
# pragma comment(lib, "comsuppwd.lib")
#else
# pragma comment(lib, "comsuppw.lib")
#endif
# pragma comment(lib, "wbemuuid.lib")

#include <iostream>
#include <Wbemidl.h>

# pragma comment(lib, "wbemuuid.lib")

#include <QtCore/QDebug>
#include <QtCore/QVariant>
#include <QtCore/QList>
#include <QtCore/QStringList>

//needed for mingw
inline OLECHAR* SysAllocString(const QString &s){
  const OLECHAR *olename = reinterpret_cast<const OLECHAR *>(s.utf16());
  return ::SysAllocString(olename);
}

using namespace Solid::Backends::Wmi;

//from http://qt.gitorious.org/qt-mobility/qt-mobility/blobs/master/src/systeminfo/windows/qwmihelper_win.cpp
QVariant WmiQuery::Item::msVariantToQVariant(VARIANT msVariant, CIMTYPE variantType)
{
    QVariant returnVariant;
    switch(variantType) {
    case CIM_STRING:
    case CIM_CHAR16:
        {
            QString str((QChar*)msVariant.bstrVal, wcslen(msVariant.bstrVal));
            QVariant vs(str);
            returnVariant = vs;
        }
        break;
    case CIM_BOOLEAN:
        {
            QVariant vb(msVariant.boolVal);
            returnVariant = vb;
        }
        break;
            case CIM_UINT8:
        {
            QVariant vb(msVariant.uintVal);
            returnVariant = vb;
        }
        break;
            case CIM_UINT16:
        {
            QVariant vb(msVariant.uintVal);
            returnVariant = vb;
        }
            case CIM_UINT32:
        {
            QVariant vb(msVariant.uintVal);
            returnVariant = vb;
        }
        break;
    };
    VariantClear(&msVariant);
    return returnVariant;
}

/**
 When a WmiQuery instance is created as a static global
 object a deadlock problem occurs in pLoc->ConnectServer.
 Please DO NOT USE the following or similar statement in
 the global space or a class.

 static WmiQuery instance;
*/

QVariant WmiQuery::Item::getProperty(const QString &property) const
{
//    qDebug() << "start property:" << property;
    QString prop = property;
    if (property == "voule.ignore")
        return "false";
    else if(property == "block.storage_device")
        return "true";
    else if (property == "volume.mount_point")
        prop = "deviceid";
    else if (property == "volume.is_mounted")
        return "true";
    // todo check first if property is available
    VARIANT vtProp;
    CIMTYPE variantType;
    BSTR bstrProp;
    bstrProp = ::SysAllocString(prop);
    HRESULT hr = m_p->Get(bstrProp, 0, &vtProp, &variantType, 0);
    QVariant result;
    if (SUCCEEDED(hr)) {
        result = msVariantToQVariant(vtProp,variantType);
    }else{
        qWarning()<<"Querying "<<property<<"failed";
    }
    ::SysFreeString(bstrProp);
//    qDebug() << "end result:" << result;
    return result;
}

WmiQuery::Item::Item(IWbemClassObject *p) : m_p(p), m_int(new QAtomicInt)
{
    m_int->ref();
}

WmiQuery::Item::Item(const Item& other) : m_p(other.m_p), m_int(other.m_int)
{
    m_int->ref();
}

WmiQuery::Item& WmiQuery::Item::operator=(const Item& other)
{
    m_p = other.m_p;
    m_int = other.m_int;
    m_int->ref();
    return *this;
}

WmiQuery::Item::~Item()
{
    if(!m_int->deref()) {
        m_p->Release();
    }
}

WmiQuery::WmiQuery()
    : m_failed(false)
    , pLoc(0)
    , pSvc(0)
    , pEnumerator(NULL)
{

    //does this all look hacky?  yes...but it came straight from the MSDN example...

    HRESULT hres;

    hres =  CoInitialize(0);
    if( FAILED(hres) && hres != S_FALSE && hres != RPC_E_CHANGED_MODE )
    {
        qCritical() << "Failed to initialize COM library.  Error code = 0x" << hex << quint32(hres) << endl;
        m_failed = true;
    }
    m_bNeedUninit = ( hres != S_FALSE && hres != RPC_E_CHANGED_MODE );
    if( !m_failed )
    {
        hres =  CoInitializeSecurity( NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
                    RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL );

        // RPC_E_TOO_LATE --> security already initialized
        if( FAILED(hres) && hres != RPC_E_TOO_LATE )
        {
            qCritical() << "Failed to initialize security. " << "Error code = " << hres << endl;
            if ( m_bNeedUninit )
              CoUninitialize();
            m_failed = true;
        }
    }
    if( !m_failed )
    {
        hres = CoCreateInstance( CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *) &pLoc );
        if (FAILED(hres))
        {
            qCritical() << "Failed to create IWbemLocator object. " << "Error code = " << hres << endl;
            if ( m_bNeedUninit )
              CoUninitialize();
            m_failed = true;
        }
    }
    if( !m_failed )
    {
        hres = pLoc->ConnectServer( L"ROOT\\CIMV2", NULL, NULL, 0, NULL, 0, 0, &pSvc );
        if( FAILED(hres) )
        {
            qCritical() << "Could not connect. Error code = " << hres << endl;
            pLoc->Release();
            if ( m_bNeedUninit )
              CoUninitialize();
            m_failed = true;
        }
//        else
//            qDebug() << "Connected to ROOT\\CIMV2 WMI namespace" << endl;
    }

    if( !m_failed )
    {
        hres = CoSetProxyBlanket( pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                    RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );
        if( FAILED(hres) )
        {
            qCritical() << "Could not set proxy blanket. Error code = " << hres << endl;
            pSvc->Release();
            pLoc->Release();
            if ( m_bNeedUninit )
              CoUninitialize();
            m_failed = true;
        }
    }
}

WmiQuery::~WmiQuery()
{
    if( m_failed )
      return; // already cleaned up properly
/* This does crash because someone else already called
   CoUninitialize()... :(
    if( pSvc )
      pSvc->Release();
    if( pLoc )
      pLoc->Release();
    if( m_bNeedUninit )
      CoUninitialize();
*/
}

WmiQuery::ItemList WmiQuery::sendQuery( const QString &wql )
{
    ItemList retList;

    if (!pSvc)
    {
        m_failed = true;
        return retList;
    }

    HRESULT hres;

    BSTR bstrQuery;
    bstrQuery = ::SysAllocString(wql);
    hres = pSvc->ExecQuery( L"WQL",bstrQuery ,
                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
    ::SysFreeString(bstrQuery);
    
    if( FAILED(hres) )
    {
        qDebug() << "Query with string \"" << wql << "\" failed. Error code = " << hres << endl;
    }
    else
    {
        ULONG uReturn = 0;

        while( pEnumerator )
        {
            IWbemClassObject *pclsObj;
            hres = pEnumerator->Next( WBEM_INFINITE, 1, &pclsObj, &uReturn );

            if( !uReturn )
                break;

         // pclsObj will be released on destruction of Item
            retList.append( Item( pclsObj ) );
        }
        if( pEnumerator ) pEnumerator->Release();
        else qDebug() << "failed to release enumerator!";
    }
    return retList;
}

bool WmiQuery::isLegit() const
{
    return !m_failed;
}

WmiQuery &WmiQuery::instance()
{
	static WmiQuery *query = 0;
	if (!query)
		query = new WmiQuery;
	return *query;
}
