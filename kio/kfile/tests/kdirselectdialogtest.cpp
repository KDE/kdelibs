#include <kapplication.h>
#include <kdirselectdialog.h>
#include <kmessagebox.h>
#include <kurl.h>

int main( int argc, char **argv )
{
    KApplication app;

    KURL u = KDirSelectDialog::selectDirectory( (argc >= 1) ? argv[1] : QString());
    if ( u.isValid() )
        KMessageBox::information( 0L,
                                QLatin1String("You selected the url: %1")
                                .arg( u.prettyURL() ), "Selected URL" );

    return 0;
}
