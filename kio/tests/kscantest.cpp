#include <kapplication.h>
#include <kscan.h>

int main( int argc, char **argv )
{
    KApplication app( argc, argv, "kscantest" );
    KScanDialog *dlg = KScanDialog::getScanDialog();
    if ( !dlg ) {
	qDebug("*** EEK, no Scan-service available, aborting!");
	return 0;
    }
    
    dlg->show();

    return app.exec();
}
