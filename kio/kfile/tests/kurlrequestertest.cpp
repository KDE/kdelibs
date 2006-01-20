#include <kapplication.h>
#include <kcmdlineargs.h>
#include <keditlistbox.h>
#include <kurlrequester.h>
#include <kurlrequesterdlg.h>

int main( int argc, char **argv )
{
    KCmdLineArgs::init(argc, argv, "kurlrequestertest","kurlrequestertest","test app","0");
    KApplication app;
    app.setQuitOnLastWindowClosed(false);
 
    KUrl url = KUrlRequesterDlg::getURL( "ftp://ftp.kde.org" );
    qDebug( "Selected url: %s", url.url().latin1());

    KUrlRequester *req = new KUrlRequester();
    KEditListBox *el = new KEditListBox( QLatin1String("Test"), req->customEditor() );
    el->show();

    KUrlRequester *req1 = new KUrlRequester();
    req1->fileDialog();
    req1->setWindowTitle("AAAAAAAAAAAA");
    req1->show();
   
    return app.exec();
}
