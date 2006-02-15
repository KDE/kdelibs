#include <kapplication.h>
#include <kdirselectdialog.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <kurl.h>
#include <QString>

int main( int argc, char **argv )
{
    KCmdLineArgs::init(argc, argv, "kdirselectdialogtest","kdirselectdialogtest","test app","0");
    KApplication app;
    app.setQuitOnLastWindowClosed(false);

    KUrl u = KDirSelectDialog::selectDirectory( (argc >= 1) ? argv[1] : QString());
    if ( u.isValid() )
        KMessageBox::information( 0L,
                                QString("You selected the url: %1")
                                .arg( u.prettyURL() ), "Selected URL" );

    return 0;
}
