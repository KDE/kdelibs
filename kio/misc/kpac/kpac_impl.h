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

#ifndef _KPAC_IMPL_H_
#define _KPAC_IMPL_H_

#include <kio/kpac.h>

class KURL;
class KPACDownloader;

namespace KJS {
  class Interpreter;
}

class KPACImpl : public KPAC
{
public:
    KPACImpl();
    virtual ~KPACImpl();
    virtual QString proxyForURL(const KURL &url);
    virtual bool init(const KURL &url);
    virtual bool discover();
    virtual void badProxy(const QString &proxy);

private:
    KJS::Interpreter *m_interpreter;
    bool m_configRead;
    bool m_inDiscovery;
    KPACDownloader *m_downloader;
};

#endif

