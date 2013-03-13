/* This file is part of the KDE libraries
    Copyright (c) 2009 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdebug_unittest.h"
#include <qstandardpaths.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <QtTest/QtTest>
#include <kdebug.h>
#include <QtCore/QProcess>

QTEST_MAIN(KDebugTest)

void KDebugTest::initTestCase()
{
    qputenv("KDE_DEBUG_TIMESTAMP", "");
    qputenv("QT_MESSAGE_PATTERN", "%{appname}(%{pid})/%{category} %{function}: %{message}");

    // The source files (kdebugrc and kdebug.areas) are in the "global" config dir:
    qputenv("XDG_CONFIG_DIRS", QFile::encodeName(QFileInfo(QFINDTESTDATA("../../../kdecore/kdebug.areas")).absolutePath()));

    QStandardPaths::enableTestMode(true);

    QString kdebugrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + "kdebugrc";
    if (!kdebugrc.isEmpty())
        QFile::remove(kdebugrc);
    QFile::remove("kdebug.dbg");
    QFile::remove("myarea.dbg");

    // Check that we can find kdebugrc and kdebug.areas
    QString filename(QStandardPaths::locate(QStandardPaths::ConfigLocation, QLatin1String("kdebug.areas")));
    QVERIFY2(QFile::exists(filename), filename.toLatin1() + " not found");
    QVERIFY(QFile::exists(QFINDTESTDATA("../../../kdecore/kdebugrc")));

    // Now set up logging to file
    KConfig config("kdebugrc");
    config.group(QString()).writeEntry("DisableAll", false); // in case of a global kdebugrc with DisableAll=true
    config.group("180").writeEntry("InfoOutput", 0 /*FileOutput*/);
    config.group("myarea").writeEntry("InfoOutput", 0 /*FileOutput*/);
    config.group("myarea").writeEntry("InfoFilename", "myarea.dbg");
    config.group("kdebug_unittest").writeEntry("InfoOutput", 0 /*FileOutput*/);
    config.group("kdebug_unittest").writeEntry("WarnOutput", 0 /*FileOutput*/);
    config.sync();

    //QCOMPARE(KDebug::hasNullOutput(QtDebugMsg, true, 0, false), false);

    // Test for crash that used to happen when using an unknown area after only dynamic areas
    KDebug::registerArea("somearea"); // gets number 1
    KDebug::registerArea("someotherarea"); // gets number 2
    QCOMPARE(KDebug::hasNullOutput(QtDebugMsg, true, 4242, false), false); // unknown area -> area 0 is being used

    kClearDebugConfig();
}

void KDebugTest::cleanupTestCase()
{
    QString kdebugrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + "kdebugrc";
    if (!kdebugrc.isEmpty())
        QFile::remove(kdebugrc);
    // TODO QFile::remove("kdebug.dbg");
    QFile::remove("myarea.dbg");
}



static QList<QByteArray> readLines(const char* fileName = "kdebug.dbg")
{
    const QString path = QFile::decodeName(fileName);
    Q_ASSERT(!path.isEmpty());
    Q_ASSERT(QFile::exists(path));
    QFile file(path);
    const bool opened = file.open(QIODevice::ReadOnly);
    Q_ASSERT(opened);
    Q_UNUSED(opened);
    QList<QByteArray> lines;
    QByteArray line;
    do {
        line = file.readLine();
        if (!line.isEmpty())
            lines.append(line);
    } while(!line.isEmpty());
    return lines;
}

void KDebugTest::compareLines(const QList<QByteArray>& expectedLines, const char* fileName)
{
    QList<QByteArray> lines = readLines(fileName);
    //qDebug() << lines;
    QCOMPARE(lines.count(), expectedLines.count());
    QVERIFY(lines[0].endsWith('\n'));
    for (int i = 0; i < lines.count(); ++i) {
        QByteArray line = lines[i];
        if (expectedLines[i].contains("[...]")) {
            const int pos = line.indexOf('[');
            QVERIFY(pos >= 0);
            line.truncate(pos);
            line.append("[...]\n");
        }
        //qDebug() << "line" << i << ":" << line << expectedLines[i];
        QVERIFY2(line.endsWith(expectedLines[i]), "Got '" + line + "'\nexpected '" + expectedLines[i] + "'");
    }
}

// Test what happens when a operator<< calls a method that itself uses kDebug,
// meaning that two kDebug instances will be active at the same time.
// In this case it works, but technically, if area 180 was configured with
// a different output file than area 0 then the output would currently go
// into the wrong file (because the stream is static) (the "after the call" string
// would go into the file for area 180)
class TestClass
{
public:
    TestClass() {}
    QString getSomething() const {
        kDebug(180) << "Nested kDebug call";
        return "TestClass";
    }
};
QDebug operator<<(QDebug s, const TestClass& me)
{
    s << me.getSomething() << "after the call";
    return s;
}

