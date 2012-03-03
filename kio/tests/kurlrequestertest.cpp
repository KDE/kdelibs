#include <qapplication.h>
#include <keditlistbox.h>
#include <kurlrequester.h>
#include <kurlrequesterdialog.h>
#include <QDebug>

int main( int argc, char **argv )
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    QUrl url = KUrlRequesterDialog::getUrl( "ftp://ftp.kde.org" );
    qDebug() << "Selected url:" << url;

    KUrlRequester *req = new KUrlRequester();
#ifndef KDE_NO_DEPRECATED
    KEditListBox *el = new KEditListBox( QLatin1String("Test"), req->customEditor() );
#else
    KEditListWidget *el = new KEditListWidget( req->customEditor() );
    el->setWindowTitle( QLatin1String("Test") );
#endif
    el->show();

    KUrlRequester *req1 = new KUrlRequester();
    req1->fileDialog();
    req1->setWindowTitle("AAAAAAAAAAAA");
    req1->show();

    return app.exec();
}
