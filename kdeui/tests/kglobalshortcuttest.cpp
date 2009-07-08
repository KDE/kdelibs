/*
    This file is part of the KDE libraries

    Copyright (c) 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    Copyright (c) 2008 Michael Jansen <kde@michael-jansen.biz>

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

#include "kglobalshortcuttest.h"
#include <qdbusinterface.h>
#include <qtest_kde.h>
#include <kaction.h>
#include <kaction_p.h>
#include <kactioncollection.h>
#include <kglobal.h>
#include <kglobalaccel.h>
#include <kdebug.h>

#include <unistd.h>

#include <QtDBus/QDBusConnectionInterface>

const QKeySequence sequenceA = QKeySequence(Qt::SHIFT + Qt::META + Qt::CTRL + Qt::ALT + Qt::Key_F28 );
const QKeySequence sequenceB = QKeySequence(Qt::Key_F29);
const QKeySequence sequenceC = QKeySequence(Qt::SHIFT + Qt::META + Qt::CTRL + Qt::Key_F28 );
const QKeySequence sequenceD = QKeySequence(Qt::META + Qt::ALT + Qt::Key_F30);
const QKeySequence sequenceE = QKeySequence(Qt::META + Qt::Key_F29);
const QKeySequence sequenceF = QKeySequence(Qt::META + Qt::Key_F27);

/* These tests could be better. They don't include actually triggering actions,
   and we just choose very improbable shortcuts to avoid conflicts with real
   applications' shortcuts. */

//we need a KComponentData and a GUI so that the implementation can grab keys
QTEST_KDEMAIN( KGlobalShortcutTest, GUI )

void KGlobalShortcutTest::setupTest(QString id)
{
    if (m_actionA) {
        m_actionA->forgetGlobalShortcut();
        delete m_actionA;
    }

    if (m_actionB) {
        m_actionB->forgetGlobalShortcut();
        delete m_actionB;
    }

    // Ensure that the previous test did cleanup correctly
    KGlobalAccel *kga = KGlobalAccel::self();
    QList<QStringList> components = kga->allMainComponents();
    QStringList componentId;
    componentId << "qttest" << QString() << "KDE Test Program" << QString();
    // QVERIFY(!components.contains(componentId));

    m_actionA = new KAction("Text For Action A", this);
    m_actionA->setObjectName("Action A:" + id);
    m_actionA->setGlobalShortcut(
            KShortcut(sequenceA, sequenceB),
            KAction::ActiveShortcut|KAction::DefaultShortcut,
            KAction::NoAutoloading);

    m_actionB = new KAction("Text For Action B", this);
    m_actionB->setObjectName("Action B:" + id);
    m_actionB->setGlobalShortcut(
            KShortcut(),
            KAction::ActiveShortcut|KAction::DefaultShortcut,
            KAction::NoAutoloading);

}


void KGlobalShortcutTest::testSetShortcut()
{
    setupTest("testSetShortcut");

    // Just ensure that the desired values are set for both actions
    KShortcut cutA(sequenceA, sequenceB);
    QCOMPARE(m_actionA->globalShortcut(), cutA);
    QCOMPARE(m_actionA->globalShortcut(KAction::DefaultShortcut), cutA);

    QVERIFY(m_actionB->globalShortcut().isEmpty());
    QVERIFY(m_actionB->globalShortcut(KAction::DefaultShortcut).isEmpty());
}

// Current state
// m_actionA: (sequenceA, sequenceB)
// m_actionB: (,)

void KGlobalShortcutTest::testFindActionByKey()
{
    // Skip this. The above testcase hasn't changed the actions
    setupTest("testFindActionByKey");

    QStringList actionId = KGlobalAccel::self()->findActionNameSystemwide(sequenceB);
    QStringList actionIdA; actionIdA << "qttest" << "Action A:testFindActionByKey" << "KDE Test Program" << "Text For Action A";
    QCOMPARE(actionId, actionIdA);
    actionId = KGlobalAccel::self()->findActionNameSystemwide(sequenceA);
    QCOMPARE(actionId, actionIdA);
}

