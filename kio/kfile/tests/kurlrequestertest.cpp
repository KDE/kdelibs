#include <kapplication.h>
#include <kurlrequesterdlg.h>

int main( int argc, char **argv )
{
    KApplication app( argc, argv, "kurlrequestertest" );
    KURL url = KURLRequesterDlg::getURL( "ftp://ftp.kde.org" );
    qDebug( "Selected url: %s", url.url().latin1());
}
