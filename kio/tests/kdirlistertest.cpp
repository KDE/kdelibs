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
#include <kio/copyjob.h>

#define WORKAROUND_BROKEN_INOTIFY 0

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
}

void KDirListerTest::cleanup()
{
    m_dirLister.clearSpies();
    disconnect(&m_dirLister, 0, this, 0);
}

void KDirListerTest::testOpenUrl()
{
    m_items.clear();
    const QString path = m_tempDir.name();
    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));
    // The call to openUrl itself, emits started
    m_dirLister.openUrl(KUrl(path), KDirLister::NoFlags);

    QCOMPARE(m_dirLister.spyStarted.count(), 1);
    QCOMPARE(m_dirLister.spyCompleted.count(), 0);
    QCOMPARE(m_dirLister.spyCompletedKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyCanceled.count(), 0);
    QCOMPARE(m_dirLister.spyCanceledKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyClear.count(), 1);
    QCOMPARE(m_dirLister.spyClearKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyRedirection.count(), 0);
    QCOMPARE(m_items.count(), 0);
    QVERIFY(!m_dirLister.isFinished());

    // then wait for completed
    qDebug("waiting for completed");
    connect(&m_dirLister, SIGNAL(completed()), this, SLOT(exitLoop()));
    enterLoop();
    QCOMPARE(m_dirLister.spyStarted.count(), 1);
    QCOMPARE(m_dirLister.spyCompleted.count(), 1);
    QCOMPARE(m_dirLister.spyCompletedKUrl.count(), 1);
    QCOMPARE(m_dirLister.spyCanceled.count(), 0);
    QCOMPARE(m_dirLister.spyCanceledKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyClear.count(), 1);
    QCOMPARE(m_dirLister.spyClearKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyRedirection.count(), 0);
    QCOMPARE(m_items.count(), fileCount());
    QVERIFY(m_dirLister.isFinished());
    disconnect(&m_dirLister, 0, this, 0);

    const QString fileName = "toplevelfile_3";
    const KUrl itemUrl = path + fileName;
    KFileItem byName = m_dirLister.findByName(fileName);
    QVERIFY(!byName.isNull());
    QCOMPARE(byName.url().url(), itemUrl.url());
    QCOMPARE(byName.entry().stringValue(KIO::UDSEntry::UDS_NAME), fileName);
    KFileItem byUrl = m_dirLister.findByUrl(itemUrl);
    QVERIFY(!byUrl.isNull());
    QCOMPARE(byUrl.url().url(), itemUrl.url());
    QCOMPARE(byUrl.entry().stringValue(KIO::UDSEntry::UDS_NAME), fileName);
    KFileItem itemForUrl = KDirLister::cachedItemForUrl(itemUrl);
    QVERIFY(!itemForUrl.isNull());
    QCOMPARE(itemForUrl.url().url(), itemUrl.url());
    QCOMPARE(itemForUrl.entry().stringValue(KIO::UDSEntry::UDS_NAME), fileName);

    KFileItem rootByUrl = m_dirLister.findByUrl(path);
    QVERIFY(!rootByUrl.isNull());
    QCOMPARE(rootByUrl.url().path() + '/', path);

    m_dirLister.clearSpies(); // for the tests that call testOpenUrl for setup
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
        MyDirLister secondDirLister;
        connect(&secondDirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));
        secondDirLister.openUrl(KUrl(path), KDirLister::NoFlags);
        QCOMPARE(secondDirLister.spyStarted.count(), 1);
        QCOMPARE(secondDirLister.spyCompleted.count(), 0);
        QCOMPARE(secondDirLister.spyCompletedKUrl.count(), 0);
        QCOMPARE(secondDirLister.spyCanceled.count(), 0);
        QCOMPARE(secondDirLister.spyCanceledKUrl.count(), 0);
        QCOMPARE(secondDirLister.spyClear.count(), 1);
        QCOMPARE(secondDirLister.spyClearKUrl.count(), 0);
        QCOMPARE(m_items.count(), 0);
        QVERIFY(!secondDirLister.isFinished());

        // then wait for completed
        qDebug("waiting for completed");
        connect(&secondDirLister, SIGNAL(completed()), this, SLOT(exitLoop()));
        enterLoop();
        QCOMPARE(secondDirLister.spyStarted.count(), 1);
        QCOMPARE(secondDirLister.spyCompleted.count(), 1);
        QCOMPARE(secondDirLister.spyCompletedKUrl.count(), 1);
        QCOMPARE(secondDirLister.spyCanceled.count(), 0);
        QCOMPARE(secondDirLister.spyCanceledKUrl.count(), 0);
        QCOMPARE(secondDirLister.spyClear.count(), 1);
        QCOMPARE(secondDirLister.spyClearKUrl.count(), 0);
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
    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));

    QTest::qWait(1000); // We need a 1s timestamp difference on the dir, otherwise FAM won't notice anything.

    kDebug() << "Creating new file";
    const QString fileName = "toplevelfile_new";
    createSimpleFile(path + fileName);

    int numTries = 0;
    // Give time for KDirWatch to notify us
    while (m_items.count() == 4) {
        QVERIFY(++numTries < 10);
        QTest::qWait(200);
    }
    //kDebug() << "numTries=" << numTries;
    QCOMPARE(m_items.count(), 5);

    QCOMPARE(m_dirLister.spyStarted.count(), 1); // Updates call started
    QCOMPARE(m_dirLister.spyCompleted.count(), 1); // and completed
    QCOMPARE(m_dirLister.spyCompletedKUrl.count(), 1);
    QCOMPARE(m_dirLister.spyCanceled.count(), 0);
    QCOMPARE(m_dirLister.spyCanceledKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyClear.count(), 0);
    QCOMPARE(m_dirLister.spyClearKUrl.count(), 0);

    const KUrl itemUrl = path + fileName;
    KFileItem itemForUrl = KDirLister::cachedItemForUrl(itemUrl);
    QVERIFY(!itemForUrl.isNull());
    QCOMPARE(itemForUrl.url().url(), itemUrl.url());
    QCOMPARE(itemForUrl.entry().stringValue(KIO::UDSEntry::UDS_NAME), fileName);
}

