/*
 *  Copyright (C) 2005-2009 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License or ( at
 *  your option ) version 3 or, at the discretion of KDE e.V. ( which shall
 *  act as a proxy as in section 14 of the GPLv3 ), any later version.
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

#include "kmimetypetest.h"

#include <kdefakes.h>
#include <kde_file.h>
#include <kdeversion.h> // KDE_MAKE_VERSION
#include <kmimetype.h>
#include <ksycoca.h>
#include <kuser.h>
#include <qtemporarydir.h>
#include <kconfiggroup.h>
#include <kdebug.h>

#include <qtest_kde.h> // WARNING: do not port to qtest.h without adding a putenv for XDG_DATA_HOME! User data loss will occur otherwise.
#include <qstandardpaths.h>
#include <qprocess.h>
#include <kmimetypetrader.h>
#include <kservicetypetrader.h>
#include <kmimetyperepository_p.h>
#include <qtemporaryfile.h>
#include <kdesktopfile.h>

#include <QtConcurrent>

int initializeLang()
{
    qputenv("LC_ALL", "en_US");
    qputenv("LANG", "en_US");
    return 0;
}

// Set LANG before QCoreApplication is created
Q_CONSTRUCTOR_FUNCTION(initializeLang)

void KMimeTypeTest::initTestCase()
{
    // Clean up local xdg dir in case of leftover mimetype definitions
    const QString xdgDir = QString::fromLocal8Bit(getenv("XDG_DATA_HOME"));
    if (!xdgDir.isEmpty()) {
        QDir d_(xdgDir);
        d_.removeRecursively();
        // No need to run update-mime-database here, the dir is entirely gone.
    }

    bool mustUpdateKSycoca = false;

    // Create fake text/x-patch part.
    const QString fakePatchPart = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kde5/services/") + "fakepatchpart.desktop";
    const bool mustCreatePatchPart = !QFile::exists(fakePatchPart);
    if (mustCreatePatchPart) {
        mustUpdateKSycoca = true;
        KDesktopFile file(fakePatchPart);
        KConfigGroup group = file.desktopGroup();
        group.writeEntry("Name", "FakePatchPart");
        group.writeEntry("Type", "Service");
        group.writeEntry("X-KDE-Library", "fakepatchpart");
        group.writeEntry("ServiceTypes", "KParts/ReadOnlyPart");
        group.writeEntry("MimeType", "text/x-diff;"); // Use an alias on purpose, to test if that works
        group.writeEntry("InitialPreference", 5);
    }

    // Create fake text/plain part with a higher initial preference than the patch part.
    const QString fakePart = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kde5/services/") + "faketextpart.desktop";
    const bool mustCreate = !QFile::exists(fakePart);
    if (mustCreate) {
        mustUpdateKSycoca = true;
        KDesktopFile file(fakePart);
        KConfigGroup group = file.desktopGroup();
        group.writeEntry("Name", "FakePart");
        group.writeEntry("Type", "Service");
        group.writeEntry("X-KDE-Library", "faketextpart");
        group.writeEntry("ServiceTypes", "KParts/ReadOnlyPart");
        group.writeEntry("MimeType", "text/plain;");
        group.writeEntry("InitialPreference",100);
    }

    // Create fake text/plain ktexteditor plugin.
    const QString fakePlugin = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kde5/services/") + "faketextplugin.desktop";
    const bool mustCreatePlugin = !QFile::exists(fakePlugin);
    if (mustCreatePlugin) {
        mustUpdateKSycoca = true;
        KDesktopFile file(fakePlugin);
        KConfigGroup group = file.desktopGroup();
        group.writeEntry("Name", "FakePlugin");
        group.writeEntry("Type", "Service");
        group.writeEntry("X-KDE-Library", "faketextplugin");
        group.writeEntry("ServiceTypes", "KPluginInfo");
        group.writeEntry("MimeType", "text/plain;");
    }

    // Create fake "NotShowIn=KDE" service
    m_nonKdeApp = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + QLatin1Char('/') + "fake_nonkde_application.desktop";
    const bool mustCreateNonKdeApp = !QFile::exists(m_nonKdeApp);
    if (mustCreateNonKdeApp) {
        mustUpdateKSycoca = true;
        KDesktopFile file(m_nonKdeApp);
        KConfigGroup group = file.desktopGroup();
        group.writeEntry("Name", "NonKDEApp");
        group.writeEntry("Type", "Application");
        group.writeEntry("Exec", "xterm");
        group.writeEntry("NotShowIn", "KDE;FVWM;");
        group.writeEntry("MimeType", "text/plain;");
        group.writeEntry("InitialPreference", "50");
        group.writeEntry("Categories", "Qt;KDE;");
    }

    // Create fake text/plain app
    m_textPlainApp = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + QLatin1Char('/') + "fake_textplain_application.desktop";
    const bool mustCreateTextPlainApp = !QFile::exists(m_textPlainApp);
    if (mustCreateTextPlainApp) {
        mustUpdateKSycoca = true;
        KDesktopFile file(m_textPlainApp);
        KConfigGroup group = file.desktopGroup();
        group.writeEntry("Name", "NonKDEApp");
        group.writeEntry("Type", "Application");
        group.writeEntry("Exec", "xterm");
        group.writeEntry("MimeType", "text/plain;");
        group.writeEntry("InitialPreference", "40");
        group.writeEntry("Categories", "Qt;KDE;");
    }

    if ( mustUpdateKSycoca ) {
        // Update ksycoca in ~/.kde-unit-test after creating the above
        QProcess::execute(QStandardPaths::findExecutable(KBUILDSYCOCA_EXENAME));
    }

    QVERIFY(KService::serviceByStorageId("fake_nonkde_application.desktop"));
    QVERIFY(KService::serviceByDesktopPath(m_nonKdeApp)); // the desktoppath is the full path nowadays
    QVERIFY(KService::serviceByStorageId("fake_textplain_application.desktop"));
    QVERIFY(KService::serviceByDesktopPath(m_textPlainApp));
}

void KMimeTypeTest::cleanupTestCase()
{
    // If I want the konqueror unit tests to work, then I better not have a non-working part
    // as the preferred part for text/plain...
    const QString fakePatchPart = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kde5/services/") + "fakepatchpart.desktop";
    QFile::remove(fakePatchPart);
    const QString fakePart = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kde5/services/") + "faketextpart.desktop";
    QFile::remove(fakePart);
    const QString fakePlugin = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kde5/services/") + "faketextplugin.desktop";
    QFile::remove(fakePlugin);
    QFile::remove(m_textPlainApp);
    QFile::remove(m_nonKdeApp);
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels); // silence kbuildsycoca output
    proc.start(QStandardPaths::findExecutable(KBUILDSYCOCA_EXENAME));
    proc.waitForFinished();
}

QTEST_KDEMAIN_CORE( KMimeTypeTest )

void KMimeTypeTest::testByName()
{
    KMimeType::Ptr s0 = KMimeType::mimeType("application/x-zerosize");
    QVERIFY( s0 );
    QCOMPARE( s0->name(), QString::fromLatin1("application/x-zerosize") );
    QCOMPARE( s0->comment(), QString::fromLatin1("empty document") );

    KMimeType::Ptr s0Again = KMimeType::mimeType("application/x-zerosize");
    QCOMPARE(s0Again->name(), s0->name());
    QVERIFY(s0Again != s0);

    KMimeType::Ptr s1 = KMimeType::mimeType("text/plain");
    QVERIFY( s1 );
    QCOMPARE( s1->name(), QString::fromLatin1("text/plain") );
    //qDebug("Comment is %s", qPrintable(s1->comment()) );

    KMimeType::Ptr krita = KMimeType::mimeType("application/x-krita");
    QVERIFY( krita );

    // Test <comment> parsing with application/rdf+xml which has the english comment after the other ones
    KMimeType::Ptr rdf = KMimeType::mimeType("application/rdf+xml");
    QVERIFY(rdf);
    QCOMPARE(rdf->comment(), QString::fromLatin1("RDF file"));

    KMimeType::Ptr bzip2 = KMimeType::mimeType("application/x-bzip2");
    QVERIFY( bzip2 );
    QCOMPARE(bzip2->comment(), QString::fromLatin1("Bzip archive"));

    KMimeType::Ptr defaultMime = KMimeType::mimeType("application/octet-stream");
    QVERIFY(defaultMime);
    QVERIFY(defaultMime->isDefault());
}

void KMimeTypeTest::testIcons()
{
    if ( !KUser().isSuperUser() ) // Can't test this one if running as root
    {
        QString emptyString; // gcc-3.3 workaround
	QTemporaryDir tmp (emptyString);
	QFile(tmp.path()).setPermissions(0);
        tmp.setAutoRemove( true );
        //KUrl url( tmp.path() );
        //QCOMPARE(KIO::iconNameForUrl(url), "inode-directory"); // was folder_locked, but we don't have that anymore - TODO
	QFile(tmp.path()).setPermissions(QFile::ReadOwner|QFile::ExeOwner); // so we can 'rm -rf' it
    }
}


void KMimeTypeTest::testFindByPathUsingFileName_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QString>("expectedMimeType");
    // Maybe we could also add a expectedAccuracy column...

    QTest::newRow("text") << "textfile.txt" << "text/plain";
    QTest::newRow("case-insensitive search") << "textfile.TxT" << "text/plain";
    // With QMime, this needs shared-mime-info > 0.91. Earlier versions wrote .Z to the mime.cache file...
    if (KMimeType::sharedMimeInfoVersion() > KDE_MAKE_VERSION(0, 91, 0)) {
        QTest::newRow("case-insensitive match on a non-lowercase glob") << "foo.z" << "application/x-compress";
    }

    QTest::newRow("case-sensitive uppercase match") << "textfile.C" << "text/x-c++src";
    QTest::newRow("case-sensitive lowercase match") << "textfile.c" << "text/x-csrc";
    QTest::newRow("case-sensitive long-extension match") << "foo.PS.gz" << "application/x-gzpostscript";
    QTest::newRow("case-sensitive-only match") << "core" << "application/x-core";
    QTest::newRow("case-sensitive-only match") << "Core" << "application/octet-stream"; // #198477

    QTest::newRow("desktop file") << "foo.desktop" << "application/x-desktop";
    QTest::newRow("old kdelnk file is x-desktop too") << "foo.kdelnk" << "application/x-desktop";
    QTest::newRow("double-extension file") << "foo.tar.bz2" << "application/x-bzip-compressed-tar";
    QTest::newRow("single-extension file") << "foo.bz2" << "application/x-bzip";
    QTest::newRow(".doc should assume msword") << "somefile.doc" << "application/msword"; // #204139
    QTest::newRow("glob that uses [] syntax, 1") << "Makefile" << "text/x-makefile";
    QTest::newRow("glob that uses [] syntax, 2") << "makefile" << "text/x-makefile";
    QTest::newRow("glob that ends with *, no extension") << "README" << "text/x-readme";
    QTest::newRow("glob that ends with *, extension") << "README.foo" << "text/x-readme";
    QTest::newRow("glob that ends with *, also matches *.txt. Higher weight wins.") << "README.txt" << "text/plain";
    QTest::newRow("glob that ends with *, also matches *.nfo. Higher weight wins.") << "README.nfo" << "text/x-nfo";
    // fdo bug 15436, needs shared-mime-info >= 0.40 (and this tests the globs2-parsing code).
    QTest::newRow("glob that ends with *, also matches *.pdf. *.pdf has higher weight") << "README.pdf" << "application/pdf";
    QTest::newRow("directory") << "/" << "inode/directory";
    QTest::newRow("doesn't exist, no extension") << "IDontExist" << "application/octet-stream";
    QTest::newRow("doesn't exist but has known extension") << "IDontExist.txt" << "text/plain";

    // Can't use KIconLoader since this is a "without GUI" test.
    if (QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("icons/") + "oxygen", QStandardPaths::LocateDirectory).isEmpty()) {
        kWarning() << "oxygen not found";
    } else {
        QString fh = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("icons/") + "oxygen/22x22/places/folder.png" );
        QVERIFY( !fh.isEmpty() ); // if the file doesn't exist, please fix the above to point to an existing icon
        QTest::newRow("png image") << fh << "image/png";
    }

    const QString exePath = QStandardPaths::findExecutable("cmake");
    QVERIFY2(!exePath.isEmpty(), "cmake not found. Isn't it in your $PATH?");
#ifdef Q_OS_WIN
    const QString executableType = QString::fromLatin1("application/x-ms-dos-executable");
#else
    const QString executableType = QString::fromLatin1("application/x-executable");
#endif
    QTest::newRow("executable") << exePath << executableType;
}

void KMimeTypeTest::testFindByPathUsingFileName()
{
    QFETCH(QString, fileName);
    QFETCH(QString, expectedMimeType);
    KMimeType::Ptr mime = KMimeType::findByPath(fileName);
    QVERIFY( mime );
    QCOMPARE(mime->name(), expectedMimeType);

}

void KMimeTypeTest::testAdditionalGlobs_data()
{
    // Other globs that are not in shared-mime-info but which users could define themselves.
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<bool>("expected");

    QTest::newRow("one star, match") << "foo.txt" << "*.txt" << true;
    QTest::newRow("README*, match") << "README.foo" << "README*" << true;
    QTest::newRow("README.*, match") << "README.foo" << "README.*" << true;
    QTest::newRow("README.*, no match") << "README" << "README.*" << false;
    QTest::newRow("two stars, match") << "andre.ts.001" << "*.ts.0*" << true;
    QTest::newRow("two stars, no match") << "andre.ts" << "*.ts.0*" << false;
}

void KMimeTypeTest::testAdditionalGlobs()
{
    QFETCH(QString, filename);
    QFETCH(QString, pattern);
    QFETCH(bool, expected);

    QCOMPARE(KMimeTypeRepository::matchFileName(filename, pattern), expected);
}

// All the simple tests for findByPath are in testFindByPathUsingFileName_data.
// In here we do the tests that need some content in a temporary file.
void KMimeTypeTest::testFindByPathWithContent()
{
    KMimeType::Ptr mime;

    // Test a real PDF file.
    // If we find x-matlab because it starts with '%' then we are not ordering by priority.
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString tempFileName = tempFile.fileName();
    tempFile.write("%PDF-");
    tempFile.close();
    mime = KMimeType::findByPath( tempFileName );
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1( "application/pdf" ) );
    // fast mode cannot find the mimetype
    mime = KMimeType::findByPath( tempFileName, 0, true );
    QVERIFY( mime );
    QCOMPARE(mime->name(), QString::fromLatin1("application/octet-stream"));

    // Test the case where the extension doesn't match the contents: extension wins
    {
        QTemporaryFile txtTempFile(QDir::tempPath() + QLatin1String("/kmimetypetest_XXXXXX.txt"));
        QVERIFY(txtTempFile.open());
        txtTempFile.write("%PDF-");
        QString txtTempFileName = txtTempFile.fileName();
        txtTempFile.close();
        mime = KMimeType::findByPath( txtTempFileName );
        QVERIFY( mime );
        QCOMPARE( mime->name(), QString::fromLatin1( "text/plain" ) );
        // fast mode finds the same
        mime = KMimeType::findByPath( txtTempFileName, 0, true );
        QVERIFY( mime );
        QCOMPARE( mime->name(), QString::fromLatin1( "text/plain" ) );
    }

    // Now the case where extension differs from contents, but contents has >80 magic rule
    // XDG spec used to say: contents wins. But we can't sniff all files...
    // XDG spec has now been amended, extensions always win.
    {
        QTemporaryFile txtTempFile(QDir::tempPath() + QLatin1String("/kmimetypetest_XXXXXX.txt"));
        QVERIFY(txtTempFile.open());
        txtTempFile.write("<smil");
        QString txtTempFileName = txtTempFile.fileName();
        txtTempFile.close();
        mime = KMimeType::findByPath( txtTempFileName );
        QVERIFY( mime );
        QCOMPARE( mime->name(), QString::fromLatin1( "text/plain" ) );
    }
}

void KMimeTypeTest::testFindByUrl()
{
    // Tests with local files are already done in testFindByPath,
    // here we test for remote urls only.
    KMimeType::Ptr mime;
    mime = KMimeType::findByUrl( KUrl("http://foo/bar.png") );
    QVERIFY( mime );

    QCOMPARE( mime->name(), QString::fromLatin1( "application/octet-stream" ) ); // HTTP can't know before downloading

    mime = KMimeType::findByUrl(KUrl("http://foo/s0/"));
    QCOMPARE( mime->name(), QString::fromLatin1( "application/octet-stream" ) ); // HTTP can't know before downloading

#if 0 // no such logic in QMimeType, we get default mimetype, KRun will figure it out
    if ( !KProtocolInfo::isKnownProtocol(KUrl("man:/")) )
        QSKIP( "man protocol not installed" );

    mime = KMimeType::findByUrl( KUrl("man:/ls") );
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("text/html") );

    mime = KMimeType::findByUrl( KUrl("man:/ls/") );
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("text/html") );
#endif

    mime = KMimeType::findByUrl(KUrl("fish://host/test1")); // like fish does, to test for known extensions
    QVERIFY(mime);
    QCOMPARE(mime->name(), QString::fromLatin1("application/octet-stream"));
}

void KMimeTypeTest::testFindByNameAndContent()
{
    KMimeType::Ptr mime;

    QByteArray textData = "Hello world";
    // textfile -> text/plain. No extension -> mimetype is found from the contents.
    mime = KMimeType::findByNameAndContent("textfile", textData);
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("text/plain") );

    // textfile.foo -> text/plain. Unknown extension -> mimetype is found from the contents.
    mime = KMimeType::findByNameAndContent("textfile.foo", textData);
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("text/plain") );

    // textfile.doc -> text/plain. We added this to the mimetype database so that it can be handled.
    mime = KMimeType::findByNameAndContent("textfile.doc", textData);
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("text/plain") );

    // mswordfile.doc -> application/msword. Found by contents, because of the above case.
    // Note that it's application/msword, not application/vnd.ms-word, since it's the former that is registered to IANA.
    QByteArray mswordData = "\320\317\021\340\241\261\032\341";
    mime = KMimeType::findByNameAndContent("mswordfile.doc", mswordData);
    QVERIFY( mime );
    if (mime->name() == "application/vnd.ms-word") { // this comes from /usr/share/mime/packages/libreoffice.xml....
        QEXPECT_FAIL("", "libreoffice.xml is messing with us", Continue);
    }
    // If you get powerpoint instead, then you're hit by https://bugs.freedesktop.org/show_bug.cgi?id=435 - upgrade to shared-mime-info >= 0.22
    QCOMPARE( mime->name(), QString::fromLatin1("application/msword") );

    // excelfile.xls -> application/vnd.ms-excel. Found by extension.
    mime = KMimeType::findByNameAndContent("excelfile.xls", mswordData /*same magic*/);
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("application/vnd.ms-excel") );

    // textfile.xls -> application/vnd.ms-excel. Found by extension. User shouldn't rename a text file to .xls ;)
    mime = KMimeType::findByNameAndContent("textfile.xls", textData);
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("application/vnd.ms-excel") );

