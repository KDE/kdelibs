/* This file is part of the KDE libraries
    Copyright (c) 2006 David Faure <faure@kde.org>

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

#include "qtest_kde.h"
#include "kmainwindow_unittest.h"
#include "kmainwindow_unittest.moc"
#include <kmainwindow.h>
#include <kstatusbar.h>
#include <kglobal.h>
#include <QResizeEvent>
#include <ktoolbar.h>
#include <kconfiggroup.h>

QTEST_KDEMAIN( KMainWindow_UnitTest, GUI )

void KMainWindow_UnitTest::testDefaultName()
{
    KMainWindow mw;
    mw.show();
    mw.ensurePolished();
    QCOMPARE( mw.objectName(), QString::fromLatin1( "MainWindow#1" ) );
    KMainWindow mw2;
    mw2.show();
    mw2.ensurePolished();
    QCOMPARE( mw2.objectName(), QString::fromLatin1( "MainWindow#2" ) );
}

void KMainWindow_UnitTest::testFixedName()
{
    KMainWindow mw;
    mw.setObjectName( "mymainwindow" );
    mw.show();
    mw.ensurePolished();
    QCOMPARE( mw.objectName(), QString::fromLatin1( "mymainwindow" ) );
    KMainWindow mw2;
    mw2.setObjectName( "mymainwindow" );
    mw2.show();
    mw2.ensurePolished();
    QCOMPARE( mw2.objectName(), QString::fromLatin1( "mymainwindow2" ) );
}

void KMainWindow_UnitTest::testNameWithHash()
{
    KMainWindow mw;
    mw.setObjectName( "composer#" );
    mw.show();
    mw.ensurePolished();
    QCOMPARE( mw.objectName(), QString::fromLatin1( "composer#1" ) );
    KMainWindow mw2;
    mw2.setObjectName( "composer#" );
    mw2.show();
    mw2.ensurePolished();
    QCOMPARE( mw2.objectName(), QString::fromLatin1( "composer#2" ) );
    KMainWindow mw4;
    mw4.setObjectName( "composer#4" );
    mw4.show();
    mw4.ensurePolished();
    QCOMPARE( mw4.objectName(), QString::fromLatin1( "composer#4" ) );
}

void KMainWindow_UnitTest::testNameWithSpecialChars()
{
    KMainWindow mw;
    mw.setObjectName( "a#@_test/" );
    mw.show();
    mw.ensurePolished();
    QCOMPARE( mw.dbusName(), QString::fromLatin1( "/qttest/a___test_" ) );
    KMainWindow mw2;
    mw2.setObjectName( "a#@_test/" );
    mw2.show();
    mw2.ensurePolished();
    QCOMPARE( mw2.dbusName(), QString::fromLatin1( "/qttest/a___test_2" ) );
}

static bool s_mainWindowDeleted;
class MyMainWindow : public KMainWindow
{
public:
    MyMainWindow() : KMainWindow(),
                     m_queryClosedCalled( false ),
                     m_queryExitCalled( false )
    {
    }
    /*reimp*/ bool queryClose() {
        m_queryClosedCalled = true;
        return true;
    }
    /*reimp*/ bool queryExit() {
        m_queryExitCalled = true;
        return true;
    }
    ~MyMainWindow() {
        s_mainWindowDeleted = true;
    }
    bool m_queryClosedCalled;
    bool m_queryExitCalled;

    void reallyResize(int width, int height) {
        const QSize oldSize = size();

        resize(width, height);

        // Send the pending resize event (resize() only sets Qt::WA_PendingResizeEvent)
        QResizeEvent e(size(), oldSize);
        QApplication::sendEvent(this, &e);
    }
};

// Here we test
// - that queryClose is called
// - that queryExit is called
// - that autodeletion happens
void KMainWindow_UnitTest::testDeleteOnClose()
{
    KGlobal::ref(); // don't let the deref in KMainWindow quit the app.
    s_mainWindowDeleted = false;
    MyMainWindow* mw = new MyMainWindow;
    QVERIFY( mw->testAttribute( Qt::WA_DeleteOnClose ) );
    mw->close();
    QVERIFY( mw->m_queryClosedCalled );
    QVERIFY( mw->m_queryExitCalled );
    qApp->sendPostedEvents( mw, QEvent::DeferredDelete );
    QVERIFY( s_mainWindowDeleted );
}

void KMainWindow_UnitTest::testSaveWindowSize()
{
    MyMainWindow mw;
    KToolBar* tb = new KToolBar(&mw); // we need a toolbar to trigger an old bug in saveMainWindowSettings
    tb->setObjectName("testtb");
    mw.reallyResize(800, 600);

    KConfigGroup cfg(KGlobal::config(), "TestWindowSize");
    mw.saveMainWindowSettings(cfg);
    mw.close();

    KMainWindow mw2;
    tb = new KToolBar(&mw2);
    tb->setObjectName("testtb");
    mw2.resize(50, 50);
    mw2.applyMainWindowSettings(cfg);
    QCOMPARE(mw2.size(), QSize(800, 600));
}

void KMainWindow_UnitTest::testAutoSaveSettings()
{
    MyMainWindow mw;
    KToolBar* tb = new KToolBar(&mw); // we need a toolbar to trigger an old bug in saveMainWindowSettings
    tb->setObjectName("testtb");
    const QString group("AutoSaveTestGroup");
    mw.setAutoSaveSettings(group);
    mw.reallyResize(800, 600);
    mw.close();

    KMainWindow mw2;
    tb = new KToolBar(&mw2);
    tb->setObjectName("testtb");
    mw2.setAutoSaveSettings(group);
    QCOMPARE(mw2.size(), QSize(800, 600));
}

void KMainWindow_UnitTest::testNoAutoSave()
{
    // A mainwindow with autosaving, but not of the window size.
    MyMainWindow mw;
    const QString group("AutoSaveTestGroup");
    mw.setAutoSaveSettings(group, false);
    mw.reallyResize(750, 550);
    mw.close();

    KMainWindow mw2;
    mw2.setAutoSaveSettings(group, false);
    // NOT 750, 550! (the 800,600 comes from testAutoSaveSettings)
    QCOMPARE(mw2.size(), QSize(800, 600));
}

void KMainWindow_UnitTest::testWidgetWithStatusBar()
{
    // KMainWindow::statusBar() should not find any indirect KStatusBar child
    // (e.g. in a case like konqueror, with one statusbar per frame)
    MyMainWindow mw;
    QWidget* frame1 = new QWidget(&mw);
    KStatusBar* frameStatusBar = new KStatusBar(frame1);
    QVERIFY(mw.statusBar() != frameStatusBar);
}
