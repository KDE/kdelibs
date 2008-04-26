/* This file is part of the KDE libraries
    Copyright (c) 2008 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kprocess.h>
#include <kconfiggroup.h>
#include <kdesktopfile.h>
#include <kmimetypetrader.h>
#include <kdebug.h>
#include <kservicefactory.h>
#include <ktemporaryfile.h>
#include <qtest_kde.h>
#include "kmimeassociations.h"
#include <ksycoca.h>
#include <kstandarddirs.h>

// We need a factory that returns the same KService::Ptr every time it's asked for a given service.
// Otherwise the changes to the service's serviceTypes by KMimeAssociationsTest have no effect
class FakeServiceFactory : public KServiceFactory
{
public:
    FakeServiceFactory() : KServiceFactory() {}

    virtual KService::Ptr findServiceByMenuId(const QString &name) {
        //kDebug() << name;
        KService::Ptr result = m_cache.value(name);
        if (!result) {
            result = KServiceFactory::findServiceByMenuId(name);
            m_cache.insert(name, result);
        }
        //kDebug() << name << result.data();
        return result;
    }
    virtual KService::Ptr findServiceByDesktopPath(const QString &name)
    {
        KService::Ptr result = m_cache.value(name); // yeah, same cache, I don't care :)
        if (!result) {
            result = KServiceFactory::findServiceByDesktopPath(name);
            m_cache.insert(name, result);
        }
        return result;
    }
private:
    QMap<QString, KService::Ptr> m_cache;
};

// Helper method for all the trader tests, comes from kmimetypetest.cpp
static bool offerListHasService( const KService::List& offers,
                                 const QString& entryPath )
{
    bool found = false;
    Q_FOREACH(const KService::Ptr &serv, offers) {
        if ( serv->entryPath() == entryPath ) {
            if( found ) { // should be there only once
                qWarning( "ERROR: %s was found twice in the list", qPrintable( entryPath ) );
                return false; // make test fail
            }
            found = true;
        }
    }
    return found;
}

class KMimeAssociationsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QString kdehome = QDir::home().canonicalPath() + "/.kde-unit-test";
        // We need a place where we can hack a mimeapps.list without harm, so not ~/.local
        ::setenv("XDG_DATA_HOME", QFile::encodeName(kdehome) + "/local", 1);
        m_localApps = kdehome + "/local/applications/";

        // Create factory on the heap and don't delete it.
        // It registers to KSycoca, which deletes it at end of program execution.
        KServiceFactory* factory = new FakeServiceFactory;
        QCOMPARE(KServiceFactory::self(), factory); // ctor sets s_self
        bool mustUpdateKSycoca = false;
        if ( !KSycoca::isAvailable() ) {
            mustUpdateKSycoca = true;
        }
        if (QFile::exists(m_localApps + "/mimeapps.list")) {
            QFile::remove(m_localApps + "/mimeapps.list");
            mustUpdateKSycoca = true;
        }

        // Create fake application for some tests below.
        fakeApplication = m_localApps + "faketextapplication.desktop";
        const bool mustCreateFakeService = !QFile::exists(fakeApplication);
        if (mustCreateFakeService) {
            mustUpdateKSycoca = true;
            KDesktopFile file(fakeApplication);
            KConfigGroup group = file.desktopGroup();
            group.writeEntry("Name", "FakeApplication");
            group.writeEntry("Type", "Application");
            group.writeEntry("Exec", "ls");
            group.writeEntry("MimeType", "text/plain");
        }

        if ( mustUpdateKSycoca ) {
            // Update ksycoca in ~/.kde-unit-test after creating the above
            runKBuildSycoca();
        }

        // For debugging: print all services and their storageId
#if 0
        const KService::List lst = KService::allServices();
        QVERIFY( !lst.isEmpty() );
        Q_FOREACH(const KService::Ptr& serv, lst) {
            qDebug() << serv->entryPath() << serv->storageId() /*<< serv->desktopEntryName()*/;
        }