#if 0   // needs shared-mime-info >= 0.20
    QByteArray tnefData = "\x78\x9f\x3e\x22";
    mime = KMimeType::findByNameAndContent("tneffile", mswordData);
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("application/vnd.ms-tnef") );
#endif

    QByteArray pdfData = "%PDF-";
    mime = KMimeType::findByNameAndContent("foo", pdfData);
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("application/pdf") );

    // High-priority rule (80)
    QByteArray phpData = "<?php";
    mime = KMimeType::findByNameAndContent("foo", phpData);
    QVERIFY( mime );
    QCOMPARE( mime->name(), QString::fromLatin1("application/x-php") );
}

void KMimeTypeTest::testFindByContent_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QString>("expectedMimeType");
    QTest::newRow("simple text") << QByteArray("Hello world") << "text/plain";
    QTest::newRow("html: <html>") << QByteArray("<html>foo</html>") << "text/html";

    // fixed in smi-0.30, xml magic has prio 40
    QTest::newRow("html: comment+<html>") << QByteArray("<!--foo--><html>foo</html>") << "text/html";
    // https://bugs.freedesktop.org/show_bug.cgi?id=11259, fixed in smi-0.22
    QTest::newRow("html: <script>") << QByteArray("<script>foo</script>") << "text/html";

    QTest::newRow("pdf") << QByteArray("%PDF-") << "application/pdf";
    QTest::newRow("no mimetype known") << QByteArray("\261\032\341\265") << "application/octet-stream";

    QByteArray mswordData = "\320\317\021\340\241\261\032\341";
    // same as \xD0\xCF\x11\xE0 \xA1\xB1\x1A\xE1
    QVERIFY(KMimeType::isBufferBinaryData(mswordData));
    // We have no magic specific to msword data, so finding x-ole-storage is correct.
    // If you get powerpoint instead, then you're hit by https://bugs.freedesktop.org/show_bug.cgi?id=435 - upgrade to shared-mime-info >= 0.22
    QTest::newRow("msword") << mswordData << "application/x-ole-storage";
}

