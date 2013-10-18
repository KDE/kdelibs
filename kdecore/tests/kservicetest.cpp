/*
 *  Copyright (C) 2006 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <locale.h>

#include "kservicetest.h"
#include "kservicetest.moc"
#include <qtest_kde.h>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdesktopfile.h>
#include <ksycoca.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <kprotocolinfo.h>
#include <kdebug.h>
#include <kprocess.h>
#include <kservicegroup.h>
#include <kservicetypetrader.h>
#include <kservicetype.h>
#include <kservicetypeprofile.h>

#include <QtCore/Q_PID>

QTEST_KDEMAIN_CORE( KServiceTest )

static void eraseProfiles()
{
    QString profilerc = KStandardDirs::locateLocal( "config", "profilerc" );
    if ( !profilerc.isEmpty() )
        QFile::remove( profilerc );

    profilerc = KStandardDirs::locateLocal( "config", "servicetype_profilerc" );
    if ( !profilerc.isEmpty() )
        QFile::remove( profilerc );
}

void KServiceTest::initTestCase()
{
    // A non-C locale is necessary for some tests.
    // This locale must have the following properties:
    //   - some character other than dot as decimal separator
    // If it cannot be set, locale-dependent tests are skipped.
    setlocale(LC_ALL, "fr_FR.utf8");
    m_hasNonCLocale = (setlocale(LC_ALL, NULL) == QByteArray("fr_FR.utf8"));
    if (!m_hasNonCLocale) {
        kDebug() << "Setting locale to fr_FR.utf8 failed";
    }

    m_hasKde4Konsole = false;
    eraseProfiles();

    // Create some fake services for the tests below, and ensure they are in ksycoca.

    // fakeservice: deleted and recreated by testKSycocaUpdate, don't use in other tests
    bool mustUpdateKSycoca = !KService::serviceByDesktopPath("fakeservice.desktop");
    const QString fakeService = KStandardDirs::locateLocal("services", "fakeservice.desktop");
    if (!QFile::exists(fakeService)) {
        mustUpdateKSycoca = true;
        createFakeService();
    }

    // fakepart: a readwrite part, like katepart
    if (!KService::serviceByDesktopPath("fakepart.desktop")) {
        mustUpdateKSycoca = true;
    }
    const QString fakePart = KStandardDirs::locateLocal("services", "fakepart.desktop");
    if (!QFile::exists(fakePart)) {
        mustUpdateKSycoca = true;
	KDesktopFile file(fakePart);
	KConfigGroup group = file.desktopGroup();
	group.writeEntry("Name", "FakePart");
	group.writeEntry("Type", "Service");
	group.writeEntry("X-KDE-Library", "fakepart");
	group.writeEntry("X-KDE-Protocols", "http,ftp");
	group.writeEntry("X-KDE-ServiceTypes", "KParts/ReadOnlyPart,Browser/View,KParts/ReadWritePart");
	group.writeEntry("MimeType", "text/plain;text/html;");
    }

    // faketextplugin: a ktexteditor plugin
    if (!KService::serviceByDesktopPath("faketextplugin.desktop")) {
        mustUpdateKSycoca = true;
    }
    const QString fakeTextplugin = KStandardDirs::locateLocal("services", "faketextplugin.desktop");
    if (!QFile::exists(fakeTextplugin)) {
        mustUpdateKSycoca = true;
	KDesktopFile file(fakeTextplugin);
	KConfigGroup group = file.desktopGroup();
	group.writeEntry("Name", "FakeTextPlugin");
	group.writeEntry("Type", "Service");
	group.writeEntry("X-KDE-Library", "faketextplugin");
	group.writeEntry("X-KDE-ServiceTypes", "KTextEditor/Plugin");
	group.writeEntry("MimeType", "text/plain;");
    }

    if ( mustUpdateKSycoca ) {
        // Update ksycoca in ~/.kde-unit-test after creating the above
        QProcess::execute( KGlobal::dirs()->findExe(KBUILDSYCOCA_EXENAME), QStringList() << "--noincremental" );
        kDebug() << "waiting for signal";
        QVERIFY(QTest::kWaitForSignal(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), 10000));
        kDebug() << "got signal";
    }
}

void KServiceTest::cleanupTestCase()
{
    // If I want the konqueror unit tests to work, then I better not have a non-working part
    // as the preferred part for text/plain...
    QStringList services; services << "fakeservice.desktop" << "fakepart.desktop" << "faketextplugin.desktop";
    Q_FOREACH(const QString& service, services) {
        const QString fakeService = KStandardDirs::locateLocal("services", service);
        QFile::remove(fakeService);
    }
    //QProcess::execute( KGlobal::dirs()->findExe(KBUILDSYCOCA_EXENAME) );
    KProcess proc;
    proc << KStandardDirs::findExe(KBUILDSYCOCA_EXENAME);
    proc.setOutputChannelMode(KProcess::MergedChannels); // silence kbuildsycoca output
    proc.execute();
}

void KServiceTest::testByName()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    KServiceType::Ptr s0 = KServiceType::serviceType("KParts/ReadOnlyPart");
    QVERIFY( s0 );
    QCOMPARE( s0->name(), QString::fromLatin1("KParts/ReadOnlyPart") );

    KService::Ptr khtml = KService::serviceByDesktopPath("khtml.desktop");
    QCOMPARE( khtml->name(), QString::fromLatin1("KHTML"));
}

void KServiceTest::testProperty()
{
    KService::Ptr kdedkcookiejar = KService::serviceByDesktopPath("kded/kcookiejar.desktop");
    QVERIFY(kdedkcookiejar);
    QCOMPARE(kdedkcookiejar->entryPath(), QString("kded/kcookiejar.desktop"));

    QCOMPARE(kdedkcookiejar->property("ServiceTypes").toStringList().join(","), QString("KDEDModule"));
    QCOMPARE(kdedkcookiejar->property("X-KDE-Kded-autoload").toBool(), false);
    QCOMPARE(kdedkcookiejar->property("X-KDE-Kded-load-on-demand").toBool(), true);
    QVERIFY(!kdedkcookiejar->property("Name").toString().isEmpty());
    QVERIFY(!kdedkcookiejar->property("Name[fr]", QVariant::String).isValid());

    KService::Ptr kjavaappletviewer = KService::serviceByDesktopPath("kjavaappletviewer.desktop");
    QVERIFY(kjavaappletviewer);
    QCOMPARE(kjavaappletviewer->property("X-KDE-BrowserView-PluginsInfo").toString(), QString("kjava/pluginsinfo"));

    // Test property("X-KDE-Protocols"), which triggers the KServiceReadProperty code.
    KService::Ptr fakePart = KService::serviceByDesktopPath("fakepart.desktop");
    QVERIFY(fakePart); // see initTestCase; it should be found.
    QVERIFY(fakePart->propertyNames().contains("X-KDE-Protocols"));
    QCOMPARE(fakePart->mimeTypes(), QStringList() << "text/plain" << "text/html"); // okular relies on subclasses being kept here
    const QStringList protocols = fakePart->property("X-KDE-Protocols").toStringList();
    QCOMPARE(protocols, QStringList() << "http" << "ftp");
}

void KServiceTest::testAllServiceTypes()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    const KServiceType::List allServiceTypes = KServiceType::allServiceTypes();

    // A bit of checking on the allServiceTypes list itself
    KServiceType::List::ConstIterator stit = allServiceTypes.begin();
    const KServiceType::List::ConstIterator stend = allServiceTypes.end();
    for ( ; stit != stend; ++stit ) {
        const KServiceType::Ptr servtype = (*stit);
        const QString name = servtype->name();
        QVERIFY( !name.isEmpty() );
        QVERIFY( servtype->sycocaType() == KST_KServiceType );
    }
}

void KServiceTest::testAllServices()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );
    const KService::List lst = KService::allServices();
    QVERIFY( !lst.isEmpty() );

    for ( KService::List::ConstIterator it = lst.begin();
          it != lst.end(); ++it ) {
        const KService::Ptr service = (*it);
        QVERIFY( service->isType( KST_KService ) );

        const QString name = service->name();
        const QString entryPath = service->entryPath();
        //kDebug() << name << "entryPath=" << entryPath << "menuId=" << service->menuId();
        QVERIFY( !name.isEmpty() );
        QVERIFY( !entryPath.isEmpty() );

        KService::Ptr lookedupService = KService::serviceByDesktopPath( entryPath );
        QVERIFY( lookedupService ); // not null
        QCOMPARE( lookedupService->entryPath(), entryPath );

        if ( service->isApplication() )
        {
            const QString menuId = service->menuId();
            if ( menuId.isEmpty() )
                qWarning( "%s has an empty menuId!", qPrintable( entryPath ) );
            else if ( menuId == "kde4-konsole.desktop" )
                m_hasKde4Konsole = true;
            QVERIFY( !menuId.isEmpty() );
            lookedupService = KService::serviceByMenuId( menuId );
            QVERIFY( lookedupService ); // not null
            QCOMPARE( lookedupService->menuId(), menuId );
        }
    }
}

// Helper method for all the trader tests
static bool offerListHasService( const KService::List& offers,
                                 const QString& entryPath )
{
    bool found = false;
    KService::List::const_iterator it = offers.begin();
    for ( ; it != offers.end() ; ++it )
    {
        if ( (*it)->entryPath() == entryPath ) {
            if( found ) { // should be there only once
                qWarning( "ERROR: %s was found twice in the list", qPrintable( entryPath ) );
                return false; // make test fail
            }
            found = true;
        }
    }
    return found;
}

void KServiceTest::testDBUSStartupType()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );
    if ( !m_hasKde4Konsole )
        QSKIP( "kde4-konsole.desktop not available", SkipAll );
    //KService::Ptr konsole = KService::serviceByMenuId( "kde4-konsole.desktop" );
    KService::Ptr konsole = KService::serviceByDesktopName( "konsole" );
    QVERIFY(konsole);
    QCOMPARE(konsole->menuId(), QString("kde4-konsole.desktop"));
    //qDebug() << konsole->entryPath();
    QCOMPARE((int)konsole->dbusStartupType(), (int)KService::DBusUnique);
}

void KServiceTest::testByStorageId()
{
    if ( !KSycoca::isAvailable() )
        QSKIP("ksycoca not available", SkipAll);
    if (KGlobal::dirs()->locate("xdgdata-apps", "kde4/kmailservice.desktop").isEmpty()) {
        QSKIP("kde4/kmailservice.desktop not available", SkipAll);
    }
    QVERIFY(KService::serviceByMenuId("kde4-kmailservice.desktop"));
    QVERIFY(!KService::serviceByMenuId("kde4-kmailservice")); // doesn't work, extension mandatory
    QVERIFY(KService::serviceByStorageId("kde4-kmailservice.desktop"));
    //QVERIFY(!KService::serviceByStorageId("kde4-kmailservice")); // doesn't work, extension mandatory; also shows a debug

    // This one fails here; probably because there are two such files, so this would be too
    // ambiguous... According to the testAllServices output, the entryPaths are
    // entryPath="/d/kde/inst/kde4/share/applications/kde4/kmailservice.desktop"
    // entryPath= "/usr/share/applications/kde4/kmailservice.desktop"
    //
    //QVERIFY(KService::serviceByDesktopPath("kmailservice.desktop"));

    QVERIFY(KService::serviceByDesktopName("kmailservice"));
    // This could fail if it finds the kde3 kmailservice from /usr/share. But who still has kde3 :-)
    QCOMPARE(KService::serviceByDesktopName("kmailservice")->menuId(), QString("kde4-kmailservice.desktop"));
}

void KServiceTest::testServiceTypeTraderForReadOnlyPart()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    // Querying trader for services associated with KParts/ReadOnlyPart
    KService::List offers = KServiceTypeTrader::self()->query("KParts/ReadOnlyPart");
    QVERIFY( offers.count() > 0 );
    //foreach( KService::Ptr service, offers )
    //    qDebug( "%s %s", qPrintable( service->name() ), qPrintable( service->entryPath() ) );

    m_firstOffer = offers[0]->entryPath();

    // Only test for parts provided by kdelibs, or better, by this unittest:
    QVERIFY( offerListHasService( offers, "fakepart.desktop" ) );

    QVERIFY( offerListHasService( offers, "kmultipart.desktop" ) );
    QVERIFY( offerListHasService( offers, "khtml.desktop" ) );
    QVERIFY( offerListHasService( offers, "khtmlimage.desktop" ) );
    QVERIFY( offerListHasService( offers, "kjavaappletviewer.desktop" ) );

    // Check ordering according to InitialPreference
    int lastPreference = -1;
    bool lastAllowedAsDefault = true;
    Q_FOREACH(KService::Ptr service, offers) {
        const QString path = service->entryPath();
        const int preference = service->initialPreference(); // ## might be wrong if we use per-servicetype preferences...
        //qDebug( "%s has preference %d, allowAsDefault=%d", qPrintable( path ), preference, service->allowAsDefault() );
        if ( lastAllowedAsDefault && !service->allowAsDefault() ) {
            // first "not allowed as default" offer
            lastAllowedAsDefault = false;
            lastPreference = -1; // restart
        }
        if ( lastPreference != -1 )
            QVERIFY( preference <= lastPreference );
        lastPreference = preference;
    }

    // Now look for any KTextEditor/Plugin
    offers = KServiceTypeTrader::self()->query("KTextEditor/Plugin");
    QVERIFY( offerListHasService( offers, "fakeservice.desktop" ) );
    QVERIFY( offerListHasService( offers, "faketextplugin.desktop" ) );
}

void KServiceTest::testTraderConstraints()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    KService::List offers = KServiceTypeTrader::self()->query("KTextEditor/Plugin", "Library == 'faketextplugin'");
    QCOMPARE(offers.count(), 1);
    QVERIFY( offerListHasService( offers, "faketextplugin.desktop" ) );

    if (m_hasNonCLocale) {
        // Test float parsing, must use dot as decimal separator independent of locale.
        offers = KServiceTypeTrader::self()->query("KTextEditor/Plugin", "([X-KDE-Version] > 4.559) and ([X-KDE-Version] < 4.561)");
        QCOMPARE(offers.count(), 1);
        QVERIFY(offerListHasService( offers, "fakeservice.desktop"));
    }

    // A test with an invalid query, to test for memleaks
    offers = KServiceTypeTrader::self()->query("KTextEditor/Plugin", "A == B OR C == D AND OR Foo == 'Parse Error'");
    QVERIFY(offers.isEmpty());
}

void KServiceTest::testHasServiceType1() // with services constructed with a full path (rare)
{
    QString fakepartPath = KStandardDirs::locate( "services", "fakepart.desktop" );
    QVERIFY( !fakepartPath.isEmpty() );
    KService fakepart( fakepartPath );
    QVERIFY( fakepart.hasServiceType( "KParts/ReadOnlyPart" ) );
    QVERIFY( fakepart.hasServiceType( "KParts/ReadWritePart" ) );
    QCOMPARE(fakepart.mimeTypes(), QStringList() << "text/plain" << "text/html");

    QString faketextPluginPath = KStandardDirs::locate( "services", "faketextplugin.desktop" );
    QVERIFY( !faketextPluginPath.isEmpty() );
    KService faketextPlugin( faketextPluginPath );
    QVERIFY( faketextPlugin.hasServiceType( "KTextEditor/Plugin" ) );
    QVERIFY( !faketextPlugin.hasServiceType( "KParts/ReadOnlyPart" ) );
}

void KServiceTest::testHasServiceType2() // with services coming from ksycoca
{
    KService::Ptr fakepart = KService::serviceByDesktopPath( "fakepart.desktop" );
    QVERIFY( !fakepart.isNull() );
    QVERIFY( fakepart->hasServiceType( "KParts/ReadOnlyPart" ) );
    QVERIFY( fakepart->hasServiceType( "KParts/ReadWritePart" ) );
    QCOMPARE(fakepart->mimeTypes(), QStringList() << "text/plain" << "text/html");

    KService::Ptr faketextPlugin = KService::serviceByDesktopPath( "faketextplugin.desktop" );
    QVERIFY( !faketextPlugin.isNull() );
    QVERIFY( faketextPlugin->hasServiceType( "KTextEditor/Plugin" ) );
    QVERIFY( !faketextPlugin->hasServiceType( "KParts/ReadOnlyPart" ) );
}

void KServiceTest::testWriteServiceTypeProfile()
{
    const QString serviceType = "KParts/ReadOnlyPart";
    KService::List services, disabledServices;
    services.append(KService::serviceByDesktopPath("khtmlimage.desktop"));
    services.append(KService::serviceByDesktopPath("fakepart.desktop"));
    disabledServices.append(KService::serviceByDesktopPath("khtml.desktop"));

    KService::List::ConstIterator servit = services.constBegin();
    for( ; servit != services.constEnd(); ++servit) {
        QVERIFY(!servit->isNull());
    }


    KServiceTypeProfile::writeServiceTypeProfile( serviceType, services, disabledServices );

    // Check that the file got written
    QString profilerc = KStandardDirs::locateLocal( "config", "servicetype_profilerc" );
    QVERIFY(!profilerc.isEmpty());
    QVERIFY(QFile::exists(profilerc));

    KService::List offers = KServiceTypeTrader::self()->query( serviceType );
    QVERIFY( offers.count() > 0 ); // not empty

    //foreach( KService::Ptr service, offers )
    //    qDebug( "%s %s", qPrintable( service->name() ), qPrintable( service->entryPath() ) );

    QVERIFY( offers.count() >= 3 ); // at least 3, even
    QCOMPARE( offers[0]->entryPath(), QString("khtmlimage.desktop") );
    QCOMPARE( offers[1]->entryPath(), QString("fakepart.desktop") );
    QVERIFY( offerListHasService( offers, "kmultipart.desktop" ) ); // should still be somewhere in there
    QVERIFY( !offerListHasService( offers, "khtml.desktop" ) ); // it got disabled above
}

void KServiceTest::testDefaultOffers()
{
    // Now that we have a user-profile, let's see if defaultOffers indeed gives us the default ordering.
    const QString serviceType = "KParts/ReadOnlyPart";
    KService::List offers = KServiceTypeTrader::self()->defaultOffers( serviceType );
    QVERIFY( offers.count() > 0 ); // not empty
    QVERIFY( offerListHasService( offers, "khtml.desktop" ) ); // it's here even though it's disabled in the profile
    if ( m_firstOffer.isEmpty() )
        QSKIP( "testServiceTypeTraderForReadOnlyPart not run", SkipAll );
    QCOMPARE( offers[0]->entryPath(), m_firstOffer );
}

void KServiceTest::testDeleteServiceTypeProfile()
{
    const QString serviceType = "KParts/ReadOnlyPart";
    KServiceTypeProfile::deleteServiceTypeProfile( serviceType );

    KService::List offers = KServiceTypeTrader::self()->query( serviceType );
    QVERIFY( offers.count() > 0 ); // not empty
    QVERIFY( offerListHasService( offers, "khtml.desktop" ) ); // it's back

    if ( m_firstOffer.isEmpty() )
        QSKIP( "testServiceTypeTraderForReadOnlyPart not run", SkipAll );
    QCOMPARE( offers[0]->entryPath(), m_firstOffer );
}

void KServiceTest::testActionsAndDataStream()
{
    const QString servicePath = KStandardDirs::locate( "services", "ScreenSavers/krandom.desktop" );
    if (servicePath.isEmpty() )
        QSKIP("kdebase not installed, krandom.desktop not found", SkipAll);
    KService service( servicePath );
    QVERIFY(!service.property("Name[fr]", QVariant::String).isValid());
    const QList<KServiceAction> actions = service.actions();
    QCOMPARE(actions.count(), 3);
    const KServiceAction setupAction = actions[0];
    QCOMPARE(setupAction.name(), QString("Setup"));
    QCOMPARE(setupAction.exec(), QString("krandom.kss -setup"));
    QVERIFY(!setupAction.icon().isEmpty());
    QCOMPARE(setupAction.noDisplay(), false);
    QVERIFY(!setupAction.isSeparator());
    const KServiceAction rootAction = actions[2];
    QCOMPARE(rootAction.name(), QString("Root"));

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    service.save(stream);
    QVERIFY(!data.isEmpty());
    // The binary size of that KService in ksycoca was 3700 when storing all Name[...] translations!
    // Now down to 755. This is on x86, so make the assert for 1500 in case x86_64 needs more.
    QVERIFY(data.size() < 1500);
    QDataStream loadingStream(data);
    // loading must first get type, see KSycocaEntryPrivate::save
    // (the path that save writes out, is read by the KSycocaEntryPrivate ctor)
    qint32 type;
    loadingStream >> type;
    KService loadedService(loadingStream, 0);
    QCOMPARE(loadedService.name(), service.name());
    QCOMPARE(loadedService.exec(), service.exec());
    QCOMPARE(loadedService.actions().count(), 3);
}

void KServiceTest::testServiceGroups()
{
    KServiceGroup::Ptr root = KServiceGroup::root();
    QVERIFY(root);
    qDebug() << root->groupEntries().count();

    KServiceGroup::Ptr group = root;
    QVERIFY(group);
    const KServiceGroup::List list = group->entries(true /* sorted */,
                                                   true /* exclude no display entries */,
                                                   false /* allow separators */,
                                                   true /* sort by generic name */);

    kDebug() << list.count();
    Q_FOREACH(KServiceGroup::SPtr s, list) {
        qDebug() << s->name() << s->entryPath();
    }

    // No unit test here yet, but at least this can be valgrinded for errors.
}

