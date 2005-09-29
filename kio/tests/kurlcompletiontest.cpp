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

class KURLCompletionTest
{
public:
    KURLCompletionTest() {}
    ~KURLCompletionTest() { teardown(); }
    void setup( bool setDirAsURL );
    void teardown();
    void testLocalRelativePath();
    void testLocalAbsolutePath();
    void testLocalURL();

private:
    void waitForCompletion();
    KURLCompletion* m_completion;
    KTempDir* m_tempDir;
    KURL m_dirURL;
    QString m_dir;
};

void KURLCompletionTest::setup( bool setDirAsURL )
{
    kdDebug() << k_funcinfo << endl;
    m_completion = new KURLCompletion;
    m_tempDir = new KTempDir;
    m_tempDir->setAutoDelete( true );
    m_dir = m_tempDir->name();
    kdDebug() << "m_dir=" << m_dir << endl;
    Q_ASSERT( m_dir.endsWith( "/" ) );
    if ( setDirAsURL ) {
        KURL d; d.setPath( m_dir );
        m_completion->setDir( d.url() );
    } else {
        m_completion->setDir( m_dir );
    }
    m_dirURL.setPath( m_dir );

    QFile f1( m_dir + "/file1" );
    bool ok = f1.open( IO_WriteOnly );
    assert( ok );
    f1.close();

    QFile f2( m_dir + "/file#a" );
    ok = f2.open( IO_WriteOnly );
    assert( ok );
    f2.close();
}

void KURLCompletionTest::teardown()
{
    delete m_completion;
    m_completion = 0;
    delete m_tempDir;
    m_tempDir = 0;
}
void KURLCompletionTest::waitForCompletion()
{
    while ( m_completion->isRunning() ) {
        kdDebug() << "waiting for thread..." << endl;
        usleep( 10 );
    }
}

void KURLCompletionTest::testLocalRelativePath()
{
    kdDebug() << k_funcinfo << endl;
    // Completion from relative path, with two matches
    m_completion->makeCompletion( "f" );
    waitForCompletion();
    QStringList comp1all = m_completion->allMatches();
    assert( comp1all.count() == 2 );
    assert( comp1all.find( "file1" ) != comp1all.end() );
    assert( comp1all.find( "file#a" ) != comp1all.end() );
    QString comp1 = m_completion->replacedPath( "file1" ); // like KURLRequester does
    assert( comp1 == "file1" );

    // Completion from relative path
    kdDebug() << endl << k_funcinfo << "now completing on 'file#'" << endl;
    m_completion->makeCompletion( "file#" );
    waitForCompletion();
    QStringList compall = m_completion->allMatches();
    kdDebug() << compall << endl;
    assert( compall.count() == 1 );
    assert( compall.first() == "file#a" );
    QString comp2 = m_completion->replacedPath( compall.first() ); // like KURLRequester does
    assert( comp2 == "file#a" );
}

void KURLCompletionTest::testLocalAbsolutePath()
{
    // Completion from absolute path
    kdDebug() << k_funcinfo << m_dir+"file#" << endl;
    m_completion->makeCompletion( m_dir + "file#" );
    waitForCompletion();
    QStringList compall = m_completion->allMatches();
    kdDebug() << compall << endl;
    assert( compall.count() == 1 );
    QString comp = compall.first();
    assert( comp == m_dir + "file#a" );
    comp = m_completion->replacedPath( comp ); // like KURLRequester does
    assert( comp == m_dir + "file#a" );
}

void KURLCompletionTest::testLocalURL()
{
    // Completion from URL
    kdDebug() << k_funcinfo << endl;
    KURL url = KURL::fromPathOrURL( m_dirURL.path() + "file" );
    m_completion->makeCompletion( url.prettyURL() );
    waitForCompletion();
    QStringList comp1all = m_completion->allMatches();
    kdDebug() << comp1all << endl;
    assert( comp1all.count() == 2 );
    assert( comp1all.find( m_dirURL.url() + "file1" ) != comp1all.end() );
    QString filehash = m_dirURL.url() + "file%23a";
    assert( comp1all.find( filehash ) != comp1all.end() );
    QString filehashPath = m_completion->replacedPath( filehash ); // note that it returns a path!!
    kdDebug() << filehashPath << endl;
    assert( filehashPath == m_dirURL.path() + "file#a" );

    // Completion from URL with no match
    url = KURL::fromPathOrURL( m_dirURL.path() + "foobar" );
    kdDebug() << k_funcinfo << "makeCompletion(" << url << ")" << endl;
    QString comp2 = m_completion->makeCompletion( url.prettyURL() );
    assert( comp2.isEmpty() );
    waitForCompletion();
    assert( m_completion->allMatches().isEmpty() );

    // Completion from URL with a ref -> no match
    url = KURL::fromPathOrURL( m_dirURL.path() + "f" );
    url.setRef( "ref" );
    kdDebug() << k_funcinfo << "makeCompletion(" << url << ")" << endl;
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
        KURLCompletionTest test;
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
