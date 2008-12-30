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

#ifndef KNEWSTUFF2_TEST_TEST_H
#define KNEWSTUFF2_TEST_TEST_H

#include <knewstuff2/core/provider.h>
#include <knewstuff2/core/entry.h>

#include <QtCore/QObject>

namespace KNS
{
class CoreEngine;
}

class KNewStuff2Test : public QObject
{
    Q_OBJECT
public:
    KNewStuff2Test();
    void setTestAll(bool testall);
    void entryTest();
    void providerTest();
    void engineTest();
public slots:
    void slotProviderLoaded(KNS::Provider *provider);
    void slotProvidersFailed();
    void slotEntryLoaded(KNS::Entry *entry, const KNS::Feed *feed, const KNS::Provider *provider);
    void slotEntriesFailed();
    void slotEntriesFinished();
    void slotPayloadLoaded(KUrl payload);
    void slotPayloadFailed();
    void slotInstallationFinished();
    void slotInstallationFailed();
private:
    void quitTest();
    KNS::CoreEngine *m_engine;
    bool m_testall;
};

#endif
