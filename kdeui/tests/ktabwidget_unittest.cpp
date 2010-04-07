/* This file is part of the KDE libraries

    Copyright (c) 2008 David Faure <faure@kde.org>

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

#include <kdebug.h>
#include <qtest_kde.h>
#include <qtestevent.h>
#include <ktabwidget.h>
#include <klineedit.h>
#include <QComboBox>
#include <QtGui/QTabBar>

class KTabWidget_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    // Test calling tabText() from slotCurrentChanged().
    // Because QTabBar::insertTab() emits currentChanged before
    // QTabWidget calls tabInserted(), we were getting an assert in tabText().
    void testTabTextFromSlotCurrentChanged()
    {
        KTabWidget w;
        w.setAutomaticResizeTabs(true);
        const QString firstTitle = "First post!";
        connect(&w, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChanged(int)));
        const int zero = w.insertTab(0, new QWidget, firstTitle); // calls slotCurrentChanged
        QCOMPARE(zero, 0);
        QCOMPARE(w.tabText(0), firstTitle);
    }

    void testCloseFirstTab()
    {
        // Test inspired by #170470, but the bug only happened when calling setTabText
        // in slotCurrentChanged so the real unit test for that bug is in konqueror.
        KTabWidget w;
        w.setAutomaticResizeTabs(true);
        w.resize(300, 400);
        // Send the pending resize event (resize() only sets Qt::WA_PendingResizeEvent)
        QResizeEvent e(w.size(), QSize());
        QApplication::sendEvent(&w, &e);

        QString prefix = "This is a long prefix for the tab title. ";
        for (int i = 0; i < 6; ++i)
            w.insertTab(i, new QWidget, prefix+QString::number(i));
        w.removeTab(0);
        for (int i = 0; i < 5; ++i) {
            //kDebug() << i << w.tabText(i);
            QCOMPARE(w.tabText(i), prefix+QString::number(i+1));
        }
        w.removeTab(0);
        for (int i = 0; i < 4; ++i) {
            //kDebug() << i << w.tabText(i);
            QCOMPARE(w.tabText(i), prefix+QString::number(i+2));
        }
    }
    void testMoveTab()
    {
        // Test inspired by #170470 and #177036 (title messup).
        // Then expanded to include the problem of #159295 (focus loss).
        QWidget topLevel;
        QComboBox* combo = new QComboBox(&topLevel);
        combo->setEditable(true);
        KTabWidget* w = new KTabWidget(&topLevel);
        w->setAutomaticResizeTabs(true);
        w->resize(300, 400);
        QResizeEvent e(w->size(), QSize());
        QApplication::sendEvent(w, &e);
        QString prefix = "This is a long prefix for the tab title. ";
        KLineEdit* lineEdits[4];
        for (int i = 0; i < 4; ++i) {
            QWidget* page = new QWidget;
            page->setObjectName(QString::number(i));
            lineEdits[i] = new KLineEdit(page); // a widget that can take focus
            lineEdits[i]->setObjectName("LineEdit"+QString::number(i));
            w->insertTab(i, page, prefix+QString::number(i));
            //kDebug() << i << w->tabText(i);
        }
        topLevel.show();
        // Necessary after show(), otherwise topLevel.focusWidget() returns NULL
        QApplication::setActiveWindow(&topLevel);

        // Make sure the tab order is so that the combo gets focus after this tab
        QWidget::setTabOrder(lineEdits[0], combo->lineEdit());
        QWidget::setTabOrder(combo->lineEdit(), lineEdits[1]);

        w->setCurrentIndex(0);
        lineEdits[0]->setFocus();
        QCOMPARE(topLevel.focusWidget()->objectName(), lineEdits[0]->objectName());
        QVERIFY(lineEdits[0]->isVisible());

        w->moveTab(0,3);
        //for (int i = 0; i < 4; ++i)
            //kDebug() << i << w->tabText(i);
        QCOMPARE(w->tabText(0), prefix+QString::number(1));
        QCOMPARE(w->tabText(1), prefix+QString::number(2));
        QCOMPARE(w->tabText(2), prefix+QString::number(3));
        QCOMPARE(w->tabText(3), prefix+QString::number(0));

        // Did the focus switch to the lineEdit, due to removeTab+insertTab? Whoops.
        QCOMPARE(topLevel.focusWidget()->objectName(), lineEdits[0]->objectName());

        w->moveTab(3,0);
        QCOMPARE(topLevel.focusWidget()->objectName(), lineEdits[0]->objectName());
        for (int i = 0; i < 4; ++i) {
            //kDebug() << i << w->tabText(i);
            QCOMPARE(w->tabText(i), prefix+QString::number(i));
        }
     }

     void testSetHidden()
     {
         KTabWidget w;
         w.insertTab(0, new QWidget, "a tab");
         QVERIFY(!w.isTabBarHidden());
         w.show();
         QVERIFY(!w.isTabBarHidden());
         w.setTabBarHidden(true);
         QVERIFY(w.isTabBarHidden());
     }

     void testMiddleClickTabReordering();

private Q_SLOTS:
    void slotCurrentChanged(int index)
    {
        QCOMPARE(index, 0);
        KTabWidget* w = qobject_cast<KTabWidget *>(sender());
        QVERIFY(w);
        QCOMPARE(w->tabText(0), QString("First post!"));
    }
};

//  MyTabWidget is a tab widget that provides access to the tab bar.
// This is needed for the following unit test.

class MyTabWidget : public KTabWidget {

public:
    QTabBar* getTabBar() const {
        return tabBar();
    }
};

void KTabWidget_UnitTest::testMiddleClickTabReordering()
{
    MyTabWidget tabWidget;
    tabWidget.setTabReorderingEnabled(true);

    QWidget* w0 = new QWidget;
    QWidget* w1 = new QWidget;
    tabWidget.insertTab(0, w0, "Tab 0");
    tabWidget.insertTab(1, w1, "Tab 1");
    tabWidget.show();

    QPoint pos0 = tabWidget.getTabBar()->tabRect(0).center();
    QPoint pos1 = tabWidget.getTabBar()->tabRect(1).center();

    // Press MMB
    QTest::mousePress(tabWidget.getTabBar(), Qt::MidButton, Qt::NoModifier, pos0);

    // We need a first move event on tab 0 to initialize tab moving
    QMouseEvent moveEvent(QEvent::MouseMove, pos0, Qt::NoButton, Qt::MidButton, Qt::NoModifier);
    bool moveEventReceived = qApp->notify(tabWidget.getTabBar(), &moveEvent);
    QVERIFY(moveEventReceived);

    // Move tab 0 to tab 1
    moveEvent = QMouseEvent(QEvent::MouseMove, pos1, Qt::NoButton, Qt::MidButton, Qt::NoModifier);
    moveEventReceived = qApp->notify(tabWidget.getTabBar(), &moveEvent);
    QVERIFY(moveEventReceived);

    QCOMPARE(tabWidget.tabText(0), QString("Tab 1"));
    QCOMPARE(tabWidget.tabText(1), QString("Tab 0"));
    QCOMPARE(tabWidget.widget(0), w1);
    QCOMPARE(tabWidget.widget(1), w0);

    QTest::mouseRelease(tabWidget.getTabBar(), Qt::MidButton, Qt::NoModifier, pos1);
}

QTEST_KDEMAIN(KTabWidget_UnitTest, GUI)

#include "ktabwidget_unittest.moc"
