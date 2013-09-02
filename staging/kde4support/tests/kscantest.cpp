#include <QApplication>
#include <kscan.h>
int main( int argc, char **argv )
{
    QApplication::setApplicationName("kscantest");
    QApplication app(argc, argv);
    KScanDialog *dlg = KScanDialog::getScanDialog();
    if ( !dlg ) {
	qDebug("*** EEK, no Scan-service available, aborting!");
	return 0;
    }

    dlg->show();

    return app.exec();
}