void KDirListerTest::testNewItemByCopy()
{
    // This test creates a file using KIO::copyAs, like knewmenu.cpp does.
    // Useful for testing #192185, i.e. whether we catch the kdirwatch event and avoid
    // a KFileItem::refresh().
    const int origItemCount = m_items.count();
    const QString path = m_tempDir.name();
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

    QCOMPARE(m_dirLister.spyStarted.count(), 1); // Updates call started
    QCOMPARE(m_dirLister.spyCompleted.count(), 1); // and completed
    QCOMPARE(m_dirLister.spyCompletedKUrl.count(), 1);
    QCOMPARE(m_dirLister.spyCanceled.count(), 0);
    QCOMPARE(m_dirLister.spyCanceledKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyClear.count(), 0);
    QCOMPARE(m_dirLister.spyClearKUrl.count(), 0);

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
    MyDirLister dirLister2;
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
    createSimpleFile(path + fileName);

#if WORKAROUND_BROKEN_INOTIFY
    org::kde::KDirNotify::emitFilesAdded(path);
#endif
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
        createSimpleFile(path + fileName2);

        int numTries = 0;
        // Give time for KDirWatch to notify us
        while (m_items2.count() == origItemCount + 1) {
            QVERIFY(++numTries < 10);
            QTest::qWait(200);
        }
        QCOMPARE(m_items2.count(), origItemCount+2);
        QCOMPARE(m_items.count(), origItemCount+2);
    }
    QCOMPARE(fileCount(), m_items.count());

    // Test file deletion
    {
        qDebug() << "Deleting" << (path+fileName);
        QTest::qWait(1000); // for timestamp difference
        QFile::remove(path + fileName);
        while (dirLister2.spyDeleteItem.count() == 0) {
            QVERIFY(++numTries < 10);
            QTest::qWait(200);
        }
        QCOMPARE(dirLister2.spyDeleteItem.count(), 1);
        const KFileItem item = dirLister2.spyDeleteItem[0][0].value<KFileItem>();
        QCOMPARE(item.url().path(), symPath + '/' + fileName);
    }

    // TODO: test file update.
    disconnect(&m_dirLister, 0, this, 0);
}

