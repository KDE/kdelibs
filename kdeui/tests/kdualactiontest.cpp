/* This file is part of the KDE libraries
 *
 * Copyright (c) 2010 Aurélien Gâteau <agateau@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include <kdualaction.h>

#include "qtest_kde.h"

Q_DECLARE_METATYPE(KAction*)

static const QString INACTIVE_TEXT = "Show Foo";
static const QString ACTIVE_TEXT = "Hide Foo";

class KDualActionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        qRegisterMetaType<KAction*>("KAction*");
    }

    void testSetGuiItem()
    {
        KDualAction action(0);
        action.setGuiItemForState(KDualAction::InactiveState, KGuiItem(INACTIVE_TEXT));
        action.setGuiItemForState(KDualAction::ActiveState, KGuiItem(ACTIVE_TEXT));
        QCOMPARE(action.textForState(KDualAction::InactiveState), INACTIVE_TEXT);
        QCOMPARE(action.textForState(KDualAction::ActiveState), ACTIVE_TEXT);
        QCOMPARE(action.text(), INACTIVE_TEXT);
    }

    void testSetIconForStates()
    {
        QIcon icon = KIcon("kde");
        KDualAction action(0);
        QVERIFY(action.iconForState(KDualAction::InactiveState).isNull());
        QVERIFY(action.iconForState(KDualAction::ActiveState).isNull());
        action.setIconForStates(icon);
        QCOMPARE(action.iconForState(KDualAction::InactiveState), icon);
        QCOMPARE(action.iconForState(KDualAction::ActiveState), icon);
    }

    void testSetActive()
    {
        KDualAction action(INACTIVE_TEXT, ACTIVE_TEXT, 0);
        QVERIFY(!action.isActive());
        QCOMPARE(action.text(), INACTIVE_TEXT);
        QSignalSpy activeChangedSpy(&action, SIGNAL(activeChanged(bool)));
        QSignalSpy activeChangedByUserSpy(&action, SIGNAL(activeChangedByUser(bool)));

        action.setActive(true);
        QVERIFY(action.isActive());
        QCOMPARE(action.text(), ACTIVE_TEXT);
        QCOMPARE(activeChangedSpy.count(), 1);
        QCOMPARE(activeChangedSpy.takeFirst().at(0).toBool(), true);
        QCOMPARE(activeChangedByUserSpy.count(), 0);

        action.setActive(false);
        QVERIFY(!action.isActive());
        QCOMPARE(action.text(), INACTIVE_TEXT);
        QCOMPARE(activeChangedSpy.count(), 1);
        QCOMPARE(activeChangedSpy.takeFirst().at(0).toBool(), false);
        QCOMPARE(activeChangedByUserSpy.count(), 0);
    }

    void testTrigger()
    {
        KDualAction action(INACTIVE_TEXT, ACTIVE_TEXT, 0);
        QVERIFY(!action.isActive());
        QCOMPARE(action.text(), INACTIVE_TEXT);
        QSignalSpy activeChangedSpy(&action, SIGNAL(activeChanged(bool)));
        QSignalSpy activeChangedByUserSpy(&action, SIGNAL(activeChangedByUser(bool)));

        action.trigger();
        QVERIFY(action.isActive());
        QCOMPARE(action.text(), ACTIVE_TEXT);
        QCOMPARE(activeChangedSpy.count(), 1);
        QCOMPARE(activeChangedSpy.takeFirst().at(0).toBool(), true);
        QCOMPARE(activeChangedByUserSpy.count(), 1);
        QCOMPARE(activeChangedByUserSpy.takeFirst().at(0).toBool(), true);

        action.trigger();
        QVERIFY(!action.isActive());
        QCOMPARE(action.text(), INACTIVE_TEXT);
        QCOMPARE(activeChangedSpy.count(), 1);
        QCOMPARE(activeChangedSpy.takeFirst().at(0).toBool(), false);
        QCOMPARE(activeChangedByUserSpy.count(), 1);
        QCOMPARE(activeChangedByUserSpy.takeFirst().at(0).toBool(), false);

        // Turn off autoToggle, nothing should happen
        action.setAutoToggle(false);
        action.trigger();
        QVERIFY(!action.isActive());
        QCOMPARE(action.text(), INACTIVE_TEXT);
        QCOMPARE(activeChangedSpy.count(), 0);
        QCOMPARE(activeChangedByUserSpy.count(), 0);

        // Turn on autoToggle, action should change
        action.setAutoToggle(true);
        action.trigger();
        QCOMPARE(action.text(), ACTIVE_TEXT);
        QCOMPARE(activeChangedSpy.count(), 1);
        QCOMPARE(activeChangedSpy.takeFirst().at(0).toBool(), true);
        QCOMPARE(activeChangedByUserSpy.count(), 1);
        QCOMPARE(activeChangedByUserSpy.takeFirst().at(0).toBool(), true);
    }
};

QTEST_KDEMAIN(KDualActionTest, GUI)

#include "kdualactiontest.moc"
