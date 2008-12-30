/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

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

#include "dxsengine.h"

#include "dxs.h"

#include <knewstuff2/core/category.h>
#include <kdebug.h>

using namespace KNS;

DxsEngine::DxsEngine(QObject* parent)
        : CoreEngine(parent), m_dxspolicy(DxsIfPossible)
{
}

DxsEngine::~DxsEngine()
{
}

void DxsEngine::setDxsPolicy(Policy policy)
{
    m_dxspolicy = policy;
}

// get the dxs object
Dxs * DxsEngine::dxsObject(const Provider * provider)
{
    return (m_dxsbyprovider.contains(provider) ? m_dxsbyprovider.value(provider) : NULL);
}

void DxsEngine::loadEntries(Provider *provider)
{
    ////kDebug() << "loading entries for provider " << provider->name().representation();
    // Ensure that the provider offers DXS at all
    // Match DXS offerings with the engine's policy
    if (provider->webService().isValid()) {
        if (m_dxspolicy == DxsNever) {
            CoreEngine::loadEntries(provider);
            return;
        }
    } else {
        if (m_dxspolicy != DxsAlways) {
            CoreEngine::loadEntries(provider);
            return;
        } else {
            kError() << "DxsEngine: DXS requested but not offered" << endl;
            return;
        }
    }

    // From here on, it's all DXS now

    if (!m_dxsbyprovider.contains(provider)) {
        Dxs * dxs = new Dxs(this, provider);
        dxs->setEndpoint(provider->webService());
        // connect entries signal
        connect(dxs, SIGNAL(signalEntries(KNS::Entry::List, Feed*)),
                SLOT(slotEntriesLoadedDXS(KNS::Entry::List, Feed*)));
        // FIXME: which one of signalFault()/signalError()? Or both?
        connect(dxs, SIGNAL(signalFault()),
                SLOT(slotEntriesFailed()));
        // connect categories signal
        connect(dxs, SIGNAL(signalCategories(QList<KNS::Category*>)),
                SLOT(slotCategories(QList<KNS::Category*>)));
        m_dxsbyprovider.insert(provider, dxs);
    }

    Dxs * dxs = m_dxsbyprovider.value(provider);

    dxs->call_categories();
}

void DxsEngine::slotCategories(QList<KNS::Category*> categories)
{
    Dxs * dxs = qobject_cast<Dxs*>(sender());
    Provider * provider = dxs->provider();

    //kDebug() << "slot categories called for provider: " << provider->name().representation();

    for (QList<KNS::Category*>::iterator it = categories.begin(); it != categories.end(); ++it) {
        Category *category = (*it);
        QStringList feeds = provider->feeds();
        for (int i = 0; i < feeds.size(); ++i) {
            dxs->call_entries(category->id(), feeds.at(i));
        }
    }
}

void DxsEngine::slotEntriesLoadedDXS(KNS::Entry::List list, Feed * feed)
{
    Dxs * dxs = qobject_cast<Dxs*>(sender());
    Provider * provider = dxs->provider();

    mergeEntries(list, feed, provider);
}

void DxsEngine::slotEntriesFailed()
{
    emit signalEntriesFailed();
}

#include "dxsengine.moc"