// This test assumes testOpenUrl was run before. So m_dirLister is holding the items already.
void KDirListerTest::testRefreshItems()
{
    m_refreshedItems.clear();

    const QString path = m_tempDir.name();
    const QString fileName = path+"toplevelfile_1";
    KFileItem cachedItem = m_dirLister.findByUrl(KUrl(fileName));
    QVERIFY(!cachedItem.isNull());
    QCOMPARE(cachedItem.mimetype(), QString("application/octet-stream"));

    connect(&m_dirLister, SIGNAL(refreshItems(const QList<QPair<KFileItem, KFileItem> > &)),
            this, SLOT(slotRefreshItems(const QList<QPair<KFileItem, KFileItem> > &)));

    QFile file(fileName);
    QVERIFY(file.open(QIODevice::Append));
    file.write(QByteArray("<html>"));
    file.close();
    QCOMPARE(QFileInfo(fileName).size(), 11LL /*Hello world*/ + 6 /*<html>*/);

    waitForRefreshedItems();

    QCOMPARE(m_dirLister.spyStarted.count(), 0); // fast path: no directory listing needed
    QCOMPARE(m_dirLister.spyCompleted.count(), 0);
    QCOMPARE(m_dirLister.spyCompletedKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyCanceled.count(), 0);
    QCOMPARE(m_dirLister.spyCanceledKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyClear.count(), 0);
    QCOMPARE(m_dirLister.spyClearKUrl.count(), 0);
    QCOMPARE(m_refreshedItems.count(), 1);
    QPair<KFileItem, KFileItem> entry = m_refreshedItems.first();
    QCOMPARE(entry.first.url().path(), fileName);
    QCOMPARE(entry.first.size(), KIO::filesize_t(11));
    QCOMPARE(entry.first.mimetype(), QString("application/octet-stream"));
    QCOMPARE(entry.second.url().path(), fileName);
    QCOMPARE(entry.second.size(), KIO::filesize_t(11 /*Hello world*/ + 6 /*<html>*/));
    QCOMPARE(entry.second.mimetype(), QString("text/html"));

    // Let's see what KDirLister has in cache now
    cachedItem = m_dirLister.findByUrl(KUrl(fileName));
    QCOMPARE(cachedItem.size(), KIO::filesize_t(11 /*Hello world*/ + 6 /*<html>*/));
    m_refreshedItems.clear();
}

// Refresh the root item, plus a hidden file, e.g. changing its icon. #190535
void KDirListerTest::testRefreshRootItem()
{
    // This test assumes testOpenUrl was run before. So m_dirLister is holding the items already.
    m_refreshedItems.clear();
    m_refreshedItems2.clear();

    // The item will be the root item of dirLister2, but also a child item
    // of m_dirLister.
    // In #190535 it would show "." instead of the subdir name, after a refresh...
    const QString path = m_tempDir.name() + "subdir";
    MyDirLister dirLister2;
    fillDirLister2(dirLister2, path);

    connect(&m_dirLister, SIGNAL(refreshItems(const QList<QPair<KFileItem, KFileItem> > &)),
            this, SLOT(slotRefreshItems(const QList<QPair<KFileItem, KFileItem> > &)));

    org::kde::KDirNotify::emitFilesChanged(QStringList() << KUrl(path).url());
    waitForRefreshedItems();

    QCOMPARE(m_dirLister.spyStarted.count(), 0);
    QCOMPARE(m_dirLister.spyCompleted.count(), 0);
    QCOMPARE(m_dirLister.spyCompletedKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyCanceled.count(), 0);
    QCOMPARE(m_dirLister.spyCanceledKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyClear.count(), 0);
    QCOMPARE(m_dirLister.spyClearKUrl.count(), 0);
    QCOMPARE(m_refreshedItems.count(), 1);
    QPair<KFileItem, KFileItem> entry = m_refreshedItems.first();
    QCOMPARE(entry.first.url().path(), path);
    QCOMPARE(entry.first.name(), QString("subdir"));
    QCOMPARE(entry.second.url().path(), path);
    QCOMPARE(entry.second.name(), QString("subdir"));

    QCOMPARE(m_refreshedItems2.count(), 1);
    entry = m_refreshedItems2.first();
    QCOMPARE(entry.first.url().path(), path);
    QCOMPARE(entry.second.url().path(), path);
    // item name() doesn't matter here, it's the root item.

    m_refreshedItems.clear();
    m_refreshedItems2.clear();

    const QString directoryFile = path + "/.directory";
    createSimpleFile(directoryFile);

    org::kde::KDirNotify::emitFilesAdded(KUrl(path).url());
    QTest::qWait(200);
    org::kde::KDirNotify::emitFilesChanged(QStringList() << KUrl(directoryFile).url());
    QCOMPARE(m_refreshedItems.count(), 0);

    org::kde::KDirNotify::emitFilesChanged(QStringList() << KUrl(path).url());
    waitForRefreshedItems();
    QCOMPARE(m_refreshedItems.count(), 1);
    entry = m_refreshedItems.first();
    QCOMPARE(entry.first.url().path(), path);
    QCOMPARE(entry.second.url().path(), path);

    m_refreshedItems.clear();
    m_refreshedItems2.clear();

    // Note: this test leaves the .directory file as a side effect.
    // Hidden though, shouldn't matter.
}

