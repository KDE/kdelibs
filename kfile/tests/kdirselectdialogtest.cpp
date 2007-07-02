#include <kapplication.h>
#include <kdirselectdialog.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <kurl.h>


int main( int argc, char **argv )
{
    KCmdLineArgs::init(argc, argv, "kdirselectdialogtest", 0, ki18n("kdirselectdialogtest"), "0", ki18n("test app"));
    KApplication app;
    app.setQuitOnLastWindowClosed(false);

    KUrl u = KDirSelectDialog::selectDirectory( (argc >= 1) ? argv[1] : QString());
    if ( u.isValid() )
        KMessageBox::information( 0L,
                                QString("You selected the url: %1")
                                .arg( u.prettyUrl() ), "Selected URL" );

    return 0;
}
