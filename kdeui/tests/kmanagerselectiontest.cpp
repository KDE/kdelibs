/*
    This file is part of the KDE Libraries

    Copyright (C) 2009 Lubos Lunak <l.lunak@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "kmanagerselectiontest.h"

#include <QtTest>

#include <kapplication.h>
#include <kmanagerselection.h>
#include <qx11info_x11.h>

#define SNAME "_KDE_KMANAGERSELECTIONTEST"

using namespace QTest;

void KManagerSelectionTest::testAcquireRelease()
    { // test that newOwner() is emitted when there is a new selection owner
    KSelectionWatcher watcher( SNAME );
    KSelectionOwner owner( SNAME );
    QVERIFY( owner.ownerWindow() == None );
    QVERIFY( watcher.owner() == None );
    SigCheckWatcher sw( watcher );
    SigCheckOwner so( owner );
    QVERIFY( owner.claim( false ));
    QVERIFY( kWaitForSignal( &watcher, SIGNAL( newOwner( Window )), 2000 ));
    QVERIFY( sw.newowner == true );
    QVERIFY( sw.lostowner == false );
    QVERIFY( so.lostownership == false );
    }

void KManagerSelectionTest::testInitiallyOwned()
    { // test that lostOwner() is emitted when the selection is disowned
    KSelectionOwner owner( SNAME );
    SigCheckOwner so( owner );
    QVERIFY( owner.claim( false ));
    KSelectionWatcher watcher( SNAME );
    SigCheckWatcher sw( watcher );
    owner.release();
    QVERIFY( kWaitForSignal( &watcher, SIGNAL( lostOwner()), 2000 ));
    QVERIFY( sw.newowner == false );
    QVERIFY( sw.lostowner == true );
    QVERIFY( so.lostownership == false );
    }

void KManagerSelectionTest::testLostOwnership()
    { // test that lostOwnership() is emitted when something else forces taking the ownership
    KSelectionOwner owner1( SNAME );
    KSelectionOwner owner2( SNAME );
    QVERIFY( owner1.claim( false ));
    QVERIFY( !owner2.claim( false ));
    XEvent ev;
    ev.xselectionclear.type = SelectionClear;
    ev.xselectionclear.serial = XLastKnownRequestProcessed( QX11Info::display());
    ev.xselectionclear.send_event = True;
    ev.xselectionclear.display = QX11Info::display();
    ev.xselectionclear.window = owner1.ownerWindow();
    ev.xselectionclear.selection = XInternAtom( QX11Info::display(), SNAME, False );
    ev.xselectionclear.time = QX11Info::appTime();
    QVERIFY( owner2.claim( true, false ));
    // SelectionClear event is not sent to the same X client, so fake it
    XPutBackEvent( QX11Info::display(), &ev );
    QVERIFY( kWaitForSignal( &owner1, SIGNAL( lostOwnership()), 2000 ));
    QVERIFY( owner1.ownerWindow() == None );
    QVERIFY( owner2.ownerWindow() != None );
    }

void KManagerSelectionTest::testWatching()
    { // test that KSelectionWatcher reports changes properly
    KSelectionWatcher watcher( SNAME );
    KSelectionOwner owner1( SNAME );
    KSelectionOwner owner2( SNAME );
    SigCheckWatcher sw( watcher );
    QVERIFY( owner1.claim( false ));
    QVERIFY( kWaitForSignal( &watcher, SIGNAL( newOwner( Window )), 2000 ));
    QVERIFY( sw.newowner == true );
    QVERIFY( sw.lostowner == false );
    sw.newowner = sw.lostowner = false;
    QVERIFY( owner2.claim( true, false ));
    QVERIFY( kWaitForSignal( &watcher, SIGNAL( newOwner( Window )), 2000 ));
    QVERIFY( sw.newowner == true );
    QVERIFY( sw.lostowner == false );
    sw.newowner = sw.lostowner = false;
    owner2.release();
    QVERIFY( kWaitForSignal( &watcher, SIGNAL( lostOwner()), 2000 ));
    QVERIFY( sw.newowner == false );
    QVERIFY( sw.lostowner == true );
    sw.newowner = sw.lostowner = false;
    QVERIFY( owner2.claim( false ));
    QVERIFY( kWaitForSignal( &watcher, SIGNAL( newOwner( Window )), 2000 ));
    QVERIFY( sw.newowner == true );
    QVERIFY( sw.lostowner == false );
    }

SigCheckOwner::SigCheckOwner( const KSelectionOwner& owner )
    : lostownership( false )
    {
    connect( &owner, SIGNAL( lostOwnership()), this, SLOT( lostOwnership()));
    }

void SigCheckOwner::lostOwnership()
    {
    lostownership = true;
    }

SigCheckWatcher::SigCheckWatcher( const KSelectionWatcher& watcher )
    : newowner( false )
    , lostowner( false )
    {
    connect( &watcher, SIGNAL( newOwner( Window )), this, SLOT( newOwner()));
    connect( &watcher, SIGNAL( lostOwner()), this, SLOT( lostOwner()));
    }

void SigCheckWatcher::newOwner()
    {
    newowner = true;
    }

void SigCheckWatcher::lostOwner()
    {
    lostowner = true;
    }


#include <kapplication.h>

// the tested classes need KApplication - this is from qtest_kde.h, with QApp -> KApp
#define QTEST_KDEMAIN_WITH_COMPONENTNAME_KAPP(TestObject, flags, componentName) \
int main(int argc, char *argv[]) \
{ \
    setenv("LC_ALL", "C", 1); \
    assert( !QDir::homePath().isEmpty() ); \
    setenv("KDEHOME", QFile::encodeName( QDir::homePath() + QLatin1String("/.kde-unit-test") ), 1); \
    setenv("XDG_DATA_HOME", QFile::encodeName( QDir::homePath() + QLatin1String("/.kde-unit-test/xdg/local") ), 1); \
    setenv("XDG_CONFIG_HOME", QFile::encodeName( QDir::homePath() + QLatin1String("/.kde-unit-test/xdg/config") ), 1); \
    setenv("KDE_SKIP_KDERC", "1", 1); \
    unsetenv("KDE_COLOR_DEBUG"); \
    QFile::remove(QDir::homePath() + QLatin1String("/.kde-unit-test/share/config/qttestrc"));  \
    KAboutData aboutData( QByteArray(componentName), QByteArray(), ki18n("KDE Test Program"), QByteArray("version") );  \
    KDEMainFlags mainFlags = flags;                         \
    KCmdLineArgs::init( argc, argv, &aboutData); \
    KApplication app; \
    app.setApplicationName( QLatin1String("qttest") ); \
    qRegisterMetaType<KUrl>(); /*as done by kapplication*/ \
    qRegisterMetaType<KUrl::List>(); \
    TestObject tc; \
    KGlobal::ref(); /* don't quit qeventloop after closing a mainwindow */ \
    return QTest::qExec( &tc, argc, argv ); \
}

#define QTEST_KDEMAIN_KAPP(TestObject, flags) QTEST_KDEMAIN_WITH_COMPONENTNAME_KAPP(TestObject, flags, "qttest")

QTEST_KDEMAIN_KAPP(KManagerSelectionTest, GUI)
