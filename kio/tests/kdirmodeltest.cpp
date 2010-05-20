/* This file is part of the KDE project
   Copyright 2006 - 2007 David Faure <faure@kde.org>

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

#include "kdirmodeltest.h"
#include <kdirnotify.h>
#include <kio/copyjob.h>
#include <kio/chmodjob.h>
#include <kprotocolinfo.h>
#include "kdirmodeltest.moc"
#include <kdirmodel.h>
#include <kdirlister.h>

#include <qtest_kde.h>

#ifdef Q_OS_UNIX
#include <utime.h>
#endif
#include <kdebug.h>
#include <kio/deletejob.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kdirwatch.h>
#include "kiotesthelper.h"

QTEST_KDEMAIN( KDirModelTest, NoGUI )

#ifndef USE_QTESTEVENTLOOP
#define exitLoop quit
#endif

#define connect(a,b,c,d) QVERIFY(QObject::connect(a,b,c,d))

#ifndef Q_WS_WIN
#define SPECIALCHARS "specialchars%:.pdf"
#else
#define SPECIALCHARS "specialchars%.pdf"
#endif

Q_DECLARE_METATYPE(KFileItemList)

void KDirModelTest::initTestCase()
{
    qRegisterMetaType<QModelIndex>("QModelIndex"); // beats me why Qt doesn't do that

    qRegisterMetaType<KFileItemList>("KFileItemList");

    m_dirModelForExpand = 0;
    m_dirModel = 0;
    s_referenceTimeStamp = QDateTime::currentDateTime().addSecs( -30 ); // 30 seconds ago
    m_tempDir = 0;
    m_topLevelFileNames << "toplevelfile_1"
                        << "toplevelfile_2"
                        << "toplevelfile_3"
                        << SPECIALCHARS
                        ;
    recreateTestData();

    fillModel( false );
}

void KDirModelTest::recreateTestData()
{
    if (m_tempDir)
        kDebug() << "Deleting old tempdir" << m_tempDir->name();
    delete m_tempDir;
    m_tempDir = new KTempDir;
    kDebug() << "new tmp dir:" << m_tempDir->name();
    // Create test data:
    /*
     * PATH/toplevelfile_1
     * PATH/toplevelfile_2
     * PATH/toplevelfile_3
     * PATH/specialchars%:.pdf
     * PATH/.hidden
     * PATH/.hidden2
     * PATH/subdir
     * PATH/subdir/testfile
     * PATH/subdir/subsubdir
     * PATH/subdir/subsubdir/testfile
     */
    const QString path = m_tempDir->name();
    foreach(const QString &f, m_topLevelFileNames) {
        createTestFile(path+f);
    }
    createTestFile(path+".hidden");
    createTestFile(path+".hidden2");
    createTestDirectory(path+"subdir");
    createTestDirectory(path+"subdir/subsubdir", NoSymlink);

    m_dirIndex = QModelIndex();
    m_fileIndex = QModelIndex();
    m_secondFileIndex = QModelIndex();
}

void KDirModelTest::cleanupTestCase()
{
    delete m_tempDir;
    m_tempDir = 0;
    delete m_dirModel;
    m_dirModel = 0;
}

void KDirModelTest::fillModel(bool reload, bool expectAllIndexes)
{
    if (!m_dirModel)
        m_dirModel = new KDirModel;
    m_dirModel->dirLister()->setAutoErrorHandlingEnabled(false, 0);
    const QString path = m_tempDir->name();
    KDirLister* dirLister = m_dirModel->dirLister();
    kDebug() << "Calling openUrl";
    dirLister->openUrl(KUrl(path), reload ? KDirLister::Reload : KDirLister::NoFlags);
    connect(dirLister, SIGNAL(completed()), this, SLOT(slotListingCompleted()));
    kDebug() << "enterLoop, waiting for completed()";
    enterLoop();

    if (expectAllIndexes)
        collectKnownIndexes();
    disconnect(dirLister, SIGNAL(completed()), this, SLOT(slotListingCompleted()));
}

// Called after test function
void KDirModelTest::cleanup()
{
    if (m_dirModel) {
        disconnect(m_dirModel, 0, &m_eventLoop, 0);
        disconnect(m_dirModel->dirLister(), 0, this, 0);
        m_dirModel->dirLister()->setNameFilter(QString());
        m_dirModel->dirLister()->setMimeFilter(QStringList());
        m_dirModel->dirLister()->emitChanges();
    }
}

void KDirModelTest::collectKnownIndexes()
{
    m_dirIndex = QModelIndex();
    m_fileIndex = QModelIndex();
    m_secondFileIndex = QModelIndex();
    // Create the indexes once and for all
    // The trouble is that the order of listing is undefined, one can get 1/2/3/subdir or subdir/3/2/1 for instance.
    for (int row = 0; row < m_topLevelFileNames.count() + 1 /*subdir*/; ++row) {
        QModelIndex idx = m_dirModel->index(row, 0, QModelIndex());
        QVERIFY(idx.isValid());
        KFileItem item = m_dirModel->itemForIndex(idx);
        kDebug() << item.url() << "isDir=" << item.isDir();
        if (item.isDir())
            m_dirIndex = idx;
        else if (item.url().fileName() == "toplevelfile_1")
            m_fileIndex = idx;
        else if (item.url().fileName() == "toplevelfile_2")
            m_secondFileIndex = idx;
        else if (item.url().fileName().startsWith("special"))
            m_specialFileIndex = idx;
    }
    QVERIFY(m_dirIndex.isValid());
    QVERIFY(m_fileIndex.isValid());
    QVERIFY(m_secondFileIndex.isValid());
    QVERIFY(m_specialFileIndex.isValid());

    // Now list subdir/
    QVERIFY(m_dirModel->canFetchMore(m_dirIndex));
    m_dirModel->fetchMore(m_dirIndex);
    kDebug() << "Listing subdir/";
    enterLoop();

    // Index of a file inside a directory (subdir/testfile)
    QModelIndex subdirIndex;
    m_fileInDirIndex = QModelIndex();
    for (int row = 0; row < 3; ++row) {
        QModelIndex idx = m_dirModel->index(row, 0, m_dirIndex);
        if (m_dirModel->itemForIndex(idx).isDir())
            subdirIndex = idx;
        else if (m_dirModel->itemForIndex(idx).name() == "testfile")
            m_fileInDirIndex = idx;
    }

    // List subdir/subsubdir
    QVERIFY(m_dirModel->canFetchMore(subdirIndex));
    kDebug() << "Listing subdir/subsubdir";
    m_dirModel->fetchMore(subdirIndex);
    enterLoop();

    // Index of ... well, subdir/subsubdir/testfile
    m_fileInSubdirIndex = m_dirModel->index(0, 0, subdirIndex);
}

void KDirModelTest::enterLoop()
{
#ifdef USE_QTESTEVENTLOOP
    m_eventLoop.enterLoop(10 /*seconds max*/);
    QVERIFY(!m_eventLoop.timeout());
#else
    m_eventLoop.exec();
#endif
}

void KDirModelTest::slotListingCompleted()
{
    kDebug();
#ifdef USE_QTESTEVENTLOOP
    m_eventLoop.exitLoop();
#else
    m_eventLoop.quit();
#endif
}

