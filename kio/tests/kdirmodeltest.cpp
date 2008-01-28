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

void KDirModelTest::initTestCase()
{
    s_referenceTimeStamp = QDateTime::currentDateTime().addSecs( -30 ); // 30 seconds ago

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
    const QString path = m_tempDir.name();
    m_topLevelFileNames << "toplevelfile_1"
                        << "toplevelfile_2"
                        << "toplevelfile_3"
                        << "specialchars%:";
    foreach(QString f, m_topLevelFileNames) {
        createTestFile(path+f);
    }
    createTestDirectory(path+"subdir");
    createTestDirectory(path+"subdir/subsubdir", NoSymlink);

    fillModel( false );
}

void KDirModelTest::fillModel( bool reload )
{
    const QString path = m_tempDir.name();
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
    QCOMPARE(m_dirModel.rowCount(m_dirIndex), 3);
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
    QCOMPARE(specialFileName, QString("specialchars%:"));

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
    QCOMPARE(fileItem.url().path(), m_tempDir.name() + "toplevelfile_1");

    KFileItem dirItem = m_dirModel.itemForIndex(m_dirIndex);
    QVERIFY(!dirItem.isNull());
    QCOMPARE(dirItem.name(), QString("subdir"));
    QVERIFY(dirItem.isDir());
    QCOMPARE(dirItem.url().path(), m_tempDir.name() + "subdir");

    KFileItem fileInDirItem = m_dirModel.itemForIndex(m_fileInDirIndex);
    QVERIFY(!fileInDirItem.isNull());
    QCOMPARE(fileInDirItem.name(), QString("testfile"));
    QVERIFY(!fileInDirItem.isDir());
    QCOMPARE(fileInDirItem.url().path(), m_tempDir.name() + "subdir/testfile");

    KFileItem fileInSubdirItem = m_dirModel.itemForIndex(m_fileInSubdirIndex);
    QVERIFY(!fileInSubdirItem.isNull());
    QCOMPARE(fileInSubdirItem.name(), QString("testfile"));
    QVERIFY(!fileInSubdirItem.isDir());
    QCOMPARE(fileInSubdirItem.url().path(), m_tempDir.name() + "subdir/subsubdir/testfile");
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
    const QString file = m_tempDir.name() + "toplevelfile_2";
    const KUrl url(file);

    qRegisterMetaType<QModelIndex>("QModelIndex"); // beats me why Qt doesn't do that
    QSignalSpy spyDataChanged(&m_dirModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)));
    connect( &m_dirModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
             &m_eventLoop, SLOT(quit()) );

    // "Touch" the file
    setTimeStamp(file, s_referenceTimeStamp.addSecs(20) );

    // In stat mode, kdirwatch doesn't notice file changes; we need to trigger it
    // by creating a file.
    //createTestFile(m_tempDir.name() + "toplevelfile_5");
    KDirWatch::self()->setDirty(m_tempDir.name());

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
    const QString file = m_tempDir.name() + "toplevelfile_2";
    const KUrl url(file);
    const QString newFile = m_tempDir.name() + "toplevelfile_2_renamed";
    const KUrl newUrl(newFile);

    qRegisterMetaType<QModelIndex>("QModelIndex"); // beats me why Qt doesn't do that
    QSignalSpy spyDataChanged(&m_dirModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)));
    connect( &m_dirModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
             &m_eventLoop, SLOT(quit()) );

    KIO::SimpleJob* job = KIO::rename(url, newUrl, KIO::HideProgressInfo);
    bool ok = job->exec();
    QVERIFY(ok);

    // Wait for the DBUS signal from KDirNotify, it's the one the triggers rowsRemoved
    enterLoop();

    // If we come here, then dataChanged() was emitted - all good.
    QCOMPARE(spyDataChanged.count(), 1);
    COMPARE_INDEXES(spyDataChanged[0][0].value<QModelIndex>(), m_secondFileIndex);
    QModelIndex receivedIndex = spyDataChanged[0][1].value<QModelIndex>();
    QCOMPARE(receivedIndex.row(), m_secondFileIndex.row()); // only compare row; column is count-1

    // check renaming happened
    QCOMPARE( m_dirModel.itemForIndex( m_secondFileIndex ).url().url(), newUrl.url() );

    disconnect( &m_dirModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                &m_eventLoop, SLOT(quit()) );
}

void KDirModelTest::testExpandToUrl_data()
{
    QTest::addColumn<QString>("expandToPath");
    QTest::addColumn<QStringList>("expectedExpandSignals");

    const QString path = m_tempDir.name();
    QTest::newRow("the root, nothing to do")
        << path << QStringList();
    QTest::newRow("already known child, nothing to do")
        << path+"subdir" << QStringList();
    const QString subsubdir = path+"subdir/subsubdir";
    QStringList sigs; sigs << subsubdir;
    // must list subdir and then expand is emitted
    QTest::newRow("subdir/subsubdir")
        << subsubdir << sigs;
    // must list subdir, emit expand for subsubdir, and then list subsubdir
    QTest::newRow("subdir/subsubdir/testfile")
        << subsubdir + "/testfile" << sigs;
    // TODO: we need an async test too (to emit expand twice)
}

void KDirModelTest::testExpandToUrl()
{
    const QString path = m_tempDir.name();
    QFETCH(QString, expandToPath);
    QFETCH(QStringList, expectedExpandSignals);

    KDirModel dirModelForExpand;
    KDirLister* dirListerForExpand = dirModelForExpand.dirLister();
    dirListerForExpand->openUrl(KUrl(path), KDirLister::NoFlags); // it gets them from the cache, so this is sync
    connect(&dirModelForExpand, SIGNAL(expand(QModelIndex)),
            this, SLOT(slotExpand(QModelIndex)));
    connect(&dirModelForExpand, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(slotRowsInserted(QModelIndex,int,int)));
    m_rowsInsertedEmitted = false;
    m_nextExpectedExpandPath = expectedExpandSignals.isEmpty() ? QString() : expectedExpandSignals.first();
    QSignalSpy spyExpand(&dirModelForExpand, SIGNAL(expand(QModelIndex)));
    dirModelForExpand.expandToUrl(KUrl(expandToPath));
    if (expectedExpandSignals.isEmpty()) {
        QCOMPARE(spyExpand.count(), 0);
    } else {
        enterLoop();
        QCOMPARE(spyExpand.count(), 1);
        QVERIFY(m_rowsInsertedEmitted);
    }
}

// Must be done last because it changes the other indexes
void KDirModelTest::testDeleteFile()
{
    const int oldTopLevelRowCount = m_dirModel.rowCount();
    const QString file = m_tempDir.name() + "toplevelfile_1";
    const KUrl url(file);

    qRegisterMetaType<QModelIndex>("QModelIndex"); // beats me why Qt doesn't do that
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
    disconnect( &m_dirModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                &m_eventLoop, SLOT(quit()) );
}

void KDirModelTest::slotExpand(const QModelIndex& index)
{
    QVERIFY(index.isValid());
    KFileItem item = m_dirModel.itemForIndex(index);
    QVERIFY(!item.isNull());
    QCOMPARE(m_nextExpectedExpandPath, item.url().path());
    // if rowsInserted wasn't emitted yet, then any proxy model would be unable to do anything with index at this point
    QVERIFY(m_rowsInsertedEmitted);
}

void KDirModelTest::slotRowsInserted(const QModelIndex&, int, int)
{
    m_rowsInsertedEmitted = true;
}
