#include <QString>
#include <QtTest>

#include <ThreadWeaver.h>

class LifecycleTests : public QObject
{
    Q_OBJECT
    
public:
    LifecycleTests();
    
private Q_SLOTS:
    void testShutdownOnQApplicationQuit();
};

LifecycleTests::LifecycleTests()
{
}

void LifecycleTests::testShutdownOnQApplicationQuit()
{
    {
        int argc = 0;
        QCoreApplication app(argc, (char**)0);
        ThreadWeaver::Weaver::instance()->suspend();
        ThreadWeaver::Weaver::instance()->resume();
        QTest::qWait(10);
    }
    QVERIFY(ThreadWeaver::Weaver::instance()==0);
}

QTEST_APPLESS_MAIN(LifecycleTests)

#include "LifecycleTests.moc"
