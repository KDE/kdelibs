/*
*   Copyright (C) 2012 Dario Freddi <drf@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation; either version 2.1 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
*/

#include <QtTest>
#include <kauth.h>
#include <kauthactionreply.h>
#include <kauthexecutejob.h>
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
    void testUserAuthorization();
    void testAuthorizationFail();

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
    emit changeCapabilities(KAuth::AuthBackend::AuthorizeFromHelperCapability | KAuth::AuthBackend::CheckActionExistenceCapability);
    KAuth::Action action(QLatin1String("i.do.not.exist"));
    QVERIFY(!action.isValid());

    action = KAuth::Action(QLatin1String("/safinvalid124%$&"));
    QVERIFY(action.isValid());

    // Now with regexp check
    emit changeCapabilities(0);

    action = KAuth::Action(QLatin1String("/safinvalid124%$&"));
    QVERIFY(!action.isValid());
}

void SetupActionTest::testBasicActionProperties()
{
    emit changeCapabilities(KAuth::AuthBackend::AuthorizeFromHelperCapability | KAuth::AuthBackend::CheckActionExistenceCapability);
    KAuth::Action action(QLatin1String("always.authorized"), QLatin1String("details"));
    QVERIFY(action.isValid());

    QCOMPARE(action.name(), QLatin1String("always.authorized"));
    QCOMPARE(action.details(), QLatin1String("details"));
    QVERIFY(!action.hasHelper());
    QVERIFY(action.helperId().isEmpty());
    QCOMPARE(action.status(), KAuth::Action::AuthorizedStatus);

    QVERIFY(action.arguments().isEmpty());
    QVariantMap args;
    args.insert(QLatin1String("akey"), QVariant::fromValue(42));
    action.setArguments(args);
    QCOMPARE(action.arguments(), args);

    action.setName(QLatin1String("i.do.not.exist"));
    QVERIFY(!action.isValid());

    emit changeCapabilities(0);

    action = KAuth::Action(QLatin1String("i.do.not.exist"), QLatin1String("details"));

    QVERIFY(action.isValid());
    QCOMPARE(action.name(), QLatin1String("i.do.not.exist"));
    QCOMPARE(action.details(), QLatin1String("details"));
    QVERIFY(!action.hasHelper());
    QVERIFY(action.helperId().isEmpty());
    QCOMPARE(action.status(), KAuth::Action::InvalidStatus);
}

void SetupActionTest::testUserAuthorization()
{
    emit changeCapabilities(KAuth::AuthBackend::CheckActionExistenceCapability);

    KAuth::Action action(QLatin1String("requires.auth"), QLatin1String("details"));
    QVERIFY(action.isValid());

    QCOMPARE(action.status(), KAuth::Action::AuthRequiredStatus);
    KAuth::ExecuteJob *job = action.execute();

    QVERIFY(!job->exec());

    QCOMPARE(job->error(), (int)KAuth::ActionReply::BackendError);

    emit changeCapabilities(KAuth::AuthBackend::CheckActionExistenceCapability | KAuth::AuthBackend::AuthorizeFromClientCapability);

    QVERIFY(action.isValid());

    QCOMPARE(action.status(), KAuth::Action::AuthRequiredStatus);
    job = action.execute();

    QVERIFY(job->exec());

    QVERIFY(!job->error());
    QVERIFY(job->data().isEmpty());
}

void SetupActionTest::testAuthorizationFail()
{
    emit changeCapabilities(KAuth::AuthBackend::CheckActionExistenceCapability | KAuth::AuthBackend::AuthorizeFromClientCapability);

    KAuth::Action action(QLatin1String("doomed.to.fail"), QLatin1String("details"));
    QVERIFY(action.isValid());

    QCOMPARE(action.status(), KAuth::Action::DeniedStatus);
    KAuth::ExecuteJob *job = action.execute();

    QVERIFY(!job->exec());

    QCOMPARE(job->error(), (int)KAuth::ActionReply::AuthorizationDeniedError);
    QVERIFY(job->data().isEmpty());
}

QTEST_MAIN(SetupActionTest)
#include "SetupActionTest.moc"