void KMimeTypeTest::testFindByContent()
{
    QFETCH(QByteArray, data);
    QFETCH(QString, expectedMimeType);

    KMimeType::Ptr mime = KMimeType::findByContent(data);
    QVERIFY( mime );
    QCOMPARE( mime->name(), expectedMimeType );
}

void KMimeTypeTest::testFindByFileContent()
{
    KMimeType::Ptr mime;
    int accuracy = 0;

    // Calling findByFileContent on a directory
    mime = KMimeType::findByFileContent("/", &accuracy);
    QVERIFY(mime);
    QCOMPARE(mime->name(), QString::fromLatin1("inode/directory"));
    QCOMPARE(accuracy, 100);

    // Albert calls findByFileContent with a URL instead of a path and gets 11021 as accuracy :)
    // It was not set inside findByFileContent -> fixed.
    mime = KMimeType::findByFileContent("file:///etc/passwd" /*bad example code, use a path instead*/, &accuracy);
    QVERIFY(mime);
    QCOMPARE(mime->name(), QString::fromLatin1("application/octet-stream"));
    QCOMPARE(accuracy, 0);
}

void KMimeTypeTest::testAllMimeTypes()
{
    const KMimeType::List lst = KMimeType::allMimeTypes(); // does NOT include aliases
    QVERIFY( !lst.isEmpty() );

    for ( KMimeType::List::ConstIterator it = lst.begin();
          it != lst.end(); ++it ) {
        const KMimeType::Ptr mime = (*it);
        const QString name = mime->name();
        //qDebug( "%s", qPrintable( name ) );
        QVERIFY( !name.isEmpty() );
        QCOMPARE( name.count( '/' ), 1 );

        const KMimeType::Ptr lookedupMime = KMimeType::mimeType( name );
        QVERIFY( lookedupMime ); // not null
        if (name != "application/vnd.ms-word" && name != "application/x-pkcs7-certificates"
                && name != "application/x-x509-ca-cert"
                && name != "application/x-vnd.kde.kexi" // due to /usr/share/mime/packages/kde.xml from KDE4
                && name != "application/x-kexiproject-sqlite" // due to /usr/share/mime/packages/kde.xml from KDE4
                ) {
            QCOMPARE( lookedupMime->name(), name );
            // if this fails, you have an alias defined as a real mimetype too!
            //
            // Note: this also happens with x-win-lnk when your kde.xml defines it as an alias, while
            // /usr/share/mime/packages/kde.xml defines it as a real mimetype. This is a false positive,
            // remove one of the kde.xml files.
            //
            // It also happens with application/x-pkcs7-certificates due to
            // /usr/share/mime/packages/gcr-crypto-types.xml. Remove that file and run
            // `update-mime-database /usr/share/mime`.
        }
    }
}

