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
#include "QtTest/qttestsystem.h"

#include <unistd.h>

namespace QtTest {
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
	VERIFY(!lockFile->isLocked());
	COMPARE(lockFile->lock(), KLockFile::LockOK);
	VERIFY(lockFile->isLocked());
}

void
Test_KLockFile::testStale()
{
	VERIFY(lockFile->isLocked());

	const int secs = 5;
	KLockFile lf = KLockFile(QLatin1String(lockName));
	lf.setStaleTime(secs);
	VERIFY(lf.staleTime() == secs);

	QtTest::wait(secs*1000);
	COMPARE(lf.lock(), KLockFile::LockStale);
	VERIFY(!lf.isLocked());

	int pid;
	QString host, app;
	if (lf.getLockInfo(pid, host, app)) {
		COMPARE(pid, ::getpid());
		char hostname[256];
		if (::gethostname(hostname, sizeof(hostname)) == 0)
			COMPARE(host, QLatin1String(hostname));
		COMPARE(app, QLatin1String("lt-klockfiletest")); // libtool name
	}
}

void
Test_KLockFile::testUnlock()
{
	VERIFY(lockFile->isLocked());
	lockFile->unlock();
	VERIFY(!lockFile->isLocked());
}

QTTEST_KDEMAIN(Test_KLockFile, 0)