void KDirModelTest::testRowCount()
{
    const int topLevelRowCount = m_dirModel->rowCount();
    QCOMPARE(topLevelRowCount, m_topLevelFileNames.count() + 1 /*subdir*/);
    const int subdirRowCount = m_dirModel->rowCount(m_dirIndex);
    QCOMPARE(subdirRowCount, 3);

    QVERIFY(m_fileIndex.isValid());
    const int fileRowCount = m_dirModel->rowCount(m_fileIndex); // #176555
    QCOMPARE(fileRowCount, 0);
}

void KDirModelTest::testIndex()
{
    QVERIFY(m_dirModel->hasChildren());

    // Index of the first file
    QVERIFY(m_fileIndex.isValid());
    QCOMPARE(m_fileIndex.model(), static_cast<const QAbstractItemModel*>(m_dirModel));
    //QCOMPARE(m_fileIndex.row(), 0);
    QCOMPARE(m_fileIndex.column(), 0);
    QVERIFY(!m_fileIndex.parent().isValid());
    QVERIFY(!m_dirModel->hasChildren(m_fileIndex));

    // Index of a directory
    QVERIFY(m_dirIndex.isValid());
    QCOMPARE(m_dirIndex.model(), static_cast<const QAbstractItemModel*>(m_dirModel));
    //QCOMPARE(m_dirIndex.row(), 3);
    QCOMPARE(m_dirIndex.column(), 0);
    QVERIFY(!m_dirIndex.parent().isValid());
    QVERIFY(m_dirModel->hasChildren(m_dirIndex));

    // Index of a file inside a directory (subdir/testfile)
    QVERIFY(m_fileInDirIndex.isValid());
    QCOMPARE(m_fileInDirIndex.model(), static_cast<const QAbstractItemModel*>(m_dirModel));
    //QCOMPARE(m_fileInDirIndex.row(), 0);
    QCOMPARE(m_fileInDirIndex.column(), 0);
    QVERIFY(m_fileInDirIndex.parent() == m_dirIndex);
    QVERIFY(!m_dirModel->hasChildren(m_fileInDirIndex));

    // Index of subdir/subsubdir/testfile
    QVERIFY(m_fileInSubdirIndex.isValid());
    QCOMPARE(m_fileInSubdirIndex.model(), static_cast<const QAbstractItemModel*>(m_dirModel));
    //QCOMPARE(m_fileInSubdirIndex.row(), 0);
    QCOMPARE(m_fileInSubdirIndex.column(), 0);
    QVERIFY(m_fileInSubdirIndex.parent().parent() == m_dirIndex);
    QVERIFY(!m_dirModel->hasChildren(m_fileInSubdirIndex));
}

void KDirModelTest::testNames()
{
    QString fileName = m_dirModel->data(m_fileIndex, Qt::DisplayRole).toString();
    QCOMPARE(fileName, QString("toplevelfile_1"));

    QString specialFileName = m_dirModel->data(m_specialFileIndex, Qt::DisplayRole).toString();
    QCOMPARE(specialFileName, QString(SPECIALCHARS));

    QString dirName = m_dirModel->data(m_dirIndex, Qt::DisplayRole).toString();
    QCOMPARE(dirName, QString("subdir"));

    QString fileInDirName = m_dirModel->data(m_fileInDirIndex, Qt::DisplayRole).toString();
    QCOMPARE(fileInDirName, QString("testfile"));

    QString fileInSubdirName = m_dirModel->data(m_fileInSubdirIndex, Qt::DisplayRole).toString();
    QCOMPARE(fileInSubdirName, QString("testfile"));
}

void KDirModelTest::testItemForIndex()
{
    // root item
    KFileItem rootItem = m_dirModel->itemForIndex(QModelIndex());
    QVERIFY(!rootItem.isNull());
    QCOMPARE(rootItem.name(), QString("."));

    KFileItem fileItem = m_dirModel->itemForIndex(m_fileIndex);
    QVERIFY(!fileItem.isNull());
    QCOMPARE(fileItem.name(), QString("toplevelfile_1"));
    QVERIFY(!fileItem.isDir());
    QCOMPARE(fileItem.url().path(), m_tempDir->name() + "toplevelfile_1");

    KFileItem dirItem = m_dirModel->itemForIndex(m_dirIndex);
    QVERIFY(!dirItem.isNull());
    QCOMPARE(dirItem.name(), QString("subdir"));
    QVERIFY(dirItem.isDir());
    QCOMPARE(dirItem.url().path(), m_tempDir->name() + "subdir");

    KFileItem fileInDirItem = m_dirModel->itemForIndex(m_fileInDirIndex);
    QVERIFY(!fileInDirItem.isNull());
    QCOMPARE(fileInDirItem.name(), QString("testfile"));
    QVERIFY(!fileInDirItem.isDir());
    QCOMPARE(fileInDirItem.url().path(), m_tempDir->name() + "subdir/testfile");

    KFileItem fileInSubdirItem = m_dirModel->itemForIndex(m_fileInSubdirIndex);
    QVERIFY(!fileInSubdirItem.isNull());
    QCOMPARE(fileInSubdirItem.name(), QString("testfile"));
    QVERIFY(!fileInSubdirItem.isDir());
    QCOMPARE(fileInSubdirItem.url().path(), m_tempDir->name() + "subdir/subsubdir/testfile");
}

void KDirModelTest::testIndexForItem()
{
    KFileItem rootItem = m_dirModel->itemForIndex(QModelIndex());
    QModelIndex rootIndex = m_dirModel->indexForItem(rootItem);
    QVERIFY(!rootIndex.isValid());

    KFileItem fileItem = m_dirModel->itemForIndex(m_fileIndex);
    QModelIndex fileIndex = m_dirModel->indexForItem(fileItem);
    QCOMPARE(fileIndex, m_fileIndex);

    KFileItem dirItem = m_dirModel->itemForIndex(m_dirIndex);
    QModelIndex dirIndex = m_dirModel->indexForItem(dirItem);
    QCOMPARE(dirIndex, m_dirIndex);

    KFileItem fileInDirItem = m_dirModel->itemForIndex(m_fileInDirIndex);
    QModelIndex fileInDirIndex = m_dirModel->indexForItem(fileInDirItem);
    QCOMPARE(fileInDirIndex, m_fileInDirIndex);

    KFileItem fileInSubdirItem = m_dirModel->itemForIndex(m_fileInSubdirIndex);
    QModelIndex fileInSubdirIndex = m_dirModel->indexForItem(fileInSubdirItem);
    QCOMPARE(fileInSubdirIndex, m_fileInSubdirIndex);
}

void KDirModelTest::testData()
{
    // First file
    QModelIndex idx1col1 = m_dirModel->index(m_fileIndex.row(), 1, QModelIndex());
    int size1 = m_dirModel->data(idx1col1, Qt::DisplayRole).toInt();
    QCOMPARE(size1, 11);

    KFileItem item = m_dirModel->data(m_fileIndex, KDirModel::FileItemRole).value<KFileItem>();
    KFileItem fileItem = m_dirModel->itemForIndex(m_fileIndex);
    QCOMPARE(item, fileItem);

    QCOMPARE(m_dirModel->data(m_fileIndex, KDirModel::ChildCountRole).toInt(), (int)KDirModel::ChildCountUnknown);



    // Second file
    QModelIndex idx2col0 = m_dirModel->index(m_secondFileIndex.row(), 0, QModelIndex());
    QString display2 = m_dirModel->data(idx2col0, Qt::DisplayRole).toString();
    QCOMPARE(display2, QString("toplevelfile_2"));

    // Subdir: check child count
    QCOMPARE(m_dirModel->data(m_dirIndex, KDirModel::ChildCountRole).toInt(), 3);

    // Subsubdir: check child count
    QCOMPARE(m_dirModel->data(m_fileInSubdirIndex.parent(), KDirModel::ChildCountRole).toInt(), 1);
}

