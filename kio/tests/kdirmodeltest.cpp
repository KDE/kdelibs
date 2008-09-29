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

#ifndef Q_WS_WIN
#define SPECIALCHARS "specialchars%:"
#else
#define SPECIALCHARS "specialchars%"
#endif

void KDirModelTest::initTestCase()
{
    qRegisterMetaType<QModelIndex>("QModelIndex"); // beats me why Qt doesn't do that

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
    delete m_tempDir;
    m_tempDir = new KTempDir;
    // Create test data:
    /*
     * PATH/toplevelfile_1
     * PATH/toplevelfile_2
     * PATH/toplevelfile_3
     * PATH/specialchars%:
     * PATH/subdir
     * PATH/subdir/testfile
     * PATH/subdir/subsubdir
     * PATH/subdir/subsubdir/testfile
     */
    const QString path = m_tempDir->name();
    foreach(const QString &f, m_topLevelFileNames) {
        createTestFile(path+f);
    }
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
}

void KDirModelTest::fillModel( bool reload )
{
    const QString path = m_tempDir->name();
    KDirLister* dirLister = m_dirModel.dirLister();
    dirLister->openUrl(KUrl(path), reload ? KDirLister::Reload : KDirLister::NoFlags);
    connect(dirLister, SIGNAL(completed()), this, SLOT(slotListingCompleted()));
    enterLoop();

    m_dirIndex = QModelIndex();
    m_fileIndex = QModelIndex();
    m_secondFileIndex = QModelIndex();
    // Create the indexes once and for all
    // The trouble is that the order of listing is undefined, one can get 1/2/3/subdir or subdir/3/2/1 for instance.
    for (int row = 0; row < m_topLevelFileNames.count() + 1 /*subdir*/; ++row) {
        QModelIndex idx = m_dirModel.index(row, 0, QModelIndex());
        KFileItem item = m_dirModel.itemForIndex(idx);
        kDebug() << item.url();
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
    QVERIFY(m_dirModel.canFetchMore(m_dirIndex));
    m_dirModel.fetchMore(m_dirIndex);
    enterLoop();

    // Index of a file inside a directory (subdir/testfile)
    QModelIndex subdirIndex;
    m_fileInDirIndex = QModelIndex();
    for (int row = 0; row < 3; ++row) {
        QModelIndex idx = m_dirModel.index(row, 0, m_dirIndex);
        if (m_dirModel.itemForIndex(idx).isDir())
            subdirIndex = idx;
        else if (m_dirModel.itemForIndex(idx).name() == "testfile")
            m_fileInDirIndex = idx;
    }

    // List subdir/subsubdir
    QVERIFY(m_dirModel.canFetchMore(subdirIndex));
    m_dirModel.fetchMore(subdirIndex);
    enterLoop();

    // Index of ... well, subdir/subsubdir/testfile
    m_fileInSubdirIndex = m_dirModel.index(0, 0, subdirIndex);
    disconnect(dirLister, SIGNAL(completed()), this, SLOT(slotListingCompleted()));
}

void KDirModelTest::enterLoop()
{
    m_eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
}

void KDirModelTest::slotListingCompleted()
{
    m_eventLoop.quit();
}

void KDirModelTest::testRowCount()
{
    const int topLevelRowCount = m_dirModel.rowCount();
    QCOMPARE(topLevelRowCount, m_topLevelFileNames.count() + 1 /*subdir*/);
    const int subdirRowCount = m_dirModel.rowCount(m_dirIndex);
    QCOMPARE(subdirRowCount, 3);
}

void KDirModelTest::testIndex()
{
    QVERIFY(m_dirModel.hasChildren());

    // Index of the first file
    QVERIFY(m_fileIndex.isValid());
    QCOMPARE(m_fileIndex.model(), &m_dirModel);
    //QCOMPARE(m_fileIndex.row(), 0);
    QCOMPARE(m_fileIndex.column(), 0);
    QVERIFY(!m_fileIndex.parent().isValid());
    QVERIFY(!m_dirModel.hasChildren(m_fileIndex));

    // Index of a directory
    QVERIFY(m_dirIndex.isValid());
    QCOMPARE(m_dirIndex.model(), &m_dirModel);
    //QCOMPARE(m_dirIndex.row(), 3);
    QCOMPARE(m_dirIndex.column(), 0);
    QVERIFY(!m_dirIndex.parent().isValid());
    QVERIFY(m_dirModel.hasChildren(m_dirIndex));

    // Index of a file inside a directory (subdir/testfile)
    QVERIFY(m_fileInDirIndex.isValid());
    QCOMPARE(m_fileInDirIndex.model(), &m_dirModel);
    //QCOMPARE(m_fileInDirIndex.row(), 0);
    QCOMPARE(m_fileInDirIndex.column(), 0);
    QVERIFY(m_fileInDirIndex.parent() == m_dirIndex);
    QVERIFY(!m_dirModel.hasChildren(m_fileInDirIndex));

    // Index of subdir/subsubdir/testfile
    QVERIFY(m_fileInSubdirIndex.isValid());
    QCOMPARE(m_fileInSubdirIndex.model(), &m_dirModel);
    //QCOMPARE(m_fileInSubdirIndex.row(), 0);
    QCOMPARE(m_fileInSubdirIndex.column(), 0);
    QVERIFY(m_fileInSubdirIndex.parent().parent() == m_dirIndex);
    QVERIFY(!m_dirModel.hasChildren(m_fileInSubdirIndex));
}

void KDirModelTest::testNames()
{
    QString fileName = m_dirModel.data(m_fileIndex, Qt::DisplayRole).toString();
    QCOMPARE(fileName, QString("toplevelfile_1"));

    QString specialFileName = m_dirModel.data(m_specialFileIndex, Qt::DisplayRole).toString();
    QCOMPARE(specialFileName, QString(SPECIALCHARS));

    QString dirName = m_dirModel.data(m_dirIndex, Qt::DisplayRole).toString();
    QCOMPARE(dirName, QString("subdir"));

    QString fileInDirName = m_dirModel.data(m_fileInDirIndex, Qt::DisplayRole).toString();
    QCOMPARE(fileInDirName, QString("testfile"));

    QString fileInSubdirName = m_dirModel.data(m_fileInSubdirIndex, Qt::DisplayRole).toString();
    QCOMPARE(fileInSubdirName, QString("testfile"));
}

void KDirModelTest::testItemForIndex()
{
    // root item
    KFileItem rootItem = m_dirModel.itemForIndex(QModelIndex());
    QVERIFY(!rootItem.isNull());
    QCOMPARE(rootItem.name(), QString("."));

    KFileItem fileItem = m_dirModel.itemForIndex(m_fileIndex);
    QVERIFY(!fileItem.isNull());
    QCOMPARE(fileItem.name(), QString("toplevelfile_1"));
    QVERIFY(!fileItem.isDir());
    QCOMPARE(fileItem.url().path(), m_tempDir->name() + "toplevelfile_1");

    KFileItem dirItem = m_dirModel.itemForIndex(m_dirIndex);
    QVERIFY(!dirItem.isNull());
    QCOMPARE(dirItem.name(), QString("subdir"));
    QVERIFY(dirItem.isDir());
    QCOMPARE(dirItem.url().path(), m_tempDir->name() + "subdir");

    KFileItem fileInDirItem = m_dirModel.itemForIndex(m_fileInDirIndex);
    QVERIFY(!fileInDirItem.isNull());
    QCOMPARE(fileInDirItem.name(), QString("testfile"));
    QVERIFY(!fileInDirItem.isDir());
    QCOMPARE(fileInDirItem.url().path(), m_tempDir->name() + "subdir/testfile");

    KFileItem fileInSubdirItem = m_dirModel.itemForIndex(m_fileInSubdirIndex);
    QVERIFY(!fileInSubdirItem.isNull());
    QCOMPARE(fileInSubdirItem.name(), QString("testfile"));
    QVERIFY(!fileInSubdirItem.isDir());
    QCOMPARE(fileInSubdirItem.url().path(), m_tempDir->name() + "subdir/subsubdir/testfile");
}

void KDirModelTest::testIndexForItem()
{
    KFileItem rootItem = m_dirModel.itemForIndex(QModelIndex());
    QModelIndex rootIndex = m_dirModel.indexForItem(rootItem);
    QVERIFY(!rootIndex.isValid());

    KFileItem fileItem = m_dirModel.itemForIndex(m_fileIndex);
    QModelIndex fileIndex = m_dirModel.indexForItem(fileItem);
    QCOMPARE(fileIndex, m_fileIndex);

    KFileItem dirItem = m_dirModel.itemForIndex(m_dirIndex);
    QModelIndex dirIndex = m_dirModel.indexForItem(dirItem);
    QCOMPARE(dirIndex, m_dirIndex);

    KFileItem fileInDirItem = m_dirModel.itemForIndex(m_fileInDirIndex);
    QModelIndex fileInDirIndex = m_dirModel.indexForItem(fileInDirItem);
    QCOMPARE(fileInDirIndex, m_fileInDirIndex);

    KFileItem fileInSubdirItem = m_dirModel.itemForIndex(m_fileInSubdirIndex);
    QModelIndex fileInSubdirIndex = m_dirModel.indexForItem(fileInSubdirItem);
    QCOMPARE(fileInSubdirIndex, m_fileInSubdirIndex);
}

void KDirModelTest::testData()
{
    // First file
    QModelIndex idx1col1 = m_dirModel.index(m_fileIndex.row(), 1, QModelIndex());
    int size1 = m_dirModel.data(idx1col1, Qt::DisplayRole).toInt();
    QCOMPARE(size1, 11);

    KFileItem item = m_dirModel.data(m_fileIndex, KDirModel::FileItemRole).value<KFileItem>();
    KFileItem fileItem = m_dirModel.itemForIndex(m_fileIndex);
    QCOMPARE(item, fileItem);

    QCOMPARE(m_dirModel.data(m_fileIndex, KDirModel::ChildCountRole).toInt(), (int)KDirModel::ChildCountUnknown);



    // Second file
    QModelIndex idx2col0 = m_dirModel.index(m_secondFileIndex.row(), 0, QModelIndex());
    QString display2 = m_dirModel.data(idx2col0, Qt::DisplayRole).toString();
    QCOMPARE(display2, QString("toplevelfile_2"));

    // Subdir: check child count
    QCOMPARE(m_dirModel.data(m_dirIndex, KDirModel::ChildCountRole).toInt(), 3);

    // Subsubdir: check child count
    QCOMPARE(m_dirModel.data(m_fileInSubdirIndex.parent(), KDirModel::ChildCountRole).toInt(), 1);
}

void KDirModelTest::testReload()
{
    fillModel( true );
    testItemForIndex();
}

void KDirModelTest::testCreateFile()
{
    // TODO
    //createTestFile("toplevelfile_4");
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

    QSignalSpy spyDataChanged(&m_dirModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)));
    connect( &m_dirModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
             &m_eventLoop, SLOT(quit()) );

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

    disconnect( &m_dirModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                &m_eventLoop, SLOT(quit()) );
}

