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
#include <ktempdir.h>

#include <QtCore/QDir>
#include <QtCore/QObject>
#include <QtCore/QDebug>

class KTempDirTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testDefaultCtor();
    void testNoDelete();
    void testAutoDelete();
    void testCreateSubDir();
};

void KTempDirTest::testDefaultCtor()
{
    QString name;
    {
	KTempDir dir;
	QVERIFY(dir.status() == 0);
	QVERIFY(dir.exists());
        name = dir.name();
        qDebug() << name;
	QVERIFY(QDir(name).exists());
    }
    QVERIFY(!QDir(name).exists());
#ifdef Q_OS_UNIX
    QVERIFY2(name.startsWith(QLatin1String("/tmp/")), qPrintable(name));
#endif
}

void KTempDirTest::testNoDelete()
{
	KTempDir dir(QLatin1String("test"));
	dir.setAutoRemove(false);
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
	KTempDir *dir = new KTempDir(QLatin1String("test"));
	QVERIFY(dir->status() == 0);
	QVERIFY(dir->exists());

	QString dName = dir->name();
	delete dir;
	QVERIFY(!QDir(dName).exists());
}

void KTempDirTest::testCreateSubDir()
{
	KTempDir *dir = new KTempDir(QLatin1String("test"));
	QVERIFY(dir->status() == 0);
	QVERIFY(dir->exists());

	QDir d ( dir->name() );
	QVERIFY(d.exists());

	QVERIFY(d.mkdir(QString(QString::fromLatin1("123"))));
	QVERIFY(d.mkdir(QString(QString::fromLatin1("456"))));

	QString dName = dir->name();
	delete dir;
	d.refresh();

	QVERIFY(!QDir(dName).exists());
	QVERIFY(!d.exists(QString::fromLatin1(("123"))));
	QVERIFY(!d.exists(QString(QString::fromLatin1("456"))));
}

QTEST_MAIN(KTempDirTest)

#include "ktempdirtest.moc"
