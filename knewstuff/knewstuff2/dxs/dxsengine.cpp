/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "dxsengine.h"

#include "dxs.h"

#include <kdebug.h>

using namespace KNS;

DxsEngine::DxsEngine(QObject* parent)
    : CoreEngine(parent)
{
    m_dxs = NULL;
    m_dxspolicy = DxsIfPossible;
}

DxsEngine::~DxsEngine()
{
}

void DxsEngine::setDxsPolicy(Policy policy)
{
    m_dxspolicy = policy;
}

void DxsEngine::loadEntries(Provider *provider)
{
    kDebug() << "loading entries";
    // Ensure that the provider offers DXS at all
    // Match DXS offerings with the engine's policy
    if (provider->webService().isValid()) {
        if (m_dxspolicy == DxsNever) {
            CoreEngine::loadEntries(provider);
            return;
        }
    }
    else {
        if (m_dxspolicy != DxsAlways) {
            CoreEngine::loadEntries(provider);
            return;
        }
        else {
            kError() << "DxsEngine: DXS requested but not offered" << endl;
            return;
        }
    }

    // From here on, it's all DXS now

    if (!m_dxs) {
        m_dxs = new Dxs(this);
    }

    m_dxs->setEndpoint(provider->webService());

    // FIXME: load all categories first, then feeds second
    m_dxs->call_entries(QString(), QString());

    connect(m_dxs,
            SIGNAL(signalEntries(KNS::Entry::List)),
            SLOT(slotEntriesLoaded(KNS::Entry::List)));
    connect(m_dxs,
            SIGNAL(signalFault()),
            SLOT(slotEntriesFailed()));
    // FIXME: which one of signalFault()/signalError()? Or both?
}

void DxsEngine::slotEntriesLoaded(KNS::Entry::List list)
{
    // FIXME: we circumvent the cache now...
    for (Entry::List::Iterator it = list.begin(); it != list.end(); ++it) {
        Entry *entry = (*it);
        // FIXME: the association to feed and provider is missing here
        emit signalEntryLoaded(entry, NULL, NULL);
    }
}

void DxsEngine::slotEntriesFailed()
{
    emit signalEntriesFailed();
}

// Unneeded for now
// #include "dxsengine.moc"
