/*
 *  Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
 *  Copyright (C) 2007, 2009 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
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
#include <kstandarddirs.h>
#include <kconfiggroup.h>
#include "kiotesthelper.h" // createTestFile etc.

void KRunUnitTest::initTestCase()
{
    // testProcessDesktopExec works only if your terminal application is set to "x-term"
    KConfigGroup cg(KGlobal::config(), "General");
    cg.writeEntry("TerminalApplication", "x-term");
}

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
static void checkPDE(const char* exec, const char* term, const char* sus,
                     const KUrl::List &urls, bool tf, const QString& b)
{
    QFile out( "kruntest.desktop" );
    if ( !out.open( QIODevice::WriteOnly ) )
        abort();
    QByteArray str ( "[Desktop Entry]\n"
                     "Type=Application\n"
                     "Name=just_a_test\n"
                     "Icon=~/icon.png\n");
    str += QByteArray(exec) + '\n';
    str += QByteArray(term) + '\n';
    str += QByteArray(sus) + '\n';
    out.write( str );
    out.close();

    KService service(QDir::currentPath() + "/kruntest.desktop");
    /*qDebug() << QString().sprintf(
        "processDesktopExec( "
        "service = {\nexec = %s\nterminal = %s, terminalOptions = %s\nsubstituteUid = %s, user = %s },"
        "\nURLs = { %s },\ntemp_files = %s )",
        service.exec().toLatin1().constData(), bt(service.terminal()), service.terminalOptions().toLatin1().constData(), bt(service.substituteUid()), service.username().toLatin1().constData(),
        KShell::joinArgs(urls.toStringList()).toLatin1().constData(), bt(tf));
    */
    QCOMPARE(KShell::joinArgs(KRun::processDesktopExec(service,urls,tf)), b);

    QFile::remove("kruntest.desktop");
}

void KRunUnitTest::testProcessDesktopExec()
{
    QString sh = KStandardDirs::findExe("sh");
    if (sh.isEmpty()) sh = "/bin/sh";

    KUrl::List l0;
    static const char
        * const execs[] = { "Exec=date -u", "Exec=echo $PWD" },
        * const terms[] = { "Terminal=false", "Terminal=true\nTerminalOptions=-T \"%f - %c\"" },
          * const sus[] = { "X-KDE-SubstituteUID=false", "X-KDE-SubstituteUID=true\nX-KDE-Username=sprallo" },
        * const rslts[] = {
            "/bin/date -u", // 0
            "/bin/sh -c 'echo $PWD '", // 1
            "x-term -T ' - just_a_test' -e /bin/date -u", // 2
            "x-term -T ' - just_a_test' -e /bin/sh -c 'echo $PWD '", // 3
            /* kdesu */ " -u sprallo -c '/bin/date -u'", // 4
            /* kdesu */ " -u sprallo -c '/bin/sh -c '\\''echo $PWD '\\'''", // 5
            "x-term -T ' - just_a_test' -e su sprallo -c '/bin/date -u'", // 6
            "x-term -T ' - just_a_test' -e su sprallo -c '/bin/sh -c '\\''echo $PWD '\\'''", // 7
        };
    for (int su = 0; su < 2; su++)
        for (int te = 0; te < 2; te++)
            for (int ex = 0; ex < 2; ex++) {
                int pt = ex+te*2+su*4;
                QString exe;
                if (pt == 4 || pt == 5)
                    exe = KStandardDirs::findExe("kdesu");
                const QString result = QString::fromLatin1(rslts[pt]).replace("/bin/sh", sh);
                checkPDE( execs[ex], terms[te], sus[su], l0, false, exe + result);
            }
}

