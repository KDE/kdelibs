/* This file is part of the KDE libraries
    Copyright (c) 2003,2007 Oswald Buddenhagen <ossi@kde.org>
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

#include <kshell.h>
#include <kuser.h>

#include <qtest_kde.h>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDir>

class KShellTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void tildeExpand();
    void joinArgs();
    void splitJoinDQ();
    void abortOnMeta();
};

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

static QString sj(const QString& str, KShell::Options flags, KShell::Errors* ret)
{
    return KShell::joinArgs(KShell::splitArgs(str, flags, ret));
}

void
KShellTest::splitJoinDQ()
{
    KShell::Errors err = KShell::NoError;

    QCOMPARE(sj("\"~sulli\" 'text' 'jo'\"jo\" $'crap' $'\\\\\\'\\e\\x21' ha\\ lo ",KShell::NoOptions, &err),
             QString("~sulli text jojo crap '\\'\\''\x1b!' 'ha lo'"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("\"~sulli\" 'text'", KShell::TildeExpand, &err),
             QString("~sulli text"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("~\"sulli\" 'text'", KShell::TildeExpand, &err),
             QString("~sulli text"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("~/\"sulli\" 'text'", KShell::TildeExpand, &err),
             QDir::homePath() + "/sulli text");
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("~ 'text' ~", KShell::TildeExpand, &err),
             QDir::homePath() + " text " + QDir::homePath());
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("~sulli ~root", KShell::TildeExpand, &err),
             QString("~sulli ") + KUser("root").homeDir());
    QVERIFY(err == KShell::NoError);
}

void
KShellTest::abortOnMeta()
{
    KShell::Errors err1 = KShell::NoError, err2 = KShell::NoError;

    QCOMPARE(sj("say \" error", KShell::NoOptions, &err1),
             QString());
    QVERIFY(err1 != KShell::NoError);

    QCOMPARE(sj("say \" still error", KShell::AbortOnMeta, &err1),
             QString());
    QVERIFY(err1 != KShell::NoError);

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

QTEST_KDEMAIN_CORE(KShellTest)

#include "kshelltest.moc"
