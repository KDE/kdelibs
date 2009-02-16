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

#include <kapplication.h>
#include <kurlcompletion.h>
#include <kdebug.h>
#include <QtCore/QDir>
#include <assert.h>
#include <QtCore/QFile>
#include <ktempdir.h>
#include <kcmdlineargs.h>
#include <unistd.h>

class KUrlCompletionTest
{
public:
    KUrlCompletionTest() {}
    ~KUrlCompletionTest() { teardown(); }
    void setup( bool setDirAsURL );
    void teardown();
    void testLocalRelativePath();
    void testLocalAbsolutePath();
    void testLocalURL();
    void testEmptyCwd();

private:
    void waitForCompletion();
    KUrlCompletion* m_completion;
    KTempDir* m_tempDir;
    KUrl m_dirURL;
    QString m_dir;
    KUrlCompletion* m_completionEmptyCwd;
};

void KUrlCompletionTest::setup( bool setDirAsURL )
{
    kDebug() ;
    m_completion = new KUrlCompletion;
    m_tempDir = new KTempDir;
    m_dir = m_tempDir->name();
    Q_ASSERT( m_dir.endsWith( "/" ) );
    m_dir += "Dir With#Spaces/";
    QDir().mkdir(m_dir);
    kDebug() << "m_dir=" << m_dir;
    if ( setDirAsURL ) {
        m_completion->setDir( KUrl(m_dir).url() );
    } else {
        m_completion->setDir( m_dir );
    }
    m_dirURL.setPath( m_dir );

    QFile f1( m_dir + "/file1" );
    bool ok = f1.open( QIODevice::WriteOnly );
    assert( ok );
    f1.close();

    QFile f2( m_dir + "/file#a" );
    ok = f2.open( QIODevice::WriteOnly );
    assert( ok );
    f2.close();

    QDir().mkdir( m_dir + "/file_subdir" );

    m_completionEmptyCwd = new KUrlCompletion;
    m_completionEmptyCwd->setDir( "" );
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
    assert( comp1all.count() == 3 );
    assert( comp1all.contains( "file1" ) );
    assert( comp1all.contains( "file#a" ) );
    assert( comp1all.contains( "file_subdir/" ) );
    QString comp1 = m_completion->replacedPath( "file1" ); // like KUrlRequester does
    assert( comp1 == "file1" );

    // Completion from relative path
    kDebug() << endl << "now completing on 'file#'";
    m_completion->makeCompletion( "file#" );
    waitForCompletion();
    QStringList compall = m_completion->allMatches();
    kDebug() << compall;
    assert( compall.count() == 1 );
    assert( compall.first() == "file#a" );
    QString comp2 = m_completion->replacedPath( compall.first() ); // like KUrlRequester does
    assert( comp2 == "file#a" );

    // Completion with empty string
    kDebug () << endl << "now completing on ''";
    m_completion->makeCompletion( "" );
    waitForCompletion();
    QStringList compEmpty = m_completion->allMatches();
    assert( compEmpty.count() == 0 );
}

void KUrlCompletionTest::testLocalAbsolutePath()
{
    // Completion from absolute path
    kDebug() << m_dir+"file#";
    m_completion->makeCompletion( m_dir + "file#" );
    waitForCompletion();
    QStringList compall = m_completion->allMatches();
    kDebug() << compall;
    assert( compall.count() == 1 );
    QString comp = compall.first();
    assert( comp == m_dir + "file#a" );
    comp = m_completion->replacedPath( comp ); // like KUrlRequester does
    assert( comp == m_dir + "file#a" );
}

void KUrlCompletionTest::testLocalURL()
{
    // Completion from URL
    kDebug() ;
    KUrl url( m_dirURL.path() + "file" );
    m_completion->makeCompletion( url.prettyUrl() );
    waitForCompletion();
    QStringList comp1all = m_completion->allMatches();
    kDebug() << comp1all;
    assert( comp1all.count() == 3 );
    kDebug() << "Looking for" << m_dirURL.prettyUrl() + "file1";
    assert( comp1all.contains( m_dirURL.prettyUrl() + "file1" ) );
    assert( comp1all.contains( m_dirURL.prettyUrl() + "file_subdir/" ) );
    QString filehash = m_dirURL.prettyUrl() + "file%23a";
    assert( comp1all.contains( filehash ) );
    QString filehashPath = m_completion->replacedPath( filehash ); // note that it returns a path!!
    kDebug() << filehashPath;
    assert( filehashPath == m_dirURL.path() + "file#a" );

    // Completion from URL with no match
    url = KUrl( m_dirURL.path() + "foobar" );
    kDebug() << "makeCompletion(" << url << ")";
    QString comp2 = m_completion->makeCompletion( url.prettyUrl() );
    assert( comp2.isEmpty() );
    waitForCompletion();
    assert( m_completion->allMatches().isEmpty() );

    // Completion from URL with a ref -> no match
    url = KUrl( m_dirURL.path() + 'f' );
    url.setRef( "ref" );
    kDebug() << "makeCompletion(" << url << ")";
    m_completion->makeCompletion( url.prettyUrl() );
    waitForCompletion();
    assert( m_completion->allMatches().isEmpty() );
}

void KUrlCompletionTest::testEmptyCwd()
{
    kDebug() ;
    // Completion with empty string (with a KUrlCompletion whose cwd is "")
    kDebug () << endl << "now completing on '' with empty cwd";
    m_completionEmptyCwd->makeCompletion( "" );
    waitForCompletion();
    QStringList compEmpty = m_completionEmptyCwd->allMatches();
    assert( compEmpty.count() == 0 );
}

int main( int argc, char **argv )
{
    //KApplication::disableAutoDcopRegistration();
    KCmdLineArgs::init(argc,argv, "kurlcompletiontest", 0, ki18n("kurlcompletiontest"), 0);
    KApplication app;

    {
        KUrlCompletionTest test;
        test.setup( false );
        test.testLocalRelativePath();
        test.testLocalAbsolutePath();
        test.testLocalURL();
        test.testEmptyCwd();
        test.teardown();

        // Try again, with another KTempDir (to check that the caching doesn't give us wrong results)
        test.setup( true );
        test.testLocalRelativePath();
        test.testLocalAbsolutePath();
        test.testLocalURL();
        test.testEmptyCwd();
        test.teardown();
    }
    qDebug( "All tests OK." );

    return 0;
}
