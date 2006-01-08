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

#include "klockfiletest.h"
#include "klockfiletest.moc"

#include <unistd.h>

namespace QTest {
template<>
char* toString(const KLockFile::LockResult& result)
{
	static const char *const strings[] = {
		"LockOK", "LockFail", "LockError", "LockStale"
	};
	return qstrdup(strings[result]);
}
}

static const char *const lockName = "klockfiletest.lock";

void
Test_KLockFile::initTestCase()
{
	lockFile = new KLockFile(QLatin1String(lockName));
}

void
Test_KLockFile::testLock()
{
	QVERIFY(!lockFile->isLocked());
	QCOMPARE(lockFile->lock(), KLockFile::LockOK);
	QVERIFY(lockFile->isLocked());
}

void
Test_KLockFile::testStale()
{
	QVERIFY(lockFile->isLocked());

	const int secs = 5;
	KLockFile lf = KLockFile(QLatin1String(lockName));
	lf.setStaleTime(secs);
	QVERIFY(lf.staleTime() == secs);

	QTest::qWait(secs*1000);
	QCOMPARE(lf.lock(), KLockFile::LockStale);
	QVERIFY(!lf.isLocked());

	int pid;
	QString host, app;
	if (lf.getLockInfo(pid, host, app)) {
		QCOMPARE(pid, ::getpid());
		char hostname[256];
		if (::gethostname(hostname, sizeof(hostname)) == 0)
			QCOMPARE(host, QLatin1String(hostname));
		QCOMPARE(app, QLatin1String("lt-klockfiletest")); // libtool name
	}
}

void
Test_KLockFile::testUnlock()
{
	QVERIFY(lockFile->isLocked());
	lockFile->unlock();
	QVERIFY(!lockFile->isLocked());
}

QTEST_KDEMAIN(Test_KLockFile, 0)
