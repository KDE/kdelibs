#include <QtGui/QApplication>
#include <kcomponentdata.h>
#include <kicondialog.h>

int main( int argc, char **argv )
{
    QApplication app(argc, argv);
    KComponentData componentData(QByteArray("kicondialogtest"));

//    KIconDialog::getIcon();

    KIconButton button;
    button.show();


    return app.exec();
}

/* vim: et sw=4
 */