void KDirModelTest::testReload()
{
    fillModel( true );
    testItemForIndex();
}

Q_DECLARE_METATYPE(QModelIndex) // needed for .value<QModelIndex>()

// We want more info than just "the values differ", if they do.
#define COMPARE_INDEXES(a, b) \
    QCOMPARE(a.row(), b.row()); \
    QCOMPARE(a.column(), b.column()); \
    QCOMPARE(a.model(), b.model()); \
    QCOMPARE(a.parent().isValid(), b.parent().isValid()); \
    QCOMPARE(a, b);

void KDirModelTest::testModifyFile()
{
    const QString file = m_tempDir->name() + "toplevelfile_2";
    const KUrl url(file);

    QSignalSpy spyDataChanged(m_dirModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)));
    connect( m_dirModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
             &m_eventLoop, SLOT(exitLoop()) );

    // "Touch" the file
    setTimeStamp(file, s_referenceTimeStamp.addSecs(20) );

    // In stat mode, kdirwatch doesn't notice file changes; we need to trigger it
    // by creating a file.
    //createTestFile(m_tempDir->name() + "toplevelfile_5");
    KDirWatch::self()->setDirty(m_tempDir->name());

    // Wait for KDirWatch to notify the change (especially when using Stat)
    enterLoop();

    // If we come here, then dataChanged() was emitted - all good.
    QCOMPARE(spyDataChanged.count(), 1);
    QModelIndex receivedIndex = spyDataChanged[0][0].value<QModelIndex>();
    COMPARE_INDEXES(receivedIndex, m_secondFileIndex);
    receivedIndex = spyDataChanged[0][1].value<QModelIndex>();
    QCOMPARE(receivedIndex.row(), m_secondFileIndex.row()); // only compare row; column is count-1

    disconnect( m_dirModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                &m_eventLoop, SLOT(exitLoop()) );
}

void KDirModelTest::testRenameFile()
{
    const KUrl url(m_tempDir->name() + "toplevelfile_2");
    const KUrl newUrl(m_tempDir->name() + "toplevelfile_2_renamed");

    QSignalSpy spyDataChanged(m_dirModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)));
    connect( m_dirModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
             &m_eventLoop, SLOT(exitLoop()) );

    KIO::SimpleJob* job = KIO::rename(url, newUrl, KIO::HideProgressInfo);
    QVERIFY(job->exec());

    // Wait for the DBUS signal from KDirNotify, it's the one the triggers dataChanged
    enterLoop();

    // If we come here, then dataChanged() was emitted - all good.
    QCOMPARE(spyDataChanged.count(), 1);
    COMPARE_INDEXES(spyDataChanged[0][0].value<QModelIndex>(), m_secondFileIndex);
    QModelIndex receivedIndex = spyDataChanged[0][1].value<QModelIndex>();
    QCOMPARE(receivedIndex.row(), m_secondFileIndex.row()); // only compare row; column is count-1

    // check renaming happened
    QCOMPARE( m_dirModel->itemForIndex( m_secondFileIndex ).url().url(), newUrl.url() );

    // check that KDirLister::cachedItemForUrl won't give a bad name if copying that item (#195385)
    KFileItem cachedItem = KDirLister::cachedItemForUrl(newUrl);
    QVERIFY(!cachedItem.isNull());
    QCOMPARE(cachedItem.name(), QString("toplevelfile_2_renamed"));
    QCOMPARE(cachedItem.entry().stringValue(KIO::UDSEntry::UDS_NAME), QString("toplevelfile_2_renamed"));

    // Put things back to normal
    job = KIO::rename(newUrl, url, KIO::HideProgressInfo);
    QVERIFY(job->exec());
    // Wait for the DBUS signal from KDirNotify, it's the one the triggers dataChanged
    enterLoop();
    QCOMPARE( m_dirModel->itemForIndex( m_secondFileIndex ).url().url(), url.url() );

    disconnect( m_dirModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                &m_eventLoop, SLOT(exitLoop()) );
}

void KDirModelTest::testMoveDirectory()
{
    testMoveDirectory("subdir");
}

void KDirModelTest::testMoveDirectory(const QString& dir /*just a dir name, no slash*/)
{
    const QString path = m_tempDir->name();
    const QString srcdir = path + dir;
    QVERIFY(QDir(srcdir).exists());
    KTempDir destDir;
    const QString dest = destDir.name();
    QVERIFY(QDir(dest).exists());

    connect(m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            &m_eventLoop, SLOT(exitLoop()));

    // Move
    kDebug() << "Moving" << srcdir << "to" << dest;
    KIO::CopyJob* job = KIO::move(KUrl(srcdir), KUrl(dest), KIO::HideProgressInfo);
    job->setUiDelegate(0);
    QVERIFY(KIO::NetAccess::synchronousRun(job, 0));

    // wait for kdirnotify
    enterLoop();

    disconnect(m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
               &m_eventLoop, SLOT(exitLoop()));

    QVERIFY(!m_dirModel->indexForUrl(path + "subdir").isValid());
    QVERIFY(!m_dirModel->indexForUrl(path + "subdir_renamed").isValid());

    connect(m_dirModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            &m_eventLoop, SLOT(exitLoop()));

    // Move back
    kDebug() << "Moving" << dest+dir << "back to" << srcdir;
    job = KIO::move(KUrl(dest + dir), KUrl(srcdir), KIO::HideProgressInfo);
    job->setUiDelegate(0);
    QVERIFY(KIO::NetAccess::synchronousRun(job, 0));

    enterLoop();

    QVERIFY(QDir(srcdir).exists());
    disconnect(m_dirModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            &m_eventLoop, SLOT(exitLoop()));

    // m_dirIndex is invalid after the above...
    fillModel(true);
}

