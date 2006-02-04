#include <kapplication.h>
#include <kurl.h>
#include <kio/defaultprogress.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
using namespace KIO;

int main(int argc, char **argv)
{
    KCmdLineArgs::init( argc, argv, "kdefaultprogresstest", 0, 0, 0, 0 );
    KApplication app;

    DefaultProgress* dlg = new DefaultProgress();
    KUrl src("http://this.host.doesn't.exist/this/dir/neither/andthisfileneither");
    KUrl dest("file:/tmp/dest");
    dlg->slotCopying( 0L, src, dest );
    dlg->slotTotalSize( 0L, 12000 );
    dlg->slotTotalFiles( 0L, 12 );
    dlg->slotTotalDirs( 0L, 1 );

    dlg->slotSpeed( 0L, 55 );
    dlg->slotInfoMessage( 0L, QLatin1String( "Starting..." ) );

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
    dlg->slotInfoMessage( 0L, QLatin1String( "Done." ) );

    delete dlg;
    return 0;
}

