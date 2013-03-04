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

#include <QtTestWidgets>
#include "kglobalshortcuttest.h"
#include <qdbusinterface.h>
#include <QtTest/QtTest>
#include <QAction>
#include <kactioncollection.h>
#include <kglobalaccel.h>
#include <kdebug.h>
#include <kservice.h>
#include <qstandardpaths.h>

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

//we need a GUI so that the implementation can grab keys
QTEST_MAIN( KGlobalShortcutTest)

void KGlobalShortcutTest::initTestCase()
{
    QStandardPaths::enableTestMode(true);
    m_daemonInstalled = !KService::serviceByDesktopName("kglobalaccel").isNull();
}

void KGlobalShortcutTest::setupTest(QString id)
{
    if (m_actionA) {
        KGlobalAccel::self()->removeAllShortcuts(m_actionA);
        delete m_actionA;
    }

    if (m_actionB) {
        KGlobalAccel::self()->removeAllShortcuts(m_actionB);
        delete m_actionB;
    }

    // Ensure that the previous test did cleanup correctly
    KGlobalAccel *kga = KGlobalAccel::self();
#ifndef KDE_NO_DEPRECATED
    QList<QStringList> components = kga->allMainComponents();
    QStringList componentId;
    componentId << "qttest" << QString() << "KDE Test Program" << QString();
    // QVERIFY(!components.contains(componentId));
#endif

    m_actionA = new QAction("Text For Action A", this);
    m_actionA->setObjectName("Action A:" + id);
    KGlobalAccel::self()->setShortcut(m_actionA, KShortcut(sequenceA, sequenceB), KGlobalAccel::NoAutoloading);
    KGlobalAccel::self()->setDefaultShortcut(m_actionA, KShortcut(sequenceA, sequenceB), KGlobalAccel::NoAutoloading);

    m_actionB = new QAction("Text For Action B", this);
    m_actionB->setObjectName("Action B:" + id);
    KGlobalAccel::self()->setShortcut(m_actionB, KShortcut(), KGlobalAccel::NoAutoloading);
    KGlobalAccel::self()->setDefaultShortcut(m_actionB, KShortcut(), KGlobalAccel::NoAutoloading);
}


void KGlobalShortcutTest::testSetShortcut()
{
    setupTest("testSetShortcut");

    if (!m_daemonInstalled)
        QSKIP("kglobalaccel not installed");

    // Just ensure that the desired values are set for both actions
    KShortcut cutA(sequenceA, sequenceB);
    QCOMPARE(KGlobalAccel::self()->shortcut(m_actionA), cutA);
    QCOMPARE(KGlobalAccel::self()->defaultShortcut(m_actionA), cutA);

    QVERIFY(KGlobalAccel::self()->shortcut(m_actionB).isEmpty());
    QVERIFY(KGlobalAccel::self()->defaultShortcut(m_actionB).isEmpty());
}

// Current state
// m_actionA: (sequenceA, sequenceB)
// m_actionB: (,)

void KGlobalShortcutTest::testFindActionByKey()
{
    // Skip this. The above testcase hasn't changed the actions
    setupTest("testFindActionByKey");
    if (!m_daemonInstalled)
        QSKIP("kglobalaccel not installed");

    QList<KGlobalShortcutInfo> actionId = KGlobalAccel::self()->getGlobalShortcutsByKey(sequenceB);
    QCOMPARE(actionId.size(), 1);

    QString actionIdAComponentUniqueName("qttest");
    QString actionIdAUniqueName("Action A:testFindActionByKey");
    QString actionIdAComponentFriendlyName("KDE Test Program");
    QString actionIdAFriendlyName("Text For Action A");

    QCOMPARE(actionId.first().componentUniqueName(), actionIdAComponentUniqueName);
    QCOMPARE(actionId.first().uniqueName(), actionIdAUniqueName);
    QCOMPARE(actionId.first().componentFriendlyName(), actionIdAComponentFriendlyName);
    QCOMPARE(actionId.first().friendlyName(), actionIdAFriendlyName);

    actionId = KGlobalAccel::self()->getGlobalShortcutsByKey(sequenceA);
    QCOMPARE(actionId.size(), 1);

    QCOMPARE(actionId.first().componentUniqueName(), actionIdAComponentUniqueName);
    QCOMPARE(actionId.first().uniqueName(), actionIdAUniqueName);
    QCOMPARE(actionId.first().componentFriendlyName(), actionIdAComponentFriendlyName);
    QCOMPARE(actionId.first().friendlyName(), actionIdAFriendlyName);
}

