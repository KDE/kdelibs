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
#include <kglobalaccel.h>
#include <kdebug.h>

#include <QtDBus/QDBusConnectionInterface>

const QKeySequence sequenceA = QKeySequence(Qt::SHIFT + Qt::META + Qt::CTRL + Qt::ALT + Qt::Key_F28 );
const QKeySequence sequenceB = QKeySequence(Qt::Key_F29);
const QKeySequence sequenceC = QKeySequence(Qt::SHIFT + Qt::META + Qt::CTRL + Qt::Key_F28 );
const QKeySequence sequenceD = QKeySequence(Qt::META + Qt::ALT + Qt::Key_F30);

/* These tests could be better. They don't include actually triggering actions,
   and we just choose very improbable shortcuts to avoid conflicts with real
   applications' shortcuts. */

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
    // We have to set the global shortcut with NoAutoloading to be independent from previous runs of the test.
    // There is no way of making kdedglobalaccel "completely forget" a global shortcut currently, this would need new API.

    //possible modifiers are SHIFT META CTRL ALT
    KShortcut cutA(sequenceA);
    //kDebug() << cutA.toString();
    m_actionA->setGlobalShortcut(cutA, KAction::ActiveShortcut|KAction::DefaultShortcut, KAction::NoAutoloading);
    //kDebug() << m_actionA->globalShortcut().toString();
    QCOMPARE(m_actionA->globalShortcut(), cutA);
    QCOMPARE(m_actionA->globalShortcut(KAction::DefaultShortcut), cutA);

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
    KShortcut newCutA(sequenceC); // same without the ALT
    m_actionA->setGlobalShortcut(newCutA, KAction::ActiveShortcut, KAction::NoAutoloading);
    QCOMPARE(m_actionA->globalShortcut(), newCutA);
    KShortcut cutA(sequenceA);
    QCOMPARE(m_actionA->globalShortcut(KAction::DefaultShortcut), cutA); // unchanged


    KShortcut cutB(m_actionA->globalShortcut().primary(), QKeySequence(sequenceB));
    m_actionB->setGlobalShortcut(cutB, KAction::ActiveShortcut,
                                 KAction::NoAutoloading);
    QVERIFY(m_actionB->globalShortcut().primary().isEmpty());
    QCOMPARE(m_actionB->globalShortcut().alternate(), QKeySequence(sequenceB));
    QVERIFY(m_actionB->globalShortcut(KAction::DefaultShortcut).isEmpty()); // unchanged

    cutB.setPrimary(sequenceD);
    m_actionB->setGlobalShortcut(cutB, KAction::ActiveShortcut,
                                 KAction::NoAutoloading);
    QCOMPARE(m_actionB->globalShortcut(), cutB);
    QVERIFY(m_actionB->globalShortcut(KAction::DefaultShortcut).isEmpty()); // unchanged
}


void KGlobalShortcutTest::testStealShortcut()
{
    QVERIFY(!m_actionB->globalShortcut().primary().isEmpty());
    KGlobalAccel::stealShortcutSystemwide(sequenceD);
    //let DBus do its thing in case it happens asynchronously
    QTest::qWait(200);
    QVERIFY(m_actionB->globalShortcut().primary().isEmpty());
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

void KGlobalShortcutTest::cleanupTestCase()
{
}

#include "kglobalshortcuttest.moc"
