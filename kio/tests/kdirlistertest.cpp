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
#include <ktemporaryfile.h>
#include "kdirlistertest.moc"
#include <kdirlister.h>
#include <qtest_kde.h>

QTEST_KDEMAIN( KDirListerTest, NoGUI )

#include <kdebug.h>
#include "kiotesthelper.h"
#include <kio/deletejob.h>
#include <kdirwatch.h>
#include <kio/job.h>

Q_DECLARE_METATYPE(KFileItemList)

void MyDirLister::handleError(KIO::Job* job)
{
    // Currently we don't expect any errors.
    kFatal() << job << job->errorString();
}

void KDirListerTest::initTestCase()
{
    m_exitCount = 1;

    s_referenceTimeStamp = QDateTime::currentDateTime().addSecs( -120 ); // 2 minutes ago

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
    qRegisterMetaType<KFileItemList>();
}

void KDirListerTest::testOpenUrl()
{
    m_items.clear();
    const QString path = m_tempDir.name();
    QSignalSpy spyStarted(&m_dirLister, SIGNAL(started(KUrl)));
    QSignalSpy spyClear(&m_dirLister, SIGNAL(clear()));
    QSignalSpy spyClearKUrl(&m_dirLister, SIGNAL(clear(KUrl)));
    QSignalSpy spyCompleted(&m_dirLister, SIGNAL(completed()));
    QSignalSpy spyCompletedKUrl(&m_dirLister, SIGNAL(completed(KUrl)));
    QSignalSpy spyCanceled(&m_dirLister, SIGNAL(canceled()));
    QSignalSpy spyCanceledKUrl(&m_dirLister, SIGNAL(canceled(KUrl)));
    QSignalSpy spyRedirection(&m_dirLister, SIGNAL(redirection(KUrl)));
    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));
    // The call to openUrl itself, emits started
    m_dirLister.openUrl(KUrl(path), KDirLister::NoFlags);

    QCOMPARE(spyStarted.count(), 1);
    QCOMPARE(spyCompleted.count(), 0);
    QCOMPARE(spyCompletedKUrl.count(), 0);
    QCOMPARE(spyCanceled.count(), 0);
    QCOMPARE(spyCanceledKUrl.count(), 0);
    QCOMPARE(spyClear.count(), 1);
    QCOMPARE(spyClearKUrl.count(), 0);
    QCOMPARE(spyRedirection.count(), 0);
    QCOMPARE(m_items.count(), 0);
    QVERIFY(!m_dirLister.isFinished());

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
    QCOMPARE(spyClearKUrl.count(), 0);
    QCOMPARE(spyRedirection.count(), 0);
    QCOMPARE(m_items.count(), fileCount());
    QVERIFY(m_dirLister.isFinished());
    disconnect(&m_dirLister, 0, this, 0);

    QVERIFY(!m_dirLister.findByName("toplevelfile_3").isNull());
}

// This test assumes testOpenUrl was run before. So m_dirLister is holding the items already.
void KDirListerTest::testOpenUrlFromCache()
{
    // Do the same again, it should behave the same, even with the items in the cache
    testOpenUrl();

    // Get into the case where another dirlister is holding the items
    {
        m_items.clear();
        const QString path = m_tempDir.name();
        KDirLister secondDirLister;
        QSignalSpy spyStarted(&secondDirLister, SIGNAL(started(KUrl)));
        QSignalSpy spyClear(&secondDirLister, SIGNAL(clear()));
        QSignalSpy spyClearKUrl(&secondDirLister, SIGNAL(clear(KUrl)));
        QSignalSpy spyCompleted(&secondDirLister, SIGNAL(completed()));
        QSignalSpy spyCompletedKUrl(&secondDirLister, SIGNAL(completed(KUrl)));
        QSignalSpy spyCanceled(&secondDirLister, SIGNAL(canceled()));
        QSignalSpy spyCanceledKUrl(&secondDirLister, SIGNAL(canceled(KUrl)));
        connect(&secondDirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));
        secondDirLister.openUrl(KUrl(path), KDirLister::NoFlags);
        QCOMPARE(spyStarted.count(), 1);
        QCOMPARE(spyCompleted.count(), 0);
        QCOMPARE(spyCompletedKUrl.count(), 0);
        QCOMPARE(spyCanceled.count(), 0);
        QCOMPARE(spyCanceledKUrl.count(), 0);
        QCOMPARE(spyClear.count(), 1);
        QCOMPARE(spyClearKUrl.count(), 0);
        QCOMPARE(m_items.count(), 0);
        QVERIFY(!secondDirLister.isFinished());

        // then wait for completed
        qDebug("waiting for completed");
        connect(&secondDirLister, SIGNAL(completed()), this, SLOT(exitLoop()));
        enterLoop();
        QCOMPARE(spyStarted.count(), 1);
        QCOMPARE(spyCompleted.count(), 1);
        QCOMPARE(spyCompletedKUrl.count(), 1);
        QCOMPARE(spyCanceled.count(), 0);
        QCOMPARE(spyCanceledKUrl.count(), 0);
        QCOMPARE(spyClear.count(), 1);
        QCOMPARE(spyClearKUrl.count(), 0);
        QCOMPARE(m_items.count(), 4);
        QVERIFY(secondDirLister.isFinished());
    }

    disconnect(&m_dirLister, 0, this, 0);
}