void KGlobalShortcutTest::testChangeShortcut()
{
    // Skip this. The above testcase hasn't changed the actions
    setupTest("testChangeShortcut");

    if (!m_daemonInstalled)
        QSKIP("kglobalaccel not installed");
    // Change the shortcut
    KShortcut newCutA(sequenceC);
    KGlobalAccel::self()->setShortcut(m_actionA, newCutA, KGlobalAccel::NoAutoloading);
    // Ensure that the change is active
    QCOMPARE(KGlobalAccel::self()->shortcut(m_actionA), newCutA);
    // We haven't changed the default shortcut, ensure it is unchanged
    KShortcut cutA(sequenceA, sequenceB);
    QCOMPARE(KGlobalAccel::self()->defaultShortcut(m_actionA), cutA);

    // Try to set a already take shortcut
    KShortcut cutB(KGlobalAccel::self()->shortcut(m_actionA).primary(), QKeySequence(sequenceE));
    KGlobalAccel::self()->setShortcut(m_actionB, cutB, KGlobalAccel::NoAutoloading);
    // Ensure that no change was made to the primary active shortcut
    QVERIFY(KGlobalAccel::self()->shortcut(m_actionB).primary().isEmpty());
    // Ensure that the change to the secondary active shortcut was made
    QCOMPARE(KGlobalAccel::self()->shortcut(m_actionB).alternate(), QKeySequence(sequenceE));
    // Ensure that the default shortcut is still empty
    QVERIFY(KGlobalAccel::self()->defaultShortcut(m_actionB).isEmpty()); // unchanged

    // Only change the active shortcut
    cutB.setPrimary(sequenceD);
    KGlobalAccel::self()->setShortcut(m_actionB, cutB, KGlobalAccel::NoAutoloading);
    // Check that the change went through
    QCOMPARE(KGlobalAccel::self()->shortcut(m_actionB), cutB);
    // Check that the default shortcut is not active
    QVERIFY(KGlobalAccel::self()->defaultShortcut(m_actionB).isEmpty()); // unchanged
}

void KGlobalShortcutTest::testStealShortcut()
{
    setupTest("testStealShortcut");
    if (!m_daemonInstalled)
        QSKIP("kglobalaccel not installed");

    // Steal a shortcut from an action. First ensure the initial state is
    // correct
    KShortcut cutA(sequenceA, sequenceB);
    QCOMPARE(KGlobalAccel::self()->shortcut(m_actionA), cutA);
    QCOMPARE(KGlobalAccel::self()->defaultShortcut(m_actionA), cutA);

    KGlobalAccel::stealShortcutSystemwide(sequenceA);
    //let DBus do its thing in case it happens asynchronously
    QTest::qWait(200);
    QVERIFY(KGlobalAccel::self()->shortcut(m_actionB).primary().isEmpty());
}


void KGlobalShortcutTest::testSaveRestore()
{
    setupTest("testSaveRestore");

    //It /would be nice/ to test persistent storage. That is not so easy...
    KShortcut cutA = KGlobalAccel::self()->shortcut(m_actionA);
    // Delete the action
    delete m_actionA;

    // Recreate it
    m_actionA = new QAction("Text For Action A", this);
    m_actionA->setObjectName("Action A:testSaveRestore");

    // Now it's empty
    QVERIFY(KGlobalAccel::self()->shortcut(m_actionA).isEmpty());

    KGlobalAccel::self()->setShortcut(m_actionA, KShortcut());
    // Now it's restored
    QCOMPARE(KGlobalAccel::self()->shortcut(m_actionA), cutA);

    // And again
    delete m_actionA;
    m_actionA = new QAction("Text For Action A", this);
    m_actionA->setObjectName("Action A:testSaveRestore");
    KGlobalAccel::self()->setShortcut(m_actionA, KShortcut(QKeySequence(), cutA.primary()));
    QCOMPARE(KGlobalAccel::self()->shortcut(m_actionA), cutA);

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
    if (!m_daemonInstalled)
        QSKIP("kglobalaccel not installed");

    // As in kdebase/workspace/kcontrol/keys/globalshortcuts.cpp
    KGlobalAccel *kga = KGlobalAccel::self();
#ifndef KDE_NO_DEPRECATED
    QList<QStringList> components = kga->allMainComponents();
    //qDebug() << components;
    QStringList componentId;
    componentId << "qttest" << QString() << "KDE Test Program" << QString();
    QVERIFY(components.contains(componentId));
#endif

#ifndef KDE_NO_DEPRECATED
    QList<QStringList> actions = kga->allActionsForComponent(componentId);
    QVERIFY(!actions.isEmpty());
    QStringList actionIdA; actionIdA << "qttest" << "Action A:testListActions" << "KDE Test Program" << "Text For Action A";
    QStringList actionIdB; actionIdB << "qttest" << "Action B:testListActions" << "KDE Test Program" << "Text For Action B";
    //qDebug() << actions;
    QVERIFY(actions.contains(actionIdA));
    QVERIFY(actions.contains(actionIdB));
#endif
}

