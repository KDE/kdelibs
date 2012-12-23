/* This file is part of the KDE project

   Copyright 2009 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License or
   ( at your option ) version 3 or, at the discretion of KDE e.V.
   ( which shall act as a proxy as in section 14 of the GPLv3 ), any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kcomponentdata.h>
#include <kservicegroup.h>
#include <kmimetype.h>
#include <qtest_kde.h>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdesktopfile.h>
#include <ksycoca.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <kdebug.h>
#include <kservicetype.h>
#include <kservicetypeprofile.h>

#include <QtCore/Q_PID>

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

class WorkerObject : public QObject
{
    Q_OBJECT
public:
    WorkerObject() : QObject() {}

public slots:
    void work() {
        kDebug() << QThread::currentThread() << "working...";

        const KServiceType::List allServiceTypes = KServiceType::allServiceTypes();
        Q_ASSERT(!allServiceTypes.isEmpty());

        const KMimeType::List allMimeTypes = KMimeType::allMimeTypes();
        Q_ASSERT(!allMimeTypes.isEmpty());

        const KService::List lst = KService::allServices();
        Q_ASSERT(!lst.isEmpty());

        for ( KService::List::ConstIterator it = lst.begin();
              it != lst.end(); ++it ) {
            const KService::Ptr service = (*it);
            Q_ASSERT(service->isType(KST_KService));
            const QString name = service->name();
            const QString entryPath = service->entryPath();
            //kDebug() << name << "entryPath=" << entryPath << "menuId=" << service->menuId();
            Q_ASSERT(!name.isEmpty());
            Q_ASSERT(!entryPath.isEmpty());

            KService::Ptr lookedupService = KService::serviceByDesktopPath( entryPath );
            Q_ASSERT( lookedupService ); // not null
            QCOMPARE( lookedupService->entryPath(), entryPath );

            if (service->isApplication()) {
                const QString menuId = service->menuId();
                if ( menuId.isEmpty() )
                    qWarning( "%s has an empty menuId!", qPrintable( entryPath ) );
                Q_ASSERT( !menuId.isEmpty() );
                lookedupService = KService::serviceByMenuId( menuId );
                Q_ASSERT( lookedupService ); // not null
                QCOMPARE( lookedupService->menuId(), menuId );
            }
        }

        KService::List offers = KServiceTypeTrader::self()->query("KParts/ReadOnlyPart");
        Q_ASSERT( offers.count() > 0 );
        offers = KServiceTypeTrader::self()->query("KTextEditor/Plugin");
        Q_ASSERT( offerListHasService( offers, "faketextplugin.desktop" ) );

        offers = KServiceTypeTrader::self()->query("KTextEditor/Plugin", "Library == 'faketextplugin'");
        Q_ASSERT(offers.count() == 1);
        QVERIFY( offerListHasService( offers, "faketextplugin.desktop" ) );

        KServiceGroup::Ptr root = KServiceGroup::root();
        Q_ASSERT(root);
    }
};

class WorkerThread : public QThread
{
    Q_OBJECT
public:
    WorkerThread() : QThread() {
        m_stop = false;
    }
    virtual void run() {
        WorkerObject wo;
        while (!m_stop) {
            wo.work();
        }
    }
    virtual void stop() { m_stop = true; }
private:
    bool m_stop;
};

/**
 * Threads with an event loop will be able to process "database changed" signals.
 * Threads without an event loop (like WorkerThread) cannot, so they will keep using
 * the old data.
 */
class EventLoopThread : public WorkerThread
{
    Q_OBJECT
public:
    virtual void run() {
        // WorkerObject must belong to this thread, this is why we don't
        // have the slot work() in WorkerThread itself. Typical QThread trap!
        WorkerObject wo;
        QTimer timer;
        connect(&timer, SIGNAL(timeout()), &wo, SLOT(work()));
        timer.start(100);
        exec();
    }
    virtual void stop() {
        quit();
    }
};

class KSycocaThreadTest : public QObject
{
    Q_OBJECT
public:
    KSycocaThreadTest() {}
    void launch();

private slots:
    // Note that this isn't a QTest based test.
    // All these methods are called manually.
    void cleanupTestCase();
    void testCreateService();
    void testDeleteService() {
        deleteFakeService();
        QTimer::singleShot(1000, this, SLOT(slotFinish()));
    }
    void slotFinish() {
        kDebug() << "Terminating";
        for (int i=0; i<threads.size(); i++) {
            threads[i]->stop();
        }
        for (int i=0; i<threads.size(); i++) {
            threads[i]->wait();
        }
        cleanupTestCase();
        QCoreApplication::instance()->quit();
    }

private:
    void createFakeService();
    void deleteFakeService();
    QVector<WorkerThread*> threads;
};