// This test assumes testOpenUrl was run before. So m_dirLister is holding the items already.
void KDirListerTest::testNewItems()
{
    QCOMPARE(m_items.count(), 4);
    const QString path = m_tempDir.name();
    QSignalSpy spyStarted(&m_dirLister, SIGNAL(started(KUrl)));
    QSignalSpy spyClear(&m_dirLister, SIGNAL(clear()));
    QSignalSpy spyClearKUrl(&m_dirLister, SIGNAL(clear(KUrl)));
    QSignalSpy spyCompleted(&m_dirLister, SIGNAL(completed()));
    QSignalSpy spyCompletedKUrl(&m_dirLister, SIGNAL(completed(KUrl)));
    QSignalSpy spyCanceled(&m_dirLister, SIGNAL(canceled()));
    QSignalSpy spyCanceledKUrl(&m_dirLister, SIGNAL(canceled(KUrl)));
    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));

    QTest::qWait(1000); // We need a 1s timestamp difference on the dir, otherwise FAM won't notice anything.

    kDebug() << "Creating new file";
    QFile file(path+"toplevelfile_new");
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QByteArray("foo"));
    file.close();

    int numTries = 0;
    // Give time for KDirWatch to notify us
    while (m_items.count() == 4) {
        QVERIFY(++numTries < 10);
        QTest::qWait(200);
    }
    //kDebug() << "numTries=" << numTries;
    QCOMPARE(m_items.count(), 5);

    QCOMPARE(spyStarted.count(), 1); // Updates call started
    QCOMPARE(spyCompleted.count(), 1); // and completed
    QCOMPARE(spyCompletedKUrl.count(), 1);
    QCOMPARE(spyCanceled.count(), 0);
    QCOMPARE(spyCanceledKUrl.count(), 0);
    QCOMPARE(spyClear.count(), 0);
    QCOMPARE(spyClearKUrl.count(), 0);
    disconnect(&m_dirLister, 0, this, 0);
}

void KDirListerTest::testNewItemByCopy()
{
    // This test creates a file using KIO::copyAs, like knewmenu.cpp does.
    // Useful for testing #192185, i.e. whether we catch the kdirwatch event and avoid
    // a KFileItem::refresh().
    const int origItemCount = m_items.count();
    const QString path = m_tempDir.name();
    QSignalSpy spyStarted(&m_dirLister, SIGNAL(started(KUrl)));
    QSignalSpy spyClear(&m_dirLister, SIGNAL(clear()));
    QSignalSpy spyClearKUrl(&m_dirLister, SIGNAL(clear(KUrl)));
    QSignalSpy spyCompleted(&m_dirLister, SIGNAL(completed()));
    QSignalSpy spyCompletedKUrl(&m_dirLister, SIGNAL(completed(KUrl)));
    QSignalSpy spyCanceled(&m_dirLister, SIGNAL(canceled()));
    QSignalSpy spyCanceledKUrl(&m_dirLister, SIGNAL(canceled(KUrl)));
    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));

    QTest::qWait(1000); // We need a 1s timestamp difference on the dir, otherwise FAM won't notice anything.

    const QString fileName = "toplevelfile_copy";
    const KUrl itemUrl(path + fileName);
    KIO::CopyJob* job = KIO::copyAs(path+"toplevelfile_3", itemUrl, KIO::HideProgressInfo);
    job->exec();

    int numTries = 0;
    // Give time for KDirWatch/KDirNotify to notify us
    while (m_items.count() == origItemCount) {
        QVERIFY(++numTries < 10);
        QTest::qWait(200);
    }
    //kDebug() << "numTries=" << numTries;
    QCOMPARE(m_items.count(), origItemCount+1);

    QCOMPARE(spyStarted.count(), 1); // Updates call started
    QCOMPARE(spyCompleted.count(), 1); // and completed
    QCOMPARE(spyCompletedKUrl.count(), 1);
    QCOMPARE(spyCanceled.count(), 0);
    QCOMPARE(spyCanceledKUrl.count(), 0);
    QCOMPARE(spyClear.count(), 0);
    QCOMPARE(spyClearKUrl.count(), 0);
    disconnect(&m_dirLister, 0, this, 0);

    // Give some time to KDirWatch
    QTest::qWait(1000);

    KFileItem itemForUrl = KDirLister::cachedItemForUrl(itemUrl);
    QVERIFY(!itemForUrl.isNull());
    QCOMPARE(itemForUrl.url().url(), itemUrl.url());
    QCOMPARE(itemForUrl.entry().stringValue(KIO::UDSEntry::UDS_NAME), fileName);
}