void KMimeTypeTest::testAlias()
{
    const KMimeType::Ptr canonical = KMimeType::mimeType( "application/xml" );
    QVERIFY( canonical );
    KMimeType::Ptr alias = KMimeType::mimeType( "text/xml" );
    QVERIFY( alias );
    QCOMPARE( alias->name(), QString("application/xml") );

    QVERIFY(alias->is("application/xml"));
    QVERIFY(canonical->is("text/xml"));

    // Test for bug 197346: does nspluginscan see that audio/mp3 already exists?
    bool mustWriteMimeType = KMimeType::mimeType("audio/mp3").isNull();
    QVERIFY(!mustWriteMimeType);
}

void KMimeTypeTest::testMimeTypeParent()
{
    // All file-like mimetypes inherit from octet-stream
    const KMimeType::Ptr wordperfect = KMimeType::mimeType("application/vnd.wordperfect");
    QVERIFY(wordperfect);
    QCOMPARE(wordperfect->parentMimeTypes().join(","), QString("application/octet-stream"));
    QVERIFY(wordperfect->is("application/octet-stream"));

    QVERIFY(KMimeType::mimeType("image/svg+xml-compressed")->is("application/x-gzip"));

    // Check that msword derives from ole-storage [it didn't in 0.20, but we added it to kde.xml]
    const KMimeType::Ptr msword = KMimeType::mimeType("application/msword");
    QVERIFY(msword);
    const KMimeType::Ptr olestorage = KMimeType::mimeType("application/x-ole-storage");
    QVERIFY(olestorage);
    QVERIFY(msword->is(olestorage->name()));
    QVERIFY(msword->is("application/octet-stream"));

    const KMimeType::Ptr directory = KMimeType::mimeType("inode/directory");
    QVERIFY(directory);
    QCOMPARE(directory->parentMimeTypes().count(), 0);
    QVERIFY(!directory->is("application/octet-stream"));

    // Check that text/x-patch knows that it inherits from text/plain (it says so explicitly)
    const KMimeType::Ptr plain = KMimeType::mimeType( "text/plain" );
    const KMimeType::Ptr derived = KMimeType::mimeType( "text/x-patch" );
    QVERIFY( derived );
    QCOMPARE( derived->parentMimeTypes().join(","), plain->name() );
    QVERIFY( derived->is("text/plain") );
    QVERIFY( derived->is("application/octet-stream") );

    // Check that application/x-shellscript inherits from application/x-executable
    // (Otherwise KRun cannot start shellscripts...)
    // This is a test for multiple inheritance...
    const KMimeType::Ptr shellscript = KMimeType::mimeType("application/x-shellscript");
    QVERIFY(shellscript);
    QVERIFY(shellscript->is("text/plain"));
    QVERIFY(shellscript->is("application/x-executable"));
    const QStringList shellParents = shellscript->parentMimeTypes();
    QVERIFY(shellParents.contains("text/plain"));
    QVERIFY(shellParents.contains("application/x-executable"));
    QCOMPARE(shellParents.count(), 2); // only the above two
    const QStringList allShellParents = shellscript->allParentMimeTypes();
    QVERIFY(allShellParents.contains("text/plain"));
    QVERIFY(allShellParents.contains("application/x-executable"));
    QVERIFY(allShellParents.contains("application/octet-stream"));
    // Must be least-specific last, i.e. breadth first.
    QCOMPARE(allShellParents.last(), QString("application/octet-stream"));

    // Check that text/x-mrml knows that it inherits from text/plain (implicitly)
    const KMimeType::Ptr mrml = KMimeType::mimeType("text/x-mrml");
    if (!mrml)
        QSKIP("kdelibs not installed");
    QVERIFY(mrml->is("text/plain"));
    QVERIFY(mrml->is("application/octet-stream"));
}

