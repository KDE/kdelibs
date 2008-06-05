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

#include <QtDBus/QDBusConnectionInterface>

const QKeySequence sequenceA = QKeySequence(Qt::SHIFT + Qt::META + Qt::CTRL + Qt::ALT + Qt::Key_F28 );
const QKeySequence sequenceB = QKeySequence(Qt::Key_F29);
const QKeySequence sequenceC = QKeySequence(Qt::SHIFT + Qt::META + Qt::CTRL + Qt::Key_F28 );
const QKeySequence sequenceD = QKeySequence(Qt::META + Qt::ALT + Qt::Key_F30);
const QKeySequence sequenceE = QKeySequence(Qt::META + Qt::Key_F29);

/* These tests could be better. They don't include actually triggering actions,
   and we just choose very improbable shortcuts to avoid conflicts with real
   applications' shortcuts. */

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   The order here i very important. An test expects the shortcuts to have
   certain values. Be careful when changing. */

//we need a KComponentData and a GUI so that the implementation can grab keys
QTEST_KDEMAIN( KGlobalShortcutTest, GUI )

void KGlobalShortcutTest::initTestCase()
{
    m_actionA = new KAction("Text For Action A", this);
    m_actionA->setObjectName("Action A");
    m_actionB = new KAction("Text For Action B", this);
    m_actionB->setObjectName("Action B");
}


void KGlobalShortcutTest::testSetShortcut()
{
    // Create a global shortcut
    KShortcut cutA(sequenceA, sequenceB);
    m_actionA->setGlobalShortcut(cutA, KAction::ActiveShortcut|KAction::DefaultShortcut, KAction::NoAutoloading);
    QCOMPARE(m_actionA->globalShortcut(), cutA);
    QCOMPARE(m_actionA->globalShortcut(KAction::DefaultShortcut), cutA);

    // Set the global shortcut to empty shortcuts
    m_actionB->setGlobalShortcut(KShortcut(), KAction::ActiveShortcut|KAction::DefaultShortcut, KAction::NoAutoloading);
    QVERIFY(m_actionB->globalShortcut().isEmpty());
    QVERIFY(m_actionB->globalShortcut(KAction::DefaultShortcut).isEmpty());

    // Check that action B, which has no shortcut set, does appear when listing the actions
    // (important for the kcontrol module)
    testListActions();

}


void KGlobalShortcutTest::testFindActionByKey()
{
    QStringList actionId = KGlobalAccel::self()->findActionNameSystemwide(sequenceB);
    QStringList actionIdA; actionIdA << "qttest" << "Action A" << "KDE Test Program" << "Text For Action A";
    QCOMPARE(actionId, actionIdA);
    actionId = KGlobalAccel::self()->findActionNameSystemwide(sequenceA);
    QCOMPARE(actionId, actionIdA);
}


void KGlobalShortcutTest::testChangeShortcut()
{
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

    QVERIFY(!m_actionB->globalShortcut(KAction::ActiveShortcut).primary().isEmpty());
    KGlobalAccel::stealShortcutSystemwide(sequenceD);
    //let DBus do its thing in case it happens asynchronously
    QTest::qWait(200);
    QVERIFY(m_actionB->globalShortcut(KAction::ActiveShortcut).primary().isEmpty());
}


void KGlobalShortcutTest::testSaveRestore()
{
    //It /would be nice/ to test persistent storage. That is not so easy...
    KShortcut cutA = m_actionA->globalShortcut();
    delete m_actionA;
    m_actionA = new KAction("Text For Action A", this);
    m_actionA->setObjectName("Action A");
    QVERIFY(m_actionA->globalShortcut().isEmpty());

    m_actionA->setGlobalShortcut(KShortcut());
    QCOMPARE(m_actionA->globalShortcut(), cutA);

    delete m_actionA;
    m_actionA = new KAction("Text For Action A", this);
    m_actionA->setObjectName("Action A");
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
    // As in kdebase/workspace/kcontrol/keys/globalshortcuts.cpp
    KGlobalAccel *kga = KGlobalAccel::self();
    QList<QStringList> components = kga->allMainComponents();
    //qDebug() << components;
    QStringList componentId;
    componentId << "qttest" << QString() << "KDE Test Program" << QString();
    QVERIFY(components.contains(componentId));

    QList<QStringList> actions = kga->allActionsForComponent(componentId);
    QVERIFY(!actions.isEmpty());
    QStringList actionIdA; actionIdA << "qttest" << "Action A" << "KDE Test Program" << "Text For Action A";
    QStringList actionIdB; actionIdB << "qttest" << "Action B" << "KDE Test Program" << "Text For Action B";
    //qDebug() << actions;
    QVERIFY(actions.contains(actionIdA));
    QVERIFY(actions.contains(actionIdB));
}

void KGlobalShortcutTest::testOverrideMainComponentData()
{
    KComponentData otherComponent("test_component1");
    KActionCollection coll((QObject*)NULL);
    coll.setComponentData(otherComponent);
    KShortcut cutB;

    // Action without action collection
    KAction *action = new KAction("Text For Action A", this);
    // That's the current state, but i'm not sure it should stay this way.
    QVERIFY(!action->d->componentData.isValid());
    action->setObjectName("Action A");
    action->setGlobalShortcut(cutB, KAction::ActiveShortcut, KAction::NoAutoloading);
    QVERIFY(!action->d->componentData.isValid());

    // Action with action collection
    delete action;
    action = coll.addAction("Text For Action A");
    QVERIFY(action->d->componentData == otherComponent);
    action->setObjectName("Action A");
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

    // Action without action collection
    action = new KAction("Text For Action A", this);
    QVERIFY(!action->d->componentData.isValid());
    action->setObjectName("Action A");
    action->setGlobalShortcut(cutB, KAction::ActiveShortcut, KAction::NoAutoloading);
    QVERIFY(!action->d->componentData.isValid());

    // Action with action collection
    delete action;
    action = coll.addAction("Text For Action A");
    QVERIFY(action->d->componentData == otherComponent);
    action->setObjectName("Action A");
    action->setGlobalShortcut(cutB, KAction::ActiveShortcut, KAction::NoAutoloading);
    QVERIFY(action->d->componentData == globalComponent);

    // forget the global shortcut
    action->forgetGlobalShortcut();
    // Actions that were created by the KActionCollection::addAction have the
    // collections as parent. Ensure action is not deleted.
}

void KGlobalShortcutTest::testForgetGlobalShortcut()
{
    // Ensure that forgetGlobalShortcut can be called on any action.
    KAction a("Test", NULL);
    a.forgetGlobalShortcut();

    // We forget these two shortcuts and check that the component is gone
    // after that. If not it can mean the forgetGlobalShortcut() call is
    // broken OR someone messed up these tests to leave an additional global
    // shortcut behind.
    m_actionB->forgetGlobalShortcut();
    m_actionA->forgetGlobalShortcut();

    KGlobalAccel *kga = KGlobalAccel::self();
    QList<QStringList> components = kga->allMainComponents();
    QStringList componentId;
    componentId << "qttest" << QString() << "KDE Test Program" << QString();
    QVERIFY(!components.contains(componentId));
}


void KGlobalShortcutTest::cleanupTestCase()
{
    delete m_actionA;
    delete m_actionB;
}

#include "kglobalshortcuttest.moc"