void KServiceTest::testKSycocaUpdate()
{
    kWarning();
    KService::Ptr fakeService = KService::serviceByDesktopPath("fakeservice.desktop");
    QVERIFY(fakeService); // see initTestCase; it should be found.

    // Test deleting a service
    const QString servPath = KStandardDirs::locateLocal("services", "fakeservice.desktop");
    QVERIFY(QFile::exists(servPath));
    QSignalSpy spy(KSycoca::self(), SIGNAL(databaseChanged(QStringList)));
    QVERIFY(spy.isValid());
    QFile::remove(servPath);
    kDebug() << QThread::currentThread() << "executing kbuildsycoca";
    QProcess::execute( KGlobal::dirs()->findExe(KBUILDSYCOCA_EXENAME) );
    kDebug() << QThread::currentThread() << "done";
    while (spy.isEmpty())
        QTest::qWait(50);
    QVERIFY(!spy.isEmpty());
    QVERIFY(!KService::serviceByDesktopPath("fakeservice.desktop")); // not in ksycoca anymore
    QVERIFY(spy[0][0].toStringList().contains("services"));
    kDebug() << QThread::currentThread() << "got signal ok";

    spy.clear();
    QVERIFY(fakeService); // the whole point of refcounting is that this KService instance is still valid.
    QVERIFY(!QFile::exists(servPath));

    // Recreate it, for future tests
    createFakeService();
    QVERIFY(QFile::exists(servPath));
    kDebug() << QThread::currentThread() << "executing kbuildsycoca (2)";
    QProcess::execute( KGlobal::dirs()->findExe(KBUILDSYCOCA_EXENAME) );
    kDebug() << QThread::currentThread() << "done (2)";
    while (spy.isEmpty())
        QTest::qWait(50);
    kDebug() << QThread::currentThread() << "got signal ok (2)";
    QVERIFY(spy[0][0].toStringList().contains("services"));
    if (QThread::currentThread() != QCoreApplication::instance()->thread())
        m_sycocaUpdateDone.ref();
}

