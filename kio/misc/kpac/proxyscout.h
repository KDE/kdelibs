/* 
   Copyright (c) 2003 Malte Starostik <malte@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#ifndef KPAC_PROXYSCOUT_H
#define KPAC_PROXYSCOUT_H

#include <q3valuelist.h>
#include <qmap.h>

#include <kdedmodule.h>
#include <kurl.h>

#include <time.h>

class DCOPClientTransaction;
class KInstance;

namespace KPAC
{
    class Downloader;
    class Script;

    class ProxyScout : public KDEDModule
    {
        Q_OBJECT
        K_DCOP
    public:
        ProxyScout( const QByteArray& );
        virtual ~ProxyScout();

    k_dcop:
        QString proxyForURL( const KURL& url );
        ASYNC blackListProxy( const QString& proxy );
        ASYNC reset();

    private slots:
        void downloadResult( bool );

    private:
        bool startDownload();
        QString handleRequest( const KURL& url );

        KInstance* m_instance;
        Downloader* m_downloader;
        Script* m_script;

        struct QueuedRequest
        {
            QueuedRequest() : transaction( 0 ) {}
            QueuedRequest( const KURL& );

            DCOPClientTransaction* transaction;
            KURL url;
        };
        typedef Q3ValueList< QueuedRequest > RequestQueue;
        RequestQueue m_requestQueue;

        typedef QMap< QString, time_t > BlackList;
        BlackList m_blackList;
        time_t m_suspendTime;
    };
}

#endif // KPAC_PROXYSCOUT_H

// vim: ts=4 sw=4 et