void KDirModelTest::testRenameFile()
{
    const QString file = m_tempDir->name() + "toplevelfile_2";
    const KUrl url(file);
    const QString newFile = m_tempDir->name() + "toplevelfile_2_renamed";
    const KUrl newUrl(newFile);

    QSignalSpy spyDataChanged(&m_dirModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)));
    connect( &m_dirModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
             &m_eventLoop, SLOT(quit()) );

    KIO::SimpleJob* job = KIO::rename(url, newUrl, KIO::HideProgressInfo);
    bool ok = job->exec();
    QVERIFY(ok);

    // Wait for the DBUS signal from KDirNotify, it's the one the triggers dataChanged
    enterLoop();

    // If we come here, then dataChanged() was emitted - all good.
    QCOMPARE(spyDataChanged.count(), 1);
    COMPARE_INDEXES(spyDataChanged[0][0].value<QModelIndex>(), m_secondFileIndex);
    QModelIndex receivedIndex = spyDataChanged[0][1].value<QModelIndex>();
    QCOMPARE(receivedIndex.row(), m_secondFileIndex.row()); // only compare row; column is count-1

    // check renaming happened
    QCOMPARE( m_dirModel.itemForIndex( m_secondFileIndex ).url().url(), newUrl.url() );

    // Put things back to normal
    job = KIO::rename(newUrl, url, KIO::HideProgressInfo);
    ok = job->exec();
    QVERIFY(ok);
    // Wait for the DBUS signal from KDirNotify, it's the one the triggers dataChanged
    enterLoop();
    QCOMPARE( m_dirModel.itemForIndex( m_secondFileIndex ).url().url(), url.url() );

    disconnect( &m_dirModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                &m_eventLoop, SLOT(quit()) );
}

