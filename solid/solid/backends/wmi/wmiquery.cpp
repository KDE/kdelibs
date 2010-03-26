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

#ifdef __GNUC__
//temporarily place this here, it should be moved to kdewin lib
BSTR BSTRFromCStr(UINT codePage, LPCSTR s)
{
    int wideLen = MultiByteToWideChar(codePage, 0, s, -1, NULL, 0);
    if( wideLen > 0 )
    {
        WCHAR* wideStr = (WCHAR*)CoTaskMemAlloc(wideLen*sizeof(WCHAR));
        if( NULL != wideStr )
        {
            BSTR bstr;

            ZeroMemory(wideStr, wideLen*sizeof(WCHAR));
            MultiByteToWideChar(codePage, 0, s, -1, wideStr, wideLen);
            bstr = SysAllocStringLen(wideStr, wideLen-1);
            CoTaskMemFree(wideStr);

            return bstr;
        }
    }
    return NULL;
};

#define _bstr_t(a) BSTRFromCStr(CP_UTF8, a)
#define bstr_t _bstr_t
#endif
using namespace Solid::Backends::Wmi;

/**
 When a WmiQuery instance is created as a static global 
 object a deadlock problem occurs in pLoc->ConnectServer. 
 Please DO NOT USE the following or similar statement in 
 the global space or a class.
 
 static WmiQuery instance; 
*/

QString WmiQuery::Item::getProperty(const QString &property )
{
    qDebug() << "start property:" << property;
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
    HRESULT hr = m_p->Get(bstr_t(qPrintable(prop)), 0, &vtProp, 0, 0);
    QString result;
    if (SUCCEEDED(hr)) {
        result = QString((QChar*)vtProp.bstrVal, wcslen(vtProp.bstrVal));
    }

    VariantClear(&vtProp);
    m_p->Release();
    qDebug() << "end result:" << result;
    return result;
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
        hres = pLoc->ConnectServer( _bstr_t("ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc );                              
        if( FAILED(hres) )
        {
            qCritical() << "Could not connect. Error code = " << hres << endl;
            pLoc->Release();
            if ( m_bNeedUninit )
              CoUninitialize();
            m_failed = true;
        }
        else
            qDebug() << "Connected to ROOT\\CIMV2 WMI namespace" << endl;
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

    hres = pSvc->ExecQuery( bstr_t("WQL"), bstr_t( qPrintable( wql ) ),
                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

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
            
         // TODO: any special thinks required to delete pclsObj ?
            retList.append( new Item(pclsObj) );
        }
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
