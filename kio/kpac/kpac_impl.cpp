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

#include <kapp.h>
#include <kdebug.h>
#include <kinstance.h>
#include <klibloader.h>
#include <kurl.h>
#include <kjs/object.h>
#include <kjs/types.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>
#include <kio/job.h>

#include "kproxybindings.h"
#include "kpac_impl.moc"

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
    kdDebug(7025) << "KPACImpl::proxyForURL(), url=" << url.url() << endl;
    if (!m_configRead)
    {
        kdDebug(7025) << "KPACImpl::proxyForURL(): config not read, not using a proxy" << endl;
        return QString::null;
    }

    QString code = QString("return FindProxyForURL('%1', '%2');").arg(url.url()).arg(url.host());
    if (!m_kjs->evaluate(code.local8Bit()))
    {
        kdDebug(7025) << "KPACImpl::proxyForURL(): JS evaluation error, not using a proxy" << endl;
        return QString::null;
    }
    
    KJS::Imp *retval = m_kjs->returnValue();
    if (retval)
    {
        QStringList proxies = QStringList::split(';', retval->toString().value().qstring());
        if (!proxies.count())
        {
            kdDebug(7025) << "KPACImpl::proxyForULR(): JS returned an empty string, not using a proxy" << endl;
            return QString::null;
        }
        KSimpleConfig blackList(locate("tmp", "badproxies"));
        for (QStringList::ConstIterator it = proxies.begin(); it != proxies.end(); ++it)
        {
            QString proxy = (*it).simplifyWhiteSpace();
            if (proxy.left(5) == "PROXY")
            {
                proxy = proxy.mid(5).stripWhiteSpace();
                if (proxy.left(7) != "http://")
                    proxy = "http://" + proxy;
                time_t badMark = blackList.readNumEntry(proxy);
                if (badMark < time(0) - 1800)
                {
                    if (badMark)
                        blackList.deleteEntry(proxy, false);
                    kdDebug(7025) << "KPACImpl::proxyForURL(): returning " << proxy << endl;
                    return proxy;
                }
            }
            else if (proxy == "DIRECT")
            {
                kdDebug(7025) << "KPACImpl::proxyForURL(): returning DIRECT" << endl;
                return proxy;
            }
        }
    }
    kdDebug(7025) << "KPACImpl::proxyForURL(): didn't find a proxy" << endl;
    return QString::null;
}

bool KPACImpl::init(const KURL &url)
{
    kdDebug(7025) << "KPACImpl::init()" << endl;
    if (m_configRead)
    {
        m_kjs->clear();
        m_configRead = false;
    }

    QCString code = KPACDownloader::download(url);
    if (code.isEmpty())
    {
        kdError(7025) << "KPACImpl::init(): couldn't download proxy config script " << url.url() << endl;
        return false;
    }
        
    if (!m_kjs)
    {
        m_kjs = new KJScript();
        Global global(Global::current());
        KJSO bindings(new KProxyBindings);
        global.put("ProxyConfig", bindings);
        global.setPrototype(bindings);
    }

    if (!(m_configRead = m_kjs->evaluate(code)))
    {
        kdError(7025) << "KPACImpl::init(): JS error in config file" << endl;
        m_kjs->clear();
    }

    return m_configRead;
}

void KPACImpl::badProxy(const QString &proxy)
{
    kdDebug(7025) << "KPACImpl::badProxy(), proxy=" << proxy << endl;
    KSimpleConfig blackList(locateLocal("tmp", "badproxies"));
    blackList.writeEntry(proxy, time(0));
}

const QCString KPACDownloader::download(const KURL &url)
{
    KPACDownloader downloader(url);
    while (downloader.m_downloading)
        kapp->processOneEvent();
    return downloader.m_data;
}

KPACDownloader::KPACDownloader(const KURL &url)
    : QObject()
{
    KIO::TransferJob *job = KIO::get(url, false /* reload */, false /* No GUI */);
    m_downloading = true;
    connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)), SLOT(slotData(KIO::Job *, const QByteArray &)));
    connect(job, SIGNAL(result(KIO::Job *)), SLOT(slotResult(KIO::Job *)));
}

void KPACDownloader::slotData(KIO::Job *, const QByteArray &data)
{
    m_data += data;
}

void KPACDownloader::slotResult(KIO::Job *job)
{
    if (job->error())
        m_data = 0;
    m_downloading = false;
}

extern "C"
{
    void *create_pac()
    {
        kdDebug(7025) << "create_pac() creating a KPACImpl" << endl;
        return new KPACImpl;
    }
};

