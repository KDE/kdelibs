/* This file is part of the KDE libraries
    Copyright (c) 2009 Daniel Calviño Sánchez <danxuliu@gmail.com>

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

#include "kselectaction_unittest.h"
#include "kselectaction_unittest.moc"
#include "qtest_kde.h"
#include <kselectaction.h>
#include <kcombobox.h>
#include <qtoolbar.h>

QTEST_KDEMAIN( KSelectAction_UnitTest, GUI )

void KSelectAction_UnitTest::testSetToolTipBeforeRequestingComboBoxWidget()
{
    KSelectAction selectAction("selectAction", 0);
    selectAction.setToolBarMode(KSelectAction::ComboBoxMode);
    selectAction.setToolTip("Test");
    selectAction.setEnabled(false); // also test disabling the action

    QWidget parent;
    QWidget* widget = selectAction.requestWidget(&parent);

    QVERIFY(widget);
    KComboBox* comboBox = qobject_cast<KComboBox*>(widget);
    QVERIFY(comboBox);
    QCOMPARE(comboBox->toolTip(), QString("Test"));
    QCOMPARE(comboBox->isEnabled(), false);
}

void KSelectAction_UnitTest::testSetToolTipAfterRequestingComboBoxWidget()
{
    KSelectAction selectAction("selectAction", 0);
    selectAction.setToolBarMode(KSelectAction::ComboBoxMode);

    QWidget parent;
    QWidget* widget = selectAction.requestWidget(&parent);

    selectAction.setToolTip("Test");
    selectAction.setEnabled(false); // also test disabling the action

    QVERIFY(widget);
    KComboBox* comboBox = qobject_cast<KComboBox*>(widget);
    QVERIFY(comboBox);
    QCOMPARE(comboBox->toolTip(), QString("Test"));
    QCOMPARE(comboBox->isEnabled(), false);
}

void KSelectAction_UnitTest::testSetToolTipBeforeRequestingToolButtonWidget()
{
    KSelectAction selectAction("selectAction", 0);
    selectAction.setToolBarMode(KSelectAction::MenuMode);
    selectAction.setToolTip("Test");

    QToolBar toolBar;
    //Don't use requestWidget, as it needs a releaseWidget when used in MenuMode
    //(in ComboBoxMode the widget is released automatically when it is
    //destroyed). When the action is added to the QToolBar, it requests and
    //releases the widget as needed.
    toolBar.addAction(&selectAction);
    QWidget* widget = toolBar.widgetForAction(&selectAction);

    QVERIFY(widget);
    QToolButton* toolButton = qobject_cast<QToolButton*>(widget);
    QVERIFY(toolButton);
    QCOMPARE(toolButton->toolTip(), QString("Test"));
}

void KSelectAction_UnitTest::testSetToolTipAfterRequestingToolButtonWidget()
{
    KSelectAction selectAction("selectAction", 0);
    selectAction.setToolBarMode(KSelectAction::MenuMode);

    QToolBar toolBar;
    //Don't use requestWidget, as it needs a releaseWidget when used in MenuMode
    //(in ComboBoxMode the widget is released automatically when it is
    //destroyed). When the action is added to the QToolBar, it requests and
    //releases the widget as needed.
    toolBar.addAction(&selectAction);
    QWidget* widget = toolBar.widgetForAction(&selectAction);

    selectAction.setToolTip("Test");

    QVERIFY(widget);
    QToolButton* toolButton = qobject_cast<QToolButton*>(widget);
    QVERIFY(toolButton);
    QCOMPARE(toolButton->toolTip(), QString("Test"));
}

void KSelectAction_UnitTest::testSetWhatsThisBeforeRequestingComboBoxWidget()
{
    KSelectAction selectAction("selectAction", 0);
    selectAction.setToolBarMode(KSelectAction::ComboBoxMode);
    selectAction.setWhatsThis("Test");

    QWidget parent;
    QWidget* widget = selectAction.requestWidget(&parent);

    QVERIFY(widget);
    KComboBox* comboBox = qobject_cast<KComboBox*>(widget);
    QVERIFY(comboBox);
    QCOMPARE(comboBox->whatsThis(), QString("Test"));
}

void KSelectAction_UnitTest::testSetWhatsThisAfterRequestingComboBoxWidget()
{
    KSelectAction selectAction("selectAction", 0);
    selectAction.setToolBarMode(KSelectAction::ComboBoxMode);

    QWidget parent;
    QWidget* widget = selectAction.requestWidget(&parent);

    selectAction.setWhatsThis("Test");

    QVERIFY(widget);
    KComboBox* comboBox = qobject_cast<KComboBox*>(widget);
    QVERIFY(comboBox);
    QCOMPARE(comboBox->whatsThis(), QString("Test"));
}

void KSelectAction_UnitTest::testSetWhatsThisBeforeRequestingToolButtonWidget()
{
    KSelectAction selectAction("selectAction", 0);
    selectAction.setToolBarMode(KSelectAction::MenuMode);
    selectAction.setWhatsThis("Test");

    QToolBar toolBar;
    //Don't use requestWidget, as it needs a releaseWidget when used in MenuMode
    //(in ComboBoxMode the widget is released automatically when it is
    //destroyed). When the action is added to the QToolBar, it requests and
    //releases the widget as needed.
    toolBar.addAction(&selectAction);
    QWidget* widget = toolBar.widgetForAction(&selectAction);

    QVERIFY(widget);
    QToolButton* toolButton = qobject_cast<QToolButton*>(widget);
    QVERIFY(toolButton);
    QCOMPARE(toolButton->whatsThis(), QString("Test"));
}

void KSelectAction_UnitTest::testSetWhatsThisAfterRequestingToolButtonWidget()
{
    KSelectAction selectAction("selectAction", 0);
    selectAction.setToolBarMode(KSelectAction::MenuMode);

    QToolBar toolBar;
    //Don't use requestWidget, as it needs a releaseWidget when used in MenuMode
    //(in ComboBoxMode the widget is released automatically when it is
    //destroyed). When the action is added to the QToolBar, it requests and
    //releases the widget as needed.
    toolBar.addAction(&selectAction);
    QWidget* widget = toolBar.widgetForAction(&selectAction);

    selectAction.setWhatsThis("Test");

    QVERIFY(widget);
    QToolButton* toolButton = qobject_cast<QToolButton*>(widget);
    QVERIFY(toolButton);
    QCOMPARE(toolButton->whatsThis(), QString("Test"));
}