void KDirListerTest::testNewItemsInSymlink() // #213799
{
    const int origItemCount = m_items.count();
    QCOMPARE(fileCount(), origItemCount);
    const QString path = m_tempDir.name();
    KTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    const QString symPath = tempFile.fileName() + "_link";
    tempFile.close();
    bool symlinkOk = ::symlink(QFile::encodeName(path), QFile::encodeName(symPath)) == 0;
    QVERIFY(symlinkOk);
    KDirLister dirLister2;
    m_items2.clear();
    connect(&dirLister2, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems2(KFileItemList)));
    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));

    // The initial listing
    dirLister2.openUrl(KUrl(symPath), KDirLister::NoFlags);
    connect(&dirLister2, SIGNAL(completed()), this, SLOT(exitLoop()));
    enterLoop();
    QCOMPARE(m_items2.count(), origItemCount);
    QVERIFY(dirLister2.isFinished());

    QTest::qWait(1000); // We need a 1s timestamp difference on the dir, otherwise FAM won't notice anything.

    kDebug() << "Creating new file";
    const QString fileName = "toplevelfile_newinlink";
    QFile file(path + fileName);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QByteArray("foo"));
    file.close();

    org::kde::KDirNotify::emitFilesAdded(path); // ### workaround for broken inotify
    int numTries = 0;
    // Give time for KDirWatch to notify us
    while (m_items2.count() == origItemCount) {
        QVERIFY(++numTries < 10);
        QTest::qWait(200);
    }
    //kDebug() << "numTries=" << numTries;
    QCOMPARE(m_items2.count(), origItemCount+1);
    QCOMPARE(m_items.count(), origItemCount+1);

    // Now create an item using the symlink-path
    const QString fileName2 = "toplevelfile_newinlink2";
    {
        QFile file(path + fileName2);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QByteArray("foo"));
        file.close();

        int numTries = 0;
        // Give time for KDirWatch to notify us
        while (m_items2.count() == origItemCount + 1) {
            QVERIFY(++numTries < 10);
            QTest::qWait(200);
        }
        QCOMPARE(m_items2.count(), origItemCount+2);
        QCOMPARE(m_items.count(), origItemCount+2);
        disconnect(&m_dirLister, 0, this, 0);
    }
    QCOMPARE(fileCount(), m_items.count());

    // TODO: test file deletion, and file update.
}

// This test assumes testOpenUrl was run before. So m_dirLister is holding the items already.
void KDirListerTest::testRefreshItems()
{
    m_refreshedItems.clear();

    const QString path = m_tempDir.name();
    const QString fileName = path+"toplevelfile_2";
    KFileItem cachedItem = m_dirLister.findByUrl(KUrl(fileName));
    QVERIFY(!cachedItem.isNull());
    QCOMPARE(cachedItem.mimetype(), QString("application/octet-stream"));

    QSignalSpy spyStarted(&m_dirLister, SIGNAL(started(KUrl)));
    QSignalSpy spyClear(&m_dirLister, SIGNAL(clear()));
    QSignalSpy spyClearKUrl(&m_dirLister, SIGNAL(clear(KUrl)));
    QSignalSpy spyCompleted(&m_dirLister, SIGNAL(completed()));
    QSignalSpy spyCompletedKUrl(&m_dirLister, SIGNAL(completed(KUrl)));
    QSignalSpy spyCanceled(&m_dirLister, SIGNAL(canceled()));
    QSignalSpy spyCanceledKUrl(&m_dirLister, SIGNAL(canceled(KUrl)));
    connect(&m_dirLister, SIGNAL(refreshItems(const QList<QPair<KFileItem, KFileItem> > &)),
            this, SLOT(slotRefreshItems(const QList<QPair<KFileItem, KFileItem> > &)));

    QFile file(fileName);
    QVERIFY(file.open(QIODevice::Append));
    file.write(QByteArray("<html>"));
    file.close();
    QCOMPARE(QFileInfo(fileName).size(), 11LL /*Hello world*/ + 6 /*<html>*/);

    // KDirWatch doesn't make this work when using FAM :(
    //KDirWatch::self()->setDirty(path+"toplevelfile_2"); // hack
    KDirWatch::self()->setDirty(path); // with only the file, we get into the new fast path that doesn't even emit started...

    int numTries = 0;
    // Give time for KDirWatch to notify us
    while (m_refreshedItems.isEmpty()) {
        QVERIFY(++numTries < 10);
        QTest::qWait(200);
    }

    QCOMPARE(spyStarted.count(), 1); // Updates (to a directory) call started...
    QCOMPARE(spyCompleted.count(), 1); // and completed
    QCOMPARE(spyCompletedKUrl.count(), 1);
    QCOMPARE(spyCanceled.count(), 0);
    QCOMPARE(spyCanceledKUrl.count(), 0);
    QCOMPARE(spyClear.count(), 0);
    QCOMPARE(spyClearKUrl.count(), 0);
    QCOMPARE(m_refreshedItems.count(), 1);
    QPair<KFileItem, KFileItem> entry = m_refreshedItems.first();
    QCOMPARE(entry.first.url().path(), fileName);
    QCOMPARE(entry.first.size(), KIO::filesize_t(11));
    QCOMPARE(entry.first.mimetype(), QString("application/octet-stream"));
    QCOMPARE(entry.second.url().path(), fileName);
    QCOMPARE(entry.second.size(), KIO::filesize_t(11 /*Hello world*/ + 6 /*<html>*/));
    QCOMPARE(entry.second.mimetype(), QString("text/html"));
    disconnect(&m_dirLister, 0, this, 0);

    // Let's see what KDirLister has in cache now
    cachedItem = m_dirLister.findByUrl(KUrl(fileName));
    QCOMPARE(cachedItem.size(), KIO::filesize_t(11 /*Hello world*/ + 6 /*<html>*/));
    m_refreshedItems.clear();
}