#endif

        KService::Ptr fakeApplicationService = KService::serviceByStorageId("faketextapplication.desktop");
        QVERIFY(fakeApplicationService);

        m_mimeAppsFileContents = "[Added Associations]\n"
                               "image/jpeg=mplayer.desktop;\n"
                               // konsole.desktop is without kde4- to test fallback lookup
                               "text/plain=kde4-kate.desktop;kde4-kwrite.desktop;konsole.desktop;idontexist.desktop;\n"
                               "[Added KParts/ReadOnlyPart Associations]\n"
                               "text/plain=katepart.desktop;\n"
                               "[Removed Associations]\n"
                               "image/jpeg=firefox.desktop;\n"
                               "text/html=firefox.desktop;kde4-kwrite.desktop;\n";
        // Expected results
        preferredApps["image/jpeg"] << "mplayer.desktop";
        preferredApps["text/plain"] << "kde4-kate.desktop" << "kde4-kwrite.desktop";
        removedApps["image/jpeg"] << "firefox.desktop";
        removedApps["text/html"] << "firefox.desktop" << "kde4-kwrite.desktop";

        // Clean-up non-existing apps
        removeNonExisting(preferredApps);
        removeNonExisting(removedApps);
    }

    void cleanupTestCase()
    {
        QFile::remove(m_localApps + "/mimeapps.list");
        runKBuildSycoca();
    }

    void testParseSingleFile()
    {
        KOfferHash offerHash;
        KMimeAssociations parser(offerHash);

        KTemporaryFile tempFile;
        QVERIFY(tempFile.open());
        tempFile.write(m_mimeAppsFileContents);
        const QString fileName = tempFile.fileName();
        tempFile.close();

        QTest::ignoreMessage(QtDebugMsg, "findServiceByDesktopPath: idontexist.desktop not found");
        parser.parseMimeAppsList(fileName, 100);

        for (ExpectedResultsMap::const_iterator it = preferredApps.begin(), end = preferredApps.end() ; it != end ; ++it) {
            const QString mime = it.key();
            const QList<KServiceOffer> offers = offerHash.offersFor(mime);
            Q_FOREACH(const QString& service, it.value()) {
                KService::Ptr serv = KService::serviceByStorageId(service);
                if (serv && !offersContains(offers, serv)) {
                    kDebug() << serv.data() << serv->entryPath() << "does not have" << mime;
                    QFAIL("offer does not have servicetype");
                }
            }
        }

        for (ExpectedResultsMap::const_iterator it = removedApps.begin(), end = removedApps.end() ; it != end ; ++it) {
            const QString mime = it.key();
            const QList<KServiceOffer> offers = offerHash.offersFor(mime);
            Q_FOREACH(const QString& service, it.value()) {
                KService::Ptr serv = KService::serviceByStorageId(service);
                if (serv && offersContains(offers, serv)) {
                    kDebug() << serv.data() << serv->entryPath() << "does not have" << mime;
                    QFAIL("offer should not have servicetype");
                }
            }
        }
    }

    void testSetupRealFile()
    {
        writeToMimeApps(m_mimeAppsFileContents);

        // Test a trader query
        KService::List offers = KMimeTypeTrader::self()->query("image/jpeg");
        if (KService::serviceByStorageId("mplayer.desktop")) { // if it's installed
            QCOMPARE(offers.first()->storageId(), QString("mplayer.desktop"));
        }

        // Now the generic variant of the above test:
        // for each mimetype, check that the preferred apps are as specified
        for (ExpectedResultsMap::const_iterator it = preferredApps.begin(), end = preferredApps.end() ; it != end ; ++it) {
            const QString mime = it.key();
            const KService::List offers = KMimeTypeTrader::self()->query(mime);
            //for (int i = 0; i < offers.count(); ++i) {
            //    kDebug() << "offers for" << mime << ":" << i << offers[i]->storageId();
            //}
            const QStringList expectedPreferredServices = it.value();
            for (int i = 0; i < expectedPreferredServices.count(); ++i) {
                //kDebug() << mime << i << expectedPreferredServices[i];
                QCOMPARE(expectedPreferredServices[i], offers[i]->storageId());
            }
        }
    }

    void testRemovedAssociation()
    {
        // I removed kate from text/plain, and it would still appear in text/x-java.

        // First, let's check our fake app is associated with text/plain
        KService::List offers = KMimeTypeTrader::self()->query("text/plain");
        QVERIFY(offerListHasService(offers, fakeApplication));

        writeToMimeApps(QByteArray("[Removed Associations]\n"
                                   "text/plain=faketextapplication.desktop;\n"));

        offers = KMimeTypeTrader::self()->query("text/plain");
        QVERIFY(!offerListHasService(offers, fakeApplication));

        offers = KMimeTypeTrader::self()->query("text/x-java");
        QVERIFY(!offerListHasService(offers, fakeApplication));
}

private:
    typedef QMap<QString /*mimetype*/, QStringList> ExpectedResultsMap;

    void runKBuildSycoca()
    {
        // Wait for notifyDatabaseChanged DBus signal
        // (The real KCM code simply does the refresh in a slot, asynchronously)
        QEventLoop loop;
        QObject::connect(KSycoca::self(), SIGNAL(databaseChanged()), &loop, SLOT(quit()));
        KProcess proc;
        proc << KStandardDirs::findExe(KBUILDSYCOCA_EXENAME);
        proc.setOutputChannelMode(KProcess::MergedChannels); // silence kbuildsycoca output
        proc.execute();
        loop.exec();
    }

    void writeToMimeApps(const QByteArray& contents)
    {
        QString mimeAppsPath = m_localApps + "/mimeapps.list";
        QFile mimeAppsFile(mimeAppsPath);
        QVERIFY(mimeAppsFile.open(QIODevice::WriteOnly));
        mimeAppsFile.write(contents);
        mimeAppsFile.close();

        runKBuildSycoca();
    }

    bool offersContains(const QList<KServiceOffer>& offers, KService::Ptr serv)
    {
        Q_FOREACH(const KServiceOffer& offer, offers) {
            if (offer.service()->storageId() == serv->storageId())
                return true;
        }
        return false;
    }

    void removeNonExisting(ExpectedResultsMap& erm) {
        for (ExpectedResultsMap::iterator it = erm.begin(), end = erm.end() ; it != end ; ++it) {
            QMutableStringListIterator serv_it( it.value() );
            while (serv_it.hasNext()) {
                if (!KService::serviceByStorageId(serv_it.next())) {
                    kDebug() << "removing non-existing entry" << serv_it.value();
                    serv_it.remove();
                }
            }
        }
    }
    QString m_localApps;
    QByteArray m_mimeAppsFileContents;
    QString fakeApplication;

    ExpectedResultsMap preferredApps;
    ExpectedResultsMap removedApps;
};

QTEST_KDEMAIN_CORE( KMimeAssociationsTest )

#include "kmimeassociationstest.moc"
