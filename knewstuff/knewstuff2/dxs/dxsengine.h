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

#ifndef KNEWSTUFF2_DXS_ENGINE_H
#define KNEWSTUFF2_DXS_ENGINE_H

#include <knewstuff2/core/coreengine.h>

namespace KNS
{

class Dxs;
class Category;

/**
 * KNewStuff DXS engine.
 * A DXS engine re-implements several of the core engine's methods to perform
 * data operations through a web service instead of through the traditional
 * GHNS uploads and downloads. Whenever this is not possible it will
 * fall back to the core engine's implementation instead.
 *
 * @internal
 */
class DxsEngine : public CoreEngine
{
    Q_OBJECT
public:
    DxsEngine(QObject* parent);
    ~DxsEngine();

    enum Policy {
        DxsNever,
        DxsIfPossible,
        DxsAlways
    };

    void setDxsPolicy(Policy policy);

    void loadEntries(Provider *provider);
    //void downloadPreview(Entry *entry);
    //void downloadPayload(Entry *entry);
    // FIXME: the upload/download stuff is only necessary when we use
    // integrated base64-encoded files; maybe delay to later version?

    //bool uploadEntry(Provider *provider, Entry *entry);

    // get the dxs object
    Dxs * dxsObject(const Provider * provider);

private slots:
    // FIXME: idem for slots
    void slotEntriesLoadedDXS(KNS::Entry::List list, Feed * feed);
    void slotEntriesFailed();

    void slotCategories(QList<KNS::Category*>);

private:
    QMap<const Provider *, Dxs *> m_dxsbyprovider;
    Policy m_dxspolicy;
};

}

#endif
