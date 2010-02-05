/* This file is part of the KDE libraries
    Copyright (c) 2003,2007-2008 Oswald Buddenhagen <ossi@kde.org>
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
    void quoteArg();
    void joinArgs();
    void splitJoin();
    void abortOnMeta();
};

void
KShellTest::tildeExpand()
{
    QString me(KUser().loginName());
    QCOMPARE(KShell::tildeExpand("~"), QDir::homePath());
    QCOMPARE(KShell::tildeExpand("~/dir"), QDir::homePath()+"/dir");
    QCOMPARE(KShell::tildeExpand("~" + me), QDir::homePath());
    QCOMPARE(KShell::tildeExpand("~" + me + "/dir"), QDir::homePath()+"/dir");
#ifdef Q_OS_WIN
    QCOMPARE(KShell::tildeExpand("^~" + me), "~" + me);
#else
    QCOMPARE(KShell::tildeExpand("\\~" + me), "~" + me);
#endif
}

void
KShellTest::quoteArg()
{
#ifdef Q_OS_WIN
    QCOMPARE(KShell::quoteArg("a space"), QString("\"a space\""));
    QCOMPARE(KShell::quoteArg("fds\\\""), QString("fds\\\\\\^\""));
    QCOMPARE(KShell::quoteArg("\\\\foo"), QString("\\\\foo"));
    QCOMPARE(KShell::quoteArg("\"asdf\""), QString("\\^\"asdf\\^\""));
    QCOMPARE(KShell::quoteArg("with\\"), QString("\"with\\\\\""));
    QCOMPARE(KShell::quoteArg("\\\\"), QString("\"\\\\\\\\\""));
    QCOMPARE(KShell::quoteArg("\"a space\\\""), QString("\\^\"\"a space\"\\\\\\^\""));
    QCOMPARE(KShell::quoteArg("as df\\"), QString("\"as df\\\\\""));
    QCOMPARE(KShell::quoteArg("foo bar\"\\\"bla"), QString("\"foo bar\"\\^\"\\\\\\^\"\"bla\""));
    QCOMPARE(KShell::quoteArg("a % space"), QString("\"a %PERCENT_SIGN% space\""));
#else
    QCOMPARE(KShell::quoteArg("a space"), QString("'a space'"));
#endif
}

void
KShellTest::joinArgs()
{
    QStringList list;
    list << "this" << "is" << "a" << "test";
    QCOMPARE(KShell::joinArgs(list), QString("this is a test"));
}

static QString sj(const QString& str, KShell::Options flags, KShell::Errors* ret)
{
    return KShell::joinArgs(KShell::splitArgs(str, flags, ret));
}

void
KShellTest::splitJoin()
{
    KShell::Errors err = KShell::NoError;

#ifdef Q_OS_WIN
    QCOMPARE(sj("\"(sulli)\" text", KShell::NoOptions, &err),
             QString("\"(sulli)\" text"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj(" ha\\ lo ", KShell::NoOptions, &err),
             QString("\"ha\\\\\" lo"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("say \" error", KShell::NoOptions, &err),
             QString());
    QVERIFY(err == KShell::BadQuoting);

    QCOMPARE(sj("no \" error\"", KShell::NoOptions, &err),
             QString("no \" error\""));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("say \" still error", KShell::NoOptions, &err),
             QString());
    QVERIFY(err == KShell::BadQuoting);

    QCOMPARE(sj("BLA;asdf sdfess d", KShell::NoOptions, &err),
             QString("\"BLA;asdf\" sdfess d"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("B\"L\"A&sdf FOO|bar sdf wer ", KShell::NoOptions, &err),
             QString("\"BLA&sdf\" \"FOO|bar\" sdf wer"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("\"\"\"just \"\" fine\"\"\"", KShell::NoOptions, &err),
             QString("\\^\"\"just \"\\^\"\" fine\"\\^\""));
    QVERIFY(err == KShell::NoError);
#else
    QCOMPARE(sj("\"~qU4rK\" 'text' 'jo'\"jo\" $'crap' $'\\\\\\'\\e\\x21' ha\\ lo \\a", KShell::NoOptions, &err),
             QString("'~qU4rK' text jojo crap '\\'\\''\x1b!' 'ha lo' a"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("\"~qU4rK\" 'text'", KShell::TildeExpand, &err),
             QString("'~qU4rK' text"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("~\"qU4rK\" 'text'", KShell::TildeExpand, &err),
             QString("'~qU4rK' text"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("~/\"dir\" 'text'", KShell::TildeExpand, &err),
             QDir::homePath() + "/dir text");
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("~ 'text' ~", KShell::TildeExpand, &err),
             QDir::homePath() + " text " + QDir::homePath());
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("\\~ blah", KShell::TildeExpand, &err),
             QString("'~' blah"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("~qU4rK ~" + KUser().loginName(), KShell::TildeExpand, &err),
             QString("'~qU4rK' ") + QDir::homePath());
    QVERIFY(err == KShell::NoError);
#endif
}

void
KShellTest::abortOnMeta()
{
    KShell::Errors err1 = KShell::NoError, err2 = KShell::NoError;

    QCOMPARE(sj("text", KShell::AbortOnMeta, &err1),
             QString("text"));
    QVERIFY(err1 == KShell::NoError);

#ifdef Q_OS_WIN
    QVERIFY(KShell::splitArgs("BLA & asdf sdfess d", KShell::AbortOnMeta, &err1).isEmpty());
    QVERIFY(err1 == KShell::FoundMeta);

    QVERIFY(KShell::splitArgs("foo %PATH% bar", KShell::AbortOnMeta, &err1).isEmpty());
    QVERIFY(err1 == KShell::FoundMeta);

    QCOMPARE(sj("foo %PERCENT_SIGN% bar", KShell::AbortOnMeta, &err1),
             QString("foo %PERCENT_SIGN% bar"));
    QVERIFY(err1 == KShell::NoError);

    QCOMPARE(sj("@foo ^& bar", KShell::AbortOnMeta, &err1),
             QString("foo \"&\" bar"));
    QVERIFY(err1 == KShell::NoError);

    QCOMPARE(sj("\"BLA|asdf\" sdfess d", KShell::AbortOnMeta, &err1),
             QString("\"BLA|asdf\" sdfess d"));
    QVERIFY(err1 == KShell::NoError);

    QCOMPARE(sj("B\"L\"A\"|\"sdf \"FOO | bar\" sdf wer", KShell::AbortOnMeta, &err1),
             QString("\"BLA|sdf\" \"FOO | bar\" sdf wer"));
    QVERIFY(err1 == KShell::NoError);

    QCOMPARE(sj("b-q me \\\\^|\\\\\\^\"", KShell::AbortOnMeta, &err1),
             QString("b-q me \"\\\\|\"\\\\\\^\""));
    QVERIFY(err1 == KShell::NoError);
#else
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
#endif
}

QTEST_KDEMAIN_CORE(KShellTest)

#include "kshelltest.moc"