// This test assumes testOpenUrl was run before. So m_dirLister is holding the items already.
void KDirListerTest::testDeleteItem()
{
    const int origItemCount = m_items.count();
    QCOMPARE(fileCount(), origItemCount);
    const QString path = m_tempDir.name();
    qRegisterMetaType<KFileItem>("KFileItem");
    QSignalSpy spyDeleteItem(&m_dirLister, SIGNAL(deleteItem(const KFileItem&)));
    QSignalSpy spyItemsDeleted(&m_dirLister, SIGNAL(itemsDeleted(const KFileItemList&)));
    connect(&m_dirLister, SIGNAL(deleteItem(const KFileItem&)), this, SLOT(exitLoop()));

    //kDebug() << "Removing " << path+"toplevelfile_1";
    QFile::remove(path+"toplevelfile_1");
    // the remove() doesn't always trigger kdirwatch in stat mode, if this all happens in the same second
    KDirWatch::self()->setDirty(path);
    if (spyDeleteItem.count() == 0) {
        qDebug("waiting for deleteItem");
        enterLoop();
    }

    QCOMPARE(spyDeleteItem.count(), 1);
    QCOMPARE(spyItemsDeleted.count(), 1);

    // OK now kdirlister told us the file was deleted, let's try a re-listing
    m_items.clear();
    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));
    m_dirLister.openUrl(KUrl(path), KDirLister::NoFlags);
    QVERIFY(!m_dirLister.isFinished());
    connect(&m_dirLister, SIGNAL(completed()), this, SLOT(exitLoop()));
    enterLoop();
    QVERIFY(m_dirLister.isFinished());
    QCOMPARE(m_items.count(), origItemCount-1);

    disconnect(&m_dirLister, 0, this, 0);
    QCOMPARE(fileCount(), m_items.count());
}

void KDirListerTest::testRenameItem()
{
    m_refreshedItems.clear();
    const QString dirPath = m_tempDir.name();
    qRegisterMetaType<KFileItem>("KFileItem");
    connect(&m_dirLister, SIGNAL(refreshItems(const QList<QPair<KFileItem, KFileItem> > &)),
            this, SLOT(slotRefreshItems(const QList<QPair<KFileItem, KFileItem> > &)));
    const QString path = dirPath+"toplevelfile_2";
    const QString newPath = dirPath+"toplevelfile_2.renamed.html";

    KIO::SimpleJob* job = KIO::rename(path, newPath, KIO::HideProgressInfo);
    bool ok = job->exec();
    QVERIFY(ok);

    if (m_refreshedItems.isEmpty()) {
        // Wait for refreshItems. Could come from KDirWatch or KDirNotify.
        //qDebug("waiting for refreshItems");
        connect(this, SIGNAL(refreshItemsReceived()), this, SLOT(exitLoop()));
        enterLoop();
    }

    QCOMPARE(m_refreshedItems.count(), 1);
    QPair<KFileItem, KFileItem> entry = m_refreshedItems.first();
    QCOMPARE(entry.first.url().path(), path);
    QCOMPARE(entry.first.mimetype(), QString("application/octet-stream"));
    QCOMPARE(entry.second.url().path(), newPath);
    QCOMPARE(entry.second.mimetype(), QString("text/html"));
    disconnect(&m_dirLister, 0, this, 0);

    // Let's see what KDirLister has in cache now
    KFileItem cachedItem = m_dirLister.findByUrl(KUrl(newPath));
    QVERIFY(!cachedItem.isNull());
    QCOMPARE(cachedItem.url().path(), newPath);
    KFileItem oldCachedItem = m_dirLister.findByUrl(KUrl(path));
    QVERIFY(oldCachedItem.isNull());
    m_refreshedItems.clear();
}

void KDirListerTest::testRenameAndOverwrite() // has to be run after testRenameItem
{
    // Rename toplevelfile_2.renamed.html to toplevelfile_2, overwriting it.
    const QString dirPath = m_tempDir.name();
    const QString path = dirPath+"toplevelfile_2";
    createTestFile(path);
    KFileItem existingItem;
    while (existingItem.isNull()) {
        QTest::qWait(100);
        existingItem = m_dirLister.findByUrl(KUrl(path));
    };
    QCOMPARE(existingItem.url().path(), path);

    m_refreshedItems.clear();
    qRegisterMetaType<KFileItem>("KFileItem");
    connect(&m_dirLister, SIGNAL(refreshItems(const QList<QPair<KFileItem, KFileItem> > &)),
            this, SLOT(slotRefreshItems(const QList<QPair<KFileItem, KFileItem> > &)));
    QSignalSpy spyItemsDeleted(&m_dirLister, SIGNAL(itemsDeleted(KFileItemList)));
    const QString newPath = dirPath+"toplevelfile_2.renamed.html";

    KIO::SimpleJob* job = KIO::rename(newPath, path, KIO::Overwrite | KIO::HideProgressInfo);
    bool ok = job->exec();
    QVERIFY(ok);

    if (m_refreshedItems.isEmpty()) {
        // Wait for refreshItems. Could come from KDirWatch or KDirNotify.
        //qDebug("waiting for refreshItems");
        connect(this, SIGNAL(refreshItemsReceived()), this, SLOT(exitLoop()));
        enterLoop();
    }

    // Check that itemsDeleted was emitted -- preferrably BEFORE refreshItems,
    // but we can't easily check that with QSignalSpy...
    QCOMPARE(spyItemsDeleted.count(), 1);

    QCOMPARE(m_refreshedItems.count(), 1);
    QPair<KFileItem, KFileItem> entry = m_refreshedItems.first();
    QCOMPARE(entry.first.url().path(), newPath);
    QCOMPARE(entry.second.url().path(), path);
    disconnect(&m_dirLister, 0, this, 0);

    // Let's see what KDirLister has in cache now
    KFileItem cachedItem = m_dirLister.findByUrl(KUrl(path));
    QCOMPARE(cachedItem.url().path(), path);
    KFileItem oldCachedItem = m_dirLister.findByUrl(KUrl(newPath));
    QVERIFY(oldCachedItem.isNull());
    m_refreshedItems.clear();
}

