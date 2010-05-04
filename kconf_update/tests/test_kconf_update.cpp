/* This file is part of the KDE libraries
   Copyright 2010 Canonical Ltd
   Author: Aurélien Gâteau <aurelien.gateau@canonical.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "test_kconf_update.h"

// Qt
#include <QFile>
#include <QSharedPointer>

// KDE
#include <kdebug.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>

#include <qtest_kde.h>

// Local

QTEST_KDEMAIN_CORE(TestKConfUpdate)

static void writeFile(const QString &path, const QString &content)
{
    QFile file(path);
    bool ok = file.open(QIODevice::WriteOnly);
    Q_ASSERT(ok);
    file.write(content.toUtf8());
}

static QString readFile(const QString &path)
{
    QFile file(path);
    bool ok = file.open(QIODevice::ReadOnly);
    Q_ASSERT(ok);
    return QString::fromUtf8(file.readAll());
}

static KTemporaryFile* writeUpdFile(const QString &content)
{
    KTemporaryFile* file = new KTemporaryFile();
    file->setSuffix(".upd");
    bool ok = file->open();
    Q_ASSERT(ok);
    file->write(content.toUtf8());
    file->flush();
    return file;
}

static void runKConfUpdate(const QString &updPath)
{
    QString exePath = KStandardDirs::findExe("kconf_update");
    KProcess::execute(QStringList() << exePath << "--debug" << updPath);
}

void TestKConfUpdate::test_data()
{
    QTest::addColumn<QString>("updContent");
    QTest::addColumn<QString>("oldConfName");
    QTest::addColumn<QString>("oldConfContent");
    QTest::addColumn<QString>("newConfName");
    QTest::addColumn<QString>("expectedNewConfContent");
    QTest::addColumn<QString>("expectedOldConfContent");

    QTest::newRow("moveKeysSameFile")
        <<
        "File=testrc\n"
        "Group=group\n"
        "Key=old,new\n"
        "Options=overwrite\n"
        <<
        "testrc"
        <<
        "[group]\n"
        "old=value\n"
        <<
        "testrc"
        <<
        "[$Version]\n"
        "update_info=%1\n"
        "\n"
        "[group]\n"
        "new=value\n"
        <<
        ""
        ;
    QTest::newRow("moveKeysOtherFile")
        <<
        "File=oldrc,newrc\n"
        "Group=group1,group2\n"
        "Key=old,new\n"
        "Options=overwrite\n"
        <<
        "oldrc"
        <<
        "[group1]\n"
        "old=value\n"
        "[stay]\n"
        "foo=bar\n"
        <<
        "newrc"
        <<
        "[$Version]\n"
        "update_info=%1\n"
        "\n"
        "[group2]\n"
        "new=value\n"
        <<
        "[$Version]\n"
        "update_info=%1\n"
        "\n"
        "[stay]\n"
        "foo=bar\n"
        ;
    QTest::newRow("allKeys")
        <<
        "File=testrc\n"
        "Group=group1,group2\n"
        "AllKeys\n"
        <<
        "testrc"
        <<
        "[group1]\n"
        "key1=value1\n"
        "key2=value2\n"
        "\n"
        "[stay]\n"
        "foo=bar\n"
        <<
        "testrc"
        <<
        "[$Version]\n"
        "update_info=%1\n"
        "\n"
        "[group2]\n"
        "key1=value1\n"
        "key2=value2\n"
        "\n"
        "[stay]\n"
        "foo=bar\n"
        <<
        ""
        ;
    QTest::newRow("allKeysSubGroup")
        <<
        "File=testrc\n"
        "Group=[group][sub1],[group][sub2]\n"
        "AllKeys\n"
        <<
        "testrc"
        <<
        "[group][sub1]\n"
        "key1=value1\n"
        "key2=value2\n"
        "\n"
        "[group][sub1][subsub]\n"
        "key3=value3\n"
        "key4=value4\n"
        "\n"
        "[stay]\n"
        "foo=bar\n"
        <<
        "testrc"
        <<
        "[$Version]\n"
        "update_info=%1\n"
        "\n"
        "[group][sub2]\n"
        "key1=value1\n"
        "key2=value2\n"
        "\n"
        "[group][sub2][subsub]\n"
        "key3=value3\n"
        "key4=value4\n"
        "\n"
        "[stay]\n"
        "foo=bar\n"
        <<
        ""
        ;
    QTest::newRow("removeGroup")
        <<
        "File=testrc\n"
        "RemoveGroup=remove\n"
        <<
        "testrc"
        <<
        "[keep]\n"
        "key=value\n"
        ""
        "[remove]\n"
        "key=value\n"
        <<
        "testrc"
        <<
        "[$Version]\n"
        "update_info=%1\n"
        "\n"
        "[keep]\n"
        "key=value\n"
        <<
        ""
        ;
}

void TestKConfUpdate::test()
{
    QFETCH(QString, updContent);
    QFETCH(QString, oldConfName);
    QFETCH(QString, oldConfContent);
    QFETCH(QString, newConfName);
    QFETCH(QString, expectedNewConfContent);
    QFETCH(QString, expectedOldConfContent);

    // Prepend the Id= field to the upd content
    updContent = QString("Id=%1\n").arg(QTest::currentDataTag()) + updContent;

    QString oldConfPath = KStandardDirs::locateLocal("config", oldConfName);
    QString newConfPath = KStandardDirs::locateLocal("config", newConfName);

    QFile::remove(oldConfPath);
    QFile::remove(newConfPath);

    writeFile(oldConfPath, oldConfContent);
    QSharedPointer<KTemporaryFile> updFile(writeUpdFile(updContent));
    runKConfUpdate(updFile->fileName());

    QString updateInfo = QString("%1:%2")
        .arg(updFile->fileName().section('/', -1))
        .arg(QTest::currentDataTag());

    QString newConfContentAfter = readFile(newConfPath);
    expectedNewConfContent = expectedNewConfContent.arg(updateInfo);
    QCOMPARE(newConfContentAfter, expectedNewConfContent);

    if (oldConfName != newConfName) {
        QString oldConfContentAfter = readFile(oldConfPath);
        expectedOldConfContent = expectedOldConfContent.arg(updateInfo);
        QCOMPARE(oldConfContentAfter, expectedOldConfContent);
    }
}

void TestKConfUpdate::testScript_data()
{
    QTest::addColumn<QString>("updContent");
    QTest::addColumn<QString>("updScript");
    QTest::addColumn<QString>("oldConfContent");
    QTest::addColumn<QString>("expectedNewConfContent");

    QTest::newRow("delete-key")
        <<
        "File=testrc\n"
        "Group=group\n"
        "Script=test.sh,sh\n"
        <<
        "echo '# DELETE deprecated'\n"
        <<
        "[group]\n"
        "deprecated=foo\n"
        "valid=bar\n"
        <<
        "[$Version]\n"
        "update_info=%1\n"
        "\n"
        "[group]\n"
        "valid=bar\n"
        ;

    QTest::newRow("delete-key2")
        <<
        "File=testrc\n"
        "Script=test.sh,sh\n"
        <<
        "echo '# DELETE [group]deprecated'\n"
        "echo '# DELETE [group][sub]deprecated2'\n"
        <<
        "[group]\n"
        "deprecated=foo\n"
        "valid=bar\n"
        "\n"
        "[group][sub]\n"
        "deprecated2=foo\n"
        "valid2=bar\n"
        <<
        "[$Version]\n"
        "update_info=%1\n"
        "\n"
        "[group]\n"
        "valid=bar\n"
        "\n"
        "[group][sub]\n"
        "valid2=bar\n"
        ;

    QTest::newRow("delete-group")
        <<
        "File=testrc\n"
        "Script=test.sh,sh\n"
        <<
        "echo '# DELETEGROUP [group1]'\n"
        "echo '# DELETEGROUP [group2][sub]'\n"
        <<
        "[group1]\n"
        "key=value\n"
        "\n"
        "[group2]\n"
        "valid=bar\n"
        "\n"
        "[group2][sub]\n"
        "key=value\n"
        <<
        "[$Version]\n"
        "update_info=%1\n"
        "\n"
        "[group2]\n"
        "valid=bar\n"
        ;

    QTest::newRow("delete-group2")
        <<
        "File=testrc\n"
        "Group=group\n"
        "Script=test.sh,sh\n"
        <<
        "echo '# DELETEGROUP'\n"
        <<
        "[group]\n"
        "key=value\n"
        "\n"
        "[group2]\n"
        "valid=bar\n"
        <<
        "[$Version]\n"
        "update_info=%1\n"
        "\n"
        "[group2]\n"
        "valid=bar\n"
        ;

    QTest::newRow("new-key")
        <<
        "File=testrc\n"
        "Script=test.sh,sh\n"
        <<
        "echo '[group]'\n"
        "echo 'new=value'\n"
        <<
        "[group]\n"
        "valid=bar\n"
        <<
        "[$Version]\n"
        "update_info=%1\n"
        "\n"
        "[group]\n"
        "new=value\n"
        "valid=bar\n"
        ;

    QTest::newRow("modify-key-no-overwrite")
        <<
        "File=testrc\n"
        "Script=test.sh,sh\n"
        <<
        "echo '[group]'\n"
        "echo 'existing=new'\n"
        <<
        "[group]\n"
        "existing=old\n"
        <<
        "[$Version]\n"
        "update_info=%1\n"
        "\n"
        "[group]\n"
        "existing=old\n"
        ;

    QTest::newRow("modify-key-overwrite")
        <<
        "File=testrc\n"
        "Options=overwrite\n"
        "Script=test.sh,sh\n"
        <<
        "echo '[group]'\n"
        "echo 'existing=new'\n"
        <<
        "[group]\n"
        "existing=old\n"
        <<
        "[$Version]\n"
        "update_info=%1\n"
        "\n"
        "[group]\n"
        "existing=new\n"
        ;

    QTest::newRow("new-key-in-subgroup")
        <<
        "File=testrc\n"
        "Script=test.sh,sh\n"
        <<
        "echo '[group][sub]'\n"
        "echo 'new=value2'\n"
        <<
        "[group][sub]\n"
        "existing=foo\n"
        <<
        "[$Version]\n"
        "update_info=%1\n"
        "\n"
        "[group][sub]\n"
        "existing=foo\n"
        "new=value2\n"
        ;

    QTest::newRow("new-key-in-subgroup2")
        <<
        "File=testrc\n"
        "Script=test.sh,sh\n"
        <<
        "echo '[group][sub]'\n"
        "echo 'new=value3'\n"
        <<
        "[group][sub]\n"
        "existing=foo\n"
        <<
        "[$Version]\n"
        "update_info=%1\n"
        "\n"
        "[group][sub]\n"
        "existing=foo\n"
        "new=value3\n"
        ;

    QTest::newRow("filter")
        <<
        "File=testrc\n"
        "Script=test.sh,sh\n"
        <<
        "echo '# DELETE [group]changed'\n"
        "sed s/value/VALUE/\n"
        <<
        "[group]\n"
        "changed=value\n"
        "unchanged=value\n"
        <<
        "[$Version]\n"
        "update_info=%1\n"
        "\n"
        "[group]\n"
        "changed=VALUE\n"
        "unchanged=value\n"
        ;

    QTest::newRow("filter-subgroup")
        <<
        "File=testrc\n"
        "Script=test.sh,sh\n"
        <<
        "echo '# DELETE [group][sub]changed'\n"
        "sed s/value/VALUE/\n"
        <<
        "[group]\n"
        "unchanged=value\n"
        "\n"
        "[group][sub]\n"
        "changed=value\n"
        "unchanged=value\n"
        <<
        "[$Version]\n"
        "update_info=%1\n"
        "\n"
        "[group]\n"
        "unchanged=value\n"
        "\n"
        "[group][sub]\n"
        "changed=VALUE\n"
        "unchanged=value\n"
        ;
}

void TestKConfUpdate::testScript()
{
    QFETCH(QString, updContent);
    QFETCH(QString, updScript);
    QFETCH(QString, oldConfContent);
    QFETCH(QString, expectedNewConfContent);

    // Prepend the Id= field to the upd content
    updContent = QString("Id=%1\n").arg(QTest::currentDataTag()) + updContent;

    QSharedPointer<KTemporaryFile> updFile(writeUpdFile(updContent));

    QString scriptPath = KStandardDirs::locateLocal("data", "kconf_update/test.sh");
    writeFile(scriptPath, updScript);

    QString confPath = KStandardDirs::locateLocal("config", "testrc");
    writeFile(confPath, oldConfContent);

    runKConfUpdate(updFile->fileName());

    QString updateInfo = QString("%1:%2")
        .arg(updFile->fileName().section('/', -1))
        .arg(QTest::currentDataTag());
    QString newConfContent = readFile(confPath);
    expectedNewConfContent = expectedNewConfContent.arg(updateInfo);
    QCOMPARE(newConfContent, expectedNewConfContent);
}

#include "test_kconf_update.moc"