void KDirModelTest::testRenameDirectory() // #172945, #174703, (and #180156)
{
    const QString path = m_tempDir->name();
    const KUrl url(path + "subdir");
    const KUrl newUrl(path + "subdir_renamed");

    // For #180156 we need a second kdirmodel, viewing the subdir being renamed.
    // I'm abusing m_dirModelForExpand for that purpose.
    delete m_dirModelForExpand;
    m_dirModelForExpand = new KDirModel;
    KDirLister* dirListerForExpand = m_dirModelForExpand->dirLister();
    connect(dirListerForExpand, SIGNAL(completed()), this, SLOT(slotListingCompleted()));
    dirListerForExpand->openUrl(url); // async
    enterLoop();

    // Now do the renaming
    QSignalSpy spyDataChanged(m_dirModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)));
    connect( m_dirModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
             &m_eventLoop, SLOT(exitLoop()) );
    KIO::SimpleJob* job = KIO::rename(url, newUrl, KIO::HideProgressInfo);
    QVERIFY(job->exec());

    // Wait for the DBUS signal from KDirNotify, it's the one the triggers dataChanged
    enterLoop();

    // If we come here, then dataChanged() was emitted - all good.
    //QCOMPARE(spyDataChanged.count(), 1); // it was in fact emitted 5 times...
    //COMPARE_INDEXES(spyDataChanged[0][0].value<QModelIndex>(), m_dirIndex);
    //QModelIndex receivedIndex = spyDataChanged[0][1].value<QModelIndex>();
    //QCOMPARE(receivedIndex.row(), m_dirIndex.row()); // only compare row; column is count-1

    // check renaming happened
    QCOMPARE(m_dirModel->itemForIndex(m_dirIndex).url().url(), newUrl.url());
    QCOMPARE(m_dirModel->indexForUrl(newUrl), m_dirIndex);
    QVERIFY(m_dirModel->indexForUrl(path + "subdir_renamed").isValid());
    QVERIFY(m_dirModel->indexForUrl(path + "subdir_renamed/testfile").isValid());
    QVERIFY(m_dirModel->indexForUrl(path + "subdir_renamed/subsubdir").isValid());
    QVERIFY(m_dirModel->indexForUrl(path + "subdir_renamed/subsubdir/testfile").isValid());

    // Check the other kdirmodel got redirected
    QCOMPARE(dirListerForExpand->url().path(), path+"subdir_renamed");

    kDebug() << "calling testMoveDirectory(subdir_renamed)";

    // Test moving the renamed directory; if something inside KDirModel
    // wasn't properly updated by the renaming, this would detect it and crash (#180673)
    testMoveDirectory("subdir_renamed");

    // Put things back to normal
    job = KIO::rename(newUrl, url, KIO::HideProgressInfo);
    QVERIFY(job->exec());
    // Wait for the DBUS signal from KDirNotify, it's the one the triggers dataChanged
    enterLoop();
    QCOMPARE(m_dirModel->itemForIndex(m_dirIndex).url().url(), url.url());

    disconnect( m_dirModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                &m_eventLoop, SLOT(exitLoop()) );

    QCOMPARE(m_dirModel->itemForIndex(m_dirIndex).url().url(), url.url());
    QCOMPARE(m_dirModel->indexForUrl(url), m_dirIndex);
    QVERIFY(m_dirModel->indexForUrl(path + "subdir").isValid());
    QVERIFY(m_dirModel->indexForUrl(path + "subdir/testfile").isValid());
    QVERIFY(m_dirModel->indexForUrl(path + "subdir/subsubdir").isValid());
    QVERIFY(m_dirModel->indexForUrl(path + "subdir/subsubdir/testfile").isValid());
    QVERIFY(!m_dirModel->indexForUrl(path + "subdir_renamed").isValid());
    QVERIFY(!m_dirModel->indexForUrl(path + "subdir_renamed/testfile").isValid());
    QVERIFY(!m_dirModel->indexForUrl(path + "subdir_renamed/subsubdir").isValid());
    QVERIFY(!m_dirModel->indexForUrl(path + "subdir_renamed/subsubdir/testfile").isValid());

    // TODO INVESTIGATE
    // QCOMPARE(dirListerForExpand->url().path(), path+"subdir");

    delete m_dirModelForExpand;
    m_dirModelForExpand = 0;
}

void KDirModelTest::testRenameDirectoryInCache() // #188807
{
    // Ensure the stuff is in cache.
    fillModel(true);
    const QString path = m_tempDir->name();
    QVERIFY(!m_dirModel->dirLister()->findByUrl(path).isNull());

    // No more dirmodel nor dirlister.
    delete m_dirModel;
    m_dirModel = 0;

    // Now let's rename a directory that is in KDirListerCache
    const KUrl url(path);
    KUrl newUrl(path);
    newUrl.adjustPath(KUrl::RemoveTrailingSlash);
    newUrl.setPath(newUrl.path() + "_renamed");
    kDebug() << newUrl;
    KIO::SimpleJob* job = KIO::rename(url, newUrl, KIO::HideProgressInfo);
    QVERIFY(job->exec());

    // Put things back to normal
    job = KIO::rename(newUrl, url, KIO::HideProgressInfo);
    QVERIFY(job->exec());

    // KDirNotify emits FileRenamed for each rename() above, which in turn
    // re-lists the directory. We need to wait for both signals to be emitted
    // otherwise the dirlister will not be in the state we expect.
    QTest::qWait(200);

    fillModel(true);

    QVERIFY(m_dirIndex.isValid());
    KFileItem rootItem = m_dirModel->dirLister()->findByUrl(path);
    QVERIFY(!rootItem.isNull());
}

void KDirModelTest::testChmodDirectory() // #53397
{
    QSignalSpy spyDataChanged(m_dirModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)));
    connect( m_dirModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
             &m_eventLoop, SLOT(exitLoop()) );
    const QString path = m_tempDir->name();
    KFileItem rootItem = m_dirModel->itemForIndex(QModelIndex());
    const mode_t origPerm = rootItem.permissions();
    mode_t newPerm = origPerm ^ S_IWGRP;
    QVERIFY(newPerm != origPerm);
    KFileItemList items; items << rootItem;
    KIO::Job* job = KIO::chmod(items, newPerm, S_IWGRP, QString(), QString(), false, KIO::HideProgressInfo);
    job->setUiDelegate(0);
    QVERIFY(KIO::NetAccess::synchronousRun(job, 0));
    // ChmodJob doesn't talk to KDirNotify, kpropertiesdialog does.
    // [this allows to group notifications after all the changes one can make in the dialog]
    org::kde::KDirNotify::emitFilesChanged( QStringList() << path );
    // Wait for the DBUS signal from KDirNotify, it's the one the triggers rowsRemoved
    enterLoop();

    // If we come here, then dataChanged() was emitted - all good.
    QCOMPARE(spyDataChanged.count(), 1);
    QModelIndex receivedIndex = spyDataChanged[0][0].value<QModelIndex>();
    kDebug() << receivedIndex;
    QVERIFY(!receivedIndex.isValid());

    QCOMPARE(m_dirModel->itemForIndex(QModelIndex()).permissions(), newPerm);

    disconnect( m_dirModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                &m_eventLoop, SLOT(exitLoop()) );
}

void KDirModelTest::testExpandToUrl_data()
{
    QTest::addColumn<bool>("newdir"); // whether to re-create a new KTempDir completely, to avoid cached fileitems
    QTest::addColumn<QString>("expandToPath"); // relative path
    QTest::addColumn<QStringList>("expectedExpandSignals");

    QTest::newRow("the root, nothing to do")
        << false << QString() << QStringList();
    QTest::newRow(".")
        << false << "." << (QStringList());
    QTest::newRow("subdir")
        << false << "subdir" << (QStringList()<<"subdir");
    QTest::newRow("subdir/.")
        << false << "subdir/." << (QStringList()<<"subdir");

    const QString subsubdir = "subdir/subsubdir";
    // Must list root, emit expand for subdir, list subdir, emit expand for subsubdir.
    QTest::newRow("subdir/subsubdir")
        << false << subsubdir << (QStringList()<<"subdir"<<subsubdir);

    // Must list root, emit expand for subdir, list subdir, emit expand for subsubdir, list subsubdir.
    const QString subsubdirfile = subsubdir + "/testfile";
    QTest::newRow("subdir/subsubdir/testfile sync")
        << false << subsubdirfile << (QStringList()<<"subdir"<<subsubdir<<subsubdirfile);

#ifndef Q_WS_WIN
    // Expand a symlink to a directory (#219547)
    const QString dirlink = m_tempDir->name() + "dirlink";
    createTestSymlink(dirlink, "/");
    QTest::newRow("dirlink")
        << false << "dirlink/tmp" << (QStringList()<<"dirlink"<<"dirlink/tmp");
#endif

    // Do a cold-cache test too, but nowadays it doesn't change anything anymore,
    // apart from testing different code paths inside KDirLister.
    QTest::newRow("subdir/subsubdir/testfile with reload")
        << true << subsubdirfile << (QStringList()<<"subdir"<<subsubdir<<subsubdirfile);
}