void KDirListerTest::testConcurrentListing()
{
    const int origItemCount = m_items.count();
    QCOMPARE(fileCount(), origItemCount);
    m_items.clear();
    m_items2.clear();

    KDirLister dirLister2;

    const QString path = m_tempDir.name();

    // spy for m_dirLister signals
    QSignalSpy spyStarted1(&m_dirLister, SIGNAL(started(KUrl)));
    QSignalSpy spyClear1(&m_dirLister, SIGNAL(clear()));
    QSignalSpy spyClearKUrl1(&m_dirLister, SIGNAL(clear(KUrl)));
    QSignalSpy spyCompleted1(&m_dirLister, SIGNAL(completed()));
    QSignalSpy spyCompletedKUrl1(&m_dirLister, SIGNAL(completed(KUrl)));
    QSignalSpy spyCanceled1(&m_dirLister, SIGNAL(canceled()));
    QSignalSpy spyCanceledKUrl1(&m_dirLister, SIGNAL(canceled(KUrl)));
    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));

    // spy for dirLister2 signals
    QSignalSpy spyStarted2(&dirLister2, SIGNAL(started(KUrl)));
    QSignalSpy spyClear2(&dirLister2, SIGNAL(clear()));
    QSignalSpy spyClearKUrl2(&dirLister2, SIGNAL(clear(KUrl)));
    QSignalSpy spyCompleted2(&dirLister2, SIGNAL(completed()));
    QSignalSpy spyCompletedKUrl2(&dirLister2, SIGNAL(completed(KUrl)));
    QSignalSpy spyCanceled2(&dirLister2, SIGNAL(canceled()));
    QSignalSpy spyCanceledKUrl2(&dirLister2, SIGNAL(canceled(KUrl)));
    connect(&dirLister2, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems2(KFileItemList)));

    // Before dirLister2 has time to emit the items, let's make m_dirLister move to another dir.
    // This reproduces the use case "clicking on a folder in dolphin iconview, and dirlister2
    // is the one used by the "folder panel". m_dirLister is going to list the subdir,
    // while dirLister2 wants to list the folder that m_dirLister has just left.
    dirLister2.stop(); // like dolphin does, noop.
    dirLister2.openUrl(KUrl(path), KDirLister::NoFlags);
    m_dirLister.openUrl(KUrl(path+"subdir"), KDirLister::NoFlags);

    QCOMPARE(spyStarted1.count(), 1);
    QCOMPARE(spyCompleted1.count(), 0);
    QCOMPARE(spyCompletedKUrl1.count(), 0);
    QCOMPARE(spyCanceled1.count(), 0);
    QCOMPARE(spyCanceledKUrl1.count(), 0);
    QCOMPARE(spyClear1.count(), 1);
    QCOMPARE(spyClearKUrl1.count(), 0);
    QCOMPARE(m_items.count(), 0);

    QCOMPARE(spyStarted2.count(), 1);
    QCOMPARE(spyCompleted2.count(), 0);
    QCOMPARE(spyCompletedKUrl2.count(), 0);
    QCOMPARE(spyCanceled2.count(), 0);
    QCOMPARE(spyCanceledKUrl2.count(), 0);
    QCOMPARE(spyClear2.count(), 1);
    QCOMPARE(spyClearKUrl2.count(), 0);
    QCOMPARE(m_items2.count(), 0);
    QVERIFY(!m_dirLister.isFinished());
    QVERIFY(!dirLister2.isFinished());

    // then wait for completed
    qDebug("waiting for completed");
    connect(&m_dirLister, SIGNAL(completed()), this, SLOT(exitLoop()));
    connect(&dirLister2, SIGNAL(completed()), this, SLOT(exitLoop()));
    enterLoop(2);

    QCOMPARE(spyStarted1.count(), 1);
    QCOMPARE(spyCompleted1.count(), 1);
    QCOMPARE(spyCompletedKUrl1.count(), 1);
    QCOMPARE(spyCanceled1.count(), 0);
    QCOMPARE(spyCanceledKUrl1.count(), 0);
    QCOMPARE(spyClear1.count(), 1);
    QCOMPARE(spyClearKUrl1.count(), 0);
    QCOMPARE(m_items.count(), 3);

    QCOMPARE(spyStarted2.count(), 1);
    QCOMPARE(spyCompleted2.count(), 1);
    QCOMPARE(spyCompletedKUrl2.count(), 1);
    QCOMPARE(spyCanceled2.count(), 0);
    QCOMPARE(spyCanceledKUrl2.count(), 0);
    QCOMPARE(spyClear2.count(), 1);
    QCOMPARE(spyClearKUrl2.count(), 0);
    QCOMPARE(m_items2.count(), origItemCount);
    QVERIFY(m_dirLister.isFinished());

    disconnect(&m_dirLister, 0, this, 0);
    disconnect(&dirLister2, 0, this, 0);
}

