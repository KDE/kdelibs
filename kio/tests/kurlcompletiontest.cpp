#include <kapplication.h>
#include <kurlcompletion.h>
#include <kdebug.h>
#include <qdir.h>
#include <assert.h>

void testReplacedPath()
{
    KURLCompletion* completion = new KURLCompletion;
    assert( completion->dir() == QDir::homeDirPath() ); // the documented default
    completion->setDir( "/etc" );
    QString rep = completion->replacedPath( "passwd" );
    kdDebug() << "rep=" << rep << endl;
    assert( rep == "/etc/passwd" );
}

int main( int argc, char **argv )
{
    KApplication app( argc, argv, "kurlcompletion" );
    testReplacedPath();

    return 0;
}
