/* This file is part of the KDE libraries
    Copyright (c) 2008 David Faure <faure@kde.org>

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

class KMimeAssociationsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        m_kdehome = QDir::home().canonicalPath() + "/.kde-unit-test";
        // We need a place where we can hack a mimeapps.list without harm, so not ~/.local
        // This test relies on shared-mime-info being installed in /usr/share [or kdedir/share]
        ::setenv("XDG_DATA_DIRS", QFile::encodeName(m_kdehome + "/share:/usr/share"), 1 );

        cleanupTestCase();

        if ( !KSycoca::isAvailable() ) {
            // Create ksycoca4 in ~/.kde-unit-test, to get kservice ptrs.
            QProcess::execute( KGlobal::dirs()->findExe(KBUILDSYCOCA_EXENAME), QStringList() << "--noincremental" );
        }
        // Create factory on the heap and don't delete it.
        // It registers to KSycoca, which deletes it at end of program execution.
        KServiceFactory* factory = new FakeServiceFactory;
        QCOMPARE(KServiceFactory::self(), factory); // ctor sets s_self

        // For debugging: print all services and their storageId
#if 0
        const KService::List lst = KService::allServices();
        QVERIFY( !lst.isEmpty() );
        Q_FOREACH(const KService::Ptr& serv, lst) {
            qDebug() << serv->entryPath() << serv->storageId() /*<< serv->desktopEntryName()*/;
        }
#endif

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
        QFile::remove(m_kdehome + "/share/applications/mimeapps.list");
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
        QString mimeAppsPath = m_kdehome + "/share/applications/mimeapps.list";
        QFile mimeAppsFile(mimeAppsPath);
        QVERIFY(mimeAppsFile.open(QIODevice::WriteOnly));
        mimeAppsFile.write(m_mimeAppsFileContents);
        mimeAppsFile.close();

        // OK, now run kbuildsycoca4.
        QProcess::execute( KGlobal::dirs()->findExe(KBUILDSYCOCA_EXENAME) );

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
            for (int i = 0; i < offers.count(); ++i) {
                kDebug() << "offers for" << mime << ":" << i << offers[i]->storageId();
            }
            const QStringList expectedPreferredServices = it.value();
            for (int i = 0; i < expectedPreferredServices.count(); ++i) {
                kDebug() << mime << i << expectedPreferredServices[i];
                QCOMPARE(expectedPreferredServices[i], offers[i]->storageId());
            }
        }
    }


private:
    typedef QMap<QString /*mimetype*/, QStringList> ExpectedResultsMap;

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
    QString m_kdehome;
    QByteArray m_mimeAppsFileContents;

    ExpectedResultsMap preferredApps;
    ExpectedResultsMap removedApps;
};

QTEST_KDEMAIN_CORE( KMimeAssociationsTest )

#include "kmimeassociationstest.moc"