void KDirListerTest::testDeleteItem()
{
    testOpenUrl(); // ensure m_items is uptodate

    const int origItemCount = m_items.count();
    QCOMPARE(fileCount(), origItemCount);
    const QString path = m_tempDir.name();
    connect(&m_dirLister, SIGNAL(deleteItem(const KFileItem&)), this, SLOT(exitLoop()));

    //kDebug() << "Removing " << path+"toplevelfile_1";
    QFile::remove(path+"toplevelfile_1");
    // the remove() doesn't always trigger kdirwatch in stat mode, if this all happens in the same second
    KDirWatch::self()->setDirty(path);
    if (m_dirLister.spyDeleteItem.count() == 0) {
        qDebug("waiting for deleteItem");
        enterLoop();
    }

    QCOMPARE(m_dirLister.spyDeleteItem.count(), 1);
    QCOMPARE(m_dirLister.spyItemsDeleted.count(), 1);

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
    connect(&m_dirLister, SIGNAL(refreshItems(const QList<QPair<KFileItem, KFileItem> > &)),
            this, SLOT(slotRefreshItems(const QList<QPair<KFileItem, KFileItem> > &)));
    const QString path = dirPath+"toplevelfile_2";
    const QString newPath = dirPath+"toplevelfile_2.renamed.html";

    KIO::SimpleJob* job = KIO::rename(path, newPath, KIO::HideProgressInfo);
    bool ok = job->exec();
    QVERIFY(ok);

    if (m_refreshedItems.isEmpty()) {
        waitForRefreshedItems(); // refreshItems could come from KDirWatch or KDirNotify.
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
#if WORKAROUND_BROKEN_INOTIFY
    org::kde::KDirNotify::emitFilesAdded(dirPath);
#endif
    KFileItem existingItem;
    while (existingItem.isNull()) {
        QTest::qWait(100);
        existingItem = m_dirLister.findByUrl(KUrl(path));
    };
    QCOMPARE(existingItem.url().path(), path);

    m_refreshedItems.clear();
    connect(&m_dirLister, SIGNAL(refreshItems(const QList<QPair<KFileItem, KFileItem> > &)),
            this, SLOT(slotRefreshItems(const QList<QPair<KFileItem, KFileItem> > &)));
    const QString newPath = dirPath+"toplevelfile_2.renamed.html";

    KIO::SimpleJob* job = KIO::rename(newPath, path, KIO::Overwrite | KIO::HideProgressInfo);
    bool ok = job->exec();
    QVERIFY(ok);

    if (m_refreshedItems.isEmpty()) {
        waitForRefreshedItems(); // refreshItems could come from KDirWatch or KDirNotify.
    }

    // Check that itemsDeleted was emitted -- preferrably BEFORE refreshItems,
    // but we can't easily check that with QSignalSpy...
    QCOMPARE(m_dirLister.spyItemsDeleted.count(), 1);

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

    MyDirLister dirLister2;

    const QString path = m_tempDir.name();

    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));
    connect(&dirLister2, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems2(KFileItemList)));

    // Before dirLister2 has time to emit the items, let's make m_dirLister move to another dir.
    // This reproduces the use case "clicking on a folder in dolphin iconview, and dirlister2
    // is the one used by the "folder panel". m_dirLister is going to list the subdir,
    // while dirLister2 wants to list the folder that m_dirLister has just left.
    dirLister2.stop(); // like dolphin does, noop.
    dirLister2.openUrl(KUrl(path), KDirLister::NoFlags);
    m_dirLister.openUrl(KUrl(path+"subdir"), KDirLister::NoFlags);

    QCOMPARE(m_dirLister.spyStarted.count(), 1);
    QCOMPARE(m_dirLister.spyCompleted.count(), 0);
    QCOMPARE(m_dirLister.spyCompletedKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyCanceled.count(), 0);
    QCOMPARE(m_dirLister.spyCanceledKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyClear.count(), 1);
    QCOMPARE(m_dirLister.spyClearKUrl.count(), 0);
    QCOMPARE(m_items.count(), 0);

    QCOMPARE(dirLister2.spyStarted.count(), 1);
    QCOMPARE(dirLister2.spyCompleted.count(), 0);
    QCOMPARE(dirLister2.spyCompletedKUrl.count(), 0);
    QCOMPARE(dirLister2.spyCanceled.count(), 0);
    QCOMPARE(dirLister2.spyCanceledKUrl.count(), 0);
    QCOMPARE(dirLister2.spyClear.count(), 1);
    QCOMPARE(dirLister2.spyClearKUrl.count(), 0);
    QCOMPARE(m_items2.count(), 0);
    QVERIFY(!m_dirLister.isFinished());
    QVERIFY(!dirLister2.isFinished());

    // then wait for completed
    qDebug("waiting for completed");
    connect(&m_dirLister, SIGNAL(completed()), this, SLOT(exitLoop()));
    connect(&dirLister2, SIGNAL(completed()), this, SLOT(exitLoop()));
    enterLoop(2);

    //QCOMPARE(m_dirLister.spyStarted.count(), 1); // 2 when subdir is already in cache.
    QCOMPARE(m_dirLister.spyCompleted.count(), 1);
    QCOMPARE(m_dirLister.spyCompletedKUrl.count(), 1);
    QCOMPARE(m_dirLister.spyCanceled.count(), 0);
    QCOMPARE(m_dirLister.spyCanceledKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyClear.count(), 1);
    QCOMPARE(m_dirLister.spyClearKUrl.count(), 0);
    QCOMPARE(m_items.count(), 3);

    QCOMPARE(dirLister2.spyStarted.count(), 1);
    QCOMPARE(dirLister2.spyCompleted.count(), 1);
    QCOMPARE(dirLister2.spyCompletedKUrl.count(), 1);
    QCOMPARE(dirLister2.spyCanceled.count(), 0);
    QCOMPARE(dirLister2.spyCanceledKUrl.count(), 0);
    QCOMPARE(dirLister2.spyClear.count(), 1);
    QCOMPARE(dirLister2.spyClearKUrl.count(), 0);
    QCOMPARE(m_items2.count(), origItemCount);
    //QVERIFY(m_dirLister.isFinished()); // false when an update is running because subdir is already in cache

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
    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));
    m_items.clear();
    m_items2.clear();
    const QString path = m_tempDir.name();
    MyDirLister dirLister2;
    connect(&dirLister2, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems2(KFileItemList)));

    dirLister2.openUrl(KUrl(path), KDirLister::Reload); // will start a list job
    QCOMPARE(dirLister2.spyStarted.count(), 1);
    QCOMPARE(dirLister2.spyCompleted.count(), 0);
    QCOMPARE(m_items.count(), 0);
    QCOMPARE(m_items2.count(), 0);

    qDebug("calling m_dirLister.openUrl");
    m_dirLister.openUrl(KUrl(path), KDirLister::NoFlags); // should emit cached items, and then "join" the running listjob
    QCOMPARE(m_dirLister.spyStarted.count(), 1);
    QCOMPARE(m_dirLister.spyCompleted.count(), 0);
    QCOMPARE(m_items.count(), 0);
    QCOMPARE(m_items2.count(), 0);

    qDebug("waiting for completed");
    connect(&dirLister2, SIGNAL(completed()), this, SLOT(exitLoop()));
    enterLoop();

    QCOMPARE(dirLister2.spyStarted.count(), 1);
    QCOMPARE(dirLister2.spyCompleted.count(), 1);
    QCOMPARE(dirLister2.spyCompletedKUrl.count(), 1);
    QCOMPARE(dirLister2.spyCanceled.count(), 0);
    QCOMPARE(dirLister2.spyCanceledKUrl.count(), 0);
    QCOMPARE(dirLister2.spyClear.count(), 1);
    QCOMPARE(dirLister2.spyClearKUrl.count(), 0);
    QCOMPARE(m_items2.count(), origItemCount);

    if (m_dirLister.spyCompleted.isEmpty()) {
        connect(&m_dirLister, SIGNAL(completed()), this, SLOT(exitLoop()));
        enterLoop();
    }

    QCOMPARE(m_dirLister.spyStarted.count(), 1);
    QCOMPARE(m_dirLister.spyCompleted.count(), 1);
    QCOMPARE(m_dirLister.spyCompletedKUrl.count(), 1);
    QCOMPARE(m_dirLister.spyCanceled.count(), 0);
    QCOMPARE(m_dirLister.spyCanceledKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyClear.count(), 1);
    QCOMPARE(m_dirLister.spyClearKUrl.count(), 0);
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
    MyDirLister secondDirLister;
    connect(&secondDirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));

    secondDirLister.openUrl(KUrl(path), KDirLister::Reload); // will start
    QCOMPARE(secondDirLister.spyStarted.count(), 1);
    QCOMPARE(secondDirLister.spyCompleted.count(), 0);

    qDebug("calling openUrl again");
    secondDirLister.openUrl(KUrl(path), KDirLister::NoFlags); // will stop + start

    qDebug("waiting for completed");
    connect(&secondDirLister, SIGNAL(completed()), this, SLOT(exitLoop()));
    enterLoop();

    QCOMPARE(secondDirLister.spyStarted.count(), 2);
    QCOMPARE(secondDirLister.spyCompleted.count(), 1);
    QCOMPARE(secondDirLister.spyCompletedKUrl.count(), 1);
    QCOMPARE(secondDirLister.spyCanceled.count(), 0); // should not be emitted, see next test
    QCOMPARE(secondDirLister.spyCanceledKUrl.count(), 0);
    QCOMPARE(secondDirLister.spyClear.count(), 2);
    QCOMPARE(secondDirLister.spyClearKUrl.count(), 0);
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
    MyDirLister secondDirLister;
    connect(&secondDirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));

    secondDirLister.openUrl(KUrl(path)); // will start a list job
    QCOMPARE(secondDirLister.spyStarted.count(), 1);
    QCOMPARE(secondDirLister.spyCompleted.count(), 0);

    qDebug("calling openUrl again");
    secondDirLister.openUrl(KUrl(path), KDirLister::Keep); // stops and restarts the job

    qDebug("waiting for completed");
    connect(&secondDirLister, SIGNAL(completed()), this, SLOT(exitLoop()));
    enterLoop();

    QCOMPARE(secondDirLister.spyStarted.count(), 2);
    QCOMPARE(secondDirLister.spyCompleted.count(), 1);
    QCOMPARE(secondDirLister.spyCompletedKUrl.count(), 1);
    QCOMPARE(secondDirLister.spyCanceled.count(), 0); // should not be emitted, it led to recursion
    QCOMPARE(secondDirLister.spyCanceledKUrl.count(), 0);
    QCOMPARE(secondDirLister.spyClear.count(), 1);
    QCOMPARE(secondDirLister.spyClearKUrl.count(), 1);
    QCOMPARE(m_items.count(), 0);
    QVERIFY(secondDirLister.isFinished());
    disconnect(&secondDirLister, 0, this, 0);

    QDir().remove(path);
}