void KMimeTypeTest::testMimeTypeInheritancePerformance()
{
    // Check performance of is(). In kde3 the list of mimetypes with previews had 63 items...
    // We could get it with KServiceTypeTrader::self()->query("ThumbCreator") and the "MimeTypes"
    // property, but this would give variable results and requires other modules installed.
    QStringList mimeTypes; mimeTypes << "image/jpeg" << "image/png" << "image/tiff" << "text/plain" << "text/html";
    mimeTypes += mimeTypes;
    mimeTypes += mimeTypes;
    mimeTypes += mimeTypes;
    QCOMPARE(mimeTypes.count(), 40);
    KMimeType::Ptr mime = KMimeType::mimeType("text/x-chdr");
    QVERIFY(mime);
    QTime dt; dt.start();
    QBENCHMARK {
        QString match;
        foreach (const QString& mt, mimeTypes) {
            if (mime->is(mt)) {
                match = mt;
                // of course there would normally be a "break" here, but we're testing worse-case
                // performance here
            }
        }
        QCOMPARE(match, QString("text/plain"));
    }
    // Results on David's machine (April 2009):
    // With the KMimeType::is() code that loaded every parent KMimeType:
    // 3.5 msec / 7,000,000 ticks / 5,021,498 instr. loads per iteration
    // After the QHash for parent mimetypes in ksycoca, removing the need to load full mimetypes:
    // 0.57 msec / 1,115,000 ticks / 938,356 instr. loads per iteration
    // After converting the QMap for aliases into a QHash too:
    // 0.48 msec / 960,000 ticks / 791,404 instr. loads per iteration
    // July 2010: After moving KMimeType out of ksycoca:
    // 0.21 msec / 494,000 ticks / 568,345 instr. loads per iteration
}