void KGlobalShortcutTest::testChangeShortcut()
{
    // Skip this. The above testcase hasn't changed the actions
    setupTest("testChangeShortcut");

    // Change the shortcut
    KShortcut newCutA(sequenceC);
    m_actionA->setGlobalShortcut(newCutA, KAction::ActiveShortcut, KAction::NoAutoloading);
    // Ensure that the change is active
    QCOMPARE(m_actionA->globalShortcut(), newCutA);
    // We haven't changed the default shortcut, ensure it is unchanged
    KShortcut cutA(sequenceA, sequenceB);
    QCOMPARE(m_actionA->globalShortcut(KAction::DefaultShortcut), cutA);

    // Try to set a already take shortcut
    KShortcut cutB(m_actionA->globalShortcut().primary(), QKeySequence(sequenceE));
    m_actionB->setGlobalShortcut(cutB, KAction::ActiveShortcut, KAction::NoAutoloading);
    // Ensure that no change was made to the primary active shortcut
    QVERIFY(m_actionB->globalShortcut().primary().isEmpty());
    // Ensure that the change to the secondary active shortcut was made
    QCOMPARE(m_actionB->globalShortcut().alternate(), QKeySequence(sequenceE));
    // Ensure that the default shortcut is still empty
    QVERIFY(m_actionB->globalShortcut(KAction::DefaultShortcut).isEmpty()); // unchanged

    // Only change the active shortcut
    cutB.setPrimary(sequenceD);
    m_actionB->setGlobalShortcut(cutB, KAction::ActiveShortcut, KAction::NoAutoloading);
    // Check that the change went through
    QCOMPARE(m_actionB->globalShortcut(), cutB);
    // Check that the default shortcut is not active
    QVERIFY(m_actionB->globalShortcut(KAction::DefaultShortcut).isEmpty()); // unchanged
}

void KGlobalShortcutTest::testStealShortcut()
{
    setupTest("testStealShortcut");

    // Steal a shortcut from an action. First ensure the initial state is
    // correct
    KShortcut cutA(sequenceA, sequenceB);
    QCOMPARE(m_actionA->globalShortcut(), cutA);
    QCOMPARE(m_actionA->globalShortcut(KAction::DefaultShortcut), cutA);

    KGlobalAccel::stealShortcutSystemwide(sequenceA);
    //let DBus do its thing in case it happens asynchronously
    QTest::qWait(200);
    QVERIFY(m_actionB->globalShortcut(KAction::ActiveShortcut).primary().isEmpty());
}


void KGlobalShortcutTest::testSaveRestore()
{
    setupTest("testSaveRestore");

    //It /would be nice/ to test persistent storage. That is not so easy...
    KShortcut cutA = m_actionA->globalShortcut();
    // Delete the action
    delete m_actionA;

    // Recreate it
    m_actionA = new KAction("Text For Action A", this);
    m_actionA->setObjectName("Action A:testSaveRestore");

    // Now it's empty
    QVERIFY(m_actionA->globalShortcut().isEmpty());

    m_actionA->setGlobalShortcut(KShortcut());
    // Now it's restored
    QCOMPARE(m_actionA->globalShortcut(), cutA);

    // And again
    delete m_actionA;
    m_actionA = new KAction("Text For Action A", this);
    m_actionA->setObjectName("Action A:testSaveRestore");
    m_actionA->setGlobalShortcut(KShortcut(QKeySequence(), cutA.primary()));
    QCOMPARE(m_actionA->globalShortcut(), cutA);

}

// Duplicated again!
enum actionIdFields
{
    ComponentUnique = 0,
    ActionUnique = 1,
    ComponentFriendly = 2,
    ActionFriendly = 3
};

void KGlobalShortcutTest::testListActions()
{
    setupTest("testListActions");

    // As in kdebase/workspace/kcontrol/keys/globalshortcuts.cpp
    KGlobalAccel *kga = KGlobalAccel::self();
    QList<QStringList> components = kga->allMainComponents();
    //qDebug() << components;
    QStringList componentId;
    componentId << "qttest" << QString() << "KDE Test Program" << QString();
    QVERIFY(components.contains(componentId));

    QList<QStringList> actions = kga->allActionsForComponent(componentId);
    QVERIFY(!actions.isEmpty());
    QStringList actionIdA; actionIdA << "qttest" << "Action A:testListActions" << "KDE Test Program" << "Text For Action A";
    QStringList actionIdB; actionIdB << "qttest" << "Action B:testListActions" << "KDE Test Program" << "Text For Action B";
    //qDebug() << actions;
    QVERIFY(actions.contains(actionIdA));
    QVERIFY(actions.contains(actionIdB));
}

void KGlobalShortcutTest::testComponentAssignment()
{
    // We don't use them here
    // setupTest();

    KComponentData otherComponent("test_component1");
    KActionCollection coll((QObject*)NULL);
    coll.setComponentData(otherComponent);
    KShortcut cutB;
    /************************************************************
     * Ensure that the actions get a correct component assigned *
     ************************************************************/
    // Action without action collection get the global component
    {
        KAction action("Text For Action A", NULL);
        action.setObjectName("Action C");

        QVERIFY(action.d->componentData == KGlobal::mainComponent());
        action.setGlobalShortcut(cutB, KAction::ActiveShortcut, KAction::NoAutoloading);
        QVERIFY(action.d->componentData == KGlobal::mainComponent());
        // cleanup
        action.forgetGlobalShortcut();
    }

    // Action with action collection get the component of the collection
    {
        KAction *action = coll.addAction("Action C");

        QVERIFY(action->d->componentData == otherComponent);
        action->setGlobalShortcut(cutB, KAction::ActiveShortcut, KAction::NoAutoloading);
        QVERIFY(action->d->componentData == otherComponent);
        // cleanup
        action->forgetGlobalShortcut();
        delete action;
    }
}