void KDebugTest::testDebugToFile()
{
    kDebug(180) << "TEST DEBUG 180";
    kDebug(0) << "TEST DEBUG 0";
    kWarning() << "TEST WARNING 0";
    // The calls to kDebug(0) created a dynamic debug area named after the componentdata name
    KConfig config("kdebugrc");
    QVERIFY(config.hasGroup("kdebug_unittest"));
    kDebug(0) << "TEST DEBUG with newline" << endl << "newline";
    TestClass tc;
    kDebug(0) << "Re-entrance test" << tc << "[ok]";
    {
        KDebug::Block block("block 1");
        {
            KDebug::Block block("block 2");
        }
    }
    QVERIFY(QFile::exists("kdebug.dbg"));
    QList<QByteArray> expected;
    expected << "/kdecore (kdelibs) KDebugTest::testDebugToFile: TEST DEBUG 180\n";
    expected << "KDebugTest::testDebugToFile: TEST DEBUG 0\n";
    expected << "KDebugTest::testDebugToFile: TEST WARNING 0\n";
    expected << "KDebugTest::testDebugToFile: TEST DEBUG with newline\n";
    expected << "newline\n";
    expected << "/kdecore (kdelibs) TestClass::getSomething: Nested kDebug call\n";
    expected << "Re-entrance test \"TestClass\" after the call [ok]\n";
    expected << "BEGIN: block 1\n";
    expected << "BEGIN: block 2\n";
    expected << "END__: block 2 [...]\n";
    expected << "END__: block 1 [...]\n";
    compareLines(expected);
}

void KDebugTest::testDisableArea()
{
    QFile::remove("kdebug.dbg");
    KConfig config("kdebugrc");
    config.group("180").writeEntry("InfoOutput", 4 /*NoOutput*/);
    config.group("kdebug_unittest").writeEntry("InfoOutput", 4 /*NoOutput*/);
    config.sync();
    kClearDebugConfig();
    kDebug(180) << "TEST DEBUG 180 - SHOULD NOT APPEAR";
    kDebug(0) << "TEST DEBUG 0 - SHOULD NOT APPEAR";
    {
        KDebug::Block block("SHOULD NOT APPEAR");
        kDebug(0) << "msg inside the block, should not appear";
    }
    QVERIFY(!QFile::exists("kdebug.dbg"));

    // Re-enable debug, for further tests
    config.group("180").writeEntry("InfoOutput", 0 /*FileOutput*/);
    config.group("kdebug_unittest").writeEntry("InfoOutput", 0 /*FileOutput*/);
    config.sync();
    kClearDebugConfig();
}

void KDebugTest::testDynamicArea()
{
    const int myArea = KDebug::registerArea("myarea"); // gets number 3
    QCOMPARE(myArea, 3);
    KConfig config("kdebugrc");
    QVERIFY(!config.hasGroup(QString::number(myArea)));
    QVERIFY(config.hasGroup("myarea"));
    kDebug(myArea) << "TEST DEBUG using myArea" << myArea;
    QList<QByteArray> expected;
    expected << "/myarea KDebugTest::testDynamicArea: TEST DEBUG using myArea 3\n";
    compareLines(expected, "myarea.dbg");
}

void KDebugTest::testDisabledDynamicArea()
{
    const int verboseArea = KDebug::registerArea("verbosearea", false);
    QVERIFY(verboseArea > 0);
    kClearDebugConfig(); // force a sync() of KDebug's own kdebugrc so that it gets written out
    KConfig config("kdebugrc");
    QVERIFY(config.hasGroup("verbosearea"));
    kDebug(verboseArea) << "TEST DEBUG using verboseArea" << verboseArea;
}

static void disableAll(bool dis)
{
    KConfig config("kdebugrc");
    config.group(QString()).writeEntry("DisableAll", dis);
    config.sync();
    kClearDebugConfig();
}

void KDebugTest::testDisableAll()
{
    // Some people really don't like debug output :-)
    disableAll(true);
    QFile::remove("kdebug.dbg");
    kDebug() << "Should not appear";
    kDebug(123465) << "Unknown area, should not appear either";
    QVERIFY(!QFile::exists("kdebug.dbg"));
    // Repair
    disableAll(false);
}

