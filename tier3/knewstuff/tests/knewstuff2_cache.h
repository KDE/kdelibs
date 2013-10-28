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

#ifndef KNEWSTUFF2_TEST_CACHE_H
#define KNEWSTUFF2_TEST_CACHE_H

#include <knewstuff2/core/entry.h>

#include <QtCore/QObject>

namespace KNS
{
class CoreEngine;
class Feed;
class Provider;
}

class KNewStuff2Cache : public QObject
{
    Q_OBJECT
public:
    KNewStuff2Cache();
    void run();
public Q_SLOTS:
    void slotEntryLoaded(KNS::Entry *entry, const KNS::Feed *feed, const KNS::Provider *provider);
    void slotEntriesFailed();
    void slotEntriesFinished();
private:
    void quitTest();
    KNS::CoreEngine *m_engine;
};

#endif
