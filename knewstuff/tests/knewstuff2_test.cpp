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

#include "knewstuff2_test.h"

#include <knewstuff2/core/ktranslatable.h>
#include <knewstuff2/core/providerhandler.h>
#include <knewstuff2/core/entryhandler.h>
#include <knewstuff2/core/coreengine.h>

#include <kstandarddirs.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

#include <QtCore/QFile>

#include <unistd.h> // for exit()
#include <stdio.h> // for stdout

KNewStuff2Test::KNewStuff2Test()
        : QObject()
{
    m_engine = NULL;
    m_testall = false;
}

void KNewStuff2Test::setTestAll(bool testall)
{
    m_testall = testall;
}

void KNewStuff2Test::entryTest()
{
    kDebug() << "-- test kns2 entry class";

    QDomDocument doc;
    QFile f(QString("%1/testdata/entry.xml").arg(KNSSRCDIR));
    if (!f.open(QIODevice::ReadOnly)) {
        kDebug() << "Error loading entry file.";
        quitTest();
    }
    if (!doc.setContent(&f)) {
        kDebug() << "Error parsing entry file.";
        f.close();
        quitTest();
    }
    f.close();

    KNS::EntryHandler eh(doc.documentElement());
    KNS::Entry e = eh.entry();

    kDebug() << "-- xml->entry test result: " << eh.isValid();

    KNS::EntryHandler eh2(e);
    QDomElement exml = eh2.entryXML();

    kDebug() << "-- entry->xml test result: " << eh.isValid();

    if (!eh.isValid()) {
        quitTest();
    } else {
        QTextStream out(stdout);
        out << exml;
    }
}

void KNewStuff2Test::providerTest()
{
    kDebug() << "-- test kns2 provider class";

    QDomDocument doc;
    QFile f(QString("%1/testdata/provider.xml").arg(KNSSRCDIR));
    if (!f.open(QIODevice::ReadOnly)) {
        kDebug() << "Error loading provider file.";
        quitTest();
    }
    if (!doc.setContent(&f)) {
        kDebug() << "Error parsing provider file.";
        f.close();
        quitTest();
    }
    f.close();

    KNS::ProviderHandler ph(doc.documentElement());
    KNS::Provider p = ph.provider();

    kDebug() << "-- xml->provider test result: " << ph.isValid();

    KNS::ProviderHandler ph2(p);
    QDomElement pxml = ph2.providerXML();

    kDebug() << "-- provider->xml test result: " << ph.isValid();

    if (!ph.isValid()) {
        quitTest();
    } else {
        QTextStream out(stdout);
        out << pxml;
    }
}

void KNewStuff2Test::engineTest()
{
    kDebug() << "-- test kns2 engine";

    m_engine = new KNS::CoreEngine(NULL);
    bool ret = m_engine->init("knewstuff2_test.knsrc");

    kDebug() << "-- engine test result: " << ret;

    if (ret) {
        connect(m_engine,
                SIGNAL(signalProviderLoaded(KNS::Provider*)),
                SLOT(slotProviderLoaded(KNS::Provider*)));
        connect(m_engine,
                SIGNAL(signalProvidersFailed()),
                SLOT(slotProvidersFailed()));
        connect(m_engine,
                SIGNAL(signalEntryLoaded(KNS::Entry*, const KNS::Feed*, const KNS::Provider*)),
                SLOT(slotEntryLoaded(KNS::Entry*, const KNS::Feed*, const KNS::Provider*)));
        connect(m_engine,
                SIGNAL(signalEntriesFinished()),
                SLOT(slotEntriesFinished()));
        connect(m_engine,
                SIGNAL(signalEntriesFailed()),
                SLOT(slotEntriesFailed()));
        connect(m_engine,
                SIGNAL(signalPayloadLoaded(KUrl)),
                SLOT(slotPayloadLoaded(KUrl)));
        connect(m_engine,
                SIGNAL(signalPayloadFailed()),
                SLOT(slotPayloadFailed()));
        connect(m_engine,
                SIGNAL(signalInstallationFinished()),
                SLOT(slotInstallationFinished()));
        connect(m_engine,
                SIGNAL(signalInstallationFailed()),
                SLOT(slotInstallationFailed()));

        m_engine->start();
    } else {
        kWarning() << "ACHTUNG: you probably need to 'make install' the knsrc file first.";
        kWarning() << "Although this is not required anymore, so something went really wrong.";
        quitTest();
    }
}

void KNewStuff2Test::slotProviderLoaded(KNS::Provider *provider)
{
    kDebug() << "SLOT: slotProviderLoaded";
    kDebug() << "-- provider: " << provider->name().representation();

    m_engine->loadEntries(provider);
}

void KNewStuff2Test::slotEntryLoaded(KNS::Entry *entry, const KNS::Feed *feed, const KNS::Provider *provider)
{
    Q_UNUSED(feed);
    Q_UNUSED(provider);

    kDebug() << "SLOT: slotEntryLoaded";
    kDebug() << "-- entry: " << entry->name().representation();

    if (m_testall) {
        kDebug() << "-- now, download the entry's preview and payload file";

        if (!entry->preview().isEmpty())
            m_engine->downloadPreview(entry);
        if (!entry->payload().isEmpty())
            m_engine->downloadPayload(entry);
    }
}

void KNewStuff2Test::slotEntriesFinished()
{
    // Wait for installation if requested
    if (!m_testall) {
        quitTest();
    }
}

void KNewStuff2Test::slotPayloadLoaded(KUrl payload)
{
    kDebug() << "-- entry downloaded successfully";
    kDebug() << "-- downloaded to " << payload.prettyUrl();

    kDebug() << "-- run installation";

    bool ret = m_engine->install(payload.path());

    kDebug() << "-- installation result: " << ret;
    kDebug() << "-- now, wait for installation to finish...";
}

void KNewStuff2Test::slotPayloadFailed()
{
    kDebug() << "SLOT: slotPayloadFailed";
    quitTest();
}

void KNewStuff2Test::slotProvidersFailed()
{
    kDebug() << "SLOT: slotProvidersFailed";
    quitTest();
}

void KNewStuff2Test::slotEntriesFailed()
{
    kDebug() << "SLOT: slotEntriesFailed";
    quitTest();
}

void KNewStuff2Test::slotInstallationFinished()
{
    kDebug() << "SLOT: slotInstallationFinished";
    kDebug() << "-- OK, finish test";
    quitTest();
}

void KNewStuff2Test::slotInstallationFailed()
{
    kDebug() << "SLOT: slotInstallationFailed";
    quitTest();
}

void KNewStuff2Test::quitTest()
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
    KCmdLineOptions options;
    options.add("testall", ki18n("Downloads all previews and payloads"));

    KCmdLineArgs::init(argc, argv, "knewstuff2_test", 0, ki18n("knewstuff2_test"), 0);
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app(false);

    // Take source directory into account
    kDebug() << "-- adding source directory " << KNSSRCDIR;
    kDebug() << "-- adding build directory " << KNSBUILDDIR;
    KGlobal::dirs()->addResourceDir("config", KNSSRCDIR);
    KGlobal::dirs()->addResourceDir("config", KNSBUILDDIR);

    KNewStuff2Test *test = new KNewStuff2Test();
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->isSet("testall")) {
        test->setTestAll(true);
        test->entryTest();
        test->providerTest();
    }
    test->engineTest();

    return app.exec();
}

#include "knewstuff2_test.moc"