void KDebugTest::testHasNullOutput()
{
    // When compiling in debug mode:
    QCOMPARE(KDebug::hasNullOutput(QtDebugMsg, true, 0, true), false);
    QCOMPARE(KDebug::hasNullOutput(QtDebugMsg, true, 180, true), false);
    QCOMPARE(KDebug::hasNullOutput(QtDebugMsg, true, 293, true), false);
    QCOMPARE(KDebug::hasNullOutput(QtDebugMsg, true, 4242, true), false);

    kClearDebugConfig(); // force dropping the cache

    // When compiling in release mode:
    QCOMPARE(KDebug::hasNullOutput(QtDebugMsg, true, 0, false), false); // controlled by "InfoOutput" key
    QCOMPARE(KDebug::hasNullOutput(QtDebugMsg, true, 180, false), false); // controlled by "InfoOutput" key
    QCOMPARE(KDebug::hasNullOutput(QtDebugMsg, true, 293, false), true); // no config -> the default is being used
    QCOMPARE(KDebug::hasNullOutput(QtDebugMsg, true, 4242, false), false); // unknown area -> area 0 is being used

    // And if we really have no config for area 0 (the app name)
    KConfig config("kdebugrc");
    config.deleteGroup("kdebug_unittest");
    config.sync();
    kClearDebugConfig();

    QCOMPARE(KDebug::hasNullOutput(QtDebugMsg, true, 0, false), true);
    QCOMPARE(KDebug::hasNullOutput(QtDebugMsg, true, 293, false), true);
    QCOMPARE(KDebug::hasNullOutput(QtDebugMsg, true, 4242, false), true);

    // Restore to normal for future tests
    config.group("kdebug_unittest").writeEntry("InfoOutput", 0 /*FileOutput*/);
    config.sync();
    kClearDebugConfig();
}

void KDebugTest::testNoMainComponentData()
{
    // This test runs kdebug_qcoreapptest and checks its output
    QProcess proc;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    // No process info, to make this easier
    environment.insert("QT_MESSAGE_PATTERN", "%{category} %{function}: %{message}");
    proc.setProcessEnvironment(environment);
    proc.setProcessChannelMode(QProcess::SeparateChannels);
#ifdef Q_OS_WIN
    proc.start("kdebug_qcoreapptest.exe");
#else
    if (QFile::exists("./kdebug_qcoreapptest.shell"))
        proc.start("./kdebug_qcoreapptest.shell");
    else {
        QVERIFY(QFile::exists("./kdebug_qcoreapptest"));
        proc.start("./kdebug_qcoreapptest");
    }
#endif
    //     kDebug() << proc.args();
    const bool ok = proc.waitForFinished();
    QVERIFY(ok);
    const QByteArray allOutput = proc.readAllStandardError();
    const QList<QByteArray> receivedLines = allOutput.split('\n');
    //qDebug() << receivedLines;
    QList<QByteArray> expectedLines;
    expectedLines << "qcoreapp_myarea main: Test debug using qcoreapp_myarea 1";
    expectedLines << "kdebug_qcoreapptest main: Debug in area 100";
    expectedLines << "kdebug_qcoreapptest main: Simple debug";
    expectedLines << "kdebug_qcoreapptest main: This should appear, under the kdebug_qcoreapptest area";
    expectedLines << "kdebug_qcoreapptest main: Debug in area 100";
    expectedLines << ""; // artefact of split, I guess?
    for (int i = 0; i < qMin(expectedLines.count(), receivedLines.count()); ++i)
        QCOMPARE(QString::fromLatin1(receivedLines[i]), QString::fromLatin1(expectedLines[i]));
    QCOMPARE(receivedLines.count(), expectedLines.count());
    QCOMPARE(receivedLines, expectedLines);
}

#include <QThreadPool>
#include <QFutureSynchronizer>
#include <qtconcurrentrun.h>

class KDebugThreadTester
{
public:
    void doDebugs()
    {
        KDEBUG_BLOCK
        for (int i = 0; i < 10; ++i)
            kDebug() << "A kdebug statement in a thread:" << i;
    }
};

void KDebugTest::testMultipleThreads()
{
    kDebug() << "kDebug works";
    QVERIFY(QFile::exists("kdebug.dbg"));
    QFile::remove("kdebug.dbg");

    KDebugThreadTester tester;
    QThreadPool::globalInstance()->setMaxThreadCount(10);
    QFutureSynchronizer<void> sync;
    for (int threadNum = 0; threadNum < 10; ++threadNum)
        sync.addFuture(QtConcurrent::run(&tester, &KDebugThreadTester::doDebugs));
    sync.waitForFinished();

    QVERIFY(QFile::exists("kdebug.dbg"));

    //QFile f("kdebug.dbg"); f.open(QIODevice::ReadOnly);
    //qDebug() << QString::fromLatin1(f.readAll());

    // We have no guarantee that the debug lines are issued one after the other.
    // The \n comes from the destruction of the temp kDebug, and that's not mutexed,
    // so we can get msg1 + msg2 + \n + \n.
    // So this test is basically only good for running in helgrind.

#if 0
    // Check that the lines are whole
    QList<QByteArray> lines = readLines();
    Q_FOREACH(const QByteArray& line, lines) {
        qDebug() << line;
        QCOMPARE(line.count("doDebugs"), 1);
        QCOMPARE(line.count('\n'), 1);
        QVERIFY(!line.startsWith("   ")); // more than 2 spaces? indentString messed up
    }
#endif
}
