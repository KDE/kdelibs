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

#include <kdebug.h>
#include <ktempdir.h>
#include <qtest_kde.h>
#include <qtestevent.h>
#include <kdirwatch.h>

// Note that kdirlistertest and kdirmodeltest also exercise KDirWatch quite a lot.

class KDirWatch_UnitTest : public QObject
{
    Q_OBJECT
public:
    KDirWatch_UnitTest()
    {
        m_path = m_tempDir.name();
    }

private Q_SLOTS: // test methods
    void touchOneFile();
    void watchAndModifyOneFile();
    void touch1000Files();
    void removeAndReAdd();
    void nestedEventLoop();

protected Q_SLOTS: // internal slots
    void nestedEventLoopSlot();
    
private:
    QList<QVariantList> waitForDirtySignal(KDirWatch& dw, int expected);
    QString createFile(int num);
    void appendToFile(int num);
    
    KTempDir m_tempDir;
    QString m_path;
};

QTEST_KDEMAIN(KDirWatch_UnitTest, GUI)

// Just to make the inotify packets bigger
static const char s_filePrefix[] = "This_is_a_test_file_";

// helper method
QString KDirWatch_UnitTest::createFile(int num)
{
    const QString fileName = s_filePrefix + QString::number(num);
    QFile file(m_path + fileName);
    bool ok = file.open(QIODevice::WriteOnly);
    Q_ASSERT(ok);
    file.write(QByteArray("foo"));
    file.close();
    return m_path + fileName;
}

void KDirWatch_UnitTest::appendToFile(int num)
{
    const QString fileName = s_filePrefix + QString::number(num);
    QFile file(m_path + fileName);
    QVERIFY(file.open(QIODevice::Append | QIODevice::WriteOnly));
    file.write(QByteArray("foobar"));
    file.close();
}

// helper method
QList<QVariantList> KDirWatch_UnitTest::waitForDirtySignal(KDirWatch& watch, int expected)
{
    QSignalSpy spyDirty(&watch, SIGNAL(dirty(QString)));
    int numTries = 0;
    // Give time for KDirWatch to notify us
    while (spyDirty.count() < expected) {
        if (++numTries > 10) {
            kWarning() << "Timeout waiting for KDirWatch. Got" << spyDirty.count() << "dirty() signals, expected" << expected;
            return spyDirty;
        }
        QTest::qWait(200);
    }
    return spyDirty;
}

void KDirWatch_UnitTest::touchOneFile() // watch a dir, create a file in it
{
    KDirWatch watch;
    watch.addDir(m_path);
    watch.startScan();

    const QString file0 = createFile(0);
    QList<QVariantList> spy = waitForDirtySignal(watch, 1);
    QVERIFY(spy.count() >= 1);
    QCOMPARE(spy[0][0].toString(), file0);
}

void KDirWatch_UnitTest::watchAndModifyOneFile() // watch a specific file, and modify it
{
    KDirWatch watch;
    const QString file = createFile(0);
    watch.addFile(file);
    watch.startScan();

    // Make sure KDirWatch treats the modification as one
    QTest::qWait(1000);

    appendToFile(0);

    QList<QVariantList> spy = waitForDirtySignal(watch, 1);
    QVERIFY(spy.count() >= 1);
    QCOMPARE(spy[0][0].toString(), file);
}

void KDirWatch_UnitTest::touch1000Files()
{
    KDirWatch watch;
    watch.addDir(m_path);
    watch.startScan();

    const int fileCount = 5000;
    for (int i = 0; i < fileCount; ++i) {
        createFile(i);
    }

    QList<QVariantList> spy = waitForDirtySignal(watch, fileCount);
    QVERIFY(spy.count() >= fileCount);
    qDebug() << spy.count();
}

void KDirWatch_UnitTest::removeAndReAdd()
{
    KDirWatch watch;
    watch.addDir(m_path);
    watch.startScan();
    createFile(0);
    QList<QVariantList> spy = waitForDirtySignal(watch, 1);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy[0][0].toString(), m_path + s_filePrefix + "0");
    // Just like KDirLister does: remove the watch, then re-add it.
    watch.removeDir(m_path);
    watch.addDir(m_path);
    const QString file1 = createFile(1);
    spy = waitForDirtySignal(watch, 1);
    QVERIFY(spy.count() >= 1);
    QCOMPARE(spy[0][0].toString(), file1);
}

void KDirWatch_UnitTest::nestedEventLoop() // #220153: watch two files, and modify 2nd while in slot for 1st
{
    KDirWatch watch;

    const QString file0 = createFile(0);
    watch.addFile(file0);
    const QString file1 = createFile(1);
    watch.addFile(file1);
    watch.startScan();

    // Make sure KDirWatch treats the modification as one
    QTest::qWait(1000);

    appendToFile(0);

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

    appendToFile(1);
    kDebug() << "now waiting for signal";
    // The nested event processing here was from a messagebox in #220153
    QList<QVariantList> spy = waitForDirtySignal(*watch, 1);
    QVERIFY(spy.count() >= 1);
    const QString file1 = m_path + s_filePrefix + "1";
    QCOMPARE(spy[spy.count()-1][0].toString(), file1);
    kDebug() << "done";

    // Now the user pressed reload...
    const QString file0 = m_path + s_filePrefix + "0";
    watch->removeFile(file0);
    watch->addFile(file0);
}

#include "kdirwatch_unittest.moc"
