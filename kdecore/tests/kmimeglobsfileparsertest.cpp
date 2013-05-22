/* This file is part of the KDE libraries
 *  Copyright 2007, 2009, 2010 David Faure <faure@kde.org>
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
#include "kmimeglobsfileparser_p.h"
#include <kmimetyperepository_p.h>
#include <ksycoca.h>
#include <kstandarddirs.h>

class KMimeFileParserTest : public QObject
{
    Q_OBJECT
public:
    KMimeFileParserTest() {}
private:

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
        KMimeGlobsFileParser::AllGlobs mimeTypeGlobs;
        QVERIFY(KMimeGlobsFileParser::parseGlobFile(&buf, KMimeGlobsFileParser::OldGlobs, mimeTypeGlobs));
        KMimeGlobsFileParser::PatternsMap patMap = mimeTypeGlobs.patternsMap();
        QCOMPARE(patMap.count(), 1);
        QVERIFY(patMap.contains("text/plain"));
        const QHash<QString, QStringList> fastPatterns = mimeTypeGlobs.m_fastPatterns;
        QCOMPARE(fastPatterns.count(), 2);
        QCOMPARE(fastPatterns["kmimefileparserunittest"], QStringList() << QString("text/plain"));
        QCOMPARE(fastPatterns["kmimefileparserunittest2"], QStringList() << QString("text/plain"));
    }

    void testParseGlobs2File()
    {
        const QString ext1 = "*.kmimefileparserunittest";
        const QString ext2 = "*.kmimefileparserunittest2";

        QByteArray testFile = "# Test data\n"
                              "40:text/plain:*.kmimefileparserunittest:cs\n"
                              "40:text/plain:*.kmimefileparserunittest\n"
                              "20:text/plain:*.kmimefileparserunittest2::futureextension";
        QBuffer buf(&testFile);
        KMimeGlobsFileParser::AllGlobs mimeTypeGlobs;
        QVERIFY(KMimeGlobsFileParser::parseGlobFile(&buf, KMimeGlobsFileParser::Globs2WithWeight, mimeTypeGlobs));
        //kDebug() << mimeTypeGlobs.keys();
        const KMimeGlobsFileParser::GlobList textGlobs = mimeTypeGlobs.m_lowWeightGlobs;
        QCOMPARE(textGlobs.count(), 2);
        QCOMPARE(textGlobs[0].pattern, ext1);
        QCOMPARE(textGlobs[0].mimeType, QString("text/plain"));
        QCOMPARE(textGlobs[0].weight, 40);
        QCOMPARE(textGlobs[0].flags, (int)KMimeTypeRepository::CaseSensitive);
        QCOMPARE(textGlobs[1].pattern, ext2);
        QCOMPARE(textGlobs[1].weight, 20);
        QCOMPARE(textGlobs[1].flags, 0);
        QCOMPARE(textGlobs[1].mimeType, QString("text/plain"));
    }

    void testParseGlobs()
    {
        KMimeGlobsFileParser parser;

        const QString ext1 = "*.kmimefileparserunittest";
        const QString ext2 = "*.kmimefileparserunittest2";

        KTemporaryFile globTempFile;
        QVERIFY(globTempFile.open());
        const QByteArray testFile = "# Test data\ntext/plain:*.kmimefileparserunittest\ntext/plain:*.kmimefileparserunittest2";
        globTempFile.write(testFile);
        const QString fileName = globTempFile.fileName();
        globTempFile.close();

        KMimeGlobsFileParser::AllGlobs globs = parser.parseGlobs(QStringList() << fileName);

        const QStringList textPlainPatterns = globs.patternsMap().value("text/plain");
        QVERIFY(textPlainPatterns.contains(ext1));
        QVERIFY(textPlainPatterns.contains(ext2));
    }

    void testMerging() // global+local merging
    {
        KMimeGlobsFileParser parser;

        const QString ext1 = "*.ext1";
        const QString ext2 = "*.ext2";
        const QString globalext = "*.globalext";

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
        // It defines *.ext1 and *.globalext
        KTemporaryFile globTempFile2;
        QVERIFY(globTempFile2.open());
        const QByteArray testFile2 = "# Test data\ntext/plain:*.ext1\ntext/plain:*.globalext";
        globTempFile2.write(testFile2);
        const QString fileName2 = globTempFile2.fileName();
        globTempFile2.close();

        KMimeGlobsFileParser::AllGlobs globs = parser.parseGlobs(QStringList() << fileName1 << fileName2);

        const QStringList textPlainPatterns = globs.patternsMap().value("text/plain");
        kDebug() << textPlainPatterns;
        QCOMPARE(textPlainPatterns.count(ext1), 1);
        QCOMPARE(textPlainPatterns.count(ext2), 1);
        QCOMPARE(textPlainPatterns.count(globalext), 1);
    }

    void testOverriding() // local overrides global when using __NOGLOBS__
    {
        KMimeGlobsFileParser parser;

        const QString ext1 = "*.o1";
        const QString ext2 = "*.o2";

        // "local" file
        // It defines *.o1 and *.o2
        KTemporaryFile globTempFile1;
        QVERIFY(globTempFile1.open());
        const QByteArray testFile1 = "# Test data\ntext/plain:__NOGLOBS__\ntext/plain:*.o1\ntext/plain:*.o2\ntext/plain:*.o2";
        globTempFile1.write(testFile1);
        const QString fileName1 = globTempFile1.fileName();
        globTempFile1.close();

        // "global" file
        // It defines *.o1 and *.exttoberemoved - this will all be overwritten by the local file
        // so it won't appear.
        KTemporaryFile globTempFile2;
        QVERIFY(globTempFile2.open());
        const QByteArray testFile2 = "# Test data\ntext/plain:*.o1\ntext/plain:*.exttoberemoved";
        globTempFile2.write(testFile2);
        const QString fileName2 = globTempFile2.fileName();
        globTempFile2.close();

        KMimeGlobsFileParser::AllGlobs globs = parser.parseGlobs(QStringList() << fileName1 << fileName2);

        const QStringList textPlainPatterns = globs.patternsMap().value("text/plain");
        kDebug() << textPlainPatterns;
        QCOMPARE(textPlainPatterns.count(ext1), 1);
        QCOMPARE(textPlainPatterns.count(ext2), 1);
        QVERIFY(!textPlainPatterns.contains("*.exttoberemoved"));
    }

    void testGlobMatchingPerformance_data()
    {
        QTest::addColumn<QString>("fileName");
        QTest::addColumn<QString>("expectedMimeType");

        QTest::newRow("text") << "textfile.txt" << "text/plain";
        QTest::newRow("case-insensitive search") << "textfile.TxT" << "text/plain";
        QTest::newRow("glob that ends with *, extension") << "README.foo" << "text/x-readme";
        QTest::newRow("doesn't exist, no extension") << "IDontExist" << "";
        QTest::newRow("random other case") << "foo.wvc" << "audio/x-wavpack-correction";
        QTest::newRow("the .tar.bz2 case; can't stop on short match") << "foo.tar.bz2" << "application/x-bzip-compressed-tar";

        // Current results (using -callgrind) :
        // (mimetype: time with ksycoca (4.4 branch), time with linear search)
        //                    4.4   4.5  linear
        // text:             122K,  75K,  323K
        // case-insensitive: 124K,  76K,  324K
        // README*:          109K,  74K,  235K
        // doesn't exist:     69K,  70K, 1280K [!]
        // random other:     116K,  67K,  254K
        // tar.bz2:          160K,  76K,  214K

        // Prepare m_allGlobs
        const QStringList globFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", "globs");
        QStringList parsedFiles;
        m_allGlobs = KMimeGlobsFileParser::parseGlobFiles(globFiles, parsedFiles);
        m_patternsMap = m_allGlobs.patternsMap();
        const int patCount = m_allGlobs.m_fastPatterns.count() + m_allGlobs.m_highWeightGlobs.count() + m_allGlobs.m_lowWeightGlobs.count();
        kDebug() << m_patternsMap.count() << "mimetypes," << patCount << "patterns";
    }

    void testGlobMatchingPerformance()
    {
        QFETCH(QString, fileName);
        QFETCH(QString, expectedMimeType);

        // Is the fast pattern dict worth it? (answer: yes)

        QBENCHMARK {

        // To investigate the difference if the current impl is slower than linear search,
        // comment out one block, callgrind, comment out the other block, callgrind, and compare.
#if 1
            QStringList mimeList = KMimeTypeRepository::self()->findFromFileName(fileName);
            QString mime = mimeList.isEmpty() ? QString() : mimeList.first();
            QCOMPARE(mime, expectedMimeType);
#endif
        }

#if 0
        QBENCHMARK {
            bool verbose = false;
            QString mime = matchGlobHelper(fileName, verbose);
            if (mime.isEmpty()) {
                const QString lowerCase = fileName.toLower();
                mime = matchGlobHelper(lowerCase, verbose);
            }
            QCOMPARE(mime, expectedMimeType);
        }
#endif

    }

private:
    // Implementation of linear search, for comparison
    QString matchGlobHelper(const QString& fileName, bool verbose) const
    {
        const QString lowerCaseFileName = fileName.toLower();

        QString mime;
        bool found = false;
        int numPatterns = 0;
        KMimeGlobsFileParser::PatternsMap::const_iterator it = m_patternsMap.begin();
        const KMimeGlobsFileParser::PatternsMap::const_iterator end = m_patternsMap.end();
        for ( ; it != end; ++it ) {
            ++numPatterns;
            Q_FOREACH(const QString& pattern, it.value()) {
                if (KMimeTypeRepository::matchFileName(/*(glob.flags & KMimeTypeRepository::CaseSensitive) ? fileName :*/ lowerCaseFileName, pattern)) {
                    found = true;
                    mime = it.key();
                }
            }
            if (found)
                break;
        }
        Q_UNUSED(verbose);
        //if (verbose)
        //    kDebug() << "Tested" << numPatterns << "patterns, found" << mime;
        return mime;
    }

private:
    KMimeGlobsFileParser::AllGlobs m_allGlobs;
    KMimeGlobsFileParser::PatternsMap m_patternsMap;
};

QTEST_KDEMAIN_CORE( KMimeFileParserTest )

#include "kmimefileparsertest.moc"
