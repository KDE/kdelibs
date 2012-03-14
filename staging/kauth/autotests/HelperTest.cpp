#include <QtTest>
#include <kauth.h>
#include <kauthactionreply.h>
#include <kauthexecutejob.h>
#include "BackendsManager.h"
#include "TestHelper.h"

#include "../src/backends/dbus/DBusHelperProxy.h"

class HelperHandler : public QObject
{
    Q_OBJECT
public:
    HelperHandler();

private Q_SLOTS:
    void init();

Q_SIGNALS:
    void ready();

private:
    DBusHelperProxy *m_helperProxy;
    TestHelper *m_helper;
    QThread *m_thread;
};

class HelperTest : public QObject
{
    Q_OBJECT

public:
    HelperTest(QObject *parent = 0) : QObject(parent)
    { }

private Q_SLOTS:
    void initTestCase();
    void init() {}

    void testBasicActionExecution();
    void testExecuteJobSignals();
    void testStartTwice();

    void cleanup() {}
    void cleanupTestCase() {}

Q_SIGNALS:
    void changeCapabilities(KAuth::AuthBackend::Capabilities capabilities);

private:
    HelperHandler *m_handler;
};

HelperHandler::HelperHandler()
    : QObject(0)
{
    /* Hello adventurer. What you see here might hurt your eyes, but let me explain why you don't want
       to touch this code. We are dealing with same-process async DBus requests, and if this seems obscure
       to you already please give up and forget about this function.
       Qt's local loop optimizations at the moment make it impossible to stream an async request to a process
       living on the same thread. So that's what we do: we instantiate a separate helperProxy and move it to
       a different thread - afterwards we can do everything as if we were in a separate process.

       If you are wondering if this means we'll have two helper proxies, you are right my friend. But please
       remember that helperProxy acts both as a client and as a server, so it makes total sense.

       tl;dr: Don't touch this and forget the weird questions in your head: it actually works.
    */

    m_thread = new QThread(this);
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), this, SLOT(init()));
    m_thread->start();
}

void HelperHandler::init()
{
    qDebug() << "Initializing helper handler";
    // Set up our Helper - of course, it is not in a separate process here so we need to copy what
    // HelperProxy::helperMain() does
    m_helperProxy = new DBusHelperProxy(QDBusConnection::sessionBus());
    m_helper = new TestHelper;
    // The timer is here just to prevent the app from crashing.
    QTimer *timer = new QTimer(0);

    QVERIFY(m_helperProxy->initHelper(QLatin1String("org.kde.auth.autotest")));

    m_helperProxy->setHelperResponder(m_helper);

    m_helper->setProperty("__KAuth_Helper_Shutdown_Timer", QVariant::fromValue(timer));
    timer->setInterval(10000);
    timer->start();

    // Make BackendsManager aware
    BackendsManager::setProxyForThread(m_thread, m_helperProxy);

    Q_EMIT ready();
}

void HelperTest::initTestCase()
{
    connect(this, SIGNAL(changeCapabilities(KAuth::AuthBackend::Capabilities)),
            KAuth::BackendsManager::authBackend(), SLOT(setNewCapabilities(KAuth::AuthBackend::Capabilities)));

    qRegisterMetaType<KAuth::Action::AuthStatus>();
    qRegisterMetaType<KAuth::ExecuteJob*>();

    // Set up our HelperHandler
    m_handler = new HelperHandler;
    QEventLoop e;
    connect(m_handler, SIGNAL(ready()), &e, SLOT(quit()));
    qDebug() << "Waiting for HelperHandler to be initialized";
    e.exec();
}

void HelperTest::testBasicActionExecution()
{
    Q_EMIT changeCapabilities(KAuth::AuthBackend::AuthorizeFromHelperCapability | KAuth::AuthBackend::CheckActionExistenceCapability);

    KAuth::Action action(QLatin1String("org.kde.auth.autotest.standardaction"));
    action.setHelperID(QLatin1String("org.kde.auth.autotest"));
    QVERIFY(action.isValid());

    QCOMPARE(action.status(), KAuth::Action::StatusAuthRequired);
    KAuth::ExecuteJob *job = action.execute();

    job->exec();

    QVERIFY(job->succeeded());
    QVERIFY(job->data().isEmpty());
}

void HelperTest::testExecuteJobSignals()
{
    KAuth::Action action(QLatin1String("org.kde.auth.autotest.longaction"));
    action.setHelperID(QLatin1String("org.kde.auth.autotest"));
    QVERIFY(action.isValid());

    QCOMPARE(action.status(), KAuth::Action::StatusAuthRequired);

    KAuth::ExecuteJob *job = action.execute();

    QSignalSpy finishedSpy(job, SIGNAL(finished(KAuth::ExecuteJob*)));
    QSignalSpy newDataSpy(job, SIGNAL(newData(QVariantMap)));
    QSignalSpy percentSpy(job, SIGNAL(percent(uint)));
    QSignalSpy statusChangedSpy(job, SIGNAL(statusChanged(KAuth::Action::AuthStatus)));

    job->exec();

    QCOMPARE(finishedSpy.size(), 1);
    QCOMPARE(finishedSpy.first().first().value<KAuth::ExecuteJob*>(), job);
    QCOMPARE(statusChangedSpy.size(), 1);
    QCOMPARE(statusChangedSpy.first().first().value<KAuth::Action::AuthStatus>(), KAuth::Action::StatusAuthorized);
    QCOMPARE(percentSpy.size(), 100);
    for (uint i = 1; i <= 100; ++i) {
        QCOMPARE(percentSpy.at(i-1).first().toUInt(), i);
    }
    QCOMPARE(newDataSpy.size(), 1);
    QCOMPARE(newDataSpy.first().first().value<QVariantMap>().value(QLatin1String("Answer")).toInt(), 42);

    QVERIFY(job->succeeded());
    QVERIFY(job->data().isEmpty());
}

void HelperTest::testStartTwice()
{
    KAuth::Action action(QLatin1String("org.kde.auth.autotest.longaction"));
    action.setHelperID(QLatin1String("org.kde.auth.autotest"));
    QVERIFY(action.isValid());

    QCOMPARE(action.status(), KAuth::Action::StatusAuthRequired);

    KAuth::ExecuteJob *job = action.execute();

    job->start();
    job->start();

    QEventLoop e;
    connect(job, SIGNAL(finished(KAuth::ExecuteJob*)), &e, SLOT(quit()));
    e.exec();

    QVERIFY(!job->succeeded());
    QCOMPARE(job->error(), KAuth::ActionReply::AlreadyStartedError);

    e.exec();

    QVERIFY(job->succeeded());
    QVERIFY(job->data().isEmpty());
}

Q_DECLARE_METATYPE(KAuth::ExecuteJob*)
Q_DECLARE_METATYPE(KAuth::Action::AuthStatus)
Q_DECLARE_METATYPE(QTimer*)
QTEST_MAIN(HelperTest)
#include "HelperTest.moc"
