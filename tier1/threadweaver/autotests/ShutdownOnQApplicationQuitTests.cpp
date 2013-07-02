#include <QString>
#include <QtTest>

#include <ThreadWeaver.h>

// The tests checks if the global ThreadWeaver instance is properly
// destroyed along with QCoreApplication. After that all the puppies
// are sad and the kittens cry, and the test exits. 
class ShutdownOnQApplicationQuitTests : public QObject
{
    Q_OBJECT
    
public:
    ShutdownOnQApplicationQuitTests();
    
private Q_SLOTS:
    void testShutdownOnQApplicationQuit();
};

ShutdownOnQApplicationQuitTests::ShutdownOnQApplicationQuitTests()
{
}

void ShutdownOnQApplicationQuitTests::testShutdownOnQApplicationQuit()
{
    {
        int argc = 0;
        QCoreApplication app(argc, (char**)0);
        QVERIFY(ThreadWeaver::Weaver::instance()!=0);
        ThreadWeaver::Weaver::instance()->suspend();
        ThreadWeaver::Weaver::instance()->resume();
        QTest::qWait(10);
    }
    QVERIFY(ThreadWeaver::Weaver::instance()==0);
}

QTEST_APPLESS_MAIN(ShutdownOnQApplicationQuitTests)

#include "ShutdownOnQApplicationQuitTests.moc"