// Helper method for all the trader tests
static bool offerListHasService( const KService::List& offers,
                                 const QString& entryPath )
{
    bool found = false;
    KService::List::const_iterator it = offers.begin();
    for ( ; it != offers.end() ; ++it )
    {
        if ( (*it)->entryPath() == entryPath ) {
            if( found ) { // should be there only once
                qWarning( "ERROR: %s was found twice in the list", qPrintable( entryPath ) );
                return false; // make test fail
            }
            found = true;
        }
    }
    return found;
}

void KMimeTypeTest::testMimeTypeTraderForTextPlain()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available" );

    // Querying mimetype trader for services associated with text/plain
    KService::List offers = KMimeTypeTrader::self()->query("text/plain", "KParts/ReadOnlyPart");
    QVERIFY(!offerListHasService(offers, "fakepatchpart.desktop"));
    QVERIFY(offerListHasService(offers, "faketextpart.desktop"));

    offers = KMimeTypeTrader::self()->query("text/plain", "KPluginInfo");
    QVERIFY( offers.count() > 0 );

    // We should have at least the fake text plugin that we created for this.
    // (The actual plugins from kdelibs don't mention text/plain anymore)
    QVERIFY( offerListHasService( offers, "faketextplugin.desktop" ) );

    // We shouldn't have non-plugins
    QVERIFY( !offerListHasService( offers, "fakepatchpart.desktop" ) );
    QVERIFY( !offerListHasService( offers, "faketextpart.desktop" ) );
}

