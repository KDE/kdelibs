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

#include "wmiquery.h"

#include <QtCore/QDebug>
#include <QtCore/QVariant>
#include <QtCore/QList>
#include <QtCore/QStringList>

#ifdef _DEBUG
# pragma comment(lib, "comsuppwd.lib")
#else
# pragma comment(lib, "comsuppw.lib")
#endif
# pragma comment(lib, "wbemuuid.lib")

#define _WIN32_DCOM
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>

# pragma comment(lib, "wbemuuid.lib")

using namespace Solid::Backends::Wmi;

QString WmiQuery::Item::getProperty(const QString &property )
{
    qDebug() << "start property:" << property;
    // todo check first if property is available
    VARIANT vtProp;
    HRESULT hr = m_p->Get((LPCWSTR)property.utf16(), 0, &vtProp, 0, 0);
    VariantClear(&vtProp);
    QString result((QChar*)vtProp.bstrVal, wcslen(vtProp.bstrVal));
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

    hres =  CoInitializeEx( 0, COINIT_MULTITHREADED ); 
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
        hres = pLoc->ConnectServer( _bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc );                              
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
*/
    if( m_bNeedUninit )
      CoUninitialize();
}  
    
WmiQuery::ItemList WmiQuery::sendQuery( const QString &wql )
{
    ItemList retList;
    
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
