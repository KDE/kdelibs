/*
 *  Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
 *  Copyright (C) 2007 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "krununittest.h"
#include "krununittest.moc"
#include <qtest_kde.h>
QTEST_KDEMAIN( KRunUnitTest, NoGUI )

#include "krun.h"
#include <kshell.h>
#include <kservice.h>

void KRunUnitTest::testBinaryName_data()
{
    QTest::addColumn<QString>("execLine");
    QTest::addColumn<bool>("removePath");
    QTest::addColumn<QString>("expected");

    QTest::newRow("/usr/bin/ls true") << "/usr/bin/ls" << true << "ls";
    QTest::newRow("/usr/bin/ls false") << "/usr/bin/ls" << false << "/usr/bin/ls";
    QTest::newRow("/path/to/wine \"long argument with path\"") << "/path/to/wine \"long argument with path\"" << true << "wine";
    QTest::newRow("/path/with/a/sp\\ ace/exe arg1 arg2") << "/path/with/a/sp\\ ace/exe arg1 arg2" << true << "exe";
    QTest::newRow("\"progname\" \"arg1\"") << "\"progname\" \"arg1\"" << true << "progname";
    QTest::newRow("'quoted' \"arg1\"") << "'quoted' \"arg1\"" << true << "quoted";
    QTest::newRow(" 'leading space'   arg1") << " 'leading space'   arg1" << true << "leading space";
}

void KRunUnitTest::testBinaryName()
{
    QFETCH(QString, execLine);
    QFETCH(bool, removePath);
    QFETCH(QString, expected);
    QCOMPARE(KRun::binaryName(execLine, removePath), expected);
}

//static const char *bt(bool tr) { return tr?"true":"false"; }
static void checkPDE(const KService &service, const KUrl::List &urls, bool tf, const QString& b)
{
    /*qDebug() << QString().sprintf(
        "processDesktopExec( "
        "service = {\nexec = %s\nterminal = %s, terminalOptions = %s\nsubstituteUid = %s, user = %s },"
        "\nURLs = { %s },\ntemp_files = %s )",
        service.exec().toLatin1().constData(), bt(service.terminal()), service.terminalOptions().toLatin1().constData(), bt(service.substituteUid()), service.username().toLatin1().constData(),
        KShell::joinArgs(urls.toStringList()).toLatin1().constData(), bt(tf));
    */
    QCOMPARE(KShell::joinArgs(KRun::processDesktopExec(service,urls,tf)), b);
}

void KRunUnitTest::testProcessDesktopExec()
{

    // ------ this test works only if your terminal application is set to "x-term" ------
    KConfigGroup cg(KGlobal::config(), "General");
    cg.writeEntry("TerminalApplication", "x-term");

    KUrl::List l0;
    KUrl::List l1; l1 << KUrl( "file:/tmp" );
    KUrl::List l2; l2 << KUrl( "http://localhost/foo" );
    KUrl::List l3; l3 << KUrl( "file:/local/file" ) << KUrl( "http://remotehost.org/bar" );

    static const char
        *execs[] = { "Exec=date -u", "Exec=echo $$PWD" },
        *terms[] = { "Terminal=false", "Terminal=true\nTerminalOptions=-T \"%f - %c\"" },
          *sus[] = { "X-KDE-SubstituteUID=false", "X-KDE-SubstituteUID=true\nX-KDE-Username=sprallo" },
        *rslts[] = {
            "date -u", // 0
            "/bin/sh -c 'echo $PWD '", // 1
            "x-term -T ' - just_a_test' -e date -u", // 2
            "x-term -T ' - just_a_test' -e /bin/sh -c 'echo $PWD '", // 3
            "kdesu -u sprallo -c 'date -u '", // 4
            "kdesu -u sprallo -c '/bin/sh -c '\\''echo $PWD '\\'''", // 5
            "x-term -T ' - just_a_test' -e su sprallo -c 'date -u '", // 6
            "x-term -T ' - just_a_test' -e su sprallo -c '/bin/sh -c '\\''echo $PWD '\\'''", // 7
            "'date -u '", // 8
            "'echo $PWD '", // 9
            "'x-term -T \" - just_a_test\"' -e 'date -u '", // a
            "'x-term -T \" - just_a_test\"' -e '/bin/sh -c '\\''echo $PWD '\\'''", // b
            "kdesu -u sprallo -c ''\\''date -u '\\'''", // c
            "kdesu -u sprallo -c ''\\''/bin/sh -c '\\''\\'\\'''\\''echo $PWD '\\''\\'\\'''\\'''\\'''", // d
            "'x-term -T \" - just_a_test\"' -e su sprallo -c ''\\''date -u '\\'''", // e
            "'x-term -T \" - just_a_test\"' -e su sprallo -c ''\\''/bin/sh -c '\\''\\'\\'''\\''echo $PWD '\\''\\'\\'''\\'''\\'''", // f
        };
    for (int su = 0; su < 2; su++)
        for (int te = 0; te < 2; te++)
            for (int ex = 0; ex < 2; ex++) {
                QFile out( "kruntest.desktop" );
                if ( !out.open( IO_WriteOnly ) )
                    abort();
                QByteArray str = "[Desktop Entry]\n"
                                 "Type=Application\n"
                                 "Name=just_a_test\n"
                                 "Icon=~/icon.png\n";
                str += QByteArray(execs[ex]) + '\n';
                str += QByteArray(terms[te]) + '\n';
                str += QByteArray(sus[su]) + '\n';
                out.write( str );
                out.close();
                KService s(QDir::currentPath() + "/kruntest.desktop");
                //::unlink("kruntest.desktop");
                checkPDE( s, l0, false, rslts[ex+te*2+su*4]);
            }

    KService s1("dummy", "kate %U", "app");
    checkPDE( s1, l0, false, "kate"); // gives runtime warning
    checkPDE( s1, l1, false, "kate /tmp");
    checkPDE( s1, l2, false, "kate http://localhost/foo");
    checkPDE( s1, l3, false, "kate /local/file http://remotehost.org/bar");
    KService s2("dummy", "kate %u", "app");
     //checkPDE( s2, l0, false, "kate"); // gives runtime warning
    checkPDE( s2, l1, false, "kate /tmp");
    checkPDE( s2, l2, false, "kate http://localhost/foo");
    //checkPDE( s2, l3, false, "kate"); // gives runtime warning
    KService s3("dummy", "kate %F", "app");
    checkPDE( s3, l0, false, "kate");
    checkPDE( s3, l1, false, "kate /tmp");
    checkPDE( s3, l2, false, "kioexec 'kate %F' http://localhost/foo");
    checkPDE( s3, l3, false, "kioexec 'kate %F' file:///local/file http://remotehost.org/bar");

    checkPDE( s3, l1, true, "kioexec --tempfiles 'kate %F' file:///tmp");

    KService s4("dummy", "sh -c \"kate \"'\\\"'\"%F\"'\\\"'", "app");
    checkPDE( s4, l1, false, "sh -c 'kate \\\"/tmp\\\"'");
}

