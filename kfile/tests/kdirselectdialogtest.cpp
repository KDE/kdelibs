#include <kapplication.h>
#include <kdirselectdialog.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <kurl.h>


int main( int argc, char **argv )
{
    KCmdLineArgs::init(argc, argv, "kdirselectdialogtest", 0, qi18n("kdirselectdialogtest"), "0", qi18n("test app"));

    KCmdLineOptions opt;
    opt.add("+[startDir]", qi18n("Directory to start in"), QByteArray());

    KCmdLineArgs::addCmdLineOptions(opt);

    KApplication app;
    app.setQuitOnLastWindowClosed(false);

    QUrl startDir = KCmdLineArgs::parsedArgs()->count() >= 1 ?
                    KCmdLineArgs::parsedArgs()->url(0) : QUrl();
    QUrl u = KDirSelectDialog::selectDirectory(startDir);
    if ( u.isValid() )
        KMessageBox::information(NULL,
                                 QString("You selected the url: %1")
                                 .arg(u.toString()), "Selected URL");

    return 0;
}