void KServiceTest::createFakeService()
{
    const QString fakeService = KStandardDirs::locateLocal("services", "fakeservice.desktop");
    KDesktopFile file(fakeService);
    KConfigGroup group = file.desktopGroup();
    group.writeEntry("Name", "FakePlugin");
    group.writeEntry("Type", "Service");
    group.writeEntry("X-KDE-Library", "fakeservice");
    group.writeEntry("X-KDE-Version", "4.56");
    group.writeEntry("ServiceTypes", "KTextEditor/Plugin");
    group.writeEntry("MimeType", "text/plain;");
}

#include <QThreadPool>
#include <qtconcurrentrun.h>

// Testing for concurrent access to ksycoca from multiple threads
// It's especially interesting to run this test as ./kservicetest testThreads
// so that even the ksycoca initialization is happening from N threads at the same time.
// Use valgrind --tool=helgrind to see the race conditions.

void KServiceTest::testReaderThreads()
{
    QThreadPool::globalInstance()->setMaxThreadCount(10);
    QList<QFuture<void> > futures;
    futures << QtConcurrent::run(this, &KServiceTest::testAllServices);
    futures << QtConcurrent::run(this, &KServiceTest::testAllServices);
    futures << QtConcurrent::run(this, &KServiceTest::testAllServices);
    futures << QtConcurrent::run(this, &KServiceTest::testHasServiceType1);
    futures << QtConcurrent::run(this, &KServiceTest::testAllServices);
    futures << QtConcurrent::run(this, &KServiceTest::testAllServices);
    Q_FOREACH(QFuture<void> f, futures) // krazy:exclude=foreach
        f.waitForFinished();
    QThreadPool::globalInstance()->setMaxThreadCount(1); // delete those threads
}

void KServiceTest::testThreads()
{
    QThreadPool::globalInstance()->setMaxThreadCount(10);
    QList<QFuture<void> > futures;
    futures << QtConcurrent::run(this, &KServiceTest::testAllServices);
    futures << QtConcurrent::run(this, &KServiceTest::testHasServiceType1);
    futures << QtConcurrent::run(this, &KServiceTest::testKSycocaUpdate);
    futures << QtConcurrent::run(this, &KServiceTest::testTraderConstraints);
    while (m_sycocaUpdateDone == 0) // not using a bool, just to silence helgrind
        QTest::qWait(100); // process D-Bus events!
    kDebug() << "Joining all threads";
    Q_FOREACH(QFuture<void> f, futures) // krazy:exclude=foreach
        f.waitForFinished();
}
