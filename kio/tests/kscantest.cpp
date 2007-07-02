#include <kapplication.h>
#include <kscan.h>
#include <kcmdlineargs.h>
int main( int argc, char **argv )
{
    KCmdLineArgs::init( argc, argv, "kscantest", 0, ki18n("kscantest"), 0);
    KApplication app;
    KScanDialog *dlg = KScanDialog::getScanDialog();
    if ( !dlg ) {
	qDebug("*** EEK, no Scan-service available, aborting!");
	return 0;
    }

    dlg->show();

    return app.exec();
}
