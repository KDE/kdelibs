/* This file is part of the KDE libraries

    Copyright (c) 2009 David Faure <faure@kde.org>

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

#include <kconfiggroup.h>
#include <QDir>
#include <kdebug.h>
#include <ktempdir.h>
#include <qtest_kde.h>
#include <qtestevent.h>
#include <kdirwatch.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <kde_file.h>

// Debugging notes: to see which inotify signals are emitted, either set s_verboseDebug=true
// at the top of kdirwatch.cpp, or use the command-line tool "inotifywait -m /path"

// Note that kdirlistertest and kdirmodeltest also exercise KDirWatch quite a lot.

class KDirWatch_UnitTest : public QObject
{
    Q_OBJECT
public:
    KDirWatch_UnitTest()
    {
        m_path = m_tempDir.name();
        Q_ASSERT(m_path.endsWith('/'));

        // Speed up the test by making the kdirwatch timer (to compress changes) faster
        KConfigGroup config(KGlobal::config(), "DirWatch");
        config.writeEntry("PollInterval", 50);
    }

private Q_SLOTS: // test methods
    void initTestCase() {
        // By creating the files upfront, we save waiting a full second for an mtime change
        createFile(m_path + "ExistingFile");
        createFile(m_path + "TestFile");
        createFile(m_path + "nested_0");
        createFile(m_path + "nested_1");
    }
    void touchOneFile();
    void touch1000Files();
    void watchAndModifyOneFile();
    void removeAndReAdd();
    void watchNonExistent();
    void testDelete();
    void testDeleteAndRecreate();
    void testMoveTo();
    void nestedEventLoop();
    void testHardlinkChange();

protected Q_SLOTS: // internal slots
    void nestedEventLoopSlot();

private:
    void waitUntilMTimeChange(const QString& path);
    QList<QVariantList> waitForDirtySignal(KDirWatch& watch, int expected);
    QList<QVariantList> waitForCreatedSignal(KDirWatch& watch, int expected);
    QList<QVariantList> waitForDeletedSignal(KDirWatch& watch, int expected);
    bool waitForOneSignal(KDirWatch& watch, const char* sig, const QString& path);
    void createFile(const QString& path);
    QString createFile(int num);
    void appendToFile(const QString& path);
    void appendToFile(int num);

    KTempDir m_tempDir;
    QString m_path;
};

QTEST_KDEMAIN_CORE(KDirWatch_UnitTest)

// Just to make the inotify packets bigger
static const char s_filePrefix[] = "This_is_a_test_file_";

static const int s_maxTries = 50;

// helper method: create a file
void KDirWatch_UnitTest::createFile(const QString& path)
{
    QFile file(path);
    bool ok = file.open(QIODevice::WriteOnly);
    Q_ASSERT(ok);
    file.write(QByteArray("foo"));
    file.close();
    //kDebug() << path;
}

// helper method: create a file (identifier by number)
QString KDirWatch_UnitTest::createFile(int num)
{
    const QString fileName = s_filePrefix + QString::number(num);
    createFile(m_path + fileName);
    return m_path + fileName;
}

static QByteArray printableTime(time_t mtime)
{
    struct tm* tmp = localtime(&mtime);
    char outstr[200];
    if (strftime(outstr, sizeof(outstr), "%T", tmp) == 0)
        return "ERROR calling strftime!";
    return outstr;
}

void KDirWatch_UnitTest::waitUntilMTimeChange(const QString& path)
{
   // Wait until the current second is more than the file's mtime
    // otherwise this change will go unnoticed
    KDE_struct_stat stat_buf;
    QCOMPARE(KDE::stat(path, &stat_buf), 0);
    int totalWait = 0;
    struct timeval now_tv;
    time_t ctime;

    Q_FOREVER {
        gettimeofday(&now_tv, NULL);
        // The mtime only has a granularity of a second, that's the whole issue;
        // We can't just QTest::qWait(now_tv.tv_sec - stat_buf.st_ctime), that would
        // be a full second every time.
#ifdef Q_OS_WIN
        // ctime is the 'creation time' on windows - use mtime instead
        ctime = stat_buf.st_mtime;
#else
        ctime = stat_buf.st_ctime;
#endif

        if (now_tv.tv_sec == ctime) {
            totalWait += 50;
            QTest::qWait(50);
        } else {
            Q_ASSERT(now_tv.tv_sec > ctime); // can't go back in time ;)
            break;
        }
    }
    //if (totalWait > 0)
        kDebug() << "File has ctime" << printableTime(stat_buf.st_ctime) << ", so I waited" << totalWait << "ms, now is" << printableTime(now_tv.tv_sec);
}

// helper method: modifies a file
void KDirWatch_UnitTest::appendToFile(const QString& path)
{
    QVERIFY(QFile::exists(path));
    waitUntilMTimeChange(path);
    //const QString directory = QDir::cleanPath(path+"/..");
    //waitUntilMTimeChange(directory);

    //KDE_struct_stat stat_buf;
    //QCOMPARE(KDE::stat(path, &stat_buf), 0);
    //kDebug() << "After append: file ctime=" << printableTime(stat_buf.st_ctime);
    //QCOMPARE(KDE::stat(directory, &stat_buf), 0);
    //kDebug() << "After append: directory mtime=" << printableTime(stat_buf.st_ctime);

    QFile file(path);
    QVERIFY(file.open(QIODevice::Append | QIODevice::WriteOnly));
    file.write(QByteArray("foobar"));
    file.close();
}

// helper method: modifies a file (identified by number)
void KDirWatch_UnitTest::appendToFile(int num)
{
    const QString fileName = s_filePrefix + QString::number(num);
    appendToFile(m_path + fileName);
}

// helper method
QList<QVariantList> KDirWatch_UnitTest::waitForDirtySignal(KDirWatch& watch, int expected)
{
    QSignalSpy spyDirty(&watch, SIGNAL(dirty(QString)));
    int numTries = 0;
    // Give time for KDirWatch to notify us
    while (spyDirty.count() < expected) {
        if (++numTries > s_maxTries) {
            kWarning() << "Timeout waiting for KDirWatch. Got" << spyDirty.count() << "dirty() signals, expected" << expected;
            return spyDirty;
        }
        QTest::qWait(50);
    }
    return spyDirty;
}

bool KDirWatch_UnitTest::waitForOneSignal(KDirWatch& watch, const char* sig, const QString& path)
{
    QSignalSpy spyDirty(&watch, sig);
    int numTries = 0;
    // Give time for KDirWatch to notify us
    while (spyDirty.isEmpty()) {
        if (++numTries > s_maxTries) {
            kWarning() << "Timeout waiting for KDirWatch signal" << QByteArray(sig).mid(1) << "(" << path << ")";
            return false;
        }
        QTest::qWait(50);
    }
    return true;
}

QList<QVariantList> KDirWatch_UnitTest::waitForCreatedSignal(KDirWatch& watch, int expected)
{
    QSignalSpy spyCreated(&watch, SIGNAL(created(QString)));
    int numTries = 0;
    // Give time for KDirWatch to notify us
    while (spyCreated.count() < expected) {
        if (++numTries > s_maxTries) {
            kWarning() << "Timeout waiting for KDirWatch. Got" << spyCreated.count() << "created() signals, expected" << expected;
            return spyCreated;
        }
        QTest::qWait(50);
    }
    return spyCreated;
}

QList<QVariantList> KDirWatch_UnitTest::waitForDeletedSignal(KDirWatch& watch, int expected)
{
    QSignalSpy spyDeleted(&watch, SIGNAL(created(QString)));
    int numTries = 0;
    // Give time for KDirWatch to notify us
    while (spyDeleted.count() < expected) {
        if (++numTries > s_maxTries) {
            kWarning() << "Timeout waiting for KDirWatch. Got" << spyDeleted.count() << "deleted() signals, expected" << expected;
            return spyDeleted;
        }
        QTest::qWait(50);
    }
    return spyDeleted;
}

void KDirWatch_UnitTest::touchOneFile() // watch a dir, create a file in it
{
    KDirWatch watch;
    watch.addDir(m_path);
    watch.startScan();

    waitUntilMTimeChange(m_path);

    const QString file0 = createFile(0);
    QVERIFY(waitForOneSignal(watch, SIGNAL(dirty(QString)), file0));
}

void KDirWatch_UnitTest::touch1000Files()
{
    KDirWatch watch;
    watch.addDir(m_path);
    watch.startScan();

    const int fileCount = 1000;
    for (int i = 0; i < fileCount; ++i) {
        createFile(i);
    }

    QList<QVariantList> spy = waitForDirtySignal(watch, fileCount);
    QVERIFY(spy.count() >= fileCount);
    qDebug() << spy.count();
}

void KDirWatch_UnitTest::watchAndModifyOneFile() // watch a specific file, and modify it
{
    KDirWatch watch;
    const QString existingFile = m_path + "ExistingFile";
    watch.addFile(existingFile);
    watch.startScan();
    appendToFile(existingFile);
    QVERIFY(waitForOneSignal(watch, SIGNAL(dirty(QString)), existingFile));
}

void KDirWatch_UnitTest::removeAndReAdd()
{
    KDirWatch watch;
    watch.addDir(m_path);
    watch.startScan();
    createFile(0);
    QVERIFY(waitForOneSignal(watch, SIGNAL(dirty(QString)), m_path + s_filePrefix + "0"));

    // Just like KDirLister does: remove the watch, then re-add it.
    watch.removeDir(m_path);
    watch.addDir(m_path);
    const QString file1 = createFile(1);
    QVERIFY(waitForOneSignal(watch, SIGNAL(dirty(QString)), file1));
}

void KDirWatch_UnitTest::watchNonExistent()
{
    KDirWatch watch;
    // Watch "subdir", that doesn't exist yet
    const QString subdir = m_path + "subdir";
    QVERIFY(!QFile::exists(subdir));
    watch.addDir(subdir);
    watch.startScan();

    // Now create it, KDirWatch should emit created()
    QDir().mkdir(subdir);

    QVERIFY(waitForOneSignal(watch, SIGNAL(created(QString)), subdir));

    // Play with addDir/removeDir, just for fun
    watch.addDir(subdir);
    watch.removeDir(subdir);
    watch.addDir(subdir);

    // Now watch files that don't exist yet
    const QString file = subdir + "/0";
    watch.addFile(file); // doesn't exist yet
    const QString file1 = subdir + "/1";
    watch.addFile(file1); // doesn't exist yet
    watch.removeFile(file1); // forget it again

    // Now create it, KDirWatch should emit created
    createFile(file);
    QVERIFY(waitForOneSignal(watch, SIGNAL(created(QString)), file));

    appendToFile(file);
    QVERIFY(waitForOneSignal(watch, SIGNAL(dirty(QString)), file));

    // Create the file after all; we're not watching for it, but the dir will emit dirty
    createFile(file1);
    QVERIFY(waitForOneSignal(watch, SIGNAL(dirty(QString)), subdir));
}

void KDirWatch_UnitTest::testDelete()
{
    const QString file1 = m_path + "del";
    if (!QFile::exists(file1))
        createFile(file1);
    waitUntilMTimeChange(file1);

    // Watch the file, then delete it, KDirWatch will emit deleted (and possibly dirty for the dir, if mtime changed)
    KDirWatch watch;
    watch.addFile(file1);

    KDirWatch::statistics();

    QSignalSpy spyDirty(&watch, SIGNAL(dirty(QString)));
    QFile::remove(file1);
    QVERIFY(waitForOneSignal(watch, SIGNAL(deleted(QString)), file1));
    QTest::qWait(40); // just in case delayed processing would emit it
    QCOMPARE(spyDirty.count(), 0);
}

void KDirWatch_UnitTest::testDeleteAndRecreate()
{
    const QString subdir = m_path + "subdir";
    QDir().mkdir(subdir);
    const QString file1 = subdir + "/1";
    if (!QFile::exists(file1))
        createFile(file1);
    waitUntilMTimeChange(file1);

    // Watch the file, then delete it, KDirWatch will emit deleted (and possibly dirty for the dir, if mtime changed)
    KDirWatch watch;
    watch.addFile(file1);

    QFile::remove(file1);
    // And recreate immediately, to try and fool KDirWatch with unchanged ctime/mtime ;)
    createFile(file1);
    //QVERIFY(waitForOneSignal(watch, SIGNAL(deleted(QString)), file1));
    //QVERIFY(waitForOneSignal(watch, SIGNAL(dirty(QString)), subdir));
    QVERIFY(waitForOneSignal(watch, SIGNAL(created(QString)), file1));

    waitUntilMTimeChange(file1);
    //KDirWatch::statistics();

    appendToFile(file1);
    QVERIFY(waitForOneSignal(watch, SIGNAL(dirty(QString)), file1));
}

void KDirWatch_UnitTest::testMoveTo()
{
    // This reproduces the famous digikam crash, #222974
    // A watched file was being rewritten (overwritten by ksavefile),
    // which gives inotify notifications "moved_to" followed by "delete_self"
    //  -> inotify bug, email sent to the author.
    //
    // What happened  then was that the delayed slotRescan
    // would adjust things, making it status==Normal but the entry was
    // listed as a "non-existent sub-entry" for the parent directory.
    // That's inconsistent, and after removeFile() a dangling sub-entry would be left.

    // Initial data: creating file subdir/1
    const QString file1 = m_path + "moveTo";
    createFile(file1);

    KDirWatch watch;
    watch.addDir(m_path);
    watch.addFile(file1);
    watch.startScan();

    // Atomic rename of "temp" to "file1", much like KAutoSave would do when saving file1 again
    const QString filetemp = m_path + "temp";
    createFile(filetemp);
    QVERIFY(KDE::rename(filetemp, file1) == 0); // overwrite file1 with the tempfile

    QVERIFY(waitForOneSignal(watch, SIGNAL(created(QString)), file1));

    // make sure we're still watching it
    // ## this doesn't work, change is not detected, must be related to the inotify bug on overwrite
    //appendToFile(file1);
    //QVERIFY(waitForOneSignal(watch, SIGNAL(dirty(QString)), file1));

    //kDebug() << "after created";
    //KDirWatch::statistics();
    watch.removeFile(file1); // now we remove it
    //kDebug() << "after removeFile";
    //KDirWatch::statistics();

    // Just touch another file to trigger a findSubEntry - this where the crash happened
    waitUntilMTimeChange(m_path);
    createFile(filetemp);
    QVERIFY(waitForOneSignal(watch, SIGNAL(dirty(QString)), m_path));
}

void KDirWatch_UnitTest::nestedEventLoop() // #220153: watch two files, and modify 2nd while in slot for 1st
{
    KDirWatch watch;

    const QString file0 = m_path + "nested_0";
    watch.addFile(file0);
    const QString file1 = m_path + "nested_1";
    watch.addFile(file1);
    watch.startScan();

    appendToFile(file0);

    // use own spy, to connect it before nestedEventLoopSlot, otherwise it reverses order
    QSignalSpy spyDirty(&watch, SIGNAL(dirty(QString)));
    connect(&watch, SIGNAL(dirty(QString)), this, SLOT(nestedEventLoopSlot()));
    waitForDirtySignal(watch, 1);
    QVERIFY(spyDirty.count() >= 2);
    QCOMPARE(spyDirty[0][0].toString(), file0);
    QCOMPARE(spyDirty[spyDirty.count()-1][0].toString(), file1);
}

void KDirWatch_UnitTest::nestedEventLoopSlot()
{
    const KDirWatch* const_watch = qobject_cast<const KDirWatch *>(sender());
    KDirWatch* watch = const_cast<KDirWatch *>(const_watch);
    // let's not come in this slot again
    disconnect(watch, SIGNAL(dirty(QString)), this, SLOT(nestedEventLoopSlot()));

    const QString file1 = m_path + "nested_1";
    appendToFile(file1);
    //kDebug() << "now waiting for signal";
    // The nested event processing here was from a messagebox in #220153
    QList<QVariantList> spy = waitForDirtySignal(*watch, 1);
    QVERIFY(spy.count() >= 1);
    QCOMPARE(spy[spy.count()-1][0].toString(), file1);
    //kDebug() << "done";

    // Now the user pressed reload...
    const QString file0 = m_path + "nested_0";
    watch->removeFile(file0);
    watch->addFile(file0);
}

void KDirWatch_UnitTest::testHardlinkChange()
{
#ifdef Q_OS_UNIX

    // The unittest for the "detecting hardlink change to /etc/localtime" problem
    // described on kde-core-devel (2009-07-03).
    // It shows that watching a specific file doesn't inform us that the file is
    // being recreated. Better watch the directory, for that.

    const QString existingFile = m_path + "ExistingFile";
    KDirWatch watch;
    watch.addFile(existingFile);
    watch.startScan();

    //waitUntilMTimeChange(existingFile);
    //waitUntilMTimeChange(m_path);

    QFile::remove(existingFile);
    const QString testFile = m_path + "TestFile";
    ::link(QFile::encodeName(testFile), QFile::encodeName(existingFile)); // make ExistingFile "point" to TestFile
    QVERIFY(QFile::exists(existingFile));
    //QVERIFY(waitForOneSignal(watch, SIGNAL(deleted(QString)), existingFile));
    QVERIFY(waitForOneSignal(watch, SIGNAL(created(QString)), existingFile));

    //KDirWatch::statistics();

    appendToFile(existingFile);
    QVERIFY(waitForOneSignal(watch, SIGNAL(dirty(QString)), existingFile));
#else
    QSKIP("Unix-specific", SkipAll);
#endif
}

#include "kdirwatch_unittest.moc"
