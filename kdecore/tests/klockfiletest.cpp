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

#include <kdebug.h>
#include <unistd.h>
#include <qtconcurrentrun.h>

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

// Here's how to test file locking on a FAT filesystem, on linux:
// cd /tmp
// dd if=/dev/zero of=fatfile count=180000
// mkfs.vfat -F32 fatfile
// mkdir -p fatsystem
// sudo mount -o loop -o uid=$UID fatfile fatsystem
// 
//static const char *const lockName = "/tmp/fatsystem/klockfiletest.lock";

static const char *const lockName = "klockfiletest.lock";

void
Test_KLockFile::initTestCase()
{
	QFile::remove( lockName );
	lockFile = new KLockFile(QLatin1String(lockName));
}

static KLockFile::LockResult testLockFromProcess(const QString& lockName)
{
    const int ret = QProcess::execute("./klockfile_testlock", QStringList() << lockName);
    return KLockFile::LockResult(ret);
}

void
Test_KLockFile::testLock()
{
	QVERIFY(!lockFile->isLocked());
	QCOMPARE(lockFile->lock(), KLockFile::LockOK);
	QVERIFY(lockFile->isLocked());

        // Try to lock it again, should fail
	KLockFile *lockFile2 = new KLockFile(QLatin1String(lockName));
	QVERIFY(!lockFile2->isLocked());
	QCOMPARE(lockFile2->lock(KLockFile::NoBlockFlag), KLockFile::LockFail);
	QVERIFY(!lockFile2->isLocked());
	delete lockFile2;

        // Also try from a different process.
        QCOMPARE(testLockFromProcess(QLatin1String(lockName)), KLockFile::LockFail);
}

void
Test_KLockFile::testStale()
{
#ifdef Q_WS_WIN
    qDebug("unix stale lock support not implemented yet");
#else
	QVERIFY(lockFile->isLocked());

	const int secs = 2;
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
		QCOMPARE(app, QLatin1String("qttest")); // this is our KComponentData name
	}
#endif
}

void
Test_KLockFile::testUnlock()
{
	QVERIFY(lockFile->isLocked());
	lockFile->unlock();
	QVERIFY(!lockFile->isLocked());
}


void
Test_KLockFile::testStaleNoBlockFlag()
{
#ifdef Q_WS_WIN
    QSKIP("lockfile on windows has different format",SkipSingle);
#else
    char hostname[256];
    ::gethostname(hostname, sizeof(hostname));

    QFile f(lockName);
    f.open(QIODevice::WriteOnly);
    QTextStream stream(&f);
    stream << QString::number(111222) << endl << QLatin1String("qttest") << endl << hostname << endl;
    stream.flush();
    f.close();

    lockFile = new KLockFile(QLatin1String(lockName));
    QVERIFY(!lockFile->isLocked());
    QCOMPARE(lockFile->lock(KLockFile::NoBlockFlag), KLockFile::LockStale);
    QByteArray expectedMsg = QByteArray("WARNING: deleting stale lockfile ") + lockName;
    QTest::ignoreMessage(QtWarningMsg, expectedMsg);
    QCOMPARE(lockFile->lock(KLockFile::NoBlockFlag|KLockFile::ForceFlag), KLockFile::LockOK);

    QVERIFY(lockFile->isLocked());
#endif
}


QTEST_KDEMAIN_CORE(Test_KLockFile)
