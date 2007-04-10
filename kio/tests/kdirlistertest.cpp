/* This file is part of the KDE project
   Copyright (C) 2007 David Faure <faure@kde.org>

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

#include "kdirlistertest.h"
#include "kdirlistertest.moc"
#include <kdirlister.h>
#include <qtest_kde.h>

QTEST_KDEMAIN( KDirListerTest, NoGUI )

#include <kdebug.h>
#include "kiotesthelper.h"

void KDirListerTest::initTestCase()
{
    s_referenceTimeStamp = QDateTime::currentDateTime().addSecs( -30 ); // 30 seconds ago

    // Create test data:
    /*
     * PATH/toplevelfile_1
     * PATH/toplevelfile_2
     * PATH/toplevelfile_3
     * PATH/subdir
     * PATH/subdir/testfile
     * PATH/subdir/subsubdir
     * PATH/subdir/subsubdir/testfile
     */
    const QString path = m_tempDir.name();
    createTestFile(path+"toplevelfile_1");
    createTestFile(path+"toplevelfile_2");
    createTestFile(path+"toplevelfile_3");
    createTestDirectory(path+"subdir");
    createTestDirectory(path+"subdir/subsubdir");

    // Hmmpf.
    qRegisterMetaType<KUrl>();
}

void KDirListerTest::testOpenUrl()
{
    m_items.clear();
    const QString path = m_tempDir.name();
    QSignalSpy spyStarted(&m_dirLister, SIGNAL(started(KUrl)));
    QSignalSpy spyClear(&m_dirLister, SIGNAL(clear()));
    QSignalSpy spyCompleted(&m_dirLister, SIGNAL(completed()));
    QSignalSpy spyCompletedKUrl(&m_dirLister, SIGNAL(completed(KUrl)));
    QSignalSpy spyCanceled(&m_dirLister, SIGNAL(canceled()));
    QSignalSpy spyCanceledKUrl(&m_dirLister, SIGNAL(canceled(KUrl)));
    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));
    // The call to openUrl itself, emits started
    m_dirLister.openUrl(KUrl(path), false, false /*reload*/);

    QCOMPARE(spyStarted.count(), 1);
    QCOMPARE(spyCompleted.count(), 0);
    QCOMPARE(spyCompletedKUrl.count(), 0);
    QCOMPARE(spyCanceled.count(), 0);
    QCOMPARE(spyCanceledKUrl.count(), 0);
    QCOMPARE(spyClear.count(), 1);
    QCOMPARE(m_items.count(), 0);

    // then wait for completed
    qDebug("waiting for completed");
    connect(&m_dirLister, SIGNAL(completed()), this, SLOT(exitLoop()));
    enterLoop();
    QCOMPARE(spyStarted.count(), 1);
    QCOMPARE(spyCompleted.count(), 1);
    QCOMPARE(spyCompletedKUrl.count(), 1);
    QCOMPARE(spyCanceled.count(), 0);
    QCOMPARE(spyCanceledKUrl.count(), 0);
    QCOMPARE(spyClear.count(), 1);
    QCOMPARE(m_items.count(), 4);
}

void KDirListerTest::testOpenAndStop()
{
    // TODO
}

void KDirListerTest::enterLoop()
{
    qDebug("enterLoop");
    m_eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
}

void KDirListerTest::exitLoop()
{
    qDebug("exitLoop");
    m_eventLoop.quit();
}

void KDirListerTest::slotNewItems(const KFileItemList& lst)
{
    m_items += lst;
}
