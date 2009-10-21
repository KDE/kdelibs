/*
    Copyright (c) 2009 Frederik Gladhorn <gladhorn@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "atticaprovider.h"

#include "xmlloader.h"
#include "core/feed.h"
#include "core/provider_p.h"

#include <kdebug.h>
#include <klocale.h>
#include <kio/job.h>

#include <attica/providermanager.h>
#include <attica/provider.h>

namespace KNS3
{

class AtticaProviderPrivate :public ProviderPrivate
{
public:
    AtticaProviderPrivate()
    {    
    }

    Attica::ProviderManager m_providerManager;
    
};

AtticaProvider::AtticaProvider()
    : d_ptr(new AtticaProviderPrivate)
{
    // TODO
    Q_D(AtticaProvider);
    d->mName = KTranslatable("Attica");

    connect(&d->m_providerManager, SIGNAL(providersChanged()), SLOT(providerLoaded()));
}

AtticaProvider::~AtticaProvider()
{
    // d_ptr is deleted in base class!
}

bool AtticaProvider::setProviderXML(QDomElement & xmldata)
{
    Q_D(AtticaProvider);
    kDebug(550) << "setting provider xml";

    if (xmldata.tagName() != "provider")
        return false;
    
    

    return true;
}

QDomElement AtticaProvider::providerXML() const
{
    Q_D(const AtticaProvider);
    QDomDocument doc;

    QDomElement el = doc.createElement("provider");

    return el;
}

void providerLoaded()
{

}

QStringList AtticaProvider::availableFeeds() const
{
    // FIXME
    return QStringList();
}

}

#include "atticaprovider.moc"
