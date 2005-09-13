#include <kapplication.h>
#include <keditlistbox.h>
#include <kurlrequester.h>
#include <kurlrequesterdlg.h>

int main( int argc, char **argv )
{
    KApplication app;
    KURL url = KURLRequesterDlg::getURL( "ftp://ftp.kde.org" );
    qDebug( "Selected url: %s", url.url().latin1());

    KURLRequester *req = new KURLRequester();
    KEditListBox *el = new KEditListBox( QLatin1String("Test"), req->customEditor() );
    el->show();
    return app.exec();
}
