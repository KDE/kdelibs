#include <kapplication.h>
#include <kdirselectdialog.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <kurl.h>


int main( int argc, char **argv )
{
    KCmdLineArgs::init(argc, argv, "kdirselectdialogtest", 0, ki18n("kdirselectdialogtest"), "0", ki18n("test app"));

    KCmdLineOptions opt;
    opt.add("+[startDir]", ki18n("Directory to start in"), QByteArray());

    KCmdLineArgs::addCmdLineOptions(opt);

    KApplication app;
    app.setQuitOnLastWindowClosed(false);

    KUrl u = KDirSelectDialog::selectDirectory( KCmdLineArgs::parsedArgs()->count() >= 1 ? 
        KCmdLineArgs::parsedArgs()->arg(0) : QString() );
    if ( u.isValid() )
        KMessageBox::information( 0L,
                                QString("You selected the url: %1")
                                .arg( u.prettyUrl() ), "Selected URL" );

    return 0;
}
