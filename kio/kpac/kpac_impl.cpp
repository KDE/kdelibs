/*
 *  $Id$
 *  Proxy Auto Configuration
 *  
 *  Copyright (C) 2000 Malte Starostik <malte.starostik@t-online.de>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <time.h>

#include <qfile.h>

#include <kinstance.h>
#include <klibloader.h>
#include <kurl.h>
#include <kjs/object.h>
#include <kjs/types.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>

#include "kproxybindings.h"
#include "kpac_impl.h"

using namespace KJS;

KPACImpl::KPACImpl()
    : m_kjs(0),
      m_configRead(false)
{
}

KPACImpl::~KPACImpl()
{
    delete m_kjs;
}

QString KPACImpl::proxyForURL(const KURL &url)
{
    if (!m_configRead)
        return QString::null;

    QString code = QString("FindProxyForURL('%1', '%2');").arg(url.url()).arg(url.host());
    if (!m_kjs->evaluate(code.local8Bit()))
        return QString::null;
    
    KJS::Imp *retval = m_kjs->returnValue();
    if (retval)
    {
        QStringList proxies = QStringList::split(';', retval->toString().value().qstring());
        if (!proxies.count())
            return QString::null;
        KSimpleConfig blackList(locate("tmp", "badproxies"));
        for (QStringList::ConstIterator it = proxies.begin(); it != proxies.end(); ++it)
        {
            QString proxy = (*it).simplifyWhiteSpace();
            if (proxy.left(5) == "PROXY")
            {
                int p = proxy.find(':');
                if (p >= 0)
                {
                    proxy = proxy.mid(p + 1).stripWhiteSpace();
                    if (proxy.left(7) != "http://")
                        proxy = "http://" + proxy;
                    time_t badMark = blackList.readNumEntry(proxy);
                    if (badMark < time(0) - 1800)
                    {
                        if (badMark)
                            blackList.deleteEntry(proxy, false);
                        return proxy;
                    }
                }
            }
            else if (proxy == "DIRECT")
                return proxy;
        }
    }
    return QString::null;
}

bool KPACImpl::init()
{
    if (m_configRead)
    {
        m_kjs->clear();
        m_configRead = false;
    }

    QString fileName = locateLocal("data", "kio_http/proxy.pac");
    if (fileName.isEmpty())
        return false;
        
    if (!m_kjs)
    {
        m_kjs = new KJScript();
        Global global(Global::current());
        KJSO bindings(new KProxyBindings);
        global.put("ProxyConfig", bindings);
        global.setPrototype(bindings);
    }
    QFile f(fileName);
    if (!f.open(IO_ReadOnly))
        return false;
    char *code = (char *)malloc(f.size() + 1);
    f.readBlock(code, f.size());
    code[f.size()] = 0;
    if (!(m_configRead = m_kjs->evaluate(code)))
        m_kjs->clear();
        
    free(code);
    f.close();
    return m_configRead;
}

void KPACImpl::badProxy(const QString &proxy)
{
    KSimpleConfig blackList(locateLocal("tmp", "badproxies"));
    blackList.writeEntry(proxy, time(0));
}

extern "C"
{
    void *create_pac()
    {
        return new KPACImpl;
    }
};