void KDirModelTest::testExpandToUrl()
{
    QFETCH(bool, newdir);
    QFETCH(QString, expandToPath); // relative
    QFETCH(QStringList, expectedExpandSignals);

    if (newdir) {
        recreateTestData();
        // WARNING! m_dirIndex, m_fileIndex, m_secondFileIndex etc. are not valid anymore after this point!

    }

    const QString path = m_tempDir->name();
    if (!m_dirModelForExpand || newdir) {
        delete m_dirModelForExpand;
        m_dirModelForExpand = new KDirModel;
        connect(m_dirModelForExpand, SIGNAL(expand(QModelIndex)),
                this, SLOT(slotExpand(QModelIndex)));
        connect(m_dirModelForExpand, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(slotRowsInserted(QModelIndex,int,int)));
        KDirLister* dirListerForExpand = m_dirModelForExpand->dirLister();
        dirListerForExpand->openUrl(KUrl(path), KDirLister::NoFlags); // async
    }
    m_rowsInsertedEmitted = false;
    m_expectedExpandSignals = expectedExpandSignals;
    m_nextExpectedExpandSignals = 0;
    QSignalSpy spyExpand(m_dirModelForExpand, SIGNAL(expand(QModelIndex)));
    m_urlToExpandTo = KUrl(path + expandToPath);
    // If KDirModel doesn't know this URL yet, then we want to see rowsInserted signals
    // being emitted, so that the slots can get the index to that url then.
    m_expectRowsInserted = !expandToPath.isEmpty() && !m_dirModelForExpand->indexForUrl(m_urlToExpandTo).isValid();
    m_dirModelForExpand->expandToUrl(m_urlToExpandTo);
    if (expectedExpandSignals.isEmpty()) {
        QTest::qWait(20); // to make sure we process queued connection calls, otherwise spyExpand.count() is always 0 even if there's a bug...
        QCOMPARE(spyExpand.count(), 0);
    } else {
        if (spyExpand.count() < expectedExpandSignals.count()) {
            enterLoop();
            QCOMPARE(spyExpand.count(), expectedExpandSignals.count());
        }
        if (m_expectRowsInserted)
            QVERIFY(m_rowsInsertedEmitted);
    }

    // Now it should exist
    if (!expandToPath.isEmpty() && expandToPath != ".") {
        kDebug() << "Do I know" << m_urlToExpandTo << "?";
        QVERIFY(m_dirModelForExpand->indexForUrl(m_urlToExpandTo).isValid());
    }

    // recreateTestData was called -> fill again, for the next tests
    if (newdir)
        fillModel(false);
}

void KDirModelTest::slotExpand(const QModelIndex& index)
{
    QVERIFY(index.isValid());
    const QString path = m_tempDir->name();
    KFileItem item = m_dirModelForExpand->itemForIndex(index);
    QVERIFY(!item.isNull());
    kDebug() << item.url().path();
    QCOMPARE(item.url().path(), path + m_expectedExpandSignals[m_nextExpectedExpandSignals++]);

    // if rowsInserted wasn't emitted yet, then any proxy model would be unable to do anything with index at this point
    if (item.url() == m_urlToExpandTo) {
        QVERIFY(m_dirModelForExpand->indexForUrl(m_urlToExpandTo).isValid());
        if (m_expectRowsInserted)
            QVERIFY(m_rowsInsertedEmitted);
    }

    if (m_nextExpectedExpandSignals == m_expectedExpandSignals.count())
        m_eventLoop.exitLoop(); // done
}

void KDirModelTest::slotRowsInserted(const QModelIndex&, int, int)
{
    m_rowsInsertedEmitted = true;
}

