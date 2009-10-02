/* This file is part of the KDE libraries
 *  Copyright 2007, 2009 David Faure <faure@kde.org>
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
        QHash<QString, KMimeFileParser::GlobList> mimeTypeGlobs;
        QVERIFY(KMimeFileParser::parseGlobFile(&buf, KMimeFileParser::OldGlobs, mimeTypeGlobs));
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

        QByteArray testFile = "# Test data\n"
                              "40:text/plain:*.kmimefileparserunittest:cs\n"
                              "40:text/plain:*.kmimefileparserunittest\n"
                              "20:text/plain:*.kmimefileparserunittest2::futureextension";
        QBuffer buf(&testFile);
        QHash<QString, KMimeFileParser::GlobList> mimeTypeGlobs;
        QVERIFY(KMimeFileParser::parseGlobFile(&buf, KMimeFileParser::Globs2WithWeight, mimeTypeGlobs));
        //kDebug() << mimeTypeGlobs.keys();
        QCOMPARE(mimeTypeGlobs.count(), 1);
        QVERIFY(mimeTypeGlobs.contains("text/plain"));
        const KMimeFileParser::GlobList textGlobs = mimeTypeGlobs.value("text/plain");
        QCOMPARE(textGlobs.count(), 2);
        QCOMPARE(textGlobs[0].pattern, ext1);
        QCOMPARE(textGlobs[0].weight, 40);
        QCOMPARE(textGlobs[0].flags, (int)KMimeTypeFactory::CaseSensitive);
        QCOMPARE(textGlobs[1].pattern, ext2);
        QCOMPARE(textGlobs[1].weight, 20);
        QCOMPARE(textGlobs[1].flags, 0);
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

        KMimeFileParser::ParsedPatternMap parsedPatterns;
        parsedPatterns.insert("text/plain", "*.txt");
        parser.setParsedPatternMap(parsedPatterns);
        parser.parseGlobs(QStringList() << fileName);

        QVERIFY(textPlain->patterns().contains(ext1));
        QVERIFY(textPlain->patterns().contains(ext2));
    }

    void testMerging() // global+local merging
    {
        KMimeFileParser parser(fakeMimeTypeFactory());

        const QString ext1 = "*.ext1";
        const QString ext2 = "*.ext2";
        const QString globalext = "*.globalext";

        KMimeType::Ptr textPlain = KMimeType::mimeType("text/plain");
        QVERIFY(textPlain);
        QVERIFY(!textPlain->patterns().contains(ext1));
        QVERIFY(!textPlain->patterns().contains(ext2));
        QVERIFY(!textPlain->patterns().contains(globalext));

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

        KMimeFileParser::ParsedPatternMap parsedPatterns;
        parsedPatterns.insert("text/plain", "*.txt");
        parser.setParsedPatternMap(parsedPatterns);
        parser.parseGlobs(QStringList() << fileName1 << fileName2);

        QCOMPARE(textPlain->patterns().count(ext1), 1);
        QCOMPARE(textPlain->patterns().count(ext2), 1);
        QCOMPARE(textPlain->patterns().count(globalext), 1);
    }

    void testOverriding() // local overrides global when using __NOGLOBS__
    {
        KMimeFileParser parser(fakeMimeTypeFactory());

        const QString ext1 = "*.o1";
        const QString ext2 = "*.o2";

        KMimeType::Ptr textPlain = KMimeType::mimeType("text/plain");
        QVERIFY(textPlain);
        QVERIFY(!textPlain->patterns().contains(ext1));
        QVERIFY(!textPlain->patterns().contains(ext2));

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

        KMimeFileParser::ParsedPatternMap parsedPatterns;
        parsedPatterns.insert("text/plain", "*.txt");
        parser.setParsedPatternMap(parsedPatterns);
        parser.parseGlobs(QStringList() << fileName1 << fileName2);

        QCOMPARE(textPlain->patterns().count(ext1), 1);
        QCOMPARE(textPlain->patterns().count(ext2), 1);
        QVERIFY(!textPlain->patterns().contains("*.exttoberemoved"));
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
        // DF: For repeat=100, on AMD Athlon(tm) 64 X2 Dual Core Processor 3800+, I get:
        // (mimetype: time with ksycoca, time with linear search)
        // text: 7, 10
        // case-insensitive: 11, 34
        // README*: 9, 9
        // doesn't exist: 6, 44
        // random other: 6, 10
        // tar.bz2: 9, 9

        // Prepare m_globList
        QVERIFY(m_globList.isEmpty());
        const QStringList globFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", "globs");
        QStringList parsedFiles;
        KMimeFileParser::AllGlobs allGlobs = KMimeFileParser::parseGlobFiles(globFiles, parsedFiles);
        // Turn into a linear list, faster than calling QHash::value() for each mime during matching
        const QStringList allMimes = allGlobs.uniqueKeys();
        Q_FOREACH(const QString& mime, allMimes) {
            const KMimeFileParser::GlobList globs = allGlobs.value(mime);
            KMimeType::Ptr mimeptr = KMimeType::mimeType(mime, KMimeType::ResolveAliases);
            if (!mimeptr) {
                kWarning() << "Unknown mimetype" << mime;
                continue;
            }
            int offset = mimeptr->offset();
            Q_FOREACH(const KMimeFileParser::Glob& glob, globs) {
                const QString adjustedPattern = (glob.flags & KMimeTypeFactory::CaseSensitive) ? glob.pattern : glob.pattern.toLower();
                m_globList.append(KMimeTypeFactory::OtherPattern(adjustedPattern,
                                                                 offset,
                                                                 glob.weight,
                                                                 glob.flags));
            }
        }
        kDebug() << allMimes.count() << "mimetypes," << m_globList.count() << "patterns";
    }

    void testGlobMatchingPerformance()
    {
        QFETCH(QString, fileName);
        QFETCH(QString, expectedMimeType);

        // Is the fast pattern dict worth it? (answer: yes)
        //QTime dt; dt.start();

        QBENCHMARK {

        // To investigate the difference if sycoca is slower than linear search,
        // comment out one block, callgrind, comment out the other block, callgrind, and compare.
#if 1
        //for (int i = 0; i < repeat; ++i) {
            QList<KMimeType::Ptr> mimeList = KMimeTypeFactory::self()->findFromFileName(fileName);
            QString mime = mimeList.isEmpty() ? QString() : mimeList.first()->name();
            QCOMPARE(mime, expectedMimeType);
        //}
        //qDebug() << "Lookup using ksycoca:" << dt.elapsed();
#endif
        }

#if 1
        QBENCHMARK {
        //dt.start();
        //for (int i = 0; i < repeat; ++i) {
            //bool verbose = i == 0;
            bool verbose = false;
            QString mime = matchGlobHelper(m_globList, fileName, verbose);
            if (mime.isEmpty()) {
                const QString lowerCase = fileName.toLower();
                mime = matchGlobHelper(m_globList, lowerCase, verbose);
            }
            QCOMPARE(mime, expectedMimeType);
        //}
        //qDebug() << "Lookup using linear search:" << dt.elapsed();
        }
#endif

    }

private:
    QString matchGlobHelper(const KMimeTypeFactory::OtherPatternList& globs, const QString& fileName, bool verbose)
    {
        const QString lowerCaseFileName = fileName.toLower();

        QString mime;
        bool found = false;
        int numPatterns = 0;
        Q_FOREACH(const KMimeTypeFactory::OtherPattern& glob, globs) {
            ++numPatterns;
            if (KMimeTypeFactory::matchFileName((glob.flags & KMimeTypeFactory::CaseSensitive) ? fileName : lowerCaseFileName, glob.pattern)) {
                found = true;
                KMimeType* ptr = KMimeTypeFactory::self()->createEntry(glob.offset);
                Q_ASSERT(ptr);
                mime = ptr->name();
                delete ptr;
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
    FakeMimeTypeFactory* m_factory;
    KMimeTypeFactory::OtherPatternList m_globList;
};

QTEST_KDEMAIN( KMimeFileParserTest, NoGUI )

#include "kmimefileparsertest.moc"
