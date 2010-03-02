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

#include <kmimetypefactory.h>
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
                                 const QString& entryPath,
                                 bool expected /* if set, show error if not found */ )
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
    if (!found && expected) {
        kWarning() << "ERROR:" << entryPath << "not found in offer list. Here's the full list:";
        Q_FOREACH(const KService::Ptr &serv, offers) {
            kDebug() << serv->entryPath();
        }
    }
    return found;
}

static void writeAppDesktopFile(const QString& path, const QStringList& mimeTypes)
{
    KDesktopFile file(path);
    KConfigGroup group = file.desktopGroup();
    group.writeEntry("Name", "FakeApplication");
    group.writeEntry("Type", "Application");
    group.writeEntry("Exec", "ls");
    group.writeXdgListEntry("MimeType", mimeTypes);
}

/**
 * This unit test verifies the parsing of mimeapps.list files, both directly
 * and via kbuildsycoca (and making trader queries).
 */
class KMimeAssociationsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QString kdehome = QDir::home().canonicalPath() + "/.kde-unit-test";
        m_localApps = KStandardDirs::locateLocal("xdgdata-apps", "");
        QVERIFY(m_localApps.startsWith(kdehome));

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
        fakeTextApplication = m_localApps + "faketextapplication.desktop";
        if (!QFile::exists(fakeTextApplication)) {
            mustUpdateKSycoca = true;
            writeAppDesktopFile(fakeTextApplication, QStringList() << "text/plain");
        }

        fakeJpegApplication = m_localApps + "fakejpegapplication.desktop";
        if (!QFile::exists(fakeJpegApplication)) {
            mustUpdateKSycoca = true;
            writeAppDesktopFile(fakeJpegApplication, QStringList() << "image/jpeg");
        }

        fakeArkApplication = m_localApps + "fakearkapplication.desktop";
        if (!QFile::exists(fakeArkApplication)) {
            mustUpdateKSycoca = true;
            writeAppDesktopFile(fakeArkApplication, QStringList() << "application/zip");
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
                               "image/jpeg=fakejpegapplication.desktop;\n"
                               "text/html=kde4-kfmclient_html.desktop;\n"
                               // konsole.desktop is without kde4- to test fallback lookup
                               "text/plain=kde4-kate.desktop;kde4-kwrite.desktop;konsole.desktop;idontexist.desktop;\n"
                               // test alias resolution
                               "application/x-pdf=fakejpegapplication.desktop;\n"
                               "[Added KParts/ReadOnlyPart Associations]\n"
                               "text/plain=katepart.desktop;\n"
                               "[Removed Associations]\n"
                               "image/jpeg=firefox.desktop;\n"
                               "text/html=kde4-dolphin.desktop;kde4-kwrite.desktop;\n";
        // Expected results
        preferredApps["image/jpeg"] << "fakejpegapplication.desktop";
        preferredApps["application/pdf"] << "fakejpegapplication.desktop";
        preferredApps["text/plain"] << "kde4-kate.desktop" << "kde4-kwrite.desktop";
        preferredApps["text/html"] << "kde4-kfmclient_html.desktop";
        removedApps["image/jpeg"] << "firefox.desktop";
        removedApps["text/html"] << "kde4-dolphin.desktop" << "kde4-kwrite.desktop";

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
        KMimeAssociations parser(offerHash, KMimeTypeFactory::self());

        KTemporaryFile tempFile;
        QVERIFY(tempFile.open());
        tempFile.write(m_mimeAppsFileContents);
        const QString fileName = tempFile.fileName();
        tempFile.close();

        //QTest::ignoreMessage(QtDebugMsg, "findServiceByDesktopPath: idontexist.desktop not found");
        parser.parseMimeAppsList(fileName, 100);

        for (ExpectedResultsMap::const_iterator it = preferredApps.constBegin(),
                                               end = preferredApps.constEnd() ; it != end ; ++it) {
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

        for (ExpectedResultsMap::const_iterator it = removedApps.constBegin(),
                                               end = removedApps.constEnd() ; it != end ; ++it) {
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

    void testGlobalAndLocalFiles()
    {
        KOfferHash offerHash;
        KMimeAssociations parser(offerHash, KMimeTypeFactory::self());

        // Write global file
        KTemporaryFile tempFileGlobal;
        QVERIFY(tempFileGlobal.open());
        QByteArray globalAppsFileContents = "[Added Associations]\n"
                                            "image/jpeg=firefox.desktop;\n" // removed by local config
                                            "text/html=firefox.desktop;\n" // mdv
                                            "image/png=fakejpegapplication.desktop;\n";
        tempFileGlobal.write(globalAppsFileContents);
        const QString globalFileName = tempFileGlobal.fileName();
        tempFileGlobal.close();

        // We didn't keep it, so we need to write the local file again
        KTemporaryFile tempFile;
        QVERIFY(tempFile.open());
        tempFile.write(m_mimeAppsFileContents);
        const QString fileName = tempFile.fileName();
        tempFile.close();

        parser.parseMimeAppsList(globalFileName, 1000);
        parser.parseMimeAppsList(fileName, 1050); // += 50 is correct.

        QList<KServiceOffer> offers = offerHash.offersFor("image/jpeg");
        qStableSort(offers); // like kbuildservicefactory.cpp does
        const QStringList expectedJpegApps = preferredApps["image/jpeg"];
        QCOMPARE(assembleOffers(offers), expectedJpegApps);

        offers = offerHash.offersFor("text/html");
        qStableSort(offers);
        QStringList textHtmlApps = preferredApps["text/html"];
        if (KService::serviceByStorageId("firefox.desktop"))
            textHtmlApps.append("firefox.desktop");
        qDebug() << assembleOffers(offers);
        QCOMPARE(assembleOffers(offers), textHtmlApps);

        offers = offerHash.offersFor("image/png");
        qStableSort(offers);
        QCOMPARE(assembleOffers(offers), QStringList() << "fakejpegapplication.desktop");
    }

    void testSetupRealFile()
    {
        writeToMimeApps(m_mimeAppsFileContents);

        // Test a trader query
        KService::List offers = KMimeTypeTrader::self()->query("image/jpeg");
        //kDebug() << m_mimeAppsFileContents;
        //kDebug() << "preferred apps for jpeg: " << preferredApps.value("image/jpeg");
        //for (int i = 0; i < offers.count(); ++i) {
        //    kDebug() << "offers for" << "image/jpeg" << ":" << i << offers[i]->storageId();
        //}
        QCOMPARE(offers.first()->storageId(), QString("fakejpegapplication.desktop"));

        // Now the generic variant of the above test:
        // for each mimetype, check that the preferred apps are as specified
        for (ExpectedResultsMap::const_iterator it = preferredApps.constBegin(), end = preferredApps.constEnd() ; it != end ; ++it) {
            const QString mime = it.key();
            const KService::List offers = KMimeTypeTrader::self()->query(mime);
            for (int i = 0; i < offers.count(); ++i) {
                kDebug() << "offers for" << mime << ":" << i << offers[i]->storageId();
            }
            QStringList offerIds = assembleServices(offers, it.value().count());
            QCOMPARE(offerIds, it.value());
            //const QStringList expectedPreferredServices = it.value();
            //for (int i = 0; i < expectedPreferredServices.count(); ++i) {
                //kDebug() << mime << i << expectedPreferredServices[i];
            //    QCOMPARE(expectedPreferredServices[i], offers[i]->storageId());
            //}
        }
    }

    void testMultipleInheritance()
    {
        // application/x-shellscript inherits from both text/plain and application/x-executable
        KService::List offers = KMimeTypeTrader::self()->query("application/x-shellscript");
        QVERIFY(offerListHasService(offers, fakeTextApplication, true));
    }

    void testRemoveAssociationFromParent()
    {
        // I removed kate from text/plain, and it would still appear in text/x-java.

        // First, let's check our fake app is associated with text/plain
        KService::List offers = KMimeTypeTrader::self()->query("text/plain");
        QVERIFY(offerListHasService(offers, fakeTextApplication, true));

        writeToMimeApps(QByteArray("[Removed Associations]\n"
                                   "text/plain=faketextapplication.desktop;\n"));

        offers = KMimeTypeTrader::self()->query("text/plain");
        QVERIFY(!offerListHasService(offers, fakeTextApplication, false));

        offers = KMimeTypeTrader::self()->query("text/x-java");
        QVERIFY(!offerListHasService(offers, fakeTextApplication, false));
    }

    void testRemovedImplicitAssociation() // remove (implicit) assoc from derived mimetype
    {
        // #164584: Removing ark from opendocument.text didn't work
        const QString opendocument = "application/vnd.oasis.opendocument.text";
        KService::List offers = KMimeTypeTrader::self()->query(opendocument);
        QVERIFY(offerListHasService(offers, fakeArkApplication, true));

        writeToMimeApps(QByteArray("[Removed Associations]\n"
                                   "application/vnd.oasis.opendocument.text=fakearkapplication.desktop;\n"));

        offers = KMimeTypeTrader::self()->query(opendocument);
        QVERIFY(!offerListHasService(offers, fakeArkApplication, false));

        offers = KMimeTypeTrader::self()->query("application/zip");
        QVERIFY(offerListHasService(offers, fakeArkApplication, true));
    }

    void testRemovedImplicitAssociation178560()
    {
        // #178560: Removing ark from interface/x-winamp-skin didn't work
        const QString mime = "interface/x-winamp-skin";
        KService::List offers = KMimeTypeTrader::self()->query(mime);
        QVERIFY(offerListHasService(offers, fakeArkApplication, true));

        writeToMimeApps(QByteArray("[Removed Associations]\n"
                                   "interface/x-winamp-skin=fakearkapplication.desktop;\n"));

        offers = KMimeTypeTrader::self()->query(mime);
        QVERIFY(!offerListHasService(offers, fakeArkApplication, false));

        offers = KMimeTypeTrader::self()->query("application/zip");
        QVERIFY(offerListHasService(offers, fakeArkApplication, true));
    }

    // remove assoc from a mime which is both a parent and a derived mimetype
    void testRemovedMiddleAssociation()
    {
        // More tricky: x-theme inherits x-desktop inherits text/plain,
        // if we remove an association for x-desktop then x-theme shouldn't
        // get it from text/plain...

        KService::List offers;
        writeToMimeApps(QByteArray("[Removed Associations]\n"
                                   "application/x-desktop=faketextapplication.desktop;\n"));

        offers = KMimeTypeTrader::self()->query("text/plain");
        QVERIFY(offerListHasService(offers, fakeTextApplication, true));

        offers = KMimeTypeTrader::self()->query("application/x-desktop");
        QVERIFY(!offerListHasService(offers, fakeTextApplication, false));

        offers = KMimeTypeTrader::self()->query("application/x-theme");
        QVERIFY(!offerListHasService(offers, fakeTextApplication, false));
    }

private:
    typedef QMap<QString /*mimetype*/, QStringList> ExpectedResultsMap;

    void runKBuildSycoca()
    {
        //kDebug();
        // Wait for notifyDatabaseChanged DBus signal
        // (The real KCM code simply does the refresh in a slot, asynchronously)
        QEventLoop loop;
        QObject::connect(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), &loop, SLOT(quit()));
        KProcess proc;
        const QString kbuildsycoca = KStandardDirs::findExe(KBUILDSYCOCA_EXENAME);
        QVERIFY(!kbuildsycoca.isEmpty());
        proc << kbuildsycoca;
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

    static bool offersContains(const QList<KServiceOffer>& offers, KService::Ptr serv)
    {
        Q_FOREACH(const KServiceOffer& offer, offers) {
            if (offer.service()->storageId() == serv->storageId())
                return true;
        }
        return false;
    }
    static QStringList assembleOffers(const QList<KServiceOffer>& offers) {
        QStringList lst;
        Q_FOREACH(const KServiceOffer& offer, offers) {
            lst.append(offer.service()->storageId());
        }
        return lst;
    }
    static QStringList assembleServices(const QList<KService::Ptr>& services, int maxCount = -1) {
        QStringList lst;
        Q_FOREACH(const KService::Ptr& service, services) {
            lst.append(service->storageId());
            if (maxCount > -1 && lst.count() == maxCount)
                break;
        }
        return lst;
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
    QString fakeTextApplication;
    QString fakeJpegApplication;
    QString fakeArkApplication;

    ExpectedResultsMap preferredApps;
    ExpectedResultsMap removedApps;
};

QTEST_KDEMAIN_CORE( KMimeAssociationsTest )

#include "kmimeassociationstest.moc"