void KDirModelTest::testExpandToUrl_data()
{
    QTest::addColumn<bool>("newdir"); // whether to re-create a new KTempDir completely, to avoid cached fileitems
    QTest::addColumn<QString>("expandToPath"); // relative path
    QTest::addColumn<QStringList>("expectedExpandSignals");

    QTest::newRow("the root, nothing to do")
        << false << QString() << QStringList();
    // When KDirLister was sync, subdir would already be known by the time we call expand
    // But now that listing is always async, we get a reliable result: expand(subdir) emitted.
    QTest::newRow("subdir")
        << false << "subdir" << (QStringList()<<"subdir");

    const QString subsubdir = "subdir/subsubdir";
    // Must list root, emit expand for subdir, list subdir, emit expand for subsubdir.
    QTest::newRow("subdir/subsubdir")
        << false << subsubdir << (QStringList()<<"subdir"<<subsubdir);

    // Must list root, emit expand for subdir, list subdir, emit expand for subsubdir, list subsubdir.
    QTest::newRow("subdir/subsubdir/testfile sync")
        << false << subsubdir + "/testfile" << (QStringList()<<"subdir"<<subsubdir);

    // Do a cold-cache test too, but nowadays it doesn't change anything anymore,
    // apart from testing different code paths inside KDirLister.
    QTest::newRow("subdir/subsubdir/testfile with reload")
        << true << subsubdir + "/testfile" << (QStringList()<<"subdir"<<subsubdir);
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
    KDirModel dirModelForExpand;
    KDirLister* dirListerForExpand = dirModelForExpand.dirLister();
    dirListerForExpand->openUrl(KUrl(path), KDirLister::NoFlags); // async
    connect(&dirModelForExpand, SIGNAL(expand(QModelIndex)),
            this, SLOT(slotExpand(QModelIndex)));
    connect(&dirModelForExpand, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(slotRowsInserted(QModelIndex,int,int)));
    m_rowsInsertedEmitted = false;
    m_expectedExpandSignals = expectedExpandSignals;
    m_dirModelForExpand = &dirModelForExpand;
    m_nextExpectedExpandSignals = 0;
    QSignalSpy spyExpand(&dirModelForExpand, SIGNAL(expand(QModelIndex)));
    dirModelForExpand.expandToUrl(KUrl(path + expandToPath));
    QCOMPARE(spyExpand.count(), 0); // nothing emitted yet
    if (expectedExpandSignals.isEmpty()) {
        QTest::qWait(20); // to make sure we process queued connection calls, otherwise spyExpand.count() is always 0 even if there's a bug...
        QCOMPARE(spyExpand.count(), 0);
    } else {
        enterLoop();
        QCOMPARE(spyExpand.count(), expectedExpandSignals.count());
        QVERIFY(m_rowsInsertedEmitted);
    }
    m_dirModelForExpand = 0;

    // recreateTestData was called -> fill again, for the next tests
    fillModel(false);
}

