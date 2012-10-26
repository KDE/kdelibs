/*
 *  Copyright (C) 2004 David Faure <faure@kde.org>
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

#include <QtTest/QtTest>
#include <qapplication.h>
#include <kurlcompletion.h>
#include <kdebug.h>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <qtemporarydir.h>
#include <kcmdlineargs.h>
#include <unistd.h>

class KUrlCompletionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();

public:
    KUrlCompletionTest() {}
    ~KUrlCompletionTest() { teardown(); }
    void setup();
    void teardown();
    void testLocalRelativePath();
    void testLocalAbsolutePath();
    void testLocalURL();
    void testEmptyCwd();

private:
    void waitForCompletion();
    KUrlCompletion* m_completion;
    QTemporaryDir* m_tempDir;
    QUrl m_dirURL;
    QString m_dir;
    KUrlCompletion* m_completionEmptyCwd;
};

void KUrlCompletionTest::setup()
{
    kDebug() ;
    m_completion = new KUrlCompletion;
    m_tempDir = new QTemporaryDir;
    m_dir = m_tempDir->path();
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    // The port to QUrl breaks handling of '#' with Qt4. That's ok, it works with Qt5.
    m_dir += "/Dir With Spaces/";
#else
    m_dir += "/Dir With#Spaces/";
#endif
    QDir().mkdir(m_dir);
    kDebug() << "m_dir=" << m_dir;
    m_completion->setDir(QUrl::fromLocalFile(m_dir));
    m_dirURL = QUrl::fromLocalFile(m_dir);

    QFile f1( m_dir + "/file1" );
    bool ok = f1.open( QIODevice::WriteOnly );
    QVERIFY( ok );
    f1.close();

    QFile f2( m_dir + "/file#a" );
    ok = f2.open( QIODevice::WriteOnly );
    QVERIFY( ok );
    f2.close();

    QDir().mkdir( m_dir + "/file_subdir" );

    m_completionEmptyCwd = new KUrlCompletion;
    m_completionEmptyCwd->setDir(QUrl());
}

void KUrlCompletionTest::teardown()
{
    delete m_completion;
    m_completion = 0;
    delete m_tempDir;
    m_tempDir = 0;
    delete m_completionEmptyCwd;
    m_completionEmptyCwd = 0;
}
void KUrlCompletionTest::waitForCompletion()
{
    while ( m_completion->isRunning() ) {
        kDebug() << "waiting for thread...";
        usleep( 10 );
    }
}

void KUrlCompletionTest::testLocalRelativePath()
{
    kDebug() ;
    // Completion from relative path, with two matches
    m_completion->makeCompletion( "f" );
    waitForCompletion();
    QStringList comp1all = m_completion->allMatches();
    kDebug() << comp1all;
    QCOMPARE(comp1all.count(), 3);
    QVERIFY(comp1all.contains("file1"));
    QVERIFY(comp1all.contains("file#a"));
    QVERIFY(comp1all.contains("file_subdir/"));
    QString comp1 = m_completion->replacedPath("file1"); // like KUrlRequester does
    QCOMPARE(comp1, QString("file1"));

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    // Completion from relative path
    kDebug() << endl << "now completing on 'file#'";
    m_completion->makeCompletion("file#");
    waitForCompletion();
    QStringList compall = m_completion->allMatches();
    kDebug() << compall;
    QCOMPARE(compall.count(), 1);
    QCOMPARE(compall.first(), QString("file#a"));
    QString comp2 = m_completion->replacedPath(compall.first()); // like KUrlRequester does
    QCOMPARE(comp2, QString("file#a"));
#endif

    // Completion with empty string
    kDebug () << endl << "now completing on ''";
    m_completion->makeCompletion("");
    waitForCompletion();
    QStringList compEmpty = m_completion->allMatches();
    QCOMPARE(compEmpty.count(), 0);
}

void KUrlCompletionTest::testLocalAbsolutePath()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    // Completion from absolute path
    kDebug() << m_dir+"file#";
    m_completion->makeCompletion(m_dir + "file#");
    waitForCompletion();
    QStringList compall = m_completion->allMatches();
    kDebug() << compall;
    QCOMPARE(compall.count(), 1);
    QString comp = compall.first();
    QCOMPARE(comp, QString(m_dir + "file#a"));
    comp = m_completion->replacedPath(comp); // like KUrlRequester does
    QCOMPARE(comp, QString(m_dir + "file#a"));
#endif
}

void KUrlCompletionTest::testLocalURL()
{
    // Completion from URL
    kDebug() ;
    QUrl url = QUrl::fromLocalFile(m_dirURL.toLocalFile() + "file");
    m_completion->makeCompletion(url.toString());
    waitForCompletion();
    QStringList comp1all = m_completion->allMatches();
    kDebug() << comp1all;
    QCOMPARE(comp1all.count(), 3);
    kDebug() << "Looking for" << m_dirURL.toString() + "file1";
    QVERIFY(comp1all.contains(m_dirURL.toString() + "file1"));
    QVERIFY(comp1all.contains(m_dirURL.toString() + "file_subdir/"));
    QString filehash = m_dirURL.toString() + "file%23a";
    kDebug() << "Looking for" << filehash;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QVERIFY(comp1all.contains(filehash));
    QString filehashPath = m_completion->replacedPath(filehash); // note that it returns a path!!
    kDebug() << filehashPath;
    QCOMPARE(filehashPath, QString(m_dirURL.toLocalFile() + "file#a"));
#endif

    // Completion from URL with no match
    url = QUrl::fromLocalFile(m_dirURL.toLocalFile() + "foobar");
    kDebug() << "makeCompletion(" << url << ")";
    QString comp2 = m_completion->makeCompletion(url.toString());
    QVERIFY(comp2.isEmpty());
    waitForCompletion();
    QVERIFY(m_completion->allMatches().isEmpty());

    // Completion from URL with a ref -> no match
    url = QUrl::fromLocalFile(m_dirURL.toLocalFile() + 'f');
    url.setFragment("ref");
    kDebug() << "makeCompletion(" << url << ")";
    m_completion->makeCompletion(url.toString());
    waitForCompletion();
    QVERIFY(m_completion->allMatches().isEmpty());
}

void KUrlCompletionTest::testEmptyCwd()
{
    kDebug() ;
    // Completion with empty string (with a KUrlCompletion whose cwd is "")
    kDebug () << endl << "now completing on '' with empty cwd";
    m_completionEmptyCwd->makeCompletion("");
    waitForCompletion();
    QStringList compEmpty = m_completionEmptyCwd->allMatches();
    QCOMPARE(compEmpty.count(), 0);
}

void KUrlCompletionTest::test()
{
    setup();
    testLocalRelativePath();
    testLocalAbsolutePath();
    testLocalURL();
    testEmptyCwd();
    teardown();

    // Try again, with another QTemporaryDir (to check that the caching doesn't give us wrong results)
    setup();
    testLocalRelativePath();
    testLocalAbsolutePath();
    testLocalURL();
    testEmptyCwd();
    teardown();
}

QTEST_MAIN(KUrlCompletionTest)

#include "kurlcompletiontest.moc"