void KGlobalShortcutTest::testConfigurationActions()
{
    setupTest("testConfigurationActions");

    // Create a configuration action
    KAction cfg_action("Text For Action A", NULL);
    cfg_action.setObjectName("Action A:testConfigurationActions");
    cfg_action.setProperty("isConfigurationAction", true);
    cfg_action.setGlobalShortcut(KShortcut());

    // Check that the configuration action has the correct shortcuts
    QCOMPARE(m_actionA->globalShortcut(), cfg_action.globalShortcut());

    // TODO:
    // - change shortcut from configuration action and test for
    //   yourShortcutGotChanged
    // - Ensure that the config action doesn't trigger(how?)
    // - Ensure that the original action is still working when the
    //   configuration action is deleted
}

void KGlobalShortcutTest::testOverrideMainComponentData()
{
    setupTest("testOverrideMainComponentData");

    KComponentData otherComponent("test_component1");
    KActionCollection coll((QObject*)NULL);
    coll.setComponentData(otherComponent);
    KShortcut cutB;

    // Action without action collection
    KAction *action = new KAction("Text For Action A", this);
    QVERIFY(action->d->componentData == KGlobal::mainComponent());
    action->setObjectName("Action A");
    action->setGlobalShortcut(cutB, KAction::ActiveShortcut, KAction::NoAutoloading);
    QVERIFY(action->d->componentData == KGlobal::mainComponent());

    // Action with action collection
    action->forgetGlobalShortcut();
    delete action;
    action = coll.addAction("Action A");
    QVERIFY(action->d->componentData == otherComponent);
    action->setGlobalShortcut(cutB, KAction::ActiveShortcut, KAction::NoAutoloading);
    QVERIFY(action->d->componentData == otherComponent);

    // cleanup
    action->forgetGlobalShortcut();
    delete coll.takeAction(action);

    // activate overrideMainComponentData, it's not revokable currently!
    // overrideMainComponentData only overrides the component if the action
    // gets a real global shortcut!
    KComponentData globalComponent("test_component2");
    KGlobalAccel::self()->overrideMainComponentData(globalComponent);

    // Action with action collection gets the global component
    action = new KAction("Text For Action A", this);
    QVERIFY(action->d->componentData == KGlobal::mainComponent());
    action->setObjectName("Action A");
    action->setGlobalShortcut(cutB, KAction::ActiveShortcut, KAction::NoAutoloading);
    QVERIFY(action->d->componentData == globalComponent);

    // Action with action collection get the component of the collection until
    // a global shortcut is set when overrideMainComponentData is active
    action->forgetGlobalShortcut();
    delete action;
    QVERIFY(coll.isEmpty());

    action = coll.addAction("Action A");
    QVERIFY(action->d->componentData == otherComponent);
    action->setGlobalShortcut(cutB, KAction::ActiveShortcut, KAction::NoAutoloading);
    QVERIFY(action->d->componentData == globalComponent);

    // forget the global shortcut
    action->forgetGlobalShortcut();
    // Actions that were created by the KActionCollection::addAction have the
    // collections as parent. Ensure action is not deleted.
}

void KGlobalShortcutTest::testNotification()
{
    setupTest("testNotification");

    // Action without action collection
    KAction *action = new KAction("Text For Action A", this);
    QVERIFY(action->d->componentData == KGlobal::mainComponent());
    action->setObjectName("Action A");
    KShortcut cutB;
    action->setGlobalShortcut(cutB, KAction::ActiveShortcut, KAction::NoAutoloading);
    QVERIFY(action->d->componentData == KGlobal::mainComponent());

    // kglobalacceld collects registrations and shows the together. Give it
    // time to kick in.
    sleep(2);

    action->forgetGlobalShortcut();
}

void KGlobalShortcutTest::testForgetGlobalShortcut()
{
    setupTest("testForgetGlobalShortcut");

    // Ensure that forgetGlobalShortcut can be called on any action.
    KAction a("Test", NULL);
    a.forgetGlobalShortcut();

    // We forget these two shortcuts and check that the component is gone
    // after that. If not it can mean the forgetGlobalShortcut() call is
    // broken OR someone messed up these tests to leave an additional global
    // shortcut behind.
    m_actionB->forgetGlobalShortcut();
    m_actionA->forgetGlobalShortcut();
    // kglobalaccel writes asynchronous.
    sleep(1);

    KGlobalAccel *kga = KGlobalAccel::self();
    QList<QStringList> components = kga->allMainComponents();
    QStringList componentId;
    componentId << "qttest" << QString() << "KDE Test Program" << QString();
    QVERIFY(!components.contains(componentId));
}


#include "kglobalshortcuttest.moc"
