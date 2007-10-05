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
#include "kdesktopfiletest.h"
#include "kdesktopfiletest.moc"

#include "kdesktopfile.h"

#include <qtest_kde.h>

QTEST_KDEMAIN_CORE( KDesktopFileTest )

void KDesktopFileTest::testRead()
{
    const QString fileName = QFile::decodeName(KDESRCDIR "/../services/kplugininfo.desktop");
    QVERIFY(QFile::exists(fileName));
    QVERIFY(KDesktopFile::isDesktopFile(fileName));
    KDesktopFile df(fileName);
    QCOMPARE(df.readType(), QString::fromLatin1("ServiceType"));
    QCOMPARE(df.readIcon(), QString());
    QCOMPARE(df.readName(), QString::fromLatin1("KDE Plugin Information"));
    QCOMPARE(df.hasLinkType(), false);
    QCOMPARE(df.hasMimeTypeType(), false);
    QCOMPARE(df.hasApplicationType(), false);
    QCOMPARE(df.fileName(), fileName);
    QCOMPARE(df.resource(), "apps"); // default for .desktop files
}