void KDirModelTest::slotExpand(const QModelIndex& index)
{
    QVERIFY(index.isValid());
    const QString path = m_tempDir->name();
    KFileItem item = m_dirModelForExpand->itemForIndex(index);
    QVERIFY(!item.isNull());
    QCOMPARE(item.url().path(), path + m_expectedExpandSignals[m_nextExpectedExpandSignals++]);
    // if rowsInserted wasn't emitted yet, then any proxy model would be unable to do anything with index at this point
    QVERIFY(m_rowsInsertedEmitted);
    if (m_nextExpectedExpandSignals == m_expectedExpandSignals.count())
        m_eventLoop.quit(); // done
}

void KDirModelTest::slotRowsInserted(const QModelIndex&, int, int)
{
    m_rowsInsertedEmitted = true;
}

void KDirModelTest::testFilter()
{
    QVERIFY(m_dirIndex.isValid());
    const int oldTopLevelRowCount = m_dirModel.rowCount();
    const int oldSubdirRowCount = m_dirModel.rowCount(m_dirIndex);
    QSignalSpy spyRowsRemoved(&m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    m_dirModel.dirLister()->setNameFilter("toplevel*");
    QCOMPARE(m_dirModel.rowCount(), oldTopLevelRowCount); // no change yet
    QCOMPARE(m_dirModel.rowCount(m_dirIndex), oldSubdirRowCount); // no change yet
    m_dirModel.dirLister()->emitChanges();

    const int expectedTopLevelRowCount = 4; // 3 toplevel* files, one subdir
    const int expectedSubdirRowCount = 1; // the files get filtered out, the subdir remains

    //while (m_dirModel.rowCount() > expectedTopLevelRowCount) {
    //    QTest::qWait(20);
    //    kDebug() << "rowCount=" << m_dirModel.rowCount();
    //}
    QCOMPARE(m_dirModel.rowCount(), expectedTopLevelRowCount);

    //while (m_dirModel.rowCount(m_dirIndex) > expectedSubdirRowCount) {
    //    QTest::qWait(20);
    //    kDebug() << "rowCount in subdir=" << m_dirModel.rowCount(m_dirIndex);
    //}
    QCOMPARE(m_dirModel.rowCount(m_dirIndex), expectedSubdirRowCount);

    // Reset the filter
    kDebug() << "reset to no filter";
    m_dirModel.dirLister()->setNameFilter(QString());
    m_dirModel.dirLister()->emitChanges();

    QCOMPARE(m_dirModel.rowCount(), oldTopLevelRowCount);
    QCOMPARE(m_dirModel.rowCount(m_dirIndex), oldSubdirRowCount);

    // The order of things changed because of filtering.
    // Fill again, so that m_fileIndex etc. are correct again.
    fillModel(true);
}

void KDirModelTest::testUrlWithRef() // #171117
{
    const QString path = m_tempDir->name();
    KDirLister* dirLister = m_dirModel.dirLister();
    KUrl url(path);
    url.setRef("ref");
    QVERIFY(url.url().endsWith("#ref"));
    dirLister->openUrl(url, KDirLister::NoFlags);
    connect(dirLister, SIGNAL(completed()), this, SLOT(slotListingCompleted()));
    enterLoop();

    QCOMPARE(dirLister->url().url(), url.url(KUrl::RemoveTrailingSlash));
    m_dirIndex = QModelIndex();
    m_fileIndex = QModelIndex();
    m_secondFileIndex = QModelIndex();
    for (int row = 0; row < m_topLevelFileNames.count() + 1 /*subdir*/; ++row) {
        QModelIndex idx = m_dirModel.index(row, 0, QModelIndex());
        KFileItem item = m_dirModel.itemForIndex(idx);
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
}

void KDirModelTest::testDeleteFile()
{
    QVERIFY(m_fileIndex.isValid());
    const int oldTopLevelRowCount = m_dirModel.rowCount();
    const QString file = m_tempDir->name() + "toplevelfile_1";
    const KUrl url(file);

    QSignalSpy spyRowsRemoved(&m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    connect( &m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
             &m_eventLoop, SLOT(quit()) );

    KIO::DeleteJob* job = KIO::del(url, KIO::HideProgressInfo);
    bool ok = job->exec();
    QVERIFY(ok);

    // Wait for the DBUS signal from KDirNotify, it's the one the triggers rowsRemoved
    enterLoop();

    // If we come here, then rowsRemoved() was emitted - all good.
    const int topLevelRowCount = m_dirModel.rowCount();
    QCOMPARE(topLevelRowCount, oldTopLevelRowCount - 1); // one less than before
    QCOMPARE(spyRowsRemoved.count(), 1);
    QCOMPARE(spyRowsRemoved[0][1].toInt(), m_fileIndex.row());
    QCOMPARE(spyRowsRemoved[0][2].toInt(), m_fileIndex.row());
    disconnect( &m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                &m_eventLoop, SLOT(quit()) );

    // Recreate the file, for consistency in the next tests
    createTestFile(file);
    fillModel(true);
}

void KDirModelTest::testDeleteFiles()
{
    const int oldTopLevelRowCount = m_dirModel.rowCount();
    const QString file = m_tempDir->name() + "toplevelfile_";
    KUrl::List urls;
    urls << KUrl(file + '1') << KUrl(file + '2') << KUrl(file + '3');

    QSignalSpy spyRowsRemoved(&m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    KIO::DeleteJob* job = KIO::del(urls, KIO::HideProgressInfo);
    bool ok = job->exec();
    QVERIFY(ok);

    int numRowsRemoved = 0;
    while (numRowsRemoved < 3) {

        QTest::qWait(20);

        numRowsRemoved = 0;
        for (int sigNum = 0; sigNum < spyRowsRemoved.count(); ++sigNum)
            numRowsRemoved += spyRowsRemoved[sigNum][2].toInt() - spyRowsRemoved[sigNum][1].toInt() + 1;
        kDebug() << "numRowsRemoved=" << numRowsRemoved;
    }

    const int topLevelRowCount = m_dirModel.rowCount();
    QCOMPARE(topLevelRowCount, oldTopLevelRowCount - 3); // three less than before
}