void KDirListerTest::testConcurrentHoldingListing()
{
    // #167851.
    // A dirlister holding the items, and a second dirlister does
    // openUrl(reload) (which triggers updateDirectory())
    // and the first lister immediately does openUrl() (which emits cached items).

    testOpenUrl(); // ensure m_dirLister holds the items.
    const int origItemCount = m_items.count();
    QSignalSpy spyStarted(&m_dirLister, SIGNAL(started(KUrl)));
    QSignalSpy spyClear(&m_dirLister, SIGNAL(clear()));
    QSignalSpy spyClearKUrl(&m_dirLister, SIGNAL(clear(KUrl)));
    QSignalSpy spyCompleted(&m_dirLister, SIGNAL(completed()));
    QSignalSpy spyCompletedKUrl(&m_dirLister, SIGNAL(completed(KUrl)));
    QSignalSpy spyCanceled(&m_dirLister, SIGNAL(canceled()));
    QSignalSpy spyCanceledKUrl(&m_dirLister, SIGNAL(canceled(KUrl)));
    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));
    m_items.clear();
    m_items2.clear();
    const QString path = m_tempDir.name();
    KDirLister dirLister2;
    QSignalSpy spyStarted2(&dirLister2, SIGNAL(started(KUrl)));
    QSignalSpy spyClear2(&dirLister2, SIGNAL(clear()));
    QSignalSpy spyClearKUrl2(&dirLister2, SIGNAL(clear(KUrl)));
    QSignalSpy spyCompleted2(&dirLister2, SIGNAL(completed()));
    QSignalSpy spyCompletedKUrl2(&dirLister2, SIGNAL(completed(KUrl)));
    QSignalSpy spyCanceled2(&dirLister2, SIGNAL(canceled()));
    QSignalSpy spyCanceledKUrl2(&dirLister2, SIGNAL(canceled(KUrl)));
    connect(&dirLister2, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems2(KFileItemList)));

    dirLister2.openUrl(KUrl(path), KDirLister::Reload); // will start a list job
    QCOMPARE(spyStarted2.count(), 1);
    QCOMPARE(spyCompleted2.count(), 0);
    QCOMPARE(m_items.count(), 0);
    QCOMPARE(m_items2.count(), 0);

    qDebug("calling m_dirLister.openUrl");
    m_dirLister.openUrl(KUrl(path), KDirLister::NoFlags); // should emit cached items, and then "join" the running listjob
    QCOMPARE(spyStarted.count(), 1);
    QCOMPARE(spyCompleted.count(), 0);
    QCOMPARE(m_items.count(), 0);
    QCOMPARE(m_items2.count(), 0);

    qDebug("waiting for completed");
    connect(&dirLister2, SIGNAL(completed()), this, SLOT(exitLoop()));
    enterLoop();

    QCOMPARE(spyStarted2.count(), 1);
    QCOMPARE(spyCompleted2.count(), 1);
    QCOMPARE(spyCompletedKUrl2.count(), 1);
    QCOMPARE(spyCanceled2.count(), 0);
    QCOMPARE(spyCanceledKUrl2.count(), 0);
    QCOMPARE(spyClear2.count(), 1);
    QCOMPARE(spyClearKUrl2.count(), 0);
    QCOMPARE(m_items2.count(), origItemCount);

    if (spyCompleted.isEmpty()) {
        connect(&m_dirLister, SIGNAL(completed()), this, SLOT(exitLoop()));
        enterLoop();
    }

    QCOMPARE(spyStarted.count(), 1);
    QCOMPARE(spyCompleted.count(), 1);
    QCOMPARE(spyCompletedKUrl.count(), 1);
    QCOMPARE(spyCanceled.count(), 0);
    QCOMPARE(spyCanceledKUrl.count(), 0);
    QCOMPARE(spyClear.count(), 1);
    QCOMPARE(spyClearKUrl.count(), 0);
    QVERIFY(dirLister2.isFinished());
    disconnect(&dirLister2, 0, this, 0);
    QVERIFY(m_dirLister.isFinished());
    disconnect(&m_dirLister, 0, this, 0);
    QCOMPARE(m_items.count(), origItemCount);
}