static void runKBuildSycoca()
{
    QProcess proc;
    const QString kbuildsycoca = KStandardDirs::findExe(KBUILDSYCOCA_EXENAME);
    QVERIFY(!kbuildsycoca.isEmpty());
    QStringList args;
    proc.setProcessChannelMode(QProcess::MergedChannels); // silence kbuildsycoca output
    proc.start(kbuildsycoca, args);
    kDebug() << "waiting for signal";
    QVERIFY(QTest::kWaitForSignal(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), 10000));
    kDebug() << "got signal";
    proc.waitForFinished();
    QCOMPARE(proc.exitStatus(), QProcess::NormalExit);
}

void KSycocaThreadTest::launch()
{
    // This service is always there. Used in the trader queries from the thread.
    const QString fakeTextPlugin = KStandardDirs::locateLocal("services", "faketextplugin.desktop");
    if (!QFile::exists(fakeTextPlugin)) {
        KDesktopFile file(fakeTextPlugin);
        KConfigGroup group = file.desktopGroup();
        group.writeEntry("Name", "FakeTextPlugin");
        group.writeEntry("Type", "Service");
        group.writeEntry("X-KDE-Library", "faketextplugin");
        group.writeEntry("X-KDE-Protocols", "http,ftp");
        group.writeEntry("ServiceTypes", "KTextEditor/Plugin");
        group.writeEntry("MimeType", "text/plain;");
        file.sync();
        qDebug() << "Created" << fakeTextPlugin << ", running kbuilsycoca";
        runKBuildSycoca();
        // Process the event
        int count = 0;
        while (!KService::serviceByDesktopPath("faketextplugin.desktop")) {
            qApp->processEvents();
            if (++count == 20) {
                qFatal("sycoca doesn't have faketextplugin.desktop");
            }
        }
    }

    // Start clean
    const QString servPath = KStandardDirs::locateLocal("services", "fakeservice.desktop");
    if (QFile::exists(servPath))
        QFile::remove(servPath);
    if (KService::serviceByDesktopPath("fakeservice.desktop"))
        deleteFakeService();
    threads.resize(5);
    for (int i=0; i<threads.size(); i++) {
        threads[i] = i < 3 ? new WorkerThread : new EventLoopThread;
        threads[i]->start();
    }
    testCreateService();
}

void KSycocaThreadTest::cleanupTestCase()
{
    const QString fakeTextPlugin = KStandardDirs::locateLocal("services", "faketextplugin.desktop");
    QFile::remove(fakeTextPlugin);
}

void KSycocaThreadTest::testCreateService()
{
    createFakeService();
    const QString servPath = KStandardDirs::locateLocal("services", "fakeservice.desktop");

    Q_ASSERT(QFile::exists(servPath));
    kDebug() << "executing kbuildsycoca (1)";
    runKBuildSycoca();

    QTimer::singleShot(1000, this, SLOT(testDeleteService()));
}

void KSycocaThreadTest::deleteFakeService()
{
    KService::Ptr fakeService = KService::serviceByDesktopPath("fakeservice.desktop");
    Q_ASSERT(fakeService);
    const QString servPath = KStandardDirs::locateLocal("services", "fakeservice.desktop");
    QFile::remove(servPath);

    QSignalSpy spy(KSycoca::self(), SIGNAL(databaseChanged(QStringList)));
    Q_ASSERT(spy.isValid());

    kDebug() << "executing kbuildsycoca (2)";
    runKBuildSycoca();
    Q_ASSERT(spy[0][0].toStringList().contains("services"));

    Q_ASSERT(fakeService); // the whole point of refcounting is that this KService instance is still valid.
    Q_ASSERT(!QFile::exists(servPath));
}

void KSycocaThreadTest::createFakeService()
{
    const QString fakeService = KStandardDirs::locateLocal("services", "fakeservice.desktop");
    KDesktopFile file(fakeService);
    KConfigGroup group = file.desktopGroup();
    group.writeEntry("Name", "FakeService");
    group.writeEntry("Type", "Service");
    group.writeEntry("X-KDE-Library", "fakeservice");
    group.writeEntry("X-KDE-Protocols", "http,ftp");
    group.writeEntry("ServiceTypes", "KTextEditor/Plugin");
    group.writeEntry("MimeType", "text/plain;");
}

int main(int argc, char** argv)
{
    setenv("KDEHOME", QFile::encodeName(QDir::homePath() + QString::fromLatin1("/.kde-unit-test")), 1);
    setenv("XDG_DATA_HOME", QFile::encodeName(QDir::homePath() + QString::fromLatin1("/.kde-unit-test/xdg/local")), 1);
    QCoreApplication app(argc, argv);
    KComponentData cData("ksycocathreadtest");
    KSycocaThreadTest mainObj;
    mainObj.launch();
    return app.exec();
}

#include "ksycocathreadtest.moc"
