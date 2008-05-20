#include <kapplication.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kpropertiesdialog.h>
#include <kcmdlineargs.h>


int main ( int argc, char** argv )
{
    KCmdLineOptions options;
    options.add("+url", ki18n("the path or url to the file/dir for which to show properties"));

    KAboutData aboutData(QByteArray("kpropertiesdialogtest"), QByteArray(), ki18n("KIO Properties Dialog Test"), QByteArray("1.0"));
    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if ( args->count() < 1 )
        KCmdLineArgs::usage();
    KUrl u = args->url( 0 );

    // This is the test for the KPropertiesDialog constructor that is now
    // documented to NOT work. Passing only a URL means a KIO::NetAccess::stat will happen,
    // and asking for the dialog to be modal too creates problems.
    // (A non-modal, URL-only dialog is the one kicker uses for app buttons, no problem there)
    {
    KPropertiesDialog dlg( u, 0 );
    dlg.exec();
    }

    return 0;
}
