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

#include "QtTest/qttest_kde.h"

#include <qdir.h>

#include "ktempdirtest.h"
#include "ktempdir.h"
#include "ktempdirtest.moc"

void KTempDirTest::testBasic()
{
	KTempDir dir = KTempDir("test");
	VERIFY(dir.status() == 0);
	VERIFY(dir.existing());
	VERIFY(QDir(dir.name()).exists());

	dir.unlink();
	VERIFY(dir.status() == 0);
	VERIFY(!dir.existing());
	VERIFY(!QDir(dir.name()).exists());
}

void KTempDirTest::testAutoDelete()
{
	KTempDir *dir = new KTempDir("test");
	dir->setAutoDelete(true);
	VERIFY(dir->status() == 0);
	VERIFY(dir->existing());

	QString dName = dir->name();
	delete dir;
	VERIFY(!QDir(dName).exists());
}

void KTempDirTest::testCreateSubDir()
{
	KTempDir dir = KTempDir("test");
	dir.setAutoDelete(true);
	VERIFY(dir.status() == 0);
	VERIFY(dir.existing());

	QDir *d = dir.qDir();
	VERIFY(d->exists());

	VERIFY(d->mkdir(QString("123")));
	VERIFY(d->mkdir(QString("456")));
}

QTTEST_KDEMAIN(KTempDirTest, 0)