void KMimeTypeTest::testMimeTypeTraderForDerivedMimeType()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available" );

    // Querying mimetype trader for services associated with text/x-patch, which inherits from text/plain
    KService::List offers = KMimeTypeTrader::self()->query("text/x-patch", "KParts/ReadOnlyPart");
    QVERIFY( offerListHasService( offers, "fakepatchpart.desktop" ) );
    QVERIFY( offerListHasService( offers, "faketextpart.desktop" ) );
    QVERIFY( (*offers.begin())->entryPath() != "faketextpart.desktop" ); // in the list, but not preferred

    offers = KMimeTypeTrader::self()->query("text/x-patch", "KPluginInfo");
    QVERIFY( offers.count() > 0 );

    // We should have at least the fake text plugin that we created for this.
    // (The actual plugins from kdelibs don't mention text/plain anymore)
    QVERIFY( offerListHasService( offers, "faketextplugin.desktop" ) );

    offers = KMimeTypeTrader::self()->query("text/x-patch", "Application");
    QVERIFY( !offerListHasService( offers, "faketextpart.desktop" ) );

    // We shouldn't have non-kde apps
    Q_FOREACH( KService::Ptr service, offers )
        kDebug() << service->name() << service->entryPath();

    QVERIFY( !offerListHasService( offers, m_nonKdeApp ) );
}

void KMimeTypeTest::testPreferredService()
{
    // The "NotShowIn=KDE" service should not be the preferred one!
    KService::Ptr serv = KMimeTypeTrader::self()->preferredService("text/plain");
    QVERIFY( serv );
    qDebug() << serv->entryPath();
    QVERIFY( serv->entryPath() != m_nonKdeApp );
    QCOMPARE(serv->entryPath(), m_textPlainApp);
}

void KMimeTypeTest::testMimeTypeTraderForAlias()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available" );

    const KService::List referenceOffers = KMimeTypeTrader::self()->query("application/xml", "KParts/ReadOnlyPart");
    QVERIFY(offerListHasService(referenceOffers, "faketextpart.desktop"));
    QVERIFY(!offerListHasService(referenceOffers, "fakepatchpart.desktop"));

    // Querying mimetype trader for services associated with text/xml, which is an alias for application/xml
    const KService::List offers = KMimeTypeTrader::self()->query("text/xml", "KParts/ReadOnlyPart");
    QVERIFY(offerListHasService(offers, "faketextpart.desktop"));
    QVERIFY(!offerListHasService(offers, "fakepatchpart.desktop"));

    QCOMPARE(offers.count(), referenceOffers.count());
}

void KMimeTypeTest::testHasServiceType1() // with services constructed with a full path (rare)
{
    QString faketextpartPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("/kde5/services/") + "faketextpart.desktop" );
    QVERIFY( !faketextpartPath.isEmpty() );
    KService faketextpart( faketextpartPath );
    QVERIFY( faketextpart.hasMimeType( "text/plain" ) );
    QVERIFY(!faketextpart.hasMimeType("text/x-patch")); // inherited mimetype; fails
    QVERIFY( !faketextpart.hasMimeType( "image/png" ) );
    QVERIFY( faketextpart.hasServiceType( "KParts/ReadOnlyPart" ) );
    QVERIFY( !faketextpart.hasServiceType( "KParts/ReadWritePart" ) );
    QVERIFY( !faketextpart.hasServiceType( "KPluginInfo" ) );

    QString textPluginPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("/kde5/services/") + "faketextplugin.desktop" );
    QVERIFY( !textPluginPath.isEmpty() );
    KService textPlugin( textPluginPath );
    QVERIFY( textPlugin.hasServiceType( "KPluginInfo" ) );
    QVERIFY( !textPlugin.hasServiceType( "KParts/ReadOnlyPart" ) );
}

void KMimeTypeTest::testHasServiceType2() // with services coming from ksycoca
{
    KService::Ptr faketextpart = KService::serviceByDesktopPath( "faketextpart.desktop" );
    QVERIFY( !faketextpart.isNull() );
    QVERIFY( faketextpart->hasMimeType( "text/plain" ) );
    QVERIFY( faketextpart->hasMimeType( "text/x-patch" ) ); // due to inheritance
    QVERIFY( !faketextpart->hasMimeType( "image/png" ) );
    QVERIFY( faketextpart->hasServiceType( "KParts/ReadOnlyPart" ) );
    QVERIFY( !faketextpart->hasServiceType( "KParts/ReadWritePart" ) );
    QVERIFY( !faketextpart->hasServiceType( "KPluginInfo" ) );

    KService::Ptr textPlugin= KService::serviceByDesktopPath( "faketextplugin.desktop" );
    QVERIFY( !textPlugin.isNull() );
    QVERIFY( textPlugin->hasServiceType( "KPluginInfo" ) );
    QVERIFY( !textPlugin->hasServiceType( "KParts/ReadOnlyPart" ) );
}

