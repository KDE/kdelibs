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

#include "knewstuff2_cache.h"

#include <knewstuff2/core/entryhandler.h>
#include <knewstuff2/core/coreengine.h>

#include <kstandarddirs.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

#include <QtCore/QFile>

#include <unistd.h> // for exit()

KNewStuff2Cache::KNewStuff2Cache()
        : QObject()
{
    m_engine = NULL;
}

void KNewStuff2Cache::run()
{
    kDebug() << "-- start the engine";

    m_engine = new KNS::CoreEngine(0);
    m_engine->setCachePolicy(KNS::CoreEngine::CacheOnly);
    bool ret = m_engine->init("knewstuff2_test.knsrc");

    kDebug() << "-- engine initialisation result: " << ret;

    if (ret) {
        connect(m_engine,
                SIGNAL(signalEntryLoaded(KNS::Entry*, const KNS::Feed*, const KNS::Provider*)),
                SLOT(slotEntryLoaded(KNS::Entry*, const KNS::Feed*, const KNS::Provider*)));
        connect(m_engine,
                SIGNAL(signalEntriesFailed()),
                SLOT(slotEntriesFailed()));
        connect(m_engine,
                SIGNAL(signalEntriesFinished()),
                SLOT(slotEntriesFinished()));

        m_engine->start();
    } else {
        kWarning() << "ACHTUNG: you probably need to 'make install' the knsrc file first.";
        kWarning() << "Although this is not required anymore, so something went really wrong.";
        quitTest();
    }
}

void KNewStuff2Cache::slotEntryLoaded(KNS::Entry *entry, const KNS::Feed *feed, const KNS::Provider *provider)
{
    Q_UNUSED(feed);
    Q_UNUSED(provider);

    kDebug() << "SLOT: slotEntryLoaded";
    kDebug() << "-- entry: " << entry->name().representation();
}

void KNewStuff2Cache::slotEntriesFailed()
{
    kDebug() << "SLOT: slotEntriesFailed";
    quitTest();
}

void KNewStuff2Cache::slotEntriesFinished()
{
    kDebug() << "SLOT: slotEntriesFinished";
    quitTest();
}

void KNewStuff2Cache::quitTest()
{
    kDebug() << "-- quitting now...";
    if (1 == 0) {
        // this would be the soft way out...
        delete m_engine;
        deleteLater();
        kapp->quit();
    } else {
        exit(1);
    }
}

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "knewstuff2_cache", 0, ki18n("knewstuff2_cache"), 0);
    KApplication app(false);

    // Take source directory into account
    kDebug() << "-- adding source directory " << KNSSRCDIR;
    kDebug() << "-- adding build directory " << KNSBUILDDIR;
    KGlobal::dirs()->addResourceDir("config", KNSSRCDIR);
    KGlobal::dirs()->addResourceDir("config", KNSBUILDDIR);

    KNewStuff2Cache *test = new KNewStuff2Cache();
    test->run();

    return app.exec();
}

#include "knewstuff2_cache.moc"
