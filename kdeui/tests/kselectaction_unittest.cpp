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
#include <QComboBox>
#include <QStandardItemModel>
#include <QtTest/QtTest>
#include <kselectaction.h>
#include <qtoolbar.h>

QTEST_MAIN( KSelectAction_UnitTest)

void KSelectAction_UnitTest::testSetToolTipBeforeRequestingComboBoxWidget()
{
    KSelectAction selectAction("selectAction", 0);
    selectAction.setToolBarMode(KSelectAction::ComboBoxMode);
    selectAction.setToolTip("Test");
    selectAction.setEnabled(false); // also test disabling the action

    QWidget parent;
    QWidget* widget = selectAction.requestWidget(&parent);

    QVERIFY(widget);
    QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
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
    QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
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
    QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
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
    QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
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

void KSelectAction_UnitTest::testChildActionStateChangeComboMode()
{
    KSelectAction selectAction("selectAction", 0);
    selectAction.setToolBarMode(KSelectAction::ComboBoxMode);
    QWidget parent;
    QWidget* widget = selectAction.requestWidget(&parent);
    QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
    QVERIFY(comboBox);
    const QString itemText = "foo";
    QAction* childAction = selectAction.addAction(itemText);
    QCOMPARE(comboBox->itemText(0), itemText);
    childAction->setEnabled(false);
    // There's no API for item-is-enabled, need to go via the internal model like kselectaction does...
    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(comboBox->model());
    QVERIFY(model);
    QVERIFY(!model->item(0)->isEnabled());

    // Now remove the action
    selectAction.removeAction(childAction);
    QCOMPARE(comboBox->count(), 0);
}

void KSelectAction_UnitTest::testRequestWidgetComboBoxModeWidgetParent()
{
    KSelectAction selectAction("selectAction", 0);
    selectAction.setToolBarMode(KSelectAction::ComboBoxMode);

    QToolBar toolBar;
    toolBar.addAction(&selectAction);
    QWidget* widget = toolBar.widgetForAction(&selectAction);

    QVERIFY(widget);
    QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
    QVERIFY(comboBox);
    QVERIFY(!comboBox->isEnabled());
}

void KSelectAction_UnitTest::testRequestWidgetComboBoxModeWidgetParentSeveralActions()
{
    KSelectAction selectAction("selectAction", 0);
    selectAction.setToolBarMode(KSelectAction::ComboBoxMode);

    selectAction.addAction(new QAction("action1", &selectAction));
    selectAction.addAction(new QAction("action2", &selectAction));
    selectAction.addAction(new QAction("action3", &selectAction));

    QToolBar toolBar;
    toolBar.addAction(&selectAction);
    QWidget* widget = toolBar.widgetForAction(&selectAction);

    QVERIFY(widget);
    QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
    QVERIFY(comboBox);
    QVERIFY(comboBox->isEnabled());
}

void KSelectAction_UnitTest::testRequestWidgetMenuModeWidgetParent()
{
    KSelectAction selectAction("selectAction", 0);
    selectAction.setToolBarMode(KSelectAction::MenuMode);

    QToolBar toolBar;
    toolBar.addAction(&selectAction);
    QWidget* widget = toolBar.widgetForAction(&selectAction);

    QVERIFY(widget);
    QToolButton* toolButton = qobject_cast<QToolButton*>(widget);
    QVERIFY(toolButton);
    QVERIFY(!toolButton->isEnabled());
    QVERIFY(toolButton->autoRaise());
    QCOMPARE((int)toolButton->focusPolicy(), (int)Qt::NoFocus);
    QCOMPARE(toolButton->defaultAction(), (QAction*)&selectAction);
    QCOMPARE(toolButton->actions().count(), 1);
    QCOMPARE(toolButton->actions().at(0)->text(), QString("selectAction"));
}

void KSelectAction_UnitTest::testRequestWidgetMenuModeWidgetParentSeveralActions()
{
    KSelectAction selectAction("selectAction", 0);
    selectAction.setToolBarMode(KSelectAction::MenuMode);

    selectAction.addAction(new QAction("action1", &selectAction));
    selectAction.addAction(new QAction("action2", &selectAction));
    selectAction.addAction(new QAction("action3", &selectAction));

    QToolBar toolBar;
    toolBar.addAction(&selectAction);
    QWidget* widget = toolBar.widgetForAction(&selectAction);

    QVERIFY(widget);
    QToolButton* toolButton = qobject_cast<QToolButton*>(widget);
    QVERIFY(toolButton);
    QVERIFY(toolButton->isEnabled());
    QVERIFY(toolButton->autoRaise());
    QCOMPARE((int)toolButton->focusPolicy(), (int)Qt::NoFocus);
    QCOMPARE(toolButton->defaultAction(), (QAction*)&selectAction);
    QCOMPARE(toolButton->actions().count(), 4);
    QCOMPARE(toolButton->actions().at(0)->text(), QString("selectAction"));
    QCOMPARE(toolButton->actions().at(1)->text(), QString("action1"));
    QCOMPARE(toolButton->actions().at(2)->text(), QString("action2"));
    QCOMPARE(toolButton->actions().at(3)->text(), QString("action3"));
}

void KSelectAction_UnitTest::testRequestWidgetMenuModeWidgetParentAddActions()
{
    KSelectAction selectAction("selectAction", 0);
    selectAction.setToolBarMode(KSelectAction::MenuMode);

    QToolBar toolBar;
    toolBar.addAction(&selectAction);
    QWidget* widget = toolBar.widgetForAction(&selectAction);

    QVERIFY(widget);
    QVERIFY(!widget->isEnabled());

    selectAction.addAction(new QAction("action1", &selectAction));
    selectAction.addAction(new QAction("action2", &selectAction));
    selectAction.addAction(new QAction("action3", &selectAction));

    QVERIFY(widget->isEnabled());
    QCOMPARE(widget->actions().count(), 4);
    QCOMPARE(widget->actions().at(0)->text(), QString("selectAction"));
    QCOMPARE(widget->actions().at(1)->text(), QString("action1"));
    QCOMPARE(widget->actions().at(2)->text(), QString("action2"));
    QCOMPARE(widget->actions().at(3)->text(), QString("action3"));
}

void KSelectAction_UnitTest::testRequestWidgetMenuModeWidgetParentRemoveActions()
{
    KSelectAction selectAction("selectAction", 0);
    selectAction.setToolBarMode(KSelectAction::MenuMode);

    QToolBar toolBar;
    toolBar.addAction(&selectAction);
    QWidget* widget = toolBar.widgetForAction(&selectAction);

    QVERIFY(widget);

    QAction* action1 = new QAction("action1", &selectAction);
    selectAction.addAction(action1);
    QAction* action2 = new QAction("action2", &selectAction);
    selectAction.addAction(action2);
    QAction* action3 = new QAction("action3", &selectAction);
    selectAction.addAction(action3);

    delete selectAction.removeAction(action1);
    delete selectAction.removeAction(action2);
    delete selectAction.removeAction(action3);

    QVERIFY(!widget->isEnabled());
    QCOMPARE(widget->actions().count(), 1);
    QCOMPARE(widget->actions().at(0)->text(), QString("selectAction"));
}