void KGlobalShortcutTest::testComponentAssignment()
{
    // We don't use them here
    // setupTest();

    QString otherComponent("test_component1");
    KActionCollection coll((QObject*)NULL);
    coll.setComponentName(otherComponent);
    KShortcut cutB;
    /************************************************************
     * Ensure that the actions get a correct component assigned *
     ************************************************************/
    // Action without action collection get the global component
    {
        QAction action("Text For Action A", NULL);
        action.setObjectName("Action C");

        QCOMPARE(action.property("componentName").toString(), QString());
        KGlobalAccel::self()->setShortcut(&action, cutB, KGlobalAccel::NoAutoloading);
        QCOMPARE(action.property("componentName").toString(), QString());
        // cleanup
        KGlobalAccel::self()->removeAllShortcuts(&action);
    }

    // Action with action collection get the component of the collection
    {
        QAction *action = coll.addAction("Action C");

        QCOMPARE(action->property("componentName").toString(), otherComponent);
        KGlobalAccel::self()->setShortcut(action, cutB, KGlobalAccel::NoAutoloading);
        QCOMPARE(action->property("componentName").toString(), otherComponent);
        // cleanup
        KGlobalAccel::self()->removeAllShortcuts(action);
        delete action;
    }
}


void KGlobalShortcutTest::testConfigurationActions()
{
    setupTest("testConfigurationActions");

    // Create a configuration action
    QAction cfg_action("Text For Action A", NULL);
    cfg_action.setObjectName("Action A:testConfigurationActions");
    cfg_action.setProperty("isConfigurationAction", true);
    KGlobalAccel::self()->setShortcut(&cfg_action, KShortcut());

    // Check that the configuration action has the correct shortcuts
    QCOMPARE(KGlobalAccel::self()->shortcut(m_actionA), KGlobalAccel::self()->shortcut(&cfg_action));

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

    QString otherComponent("test_component1");
    KActionCollection coll((QObject*)NULL);
    coll.setComponentName(otherComponent);
    KShortcut cutB;

    // Action without action collection
    QAction *action = new QAction("Text For Action A", this);
    QCOMPARE(action->property("componentName").toString(), QString());
    action->setObjectName("Action A");
    KGlobalAccel::self()->setShortcut(action, cutB, KGlobalAccel::NoAutoloading);
    QCOMPARE(action->property("componentName").toString(), QString());

    // Action with action collection
    KGlobalAccel::self()->removeAllShortcuts(action);
    delete action;
    action = coll.addAction("Action A");
    QCOMPARE(action->property("componentName").toString(), otherComponent);
    KGlobalAccel::self()->setShortcut(action, cutB, KGlobalAccel::NoAutoloading);
    QCOMPARE(action->property("componentName").toString(), otherComponent);

    // cleanup
    KGlobalAccel::self()->removeAllShortcuts(action);
    delete coll.takeAction(action);
}

void KGlobalShortcutTest::testNotification()
{
    setupTest("testNotification");

    // Action without action collection
    QAction *action = new QAction("Text For Action A", this);
    QCOMPARE(action->property("componentName").toString(), QString());
    action->setObjectName("Action A");
    KShortcut cutB;
    KGlobalAccel::self()->setShortcut(action, cutB, KGlobalAccel::NoAutoloading);
    QCOMPARE(action->property("componentName").toString(), QString());

    // kglobalacceld collects registrations and shows the together. Give it
    // time to kick in.
    sleep(2);

    KGlobalAccel::self()->removeAllShortcuts(action);
}

void KGlobalShortcutTest::testForgetGlobalShortcut()
{
    setupTest("testForgetGlobalShortcut");

    // Ensure that forgetGlobalShortcut can be called on any action.
    QAction a("Test", NULL);
    KGlobalAccel::self()->removeAllShortcuts(&a);
    if (!m_daemonInstalled)
        QSKIP("kglobalaccel not installed");

    // We forget these two shortcuts and check that the component is gone
    // after that. If not it can mean the forgetGlobalShortcut() call is
    // broken OR someone messed up these tests to leave an additional global
    // shortcut behind.
    KGlobalAccel::self()->removeAllShortcuts(m_actionB);
    KGlobalAccel::self()->removeAllShortcuts(m_actionA);
    // kglobalaccel writes asynchronous.
    sleep(1);

    KGlobalAccel *kga = KGlobalAccel::self();
#ifndef KDE_NO_DEPRECATED
    QList<QStringList> components = kga->allMainComponents();
    QStringList componentId;
    componentId << "qttest" << QString() << "KDE Test Program" << QString();
    QVERIFY(!components.contains(componentId));
#endif
}