void KDirListerTest::testOpenUrlTwice()
{
    // Calling openUrl(reload)+openUrl(normal) before listing even starts.
    const int origItemCount = m_items.count();
    m_items.clear();
    const QString path = m_tempDir.name();
    KDirLister secondDirLister;
    QSignalSpy spyStarted(&secondDirLister, SIGNAL(started(KUrl)));
    QSignalSpy spyClear(&secondDirLister, SIGNAL(clear()));
    QSignalSpy spyClearKUrl(&secondDirLister, SIGNAL(clear(KUrl)));
    QSignalSpy spyCompleted(&secondDirLister, SIGNAL(completed()));
    QSignalSpy spyCompletedKUrl(&secondDirLister, SIGNAL(completed(KUrl)));
    QSignalSpy spyCanceled(&secondDirLister, SIGNAL(canceled()));
    QSignalSpy spyCanceledKUrl(&secondDirLister, SIGNAL(canceled(KUrl)));
    connect(&secondDirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));

    secondDirLister.openUrl(KUrl(path), KDirLister::Reload); // will start
    QCOMPARE(spyStarted.count(), 1);
    QCOMPARE(spyCompleted.count(), 0);

    qDebug("calling openUrl again");
    secondDirLister.openUrl(KUrl(path), KDirLister::NoFlags); // will stop + start

    qDebug("waiting for completed");
    connect(&secondDirLister, SIGNAL(completed()), this, SLOT(exitLoop()));
    enterLoop();

    QCOMPARE(spyStarted.count(), 2);
    QCOMPARE(spyCompleted.count(), 1);
    QCOMPARE(spyCompletedKUrl.count(), 1);
    QCOMPARE(spyCanceled.count(), 0); // should not be emitted, see next test
    QCOMPARE(spyCanceledKUrl.count(), 0);
    QCOMPARE(spyClear.count(), 2);
    QCOMPARE(spyClearKUrl.count(), 0);
    QCOMPARE(m_items.count(), origItemCount);
    QVERIFY(secondDirLister.isFinished());
    disconnect(&secondDirLister, 0, this, 0);
}

void KDirListerTest::testOpenUrlTwiceWithKeep()
{
    // Calling openUrl(reload)+openUrl(keep) on a new dir,
    // before listing even starts (#177387)
    // Well, in 177387 the second openUrl call was made from within slotCanceled
    // called by the first openUrl
    // (slotLoadingFinished -> setCurrentItem -> expandToUrl -> listDir),
    // which messed things up in kdirlister (unexpected reentrancy).
    m_items.clear();
    const QString path = m_tempDir.name() + "/newsubdir";
    QDir().mkdir(path);
    KDirLister secondDirLister;
    QSignalSpy spyStarted(&secondDirLister, SIGNAL(started(KUrl)));
    QSignalSpy spyClear(&secondDirLister, SIGNAL(clear()));
    QSignalSpy spyClearKUrl(&secondDirLister, SIGNAL(clear(KUrl)));
    QSignalSpy spyCompleted(&secondDirLister, SIGNAL(completed()));
    QSignalSpy spyCompletedKUrl(&secondDirLister, SIGNAL(completed(KUrl)));
    QSignalSpy spyCanceled(&secondDirLister, SIGNAL(canceled()));
    QSignalSpy spyCanceledKUrl(&secondDirLister, SIGNAL(canceled(KUrl)));
    connect(&secondDirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));

    secondDirLister.openUrl(KUrl(path)); // will start a list job
    QCOMPARE(spyStarted.count(), 1);
    QCOMPARE(spyCompleted.count(), 0);

    qDebug("calling openUrl again");
    secondDirLister.openUrl(KUrl(path), KDirLister::Keep); // stops and restarts the job

    qDebug("waiting for completed");
    connect(&secondDirLister, SIGNAL(completed()), this, SLOT(exitLoop()));
    enterLoop();

    QCOMPARE(spyStarted.count(), 2);
    QCOMPARE(spyCompleted.count(), 1);
    QCOMPARE(spyCompletedKUrl.count(), 1);
    QCOMPARE(spyCanceled.count(), 0); // should not be emitted, it led to recursion
    QCOMPARE(spyCanceledKUrl.count(), 0);
    QCOMPARE(spyClear.count(), 1);
    QCOMPARE(spyClearKUrl.count(), 1);
    QCOMPARE(m_items.count(), 0);
    QVERIFY(secondDirLister.isFinished());
    disconnect(&secondDirLister, 0, this, 0);

    QDir().remove(path);
}

void KDirListerTest::testOpenAndStop()
{
    m_items.clear();
    const QString path = "/"; // better not use a directory that we already listed!
    QSignalSpy spyStarted(&m_dirLister, SIGNAL(started(KUrl)));
    QSignalSpy spyClear(&m_dirLister, SIGNAL(clear()));
    QSignalSpy spyClearKUrl(&m_dirLister, SIGNAL(clear(KUrl)));
    QSignalSpy spyCompleted(&m_dirLister, SIGNAL(completed()));
    QSignalSpy spyCompletedKUrl(&m_dirLister, SIGNAL(completed(KUrl)));
    QSignalSpy spyCanceled(&m_dirLister, SIGNAL(canceled()));
    QSignalSpy spyCanceledKUrl(&m_dirLister, SIGNAL(canceled(KUrl)));
    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));
    m_dirLister.openUrl(KUrl(path), KDirLister::NoFlags);
    kDebug() << "Calling stop!";
    m_dirLister.stop(); // we should also test stop(KUrl(path))...

    QCOMPARE(spyStarted.count(), 1); // The call to openUrl itself, emits started
    QCOMPARE(spyCompleted.count(), 0); // we had time to stop before the job even started
    QCOMPARE(spyCompletedKUrl.count(), 0);
    QCOMPARE(spyCanceled.count(), 1);
    QCOMPARE(spyCanceledKUrl.count(), 1);
    QCOMPARE(spyClear.count(), 1);
    QCOMPARE(spyClearKUrl.count(), 0);
    QCOMPARE(m_items.count(), 0); // we had time to stop before the job even started
    QVERIFY(m_dirLister.isFinished());
    disconnect(&m_dirLister, 0, this, 0);
}