void KDirModelTest::testFilter()
{
    QVERIFY(m_dirIndex.isValid());
    const int oldTopLevelRowCount = m_dirModel->rowCount();
    const int oldSubdirRowCount = m_dirModel->rowCount(m_dirIndex);
    QSignalSpy spyItemsFilteredByMime(m_dirModel->dirLister(), SIGNAL(itemsFilteredByMime(KFileItemList)));
    QSignalSpy spyItemsDeleted(m_dirModel->dirLister(), SIGNAL(itemsDeleted(KFileItemList)));
    QSignalSpy spyRowsRemoved(m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    m_dirModel->dirLister()->setNameFilter("toplevel*");
    QCOMPARE(m_dirModel->rowCount(), oldTopLevelRowCount); // no change yet
    QCOMPARE(m_dirModel->rowCount(m_dirIndex), oldSubdirRowCount); // no change yet
    m_dirModel->dirLister()->emitChanges();

    QCOMPARE(m_dirModel->rowCount(), 4); // 3 toplevel* files, one subdir
    QCOMPARE(m_dirModel->rowCount(m_dirIndex), 1); // the files get filtered out, the subdir remains

    QCOMPARE(spyRowsRemoved.count(), 3); // once for every dir
    QCOMPARE(spyItemsDeleted.count(), 3); // once for every dir
    QCOMPARE(spyItemsDeleted[0][0].value<KFileItemList>().count(), 1); // one from toplevel ('specialchars')
    QCOMPARE(spyItemsDeleted[1][0].value<KFileItemList>().count(), 2); // two from subdir
    QCOMPARE(spyItemsDeleted[2][0].value<KFileItemList>().count(), 1); // one from subsubdir
    QCOMPARE(spyItemsFilteredByMime.count(), 0);
    spyItemsDeleted.clear();
    spyItemsFilteredByMime.clear();

    // Reset the filter
    kDebug() << "reset to no filter";
    m_dirModel->dirLister()->setNameFilter(QString());
    m_dirModel->dirLister()->emitChanges();

    QCOMPARE(m_dirModel->rowCount(), oldTopLevelRowCount);
    QCOMPARE(m_dirModel->rowCount(m_dirIndex), oldSubdirRowCount);
    QCOMPARE(spyItemsDeleted.count(), 0);
    QCOMPARE(spyItemsFilteredByMime.count(), 0);

    // The order of things changed because of filtering.
    // Fill again, so that m_fileIndex etc. are correct again.
    fillModel(true);
}

void KDirModelTest::testMimeFilter()
{
    QVERIFY(m_dirIndex.isValid());
    const int oldTopLevelRowCount = m_dirModel->rowCount();
    const int oldSubdirRowCount = m_dirModel->rowCount(m_dirIndex);
    QSignalSpy spyItemsFilteredByMime(m_dirModel->dirLister(), SIGNAL(itemsFilteredByMime(KFileItemList)));
    QSignalSpy spyItemsDeleted(m_dirModel->dirLister(), SIGNAL(itemsDeleted(KFileItemList)));
    QSignalSpy spyRowsRemoved(m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    m_dirModel->dirLister()->setMimeFilter(QStringList() << "application/pdf");
    QCOMPARE(m_dirModel->rowCount(), oldTopLevelRowCount); // no change yet
    QCOMPARE(m_dirModel->rowCount(m_dirIndex), oldSubdirRowCount); // no change yet
    m_dirModel->dirLister()->emitChanges();

    QCOMPARE(m_dirModel->rowCount(), 1); // 1 pdf files, no subdir anymore

    QVERIFY(spyRowsRemoved.count() >= 1); // depends on contiguity...
    QVERIFY(spyItemsDeleted.count() >= 1); // once for every dir
    // Maybe it would make sense to have those items in itemsFilteredByMime,
    // but well, for the only existing use of that signal (mime filter plugin),
    // it's not really necessary, the plugin has seen those files before anyway.
    // The signal is mostly useful for the case of listing a dir with a mime filter set.
    //QCOMPARE(spyItemsFilteredByMime.count(), 1);
    //QCOMPARE(spyItemsFilteredByMime[0][0].value<KFileItemList>().count(), 4);
    spyItemsDeleted.clear();
    spyItemsFilteredByMime.clear();

    // Reset the filter
    kDebug() << "reset to no filter";
    m_dirModel->dirLister()->setMimeFilter(QStringList());
    m_dirModel->dirLister()->emitChanges();

    QCOMPARE(m_dirModel->rowCount(), oldTopLevelRowCount);
    QCOMPARE(spyItemsDeleted.count(), 0);
    QCOMPARE(spyItemsFilteredByMime.count(), 0);

    // The order of things changed because of filtering.
    // Fill again, so that m_fileIndex etc. are correct again.
    fillModel(true);
}

void KDirModelTest::testShowHiddenFiles() // #174788
{
    KDirLister* dirLister = m_dirModel->dirLister();

    QSignalSpy spyRowsRemoved(m_dirModel, SIGNAL(rowsRemoved(QModelIndex, int, int)));
    QSignalSpy spyNewItems(dirLister, SIGNAL(newItems(KFileItemList)));
    QSignalSpy spyRowsInserted(m_dirModel, SIGNAL(rowsInserted(QModelIndex,int,int)));
    dirLister->setShowingDotFiles(true);
    dirLister->emitChanges();
    const int numberOfDotFiles = 2;
    QCOMPARE(spyNewItems.count(), 1);
    QCOMPARE(spyNewItems[0][0].value<KFileItemList>().count(), numberOfDotFiles);
    QCOMPARE(spyRowsInserted.count(), 1);
    QCOMPARE(spyRowsRemoved.count(), 0);
    spyNewItems.clear();
    spyRowsInserted.clear();

    dirLister->setShowingDotFiles(false);
    dirLister->emitChanges();
    QCOMPARE(spyNewItems.count(), 0);
    QCOMPARE(spyRowsInserted.count(), 0);
    QCOMPARE(spyRowsRemoved.count(), 1);
}

void KDirModelTest::testMultipleSlashes()
{
    const QString path = m_tempDir->name();

    QModelIndex index = m_dirModel->indexForUrl(KUrl(path+"subdir//testfile"));
    QVERIFY(index.isValid());

    index = m_dirModel->indexForUrl(path+"subdir//subsubdir//");
    QVERIFY(index.isValid());

    index = m_dirModel->indexForUrl(path+"subdir///subsubdir////testfile");
    QVERIFY(index.isValid());
}

void KDirModelTest::testUrlWithRef() // #171117
{
    const QString path = m_tempDir->name();
    KDirLister* dirLister = m_dirModel->dirLister();
    KUrl url(path);
    url.setRef("ref");
    QVERIFY(url.url().endsWith("#ref"));
    dirLister->openUrl(url, KDirLister::NoFlags);
    connect(dirLister, SIGNAL(completed()), this, SLOT(slotListingCompleted()));
    enterLoop();

    QCOMPARE(dirLister->url().url(), url.url(KUrl::RemoveTrailingSlash));
    collectKnownIndexes();
    disconnect(dirLister, SIGNAL(completed()), this, SLOT(slotListingCompleted()));
}

void KDirModelTest::testFontUrlWithHost() // #160057
{
    if (!KProtocolInfo::isKnownProtocol("fonts")) {
        QSKIP("kio_fonts not installed", SkipAll);
    }
    KUrl url("fonts://foo/System");
    KDirLister* dirLister = m_dirModel->dirLister();
    dirLister->openUrl(url, KDirLister::NoFlags);
    connect(dirLister, SIGNAL(completed()), this, SLOT(slotListingCompleted()));
    enterLoop();

    QCOMPARE(dirLister->url().url(), QString("fonts:/System"));
}

void KDirModelTest::testRemoteUrlWithHost() // #178416
{
    if (!KProtocolInfo::isKnownProtocol("remote")) {
        QSKIP("kio_remote not installed", SkipAll);
    }
    KUrl url("remote://foo");
    KDirLister* dirLister = m_dirModel->dirLister();
    dirLister->openUrl(url, KDirLister::NoFlags);
    connect(dirLister, SIGNAL(completed()), this, SLOT(slotListingCompleted()));
    enterLoop();

    QCOMPARE(dirLister->url().url(), QString("remote:"));
}

void KDirModelTest::testZipFile() // # 171721
{
    const QString path = KDESRCDIR;
    KDirLister* dirLister = m_dirModel->dirLister();
    dirLister->openUrl(KUrl(path), KDirLister::NoFlags);
    connect(dirLister, SIGNAL(completed()), this, SLOT(slotListingCompleted()));
    enterLoop();
    disconnect(dirLister, SIGNAL(completed()), this, SLOT(slotListingCompleted()));

    KUrl zipUrl(path);
    zipUrl.addPath("wronglocalsizes.zip"); // just a zip file lying here for other reasons
    QVERIFY(QFile::exists(zipUrl.path()));
    zipUrl.setProtocol("zip");
    QModelIndex index = m_dirModel->indexForUrl(zipUrl);
    QVERIFY(!index.isValid()); // protocol mismatch, can't find it!
    zipUrl.setProtocol("file");
    index = m_dirModel->indexForUrl(zipUrl);
    QVERIFY(index.isValid());
}

void KDirModelTest::testSmb()
{
    const KUrl smbUrl("smb:/");
    // TODO: feed a KDirModel without using a KDirLister.
    // Calling the slots directly.
    // This requires that KDirModel does not ask the KDirLister for its rootItem anymore,
    // but that KDirLister emits the root item whenever it changes.
    if (!KProtocolInfo::isKnownProtocol("smb")) {
        QSKIP("kio_smb not installed", SkipAll);
    }
    KDirLister* dirLister = m_dirModel->dirLister();
    dirLister->openUrl(smbUrl, KDirLister::NoFlags);
    connect(dirLister, SIGNAL(completed()), this, SLOT(slotListingCompleted()));
    connect(dirLister, SIGNAL(canceled()), this, SLOT(slotListingCompleted()));
    QSignalSpy spyCanceled(dirLister, SIGNAL(canceled()));
    enterLoop(); // wait for completed signal

    if (spyCanceled.count() > 0) {
        QSKIP("smb:/ returns an error, probably no network available", SkipAll);
    }

    QModelIndex index = m_dirModel->index(0, 0);
    if (index.isValid()) {
        QVERIFY(m_dirModel->canFetchMore(index));
        m_dirModel->fetchMore(index);
        enterLoop(); // wait for completed signal
        disconnect(dirLister, SIGNAL(completed()), this, SLOT(slotListingCompleted()));
    }
}

void KDirModelTest::testDeleteFile()
{
    fillModel(false);

    QVERIFY(m_fileIndex.isValid());
    const int oldTopLevelRowCount = m_dirModel->rowCount();
    const QString path = m_tempDir->name();
    const QString file = path + "toplevelfile_1";
    const KUrl url(file);

    QSignalSpy spyRowsRemoved(m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    connect( m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
             &m_eventLoop, SLOT(exitLoop()) );

    KIO::DeleteJob* job = KIO::del(url, KIO::HideProgressInfo);
    QVERIFY(job->exec());

    // Wait for the DBUS signal from KDirNotify, it's the one the triggers rowsRemoved
    enterLoop();

    // If we come here, then rowsRemoved() was emitted - all good.
    const int topLevelRowCount = m_dirModel->rowCount();
    QCOMPARE(topLevelRowCount, oldTopLevelRowCount - 1); // one less than before
    QCOMPARE(spyRowsRemoved.count(), 1);
    QCOMPARE(spyRowsRemoved[0][1].toInt(), m_fileIndex.row());
    QCOMPARE(spyRowsRemoved[0][2].toInt(), m_fileIndex.row());
    disconnect( m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                &m_eventLoop, SLOT(exitLoop()) );

    QModelIndex fileIndex = m_dirModel->indexForUrl(path + "toplevelfile_1");
    QVERIFY(!fileIndex.isValid());

    // Recreate the file, for consistency in the next tests
    // So the second part of this test is a "testCreateFile"
    createTestFile(file);
    // Tricky problem - KDirLister::openUrl will emit items from cache
    // and then schedule an update; so just calling fillModel would
    // not wait enough, it would abort due to not finding toplevelfile_1
    // in the items from cache. This progressive-emitting behavior is fine
    // for GUIs but not for unit tests ;-)
    fillModel(true, false);
    fillModel(false);
}

void KDirModelTest::testDeleteFileWhileListing() // doesn't really test that yet, the kdirwatch deleted signal comes too late
{
    const int oldTopLevelRowCount = m_dirModel->rowCount();
    const QString path = m_tempDir->name();
    const QString file = path + "toplevelfile_1";
    const KUrl url(file);

    KDirLister* dirLister = m_dirModel->dirLister();
    QSignalSpy spyCompleted(dirLister, SIGNAL(completed()));
    connect(dirLister, SIGNAL(completed()), this, SLOT(slotListingCompleted()));
    dirLister->openUrl(KUrl(path), KDirLister::NoFlags);
    if (!spyCompleted.isEmpty())
        QSKIP("listing completed too early", SkipAll);
    QSignalSpy spyRowsRemoved(m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    KIO::DeleteJob* job = KIO::del(url, KIO::HideProgressInfo);
    QVERIFY(job->exec());

    if (spyCompleted.isEmpty())
        enterLoop();
    // TODO QTest::kWaitForSignalSpy(spyRowsRemoved)?
    QTest::kWaitForSignal(m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    const int topLevelRowCount = m_dirModel->rowCount();
    QCOMPARE(topLevelRowCount, oldTopLevelRowCount - 1); // one less than before
    QCOMPARE(spyRowsRemoved.count(), 1);
    QCOMPARE(spyRowsRemoved[0][1].toInt(), m_fileIndex.row());
    QCOMPARE(spyRowsRemoved[0][2].toInt(), m_fileIndex.row());

    QModelIndex fileIndex = m_dirModel->indexForUrl(path + "toplevelfile_1");
    QVERIFY(!fileIndex.isValid());

    kDebug() << "Test done, recreating file";

    // Recreate the file, for consistency in the next tests
    // So the second part of this test is a "testCreateFile"
    createTestFile(file);
    fillModel(true, false); // see testDeleteFile
    fillModel(false);
}

void KDirModelTest::testOverwriteFileWithDir() // #151851 c4
{
    fillModel(false);
    const QString path = m_tempDir->name();
    const QString dir = path + "subdir";
    const QString file = path + "toplevelfile_1";
    const int oldTopLevelRowCount = m_dirModel->rowCount();

    QSignalSpy spyRowsRemoved(m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    connect( m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
             &m_eventLoop, SLOT(exitLoop()) );

    KIO::Job* job = KIO::move(dir, file, KIO::HideProgressInfo);
    PredefinedAnswerJobUiDelegate* delegate = new PredefinedAnswerJobUiDelegate;
    delegate->m_renameResult = KIO::R_OVERWRITE;
    job->setUiDelegate(delegate);
    QVERIFY(job->exec());

    QCOMPARE(delegate->m_askFileRenameCalled, 1);

    if (spyRowsRemoved.isEmpty()) {
        // Wait for the DBUS signal from KDirNotify, it's the one the triggers rowsRemoved
        enterLoop();
        QVERIFY(!spyRowsRemoved.isEmpty());
    }

    // If we come here, then rowsRemoved() was emitted - all good.
    const int topLevelRowCount = m_dirModel->rowCount();
    QCOMPARE(topLevelRowCount, oldTopLevelRowCount - 1); // one less than before

    QVERIFY(!m_dirModel->indexForUrl(dir).isValid());
    QModelIndex newIndex = m_dirModel->indexForUrl(path + "toplevelfile_1");
    QVERIFY(newIndex.isValid());
    KFileItem newItem = m_dirModel->itemForIndex(newIndex);
    QVERIFY(newItem.isDir()); // yes, the file is a dir now ;-)

    kDebug() << "========= Test done, recreating test data =========";

    recreateTestData();
    fillModel(false);
}

void KDirModelTest::testDeleteFiles()
{
    const int oldTopLevelRowCount = m_dirModel->rowCount();
    const QString file = m_tempDir->name() + "toplevelfile_";
    KUrl::List urls;
    urls << KUrl(file + '1') << KUrl(file + '2') << KUrl(file + '3');

    QSignalSpy spyRowsRemoved(m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    KIO::DeleteJob* job = KIO::del(urls, KIO::HideProgressInfo);
    QVERIFY(job->exec());

    int numRowsRemoved = 0;
    while (numRowsRemoved < 3) {

        QTest::qWait(20);

        numRowsRemoved = 0;
        for (int sigNum = 0; sigNum < spyRowsRemoved.count(); ++sigNum)
            numRowsRemoved += spyRowsRemoved[sigNum][2].toInt() - spyRowsRemoved[sigNum][1].toInt() + 1;
        kDebug() << "numRowsRemoved=" << numRowsRemoved;
    }

    const int topLevelRowCount = m_dirModel->rowCount();
    QCOMPARE(topLevelRowCount, oldTopLevelRowCount - 3); // three less than before

    kDebug() << "Recreating test data";
    recreateTestData();
    kDebug() << "Re-filling model";
    fillModel(false);
}

// A renaming that looks more like a deletion to the model
void KDirModelTest::testRenameFileToHidden() // #174721
{
    const KUrl url(m_tempDir->name() + "toplevelfile_2");
    const KUrl newUrl(m_tempDir->name() + ".toplevelfile_2");

    QSignalSpy spyDataChanged(m_dirModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)));
    QSignalSpy spyRowsRemoved(m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    QSignalSpy spyRowsInserted(m_dirModel, SIGNAL(rowsInserted(QModelIndex,int,int)));
    connect( m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
             &m_eventLoop, SLOT(exitLoop()) );

    KIO::SimpleJob* job = KIO::rename(url, newUrl, KIO::HideProgressInfo);
    QVERIFY(job->exec());

    // Wait for the DBUS signal from KDirNotify, it's the one the triggers KDirLister
    enterLoop();

    // If we come here, then rowsRemoved() was emitted - all good.
    QCOMPARE(spyDataChanged.count(), 0);
    QCOMPARE(spyRowsRemoved.count(), 1);
    QCOMPARE(spyRowsInserted.count(), 0);
    COMPARE_INDEXES(spyRowsRemoved[0][0].value<QModelIndex>(), QModelIndex()); // parent is invalid
    const int row = spyRowsRemoved[0][1].toInt();
    QCOMPARE(row, m_secondFileIndex.row()); // only compare row

    disconnect(m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
               &m_eventLoop, SLOT(exitLoop()));
    spyRowsRemoved.clear();

    // Put things back to normal, should make the file reappear
    connect(m_dirModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            &m_eventLoop, SLOT(exitLoop()));
    job = KIO::rename(newUrl, url, KIO::HideProgressInfo);
    QVERIFY(job->exec());
    // Wait for the DBUS signal from KDirNotify, it's the one the triggers KDirLister
    enterLoop();
    QCOMPARE(spyDataChanged.count(), 0);
    QCOMPARE(spyRowsRemoved.count(), 0);
    QCOMPARE(spyRowsInserted.count(), 1);
    int newRow = spyRowsInserted[0][1].toInt();
    m_secondFileIndex = m_dirModel->index(newRow, 0);
    QVERIFY(m_secondFileIndex.isValid());
    QCOMPARE(m_dirModel->itemForIndex( m_secondFileIndex ).url().url(), url.url());
}

void KDirModelTest::testDeleteDirectory()
{
    const QString path = m_tempDir->name();
    const KUrl url(path + "subdir/subsubdir");

    QSignalSpy spyRowsRemoved(m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    connect( m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
             &m_eventLoop, SLOT(exitLoop()) );

    QSignalSpy spyDirWatchDeleted(KDirWatch::self(), SIGNAL(deleted(QString)));

    KIO::DeleteJob* job = KIO::del(url, KIO::HideProgressInfo);
    QVERIFY(job->exec());

    // Wait for the DBUS signal from KDirNotify, it's the one the triggers rowsRemoved
    enterLoop();

    // If we come here, then rowsRemoved() was emitted - all good.
    QCOMPARE(spyRowsRemoved.count(), 1);
    disconnect( m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                &m_eventLoop, SLOT(exitLoop()) );

    QModelIndex deletedDirIndex = m_dirModel->indexForUrl(path + "subdir/subsubdir");
    QVERIFY(!deletedDirIndex.isValid());
    QModelIndex dirIndex = m_dirModel->indexForUrl(path + "subdir");
    QVERIFY(dirIndex.isValid());

    // TODO!!! Bug in KDirWatch? ###
    // QCOMPARE(spyDirWatchDeleted.count(), 1);
}

void KDirModelTest::testDeleteCurrentDirectory()
{
    const int oldTopLevelRowCount = m_dirModel->rowCount();
    const QString path = m_tempDir->name();
    const KUrl url(path);

    QSignalSpy spyRowsRemoved(m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    connect( m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
             &m_eventLoop, SLOT(exitLoop()) );


    KDirWatch::self()->statistics();

    KIO::DeleteJob* job = KIO::del(url, KIO::HideProgressInfo);
    QVERIFY(job->exec());

    // Wait for the DBUS signal from KDirNotify, it's the one the triggers rowsRemoved
    enterLoop();

    // If we come here, then rowsRemoved() was emitted - all good.
    const int topLevelRowCount = m_dirModel->rowCount();
    QCOMPARE(topLevelRowCount, 0); // empty

    // We can get rowsRemoved for subdirs first, since kdirwatch notices that.
    QVERIFY(spyRowsRemoved.count() >= 1);

    // Look for the signal(s) that had QModelIndex() as parent.
    int i;
    int numDeleted = 0;
    for (i = 0; i < spyRowsRemoved.count(); ++i) {
        const int from = spyRowsRemoved[i][1].toInt();
        const int to = spyRowsRemoved[i][2].toInt();
        kDebug() << spyRowsRemoved[i][0].value<QModelIndex>() << from << to;
        if (!spyRowsRemoved[i][0].value<QModelIndex>().isValid()) {
            numDeleted += (to - from) + 1;
        }
    }

    QCOMPARE(numDeleted, oldTopLevelRowCount);
    disconnect( m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                &m_eventLoop, SLOT(exitLoop()) );

    QModelIndex fileIndex = m_dirModel->indexForUrl(path + "toplevelfile_1");
    QVERIFY(!fileIndex.isValid());
}

#if QT_VERSION < 0x040700
// The old slow way. (this isn't QUrl's fault, I'm just using QUrl in order
// to be able to test a different hashing function than the KUrl one).
inline uint qHash(const QUrl& qurl) {
    return qHash(qurl.toEncoded());
}
#endif

void KDirModelTest::testKUrlHash()
{
    const int count = 3000;
    // Prepare an array of QUrls so that url constructing isn't part of the timing
    QVector<QUrl> urls;
    urls.resize(count);
    for (int i = 0; i < count; ++i) {
        urls[i] = QUrl("http://www.kde.org/path/"+QString::number(i));
    }
    QHash<QUrl, int> qurlHash;
    QHash<KUrl, int> kurlHash;
    QTime dt; dt.start();
    for (int i = 0; i < count; ++i) {
        qurlHash.insert(urls[i], i);
    }
    //kDebug() << "inserting" << count << "urls into QHash using old qHash:" << dt.elapsed() << "msecs";
    dt.start();
    for (int i = 0; i < count; ++i) {
        kurlHash.insert(urls[i], i);
    }
    //kDebug() << "inserting" << count << "urls into QHash using new qHash:" << dt.elapsed() << "msecs";
    // Nice results: for count=30000 I got 4515 (before) and 103 (after)

    dt.start();
    for (int i = 0; i < count; ++i) {
        QCOMPARE(qurlHash.value(urls[i]), i);
    }
    //kDebug() << "looking up" << count << "urls into QHash using old qHash:" << dt.elapsed() << "msecs";
    dt.start();
    for (int i = 0; i < count; ++i) {
        QCOMPARE(kurlHash.value(urls[i]), i);
    }
    //kDebug() << "looking up" << count << "urls into QHash using new qHash:" << dt.elapsed() << "msecs";
    // Nice results: for count=30000 I got 4296 (before) and 63 (after)
}
