#include <kapplication.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kpropertiesdialog.h>
#include <kcmdlineargs.h>

int main ( int argc, char** argv )
{
    KAboutData aboutData("kpropertiesdialogtest", "KIO Properties Dialog Test", "1.0");
    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    // This is the test for the KPropertiesDialog constructor that is now
    // documented to NOT work. Passing only a URL means a KIO::stat will happen,
    // and asking for the dialog to be modal too creates problems.
    // (A non-modal, URL-only dialog is the one kicker uses for app buttons, no problem there)
    KURL u;
    u.setPath( "/tmp" );
    KPropertiesDialog* dlg = new KPropertiesDialog( u, 0, 0, true /*modal*/, false /*we do exec ourselves*/ );
    dlg->exec();

    return 0;
}