void KDirListerTest::testOpenAndStop()
{
    m_items.clear();
    const QString path = "/"; // better not use a directory that we already listed!
    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));
    m_dirLister.openUrl(KUrl(path), KDirLister::NoFlags);
    kDebug() << "Calling stop!";
    m_dirLister.stop(); // we should also test stop(KUrl(path))...

    QCOMPARE(m_dirLister.spyStarted.count(), 1); // The call to openUrl itself, emits started
    QCOMPARE(m_dirLister.spyCompleted.count(), 0); // we had time to stop before the job even started
    QCOMPARE(m_dirLister.spyCompletedKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyCanceled.count(), 1);
    QCOMPARE(m_dirLister.spyCanceledKUrl.count(), 1);
    QCOMPARE(m_dirLister.spyClear.count(), 1);
    QCOMPARE(m_dirLister.spyClearKUrl.count(), 0);
    QCOMPARE(m_items.count(), 0); // we had time to stop before the job even started
    QVERIFY(m_dirLister.isFinished());
    disconnect(&m_dirLister, 0, this, 0);
}

void KDirListerTest::testRedirection()
{
    m_items.clear();
    const KUrl url("file://somemachine/");
    connect(&m_dirLister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems(KFileItemList)));
    // The call to openUrl itself, emits started
    m_dirLister.openUrl(url, KDirLister::NoFlags);

    QCOMPARE(m_dirLister.spyStarted.count(), 1);
    QCOMPARE(m_dirLister.spyCompleted.count(), 0);
    QCOMPARE(m_dirLister.spyCompletedKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyCanceled.count(), 0);
    QCOMPARE(m_dirLister.spyCanceledKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyClear.count(), 1);
    QCOMPARE(m_dirLister.spyClearKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyRedirection.count(), 0);
    QCOMPARE(m_items.count(), 0);
    QVERIFY(!m_dirLister.isFinished());

    // then wait for the redirection signal
    qDebug("waiting for redirection");
    connect(&m_dirLister, SIGNAL(redirection(KUrl, KUrl)), this, SLOT(exitLoop()));
    enterLoop();
    QCOMPARE(m_dirLister.spyStarted.count(), 1);
    QCOMPARE(m_dirLister.spyCompleted.count(), 0); // we stopped before the listing.
    QCOMPARE(m_dirLister.spyCompletedKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyCanceled.count(), 0);
    QCOMPARE(m_dirLister.spyCanceledKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyClear.count(), 2); // redirection cleared a second time (just in case...)
    QCOMPARE(m_dirLister.spyClearKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyRedirection.count(), 1);
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

