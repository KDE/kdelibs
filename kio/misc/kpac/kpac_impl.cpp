/*
 *  $Id$
 *  Proxy Auto Configuration
 *  
 *  Copyright (C) 2000 Malte Starostik <malte@kde.org>
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

#include <kdebug.h>
#include <kurl.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <kjs/object.h>
#include <kjs/types.h>
#include <kjs/interpreter.h>

#include "kproxybindings.h"
#include "kpac_impl.h"
#include "kpac_downloader.h"
#include "kpac_discovery.h"

using namespace KJS;

KPACImpl::KPACImpl()
    : m_interpreter(0),
      m_configRead(false),
      m_inDiscovery(false),
      m_downloader(0)
{
}

KPACImpl::~KPACImpl()
{
    delete m_interpreter;
}

QString KPACImpl::proxyForURL(const KURL &url)
{
    kdDebug(7025) << "KPACImpl::proxyForURL(), url=" << url.prettyURL() << endl;

    QString p = url.protocol();
    if (p != "http" && p != "https" && p != "ftp" && p != "gopher")
      return "DIRECT";

    if (!m_configRead)
    {
        kdDebug(7025) << "KPACImpl::proxyForURL(): config not (yet) read, not using a proxy" << endl;
        return QString::null;
    }
    // don't use "return FindProxyForURL('%1','%2')" here - it breaks with URLs containing %20
    QString code = QString("return FindProxyForURL('" ) + url.url() + "', '" + url.host() + "');";
    UString ucode = code.local8Bit().data();
    Completion comp = m_interpreter->evaluate(ucode);
    if (comp.complType() == Throw)
    {
        kdDebug(7025) << "KPACImpl::proxyForURL(): JS evaluation error, not using a proxy" << endl;
        return QString::null;
    }
    else if (comp.complType() == ReturnValue)
    {
        QString val = comp.value().toString(m_interpreter->globalExec()).qstring();
        QStringList proxies = QStringList::split(';', val);
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
                KURL proxyURL(proxy = proxy.mid(5).stripWhiteSpace());
                // If the URL is invalid or the URL is valid but in opaque
                // format which indicates a port number being present in
                // this particular case, simply calling setProtocol() on
                // it trashes the whole URL.
                int len = proxyURL.protocol().length();
                if (!proxyURL.isValid() || proxy.find(":/", len) != len)
                    proxy.prepend("http://");
                time_t badMark = blackList.readNumEntry(proxy);
                if (badMark < time(0) - 1800)
                {
                    if (badMark)
                        blackList.deleteEntry(proxy, false);
                    kdDebug(7025) << "KPACImpl::proxyForURL(): returning " << proxy << endl;
                    return proxy;
                }
            }
            else if (proxy.left(5) == "SOCKS")
            {
                // FIXME
                kdWarning(7025) << "KPACImpl::proxyForURL(): SOCKS support not implemented yet" << endl;
                return "DIRECT";
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
        delete m_interpreter;
        m_interpreter = 0;
        m_configRead = false;
    }

    bool ownDownloader = m_downloader == 0;
    if (ownDownloader)
        m_downloader = new KPACDownloader;

    if (m_downloader->download(url))
    {
        if (!m_interpreter)
        {
            m_interpreter = new Interpreter();
            Object global(m_interpreter->globalObject());
            KProxyFunc::init(m_interpreter->globalExec(),global);
        }
        UString code = m_downloader->data().data();
        Completion comp = m_interpreter->evaluate(code);
        m_configRead = (comp.complType() != Throw);
        if (!m_configRead)
        {
            kdError(7025) << "KPACImpl::init(): JS error in config file" << endl;
            delete m_interpreter;
            m_interpreter = 0;
        }
    }
    else
        kdError(7025) << "KPACImpl::init(): couldn't download proxy config script " << url.url() << endl;
    if (ownDownloader)
    {
        delete m_downloader;
        m_downloader = 0;
    }

    return m_configRead;
}

bool KPACImpl::discover()
{
    if (m_inDiscovery)
        return false;
    m_inDiscovery = true;
    bool success = false;
    KPACDiscovery discovery;
    m_downloader = new KPACDownloader;
    while (discovery.tryDiscovery())
    {
        if ((success = init(discovery.curl())))
            break;
    }
    delete m_downloader;
    m_downloader = 0;
    m_inDiscovery = false;
    return success;
}

void KPACImpl::badProxy(const QString &proxy)
{
    kdDebug(7025) << "KPACImpl::badProxy(), proxy=" << proxy << endl;
    KSimpleConfig blackList(locateLocal("tmp", "badproxies"));
    blackList.writeEntry(proxy, time(0));
}

extern "C"
{
    KPAC *create_pac()
    {
        return new KPACImpl;
    }
};

// vim: ts=4 sw=4 et
