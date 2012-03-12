#include <QtTest>
#include <kauth.h>
#include "BackendsManager.h"

class SetupActionTest : public QObject
{
    Q_OBJECT

public:
    SetupActionTest(QObject *parent = 0)
        : QObject(parent)
    { }

private Q_SLOTS:
    void initTestCase();
    void init() {}

    void testNonExistentAction();
    void testBasicActionProperties();

    void cleanup() {}
    void cleanupTestCase() {}

Q_SIGNALS:
    void changeCapabilities(KAuth::AuthBackend::Capabilities capabilities);

private:
};

void SetupActionTest::initTestCase()
{
    connect(this, SIGNAL(changeCapabilities(KAuth::AuthBackend::Capabilities)),
            KAuth::BackendsManager::authBackend(), SLOT(setNewCapabilities(KAuth::AuthBackend::Capabilities)));
}

void SetupActionTest::testNonExistentAction()
{
    Q_EMIT changeCapabilities(KAuth::AuthBackend::AuthorizeFromHelperCapability | KAuth::AuthBackend::CheckActionExistenceCapability);
    KAuth::Action action(QLatin1String("i.do.not.exist"));
    QVERIFY(!action.isValid());

    action = KAuth::Action(QLatin1String("/safinvalid124%$&"));
    QVERIFY(action.isValid());

    // Now with regexp check
    Q_EMIT changeCapabilities(0);

    action = KAuth::Action(QLatin1String("/safinvalid124%$&"));
    QVERIFY(!action.isValid());
}

void SetupActionTest::testBasicActionProperties()
{
    Q_EMIT changeCapabilities(KAuth::AuthBackend::AuthorizeFromHelperCapability | KAuth::AuthBackend::CheckActionExistenceCapability);
    KAuth::Action action(QLatin1String("always.authorized"), QLatin1String("details"));
    QVERIFY(action.isValid());

    QCOMPARE(action.name(), QLatin1String("always.authorized"));
    QCOMPARE(action.details(), QLatin1String("details"));
    QVERIFY(!action.hasHelper());
    QVERIFY(action.helperID().isEmpty());
    QCOMPARE(action.status(), KAuth::Action::StatusAuthorized);

    QVERIFY(action.arguments().isEmpty());
    QVariantMap args;
    args.insert(QLatin1String("akey"), QVariant::fromValue(42));
    action.setArguments(args);
    QCOMPARE(action.arguments(), args);

    action.setName(QLatin1String("i.do.not.exist"));
    QVERIFY(!action.isValid());

    Q_EMIT changeCapabilities(0);

    action = KAuth::Action(QLatin1String("i.do.not.exist"), QLatin1String("details"));

    QVERIFY(action.isValid());
    QCOMPARE(action.name(), QLatin1String("i.do.not.exist"));
    QCOMPARE(action.details(), QLatin1String("details"));
    QVERIFY(!action.hasHelper());
    QVERIFY(action.helperID().isEmpty());
    QCOMPARE(action.status(), KAuth::Action::StatusInvalid);
}

QTEST_MAIN(SetupActionTest)
#include "SetupActionTest.moc"
