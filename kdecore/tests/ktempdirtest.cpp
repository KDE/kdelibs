/* This file is part of the KDE libraries
    Copyright (c) 2005 Thomas Braxton <brax108@cox.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "qtest_kde.h"

#include <qdir.h>

#include "ktempdirtest.h"
#include "ktempdir.h"
#include "ktempdirtest.moc"

void KTempDirTest::testBasic()
{
	KTempDir dir = KTempDir("test");
	QVERIFY(dir.status() == 0);
	QVERIFY(dir.exists());
	QVERIFY(QDir(dir.name()).exists());

	dir.unlink();
	QVERIFY(dir.status() == 0);
	QVERIFY(!dir.exists());
	QVERIFY(!QDir(dir.name()).exists());
}

void KTempDirTest::testAutoDelete()
{
	KTempDir *dir = new KTempDir("test");
	dir->setAutoDelete(true);
	QVERIFY(dir->status() == 0);
	QVERIFY(dir->exists());

	QString dName = dir->name();
	delete dir;
	QVERIFY(!QDir(dName).exists());
}

void KTempDirTest::testCreateSubDir()
{
	KTempDir dir = KTempDir("test");
	dir.setAutoDelete(true);
	QVERIFY(dir.status() == 0);
	QVERIFY(dir.exists());

	QDir *d = dir.qDir();
	QVERIFY(d->exists());

	QVERIFY(d->mkdir(QString("123")));
	QVERIFY(d->mkdir(QString("456")));
}

QTEST_KDEMAIN(KTempDirTest, 0)
