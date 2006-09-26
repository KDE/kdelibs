/*
 *  Copyright (C) 2006 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kdirmodeltest.h"
#include "kdirmodeltest.moc"
#include <kdirmodel.h>

#include <qtest_kde.h>
#include <kdebug.h>
#ifdef Q_OS_UNIX
#include <utime.h>
#include <kdirlister.h>
#endif

QTEST_KDEMAIN( KDirModelTest, NoGUI )

QDateTime s_referenceTimeStamp;

static void setTimeStamp( const QString& path )
{
#ifdef Q_OS_UNIX
    // Put timestamp in the past so that we can check that the listing is correct
    struct utimbuf utbuf;
    utbuf.actime = s_referenceTimeStamp.toTime_t();
    utbuf.modtime = utbuf.actime;
    utime( QFile::encodeName( path ), &utbuf );
    qDebug( "Time changed for %s", qPrintable( path ) );
#endif
}

static void createTestFile( const QString& path )
{
    QFile f( path );
    if ( !f.open( QIODevice::WriteOnly ) )
        kFatal() << "Can't create " << path << endl;
    f.write( QByteArray( "Hello world" ) );
    f.close();
    setTimeStamp( path );
}

static void createTestDirectory( const QString& path )
{
    QDir dir;
    bool ok = dir.mkdir( path );
    if ( !ok && !dir.exists() )
        kFatal() << "couldn't create " << path << endl;
    createTestFile( path + "/testfile" );
    //createTestSymlink( path + "/testlink" );
    //QVERIFY( QFileInfo( path + "/testlink" ).isSymLink() );
    setTimeStamp( path );
}

void KDirModelTest::initTestCase()
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
    m_tempDir.setAutoDelete(true);
    const QString path = m_tempDir.name();
    createTestFile(path+"toplevelfile_1");
    createTestFile(path+"toplevelfile_2");
    createTestFile(path+"toplevelfile_3");
    createTestDirectory(path+"subdir");
    createTestDirectory(path+"subdir/subsubdir");

    // Now fill the model
    KDirLister* dirLister = m_dirModel.dirLister();
    dirLister->openUrl(KUrl(path));
    connect(dirLister, SIGNAL(completed()), this, SLOT(slotListingCompleted()));
    enterLoop();

    // Create the indexes once and for all
    // Index of the first file
    m_fileIndex = m_dirModel.index(0, 0, QModelIndex());
    // Index of a directory
    m_dirIndex = m_dirModel.index(3, 0, QModelIndex());
    QVERIFY(m_dirIndex.isValid());

    // Now list subdir/
    QVERIFY(m_dirModel.canFetchMore(m_dirIndex));
    m_dirModel.fetchMore(m_dirIndex);
    enterLoop();

    // Index of a file inside a directory (subdir/testfile)
    m_fileInDirIndex = m_dirModel.index(0, 0, m_dirIndex);
    QModelIndex subdirIndex = m_dirModel.index(1, 0, m_dirIndex);

    // List subdir/subsubdir
    QVERIFY(m_dirModel.canFetchMore(subdirIndex));
    m_dirModel.fetchMore(subdirIndex);
    enterLoop();

    // Index of ... well, subdir/subsubdir/testfile
    m_fileInSubdirIndex = m_dirModel.index(0, 0, subdirIndex);
}

void KDirModelTest::enterLoop()
{
    QEventLoop eventLoop;
    connect(this, SIGNAL(exitLoop()),
            &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
}

void KDirModelTest::slotListingCompleted()
{
    emit exitLoop();
}

void KDirModelTest::testRowCount()
{
    const int topLevelRowCount = m_dirModel.rowCount();
    QCOMPARE(topLevelRowCount, 4);
    QCOMPARE(m_dirModel.rowCount(m_dirIndex), 2);
}

void KDirModelTest::testIndex()
{
    QVERIFY(m_dirModel.hasChildren());

    // Index of the first file
    QVERIFY(m_fileIndex.isValid());
    QCOMPARE(m_fileIndex.model(), &m_dirModel);
    QCOMPARE(m_fileIndex.row(), 0);
    QCOMPARE(m_fileIndex.column(), 0);
    QVERIFY(!m_fileIndex.parent().isValid());
    QVERIFY(!m_dirModel.hasChildren(m_fileIndex));

    // Index of a directory
    QVERIFY(m_dirIndex.isValid());
    QCOMPARE(m_dirIndex.model(), &m_dirModel);
    QCOMPARE(m_dirIndex.row(), 3);
    QCOMPARE(m_dirIndex.column(), 0);
    QVERIFY(!m_dirIndex.parent().isValid());
    QVERIFY(m_dirModel.hasChildren(m_dirIndex));

    // Index of a file inside a directory (subdir/testfile)
    QVERIFY(m_fileInDirIndex.isValid());
    QCOMPARE(m_fileInDirIndex.model(), &m_dirModel);
    QCOMPARE(m_fileInDirIndex.row(), 0);
    QCOMPARE(m_fileInDirIndex.column(), 0);
    QVERIFY(m_fileInDirIndex.parent() == m_dirIndex);
    QVERIFY(!m_dirModel.hasChildren(m_fileInDirIndex));

    // Index of subdir/subsubdir/testfile
    QVERIFY(m_fileInSubdirIndex.isValid());
    QCOMPARE(m_fileInSubdirIndex.model(), &m_dirModel);
    QCOMPARE(m_fileInSubdirIndex.row(), 0);
    QCOMPARE(m_fileInSubdirIndex.column(), 0);
    QVERIFY(m_fileInSubdirIndex.parent().parent() == m_dirIndex);
    QVERIFY(!m_dirModel.hasChildren(m_fileInSubdirIndex));
}

void KDirModelTest::testNames()
{
    QString fileName = m_dirModel.data(m_fileIndex, Qt::DisplayRole).toString();
    QCOMPARE(fileName, QString("toplevelfile_1"));

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
    KFileItem* rootItem = m_dirModel.itemForIndex(QModelIndex());
    QVERIFY(rootItem != 0);
    QCOMPARE(rootItem->name(), QString("."));

    KFileItem* fileItem = m_dirModel.itemForIndex(m_fileIndex);
    QVERIFY(fileItem != 0);
    QCOMPARE(fileItem->name(), QString("toplevelfile_1"));
    QVERIFY(!fileItem->isDir());
    QCOMPARE(fileItem->url().path(), m_tempDir.name() + "toplevelfile_1");

    KFileItem* dirItem = m_dirModel.itemForIndex(m_dirIndex);
    QVERIFY(dirItem != 0);
    QCOMPARE(dirItem->name(), QString("subdir"));
    QVERIFY(dirItem->isDir());
    QCOMPARE(dirItem->url().path(), m_tempDir.name() + "subdir");

    KFileItem* fileInDirItem = m_dirModel.itemForIndex(m_fileInDirIndex);
    QVERIFY(fileInDirItem != 0);
    QCOMPARE(fileInDirItem->name(), QString("testfile"));
    QVERIFY(!fileInDirItem->isDir());
    QCOMPARE(fileInDirItem->url().path(), m_tempDir.name() + "subdir/testfile");

    KFileItem* fileInSubdirItem = m_dirModel.itemForIndex(m_fileInSubdirIndex);
    QVERIFY(fileInSubdirItem != 0);
    QCOMPARE(fileInSubdirItem->name(), QString("testfile"));
    QVERIFY(!fileInSubdirItem->isDir());
    QCOMPARE(fileInSubdirItem->url().path(), m_tempDir.name() + "subdir/subsubdir/testfile");
}

void KDirModelTest::testData()
{
    // Index of the first file
    QModelIndex idx1col1 = m_dirModel.index(0, 1, QModelIndex());
    int size1 = m_dirModel.data(idx1col1, Qt::DisplayRole).toInt();
    QCOMPARE(size1, 11);

    //QModelIndex idx1col2 = m_dirModel.index(0, 2, QModelIndex());


    // Index of the second file
    QModelIndex idx2col0 = m_dirModel.index(1, 0, QModelIndex());
    QString display2 = m_dirModel.data(idx2col0, Qt::DisplayRole).toString();
    QCOMPARE(display2, QString("toplevelfile_2"));
}