void KRunUnitTest::testProcessDesktopExecNoFile_data()
{
    QTest::addColumn<QString>("execLine");
    QTest::addColumn<KUrl::List>("urls");
    QTest::addColumn<bool>("tempfiles");
    QTest::addColumn<QString>("expected");

    KUrl::List l0;
    KUrl::List l1; l1 << KUrl( "file:/tmp" );
    KUrl::List l2; l2 << KUrl( "http://localhost/foo" );
    KUrl::List l3; l3 << KUrl( "file:/local/file" ) << KUrl( "http://remotehost.org/bar" );
    KUrl::List l4; l4 << KUrl( "http://login:password@www.kde.org" );

    // A real-world use case would be kate.
    // But I picked kdeinit4 since it's installed by kdelibs
    QString kdeinit = KStandardDirs::findExe("kdeinit4");
    if (kdeinit.isEmpty()) kdeinit = "kdeinit4";

    QString kioexec = KStandardDirs::findExe("kioexec");
    if (kioexec.isEmpty()) kioexec = "kioexec";

    QString kmailservice = KStandardDirs::findExe("kmailservice");
    if (kmailservice.isEmpty()) kmailservice = "kmailservice";
    if (!kdeinit.isEmpty()) {
        QVERIFY(!kmailservice.isEmpty());
        QVERIFY(kmailservice.contains("kde4/libexec"));
    }

    QTest::newRow("%U l0") << "kdeinit4 %U" << l0 << false << kdeinit;
    QTest::newRow("%U l1") << "kdeinit4 %U" << l1 << false << kdeinit + " /tmp";
    QTest::newRow("%U l2") << "kdeinit4 %U" << l2 << false << kdeinit + " http://localhost/foo";
    QTest::newRow("%U l3") << "kdeinit4 %U" << l3 << false << kdeinit + " /local/file http://remotehost.org/bar";

    //QTest::newRow("%u l0") << "kdeinit4 %u" << l0 << false << kdeinit; // gives runtime warning
    QTest::newRow("%u l1") << "kdeinit4 %u" << l1 << false << kdeinit + " /tmp";
    QTest::newRow("%u l2") << "kdeinit4 %u" << l2 << false << kdeinit + " http://localhost/foo";
    //QTest::newRow("%u l3") << "kdeinit4 %u" << l3 << false << kdeinit; // gives runtime warning

    QTest::newRow("%F l0") << "kdeinit4 %F" << l0 << false << kdeinit;
    QTest::newRow("%F l1") << "kdeinit4 %F" << l1 << false << kdeinit + " /tmp";
    QTest::newRow("%F l2") << "kdeinit4 %F" << l2 << false << kioexec + " 'kdeinit4 %F' http://localhost/foo";
    QTest::newRow("%F l3") << "kdeinit4 %F" << l3 << false << kioexec + " 'kdeinit4 %F' file:///local/file http://remotehost.org/bar";

    QTest::newRow("%F l1 tempfile") << "kdeinit4 %F" << l1 << true << kioexec + " --tempfiles 'kdeinit4 %F' file:///tmp";

    QTest::newRow("sh -c kdeinit4 %F") << "sh -c \"kdeinit4 \"'\\\"'\"%F\"'\\\"'"
                                   << l1 << false << "/bin/sh -c 'kdeinit4 \\\"/tmp\\\"'";

    QTest::newRow("kmailservice %u l1") << "kmailservice %u" << l1 << false << kmailservice + " /tmp";
    QTest::newRow("kmailservice %u l4") << "kmailservice %u" << l4 << false << kmailservice + " http://login:password@www.kde.org";
}

void KRunUnitTest::testProcessDesktopExecNoFile()
{
    QFETCH(QString, execLine);
    KService service("dummy", execLine, "app");
    QFETCH(KUrl::List, urls);
    QFETCH(bool, tempfiles);
    QFETCH(QString, expected);
    QCOMPARE(KShell::joinArgs(KRun::processDesktopExec(service,urls,tempfiles)), expected);
}

class KRunImpl : public KRun
{
public:
    KRunImpl(const KUrl& url, bool isLocalFile = false)
        : KRun(url, 0, 0, isLocalFile, false) {}

    virtual void foundMimeType(const QString& type) {
        m_mimeType = type;
        // don't call KRun::foundMimeType, we don't want to start an app ;-)
        setFinished(true);
    }

    QString mimeTypeFound() const { return m_mimeType; }

private:
    QString m_mimeType;
};

void KRunUnitTest::testMimeTypeFile()
{
    const QString filePath = homeTmpDir() + "file";
    createTestFile(filePath, true);
    KRunImpl* krun = new KRunImpl(filePath, true);
    QTest::kWaitForSignal(krun, SIGNAL(finished()), 1000);
    QCOMPARE(krun->mimeTypeFound(), QString::fromLatin1("text/plain"));
}

void KRunUnitTest::testMimeTypeDirectory()
{
    const QString dir = homeTmpDir() + "dir";
    createTestDirectory(dir);
    KRunImpl* krun = new KRunImpl(dir, true);
    QTest::kWaitForSignal(krun, SIGNAL(finished()), 1000);
    QCOMPARE(krun->mimeTypeFound(), QString::fromLatin1("inode/directory"));
}

void KRunUnitTest::testMimeTypeBrokenLink()
{
    const QString dir = homeTmpDir() + "dir";
    createTestDirectory(dir);
    KRunImpl* krun = new KRunImpl(dir + "/testlink", true);
    QSignalSpy spyError(krun, SIGNAL(error()));
    QTest::kWaitForSignal(krun, SIGNAL(finished()), 1000);
    QVERIFY(krun->mimeTypeFound().isEmpty());
    QCOMPARE(spyError.count(), 1);
}

void KRunUnitTest::testMimeTypeDoesNotExist()
{
    KRunImpl* krun = new KRunImpl(KUrl("/does/not/exist"));
    QSignalSpy spyError(krun, SIGNAL(error()));
    QTest::kWaitForSignal(krun, SIGNAL(finished()), 1000);
    QVERIFY(krun->mimeTypeFound().isEmpty());
    QCOMPARE(spyError.count(), 1);
}
