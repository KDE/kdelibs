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
#include "QtTest/qttest_kde.h"

#include "kshell.h"
#include "kuser.h"
#include <qstring.h>
#include <qstringlist.h>
#include <qdir.h>

#include "kshelltest.moc"

void
KShellTest::tildeExpand()
{
	COMPARE(KShell::tildeExpand("~"), QDir::homePath());
	COMPARE(KShell::tildeExpand("~/sulli"), QDir::homePath()+"/sulli");
	COMPARE(KShell::tildeExpand("~root"), KUser("root").homeDir());
	COMPARE(KShell::tildeExpand("~root/sulli"),
		KUser("root").homeDir()+"/sulli");
	COMPARE(KShell::tildeExpand("~sulli"), KUser("sulli").homeDir());
}

void
KShellTest::joinArgs()
{
	QStringList list;

	list << "this" << "is" << "a" << "test";
	COMPARE(KShell::joinArgs(list), QString("this is a test"));
	list.clear();

	list << "this" << "is" << "with" << "a space";
	COMPARE(KShell::joinArgs(list), QString("this is with 'a space'"));
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

	COMPARE(sj("\"~sulli\" 'text' 'jo'\"jo\" $'crap' $'\\\\\\'\\e\\x21' ha\\ lo ",KShell::NoOptions, &err),
		QString("~sulli text jojo crap $'\\\\\\'\\e!' $'ha lo'"));
	VERIFY(err == 0);

	COMPARE(sj("\"~sulli\" 'text'", KShell::TildeExpand, &err),
		QString("~sulli text"));
	VERIFY(err == 0);

	COMPARE(sj("~\"sulli\" 'text'", KShell::TildeExpand, &err),
		QString("~sulli text"));
	VERIFY(err == 0);

	COMPARE(sj("~/\"sulli\" 'text'", KShell::TildeExpand, &err),
		QDir::homePath() + "/sulli text");
	VERIFY(err == 0);

	COMPARE(sj("~ 'text' ~", KShell::TildeExpand, &err),
		QDir::homePath() + " text " + QDir::homePath());
	VERIFY(err == 0);

	COMPARE(sj("~sulli ~root", KShell::TildeExpand, &err),
		QString("~sulli ") + KUser("root").homeDir());
	VERIFY(err == 0);
}

void
KShellTest::abortOnMeta()
{
	int err1=0, err2=0;

	COMPARE(sj("say \" error", KShell::NoOptions, &err1),
		QString());
	VERIFY(err1 != 0);

	COMPARE(sj("say \" still error", KShell::AbortOnMeta, &err1),
		QString());
	VERIFY(err1 != 0);

	VERIFY(sj("say `echo no error`", KShell::NoOptions, &err1) !=
		sj("say `echo no error`", KShell::AbortOnMeta, &err2));
	VERIFY(err1 != err2);

	VERIFY(sj("BLA=say echo meta", KShell::NoOptions, &err1) !=
		sj("BLA=say echo meta", KShell::AbortOnMeta, &err2));
	VERIFY(err1 != err2);

	VERIFY(sj("B\"L\"A=say FOO=bar echo meta", KShell::NoOptions, &err1) ==
		sj("B\"L\"A=say FOO=bar echo meta", KShell::AbortOnMeta, &err2));
	VERIFY(err1 == err2);
}

QTTEST_KDEMAIN(KShellTest, 0)