void KMimeTypeTest::testPatterns_data()
{
    QTest::addColumn<QString>("mimeType");
    QTest::addColumn<QString>("patterns");
    QTest::addColumn<QString>("mainExtension");
    QTest::newRow("mimetype with a single pattern") << "application/pdf" << "*.pdf" << ".pdf";
    QTest::newRow("mimetype with multiple patterns") << "application/x-kpresenter" << "*.kpr;*.kpt" << ".kpr";
    if (KMimeType::sharedMimeInfoVersion() > KDE_MAKE_VERSION(0, 60, 0)) {
        QTest::newRow("mimetype with many patterns") << "application/vnd.wordperfect" << "*.wp;*.wp4;*.wp5;*.wp6;*.wpd;*.wpp" << ".wp";
    }
    QTest::newRow("oasis text mimetype") << "application/vnd.oasis.opendocument.text" << "*.odt" << ".odt";
    QTest::newRow("oasis presentation mimetype") << "application/vnd.oasis.opendocument.presentation" << "*.odp" << ".odp";
    QTest::newRow("mimetype with multiple patterns, *.doc added by kde") << "text/plain" << "*.asc;*.txt;*.doc;*,v" << ".txt";
    QTest::newRow("mimetype with uncommon pattern") << "application/x-kcachegrind" << "callgrind.out*;cachegrind.out*" << QString();
    QTest::newRow("mimetype with no patterns") << "application/x-ole-storage" << QString() << QString();
}

void KMimeTypeTest::testPatterns()
{
    QFETCH(QString, mimeType);
    QFETCH(QString, patterns);
    QFETCH(QString, mainExtension);
    KMimeType::Ptr mime = KMimeType::mimeType( mimeType );
    QVERIFY(mime);
    // Sort both lists; order is unreliable since shared-mime-info uses hashes internally.
    QStringList expectedPatterns = patterns.split(';');
    expectedPatterns.sort();
    QStringList mimePatterns = mime->patterns();

    if (mimeType == "application/vnd.oasis.opendocument.text" && mimePatterns.contains("*.fodt")) {
        QSKIP("Skipping test which would fail due to an upstream bug, see https://bugs.freedesktop.org/show_bug.cgi?id=31242");
    }

    if (mimeType == "application/vnd.oasis.opendocument.presentation" && mimePatterns.contains("*.fodp")) {
        QSKIP("Skipping test which would fail due to an upstream bug, see https://bugs.freedesktop.org/show_bug.cgi?id=31242");
    }

    // shared-mime-info 0.30 adds *,v to text/plain, let's add it from this test so that it works
    // with older versions too.
    if (mimeType == "text/plain" && !mimePatterns.contains("*,v"))
        mimePatterns.append("*,v");
    mimePatterns.sort();
    QCOMPARE(mimePatterns.join(";"), expectedPatterns.join(";"));

    QCOMPARE(mime->mainExtension(), mainExtension);
}

void KMimeTypeTest::testExtractKnownExtension_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QString>("extension");
    QTest::newRow("simple extension") << "foo.pdf" << "pdf";
    QTest::newRow("filename has two extensions, last one matches") << "kpresenter.foo.kpt" << "kpt";
    QTest::newRow("filename has two extensions, pattern for both exist") << "foo.tar.bz2" << "tar.bz2";
    QTest::newRow("bz2 alone works too") << "foo.bz2" << "bz2";
}

void KMimeTypeTest::testExtractKnownExtension()
{
    QFETCH(QString, fileName);
    QFETCH(QString, extension);
    QCOMPARE(KMimeType::extractKnownExtension(fileName), extension);
}

struct LessMimeType_ByComment
{
    bool operator()(const KMimeType::Ptr& lhs, const KMimeType::Ptr& rhs) const
    {
        return lhs->comment() < rhs->comment();
    }
};

void KMimeTypeTest::testSortByComment()
{
    QBENCHMARK {
        KMimeType::List sortedList = KMimeType::allMimeTypes();
        qSort( sortedList.begin(), sortedList.end(), LessMimeType_ByComment() );
    }
}

void KMimeTypeTest::testFromThread()
{
    // Some simple tests to test more API from testThreads without using _data()
    KMimeType::Ptr mime = KMimeType::mimeType("application/pdf");
    QVERIFY(mime);
    QCOMPARE(mime->mainExtension(), QString::fromLatin1(".pdf"));
}

void KMimeTypeTest::testThreads()
{
    QThreadPool::globalInstance()->setMaxThreadCount(20);
    // Note that data-based tests cannot be used here (QTest::fetchData asserts).
    QList<QFuture<void> > futures;
    futures << QtConcurrent::run(this, &KMimeTypeTest::testFindByUrl);
    futures << QtConcurrent::run(this, &KMimeTypeTest::testFindByFileContent);
    futures << QtConcurrent::run(this, &KMimeTypeTest::testFindByNameAndContent);
    futures << QtConcurrent::run(this, &KMimeTypeTest::testFindByPathWithContent);
    futures << QtConcurrent::run(this, &KMimeTypeTest::testAllMimeTypes);
    futures << QtConcurrent::run(this, &KMimeTypeTest::testAlias);
    futures << QtConcurrent::run(this, &KMimeTypeTest::testMimeTypeParent);
    futures << QtConcurrent::run(this, &KMimeTypeTest::testPreferredService);
    futures << QtConcurrent::run(this, &KMimeTypeTest::testFromThread);
    kDebug() << "Joining all threads";
    Q_FOREACH(QFuture<void> f, futures) // krazy:exclude=foreach
        f.waitForFinished();
}

#include "kmimetypetest.moc"