void KDirListerTest::slotRefreshItems2(const QList<QPair<KFileItem, KFileItem> > & lst)
{
    m_refreshedItems2 += lst;
}

void KDirListerTest::testDeleteCurrentDir()
{
    // ensure m_dirLister holds the items.
    m_dirLister.openUrl(KUrl(path()), KDirLister::NoFlags);
    connect(&m_dirLister, SIGNAL(completed()), this, SLOT(exitLoop()));
    enterLoop();
    disconnect(&m_dirLister, SIGNAL(completed()), this, SLOT(exitLoop()));

    m_dirLister.clearSpies();
    connect(&m_dirLister, SIGNAL(clear()), &m_eventLoop, SLOT(quit()));
    KIO::DeleteJob* job = KIO::del(path(), KIO::HideProgressInfo);
    bool ok = job->exec();
    QVERIFY(ok);
    enterLoop();
    QCOMPARE(m_dirLister.spyClear.count(), 1);
    QCOMPARE(m_dirLister.spyClearKUrl.count(), 0);
    QCOMPARE(m_dirLister.spyItemsDeleted.count(), 1);
    QCOMPARE(m_dirLister.spyItemsDeleted[0][0].value<KFileItemList>().count(), 1);
}

int KDirListerTest::fileCount() const
{
    return QDir(path()).entryList( QDir::AllEntries | QDir::NoDotAndDotDot).count();
}

void KDirListerTest::waitForRefreshedItems()
{
    int numTries = 0;
    // Give time for KDirWatch to notify us
    while (m_refreshedItems.isEmpty()) {
        QVERIFY(++numTries < 10);
        QTest::qWait(200);
    }
}

void KDirListerTest::createSimpleFile(const QString& fileName)
{
    QFile file(fileName);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QByteArray("foo"));
    file.close();
}

void KDirListerTest::fillDirLister2(MyDirLister& lister, const QString& path)
{
    m_items2.clear();
    connect(&lister, SIGNAL(newItems(KFileItemList)), this, SLOT(slotNewItems2(KFileItemList)));
    connect(&lister, SIGNAL(refreshItems(QList<QPair<KFileItem, KFileItem> >)),
            this, SLOT(slotRefreshItems2(QList<QPair<KFileItem, KFileItem> >)));
    lister.openUrl(KUrl(path), KDirLister::NoFlags);
    connect(&lister, SIGNAL(completed()), this, SLOT(exitLoop()));
    enterLoop();
    QVERIFY(lister.isFinished());
}