void KDirListerTest::testRedirection()
{
    m_items.clear();
    const KUrl url("file://somemachine/");
    QSignalSpy spyStarted(&m_dirLister, SIGNAL(started(KUrl)));
    QSignalSpy spyClear(&m_dirLister, SIGNAL(clear()));
    QSignalSpy spyClearKUrl(&m_dirLister, SIGNAL(clear(KUrl)));
    QSignalSpy spyCompleted(&m_dirLister, SIGNAL(completed()));
    QSignalSpy spyCompletedKUrl(&m_dirLister, SIGNAL(completed(KUrl)));
    QSignalSpy spyCanceled(&m_dirLister, SIGNAL(canceled()));
    QSignalSpy spyCanceledKUrl(&m_dirLister, SIGNAL(canceled(KUrl)));
    QSignalSpy spyRedirection(&m_dirLister, SIGNAL(redirection(KUrl)));
    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));
    // The call to openUrl itself, emits started
    m_dirLister.openUrl(url, KDirLister::NoFlags);

    QCOMPARE(spyStarted.count(), 1);
    QCOMPARE(spyCompleted.count(), 0);
    QCOMPARE(spyCompletedKUrl.count(), 0);
    QCOMPARE(spyCanceled.count(), 0);
    QCOMPARE(spyCanceledKUrl.count(), 0);
    QCOMPARE(spyClear.count(), 1);
    QCOMPARE(spyClearKUrl.count(), 0);
    QCOMPARE(spyRedirection.count(), 0);
    QCOMPARE(m_items.count(), 0);
    QVERIFY(!m_dirLister.isFinished());

    // then wait for the redirection signal
    qDebug("waiting for redirection");
    connect(&m_dirLister, SIGNAL(redirection(KUrl, KUrl)), this, SLOT(exitLoop()));
    enterLoop();
    QCOMPARE(spyStarted.count(), 1);
    QCOMPARE(spyCompleted.count(), 0); // we stopped before the listing.
    QCOMPARE(spyCompletedKUrl.count(), 0);
    QCOMPARE(spyCanceled.count(), 0);
    QCOMPARE(spyCanceledKUrl.count(), 0);
    QCOMPARE(spyClear.count(), 2); // redirection cleared a second time (just in case...)
    QCOMPARE(spyClearKUrl.count(), 0);
    QCOMPARE(spyRedirection.count(), 1);
    QVERIFY(m_items.isEmpty());
    QVERIFY(!m_dirLister.isFinished());

    m_dirLister.stop(url);
    QVERIFY(!m_dirLister.isFinished());
    disconnect(&m_dirLister, 0, this, 0);

}

void KDirListerTest::enterLoop(int exitCount)
{
    //qDebug("enterLoop");
    m_exitCount = exitCount;
    m_eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
}

void KDirListerTest::exitLoop()
{
    //qDebug("exitLoop");
    --m_exitCount;
    if (m_exitCount <= 0) {
        m_eventLoop.quit();
    }
}

void KDirListerTest::slotNewItems(const KFileItemList& lst)
{
    m_items += lst;
}

void KDirListerTest::slotNewItems2(const KFileItemList& lst)
{
    m_items2 += lst;
}

void KDirListerTest::slotRefreshItems(const QList<QPair<KFileItem, KFileItem> > & lst)
{
    m_refreshedItems += lst;
    emit refreshItemsReceived();
}

void KDirListerTest::testDeleteCurrentDir()
{
    // ensure m_dirLister holds the items.
    m_dirLister.openUrl(KUrl(path()), KDirLister::NoFlags);
    connect(&m_dirLister, SIGNAL(completed()), this, SLOT(exitLoop()));
    enterLoop();
    disconnect(&m_dirLister, SIGNAL(completed()), this, SLOT(exitLoop()));

    QSignalSpy spyClear(&m_dirLister, SIGNAL(clear()));
    QSignalSpy spyClearKUrl(&m_dirLister, SIGNAL(clear(KUrl)));
    QSignalSpy spyItemsDeleted(&m_dirLister, SIGNAL(itemsDeleted(KFileItemList)));
    connect(&m_dirLister, SIGNAL(clear()), &m_eventLoop, SLOT(quit()));
    KIO::DeleteJob* job = KIO::del(path(), KIO::HideProgressInfo);
    bool ok = job->exec();
    QVERIFY(ok);
    enterLoop();
    QCOMPARE(spyClear.count(), 1);
    QCOMPARE(spyClearKUrl.count(), 0);
    QCOMPARE(spyItemsDeleted.count(), 1);
    QCOMPARE(spyItemsDeleted[0][0].value<KFileItemList>().count(), 1);
}

int KDirListerTest::fileCount() const
{
    return QDir(path()).entryList( QDir::AllEntries | QDir::NoDotAndDotDot).count();
}
