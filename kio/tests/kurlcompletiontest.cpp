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
#include <qdir.h>
#include <assert.h>
#include <qfile.h>
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

private:
    void waitForCompletion();
    KUrlCompletion* m_completion;
    KTempDir* m_tempDir;
    KUrl m_dirURL;
    QString m_dir;
};

void KUrlCompletionTest::setup( bool setDirAsURL )
{
    kDebug() << k_funcinfo << endl;
    m_completion = new KUrlCompletion;
    m_tempDir = new KTempDir;
    m_tempDir->setAutoDelete( true );
    m_dir = m_tempDir->name();
    kDebug() << "m_dir=" << m_dir << endl;
    Q_ASSERT( m_dir.endsWith( "/" ) );
    if ( setDirAsURL ) {
        KUrl d; d.setPath( m_dir );
        m_completion->setDir( d.url() );
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
}

void KUrlCompletionTest::teardown()
{
    delete m_completion;
    m_completion = 0;
    delete m_tempDir;
    m_tempDir = 0;
}
void KUrlCompletionTest::waitForCompletion()
{
    while ( m_completion->isRunning() ) {
        kDebug() << "waiting for thread..." << endl;
        usleep( 10 );
    }
}

void KUrlCompletionTest::testLocalRelativePath()
{
    kDebug() << k_funcinfo << endl;
    // Completion from relative path, with two matches
    m_completion->makeCompletion( "f" );
    waitForCompletion();
    QStringList comp1all = m_completion->allMatches();
    assert( comp1all.count() == 2 );
    assert( comp1all.find( "file1" ) != comp1all.end() );
    assert( comp1all.find( "file#a" ) != comp1all.end() );
    QString comp1 = m_completion->replacedPath( "file1" ); // like KUrlRequester does
    assert( comp1 == "file1" );

    // Completion from relative path
    kDebug() << endl << k_funcinfo << "now completing on 'file#'" << endl;
    m_completion->makeCompletion( "file#" );
    waitForCompletion();
    QStringList compall = m_completion->allMatches();
    kDebug() << compall << endl;
    assert( compall.count() == 1 );
    assert( compall.first() == "file#a" );
    QString comp2 = m_completion->replacedPath( compall.first() ); // like KUrlRequester does
    assert( comp2 == "file#a" );
}

void KUrlCompletionTest::testLocalAbsolutePath()
{
    // Completion from absolute path
    kDebug() << k_funcinfo << m_dir+"file#" << endl;
    m_completion->makeCompletion( m_dir + "file#" );
    waitForCompletion();
    QStringList compall = m_completion->allMatches();
    kDebug() << compall << endl;
    assert( compall.count() == 1 );
    QString comp = compall.first();
    assert( comp == m_dir + "file#a" );
    comp = m_completion->replacedPath( comp ); // like KUrlRequester does
    assert( comp == m_dir + "file#a" );
}

void KUrlCompletionTest::testLocalURL()
{
    // Completion from URL
    kDebug() << k_funcinfo << endl;
    KUrl url = KUrl::fromPathOrURL( m_dirURL.path() + "file" );
    m_completion->makeCompletion( url.prettyURL() );
    waitForCompletion();
    QStringList comp1all = m_completion->allMatches();
    kDebug() << comp1all << endl;
    assert( comp1all.count() == 2 );
    assert( comp1all.find( m_dirURL.url() + "file1" ) != comp1all.end() );
    QString filehash = m_dirURL.url() + "file%23a";
    assert( comp1all.find( filehash ) != comp1all.end() );
    QString filehashPath = m_completion->replacedPath( filehash ); // note that it returns a path!!
    kDebug() << filehashPath << endl;
    assert( filehashPath == m_dirURL.path() + "file#a" );

    // Completion from URL with no match
    url = KUrl::fromPathOrURL( m_dirURL.path() + "foobar" );
    kDebug() << k_funcinfo << "makeCompletion(" << url << ")" << endl;
    QString comp2 = m_completion->makeCompletion( url.prettyURL() );
    assert( comp2.isEmpty() );
    waitForCompletion();
    assert( m_completion->allMatches().isEmpty() );

    // Completion from URL with a ref -> no match
    url = KUrl::fromPathOrURL( m_dirURL.path() + "f" );
    url.setRef( "ref" );
    kDebug() << k_funcinfo << "makeCompletion(" << url << ")" << endl;
    m_completion->makeCompletion( url.prettyURL() );
    waitForCompletion();
    assert( m_completion->allMatches().isEmpty() );
}

int main( int argc, char **argv )
{
    KApplication::disableAutoDcopRegistration();
    KCmdLineArgs::init(argc,argv,"kurlcompletiontest", 0, 0, 0, 0);
    KApplication app;

    {
        KUrlCompletionTest test;
        test.setup( false );
        test.testLocalRelativePath();
        test.testLocalAbsolutePath();
        test.testLocalURL();
        test.teardown();

        // Try again, with another KTempDir (to check that the caching doesn't give us wrong results)
        test.setup( true );
        test.testLocalRelativePath();
        test.testLocalAbsolutePath();
        test.testLocalURL();
        test.teardown();
    }
    qDebug( "All tests OK." );

    return 0;
}
