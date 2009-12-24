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

private Q_SLOTS:
    void touchOneFile();
    void touch1000Files();
    void removeAndReAdd();

private:
    QList<QVariantList> waitForDirtySignal(KDirWatch& dw, int expected);
    void createFile(int num);
    KTempDir m_tempDir;
    QString m_path;
};

QTEST_KDEMAIN(KDirWatch_UnitTest, GUI)

// Just to make the inotify packets bigger
static const char* s_filePrefix = "This_is_a_test_file_";

// helper method
void KDirWatch_UnitTest::createFile(int num)
{
    const QString fileName = s_filePrefix + QString::number(num);
    QFile file(m_path + fileName);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QByteArray("foo"));
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

void KDirWatch_UnitTest::touchOneFile()
{
    KDirWatch watch;
    watch.addDir(m_path);
    watch.startScan();

    createFile(0);
    QList<QVariantList> spy = waitForDirtySignal(watch, 1);
    QVERIFY(spy.count() >= 1);
    QCOMPARE(spy[0][0].toString(), m_path + s_filePrefix + "0");
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
    createFile(1);
    spy = waitForDirtySignal(watch, 1);
    QVERIFY(spy.count() >= 1);
    QCOMPARE(spy[0][0].toString(), m_path + s_filePrefix + "1");
}

#include "kdirwatch_unittest.moc"
