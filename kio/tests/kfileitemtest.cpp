/*
 *  Copyright (C) 2005 David Faure   <faure@kde.org>
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

#include "kfileitemtest.h"
#include <qtest_kde.h>
#include "kfileitemtest.moc"
#include <kfileitem.h>

#include <ktempdir.h>

QTEST_KDEMAIN( KFileItemTest, NoGUI )

void KFileItemTest::initTestCase()
{
}

void KFileItemTest::testPermissionsString()
{
    // Directory
    KTempDir tempDir;
    tempDir.setAutoDelete(true);
    KFileItem dirItem(KUrl(tempDir.name()), QString(), KFileItem::Unknown);
    QCOMPARE(dirItem.permissions(), (uint)0700);
    QCOMPARE(dirItem.permissionsString(), QString("drwx------"));

    // File
    QFile file(tempDir.name() + "afile");
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadOther); // 0604
    KFileItem fileItem(KUrl(file.fileName()), QString(), KFileItem::Unknown);
    QCOMPARE(fileItem.permissions(), (uint)0604);
    QCOMPARE(fileItem.permissionsString(), QString("-rw----r--"));

    // Symlink
    QString symlink = tempDir.name() + "asymlink";
    QVERIFY( file.link( symlink ) );
    KFileItem symlinkItem(KUrl(symlink), QString(), KFileItem::Unknown);
    QCOMPARE(symlinkItem.permissions(), (uint)0604);
    // This is a bit different from "ls -l": we get the 'l' but we see the permissions of the target.
    // This is actually useful though; the user sees it's a link, and can check if he can read the [target] file.
    QCOMPARE(symlinkItem.permissionsString(), QString("lrw----r--"));
}

