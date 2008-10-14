/* This file is part of the KDE libraries
 *  Copyright 2007 David Faure <faure@kde.org>
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
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <kdebug.h>
#include <ktemporaryfile.h>
#include <qtest_kde.h>
#include "kmimefileparser.h"
#include <kmimetypefactory.h>
#include <ksycoca.h>
#include <kstandarddirs.h>

// We need a factory that returns the same KMimeType::Ptr every time it's asked for a given mimetype.
// Otherwise the calls to addPattern by KMimeFileParser have no effect that we can test from here.
class FakeMimeTypeFactory : public KMimeTypeFactory
{
public:
    FakeMimeTypeFactory() : KMimeTypeFactory() {}

    virtual KMimeType::Ptr findMimeTypeByName(const QString &_name, KMimeType::FindByNameOption options = KMimeType::DontResolveAlias) {
        Q_UNUSED(options); // not useful here - but this means, don't test anything that relies on aliases ;)
        KMimeType::Ptr result = m_cache.value(_name);
        if (!result) {
            result = KMimeTypeFactory::findMimeTypeByName(_name, options);
            m_cache.insert(_name, result);
        }
        return result;
    }
private:
    QMap<QString, KMimeType::Ptr> m_cache;
};

class KMimeFileParserTest : public QObject
{
    Q_OBJECT
public:
    KMimeFileParserTest() : m_factory(0) {}
private:

    FakeMimeTypeFactory* fakeMimeTypeFactory()
    {
        if ( !KSycoca::isAvailable() ) {
            // Create ksycoca4 in ~/.kde-unit-test
            QProcess::execute( KGlobal::dirs()->findExe(KBUILDSYCOCA_EXENAME), QStringList() << "--noincremental" );
        }
        if (!m_factory) {
            // Create factory on the heap and don't delete it.
            // It registers to KSycoca, which deletes it at end of program execution.
            m_factory = new FakeMimeTypeFactory;
        }
        return m_factory;
    }

private Q_SLOTS:
    void initTestCase()
    {
    }

    void testParseGlobsFile()
    {
        const QString ext1 = "*.kmimefileparserunittest";
        const QString ext2 = "*.kmimefileparserunittest2";

        QByteArray testFile = "# Test data\ntext/plain:*.kmimefileparserunittest\ntext/plain:*.kmimefileparserunittest2";
        QBuffer buf(&testFile);
        const QHash<QString, KMimeFileParser::GlobList> mimeTypeGlobs =
            KMimeFileParser::parseGlobFile(&buf, KMimeFileParser::OldGlobs);
        QCOMPARE(mimeTypeGlobs.count(), 1);
        QVERIFY(mimeTypeGlobs.contains("text/plain"));
        const KMimeFileParser::GlobList textGlobs = mimeTypeGlobs.value("text/plain");
        QCOMPARE(textGlobs[0].pattern, ext1);
        QCOMPARE(textGlobs[0].weight, 50);
        QCOMPARE(textGlobs[1].pattern, ext2);
        QCOMPARE(textGlobs[1].weight, 50);
    }

    void testParseGlobs2File()
    {
        const QString ext1 = "*.kmimefileparserunittest";
        const QString ext2 = "*.kmimefileparserunittest2";

        QByteArray testFile = "# Test data\n40:text/plain:*.kmimefileparserunittest\n20:text/plain:*.kmimefileparserunittest2";
        QBuffer buf(&testFile);
        const QHash<QString, KMimeFileParser::GlobList> mimeTypeGlobs =
            KMimeFileParser::parseGlobFile(&buf, KMimeFileParser::Globs2WithWeight);
        //kDebug() << mimeTypeGlobs.keys();
        QCOMPARE(mimeTypeGlobs.count(), 1);
        QVERIFY(mimeTypeGlobs.contains("text/plain"));
        const KMimeFileParser::GlobList textGlobs = mimeTypeGlobs.value("text/plain");
        QCOMPARE(textGlobs[0].pattern, ext1);
        QCOMPARE(textGlobs[0].weight, 40);
        QCOMPARE(textGlobs[1].pattern, ext2);
        QCOMPARE(textGlobs[1].weight, 20);
    }

    void testParseGlobs()
    {
        KMimeFileParser parser(fakeMimeTypeFactory());

        const QString ext1 = "*.kmimefileparserunittest";
        const QString ext2 = "*.kmimefileparserunittest2";

        KMimeType::Ptr textPlain = KMimeType::mimeType("text/plain");
        QVERIFY(textPlain);
        QVERIFY(!textPlain->patterns().contains(ext1));
        QVERIFY(!textPlain->patterns().contains(ext2));

        KTemporaryFile globTempFile;
        QVERIFY(globTempFile.open());
        const QByteArray testFile = "# Test data\ntext/plain:*.kmimefileparserunittest\ntext/plain:*.kmimefileparserunittest2";
        globTempFile.write(testFile);
        const QString fileName = globTempFile.fileName();
        globTempFile.close();

        parser.parseGlobs(QStringList() << fileName);

        QVERIFY(textPlain->patterns().contains(ext1));
        QVERIFY(textPlain->patterns().contains(ext2));
    }

    void testDuplicateGlob()
    {
        KMimeFileParser parser(fakeMimeTypeFactory());

        const QString ext1 = "*.ext1";
        const QString ext2 = "*.ext2";

        KMimeType::Ptr textPlain = KMimeType::mimeType("text/plain");
        QVERIFY(textPlain);
        QVERIFY(!textPlain->patterns().contains(ext1));
        QVERIFY(!textPlain->patterns().contains(ext2));

        // "local" file
        // It defines *.ext1 and *.ext2
        KTemporaryFile globTempFile1;
        QVERIFY(globTempFile1.open());
        // Defining ext2 twice is a bonus in this test: it tests the case where
        // people install freedesktop.org.xml and kde.xml into the same prefix;
        // we shouldn't end up with *.txt twice in the text/plain patterns.
        const QByteArray testFile1 = "# Test data\ntext/plain:*.ext1\ntext/plain:*.ext2\ntext/plain:*.ext2";
        globTempFile1.write(testFile1);
        const QString fileName1 = globTempFile1.fileName();
        globTempFile1.close();

        // "global" file
        // It defines *.ext1 and *.exttoberemoved - this will all be overwritten by the local file
        // so it won't appear.
        KTemporaryFile globTempFile2;
        QVERIFY(globTempFile2.open());
        const QByteArray testFile2 = "# Test data\ntext/plain:*.ext1\ntext/plain:*.exttoberemoved";
        globTempFile2.write(testFile2);
        const QString fileName2 = globTempFile2.fileName();
        globTempFile2.close();

        parser.parseGlobs(QStringList() << fileName1 << fileName2);

        QCOMPARE(textPlain->patterns().count(ext1), 1);
        QCOMPARE(textPlain->patterns().count(ext2), 1);
        QVERIFY(!textPlain->patterns().contains("*.exttoberemoved"));
    }

private:
    FakeMimeTypeFactory* m_factory;
};

QTEST_KDEMAIN( KMimeFileParserTest, NoGUI )

#include "kmimefileparsertest.moc"
