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
#include <kconfig.h>
#include <kconfiggroup.h>
#include <qtest_kde.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include "kdebug_unittest.moc"

QTEST_KDEMAIN_CORE( KDebugTest )

void KDebugTest::initTestCase()
{
    QString kdebugrc = KStandardDirs::locateLocal("config", "kdebugrc");
    if (!kdebugrc.isEmpty())
        QFile::remove(kdebugrc);
    QFile::remove("kdebug.dbg");

    // Now set up logging to file
    KConfig config("kdebugrc");
    config.group("0").writeEntry("InfoOutput", 0 /*FileOutput*/);
    config.group("180").writeEntry("InfoOutput", 0 /*FileOutput*/);
    config.group("myarea").writeEntry("InfoOutput", 0 /*FileOutput*/);
    config.sync();

    kClearDebugConfig();
}

void KDebugTest::cleanupTestCase()
{
    QString kdebugrc = KStandardDirs::locateLocal("config", "kdebugrc");
    if (!kdebugrc.isEmpty())
        QFile::remove(kdebugrc);
}

static QList<QByteArray> readLines(const char* fileName = "kdebug.dbg")
{
    const QString path = QFile::decodeName(fileName);
    Q_ASSERT(!path.isEmpty());
    Q_ASSERT(QFile::exists(path));
    QFile file(path);
    Q_ASSERT(file.open(QIODevice::ReadOnly));
    QList<QByteArray> lines;
    QByteArray line;
    do {
        line = file.readLine();
        if (!line.isEmpty())
            lines.append(line);
    } while(!line.isEmpty());
    return lines;
}

void KDebugTest::compareLines(const QList<QByteArray>& expectedLines)
{
    QList<QByteArray> lines = readLines();
    //qDebug() << lines;
    QCOMPARE(lines.count(), expectedLines.count());
    QVERIFY(lines[0].endsWith("\n"));
    for (int i = 0; i < lines.count(); ++i) {
        //qDebug() << "line" << i << ":" << lines[i] << expectedLines[i];
        QVERIFY(lines[i].endsWith(expectedLines[i]));
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
    kDebug(0) << "TEST DEBUG with newline" << endl << "newline";
    TestClass tc;
    kDebug(0) << "Re-entrance test" << tc << "[ok]";
    QVERIFY(QFile::exists("kdebug.dbg"));
    QList<QByteArray> expected;
    expected << "/kdecore (kdelibs) KDebugTest::testDebugToFile: TEST DEBUG 180\n";
    expected << "KDebugTest::testDebugToFile: TEST DEBUG 0\n";
    expected << "KDebugTest::testDebugToFile: TEST DEBUG with newline\n";
    expected << "newline\n";
    expected << "/kdecore (kdelibs) TestClass::getSomething: Nested kDebug call\n";
    expected << "Re-entrance test \"TestClass\" after the call [ok]\n";
    compareLines(expected);
}

void KDebugTest::testDisableArea()
{
    QFile::remove("kdebug.dbg");
    KConfig config("kdebugrc");
    config.group("180").writeEntry("InfoOutput", 4 /*NoOutput*/);
    config.group("0").writeEntry("InfoOutput", 4 /*NoOutput*/);
    config.sync();
    kClearDebugConfig();
    kDebug(180) << "TEST DEBUG 180 - SHOULD NOT APPEAR";
    kDebug(0) << "TEST DEBUG 0 - SHOULD NOT APPEAR";
    QVERIFY(!QFile::exists("kdebug.dbg"));
}

void KDebugTest::testDynamicArea()
{
    const int myArea = KDebug::registerArea("myarea");
    QVERIFY(myArea > 0);
    KConfig config("kdebugrc");
    QVERIFY(!config.hasGroup(QString::number(myArea)));
    QVERIFY(config.hasGroup("myarea"));
    kDebug(myArea) << "TEST DEBUG using myArea" << myArea;
    QList<QByteArray> expected;
    expected << "/myarea KDebugTest::testDynamicArea: TEST DEBUG using myArea 1\n";
    compareLines(expected);
}
