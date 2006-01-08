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

#include "kshelltest.h"
#include "qtest_kde.h"

#include "kshell.h"
#include "kuser.h"
#include <qstring.h>
#include <qstringlist.h>
#include <qdir.h>

#include "kshelltest.moc"

void
KShellTest::tildeExpand()
{
	QCOMPARE(KShell::tildeExpand("~"), QDir::homePath());
	QCOMPARE(KShell::tildeExpand("~/sulli"), QDir::homePath()+"/sulli");
	QCOMPARE(KShell::tildeExpand("~root"), KUser("root").homeDir());
	QCOMPARE(KShell::tildeExpand("~root/sulli"),
		KUser("root").homeDir()+"/sulli");
	QCOMPARE(KShell::tildeExpand("~sulli"), KUser("sulli").homeDir());
}

void
KShellTest::joinArgs()
{
	QStringList list;

	list << "this" << "is" << "a" << "test";
	QCOMPARE(KShell::joinArgs(list), QString("this is a test"));
	list.clear();

	list << "this" << "is" << "with" << "a space";
	QCOMPARE(KShell::joinArgs(list), QString("this is with 'a space'"));
	list.clear();
}

static QString sj(const QString& str, int flags, int* ret)
{
	return KShell::joinArgsDQ(KShell::splitArgs(str, flags, ret));
}

void
KShellTest::splitJoinDQ()
{
	int err=0;

	QCOMPARE(sj("\"~sulli\" 'text' 'jo'\"jo\" $'crap' $'\\\\\\'\\e\\x21' ha\\ lo ",KShell::NoOptions, &err),
		QString("~sulli text jojo crap $'\\\\\\'\\e!' $'ha lo'"));
	QVERIFY(err == 0);

	QCOMPARE(sj("\"~sulli\" 'text'", KShell::TildeExpand, &err),
		QString("~sulli text"));
	QVERIFY(err == 0);

	QCOMPARE(sj("~\"sulli\" 'text'", KShell::TildeExpand, &err),
		QString("~sulli text"));
	QVERIFY(err == 0);

	QCOMPARE(sj("~/\"sulli\" 'text'", KShell::TildeExpand, &err),
		QDir::homePath() + "/sulli text");
	QVERIFY(err == 0);

	QCOMPARE(sj("~ 'text' ~", KShell::TildeExpand, &err),
		QDir::homePath() + " text " + QDir::homePath());
	QVERIFY(err == 0);

	QCOMPARE(sj("~sulli ~root", KShell::TildeExpand, &err),
		QString("~sulli ") + KUser("root").homeDir());
	QVERIFY(err == 0);
}

void
KShellTest::abortOnMeta()
{
	int err1=0, err2=0;

	QCOMPARE(sj("say \" error", KShell::NoOptions, &err1),
		QString());
	QVERIFY(err1 != 0);

	QCOMPARE(sj("say \" still error", KShell::AbortOnMeta, &err1),
		QString());
	QVERIFY(err1 != 0);

	QVERIFY(sj("say `echo no error`", KShell::NoOptions, &err1) !=
		sj("say `echo no error`", KShell::AbortOnMeta, &err2));
	QVERIFY(err1 != err2);

	QVERIFY(sj("BLA=say echo meta", KShell::NoOptions, &err1) !=
		sj("BLA=say echo meta", KShell::AbortOnMeta, &err2));
	QVERIFY(err1 != err2);

	QVERIFY(sj("B\"L\"A=say FOO=bar echo meta", KShell::NoOptions, &err1) ==
		sj("B\"L\"A=say FOO=bar echo meta", KShell::AbortOnMeta, &err2));
	QVERIFY(err1 == err2);
}

QTEST_KDEMAIN(KShellTest, 0)
