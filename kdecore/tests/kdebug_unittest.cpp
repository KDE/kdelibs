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
    config.group("126").writeEntry("InfoOutput", 0 /*FileOutput*/);
    config.sync();

    kClearDebugConfig();
}

static QList<QByteArray> readLines(const char* fileName = "kdebug.dbg")
{
    const QString path = QFile::decodeName(fileName);
    Q_ASSERT(!path.isEmpty());
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

void KDebugTest::testDebugToFile()
{
    kDebug(126) << "TEST DEBUG 126";
    kDebug(0) << "TEST DEBUG 0";
    QVERIFY(QFile::exists("kdebug.dbg"));
    QList<QByteArray> lines = readLines();
    QCOMPARE(lines.count(), 2);
    //qDebug() << lines[0];
    QVERIFY(lines[0].endsWith(" \n")); // Hmm. Something adds a trailing space.
    QVERIFY(lines[0].endsWith("/kdecore (KUrl) KDebugTest::testDebugToFile: TEST DEBUG 126 \n"));
    QVERIFY(lines[1].endsWith("KDebugTest::testDebugToFile: TEST DEBUG 0 \n"));
}

void KDebugTest::testDisableArea()
{
    QFile::remove("kdebug.dbg");
    KConfig config("kdebugrc");
    config.group("126").writeEntry("InfoOutput", 4 /*NoOutput*/);
    config.group("0").writeEntry("InfoOutput", 4 /*NoOutput*/);
    config.sync();
    kClearDebugConfig();
    kDebug(126) << "TEST DEBUG 126 - SHOULD NOT APPEAR";
    kDebug(0) << "TEST DEBUG 0 - SHOULD NOT APPEAR";
    QVERIFY(!QFile::exists("kdebug.dbg"));
}

void KDebugTest::cleanupTestCase()
{
    QString kdebugrc = KStandardDirs::locateLocal("config", "kdebugrc");
    if (!kdebugrc.isEmpty())
        QFile::remove(kdebugrc);
}
