#include <kapplication.h>
#include <kio/defaultprogress.h>
#include <kurl.h>
#include <kdebug.h>

using namespace KIO;

int main(int argc, char **argv)
{
    KApplication app(argc, argv, "kdefaultprogresstest");

    DefaultProgress* dlg = new DefaultProgress();
    KURL src("http://this.host.doesnt.exist/this/dir/neither/andthisfileneither");
    KURL dest("file:/tmp/dest");
    dlg->slotCopying( 0L, src, dest );
    dlg->slotTotalSize( 0L, 12000 );
    dlg->slotTotalFiles( 0L, 12 );
    dlg->slotTotalDirs( 0L, 1 );

    dlg->slotSpeed( 0L, 55 );
    dlg->slotInfoMessage( 0L, QString::fromLatin1( "Starting..." ) );

    int files = 0;
    for ( int size = 0 ; size < 12000 ; size += 1 )
    {
        dlg->slotProcessedSize( 0L, size );
        dlg->slotPercent( 0L, 100 * size / 12000 );
        if ( size % 1000 == 0 )
        {
            dlg->slotProcessedFiles( 0L, ++files );
        }
        kapp->processEvents();
    }
    dlg->slotInfoMessage( 0L, QString::fromLatin1( "Done." ) );

    delete dlg;
    return 0;
}

